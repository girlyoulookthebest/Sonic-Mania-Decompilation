// ---------------------------------------------------------------------
// RSDK Project: Sonic Mania
// Object Description: UFO_Decoration Object
// Object Author: Christian Whitehead/Simon Thomley/Hunter Bridges
// Decompiled by: Rubberduckycooly & RMGRich
// ---------------------------------------------------------------------

#include "Game.h"

ObjectUFO_Decoration *UFO_Decoration;

#define UFO_DECOR_LOG 0
#if UFO_DECOR_LOG
#include <stdio.h>
static int32 dbgCreate = 0, dbgUpdate = 0, dbgOnScreen = 0, dbgLate = 0, dbgDraw = 0, dbgDrawGated = 0;
static int32 dbgZmin = 0x7FFFFFFF, dbgZmax = -0x7FFFFFFF;
static int32 dbgVisible = 0;
static int32 dbgReached = 0, dbgTypeBad = 0;
static int32 dbgTypeSeen[16], dbgTypeModel[16];
#endif

void UFO_Decoration_Update(void)
{
    RSDK_THIS(UFO_Decoration);

#if UFO_DECOR_LOG
    ++dbgUpdate;
#endif
    if (RSDK.CheckOnScreen(self, NULL)) {
#if UFO_DECOR_LOG
        ++dbgOnScreen;
#endif
        self->scale.x += (self->size - self->scale.x) >> 3;
        self->active = ACTIVE_NORMAL;
    }
    else {
        self->scale.x = 0x000;
        self->active  = ACTIVE_BOUNDS;
    }

    if (self->type > UFO_DECOR_PILLAR2)
        RSDK.ProcessAnimation(&self->animator);
}

void UFO_Decoration_LateUpdate(void)
{
    RSDK_THIS(UFO_Decoration);

    int32 x = self->position.x;
    int32 y = self->height;
    int32 z = self->position.y;

    Matrix *m = &UFO_Camera->matWorld;

    self->zdepth = m->values[2][1] * (y >> 16) + m->values[2][2] * (z >> 16) + m->values[2][0] * (x >> 16) + m->values[2][3];
#if UFO_DECOR_LOG
    ++dbgLate;
    if (self->zdepth < dbgZmin) dbgZmin = self->zdepth;
    if (self->zdepth > dbgZmax) dbgZmax = self->zdepth;
    if (self->visible) ++dbgVisible;
#endif

    if (self->zdepth >= 0x4000) {
        int32 depth = (int32)((m->values[0][3] << 8) + (m->values[0][2] * (z >> 8) & 0xFFFFFF00) + (m->values[0][0] * (x >> 8) & 0xFFFFFF00)
                              + (m->values[0][1] * (y >> 8) & 0xFFFFFF00))
                      / self->zdepth;
        self->visible = abs(depth) < 0x100;
    }
}

void UFO_Decoration_StaticUpdate(void)
{
#if UFO_DECOR_LOG
    static int32 frames = 0;
    if (++frames == 240) {
        FILE *f = fopen("ufo_decor.log", "w");
        if (f) {
            fprintf(f, "over 240 frames of UFO_Decoration:\n");
            fprintf(f, "  Create calls    : %d\n", (int)dbgCreate);
            fprintf(f, "  Update calls    : %d\n", (int)dbgUpdate);
            fprintf(f, "   of which onscreen: %d\n", (int)dbgOnScreen);
            fprintf(f, "  LateUpdate calls: %d\n", (int)dbgLate);
            fprintf(f, "   of which visible : %d\n", (int)dbgVisible);
            fprintf(f, "  Draw calls      : %d\n", (int)dbgDraw);
            fprintf(f, "   passing zdepth>=0x4000: %d\n", (int)dbgDrawGated);
            fprintf(f, "  zdepth range    : %d .. %d  (gate is 16384)\n", (int)dbgZmin, (int)dbgZmax);
            fprintf(f, "  reached AddModel: %d   (bad type: %d)\n", (int)dbgReached, (int)dbgTypeBad);
            fprintf(f, "  sceneIndex      : %d   (must be < 32)\n", (int)UFO_Decoration->sceneIndex);
            fprintf(f, "  MODEL_COUNT gate: a model id >= 256 is skipped silently\n");
            fprintf(f, "\n  type : draws : modelIndices[type]\n");
            for (int32 t = 0; t < 16; ++t)
                if (dbgTypeSeen[t] || dbgTypeModel[t])
                    fprintf(f, "    %2d : %5d : %d\n", (int)t, (int)dbgTypeSeen[t], (int)dbgTypeModel[t]);
            fclose(f);
        }
    }
#endif
}

