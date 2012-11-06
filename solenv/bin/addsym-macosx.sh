#!/bin/sh
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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

# The fields in the nm -gx output are apparently (see
# /usr/include/mach-o/nlist.h>):

# xxxxxxxx xx xx xxxx xxxxxxxx symbol
# !        !  !  !    n_value
# !        !  !  n_desc
# !        !  n_sect
# !        n_type
# n_strx

# Original comment:
# On Panther we have to filter out symbols with a value "1f" otherwise external
# symbols will erroneously be added to the generated export symbols list file.
#
# Of course it isn't actually the "value" (n_value) of the symbol that
# is meant, but (as is seen from the use of $2) the n_type .
#
# Now, what does a n_type of 1f actually mean? The N_PEXT bit (0x10)
# is on and the N_EXT (0x01) bit is on. It is what in Mach-O
# documentation is called "private external". This includes symbols
# produced by using -fvisibility=hidden. Whether that is a problem I
# don't know.
#
awk -v SYMBOLSREGEXP="`cat $2`" '
match ($6,SYMBOLSREGEXP) > 0 &&  $6 !~ /_GLOBAL_/ { if (($2 != 1) && ( $2 != "1f" ) ) print $6 }'

