#! /bin/bash
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

root=`dirname $0`
root=`cd $root/../.. && pwd`

if test -z $1; then
headers=`ls $root/*/inc/pch/precompiled_*.hxx`
else
headers="$1"
fi

for x in $headers; do
header=$x
echo updating `echo $header | sed -e s%$root/%%`
module=`echo $header | sed -e s%$root/%% -e s%/.*%%`
name=`echo $header | sed -e s/.*precompiled_// -e s/\.hxx//`
makefile="$root/$module/Library_$name.mk"

tmpfile=`mktemp`

cat "$makefile" | sed 's#\\$##' | \
    (
    inobjects=
    while read line ; do
    if (test "$line" = "))") || (echo $line | grep -q ", "); then
        inobjects=
    elif echo $line | grep -q -e add_exception_objects -e add_noexception_objects -e add_cxxobject -e add_cxxobjects ; then
        inobjects=1
    elif test -n "$inobjects"; then
        file=$line
        if ! test -f "$root/$file".cxx ; then
            echo No file $file in makefile `echo $makefile | sed -e s%$root/%%` >&2
        else
            cat "$root/$file".cxx | grep -e '^\s*#include' | sed 's/\(#include [<"][^<"]*[>"]\).*/\1/' | sed 's#\.\./##g#' >>$tmpfile
        fi
    fi
    done
    )

cat >$header <<EOF
/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

EOF

localdir="`dirname $makefile`"

function local_file()
(
    file="$1"
    find "$localdir" -type f | grep /"$file"'$' -q
)

function skip_ignore()
(
    grep -v -F -e '#include "gperffasttoken.hxx"'
)

# " in #include "foo" breaks echo down below, so " -> @
cat $tmpfile | sort -u | skip_ignore | sed 's/"/@/g' | \
    (
    while read line; do
        file=`echo $line | sed 's/.*[<"@]\(.*\)[>"@].*/\1/'`
        if ! local_file "$file"; then
            echo $line | sed 's/@/"/g' >>$header
        fi
    done
    )

cat >>$header <<EOF

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
EOF


rm $tmpfile
done
#echo Done.
exit 0
