#!/bin/bash

module="$1"
mode="$2"
SRCDIR="$3"
BASE_OUTPUT="$4"


module_include=""
if [ -d "$module/inc" ]; then
    module_include="$module/inc"
fi

if [ -d "include/$module" ]; then
    module_include="$module_include include/$module"
fi

DOXYGEN_INPUT=$(printf "%s" "$module/source $module_include")
DOXYGEN_OUTPUT="$BASE_OUTPUT/$module"
DOXYGEN_VERSION="master"
DOXYGEN_PROJECTNAME="LibreOffice Module $module"
if [ "$mode" = "tag" ] ; then
    DOXYGEN_OUR_TAGFILE="$DOXYGEN_OUTPUT/$module.tags"
    DOXYGEN_REF_TAGFILES=
else
    DOXYGEN_OUR_TAGFILE="$DOXYGEN_OUTPUT/$module.tags2"
    DOXYGEN_REF_TAGFILES="${DOXYGEN_REF_TAGFILES//s@DOXYGEN_OUTPUT/module.tags@@})"
fi

# export variables referenced in doxygen config file
export DOXYGEN_INPUT
export DOXYGEN_OUTPUT
export DOXYGEN_INCLUDE_PATH
export DOXYGEN_VERSION
export DOXYGEN_OUR_TAGFILE
export DOXYGEN_REF_TAGFILES
export DOXYGEN_PROJECTNAME


echo "Calling doxygen mode $mode for $module"
echo "using tagfiles : $DOXYGEN_REF_TAGFILES" >> "$BASE_OUTPUT"/"$module".log 2>&1
doxygen "$SRCDIR/solenv/inc/doxygen_$mode.cfg" >> "$BASE_OUTPUT"/"$module".log 2>&1
echo "$(date "+%F %T") doxygen $module.$mode rc:$?"


