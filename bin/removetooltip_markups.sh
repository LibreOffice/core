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

SED_BIN=`which sed`
CUT_BIN=`which cut`
LOG_FILE="modified-$(date +%s).log"

removeTooltipMarkup()
{
    LINE=$(grep -n "<property name=\"tooltip_markup\"" $1 | $CUT_BIN -f 1 -d ':')
    TEXT=$(grep "<property name=\"tooltip_markup\"" $1)
    grep -v "<property name=\"tooltip_markup\"" $1 > temp && mv temp $1
    echo "removed $TEXT from $1 at line $LINE" >> $LOG_FILE
}

changeTooltipMarkup()
{
   LINE=$(grep -n "<property name=\"tooltip_markup\"" $1 | $CUT_BIN -f 1 -d ':')
   $SED_BIN "s/tooltip_markup/tooltip_text/g" $i > temp && mv temp $1
   echo "renamed tooltip_markup from $1 at line $LINE" >> $LOG_FILE
}

checkTooltipMarkup()
{
   TEXT=`grep "<property name=\"tooltip_text\"" $1`
   MARKUP=`grep "<property name=\"tooltip_markup\"" $1`

   if [[ $MARKUP ]] && [[ $TEXT ]]
   then
      removeTooltipMarkup "$1"
   fi
   if [[ $MARKUP ]] && [[ ! $TEXT ]]
   then
      changeTooltipMarkup "$1"
   fi
}

shopt -s globstar
echo " " > $LOG_FILE
for i in **/*.ui; do
    echo -n "."
    checkTooltipMarkup "$i"
done

echo
echo "Done!"
