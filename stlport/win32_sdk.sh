#!/bin/sh
#rem parameters in order of their apearance:
#rem - path to unpacked tarball
#rem - relative path back to module root

cd $1

patch -i $2/STLport-4.0_SDK.patch -b -p2

