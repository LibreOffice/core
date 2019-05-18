#! /bin/bash

# This script shows usage count for included files.
# It somewhat supplements/duplicates update_pch.sh .

# Usage:
# - modify solenv/gbuild/platform/com_GCC_class.mk and change those two -MMD to -MD
# - rebuild whatever you need analysed
# - $ analyse-includes-usage.sh <dir-in-Deps> >output.txt
# (where dir-in-Deps is e.g. workdir/Dep/GenCxxObject/UnpackedTarball/pdfium)
# - revert change in com_GCC_class.mk

function filterfiles
{
# filter out internal headers that are normally not included directly
grep -v -e '^[^ ]* /usr/include/bits/' -e '^[^ ]* /usr/include/c++/.*/.*/.*$'
}

function catdfiles
{
files=$(find "$1" -name '*.d')
for file in $files; do
    cat $file | grep '^.*:$' | sort -u | sed 's/:$//'
done
}

function countincludes
{
sort | uniq -c | ( while read count file; do echo $count $(realpath -L "$file"); done) | sort -n -k1 -r
}

echo Total .d files count: $(find "$1" -name '*.d' | wc -l)
catdfiles "$1" | countincludes | filterfiles
