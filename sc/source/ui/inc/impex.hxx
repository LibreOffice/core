/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_IMPEX_HXX
#define INCLUDED_SC_SOURCE_UI_INC_IMPEX_HXX

#include <o3tl/deleter.hxx>
#include <sot/formats.hxx>
#include <address.hxx>
#include <tools/stream.hxx>

class ScDocShell;
class ScDocument;
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
    ScDocument& rDoc;
    std::unique_ptr<ScDocument, o3tl::default_delete<ScDocument>> pUndoDoc;
    ScRange     aRange;
    OUString    aStreamPath;
    OUString    aNonConvertibleChars;
    OUString    maFilterOptions;
    sal_uInt32  nSizeLimit;
    SCROW       nMaxImportRow;
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
    bool        mbImportBroadcast; // whether or not to broadcast after data import.
    bool        mbOverwriting;  // Whether we could be overwriting existing values (paste).
                                // In this case we cannot use the insert optimization, but we
                                // do not need to broadcast after the import.
    ScExportTextOptions mExportTextOptions;

    std::unique_ptr<ScAsciiOptions> pExtOptions;        // extended options

    bool StartPaste();                  // Protect check, set up Undo
    void EndPaste(bool bAutoRowHeight = true);                    // Undo/Redo actions, Repaint
    bool Doc2Text( SvStream& );
    bool Text2Doc( SvStream& );
    bool Doc2Sylk( SvStream& );
    bool Sylk2Doc( SvStream& );
    bool Doc2HTML( SvStream&, const OUString& );
    bool Doc2RTF( SvStream& );
    bool Doc2Dif( SvStream& );
    bool Dif2Doc( SvStream& );
    bool ExtText2Doc( SvStream& );      // with pExtOptions
    bool RTF2Doc( SvStream&, const OUString& rBaseURL );
    bool HTML2Doc( SvStream&, const OUString& rBaseURL );

public:
    ScImportExport( ScDocument& );                  // the whole document
    ScImportExport( ScDocument&, const OUString& );   // Range/cell input
    ScImportExport( ScDocument&, const ScAddress& );
    ScImportExport( ScDocument&, const ScRange& );
   ~ScImportExport() COVERITY_NOEXCEPT_FALSE;

    void SetExtOptions( const ScAsciiOptions& rOpt );
    void SetFilterOptions( const OUString& rFilterOptions );
    bool IsRef() const       { return !bAll; }

    const ScRange& GetRange() const { return aRange; }

    static void EmbeddedNullTreatment( OUString & rStr );

    static bool  IsFormatSupported( SotClipboardFormatId nFormat );
    static const sal_Unicode* ScanNextFieldFromString( const sal_Unicode* p,
            OUString& rField, sal_Unicode cStr, const sal_Unicode* pSeps,
            bool bMergeSeps, bool& rbIsQuoted, bool& rbOverflowCell, bool bRemoveSpace );
    static  void    WriteUnicodeOrByteString( SvStream& rStrm, const OUString& rString, bool bZero = false );
    static  void    WriteUnicodeOrByteEndl( SvStream& rStrm );

    /** ScImportExport::CountVisualWidth
        Count the width of string visually ( in multiple of western characters), considering CJK
        ideographs and CJK symbols (U+3000-U+303F) as twice the width of western characters.
        @param rStr the string.
        @param nIdx the starting index, index is incremented for each counted character.
        @param nMaxWidth the maximum width to count.
        @return the sum of the width of counted characters.
    **/
    static sal_Int32 CountVisualWidth(const OUString& rStr, sal_Int32& nIdx, sal_Int32 nMaxWidth);

    /** ScImportExport::CountVisualWidth
        @return the sum of the viusal width of the whole string.
    **/
    static sal_Int32 CountVisualWidth(const OUString& rStr);

    //! only if stream is only used in own (!) memory
    static  void    SetNoEndianSwap( SvStream& rStrm );

    void SetSeparator( sal_Unicode c ) { cSep = c; }
    void SetDelimiter( sal_Unicode c ) { cStr = c; }
    void SetFormulas( bool b ) { bFormulas = b; }
    void SetIncludeFiltered( bool b ) { bIncludeFiltered = b; }

    void            SetStreamPath( const OUString& rPath ) { aStreamPath = rPath; }

    bool ImportString( const OUString&, SotClipboardFormatId );
    bool ExportString( OUString&, SotClipboardFormatId );
    bool ExportByteString( OString&, rtl_TextEncoding, SotClipboardFormatId );

    bool ImportStream( SvStream&, const OUString& rBaseURL, SotClipboardFormatId );
    bool ExportStream( SvStream&, const OUString& rBaseURL, SotClipboardFormatId );

    bool ExportData( const OUString& rMimeType,
                     css::uno::Any & rValue  );

    // after import
    bool IsOverflowRow() const { return bOverflowRow; }
    bool IsOverflowCol() const { return bOverflowCol; }
    bool IsOverflowCell() const { return bOverflowCell; }
    bool IsOverflow() const { return bOverflowRow || bOverflowCol || bOverflowCell; }

    const OUString& GetNonConvertibleChars() const { return aNonConvertibleChars; }

    void SetApi( bool bApi ) { mbApi = bApi; }
    void SetImportBroadcast( bool b ) { mbImportBroadcast = b; }
    void SetOverwriting( const bool bOverwriting ) { mbOverwriting = bOverwriting; }
    void SetExportTextOptions( const ScExportTextOptions& options ) { mExportTextOptions = options; }
};

// Helper class for importing clipboard strings as streams.
class ScImportStringStream : public SvMemoryStream
{
public:
    ScImportStringStream(const OUString& rStr);
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
    If rcDetectSep was 0 and a separator is detected then it is appended to
    rFieldSeparators.

    @param cFieldQuote
    The quote character used.

    @param rcDetectSep
    If 0 then attempt to detect a possible space (blank) separator if
    rFieldSeparators doesn't include it already. This can be necessary because
    of the "accept broken misquoted CSV fields" feature that tries to ignore
    trailing blanks after a quoted field and if no separator follows continues
    to add content to the field assuming the single double quote was in error.
    If this blank separator is detected it is added to rFieldSeparators and the
    line is reread with the new separators

    check Stream::good() to detect IO problems during read

    @ATTENTION
    Note that the string returned may be truncated even inside
    a quoted field if some (arbitrary) maximum length was reached.
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
SC_DLLPUBLIC OUString ReadCsvLine( SvStream &rStream, bool bEmbeddedLineBreak,
        OUString& rFieldSeparators, sal_Unicode cFieldQuote, sal_Unicode& rcDetectSep );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
