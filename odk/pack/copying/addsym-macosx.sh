#!/bin/bash

# This script is needed in the process of generating exported
# symbols list on Mac OS X

# Please note that the awk expression expects to get the output of 'nm -gx'!
# On Panther we have to filter out symbols with a value "1f" otherwise external
# symbols will erroneously be added to the generated export symbols list file.
awk -v SYMBOLSREGEXP="^__ZTI.*$|^__ZTS.*$" '
match ($6,SYMBOLSREGEXP) > 0 &&  $6 !~ /_GLOBAL_/ { if (($2 != 1) && ( $2 != "1f" ) ) print $6 }'

