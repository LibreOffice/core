= Generating Custom Shape Presets

The Perl code (`generatePresetsData.pl` and `generatePresetsPPTXs.pl`) is used
to generate the custom shape presets.

The `generate.sh` script which invokes the above Perl files, should be invoked
from the libreoffice core top-level source folder:
    ./oox/source/drawingml/customshapes/generate.sh
The logs will be created in the file:
    ./oox/source/drawingml/customshapes/custom-shapes.log
    
The output would be placed in this folder:
    oox/source/drawingml/customshapes/pptx/

