#!/usr/bin/gawk -f
# -*- Mode: awk; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
#
# Copyright 2012 LibreOffice contributors.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Usage: gawk -f list-dateacceptancepattern.awk *.xml [--html]
#
# Outputs three lists of locales, one with DateAcceptancePattern elements
# defined, one with inherited LC_FORMAT elements and thus date patterns, and
# one where no DateAcceptancePattern are defined.
#
# If --html is given as the last parameter, format output suitable for
# inclusion in HTML.

BEGIN {
    html = 0
    if (ARGV[ARGC-1] == "--html") {
        html = 1
        --ARGC
    }
    file = ""
    offlocale = 0
    offpatterns = 1
    offinherit = 2
    inheritedcount = 0
    if (html)
        print "<p>"
    else
        print ""
    print "Locales with explicit DateAcceptancePattern elements:"
    if (html)
        print "<ul>"
}


file != FILENAME {
    if (file)
        endFile()
    file = FILENAME
    patterns = 0
    inherited = ""
}

/<DateAcceptancePattern>/ {
    split( $0, a, /<|>/ )
    pattern[patterns++] = a[3]
}

# pattern inherited as well
/<LC_FORMAT[^>]* ref="[^>"]+"[^>]*>/ {
    split( $0, a, /.* ref="|"/ )
    inherited = a[2]
}


END {
    if (file)
        endFile()

    PROCINFO["sorted_in"] = "@ind_str_asc"

    fillAllInherited()

    if (html)
    {
        print "</ul>"
        print "\n<p>"
    }
    else
        print "\n"

    print "Locales inheriting patterns:"
    if (html)
    {
        print "<br>"
        for (i in LocaleList)
        {
            if (LocaleList[i][offinherit] && LocaleList[i][offpatterns])
                print LocaleList[i][offlocale] " = " LocaleList[i][offinherit] "&nbsp;&nbsp;&nbsp; "
        }
        print "\n<p>"
    }
    else
    {
        for (i in LocaleList)
        {
            if (LocaleList[i][offinherit] && LocaleList[i][offpatterns])
                print LocaleList[i][offlocale] " = " LocaleList[i][offinherit]
        }
        print "\n"
    }

    print "Locales without explicit DateAcceptancePattern elements:"
    if (html)
        print "<br>"
    print "(one implicit full date pattern is always generated)"
    if (html)
        print "<p>"
    if (html)
    {
        for (i in LocaleList)
        {
            if (!LocaleList[i][offpatterns])
                print LocaleList[i][offlocale] "&nbsp;&nbsp;&nbsp; "
        }
    }
    else
    {
        for (i in LocaleList)
        {
            if (!LocaleList[i][offpatterns])
                print LocaleList[i][offlocale]
        }
    }
}


function endFile() {
    locale =  getLocale( file)
    LocaleList[locale][offlocale] = locale
    LocaleList[locale][offpatterns] = patterns
    if (patterns)
    {
        if (html)
        {
            print "  <li> " locale ":"
            print "  <ul>"
            for ( i=0; i<patterns; ++i )
            {
                print "    <li> " pattern[i]
            }
            print "  </ul>"
        }
        else
        {
            print locale ":"
            for ( i=0; i<patterns; ++i )
            {
                print "    " pattern[i]
            }
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
    if (!LocaleList[locale][offpatterns] && LocaleList[locale][offinherit])
        LocaleList[locale][offpatterns] = fillInherited( LocaleList[locale][offinherit])
    return LocaleList[locale][offpatterns]
}


function fillAllInherited(      i ) {
    for (i in LocaleList)
    {
        if (!LocaleList[i][offpatterns] && LocaleList[i][offinherit])
            LocaleList[i][offpatterns] = fillInherited( LocaleList[i][offinherit])
    }
}

# vim:set shiftwidth=4 softtabstop=4 expandtab:
