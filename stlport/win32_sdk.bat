rem parameters in order of their apearance:
rem - path to unpacked tarball
rem - relative path back to module root

setlocal

cd %1

rem type %2\dos_lineends.patch | patch -b -p2
patch -i %2\STLport-4.0_SDK.patch -b -p2

