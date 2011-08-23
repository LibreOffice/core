/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_IMPEX_HXX
#define SC_IMPEX_HXX

#ifndef _SOT_EXCHANGE_HXX //autogen
#include <sot/exchange.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef _OSL_ENDIAN_H_
#include <osl/endian.h>
#endif
class SvStream;
namespace binfilter {

class ScDocShell;
class ScDocument;
class SfxMedium;
class ScAsciiOptions;

class ScImportExport
{
    ScDocShell* pDocSh;
    ScDocument* pDoc;
    ScDocument* pUndoDoc;
    ScRange     aRange;
    String		aStreamPath;
    String      aNonConvertibleChars;
    ULONG		nSizeLimit;
    sal_Unicode	cSep;					// Separator
    sal_Unicode	cStr;					// String Delimiter
    BOOL		bFormulas;				// Formeln im Text?
    BOOL		bIncludeFiltered;		// include filtered rows? (default true)
    BOOL		bAll;					// keine Selektion
    BOOL		bSingle;				// Einfachselektion
    BOOL		bUndo;					// Mit Undo?
    BOOL		bOverflow;				// zuviele Zeilen/Spalten

    ScAsciiOptions*	pExtOptions;		// erweiterte Optionen

    BOOL ExtText2Doc( SvStream& );		// mit pExtOptions

    //! only if stream is only used in own (!) memory

public:
    ScImportExport( ScDocument* );					// Gesamtdokument
   ~ScImportExport();

    void SetExtOptions( const ScAsciiOptions& rOpt );

    BOOL IsDoubleRef() const { return BOOL( !( bAll || bSingle ) ); }
    BOOL IsSingleRef() const { return bSingle; }
    BOOL IsNoRef() const	 { return bAll;    }
    BOOL IsRef() const	 	 { return BOOL( !bAll ); }

    const ScRange& GetRange() const { return aRange; }

    BOOL IsUndo() const		 { return bUndo; }
    void SetUndo( BOOL b )	 { bUndo = b;	 }

    static const sal_Unicode* ScanNextFieldFromString( const sal_Unicode* p,
            String& rField, sal_Unicode cStr, const sal_Unicode* pSeps, BOOL bMergeSeps );
    static	inline	BOOL	IsEndianSwap( const SvStream& rStrm );

    sal_Unicode GetSeparator() const { return cSep; }
    void SetSeparator( sal_Unicode c ) { cSep = c; }
    sal_Unicode GetDelimiter() const { return cStr; }
    void SetDelimiter( sal_Unicode c ) { cStr = c; }
    BOOL IsFormulas() const { return bFormulas; }
    void SetFormulas( BOOL b ) { bFormulas = b; }
    BOOL IsIncludeFiltered() const { return bIncludeFiltered; }
    void SetIncludeFiltered( BOOL b ) { bIncludeFiltered = b; }

    void SetSizeLimit( ULONG nNew ) { nSizeLimit = nNew; }	// momentan nur fuer Ascii

    void			SetStreamPath( const String& rPath ) { aStreamPath = rPath; }
    const String&	GetStreamPath() const { return aStreamPath; }


    BOOL ImportStream( SvStream&, ULONG=FORMAT_STRING );


    BOOL IsOverflow() const	{ return bOverflow; }		// nach dem Importieren

    const String& GetNonConvertibleChars() const { return aNonConvertibleChars; }
};


// static
inline BOOL ScImportExport::IsEndianSwap( const SvStream& rStrm )
{
#ifdef OSL_BIGENDIAN
    return rStrm.GetNumberFormatInt() != NUMBERFORMAT_INT_BIGENDIAN;
#else
    return rStrm.GetNumberFormatInt() != NUMBERFORMAT_INT_LITTLEENDIAN;
#endif
}


} //namespace binfilter
#endif

