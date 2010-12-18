/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <osl/endian.h>
#include <sot/exchange.hxx>
#include "global.hxx"
#include "address.hxx"

class ScDocShell;
class ScDocument;
class SvStream;
class SfxMedium;
class ScAsciiOptions;

struct ScExportTextOptions
{
    enum NewlineConversion { ToSystem, ToSpace, None };
    ScExportTextOptions( NewlineConversion eNewlineConversion = ToSystem, sal_Unicode cSeparatorConvertTo = 0, bool bAddQuotes = false ) :
        meNewlineConversion( eNewlineConversion ), mcSeparatorConvertTo( cSeparatorConvertTo ), mbAddQuotes( bAddQuotes ) {}

    NewlineConversion meNewlineConversion;
    sal_Unicode mcSeparatorConvertTo;   // Convert separator to this character
    bool mbAddQuotes;
};

class SC_DLLPUBLIC ScImportExport
{
    ScDocShell* pDocSh;
    ScDocument* pDoc;
    ScDocument* pUndoDoc;
    ScRange     aRange;
    String      aStreamPath;
    String      aNonConvertibleChars;
    ULONG       nSizeLimit;
    sal_Unicode cSep;                   // Separator
    sal_Unicode cStr;                   // String Delimiter
    BOOL        bFormulas;              // Formeln im Text?
    BOOL        bIncludeFiltered;       // include filtered rows? (default true)
    BOOL        bAll;                   // keine Selektion
    BOOL        bSingle;                // Einfachselektion
    BOOL        bUndo;                  // Mit Undo?
    BOOL        bOverflow;              // zuviele Zeilen/Spalten
    bool        mbApi;
    ScExportTextOptions mExportTextOptions;

    ScAsciiOptions* pExtOptions;        // erweiterte Optionen

    BOOL StartPaste();                  // Protect-Check, Undo einrichten
    void EndPaste();                    // Undo/Redo-Aktionen, Repaint
    BOOL Doc2Text( SvStream& );
    BOOL Text2Doc( SvStream& );
    BOOL Doc2Sylk( SvStream& );
    BOOL Sylk2Doc( SvStream& );
    BOOL Doc2HTML( SvStream&, const String& );
    BOOL Doc2RTF( SvStream& );
    BOOL Doc2Dif( SvStream& );
    BOOL Dif2Doc( SvStream& );
    BOOL ExtText2Doc( SvStream& );      // mit pExtOptions
    BOOL RTF2Doc( SvStream&, const String& rBaseURL );
    BOOL HTML2Doc( SvStream&, const String& rBaseURL );

public:
    ScImportExport( ScDocument* );                  // Gesamtdokument
    ScImportExport( ScDocument*, const String& );   // Bereichs/Zellangabe
    ScImportExport( ScDocument*, const ScAddress& );
    ScImportExport( ScDocument*, const ScRange& );
   ~ScImportExport();

    void SetExtOptions( const ScAsciiOptions& rOpt );

    BOOL IsDoubleRef() const { return BOOL( !( bAll || bSingle ) ); }
    BOOL IsSingleRef() const { return bSingle; }
    BOOL IsNoRef() const     { return bAll;    }
    BOOL IsRef() const       { return BOOL( !bAll ); }

    const ScRange& GetRange() const { return aRange; }

    BOOL IsUndo() const      { return bUndo; }
    void SetUndo( BOOL b )   { bUndo = b;    }

    static BOOL  IsFormatSupported( ULONG nFormat );
    static const sal_Unicode* ScanNextFieldFromString( const sal_Unicode* p,
            String& rField, sal_Unicode cStr, const sal_Unicode* pSeps, bool bMergeSeps, bool& rbIsQuoted );
    static  void    WriteUnicodeOrByteString( SvStream& rStrm, const String& rString, BOOL bZero = FALSE );
    static  void    WriteUnicodeOrByteEndl( SvStream& rStrm );
    static  inline  BOOL    IsEndianSwap( const SvStream& rStrm );

    //! only if stream is only used in own (!) memory
    static  inline  void    SetNoEndianSwap( SvStream& rStrm );

    sal_Unicode GetSeparator() const { return cSep; }
    void SetSeparator( sal_Unicode c ) { cSep = c; }
    sal_Unicode GetDelimiter() const { return cStr; }
    void SetDelimiter( sal_Unicode c ) { cStr = c; }
    BOOL IsFormulas() const { return bFormulas; }
    void SetFormulas( BOOL b ) { bFormulas = b; }
    BOOL IsIncludeFiltered() const { return bIncludeFiltered; }
    void SetIncludeFiltered( BOOL b ) { bIncludeFiltered = b; }

    void SetSizeLimit( ULONG nNew ) { nSizeLimit = nNew; }  // momentan nur fuer Ascii

    void            SetStreamPath( const String& rPath ) { aStreamPath = rPath; }
    const String&   GetStreamPath() const { return aStreamPath; }

    BOOL ImportString( const ::rtl::OUString&, ULONG=FORMAT_STRING );
    BOOL ExportString( ::rtl::OUString&, ULONG=FORMAT_STRING );
    BOOL ExportByteString( ByteString&, rtl_TextEncoding, ULONG=FORMAT_STRING );

    BOOL ImportStream( SvStream&, const String& rBaseURL, ULONG=FORMAT_STRING );
    BOOL ExportStream( SvStream&, const String& rBaseURL, ULONG=FORMAT_STRING );

    BOOL ImportData( const String& rMimeType,
                     const ::com::sun::star::uno::Any & rValue );
    BOOL ExportData( const String& rMimeType,
                     ::com::sun::star::uno::Any & rValue  );

    BOOL IsOverflow() const { return bOverflow; }       // nach dem Importieren

    const String& GetNonConvertibleChars() const { return aNonConvertibleChars; }

    bool IsApi() const { return mbApi; }
    void SetApi( bool bApi ) { mbApi = bApi; }
    const ScExportTextOptions& GetExportTextOptions() { return mExportTextOptions; }
    void SetExportTextOptions( const ScExportTextOptions& options ) { mExportTextOptions = options; }
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

// static
inline void ScImportExport::SetNoEndianSwap( SvStream& rStrm )
{
#ifdef OSL_BIGENDIAN
    rStrm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
#else
    rStrm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
#endif
}


// Helper class for importing clipboard strings as streams.
class ScImportStringStream : public SvMemoryStream
{
public:
    ScImportStringStream( const ::rtl::OUString rStr )
        : SvMemoryStream( (void*)rStr.getStr(),
                rStr.getLength() * sizeof(sal_Unicode), STREAM_READ)
    {
        SetStreamCharSet( RTL_TEXTENCODING_UNICODE );
        SetEndianSwap( FALSE );
    }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