void UFO_Decoration_Draw(void)
{
    RSDK_THIS(UFO_Decoration);
#if UFO_DECOR_LOG
    ++dbgDraw;
#endif

    if (self->zdepth >= 0x4000) {
#if UFO_DECOR_LOG
        ++dbgDrawGated;
#endif
        RSDK.Prepare3DScene(UFO_Decoration->sceneIndex);

        RSDK.MatrixScaleXYZ(&self->matTransform, self->scale.x, self->size, self->scale.x);
        RSDK.MatrixTranslateXYZ(&self->matTransform, self->position.x, self->height, self->position.y, 0);

        RSDK.MatrixRotateY(&self->matNormal, self->angle);
        RSDK.MatrixMultiply(&self->matWorld, &self->matNormal, &self->matTransform);
        RSDK.MatrixMultiply(&self->matWorld, &self->matWorld, &UFO_Camera->matWorld);

        if (UFO_Camera->isSS7)
            RSDK.MatrixMultiply(&self->matNormal, &self->matNormal, &UFO_Camera->matTemp);
        else
            RSDK.MatrixMultiply(&self->matNormal, &self->matNormal, &UFO_Camera->matView);

#if UFO_DECOR_LOG
        ++dbgReached;
        if (self->type >= 0 && self->type < 16) {
            dbgTypeSeen[self->type]++;
            dbgTypeModel[self->type] = UFO_Decoration->modelIndices[self->type];
        }
        else
            ++dbgTypeBad;
#endif
        if (self->type <= UFO_DECOR_PILLAR2)
            RSDK.AddModelTo3DScene(UFO_Decoration->modelIndices[self->type], UFO_Decoration->sceneIndex, UFO_Decoration->drawType, &self->matWorld,
                                   &self->matNormal, 0xFFFFFF);
        else
            RSDK.AddMeshFrameTo3DScene(UFO_Decoration->modelIndices[self->type], UFO_Decoration->sceneIndex, &self->animator,
                                       UFO_Decoration->drawType, &self->matWorld, &self->matNormal, 0xFFFFFF);

        RSDK.Draw3DScene(UFO_Decoration->sceneIndex);
    }
}

void UFO_Decoration_Create(void *data)
{
    RSDK_THIS(UFO_Decoration);
#if UFO_DECOR_LOG
    ++dbgCreate;
#endif

    if (!SceneInfo->inEditor) {
        if (!self->size)
            self->size = 0x400;

        self->visible       = true;
        self->drawGroup     = 4;
        self->active        = ACTIVE_BOUNDS;
        self->updateRange.x = 0x4000000;
        self->updateRange.y = 0x4000000;

        if (self->type == UFO_DECOR_BIRD)
            self->height = 0x600000;

        RSDK.SetModelAnimation(UFO_Decoration->modelIndices[self->type], &self->animator, 96, 0, true, 0);
    }
}

void UFO_Decoration_StageLoad(void)
{
    UFO_Decoration->modelIndices[UFO_DECOR_TREE]    = RSDK.LoadMesh("Decoration/Tree.bin", SCOPE_STAGE);
    UFO_Decoration->modelIndices[UFO_DECOR_FLOWER1] = RSDK.LoadMesh("Decoration/Flower1.bin", SCOPE_STAGE);
    UFO_Decoration->modelIndices[UFO_DECOR_FLOWER2] = RSDK.LoadMesh("Decoration/Flower2.bin", SCOPE_STAGE);
    UFO_Decoration->modelIndices[UFO_DECOR_FLOWER3] = RSDK.LoadMesh("Decoration/Flower3.bin", SCOPE_STAGE);
    UFO_Decoration->modelIndices[UFO_DECOR_PILLAR1] = RSDK.LoadMesh("Decoration/Pillar1.bin", SCOPE_STAGE);
    UFO_Decoration->modelIndices[UFO_DECOR_PILLAR2] = RSDK.LoadMesh("Decoration/Pillar2.bin", SCOPE_STAGE);
    UFO_Decoration->modelIndices[UFO_DECOR_BIRD]    = RSDK.LoadMesh("Decoration/Bird.bin", SCOPE_STAGE);
    UFO_Decoration->modelIndices[UFO_DECOR_FISH]    = RSDK.LoadMesh("Decoration/Fish.bin", SCOPE_STAGE);

    UFO_Decoration->sceneIndex = RSDK.Create3DScene("View:Special", 4096, SCOPE_STAGE);

    UFO_Decoration->drawType = S3D_SOLIDCOLOR_SHADED_BLENDED_SCREEN;
}

#if GAME_INCLUDE_EDITOR
void UFO_Decoration_EditorDraw(void) {}

void UFO_Decoration_EditorLoad(void)
{

    RSDK_ACTIVE_VAR(UFO_Decoration, type);
    RSDK_ENUM_VAR("Tree", UFO_DECOR_TREE);
    RSDK_ENUM_VAR("Flower 1", UFO_DECOR_FLOWER1);
    RSDK_ENUM_VAR("Flower 2", UFO_DECOR_FLOWER2);
    RSDK_ENUM_VAR("Flower 3", UFO_DECOR_FLOWER3);
    RSDK_ENUM_VAR("Pillar 1", UFO_DECOR_PILLAR1);
    RSDK_ENUM_VAR("Pillar 2", UFO_DECOR_PILLAR2);
    RSDK_ENUM_VAR("Bird", UFO_DECOR_BIRD);
    RSDK_ENUM_VAR("Fish", UFO_DECOR_FISH);
}
#endif

void UFO_Decoration_Serialize(void)
{
    RSDK_EDITABLE_VAR(UFO_Decoration, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(UFO_Decoration, VAR_ENUM, angle);
    RSDK_EDITABLE_VAR(UFO_Decoration, VAR_ENUM, size);
}
