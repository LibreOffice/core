#!/usr/bin/gawk -f
#// Usage: gawk -f list-locales.awk *.xml
#// Simply create a verbose list of known locales as stated in XML files.
#// Author: Eike Rathke <erack@sun.com>

BEGIN {
    lcinfo = 0
    inlang = 0
    incoun = 0
    file = ""
    language = ""
    country = ""
}

FILENAME != file {
    printEntry()
    file = FILENAME
}

{
    if ( !lcinfo )
    {
        if ( /<LC_INFO>/ )
            lcinfo = 1
        next
    }
    if ( /<\/LC_INFO>/ )
    {
        lcinfo = 0
        next
    }
    if ( /<Language>/ )
        inlang = 1
    if ( inlang && /<DefaultName>/ )
    {
        split( $0, x, /<|>/ )
        language = x[3]
    }
    if ( /<\/Language>/ )
        inlang = 0
    if ( /<Country>/ )
        incoun = 1
    if ( incoun && /<DefaultName>/ )
    {
        split( $0, x, /<|>/ )
        country = x[3]
    }
    if ( /<\/Country>/ )
        incoun = 0
}

END {
    printEntry()
}

function printEntry() {
    if ( file )
        printf( "%s: %s %s\n", file, language, country )
}
