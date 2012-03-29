#!/usr/bin/gawk -f
# *************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
# *************************************************************
#// Usage: gawk -f list-locales.awk *.xml
#// Simply create a verbose list of known locales as stated in XML files.
#// Author: Eike Rathke <erack@sun.com>

BEGIN {
    file = ""
    count = 0
}

function init_locale() {
    lcinfo = 0
    inlang = 0
    incoun = 0
    language = ""
    country = ""
}

FILENAME != file {
    printEntry()
    file = FILENAME
    ++count
    init_locale()
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
    print "\n" count " locales"
}

function printEntry() {
    if ( file )
    {
        tmp = file
        gsub( /.*\//, "", tmp )
        gsub( /\.xml/, "", tmp )
        split( tmp, iso, /_/ )
        if ( iso[2] )
            printf( "%3s_%2s: %s - %s\n", iso[1], iso[2], language, country )
        else
            printf( "%3s %2s: %s   %s\n", iso[1], iso[2], language, country )
    }
}
