#!/bin/sh

cat > /dev/null
[[ "${OUTPATH}" == unxlngx* ]] && mark64="()(64bit)"
echo "libfreetype.so.6${mark64}"

