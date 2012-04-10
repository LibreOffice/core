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

/**
 * These options control how multi-line cells are converted during export in
 * certain lossy formats (such as csv).
 */
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
    sal_uLong       nSizeLimit;
    sal_Unicode cSep;                   // Separator
    sal_Unicode cStr;                   // String Delimiter
    sal_Bool        bFormulas;              // Formeln im Text?
    sal_Bool        bIncludeFiltered;       // include filtered rows? (default true)
    sal_Bool        bAll;                   // keine Selektion
    sal_Bool        bSingle;                // Einfachselektion
    sal_Bool        bUndo;                  // Mit Undo?
    sal_Bool        bOverflow;              // zuviele Zeilen/Spalten
    bool        mbApi;
    ScExportTextOptions mExportTextOptions;

    ScAsciiOptions* pExtOptions;        // erweiterte Optionen

    sal_Bool StartPaste();                  // Protect-Check, Undo einrichten
    void EndPaste();                    // Undo/Redo-Aktionen, Repaint
    sal_Bool Doc2Text( SvStream& );
    sal_Bool Text2Doc( SvStream& );
    sal_Bool Doc2Sylk( SvStream& );
    sal_Bool Sylk2Doc( SvStream& );
    sal_Bool Doc2HTML( SvStream&, const String& );
    sal_Bool Doc2RTF( SvStream& );
    sal_Bool Doc2Dif( SvStream& );
    sal_Bool Dif2Doc( SvStream& );
    sal_Bool ExtText2Doc( SvStream& );      // mit pExtOptions
    sal_Bool RTF2Doc( SvStream&, const String& rBaseURL );
    sal_Bool HTML2Doc( SvStream&, const String& rBaseURL );

public:
    ScImportExport( ScDocument* );                  // Gesamtdokument
    ScImportExport( ScDocument*, const String& );   // Bereichs/Zellangabe
    ScImportExport( ScDocument*, const ScAddress& );
    ScImportExport( ScDocument*, const ScRange& );
   ~ScImportExport();

    void SetExtOptions( const ScAsciiOptions& rOpt );

    sal_Bool IsDoubleRef() const { return sal_Bool( !( bAll || bSingle ) ); }
    sal_Bool IsSingleRef() const { return bSingle; }
    sal_Bool IsNoRef() const     { return bAll;    }
    sal_Bool IsRef() const       { return sal_Bool( !bAll ); }

    const ScRange& GetRange() const { return aRange; }

    sal_Bool IsUndo() const      { return bUndo; }
    void SetUndo( sal_Bool b )   { bUndo = b;    }

    static sal_Bool  IsFormatSupported( sal_uLong nFormat );
    static const sal_Unicode* ScanNextFieldFromString( const sal_Unicode* p,
            String& rField, sal_Unicode cStr, const sal_Unicode* pSeps, bool bMergeSeps, bool& rbIsQuoted );
    static  void    WriteUnicodeOrByteString( SvStream& rStrm, const String& rString, sal_Bool bZero = false );
    static  void    WriteUnicodeOrByteEndl( SvStream& rStrm );
    static  inline  sal_Bool    IsEndianSwap( const SvStream& rStrm );

    //! only if stream is only used in own (!) memory
    static  inline  void    SetNoEndianSwap( SvStream& rStrm );

    sal_Unicode GetSeparator() const { return cSep; }
    void SetSeparator( sal_Unicode c ) { cSep = c; }
    sal_Unicode GetDelimiter() const { return cStr; }
    void SetDelimiter( sal_Unicode c ) { cStr = c; }
    sal_Bool IsFormulas() const { return bFormulas; }
    void SetFormulas( sal_Bool b ) { bFormulas = b; }
    sal_Bool IsIncludeFiltered() const { return bIncludeFiltered; }
    void SetIncludeFiltered( sal_Bool b ) { bIncludeFiltered = b; }

    void SetSizeLimit( sal_uLong nNew ) { nSizeLimit = nNew; }  // momentan nur fuer Ascii

    void            SetStreamPath( const String& rPath ) { aStreamPath = rPath; }
    const String&   GetStreamPath() const { return aStreamPath; }

    sal_Bool ImportString( const ::rtl::OUString&, sal_uLong=FORMAT_STRING );
    sal_Bool ExportString( ::rtl::OUString&, sal_uLong=FORMAT_STRING );
    sal_Bool ExportByteString( ::rtl::OString&, rtl_TextEncoding, sal_uLong=FORMAT_STRING );

    sal_Bool ImportStream( SvStream&, const String& rBaseURL, sal_uLong=FORMAT_STRING );
    sal_Bool ExportStream( SvStream&, const String& rBaseURL, sal_uLong=FORMAT_STRING );

    sal_Bool ImportData( const String& rMimeType,
                     const ::com::sun::star::uno::Any & rValue );
    sal_Bool ExportData( const String& rMimeType,
                     ::com::sun::star::uno::Any & rValue  );

    sal_Bool IsOverflow() const { return bOverflow; }       // nach dem Importieren

    const String& GetNonConvertibleChars() const { return aNonConvertibleChars; }

    bool IsApi() const { return mbApi; }
    void SetApi( bool bApi ) { mbApi = bApi; }
    const ScExportTextOptions& GetExportTextOptions() { return mExportTextOptions; }
    void SetExportTextOptions( const ScExportTextOptions& options ) { mExportTextOptions = options; }
};


