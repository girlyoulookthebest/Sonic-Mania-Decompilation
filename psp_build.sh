cd dependencies
git clone https://github.com/smb123w64gb/RSDKv5-Decompilation.git --recursive -b PSP RSDKv5
cd ..
mkdir build
cd build
psp-cmake .. -DGAME_STATIC=ON -DRETRO_MOD_LOADER=OFF -DRETRO_REVISION=2 -DGAME_INCLUDE_EDITOR=OFF -DCMAKE_BUILD_TYPE=Release -DRETRO_AUDIO=ON -DRETRO_VIDEO_CUTSCENE=OFF -DRETRO_DISABLE_PLUS=ON
make -j$(nproc)