#!/usr/bin/gawk -f
# -*- Mode: awk; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Usage: gawk -f sort-formats-by-formatindex.awk ll_CC.xml
#
# Sort the LC_FORMAT child elements FormatElement and their children by
# formatindex="..." value for easier comparison between locales.
# Output goes to stdout.

BEGIN {
    file = ""
}

file != FILENAME {
    file = FILENAME
    informats = 0
    currformat = 0
    delete formats
}

/<LC_FORMAT[ >]/ {
    if (!/\/>/)
        informats = 1
    print
    next
}

informats && /<\/LC_FORMAT>/ {
    PROCINFO["sorted_in"] = "@ind_num_asc"
    for (f in formats)
    {
        if (isarray(formats[f]))
        {
            for (i in formats[f])
                print formats[f][i]
        }
        else
        {
            # Something unhandled, adapt code.
            print "XXX error: " formats[f]
        }
    }
    informats = 0
}

{
    if (!informats)
    {
        print
        next
    }
}

/<FormatElement / {
    split( $0, a, /formatindex="/)
    split( a[2], b, /"/)
    currformat = b[1]
    child = 0   # 1-based
    formats[currformat][++child] = $0
    next
}

/<DateAcceptancePattern[ >]/ {
    print
    next
}

# Associate any element or comment with the current FormatElement.
{
    formats[currformat][++child] = $0
}

END {
}

# vim:set shiftwidth=4 softtabstop=4 expandtab:
