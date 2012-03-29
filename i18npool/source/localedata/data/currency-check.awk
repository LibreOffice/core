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
# Usage: gawk -f currency-check.awk *.xml
# Check any
# <FormatCode>...[$xxx-...]...</FormatCode>
# against every
# <CurrencySymbol>xxx</CurrencySymbol>
# definition of the same XML file and output symbols if no match was found.
# For formatindex="12" to formatindex="15" and for formatindex="17" it is
# checked if the used currency symbol is the usedInCompatibleFormatCodes
# currency symbol as it is needed by the number formatter.
# Also generates output if the generic currency symbol (UTF8 string 0xC2A4)
# is used instead of a real currency symbol.
# Author: Eike Rathke <er@openoffice.org>

BEGIN {
    file = ""
}


file != FILENAME {
    if ( file )
        checkIt()
    file = FILENAME
    line = 0
    nFormats = 0
    nCurrencies = 0
    bFormatAuto = 0
    sReplaceFrom = ""
    sReplaceTo = ""
    sMatchReplace = ""
    sRefCurrencyFromLocale = ""
    crlf = 0
}

{
    ++line
    # If run under Unix a CrLf spoils ...$ line end checks. DOS line endings
    # are boo anyways.
    if ( /\x0D$/ )
    {
        print "Error: not Unix line ending in line " line
        crlf = 1
        exit(1)
    }
    if ( $1 ~ /^<LC_FORMAT(>|$)/ )
    {
        if ( $0 ~ /replaceFrom="\[CURRENCY\]"/ )
        {
            sReplaceFrom = "\\[CURRENCY\\]"
            sMatchReplace = "^<FormatCode>.*" sReplaceFrom
        }
        for ( j=2; j<=NF; ++j )
        {
            if ( $j ~ /^replaceTo="/ )
            {
                l = 12
                if ( $j ~ />$/ )
                    ++l
                if ( $j ~ /\/>$/ )
                    ++l
                sReplaceTo = substr( $j, 12, length($j)-l )
            }
        }
    }
    else if ( $1 ~ /^<FormatElement(>|$)/ )
    {
        if ( $0 ~ /usage="CURRENCY"/ )
        {
            if ( $0 ~ /formatindex="1[23457]"/ )
                bFormatAuto = 1
            else
                bFormatAuto = 0
        }
    }
    else if ( $0 ~ /^[[:blank:]]*<FormatCode>.*\[\$.*-[0-9a-fA-F]+\]/ ||
            (sMatchReplace && $0 ~ sMatchReplace ) )
    {
        if ( sReplaceFrom )
            gsub( sReplaceFrom, sReplaceTo )
        split( $0, arr, /<|>/ )
        split( arr[3], code, /(\[\$)|(-[0-9a-fA-F]+\])/ )
        for ( j in code )
        {
            if ( code[j] && code[j] !~ /\#|0|\[NatNum/ )
            {
                FormatLine[nFormats] = file " line " line
                FormatAuto[nFormats] = bFormatAuto
                Formats[nFormats++] = code[j]
            }
        }
        bFormatAuto = 0
    }
    else if ( $1 ~ /^<LC_CURRENCY(>|$)/ )
    {
        for ( j=2; j<=NF; ++j )
        {
            if ( $j ~ /^ref="/ )
            {
                l = 6
                if ( $j ~ />$/ )
                    ++l
                if ( $j ~ /\/>$/ )
                    ++l
                locale = substr( $j, 6, length($j)-l )
                sRefCurrencyFromLocale = file
                oldfile = file
                oldline = line
                file = locale ".xml"
                line = 0
                while ( (getline <file) > 0 )
                {
                    ++line
                    getCurrencyParams()
                }
                close( file )
                if ( !line )
                    print "ref locale not available: " file \
                        " (from " oldfile " line " oldline ")"
                file = oldfile
                line = oldline
                sRefCurrencyFromLocale = ""
            }
        }
    }
    else
        getCurrencyParams()
}


END {
    if ( file && !crlf )
        checkIt()
}


function getCurrencyParams() {
    # Assumes that each element is on a line on its own!
    if ( $1 ~ /^<Currency(>|$)/ )
    {
        if ( $0 ~ /default="true"/ )
            SymbolDefault[nCurrencies] = 1
        else
            SymbolDefault[nCurrencies] = 0
        if ( $0 ~ /usedInCompatibleFormatCodes="true"/ )
            SymbolCompati[nCurrencies] = 1
        else
            SymbolCompati[nCurrencies] = 0
    }
    else if ( $0 ~ /^[[:blank:]]*<CurrencyID>/ )
    {
        split( $0, arr, /<|>/ )
        if ( sRefCurrencyFromLocale )
            IDLine[nCurrencies] = file " line " line \
                " (referenced from " sRefCurrencyFromLocale ")"
        else
            IDLine[nCurrencies] = file " line " line
        IDs[nCurrencies] = arr[3]
    }
    else if ( $0 ~ /^[[:blank:]]*<CurrencySymbol>/ )
    {
        split( $0, arr, /<|>/ )
        if ( sRefCurrencyFromLocale )
            SymbolLine[nCurrencies] = file " line " line \
                " (referenced from " sRefCurrencyFromLocale ")"
        else
            SymbolLine[nCurrencies] = file " line " line
        Symbols[nCurrencies] = arr[3]
    }
    else if ( $0 ~ /^[[:blank:]]*<BankSymbol>/ )
    {
        split( $0, arr, /<|>/ )
        if ( sRefCurrencyFromLocale )
            BankSymbolLine[nCurrencies] = file " line " line \
                " (referenced from " sRefCurrencyFromLocale ")"
        else
            BankSymbolLine[nCurrencies] = file " line " line
        BankSymbols[nCurrencies] = arr[3]
    }
    else if ( $1 ~ /^<\/Currency>/ )
    {
        ++nCurrencies
    }
}


function checkIt() {
    bad = 0
    for ( j=0; j<nFormats; ++j )
    {
        state = FormatInSymbol( Formats[j] )
        if ( Formats[j] == "\xc2\xa4" )
        {
            bad = 1
            print "    bad: `" Formats[j] "'   (" FormatLine[j] ")"
        }
        else if ( state == 0 )
        {
            bad = 1
            print "unknown: `" Formats[j] "'   (" FormatLine[j] ")"
        }
        else if ( FormatAuto[j] && state < 2 )
        {
            bad = 1
            print "badauto: `" Formats[j] "'   (" FormatLine[j] ")"
        }
    }
    if ( bad )
    {
        for ( j=0; j<nCurrencies; ++j )
        {
            bDef = 0
            if ( Symbols[j] == "\xc2\xa4" )
                print "def bad: `" Symbols[j] "'   (" SymbolLine[j] ")"
            if ( SymbolDefault[j] )
            {
                bDef = 1
                print "default: `" Symbols[j] "'   (" SymbolLine[j] ")"
            }
            if ( SymbolCompati[j] )
            {
                bDef = 1
                print "compati: `" Symbols[j] "'   (" SymbolLine[j] ")"
            }
            if ( !bDef )
                print "defined: `" Symbols[j] "'   (" SymbolLine[j] ")"
        }
    }
    else
    {
        bHasDefault = 0
        bHasCompati = 0
        for ( j=0; j<nCurrencies; ++j )
        {
            if ( Symbols[j] == "\xc2\xa4" )
            {
                bad = 1
                print "def bad: `" Symbols[j] "'   (" SymbolLine[j] ")"
            }
            if ( SymbolDefault[j] )
            {
                if ( !bHasDefault )
                    bHasDefault = 1
                else
                {
                    bad = 1
                    print "dupe default: `" Symbols[j] "'   (" SymbolLine[j] ")"
                }
            }
            if ( SymbolCompati[j] )
            {
                if ( !bHasCompati )
                    bHasCompati = 1
                else
                {
                    bad = 1
                    print "dupe compati: `" Symbols[j] "'   (" SymbolLine[j] ")"
                }
            }
        }
        if ( !bHasDefault )
        {
            bad = 1
            print "  no default: (" file ")"
        }
        if ( !bHasCompati )
        {
            bad = 1
            print "  no compati: (" file ")"
        }
    }
    for ( j=0; j<nCurrencies; ++j )
    {
        # Check if CurrencyID at least resembles some ISO 4217 code.
        # The only exception is zh_MO that had an erroneous original data set
        # with BankSymbol="P" (stored as ISO code in documents, hence copied to
        # CurrencyID now) and needs that entry for legacy documents.
        # There is a strange bug in gawk 3.1.4 that does a match of [A-Z] on
        # lower case except 'a', regardless of IGNORECASE setting, hence this
        # ugly notation. [[:upper:]] wouldn't be correct since we want only
        # ASCII to match.
        if ( IDs[j] !~ /^[ABCDEFGHIJKLMNOPQRSTUVWXYZ][ABCDEFGHIJKLMNOPQRSTUVWXYZ][ABCDEFGHIJKLMNOPQRSTUVWXYZ]$/ \
              && !(file == "zh_MO.xml" && IDs[j] == "P") )
        {
            bad = 1
            print "no ISO 4217 code: `" IDs[j] "'   (" IDLine[j] ")"
        }
        # CurrencyID should equal BankSymbol for now.
        if ( IDs[j] != BankSymbols[j] )
        {
            bad = 1
            print "not equal: CurrencyID `" IDs[j] "' != BankSymbol `" BankSymbols[j] \
                  "'   (" IDLine[j] " and " BankSymbolLine[j] ")"
        }
    }
    if ( bad )
        print ""
}


function FormatInSymbol( format ) {
    state = 0
    for ( nSym=0; nSym<nCurrencies; ++nSym )
    {
        if ( format == Symbols[nSym] )
        {
            # Two currencies can have the same symbol (e.g. az_AZ.xml 'man.'
            # for AZM and AZN), continue to lookup if the match isn't the
            # compatible one.
            if ( SymbolCompati[nSym] )
                return 2
            else
                state = 1
        }
    }
    return state
}

# vim: ts=4 sw=4 expandtab
