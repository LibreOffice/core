#!/usr/bin/gawk -f
#// Usage: gawk -f currency-check.awk *.xml
#// Check any
#// <FormatCode>...[$xxx-...]...</FormatCode>
#// against every
#// <CurrencySymbol>xxx</CurrencySymbol>
#// definition of the same XML file and output symbols if no match was found.
#// For formatindex="12" to formatindex="15" and for formatindex="17" it is
#// checked if the used currency symbol is the usedInCompatibleFormatCodes
#// currency symbol as it is needed by the number formatter.
#// Also generates output if the generic currency symbol (UTF8 string 0xC2A4)
#// is used instead of a real currency symbol.
#// Author: Eike Rathke <erack@sun.com>

BEGIN {
	file = ""
	while ( getline > 0 )
	{
		if ( file != FILENAME )
		{
			if ( file )
				checkIt()
			file = FILENAME
			line = 0
			nFormats = 0
			nSymbols = 0
			bSymbolDefault = 0
            bSymbolCompati = 0
			bFormatAuto = 0
		}
		++line
		if ( $1 ~ /^<FormatElement$/ )
		{
			if ( $5 ~ /^usage="CURRENCY"$/ )
			{
				if ( $6 ~ /^formatindex="1[23457]">$/ )
					bFormatAuto = 1
				else
					bFormatAuto = 0
			}
		}
		else if ( $0 ~ /^<FormatCode>.*\[\$.*-[0-9a-fA-F]+\]/ )
		{
			split( $0, arr, /<|>/ )
			split( arr[3], code, /(\[\$)|(-[0-9a-fA-F]+\])/ )
			for ( j in code )
			{
				if ( code[j] && code[j] !~ /\#|0/ )
				{
					FormatLine[nFormats] = file " line " line
					FormatAuto[nFormats] = bFormatAuto
					Formats[nFormats++] = code[j]
				}
			}
			bFormatAuto = 0
		}
		else if ( $1 ~ /^<Currency$/ )
		{
            if ( $2 ~ /^default="true"/ )
				bSymbolDefault = 1
			else
				bSymbolDefault = 0
            if ( $3 ~ /^usedInCompatibleFormatCodes="true"/ )
                bSymbolCompati = 1
			else
                bSymbolCompati = 0
		}
		else if ( $0 ~ /^<CurrencySymbol>/ )
		{
			split( $0, arr, /<|>/ )
			SymbolLine[nSymbols] = file " line " line
			SymbolDefault[nSymbols] = bSymbolDefault
            SymbolCompati[nSymbols] = bSymbolCompati
			Symbols[nSymbols++] = arr[3]
			bSymbolDefault = 0
            bSymbolCompati = 0
		}
	}
	checkIt()
}


END {
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
		for ( j=0; j<nSymbols; ++j )
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
		for ( j=0; j<nSymbols; ++j )
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
	if ( bad )
		print ""
}


function FormatInSymbol( format ) {
	for ( nSym=0; nSym<nSymbols; ++nSym )
	{
		if ( format == Symbols[nSym] )
		{
            if ( SymbolCompati[nSym] )
				return 2
			else
				return 1
		}
	}
	return 0
}