inline sal_Bool ScImportExport::IsEndianSwap( const SvStream& rStrm )
{
#ifdef OSL_BIGENDIAN
    return rStrm.GetNumberFormatInt() != NUMBERFORMAT_INT_BIGENDIAN;
#else
    return rStrm.GetNumberFormatInt() != NUMBERFORMAT_INT_LITTLEENDIAN;
#endif
}

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
        SetEndianSwap( false );
    }
};

/** Read a CSV (comma separated values) data line using
    ReadUniOrByteStringLine().

    @param bEmbeddedLineBreak
    If sal_True and a line-break occurs inside a field of data,
    a line feed LF '\n' and the next line are appended. Repeats
    until a line-break is not in a field. A field is determined
    by delimiting rFieldSeparators and optionally surrounded by
    a pair of cFieldQuote characters. For a line-break to be
    within a field, the field content MUST be surrounded by
    cFieldQuote characters, and the opening cFieldQuote MUST be
    at the very start of a line or follow right behind a field
    separator with no extra characters in between. Anything,
    including field separators and escaped quotes (by doubling
    them, or preceding them with a backslash if
    bAllowBackslashEscape==sal_True) may appear in a quoted
    field.

    If bEmbeddedLineBreak==sal_False, nothing is parsed and the
    string returned is simply one ReadUniOrByteStringLine().

    @param rFieldSeparators
    A list of characters that each may act as a field separator.

    @param cFieldQuote
    The quote character used.

    @param bAllowBackslashEscape
    If sal_True, an embedded quote character inside a quoted
    field may also be escaped with a preceding backslash.
    Normally, quotes are escaped by doubling them.

    check Stream::good() to detect IO problems during read

    @ATTENTION
    Note that the string returned may be truncated even inside
    a quoted field if some (arbritary) maximum length was reached.
    There currently is no way to exactly determine the conditions,
    whether this was at a line end, or whether open quotes
    would have closed the field before the line end, as even a
    ReadUniOrByteStringLine() may return prematurely but the
    stream was positioned ahead until the real end of line.
    Additionally, due to character encoding conversions, string
    length and bytes read don't necessarily match, and
    resyncing to a previous position matching the string's
    length isn't always possible. As a result, a logical line
    with embedded line breaks and more than the maximum length
    characters will be spoiled, and a subsequent ReadCsvLine()
    may start under false preconditions.

  */
SC_DLLPUBLIC rtl::OUString ReadCsvLine(SvStream &rStream, sal_Bool bEmbeddedLineBreak,
        const String& rFieldSeparators, sal_Unicode cFieldQuote,
        sal_Bool bAllowBackslashEscape = sal_False);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
