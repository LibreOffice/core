#!/bin/sh

cat > /dev/null
[[ "${OUTPATH}" == unxlngx* ]] && mark64="()(64bit)"
if [[ "${OUTPATH}" == unxaig* ]]; then
  echo "libfreetype.a(libfreetype.so.6${mark64})"
else
  echo "libfreetype.so.6${mark64}"
fi
