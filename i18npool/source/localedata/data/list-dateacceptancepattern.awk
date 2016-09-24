#!/usr/bin/gawk -f
# -*- Mode: awk; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Usage: gawk -f list-dateacceptancepattern.awk *.xml [--sep [--html]]
#
# Outputs three lists of locales, one with DateAcceptancePattern elements
# defined, one with inherited LC_FORMAT elements and thus date patterns, and
# one where no DateAcceptancePattern are defined.
#
# If --sep is given, display date separator for each locale.
# If --html is given as the last parameter, format output suitable for
# inclusion in HTML.

BEGIN {
    html = 0
    if (ARGV[ARGC-1] == "--html") {
        html = 1
        --ARGC
    }
    sep = 0
    if (ARGV[ARGC-1] == "--sep") {
        sep = 1
        --ARGC
    }
    file = ""
    offlocale = 0
    offpatterncount = 1
    offinherit = 2
    offbequeath = 3
    offdatesep = 4
    offdateformat = 5
    offpatterns = 6
}


file != FILENAME {
    if (file)
        endFile()
    file = FILENAME
    patterncount = 0
    inherited = ""
    formatelement = 0
    datesep = ""
    dateformat = ""
}

/<DateAcceptancePattern>/ {
    split( $0, a, /<|>/ )
    patterns[patterncount++] = a[3]
}

# pattern inherited as well
/<LC_FORMAT[^>]* ref="[^>"]+"[^>]*>/ {
    split( $0, a, /.* ref="|"/ )
    inherited = a[2]
}

/<FormatElement[^>]* formatindex="21"[^>]*>/ { formatelement = 1 }
/<FormatCode>/ {
    if (formatelement)
    {
        formatelement = 0
        split( $0, a, /<|>/ )
        split( a[3], b, /[0-9A-Za-z\[\~\]]+/ )
        datesep = b[2]
        dateformat = a[3]
    }
}


END {
    if (file)
        endFile()

    fillAllInherited()

    PROCINFO["sorted_in"] = "@ind_str_asc"

    if (html)
        print "<p>"
    else
        print ""
    printLine( "Trailing + indicates that another locale inherits from this." )
    if (sep)
        printLine( "Appended is the locale's date separator and edit format code." )
    printLine("")
    printLine( "Locales with explicit DateAcceptancePattern elements:" )
    if (html)
    {
        print "<ul>"
        for (i in LocaleList)
        {
            if (LocaleList[i][offpatterns][0])
            {
                print "  <li> " getInheritance( LocaleList[i][offlocale], LocaleList[i][offlocale])
                print "  <ul>"
                for (p = 0; p < LocaleList[i][offpatterncount]; ++p)
                {
                    print "    <li> " LocaleList[i][offpatterns][p]
                }
                print "  </ul>"
            }
        }
        print "</ul>"
        print "\n<p>"
    }
    else
    {
        for (i in LocaleList)
        {
            if (LocaleList[i][offpatterns][0])
            {
                print getInheritance( LocaleList[i][offlocale], LocaleList[i][offlocale])
                for (p = 0; p < LocaleList[i][offpatterncount]; ++p)
                {
                    print "    " LocaleList[i][offpatterns][p]
                }
            }
        }
        print "\n"
    }

    printLine( "Locales inheriting patterns:" )
    if (html)
    {
        for (i in LocaleList)
        {
            if (LocaleList[i][offinherit] && LocaleList[i][offpatterncount])
                print getInheritance( LocaleList[i][offlocale], LocaleList[i][offlocale]) "&nbsp;&nbsp;&nbsp; "
        }
        print "\n<p>"
    }
    else
    {
        for (i in LocaleList)
        {
            if (LocaleList[i][offinherit] && LocaleList[i][offpatterncount])
                print getInheritance( LocaleList[i][offlocale], LocaleList[i][offlocale])
        }
        print "\n"
    }

    printLine( "Locales without explicit DateAcceptancePattern elements:" )
    printLine( "(one implicit full date pattern is always generated)" )
    if (html)
    {
        print "<p>"
        for (i in LocaleList)
        {
            if (!LocaleList[i][offpatterncount])
                print getInheritance( LocaleList[i][offlocale], LocaleList[i][offlocale]) "&nbsp;&nbsp;&nbsp; "
        }
    }
    else
    {
        for (i in LocaleList)
        {
            if (!LocaleList[i][offpatterncount])
                print getInheritance( LocaleList[i][offlocale], LocaleList[i][offlocale])
        }
    }
}


function printLine( text ) {
    print text
    if (html)
        print "<br>"
}


function endFile(       locale ) {
    locale =  getLocale( file)
    LocaleList[locale][offlocale] = locale
    LocaleList[locale][offpatterncount] = patterncount
    LocaleList[locale][offdatesep] = datesep
    LocaleList[locale][offdateformat] = dateformat
    if (patterncount)
    {
        for ( i=0; i<patterncount; ++i )
        {
            LocaleList[locale][offpatterns][i] = patterns[i]
        }
    }
    else if (inherited)
        LocaleList[locale][offinherit] = inherited
}


function getLocale( file,       tmp ) {
    tmp = file
    gsub( /.*\//, "", tmp )
    gsub( /\.xml/, "", tmp )
    return tmp
}


function fillInherited( locale ) {
    LocaleList[locale][offbequeath] = 1
    if (!LocaleList[locale][offpatterncount] && LocaleList[locale][offinherit])
        LocaleList[locale][offpatterncount] = fillInherited( LocaleList[locale][offinherit])
    return LocaleList[locale][offpatterncount]
}


function fillAllInherited(      i ) {
    for (i in LocaleList)
    {
        LocaleList[i][offbequeath] = 0
    }
    for (i in LocaleList)
    {
        if (!LocaleList[i][offpatterncount] && LocaleList[i][offinherit])
            LocaleList[i][offpatterncount] = fillInherited( LocaleList[i][offinherit])
    }
}


function getInheritance( str, locale ) {
    if (LocaleList[locale][offbequeath])
        str = str " +"
    if (LocaleList[locale][offinherit])
        str = getInheritance( str " = " LocaleList[locale][offinherit], LocaleList[locale][offinherit])
    else if (sep)
        str = str "\t'" LocaleList[locale][offdatesep] "'  (" LocaleList[locale][offdateformat] ")"
    return str
}

# vim:set shiftwidth=4 softtabstop=4 expandtab:
