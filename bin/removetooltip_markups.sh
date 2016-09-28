#!/usr/bin/env bash
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Run the script in the core directory to remove all tooltip_markup
# properties from the .ui files

removeTooltipMarkup()
{
    grep -v "<property name=\"tooltip_markup\"" $1 > temp && mv temp $1
}

shopt -s globstar
for i in **/*.ui; do
    echo -n "."
    removeTooltipMarkup "$i"
done

echo
echo "Done!"
