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

class ScImportExport
{
    ScDocShell* pDocSh;
    ScDocument* pDoc;
    ScDocument* pUndoDoc;
    ScRange     aRange;
    String      aStreamPath;
    String      aNonConvertibleChars;
    sal_uLong   nSizeLimit;
    sal_Unicode cSep;                   // Separator
    sal_Unicode cStr;                   // String Delimiter
    bool        bFormulas;              // Formula in Text?
    bool        bIncludeFiltered;       // include filtered rows? (default true)
    bool        bAll;                   // no selection
    bool        bSingle;                // Single selection
    bool        bUndo;                  // with Undo?
    bool        bOverflowRow;           // too many rows
    bool        bOverflowCol;           // too many columns
    bool        bOverflowCell;          // too much data for a cell
    bool        mbApi;
    ScExportTextOptions mExportTextOptions;

    ScAsciiOptions* pExtOptions;        // extended options

    bool StartPaste();                  // Protect check, set up Undo
    void EndPaste();                    // Undo/Redo actions, Repaint
    bool Doc2Text( SvStream& );
    bool Text2Doc( SvStream& );
    bool Doc2Sylk( SvStream& );
    bool Sylk2Doc( SvStream& );
    bool Doc2HTML( SvStream&, const String& );
    bool Doc2RTF( SvStream& );
    bool Doc2Dif( SvStream& );
    bool Dif2Doc( SvStream& );
    bool ExtText2Doc( SvStream& );      // with pExtOptions
    bool RTF2Doc( SvStream&, const String& rBaseURL );
    bool HTML2Doc( SvStream&, const String& rBaseURL );

public:
    ScImportExport( ScDocument* );                  // the whole document
    ScImportExport( ScDocument*, const String& );   // Range/cell input
    ScImportExport( ScDocument*, const ScAddress& );
    ScImportExport( ScDocument*, const ScRange& );
   ~ScImportExport();

    void SetExtOptions( const ScAsciiOptions& rOpt );

    bool IsDoubleRef() const { return !( bAll || bSingle ); }
    bool IsSingleRef() const { return bSingle; }
    bool IsNoRef() const     { return bAll;    }
    bool IsRef() const       { return !bAll; }

    const ScRange& GetRange() const { return aRange; }

    bool IsUndo() const      { return bUndo; }
    void SetUndo( bool b )   { bUndo = b;    }

    static bool  IsFormatSupported( sal_uLong nFormat );
    static const sal_Unicode* ScanNextFieldFromString( const sal_Unicode* p,
            String& rField, sal_Unicode cStr, const sal_Unicode* pSeps,
            bool bMergeSeps, bool& rbIsQuoted, bool& rbOverflowCell );
    static  void    WriteUnicodeOrByteString( SvStream& rStrm, const String& rString, bool bZero = false );
    static  void    WriteUnicodeOrByteEndl( SvStream& rStrm );
    static  inline  bool    IsEndianSwap( const SvStream& rStrm );

    //! only if stream is only used in own (!) memory
    static  inline  void    SetNoEndianSwap( SvStream& rStrm );

    sal_Unicode GetSeparator() const { return cSep; }
    void SetSeparator( sal_Unicode c ) { cSep = c; }
    sal_Unicode GetDelimiter() const { return cStr; }
    void SetDelimiter( sal_Unicode c ) { cStr = c; }
    bool IsFormulas() const { return bFormulas; }
    void SetFormulas( bool b ) { bFormulas = b; }
    bool IsIncludeFiltered() const { return bIncludeFiltered; }
    void SetIncludeFiltered( bool b ) { bIncludeFiltered = b; }

    void SetSizeLimit( sal_uLong nNew ) { nSizeLimit = nNew; }  // for the moment only for Ascii

    void            SetStreamPath( const String& rPath ) { aStreamPath = rPath; }
    const String&   GetStreamPath() const { return aStreamPath; }

    bool ImportString( const ::rtl::OUString&, sal_uLong=FORMAT_STRING );
    bool ExportString( ::rtl::OUString&, sal_uLong=FORMAT_STRING );
    bool ExportByteString( ::rtl::OString&, rtl_TextEncoding, sal_uLong=FORMAT_STRING );

    bool ImportStream( SvStream&, const String& rBaseURL, sal_uLong=FORMAT_STRING );
    bool ExportStream( SvStream&, const String& rBaseURL, sal_uLong=FORMAT_STRING );

    bool ImportData( const String& rMimeType,
                     const ::com::sun::star::uno::Any & rValue );
    bool ExportData( const String& rMimeType,
                     ::com::sun::star::uno::Any & rValue  );

    // after import
    bool IsOverflowRow() const { return bOverflowRow; }
    bool IsOverflowCol() const { return bOverflowCol; }
    bool IsOverflowCell() const { return bOverflowCell; }
    bool IsOverflow() const { return bOverflowRow || bOverflowCol || bOverflowCell; }

    const String& GetNonConvertibleChars() const { return aNonConvertibleChars; }

    bool IsApi() const { return mbApi; }
    void SetApi( bool bApi ) { mbApi = bApi; }
    const ScExportTextOptions& GetExportTextOptions() { return mExportTextOptions; }
    void SetExportTextOptions( const ScExportTextOptions& options ) { mExportTextOptions = options; }
};


inline bool ScImportExport::IsEndianSwap( const SvStream& rStrm )
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
    If TRUE and a line-break occurs inside a field of data,
    a line feed LF '\n' and the next line are appended. Repeats
    until a line-break is not in a field. A field is determined
    by delimiting rFieldSeparators and optionally surrounded by
    a pair of cFieldQuote characters. For a line-break to be
    within a field, the field content MUST be surrounded by
    cFieldQuote characters, and the opening cFieldQuote MUST be
    at the very start of a line or follow right behind a field
    separator with no extra characters in between, with the
    exception of blanks contradictory to RFC 4180. Anything,
    including field separators and escaped quotes (by doubling
    them) may appear in a quoted field.

    If bEmbeddedLineBreak==FALSE, nothing is parsed and the
    string returned is simply one ReadUniOrByteStringLine().

    @param rFieldSeparators
    A list of characters that each may act as a field separator.

    @param cFieldQuote
    The quote character used.

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
SC_DLLPUBLIC rtl::OUString ReadCsvLine( SvStream &rStream, bool bEmbeddedLineBreak,
        const String& rFieldSeparators, sal_Unicode cFieldQuote );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
