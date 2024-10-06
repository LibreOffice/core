#!/usr/bin/env bash
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Search for headers not included in any source files
# Note: there are still exceptions (such as ODK) so results are not completely foolproof

# Search in all subdirs, except for those not containing C/C++ headers
for subdir in $(ls -d */ | grep -v \
                                -e include/ `# Handled differently` \
                                -e android \
                                -e animations `# No headers here` \
                                -e bean \
                                -e bin/ `# Skip subdirs not containing C/C++ code ` \
                                -e cpputools/ \
                                -e distro-configs/ \
                                -e docmodel/ `# No headers here` \
                                -e eventattacher/ \
                                -e external/ `# FIXME Should be handled differently, but it\'s such a mess` \
                                -e extras/ \
                                -e i18nlangtag/ \
                                -e icon-themes/ \
                                -e idlc/ \
                                -e instsetoo_native/ \
                                -e jurt/ \
                                -e jvmaccess/ \
                                -e librelogo/ \
                                -e m4/ \
                                -e msicreator/ \
                                -e nlpsolver/ \
                                -e offapi/ \
                                -e officecfg/ \
                                -e oovbaapi/ \
                                -e osx/ \
                                -e pch/ \
                                -e postprocess/ \
                                -e qadevOOo/ \
                                -e readlicense_oo/ \
                                -e remotebridges/ \
                                -e reportbuilder/ \
                                -e ridljar/ \
                                -e schema/ \
                                -e scp2/ \
                                -e smoketest/ \
                                -e swext/ \
                                -e sysui/ \
                                -e udkapi/ \
                                -e uitest/ \
                                -e unoil/ \
                                -e unotest/ \
                                -e ure/ \
                                -e wizards/ \
                                -e xmlreader/ \
                                -e instdir/ `# Skip typical build-related temporaries` \
                                -e workdir/ \
                                -e autom4te.cache/ \
                                -e config_host/ \
                                -e dictionaries/ `# Skip typical submodules` \
                                -e helpcontent2/ \
                                -e translations/
                                ) ; do

    # Get a feeling of progress :)
    echo "Checking module: $subdir";

    # Find all .h / .hxx files and see if they are mentioned in the module
    # skip special directories: pch and precompiled_ (compilerplugins does not have separate pch dir), workben (playground code), test (dead code?)
    for i in  $(find "$subdir" -name "*\.h" -o -name "*\.hxx" -o -name "\.hrc" -o -name "*\.hlst" | grep -v -e "/pch/" -e "/precompiled_" -e "/workben/" -e "/test/" | xargs basename -a ); do
        # Search only in source files, and skip mentions in makefiles, .yaml, clang-format excludelist etc.
        if [ $(git grep -l "$i" "$subdir"/{*\.[hc]xx,*\.[hc],*\.hrc,*\.mm,*\.m,*\.py} | wc -l) -eq 0 ] ; then
            echo "Out of use header: $(find "$subdir" -name "$i")";
        fi
    done
done

echo "Checking global headers";
# Search for files in include is different since they can be used in any module
for i in  $(find include/ -name "*\.h" -o -name "*\.hxx" -o -name "\.hrc" | cut -d "/" -f 2- ); do
    # Some headers are only included between double quotes
    if [ $(git grep -l -e \<$i\> -e \"$i\" {*\.[hc]xx,*\.[hc],*\.hrc,*\.mm,*\.m} | grep -v pch | wc -l) -eq 0 ] ; then
        echo "Out of use header: include/$i";
    fi
done
