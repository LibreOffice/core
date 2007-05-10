#!/bin/bash

# This script is needed in the process of generating exported
# symbols list out of map files on Mac OS X (see also #i69351#)
# The magic generating the regular expression from the temporary
# mapfile containing only star and question mark symbols
#
# The script has to be called as follows:
# nm -gx <file>.o | addsym-macosx.sh <file-with-wildcard-symbols> <temporary-file-where-to-write-the-search-expression-to>
# See tg_shl.mk for an example of how to use the script
#
# Replace every * with .* and every ? with . to get awk expression
# Put ^ at the beginning of every expression
# Put $ at the beginning of every expression
# Connect them all on one line, separated by |
# Remove | at the end of this regular expression because the last end
# of line was also replaced by |

cat $1 | sed 's#*#.*#g
s#?#.#g
s#^#^#
s#$#$#' | tr '\n' '|' | sed "s#|\$##" >$2

# Please note that the awk expression expects to get the output of 'nm -gx'!
# On Panther we have to filter out symbols with a value "1f" otherwise external
# symbols will erroneously be added to the generated export symbols list file.
awk -v SYMBOLSREGEXP="`cat $2`" '
match ($6,SYMBOLSREGEXP) > 0 &&  $6 !~ /_GLOBAL_/ { if (($2 != 1) && ( $2 != "1f" ) ) print $6 }'

