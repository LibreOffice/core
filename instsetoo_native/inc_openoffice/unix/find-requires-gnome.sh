#!/bin/sh

cat > /dev/null
[[ "${OUTPATH}" == unxlngx* ]] && mark64="()(64bit)"
echo "libgnomevfs-2.so.0${mark64}"
echo "libgconf-2.so.4${mark64}"

