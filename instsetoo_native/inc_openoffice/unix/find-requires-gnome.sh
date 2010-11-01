#!/bin/sh

cat > /dev/null
[[ "${OUTPATH}" == unxlngx* ]] && mark64="()(64bit)"
if [[ "${OUTPATH}" == unxaig* ]]; then
  echo "libgnomevfs-2.a(libgnomevfs-2.so.0${mark64})"
  echo "libgconf-2.a(libgconf-2.so.4${mark64})"
else
  echo "libgnomevfs-2.so.0${mark64}"
  echo "libgconf-2.so.4${mark64}"
fi
