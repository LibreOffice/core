#!/usr/bin/env bash

# This script is used to generate the custom shape presets

# We want to exit on errors...

set -o errexit

# oox needs to be build with dbglevel=2 so that OSL_DEBUG_LEVEL=2 is defined.

make oox.clean && make oox dbglevel=2

# This reads the
# oox/source/drawingml/customshapes/presetShapeDefinitions.xml file.
# It will produce the file
# oox/source/drawingml/customshapes/pptx/cshape-all.ppx and a whole
# bunch of other files that aren't needed further.

(cd oox/source/drawingml/customshapes && ./generatePresetsPPTXs.pl)


# Then load it and store the debugging output.
# We need only the SAL_INFO output with tag "oox.csdata", plus stderr
# for PropertyMap::dumpData() output.

SAL_LOG='+INFO.oox.csdata-WARN' instdir/program/soffice --headless --convert-to odp --outdir oox/source/drawingml/customshapes/ oox/source/drawingml/customshapes/pptx/cshape-all.pptx > oox/source/drawingml/customshapes/custom-shapes.log 2>&1


# Now run a script that reads the above log file and generates the
# oox-drawingml-cs-presets data:

(cd oox/source/drawingml/customshapes && ./generatePresetsData.pl)


echo
echo "To see what has been done, run git diff --patience oox/source/drawingml"
echo

