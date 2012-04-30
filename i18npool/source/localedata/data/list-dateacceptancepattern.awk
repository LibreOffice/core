#!/usr/bin/gawk -f
# -*- Mode: awk; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
#
# Copyright 2012 LibreOffice contributors.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Usage: gawk -f list-dateacceptancepattern.awk *.xml [--html]
# Outputs two lists of locales, one with DateAcceptancePattern elements
# defined, and one where none are defined.
# If --html is given as the last parameter, format output suitable for
# inclusion in HTML.

BEGIN {
    html = 0
    if (ARGV[ARGC-1] == "--html") {
        html = 1
        --ARGC
    }
    file = ""
    nopatterns = 0
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
    noFormatCode = 1
}

/<DateAcceptancePattern>/ {
    split( $0, a, /<|>/ )
    pattern[patterns++] = a[3]
}

# No FormatCode element means inherited LC_FORMAT ref=...
# hence pattern inherited as well.
/<FormatCode>/ {
    noFormatCode = 0
}


END {
    if (file)
        endFile()
    if (html)
    {
        print "</ul>"
        print "\n<p>"
    }
    else
        print "\n"
    print "Locales without explicit DateAcceptancePattern elements:"
    if (html)
        print "<br>"
    print "(one implicit full date pattern is always generated)"
    if (html)
        print "<p>"
    if (html)
    {
        for (i=0; i<nopatterns; ++i)
        {
            print NoPatternList[i] "&nbsp;&nbsp;&nbsp; "
        }
    }
    else
    {
        for (i=0; i<nopatterns; ++i)
        {
            print NoPatternList[i]
        }
    }
}


function endFile() {
    if (patterns)
    {
        if (html)
        {
            print "  <li> " getLocale( file) ":"
            print "  <ul>"
            for ( i=0; i<patterns; ++i )
            {
                print "    <li> " pattern[i]
            }
            print "  </ul>"
        }
        else
        {
            print getLocale( file) ":"
            for ( i=0; i<patterns; ++i )
            {
                print "    " pattern[i]
            }
        }
    }
    else if (!noFormatCode)
        NoPatternList[nopatterns++] = getLocale( file)
}


function getLocale( file, tmp ) {
    tmp = file
    gsub( /.*\//, "", tmp )
    gsub( /\.xml/, "", tmp )
    return tmp
}

# vim:set shiftwidth=4 softtabstop=4 expandtab:
