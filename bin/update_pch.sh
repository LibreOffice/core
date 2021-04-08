#! /bin/bash
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Usage: update_pch.sh [<module>/inc/pch/precompiled_xxx.hxx]
# Usage: update_pch.sh [<module>]
# Invoke: make cmd cmd="./bin/update_pch.sh [..]"

if test -n "$SRC_DIR"; then
    root="$SRC_DIR"
else
    root=`dirname $0`
    root=`cd $root/.. >/dev/null && pwd`
fi
root=`readlink -f $root`
cd $root

if test -z "$1"; then
    headers=`ls ./*/inc/pch/precompiled_*.hxx`
else
    headers="$@"
fi

# Split the headers into an array.
IFS=' ' read -a aheaders <<< $headers
hlen=${#aheaders[@]};
if [ $hlen -gt 1 ]; then
    if [ -z "$PARALLELISM" ]; then
        PARALLELISM=0 # Let xargs decide
    fi
    echo $headers | xargs -n 1 -P $PARALLELISM $0
    exit $?
fi

for x in $headers; do
    if [ -d "$x" ]; then
        # We got a directory, find pch files to update.
        headers=`find $root/$x/ -type f -iname "precompiled_*.hxx"`
        if test -n "$headers"; then
            $0 "$headers"
        fi
    else
        header=$x
        update_msg=`echo $header | sed -e s%$root/%%`
        module=`readlink -f $header | sed -e s%$root/%% -e s%/.*%%`
        if [ "$module" = "pch" ]; then
            continue # PCH's in pch/inc/pch/ are handled manually
        fi
        echo updating $update_msg
        if [ "$module" = "external" ]; then
            module=external/`readlink -f $header | sed -e s%$root/external/%% -e s%/.*%%`
        fi
        libname=`echo $header | sed -e s/.*precompiled_// -e s/\.hxx//`

        ./bin/update_pch "$module" "$libname"
        exitcode=$?
        if test $exitcode -ne 0 -a $exitcode -ne 2; then
            echo Failed.
            exit 1
        fi
    fi
done

#echo Done.
exit 0
