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
        # Search for all nodes with the given node type $gs
        for gname in $(git grep -aIh "<$gs" "$filename" 2>/dev/null | awk -F'oor:name="' '{print $2}' | awk -F'"' '{print $1}') ; do
            # Check whether a group name appears outside of officecfg
            if [ $(git grep -aI "$gname" 2>/dev/null | grep -saIv ^officecfg 2>/dev/null | wc -l) -eq 0 ] ;
            then
                # group, set or node-ref names may serve as oor:node-type templates
                # check whether this is also unused - report only if both are unused
                if [ $(git grep -aIh "oor:node-type=\"$gname" officecfg | wc -l ) -gt 0 ] ; then
                for tmpl in $(git grep -aIh oor:node-type=\""$gname" officecfg 2>/dev/null | awk -F'oor:name="' '{print $2}' | awk -F '"' '{print $1}' ) ; do
                    # check whether the set is used outside of officecfg
                    if [ $(git grep -aI "$tmpl" 2>/dev/null | grep -saIv ^officecfg 2>/dev/null | wc -l) -eq 0 ];
                    then
                        echo "$gname group and $tmpl set in "$filename" appears only in officecfg";
                    else
                        if [ $(git grep -aI "$gname" officecfg 2>/dev/null | grep -saI oor:node-type 2>/dev/null | wc -l ) -eq 0 ] ;
                        then
                           echo "$gname group in "$filename" appears only in officecfg";
                        fi
                    fi
                done
                # If it's not used in a template and does not appears outside, report
                else
                    echo "$gname group/set/node-ref in "$filename" appears only in officecfg";
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
