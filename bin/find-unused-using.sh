# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#!/bin/bash

for ns in accessibility basegfx chart com css cppu comphelper connectivity formula dbtools editeng rtl sfx2 svt osl oox sax_fastparser sal sd ucbhelper utl vcl xmloff; do
    echo "Searching for namespace: $ns";
    # search in cxx files, excluding URE headers, plus some files with false positives
    for file in $(git grep -l "^\s*using :*$ns::" *hxx *cxx \
        ':!include/com/' \
        ':!include/cppu/' \
        ':!include/cppuhelper/' \
        ':!include/osl/' \
        ':!include/rtl/' \
        ':!include/sal/' \
        ':!include/salhelper/' \
        ':!include/systools/' \
        ':!include/typelib/' \
        ':!include/uno/' \
        ':!include/sfx2/stbitem.hxx' \
        ':!sw/source/uibase/inc/maildispatcher.hxx'
        ) ; do
        for class in $(git grep -h  "using :*$ns::" "$file" | rev | cut  -d : -f -1 | rev | cut -d " " -f 1 | tr -d ";") ; do
            if [ "$ns" == "com" ] ; then # com namespace may be mentioned in relevant header name too
                if [[ $(grep -c "$class" "$file") -eq 1 || $(grep -c "$class" "$file") -le 2 && $(grep -c -e "$class".hpp -e "$class".hxx -e "$class".h "$file") -eq 1 ]]; then
                    echo "$file";
                    echo "Class name in above file is mentioned once or twice: $class";
                fi
            else
                if [ $(grep -c -F "$class" "$file") -eq 1 ]; then
                    echo "$file";
                    echo "Class name in above file is mentioned once: $class";
                fi
            fi
        done
    done
done

# vim: set noet sw=4 ts=4:
