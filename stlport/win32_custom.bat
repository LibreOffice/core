rem parameters in order of their apearance:
rem - name of subdirectory in "build"
rem - name of tarball root directory
rem - path to desired directory (seen from tarball root dir)

setlocal

cd build\%1

type  ..\..\dos_lineends.patch | patch -b -p2

