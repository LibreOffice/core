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
# Usage: gawk -f linkermapfile-check.awk *.map *.xml
# Order of *.map *.xml is important, otherwise all symbols are reported to be
# missing.
# Checks if all symbols of all locale data are present in the symbol scoping
# linker mapfiles.  Any output indicates a missing symbol, ../localedata.cxx is
# grep'ed to indicate the library to which mapfile the symbol should be added.
# Author: Eike Rathke <er@openoffice.org>

BEGIN {
    bAnyMissing = 0
    file = ""
    nMap = 0
    nMaps = 0
    nPublics = 0
    sPublic[nPublics++] = "getAllCalendars_"
    sPublic[nPublics++] = "getAllCurrencies_"
    sPublic[nPublics++] = "getAllFormats0_"
    bOptional[nPublics] = 1     # getAllFormats1 most times not present
    sPublic[nPublics++] = "getAllFormats1_"
    sPublic[nPublics++] = "getBreakIteratorRules_"
    sPublic[nPublics++] = "getCollationOptions_"
    sPublic[nPublics++] = "getCollatorImplementation_"
    sPublic[nPublics++] = "getContinuousNumberingLevels_"
    sPublic[nPublics++] = "getForbiddenCharacters_"
    sPublic[nPublics++] = "getLCInfo_"
    sPublic[nPublics++] = "getLocaleItem_"
    sPublic[nPublics++] = "getOutlineNumberingLevels_"
    sPublic[nPublics++] = "getReservedWords_"
    sPublic[nPublics++] = "getSearchOptions_"
    sPublic[nPublics++] = "getTransliterations_"
    sPublic[nPublics++] = "getIndexAlgorithm_"
    sPublic[nPublics++] = "getUnicodeScripts_"
    sPublic[nPublics++] = "getFollowPageWords_"
}

file != FILENAME {
    file = FILENAME
    if ( file ~ /\.map$/ )
    {
        sMapFile[nMaps] = file
        nMap = nMaps
        ++nMaps
    }
    else if ( file ~ /\.xml$/ )
    {
        bOut = 0
        n = split( file, arr, /[:\\\/.]/ )
        locale = arr[n-1]
        for ( i=0; i<nPublics; ++i )
        {
            symbol = sPublic[i] locale ";"
            bFound = 0
            for ( j=0; j<nMaps && !bFound; ++j )
            {
                if ( sSymbol[j,symbol] )
                    bFound = 1
            }
            if ( !bFound && bOptional[i] )
            {
                print symbol " not present but optional"
                bFound = 1
            }
            if ( !bFound )
            {
                if ( !bOut )
                {
                    search = "\"" locale "\""
                    while ( !bOut && (getline <"../localedata.cxx") > 0 )
                    {
                        if ( $0 ~ search )
                        {
                            bOut = 1
                            print "../localedata.cxx says this should go into: " $0
                        }
                    }
                    close( "../localedata.cxx" )
                    if ( !bOut )
                        print "../localedata.cxx doesn't indicate to which lib this belongs to:"
                    bOut = 1
                }
                print symbol
            }
        }
        if ( bOut)
        {
            printf("\n")
            bAnyMissing = 1
        }
        nextfile
    }
    else
        nextfile
}

# only reached if .map file encountered, read in symbols
{
    if ( $1 ~ /;$/ )
        sSymbol[nMap,$1] = 1
}

END {
    if ( !bAnyMissing )
        print "All good." >>"/dev/stderr"
}
