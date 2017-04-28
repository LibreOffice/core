#!/usr/bin/gawk -f
# -*- Mode: awk; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Usage: gawk -f sort-formats-by-formatindex.awk [-v group=1] ll_CC.xml
#
# Sort the LC_FORMAT child elements FormatElement and their children by
# formatindex="..." value for easier comparison between locales.
# If -v group=1 is given, the output is sorted by usage groups first, then by
# formatindex. This could be the final sorting to commit.
# Output goes to stdout.

BEGIN {
    file = ""
    usage["FIXED_NUMBER"] = 1
    usage["SCIENTIFIC_NUMBER"] = 2
    usage["PERCENT_NUMBER"] = 3
    usage["CURRENCY"] = 4
    usage["DATE"] = 5
    usage["TIME"] = 6
    usage["DATE_TIME"] = 7
    group = (group ? 1 : 0)     # -v group=... given or not
}

file != FILENAME {
    file = FILENAME
    informats = 0
    currusage = 0
    currformat = 0
    inFormatElement = 0
    delete formats
    currleader = 0
    delete leaders
}

/<LC_FORMAT[ >]/ {
    if (!/\/>/)
        informats = 1
    print
    next
}

informats && /<\/LC_FORMAT>/ {
    PROCINFO["sorted_in"] = "@ind_num_asc"
    for (u in formats)
    {
        if (isarray(formats[u]))
        {
            for (f in formats[u])
            {
                if (isarray(formats[u][f]))
                {
                    for (i in formats[u][f])
                        print formats[u][f][i]
                }
                else
                {
                    # Something unhandled, adapt code.
                    print "XXX formats[u][f] error: " formats[u][f]
                }
            }
        }
        else
        {
            # Something unhandled, adapt code.
            print "XXX formats[u] error: " formats[u]
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
    if (group)
    {
        split( $0, a, / usage="/)
        split( a[2], b, /"/)
        currusage = usage[b[1]]
    }
    else
    {
        currusage = 0
    }
    split( $0, a, / formatindex="/)
    split( a[2], b, /"/)
    currformat = b[1]
    child = 0   # 1-based
    for (l in leaders)
        formats[currusage][currformat][++child] = leaders[l]
    delete leaders
    currleader = 0
    formats[currusage][currformat][++child] = $0
    inFormatElement = 1
    next
}

/<DateAcceptancePattern[ >]/ {
    print
    next
}

# Prefix a leading comment (or even an element) to the next FormatElement.
!inFormatElement {
    leaders[++currleader] = $0
    next
}

# Associate any element or comment with the current FormatElement.
{
    formats[currusage][currformat][++child] = $0
}

/<\/FormatElement>/ {
    inFormatElement = 0
}

END {
}

# vim:set shiftwidth=4 softtabstop=4 expandtab:
