#!/bin/bash

# This script is needed in the process of generating exported
# symbols list out of map files on MinGW
# The magic generating the regular expression from the temporary
# mapfile containing only star and question mark symbols
#
# The script has to be called as follows:
# nm -gx <file>.o | addsym-mingw.sh <file-with-wildcard-symbols> <temporary-file-where-to-write-the-search-expression-to>
# See tg_shl.mk for an example of how to use the script
#
# Replace every * with .* and every ? with . to get awk expression
# Remove whitespaces and comments in expression
# Put ^ at the beginning of every expression
# Put $ at the beginning of every expression
# Connect them all on one line, separated by |
# Remove | at the end of this regular expression because the last end
# of line was also replaced by |

if [ -s $1 ]
then
cat $1 | sed 's#*#.*#g
s#?#.#g
s#;.*##g
s# ##g
s#  ##g
s#^#^#
s#$#$#' | tr '\n' '|' | sed "s#|\$##" >$2

# Please note that the awk expression expects to get the output of 'nm -gP'!
awk -v SYMBOLSREGEXP="`cat $2`" '
match (substr ($1,2) ,SYMBOLSREGEXP) > 0 { print substr ($1,2) ";" }'
fi

