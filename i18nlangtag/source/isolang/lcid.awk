#!/usr/bin/awk -f
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
# Utility to compare MS-LANGID definitions with those defined in ../../inc/i18nlangtag/lang.h
# Run in i18nlangtag/source/isolang
#
# outputs new #define LANGUAGE_... 0x... and also some commented out substrings
# that were matched in already existing defines.
#
# ATTENTION! The sed filter in the command line examples below assures that a
# '|' border is drawn by html2text in data tables, and nowhere else, on which
# this awk script relies. This script also heavily relies on the column layout
# encountered. Should MS decide to change their layout or their CSS names
# ("data..."), this would probably break. Should html2text decide that the last
# border="..." attribute encountered wins instead of the first, this may break
# also.
#
# sed -e 's/|/,/g; s/<TABLE/<table/g; /<table/\!b; s/\(<table[^>]*\)\(border\|BORDER\)="[0-9]*"/\1/g; s/\(<table\)\([^>]*\(class\|CLASS\)="data\)/\1 border="1"\2/g'
#
# After html2text best if file cleaned up to _only_ contain the table entries,
# but not necessary, entries are filtered. Check output.
#
# Expects input from the saved page of one of
#
# (1)
# http://www.microsoft.com/globaldev/reference/lcid-all.mspx
# filtered through ``html2text -nobs ...'', generated table:
# blank,name,hex,dec,blank fields:
#    |Afrikaans_-_South_Africa___|0436___|1078___|
#
# complete command line:
# lynx -dump -source http://www.microsoft.com/globaldev/reference/lcid-all.mspx | sed -e 's/|/,/g; s/<TABLE/<table/g; /<table/\!b; s/\(<table[^>]*\)\(border\|BORDER\)="[0-9]*"/\1/g; s/\(<table\)\([^>]*\(class\|CLASS\)="data\)/\1 border="1"\2/g' | html2text -nobs -width 234 | awk -f lcid.awk >outfile
#
#
# (2)
# http://www.microsoft.com/globaldev/reference/winxp/xp-lcid.mspx
# filtered through ``html2text -nobs ...'', generated table:
# blank,name,hex,dec,inputlocales,collection,blank fields:
#    |Afrikaans   |0436   |1078   |0436:00000409,   |Basic   |
#
# complete command line:
# lynx -dump -source http://www.microsoft.com/globaldev/reference/winxp/xp-lcid.mspx | sed -e 's/|/,/g; s/<TABLE/<table/g; /<table/\!b; s/\(<table[^>]*\)\(border\|BORDER\)="[0-9]*"/\1/g; s/\(<table\)\([^>]*\(class\|CLASS\)="data\)/\1 border="1"\2/g' | html2text -nobs -width 234 | awk -f lcid.awk >outfile
#
#
# (3)
# http://msdn.microsoft.com/library/en-us/intl/nls_238z.asp
# filtered through ``html2text -nobs ...'', generated table:
# blank,hex,locale,name,blank  fields:
#   |0x0436___|af-ZA___|Afrikaans_(South_Africa)___|
#
# complete command line:
# lynx -dump -source http://msdn.microsoft.com/library/en-us/intl/nls_238z.asp | sed -e 's/|/,/g; s/<TABLE/<table/g; /<table/\!b; s/\(<table[^>]*\)\(border\|BORDER\)="[0-9]*"/\1/g; s/\(<table\)\([^>]*\(class\|CLASS\)="data\)/\1 border="1"\2/g' | html2text -nobs -width 234 | awk -f lcid.awk >outfile
#
# Author: Eike Rathke <erack@sun.com>, <er@openoffice.org>
#

BEGIN {
    while ((getline < "../../inc/i18nlangtag/lang.h") > 0)
    {
        if ($0 ~ /^#define[ ]*LANGUAGE_[_A-Za-z0-9]*[ ]*0x[0-9a-fA-F]/)
        {
            # lang[HEX]=NAME 
            lang[toupper(substr($3,3))] = toupper($2)
            #print substr($3,3) "=" $2
        }
    }
    # html2text table follows
    FS = "\|"
    filetype = 0
    lcid_all = 1
    xp_lcid  = 2
    nls_238z = 3
    filetypename[filetype] = "unknown"
    filetypename[lcid_all] = "lcid_all"
    filetypename[xp_lcid]  = "xp_lcid"
    filetypename[nls_238z] = "nls_238z"
    namefield[lcid_all] = 2
    namefield[xp_lcid]  = 2
    namefield[nls_238z] = 4
    hexfield[lcid_all]  = 3
    hexfield[xp_lcid]   = 3
    hexfield[nls_238z]  = 2
    locfield[lcid_all]  = 0
    locfield[xp_lcid]   = 0
    locfield[nls_238z]  = 3
}

(NF < 5) { next }

!filetype {
    if (NF == 5)
    {
        if ($2 ~ /^0x/)
            filetype = nls_238z
        else if ($2 ~ /^Afrikaans/)
            filetype = lcid_all
    }
    else if (NF == 7)
        filetype = xp_lcid
    if (!filetype)
        next
    name = namefield[filetype]
    hex = hexfield[filetype]
    loc = locfield[filetype]
}

{
    gsub( /^[^:]*:/, "", $name)
    gsub( /\..*/, "", $name)
    gsub( /(^[ _]+)|([ _]+$)/, "", $hex)
    gsub( /(^[ _]+)|([ _]+$)/, "", $name)
    if (loc)
        gsub( /(^[ _]+)|([ _]+$)/, "", $loc)
}

($hex ~ /^0x/) { $hex = substr( $hex, 3) }

# if only 464 instead of 0464, make it match lang.h
(length($hex) < 4) { $hex = "0" $hex }

($hex !~ /^[0-9a-fA-F][0-9a-fA-F]*$/) { filtered[$hex] = $0; next }

# all[HEX]=string
{ all[toupper($hex)] = $name }

(loc) { comment[toupper($hex)] = "  /* " $loc " */" }

# new hex: newlang[HEX]=string
!(toupper($hex) in lang) { newlang[toupper($hex)] = $name }

END {
    if (!filetype)
    {
        print "No file type recognized." >>"/dev/stderr"
        exit(1)
    }
    print "// assuming " filetypename[filetype] " file"
    # every new language
    for (x in newlang)
    {
        printf( "xxxxxxx LANGUAGE_%-26s 0x%s%s\n", newlang[x], x, comment[x])
        n = split(newlang[x],arr,/[^A-Za-z0-9]/)
        def = ""
        for (i=1; i<=n; ++i)
        {
            if (length(arr[i]))
            {
                # each identifier word of the language name
                if (def)
                    def = def "_"
                aup = toupper(arr[i])
                def = def aup
                for (l in lang)
                {
                    #  contained in already existing definitions?
                    if (lang[l] ~ aup)
                        printf( "// %-50s %s\n", arr[i] ": " lang[l], l)
                }
            }
        }
        printf( "#define LANGUAGE_%-26s 0x%s\n", def, x)
    }
    print "\n// --- reverse check follows ----------------------------------\n"
    for (x in lang)
    {
        if (!(x in all))
            print "// not in input file:   " x "  " lang[x]
    }
    print "\n// --- filtered table entries follow (if any) -----------------\n"
    for (x in filtered)
        print "// filtered:   " x "  " filtered[x]
}
