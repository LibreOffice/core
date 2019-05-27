#!/bin/bash
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This script finds .ui files with incorrect translation domain set
# and prints the domain, the file name and the expected domain
# See also the discussion at https://gerrit.libreoffice.org/#/c/72973/

declare -A modules

# List of modules with .ui files and their expected translation domain
modules+=(  \
            [basctl]=basctl \
            [chart2]=chart \
            [cui]=cui \
            [dbaccess]=dba \
            [desktop]=dkt \
            [editeng]=editeng \
            [extensions]=pcr \
            [filter]=flt \
            [formula]="for" \
            [fpicker]=fps \
            [framework]=fwk \
            [reportdesign]=rpt \
            [sc]=sc \
            [sd]=sd \
            [sfx2]=sfx \
            [starmath]=sm \
            [svtools]=svt \
            [svx]=svx \
            [sw]=sw \
            [uui]=uui \
            [vcl]=vcl \
            [writerperfect]=wpt \
            [xmlsecurity]=xsc \
)

# Iterate the keys, i.e. modules with a uiconfig subdir
for key in ${!modules[@]}; do
    # Enumerate all .ui files in each module
    for uifile in $(git ls-files ${key}/uiconfig/*\.ui); do
        # Check that they contain the expected domain in double quotation marks, print the line if they don't
        grep "\<interface domain=" $uifile | grep -v "\"${modules[${key}]}\"";
        if [ "$?" -eq 0 ] ;
            # Report the file name and the expected domain
            then echo "^Problematic interface domain in file: $uifile ; should be: "${modules[${key}]}"";
        fi
    done
done
