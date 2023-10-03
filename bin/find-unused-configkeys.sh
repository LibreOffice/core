#!/usr/bin/env bash
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# find group, set and key names in officecfg that are not used in the code base
# caveat: only effective for reasonably unique strings

for filename in $(find officecfg/ -name "*xcs"); do
    for gs in group set node-ref; do
        for gname in $(git grep -h "<$gs" "$filename" | awk -F'oor:name="' '{print $2}' | awk -F'"' '{print $1}') ; do
            if [ $(git grep "$gname" | grep -v ^officecfg | wc -l) -eq 0 ] ;
            then
                # group, set or node-ref names may serve as oor:node-type templates
                if [ $(git grep "$gname" officecfg | grep oor:node-type | wc -l ) -eq 0 ] ;
                then
                    echo "$gname group in "$filename" appears only in officecfg";
                fi
            fi
        done
    done

    for pname in $(git grep -h "<prop" "$filename" | awk -F'oor:name="' '{print $2}' | awk -F'"' '{print $1}') ; do
        if [ $(git grep "$pname" | grep -v ^officecfg | wc -l) -eq 0 ] ;
        then
            echo "$pname property in "$filename" appears only in officecfg";
        fi
    done

done
