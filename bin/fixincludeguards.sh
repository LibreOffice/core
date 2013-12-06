#!/bin/bash
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# corrects include guards for hxx/h files automatically by its path.

# Usage:
# a) fixincludeguards.sh header.hxx
# b) find . -name *.hxx -or -name *.h | xargs sh fixincludeguards.sh

# TODO: This doesn't fix wrong #endif comments, like:
# #ifndef FOO_BAR_HXX
# #define FOO_BAR_HXX
# ...
# #endif // OTHER_BAR_HXX

# TODO: Make this portable. As it is now, it likely only works on Linux, or
# other platforms with a purely GNU toolset.

guard_prefix="INCLUDED_"

for fn in "$@"; do
    # remove leading ./, if invoked with find
    fn=`echo "$fn" | sed 's/^.\///g'`

    # global header in include/ top level dir:
    # drop the project dir
    fnfixed=`echo $fn | sed 's/include\///g'`

    # convert file path to header guard
    guard=`echo "$fnfixed" | sed 's/[\/\.-]/_/g' | tr 'a-z' 'A-Z'`

    if [ aa"`git grep -h "^\s*#ifndef ${guard_prefix}$guard" "$fn" | wc -l`" != "aa1" ] ||
       [ aa"`git grep -h "^\s*#define ${guard_prefix}$guard" "$fn" | wc -l`" != "aa1" ]; then

        # pattern which identifies guards, common one look like
        # _MODULE_FILE_HXX, FILE_H, FILE_INC
        pattern=".*\(_HXX\|_H\|_INC\)"

        ### extract guard definition
        # head to take only the first match
        old_guard=`git grep -h "#ifndef $pattern" "$fn" | head -n1 | sed "s/.*\s\($pattern.*\)/\1/"`

        if [ aa"$old_guard" == aa"" ]; then
            echo -e "$fn: \e[00;31mwarning:\e[00m guard not detectable"
            continue
        fi


        if [ aa"`git grep -w "$old_guard" | cut -d ':' -f1 | sort -u | wc -l `" != aa"1" ]; then
            echo -e "$fn: \e[00;31mwarning:\e[00m $old_guard guard definition used in other files"
            continue
        fi

        ### skip some special files...

        # skip this comphelper stuff:
        # INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_14
        if [ aa"INCLUDED_COMPHELPER_IMPLBASE_" == aa"`echo $old_guard | sed "s/VAR_HXX_[0-9]\+//g"`" ]; then
            continue
        fi

        # skip files like xmloff/source/forms/elementimport_impl.hxx
        if [ aa"`git grep -h "#error.*directly" "$fn" | wc -l`" != "aa0" ]; then
            continue
        fi


        ### replace old guard with new scheme guard
        echo "$fn: $old_guard"

        # includes leading whitespace removal
        sed -i "s/\s*${old_guard}/ ${guard_prefix}${guard}/g" "$fn"


        ### clean up endif
        sed -i "s/#endif\s*\(\/\/\|\/\*\)\s*\#\?\(ifndef\)\?\s*\(${guard_prefix}${guard}\).*/#endif \/\/ \3/g" "$fn"

    fi
done
