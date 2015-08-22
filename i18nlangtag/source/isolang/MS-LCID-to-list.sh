#!/usr/bin/env bash
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Generates language ID table and defines and mappings of
# http://download.microsoft.com/download/9/5/E/95EF66AF-9026-4BB0-A41D-A4F81802D92C/%5BMS-LCID%5D.pdf
# downloaded from http://msdn.microsoft.com/library/cc233965.aspx
# At least this worked for Release: Monday, July 22, 2013; 08/08/2013 Revision 6.0
# Also worked for 6/30/2015 revision 7.0
#
# Uses pdftotext (from poppler-utils), grep and gawk.
#
# The script expects the downloaded [MS-LCID].pdf as MS-LCID.pdf
#
# Files created/OVERWRITTEN: MS-LCID.txt, MS-LCID.lst, MS-LCID.lst.h
#
# Best invoked in a temporary directory ...
# Layout may change, diff MS-LCID.lst with ignore spaces against the previous
# version for changes and additions, e.g.
# gvimdiff -c 'set diffopt+=iwhite' ../MS-LCID.lst MS-LCID.lst
# The generated MS-LCID.lst.h file is only a copy&paste help to add entries in
# isolang.cxx and not to be committed, the #define names have to be adapted for
# lang.h and isolang.cxx

pdftotext -layout MS-LCID.pdf
grep '^ *0x[0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F] ' MS-LCID.txt > MS-LCID.lst
gawk -e '
{
    val = "0x" toupper( substr( $1, 3));
    tag = $2;
    tag = gensub( /,.*/, "", 1, tag);
    def = $2;
    for (i=3; i<=NF; ++i)
    {
        def = def "_" $i;
    }
    def = gensub( /[^a-zA-Z0-9_]/, "_", "g", def);
    def = "LANGUAGE_" def
    if (def == "LANGUAGE_Neither_defined_nor_reserved")
    {
        def = def "_" val
    }
    usedef = def ","
    n = split( tag, arr, /-/);
    switch (n)
    {
        case 1:
            # lll
            mapping = sprintf( "    { %-36s %5s, \"\"  , 0     },", usedef, "\"" arr[1] "\"");
            break;
        case 2:
            if (length(arr[2]) == 2)
            {
                # lll-CC
                mapping = sprintf( "    { %-36s %5s, \"%s\", 0     },", usedef, "\"" arr[1] "\"", arr[2]);
            }
            else if (length(arr[2]) == 4)
            {
                # lll-Ssss
                mapping = sprintf( "    { %-44s %10s, \"\"  , 0     },", usedef, "\"" tag "\"");
            }
            else
            {
                # lll-### or lll-vvvvvvvv
                mapping = sprintf( "    { %-33s %16s,   \"\", \"\" },", usedef, "\"" tag "\"");
            }
            break;
        default:
            if (length(arr[2]) == 2)
            {
                # lll-CC-vvvvvvvv
                mapping = sprintf( "    { %-33s %16s, \"%s\", \"%s\" },", usedef, "\"" tag "\"", arr[2], arr[1] "-" arr[3]);
            }
            else if (length(arr[2]) == 4)
            {
                # lll-Ssss-CC
                mapping = sprintf( "    { %-44s %10s, \"%s\", 0     },", usedef, "\"" arr[1] "-" arr[2] "\"", arr[3]);
            }
            else
            {
                # grandfathered or stuff
                if (length(arr[3] == 2))
                    mapping = sprintf( "    { %-33s %16s, \"%s\", \"\" },", usedef, "\"" tag "\"", arr[3]);
                else
                    mapping = sprintf( "    { %-33s %16s, \"\", \"\" },", usedef, "\"" tag "\"");
            }
            break;
    }
    printf "#define %-35s %s\n", def, val;
    print mapping;
    print ""
}
' MS-LCID.lst > MS-LCID.lst.h

# vim: set noet sw=4 ts=4:
