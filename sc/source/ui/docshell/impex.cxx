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

#include "sc.hrc"

#include <comphelper/processfactory.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <sot/formats.hxx>
#include <sfx2/mieclip.hxx>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>

#include "global.hxx"
#include "scerrors.hxx"
#include "docsh.hxx"
#include "undoblk.hxx"
#include "rangenam.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "filter.hxx"
#include "asciiopt.hxx"
#include "formulacell.hxx"
#include "docoptio.hxx"
#include "progress.hxx"
#include "scitems.hxx"
#include "editable.hxx"
#include "compiler.hxx"
#include "warnbox.hxx"
#include "clipparam.hxx"
#include "impex.hxx"
#include "editutil.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "stringutil.hxx"
#include "cellvalue.hxx"
#include "tokenarray.hxx"

#include "globstr.hrc"
#include <vcl/svapp.hxx>

//========================================================================

// We don't want to end up with 2GB read in one line just because of malformed
// multiline fields, so chop it _somewhere_, which is twice supported columns
// times maximum cell content length, 2*1024*64K=128M, and because it's
// sal_Unicode that's 256MB. If it's 2GB of data without LF we're out of luck
// anyway.
static const sal_Int32 nArbitraryLineLengthLimit = 2 * MAXCOLCOUNT * STRING_MAXLEN;

namespace
{
    const char SYLK_LF[]  = "\x1b :";
    const char DOUBLE_SEMICOLON[] = ";;";
    const char DOUBLE_DOUBLEQUOTE[] = "\"\"";
}

enum SylkVersion
{
    SYLK_SCALC3,    // Wrote wrongly quoted strings and unescaped semicolons.
    SYLK_OOO32,     // Correct strings, plus multiline content.
    SYLK_OWN,       // Place our new versions, if any, before this value.
    SYLK_OTHER      // Assume that aliens wrote correct strings.
};


// Gesamtdokument ohne Undo


ScImportExport::ScImportExport( ScDocument* p )
    : pDocSh( PTR_CAST(ScDocShell,p->GetDocumentShell()) ), pDoc( p ),
      nSizeLimit( 0 ), cSep( '\t' ), cStr( '"' ),
      bFormulas( false ), bIncludeFiltered( true ),
      bAll( true ), bSingle( true ), bUndo( false ),
      bOverflowRow( false ), bOverflowCol( false ), bOverflowCell( false ),
      mbApi( true ), mExportTextOptions()
{
    pUndoDoc = NULL;
    pExtOptions = NULL;
}

// Insert am Punkt ohne Bereichschecks


ScImportExport::ScImportExport( ScDocument* p, const ScAddress& rPt )
    : pDocSh( PTR_CAST(ScDocShell,p->GetDocumentShell()) ), pDoc( p ),
      aRange( rPt ),
      nSizeLimit( 0 ), cSep( '\t' ), cStr( '"' ),
      bFormulas( false ), bIncludeFiltered( true ),
      bAll( false ), bSingle( true ), bUndo( pDocSh != NULL ),
      bOverflowRow( false ), bOverflowCol( false ), bOverflowCell( false ),
      mbApi( true ), mExportTextOptions()
{
    pUndoDoc = NULL;
    pExtOptions = NULL;
}


//  ctor with a range is only used for export
//! ctor with a string (and bSingle=true) is also used for DdeSetData

ScImportExport::ScImportExport( ScDocument* p, const ScRange& r )
    : pDocSh( PTR_CAST(ScDocShell,p->GetDocumentShell()) ), pDoc( p ),
      aRange( r ),
      nSizeLimit( 0 ), cSep( '\t' ), cStr( '"' ),
      bFormulas( false ), bIncludeFiltered( true ),
      bAll( false ), bSingle( false ), bUndo( pDocSh != NULL ),
      bOverflowRow( false ), bOverflowCol( false ), bOverflowCell( false ),
      mbApi( true ), mExportTextOptions()
{
    pUndoDoc = NULL;
    pExtOptions = NULL;
    // Zur Zeit nur in einer Tabelle!
    aRange.aEnd.SetTab( aRange.aStart.Tab() );
}

// String auswerten: Entweder Bereich, Punkt oder Gesamtdoc (bei Fehler)
// Falls eine View existiert, wird die TabNo der View entnommen!


ScImportExport::ScImportExport( ScDocument* p, const String& rPos )
    : pDocSh( PTR_CAST(ScDocShell,p->GetDocumentShell()) ), pDoc( p ),
      nSizeLimit( 0 ), cSep( '\t' ), cStr( '"' ),
      bFormulas( false ), bIncludeFiltered( true ),
      bAll( false ), bSingle( true ), bUndo( pDocSh != NULL ),
      bOverflowRow( false ), bOverflowCol( false ), bOverflowCell( false ),
      mbApi( true ), mExportTextOptions()
{
    pUndoDoc = NULL;
    pExtOptions = NULL;

    SCTAB nTab = ScDocShell::GetCurTab();
    aRange.aStart.SetTab( nTab );
    String aPos( rPos );
    //  Benannter Bereich?
    ScRangeName* pRange = pDoc->GetRangeName();
    if( pRange )
    {
        const ScRangeData* pData = pRange->findByUpperName(ScGlobal::pCharClass->uppercase(aPos));
        if (pData)
        {
            if( pData->HasType( RT_REFAREA )
                || pData->HasType( RT_ABSAREA )
                || pData->HasType( RT_ABSPOS ) )
                pData->GetSymbol( aPos );                   // mit dem Inhalt weitertesten
        }
    }
    formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    // Bereich?
    if( aRange.Parse( aPos, pDoc, eConv ) & SCA_VALID )
        bSingle = false;
    // Zelle?
    else if( aRange.aStart.Parse( aPos, pDoc, eConv ) & SCA_VALID )
        aRange.aEnd = aRange.aStart;
    else
        bAll = true;
}


ScImportExport::~ScImportExport()
{
    delete pUndoDoc;
    delete pExtOptions;
}


void ScImportExport::SetExtOptions( const ScAsciiOptions& rOpt )
{
    if ( pExtOptions )
        *pExtOptions = rOpt;
    else
        pExtOptions = new ScAsciiOptions( rOpt );

    //  "normale" Optionen uebernehmen

    cSep = rOpt.GetFieldSeps().GetChar(0);
    cStr = rOpt.GetTextSep();
}


bool ScImportExport::IsFormatSupported( sal_uLong nFormat )
{
    return nFormat == FORMAT_STRING
              || nFormat == SOT_FORMATSTR_ID_SYLK
              || nFormat == SOT_FORMATSTR_ID_LINK
              || nFormat == SOT_FORMATSTR_ID_HTML
              || nFormat == SOT_FORMATSTR_ID_HTML_SIMPLE
              || nFormat == SOT_FORMATSTR_ID_DIF;
}


//////////////////////////////////////////////////////////////////////////////

// Vorbereitung fuer Undo: Undo-Dokument erzeugen


bool ScImportExport::StartPaste()
{
    if ( !bAll )
    {
        ScEditableTester aTester( pDoc, aRange );
        if ( !aTester.IsEditable() )
        {
            InfoBox aInfoBox(Application::GetDefDialogParent(),
                                ScGlobal::GetRscString( aTester.GetMessageId() ) );
            aInfoBox.Execute();
            return false;
        }
    }
    if( bUndo && pDocSh && pDoc->IsUndoEnabled())
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, aRange.aStart.Tab(), aRange.aEnd.Tab() );
        pDoc->CopyToDocument( aRange, IDF_ALL | IDF_NOCAPTIONS, false, pUndoDoc );
    }
    return true;
}

// Nachbereitung Insert: Undo/Redo-Aktionen erzeugen, Invalidate/Repaint


void ScImportExport::EndPaste()
{
    bool bHeight = pDocSh && pDocSh->AdjustRowHeight(
                    aRange.aStart.Row(), aRange.aEnd.Row(), aRange.aStart.Tab() );

    if( pUndoDoc && pDoc->IsUndoEnabled() )
    {
        ScDocument* pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
        pRedoDoc->InitUndo( pDoc, aRange.aStart.Tab(), aRange.aEnd.Tab() );
        pDoc->CopyToDocument( aRange, IDF_ALL | IDF_NOCAPTIONS, false, pRedoDoc );
        ScMarkData aDestMark;
        aDestMark.SetMarkArea(aRange);
        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoPaste(pDocSh, aRange, aDestMark, pUndoDoc, pRedoDoc, IDF_ALL, NULL));
    }
    pUndoDoc = NULL;
    if( pDocSh )
    {
        if (!bHeight)
            pDocSh->PostPaint( aRange, PAINT_GRID );    // AdjustRowHeight paintet evtl. selber
        pDocSh->SetDocumentModified();
    }
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh )
        pViewSh->UpdateInputHandler();

}

/////////////////////////////////////////////////////////////////////////////

bool ScImportExport::ImportData( const String& /* rMimeType */,
                     const ::com::sun::star::uno::Any & /* rValue */ )
{
    OSL_ENSURE( !this, "Implementation is missing" );
    return false;
}

bool ScImportExport::ExportData( const String& rMimeType,
                                 ::com::sun::star::uno::Any & rValue )
{
    SvMemoryStream aStrm;
    // mba: no BaseURL for data exchange
    if( ExportStream( aStrm, String(),
                SotExchange::GetFormatIdFromMimeType( rMimeType ) ))
    {
        aStrm << (sal_uInt8) 0;
        rValue <<= ::com::sun::star::uno::Sequence< sal_Int8 >(
                                        (sal_Int8*)aStrm.GetData(),
                                        aStrm.Seek( STREAM_SEEK_TO_END ) );
        return true;
    }
    return false;
}


bool ScImportExport::ImportString( const OUString& rText, sal_uLong nFmt )
{
    switch ( nFmt )
    {
        // formats supporting unicode
        case FORMAT_STRING :
        {
            ScImportStringStream aStrm( rText);
            return ImportStream( aStrm, String(), nFmt );
            // ImportStream must handle RTL_TEXTENCODING_UNICODE
        }
        //break;
        default:
        {
            rtl_TextEncoding eEnc = osl_getThreadTextEncoding();
            OString aTmp( rText.getStr(), rText.getLength(), eEnc );
            SvMemoryStream aStrm( (void*)aTmp.getStr(), aTmp.getLength() * sizeof(sal_Char), STREAM_READ );
            aStrm.SetStreamCharSet( eEnc );
            SetNoEndianSwap( aStrm );       //! no swapping in memory
            return ImportStream( aStrm, String(), nFmt );
        }
    }
}


bool ScImportExport::ExportString( OUString& rText, sal_uLong nFmt )
{
    OSL_ENSURE( nFmt == FORMAT_STRING, "ScImportExport::ExportString: Unicode not supported for other formats than FORMAT_STRING" );
    if ( nFmt != FORMAT_STRING )
    {
        rtl_TextEncoding eEnc = osl_getThreadTextEncoding();
        OString aTmp;
        bool bOk = ExportByteString( aTmp, eEnc, nFmt );
        rText = OStringToOUString( aTmp, eEnc );
        return bOk;
    }
    //  nSizeLimit not needed for OUString

    SvMemoryStream aStrm;
    aStrm.SetStreamCharSet( RTL_TEXTENCODING_UNICODE );
    SetNoEndianSwap( aStrm );       //! no swapping in memory
    // mba: no BaseURL for data exc
    if( ExportStream( aStrm, String(), nFmt ) )
    {
        aStrm << (sal_Unicode) 0;
        aStrm.Seek( STREAM_SEEK_TO_END );

        rText = OUString( (const sal_Unicode*) aStrm.GetData() );
        return true;
    }
    rText = OUString();
    return false;

    // ExportStream must handle RTL_TEXTENCODING_UNICODE
}


bool ScImportExport::ExportByteString( OString& rText, rtl_TextEncoding eEnc, sal_uLong nFmt )
{
    OSL_ENSURE( eEnc != RTL_TEXTENCODING_UNICODE, "ScImportExport::ExportByteString: Unicode not supported" );
    if ( eEnc == RTL_TEXTENCODING_UNICODE )
        eEnc = osl_getThreadTextEncoding();

    if (!nSizeLimit)
        nSizeLimit = STRING_MAXLEN;

    SvMemoryStream aStrm;
    aStrm.SetStreamCharSet( eEnc );
    SetNoEndianSwap( aStrm );       //! no swapping in memory
    // mba: no BaseURL for data exchange
    if( ExportStream( aStrm, String(), nFmt ) )
    {
        aStrm << (sal_Char) 0;
        aStrm.Seek( STREAM_SEEK_TO_END );
        // Sicherheits-Check:
        if( aStrm.Tell() <= (sal_uLong) STRING_MAXLEN )
        {
            rText = (const sal_Char*) aStrm.GetData();
            return true;
        }
    }
    rText = OString();
    return false;
}


bool ScImportExport::ImportStream( SvStream& rStrm, const String& rBaseURL, sal_uLong nFmt )
{
    if( nFmt == FORMAT_STRING )
    {
        if( ExtText2Doc( rStrm ) )      // pExtOptions auswerten
            return true;
    }
    if( nFmt == SOT_FORMATSTR_ID_SYLK )
    {
        if( Sylk2Doc( rStrm ) )
            return true;
    }
    if( nFmt == SOT_FORMATSTR_ID_DIF )
    {
        if( Dif2Doc( rStrm ) )
            return true;
    }
    if( nFmt == FORMAT_RTF )
    {
        if( RTF2Doc( rStrm, rBaseURL ) )
            return true;
    }
    if( nFmt == SOT_FORMATSTR_ID_LINK )
        return true;            // Link-Import?
    if ( nFmt == SOT_FORMATSTR_ID_HTML )
    {
        if( HTML2Doc( rStrm, rBaseURL ) )
            return true;
    }
    if ( nFmt == SOT_FORMATSTR_ID_HTML_SIMPLE )
    {
        MSE40HTMLClipFormatObj aMSE40ClpObj;                // needed to skip the header data
        SvStream* pHTML = aMSE40ClpObj.IsValid( rStrm );
        if ( pHTML && HTML2Doc( *pHTML, rBaseURL ) )
            return true;
    }

    return false;
}


bool ScImportExport::ExportStream( SvStream& rStrm, const String& rBaseURL, sal_uLong nFmt )
{
    if( nFmt == FORMAT_STRING )
    {
        if( Doc2Text( rStrm ) )
            return true;
    }
    if( nFmt == SOT_FORMATSTR_ID_SYLK )
    {
        if( Doc2Sylk( rStrm ) )
            return true;
    }
    if( nFmt == SOT_FORMATSTR_ID_DIF )
    {
        if( Doc2Dif( rStrm ) )
            return true;
    }
    if( nFmt == SOT_FORMATSTR_ID_LINK && !bAll )
    {
        String aDocName;
        if ( pDoc->IsClipboard() )
            aDocName = ScGlobal::GetClipDocName();
        else
        {
            SfxObjectShell* pShell = pDoc->GetDocumentShell();
            if (pShell)
                aDocName = pShell->GetTitle( SFX_TITLE_FULLNAME );
        }

        OSL_ENSURE( aDocName.Len(), "ClipBoard document has no name! :-/" );
        if( aDocName.Len() )
        {
            // Always use Calc A1 syntax for paste link.
            String aRefName;
            sal_uInt16 nFlags = SCA_VALID | SCA_TAB_3D;
            if( bSingle )
                aRange.aStart.Format( aRefName, nFlags, pDoc, formula::FormulaGrammar::CONV_OOO );
            else
            {
                if( aRange.aStart.Tab() != aRange.aEnd.Tab() )
                    nFlags |= SCA_TAB2_3D;
                aRange.Format( aRefName, nFlags, pDoc, formula::FormulaGrammar::CONV_OOO );
            }
            String aAppName = Application::GetAppName();

            // extra bits are used to tell the client to prefer external
            // reference link.
            OUString aExtraBits("calc:extref");

            WriteUnicodeOrByteString( rStrm, aAppName, true );
            WriteUnicodeOrByteString( rStrm, aDocName, true );
            WriteUnicodeOrByteString( rStrm, aRefName, true );
            WriteUnicodeOrByteString( rStrm, aExtraBits, true );
            if ( rStrm.GetStreamCharSet() == RTL_TEXTENCODING_UNICODE )
                rStrm << sal_Unicode(0);
            else
                rStrm << sal_Char(0);
            return rStrm.GetError() == SVSTREAM_OK;
        }
    }
    if( nFmt == SOT_FORMATSTR_ID_HTML )
    {
        if( Doc2HTML( rStrm, rBaseURL ) )
            return true;
    }
    if( nFmt == FORMAT_RTF )
    {
        if( Doc2RTF( rStrm ) )
            return true;
    }

    return false;
}


void ScImportExport::WriteUnicodeOrByteString( SvStream& rStrm, const String& rString, bool bZero )
{
    rtl_TextEncoding eEnc = rStrm.GetStreamCharSet();
    if ( eEnc == RTL_TEXTENCODING_UNICODE )
    {
        if ( !IsEndianSwap( rStrm ) )
            rStrm.Write( rString.GetBuffer(), rString.Len() * sizeof(sal_Unicode) );
        else
        {
            const sal_Unicode* p = rString.GetBuffer();
            const sal_Unicode* const pStop = p + rString.Len();
            while ( p < pStop )
            {
                rStrm << *p;
            }
        }
        if ( bZero )
            rStrm << sal_Unicode(0);
    }
    else
    {
        OString aByteStr(OUStringToOString(rString, eEnc));
        rStrm << aByteStr.getStr();
        if ( bZero )
            rStrm << sal_Char(0);
    }
}


// This function could be replaced by endlub()
void ScImportExport::WriteUnicodeOrByteEndl( SvStream& rStrm )
{
    if ( rStrm.GetStreamCharSet() == RTL_TEXTENCODING_UNICODE )
    {   // same as endl() but unicode
        switch ( rStrm.GetLineDelimiter() )
        {
            case LINEEND_CR :
                rStrm << sal_Unicode('\r');
            break;
            case LINEEND_LF :
                rStrm << sal_Unicode('\n');
            break;
            default:
                rStrm << sal_Unicode('\r') << sal_Unicode('\n');
        }
    }
    else
        endl( rStrm );
}


enum QuoteType
{
    FIELDSTART_QUOTE,
    FIRST_QUOTE,
    SECOND_QUOTE,
    FIELDEND_QUOTE,
    DONTKNOW_QUOTE
};


/** Determine if *p is a quote that ends a quoted field.

    Precondition: we are parsing a quoted field already and *p is a quote.

    @return
        FIELDEND_QUOTE if end of field quote
        DONTKNOW_QUOTE anything else
 */
static QuoteType lcl_isFieldEndQuote( const sal_Unicode* p, const sal_Unicode* pSeps )
{
    // Due to broken CSV generators that don't double embedded quotes check if
    // a field separator immediately or with trailing spaces follows the quote,
    // only then end the field, or at end of string.
    const sal_Unicode cBlank = ' ';
    if (p[1] == cBlank && ScGlobal::UnicodeStrChr( pSeps, cBlank))
        return FIELDEND_QUOTE;
    while (p[1] == cBlank)
        ++p;
    if (!p[1] || ScGlobal::UnicodeStrChr( pSeps, p[1]))
        return FIELDEND_QUOTE;
    return DONTKNOW_QUOTE;
}


/** Determine if *p is a quote that is escaped by being doubled or ends a
    quoted field.

    Precondition: *p is a quote.

    @param nQuotes
        Quote characters encountered so far.
        Odd (after opening quote) means either no embedded quotes or only quote
        pairs so far.
        Even means either not in a quoted field or already one quote
        encountered, the first of a pair.

    @return
        FIELDSTART_QUOTE if first quote in a field, either starting content or
                            embedded so caller should check beforehand.
        FIRST_QUOTE      if first of a doubled quote
        SECOND_QUOTE     if second of a doubled quote
        FIELDEND_QUOTE   if end of field quote
        DONTKNOW_QUOTE   if an unescaped quote we don't consider as end of field,
                            do not increment nQuotes in caller then!
 */
static QuoteType lcl_isEscapedOrFieldEndQuote( sal_Int32 nQuotes, const sal_Unicode* p,
        const sal_Unicode* pSeps, sal_Unicode cStr )
{
    if ((nQuotes % 2) == 0)
    {
        if (p[-1] == cStr)
            return SECOND_QUOTE;
        else
        {
            SAL_WARN( "sc", "lcl_isEscapedOrFieldEndQuote: really want a FIELDSTART_QUOTE?");
            return FIELDSTART_QUOTE;
        }
    }
    if (p[1] == cStr)
        return FIRST_QUOTE;
    return lcl_isFieldEndQuote( p, pSeps);
}


/** Append characters of [p1,p2) to rField.

    @returns TRUE if ok; FALSE if data overflow, truncated
 */
static bool lcl_appendLineData( String& rField, const sal_Unicode* p1, const sal_Unicode* p2 )
{
    OSL_ENSURE( rField.Len() + (p2 - p1) <= STRING_MAXLEN, "lcl_appendLineData: data overflow");
    if (rField.Len() + (p2 - p1) <= STRING_MAXLEN)
    {
        rField.Append( p1, sal::static_int_cast<xub_StrLen>( p2 - p1 ) );
        return true;
    }
    else
    {
        // If STRING_MAXLEN is passed as length, then String attempts to
        // determine the length of the string and comes up with an overflow
        // casted to xub_StrLen again ... so pass max-1, data will be truncated
        // anyway.
        rField.Append( p1, (rField.Len() ? STRING_MAXLEN - rField.Len() : STRING_MAXLEN - 1) );
        return false;
    }
}


enum DoubledQuoteMode
{
    DQM_KEEP_ALL,   // both are taken, additionally start and end quote are included in string
    DQM_KEEP,       // both are taken
    DQM_ESCAPE,     // escaped quote, one is taken, one ignored
    DQM_CONCAT,     // first is end, next is start, both ignored => strings combined
    DQM_SEPARATE    // end one string and begin next
};

static const sal_Unicode* lcl_ScanString( const sal_Unicode* p, String& rString,
            const sal_Unicode* pSeps, sal_Unicode cStr, DoubledQuoteMode eMode, bool& rbOverflowCell )
{
    if (eMode != DQM_KEEP_ALL)
        p++;    //! jump over opening quote
    bool bCont;
    do
    {
        bCont = false;
        const sal_Unicode* p0 = p;
        for( ;; )
        {
            if( !*p )
                break;
            if( *p == cStr )
            {
                if ( *++p != cStr )
                {
                    // break or continue for loop
                    if (eMode == DQM_ESCAPE)
                    {
                        if (lcl_isFieldEndQuote( p-1, pSeps) == FIELDEND_QUOTE)
                            break;
                        else
                            continue;
                    }
                    else
                        break;
                }
                // doubled quote char
                switch ( eMode )
                {
                    case DQM_KEEP_ALL :
                    case DQM_KEEP :
                        p++;            // both for us (not breaking for-loop)
                    break;
                    case DQM_ESCAPE :
                        p++;            // one for us (breaking for-loop)
                        bCont = true;   // and more
                    break;
                    case DQM_CONCAT :
                        if ( p0+1 < p )
                        {
                            // first part
                            if (!lcl_appendLineData( rString, p0, p-1))
                                rbOverflowCell = true;
                        }
                        p0 = ++p;       // text of next part starts here
                    break;
                    case DQM_SEPARATE :
                                        // positioned on next opening quote
                    break;
                }
                if ( eMode == DQM_ESCAPE || eMode == DQM_SEPARATE )
                    break;
            }
            else
                p++;
        }
        if ( p0 < p )
        {
            if (!lcl_appendLineData( rString, p0, ((eMode != DQM_KEEP_ALL && (*p || *(p-1) == cStr)) ? p-1 : p)))
                rbOverflowCell = true;
        }
    } while ( bCont );
    return p;
}

static void lcl_UnescapeSylk( String & rString, SylkVersion eVersion )
{
    // Older versions didn't escape the semicolon.
    // Older versions quoted the string and doubled embedded quotes, but not
    // the semicolons, which was plain wrong.
    if (eVersion >= SYLK_OOO32)
        rString.SearchAndReplaceAll( OUString(DOUBLE_SEMICOLON), OUString(';') );
    else
        rString.SearchAndReplaceAll( OUString(DOUBLE_DOUBLEQUOTE), OUString('"') );

    rString.SearchAndReplaceAll( OUString(SYLK_LF), OUString('\n') );
}

static const sal_Unicode* lcl_ScanSylkString( const sal_Unicode* p,
        String& rString, SylkVersion eVersion )
{
    const sal_Unicode* pStartQuote = p;
    const sal_Unicode* pEndQuote = 0;
    while( *(++p) )
    {
        if( *p == '"' )
        {
            pEndQuote = p;
            if (eVersion >= SYLK_OOO32)
            {
                if (*(p+1) == ';')
                {
                    if (*(p+2) == ';')
                    {
                        p += 2;     // escaped ';'
                        pEndQuote = 0;
                    }
                    else
                        break;      // end field
                }
            }
            else
            {
                if (*(p+1) == '"')
                {
                    ++p;            // escaped '"'
                    pEndQuote = 0;
                }
                else if (*(p+1) == ';')
                    break;          // end field
            }
        }
    }
    if (!pEndQuote)
        pEndQuote = p;  // Take all data as string.
    rString.Append( pStartQuote + 1, sal::static_int_cast<xub_StrLen>( pEndQuote - pStartQuote - 1 ) );
    lcl_UnescapeSylk( rString, eVersion);
    return p;
}

static const sal_Unicode* lcl_ScanSylkFormula( const sal_Unicode* p,
        String& rString, SylkVersion eVersion )
{
    const sal_Unicode* pStart = p;
    if (eVersion >= SYLK_OOO32)
    {
        while (*p)
        {
            if (*p == ';')
            {
                if (*(p+1) == ';')
                    ++p;        // escaped ';'
                else
                    break;      // end field
            }
            ++p;
        }
        rString.Append( pStart, sal::static_int_cast<xub_StrLen>( p - pStart));
        lcl_UnescapeSylk( rString, eVersion);
    }
    else
    {
        // Nasty. If in old versions the formula contained a semicolon, it was
        // quoted and embedded quotes were doubled, but semicolons were not. If
        // there was no semicolon, it could still contain quotes and doubled
        // embedded quotes if it was something like ="a""b", which was saved as
        // E"a""b" as is and has to be preserved, even if older versions
        // couldn't even load it correctly. However, theoretically another
        // field might follow and thus the line contain a semicolon again, such
        // as ...;E"a""b";...
        bool bQuoted = false;
        if (*p == '"')
        {
            // May be a quoted expression or just a string constant expression
            // with quotes.
            while (*(++p))
            {
                if (*p == '"')
                {
                    if (*(p+1) == '"')
                        ++p;            // escaped '"'
                    else
                        break;          // closing '"', had no ';' yet
                }
                else if (*p == ';')
                {
                    bQuoted = true;     // ';' within quoted expression
                    break;
                }
            }
            p = pStart;
        }
        if (bQuoted)
            p = lcl_ScanSylkString( p, rString, eVersion);
        else
        {
            while (*p && *p != ';')
                ++p;
            rString.Append( pStart, sal::static_int_cast<xub_StrLen>( p - pStart));
        }
    }
    return p;
}

static void lcl_DoubleEscapeChar( String& rString, sal_Unicode cStr )
{
    xub_StrLen n = 0;
    while( ( n = rString.Search( cStr, n ) ) != STRING_NOTFOUND )
    {
        rString.Insert( cStr, n );
        n += 2;
    }
}

static void lcl_WriteString( SvStream& rStrm, String& rString, sal_Unicode cQuote, sal_Unicode cEsc )
{
    if (cEsc)
        lcl_DoubleEscapeChar( rString, cEsc );

    if (cQuote)
    {
        rString.Insert( cQuote, 0 );
        rString.Append( cQuote );
    }

    ScImportExport::WriteUnicodeOrByteString( rStrm, rString );
}

static inline void lcl_WriteSimpleString( SvStream& rStrm, const String& rString )
{
    ScImportExport::WriteUnicodeOrByteString( rStrm, rString );
}

//////////////////////////////////////////////////////////////////////////////


bool ScImportExport::Text2Doc( SvStream& rStrm )
{
    bool bOk = true;

    sal_Unicode pSeps[2];
    pSeps[0] = cSep;
    pSeps[1] = 0;

    SCCOL nStartCol = aRange.aStart.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nEndRow = aRange.aEnd.Row();
    sal_uLong  nOldPos = rStrm.Tell();
    rStrm.StartReadingUnicodeText( rStrm.GetStreamCharSet() );
    bool   bData = !bSingle;
    if( !bSingle)
        bOk = StartPaste();

    while( bOk )
    {
        OUString aLine;
        String aCell;
        SCROW nRow = nStartRow;
        rStrm.Seek( nOldPos );
        for( ;; )
        {
            rStrm.ReadUniOrByteStringLine( aLine, rStrm.GetStreamCharSet(), nArbitraryLineLengthLimit );
            if( rStrm.IsEof() )
                break;
            SCCOL nCol = nStartCol;
            const sal_Unicode* p = aLine.getStr();
            while( *p )
            {
                aCell.Erase();
                const sal_Unicode* q = p;
                while (*p && *p != cSep)
                {
                    // Always look for a pairing quote and ignore separator in between.
                    while (*p && *p == cStr)
                        q = p = lcl_ScanString( p, aCell, pSeps, cStr, DQM_KEEP_ALL, bOverflowCell );
                    // All until next separator or quote.
                    while (*p && *p != cSep && *p != cStr)
                        ++p;
                    if (!lcl_appendLineData( aCell, q, p))
                        bOverflowCell = true;   // display warning on import
                    q = p;
                }
                if (*p)
                    ++p;
                if (ValidCol(nCol) && ValidRow(nRow) )
                {
                    if( bSingle )
                    {
                        if (nCol>nEndCol) nEndCol = nCol;
                        if (nRow>nEndRow) nEndRow = nRow;
                    }
                    if( bData && nCol <= nEndCol && nRow <= nEndRow )
                        pDoc->SetString( nCol, nRow, aRange.aStart.Tab(), aCell );
                }
                else                            // zuviele Spalten/Zeilen
                {
                    if (!ValidRow(nRow))
                        bOverflowRow = true;    // display warning on import
                    if (!ValidCol(nCol))
                        bOverflowCol = true;    // display warning on import
                }
                ++nCol;
            }
            ++nRow;
        }

        if( !bData )
        {
            aRange.aEnd.SetCol( nEndCol );
            aRange.aEnd.SetRow( nEndRow );
            bOk = StartPaste();
            bData = true;
        }
        else
            break;
    }

    EndPaste();
    return bOk;
}

        //
        //  erweiterter Ascii-Import
        //


static bool lcl_PutString(
    ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rStr, sal_uInt8 nColFormat,
    SvNumberFormatter* pFormatter, bool bDetectNumFormat,
    ::utl::TransliterationWrapper& rTransliteration, CalendarWrapper& rCalendar,
    ::utl::TransliterationWrapper* pSecondTransliteration, CalendarWrapper* pSecondCalendar )
{
    bool bMultiLine = false;
    if ( nColFormat == SC_COL_SKIP || !rStr.Len() || !ValidCol(nCol) || !ValidRow(nRow) )
        return bMultiLine;

    if ( nColFormat == SC_COL_TEXT )
    {
        double fDummy;
        sal_uInt32 nIndex = 0;
        if (pFormatter->IsNumberFormat(rStr, nIndex, fDummy))
        {
            // Set the format of this cell to Text.
            sal_uInt32 nFormat = pFormatter->GetStandardFormat(NUMBERFORMAT_TEXT);
            ScPatternAttr aNewAttrs(pDoc->GetPool());
            SfxItemSet& rSet = aNewAttrs.GetItemSet();
            rSet.Put( SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat) );
            pDoc->ApplyPattern(nCol, nRow, nTab, aNewAttrs);

        }
        pDoc->SetTextCell(ScAddress(nCol,nRow,nTab), rStr);
        return bMultiLine;
    }

    if ( nColFormat == SC_COL_ENGLISH )
    {
        //! SetString mit Extra-Flag ???

        SvNumberFormatter* pDocFormatter = pDoc->GetFormatTable();
        sal_uInt32 nEnglish = pDocFormatter->GetStandardIndex(LANGUAGE_ENGLISH_US);
        double fVal;
        if ( pDocFormatter->IsNumberFormat( rStr, nEnglish, fVal ) )
        {
            //  Zahlformat wird nicht auf englisch gesetzt
            pDoc->SetValue( nCol, nRow, nTab, fVal );
            return bMultiLine;
        }
        //  sonst weiter mit SetString
    }
    else if ( nColFormat != SC_COL_STANDARD )                   // Datumsformate
    {
        const sal_uInt16 nMaxNumberParts = 7;   // Y-M-D h:m:s.t
        xub_StrLen nLen = rStr.Len();
        xub_StrLen nStart[nMaxNumberParts];
        xub_StrLen nEnd[nMaxNumberParts];

        sal_uInt16 nDP, nMP, nYP;
        switch ( nColFormat )
        {
            case SC_COL_YMD: nDP = 2; nMP = 1; nYP = 0; break;
            case SC_COL_MDY: nDP = 1; nMP = 0; nYP = 2; break;
            case SC_COL_DMY:
            default:         nDP = 0; nMP = 1; nYP = 2; break;
        }

        sal_uInt16 nFound = 0;
        bool bInNum = false;
        for ( xub_StrLen nPos=0; nPos<nLen && (bInNum ||
                    nFound<nMaxNumberParts); nPos++ )
        {
            if (bInNum && nFound == 3 && nColFormat == SC_COL_YMD &&
                    nPos <= nStart[nFound]+2 && rStr.GetChar(nPos) == 'T')
                bInNum = false;     // ISO-8601: YYYY-MM-DDThh:mm...
            else if ((((!bInNum && nFound==nMP) || (bInNum && nFound==nMP+1))
                        && ScGlobal::pCharClass->isLetterNumeric( rStr, nPos))
                    || ScGlobal::pCharClass->isDigit( rStr, nPos))
            {
                if (!bInNum)
                {
                    bInNum = true;
                    nStart[nFound] = nPos;
                    ++nFound;
                }
                nEnd[nFound-1] = nPos;
            }
            else
                bInNum = false;
        }

        if ( nFound == 1 )
        {
            //  try to break one number (without separators) into date fields

            xub_StrLen nDateStart = nStart[0];
            xub_StrLen nDateLen = nEnd[0] + 1 - nDateStart;

            if ( nDateLen >= 5 && nDateLen <= 8 &&
                    ScGlobal::pCharClass->isNumeric( rStr.Copy( nDateStart, nDateLen ) ) )
            {
                //  6 digits: 2 each for day, month, year
                //  8 digits: 4 for year, 2 each for day and month
                //  5 or 7 digits: first field is shortened by 1

                bool bLongYear = ( nDateLen >= 7 );
                bool bShortFirst = ( nDateLen == 5 || nDateLen == 7 );

                sal_uInt16 nFieldStart = nDateStart;
                for (sal_uInt16 nPos=0; nPos<3; nPos++)
                {
                    sal_uInt16 nFieldEnd = nFieldStart + 1;     // default: 2 digits
                    if ( bLongYear && nPos == nYP )
                        nFieldEnd += 2;                     // 2 extra digits for long year
                    if ( bShortFirst && nPos == 0 )
                        --nFieldEnd;                        // first field shortened?

                    nStart[nPos] = nFieldStart;
                    nEnd[nPos]   = nFieldEnd;
                    nFieldStart  = nFieldEnd + 1;
                }
                nFound = 3;
            }
        }

        if ( nFound >= 3 )
        {
            using namespace ::com::sun::star;
            bool bSecondCal = false;
            sal_uInt16 nDay  = (sal_uInt16) rStr.Copy( nStart[nDP], nEnd[nDP]+1-nStart[nDP] ).ToInt32();
            sal_uInt16 nYear = (sal_uInt16) rStr.Copy( nStart[nYP], nEnd[nYP]+1-nStart[nYP] ).ToInt32();
            String aMStr = rStr.Copy( nStart[nMP], nEnd[nMP]+1-nStart[nMP] );
            sal_Int16 nMonth = (sal_Int16) aMStr.ToInt32();
            if (!nMonth)
            {
                static const String aSeptCorrect( RTL_CONSTASCII_USTRINGPARAM( "SEPT" ) );
                static const String aSepShortened( RTL_CONSTASCII_USTRINGPARAM( "SEP" ) );
                uno::Sequence< i18n::CalendarItem2 > xMonths;
                sal_Int32 i, nMonthCount;
                //  first test all month names from local international
                xMonths = rCalendar.getMonths();
                nMonthCount = xMonths.getLength();
                for (i=0; i<nMonthCount && !nMonth; i++)
                {
                    if ( rTransliteration.isEqual( aMStr, xMonths[i].FullName ) ||
                         rTransliteration.isEqual( aMStr, xMonths[i].AbbrevName ) )
                        nMonth = sal::static_int_cast<sal_Int16>( i+1 );
                    else if ( i == 8 && rTransliteration.isEqual( aSeptCorrect,
                                xMonths[i].AbbrevName ) &&
                            rTransliteration.isEqual( aMStr, aSepShortened ) )
                    {   // correct English abbreviation is SEPT,
                        // but data mostly contains SEP only
                        nMonth = sal::static_int_cast<sal_Int16>( i+1 );
                    }
                }
                //  if none found, then test english month names
                if ( !nMonth && pSecondCalendar && pSecondTransliteration )
                {
                    xMonths = pSecondCalendar->getMonths();
                    nMonthCount = xMonths.getLength();
                    for (i=0; i<nMonthCount && !nMonth; i++)
                    {
                        if ( pSecondTransliteration->isEqual( aMStr, xMonths[i].FullName ) ||
                             pSecondTransliteration->isEqual( aMStr, xMonths[i].AbbrevName ) )
                        {
                            nMonth = sal::static_int_cast<sal_Int16>( i+1 );
                            bSecondCal = true;
                        }
                        else if ( i == 8 && pSecondTransliteration->isEqual(
                                    aMStr, aSepShortened ) )
                        {   // correct English abbreviation is SEPT,
                            // but data mostly contains SEP only
                            nMonth = sal::static_int_cast<sal_Int16>( i+1 );
                            bSecondCal = true;
                        }
                    }
                }
            }

            SvNumberFormatter* pDocFormatter = pDoc->GetFormatTable();
            if ( nYear < 100 )
                nYear = pDocFormatter->ExpandTwoDigitYear( nYear );

            CalendarWrapper* pCalendar = (bSecondCal ? pSecondCalendar : &rCalendar);
            sal_Int16 nNumMonths = pCalendar->getNumberOfMonthsInYear();
            if ( nDay && nMonth && nDay<=31 && nMonth<=nNumMonths )
            {
                --nMonth;
                pCalendar->setValue( i18n::CalendarFieldIndex::DAY_OF_MONTH, nDay );
                pCalendar->setValue( i18n::CalendarFieldIndex::MONTH, nMonth );
                pCalendar->setValue( i18n::CalendarFieldIndex::YEAR, nYear );
                sal_Int16 nHour, nMinute, nSecond, nMilli;
                // #i14974# The imported value should have no fractional value, so set the
                // time fields to zero (ICU calendar instance defaults to current date/time)
                nHour = nMinute = nSecond = nMilli = 0;
                if (nFound > 3)
                    nHour = (sal_Int16) rStr.Copy( nStart[3], nEnd[3]+1-nStart[3]).ToInt32();
                if (nFound > 4)
                    nMinute = (sal_Int16) rStr.Copy( nStart[4], nEnd[4]+1-nStart[4]).ToInt32();
                if (nFound > 5)
                    nSecond = (sal_Int16) rStr.Copy( nStart[5], nEnd[5]+1-nStart[5]).ToInt32();
                if (nFound > 6)
                {
                    sal_Unicode cDec = '.';
                    OUString aT( &cDec, 1);
                    aT += rStr.Copy( nStart[6], nEnd[6]+1-nStart[6]);
                    rtl_math_ConversionStatus eStatus;
                    double fV = rtl::math::stringToDouble( aT, cDec, 0, &eStatus, 0);
                    if (eStatus == rtl_math_ConversionStatus_Ok)
                        nMilli = (sal_Int16) (1000.0 * fV + 0.5);
                }
                pCalendar->setValue( i18n::CalendarFieldIndex::HOUR, nHour );
                pCalendar->setValue( i18n::CalendarFieldIndex::MINUTE, nMinute );
                pCalendar->setValue( i18n::CalendarFieldIndex::SECOND, nSecond );
                pCalendar->setValue( i18n::CalendarFieldIndex::MILLISECOND, nMilli );
                if ( pCalendar->isValid() )
                {
                    double fDiff = DateTime(*pDocFormatter->GetNullDate()) -
                        pCalendar->getEpochStart();
                    // #i14974# must use getLocalDateTime to get the same
                    // date values as set above
                    double fDays = pCalendar->getLocalDateTime();
                    fDays -= fDiff;

                    LanguageType eLatin, eCjk, eCtl;
                    pDoc->GetLanguage( eLatin, eCjk, eCtl );
                    LanguageType eDocLang = eLatin;     //! which language for date formats?

                    short nType = (nFound > 3 ? NUMBERFORMAT_DATETIME : NUMBERFORMAT_DATE);
                    sal_uLong nFormat = pDocFormatter->GetStandardFormat( nType, eDocLang );
                    // maybe there is a special format including seconds or milliseconds
                    if (nFound > 5)
                        nFormat = pDocFormatter->GetStandardFormat( fDays, nFormat, nType, eDocLang);

                    ScAddress aPos(nCol,nRow,nTab);
                    pDoc->SetValue(aPos, fDays);
                    pDoc->SetNumberFormat(aPos, nFormat);

                    return bMultiLine;     // success
                }
            }
        }
    }

    // Standard or date not determined -> SetString / EditCell
    if( rStr.Search( '\n' ) == STRING_NOTFOUND )
    {
        ScSetStringParam aParam;
        aParam.mpNumFormatter = pFormatter;
        aParam.mbDetectNumberFormat = bDetectNumFormat;
        aParam.meSetTextNumFormat = ScSetStringParam::SpecialNumberOnly;
        aParam.mbHandleApostrophe = false;
        pDoc->SetString( nCol, nRow, nTab, rStr, &aParam );
    }
    else
    {
        bMultiLine = true;
        ScFieldEditEngine& rEngine = pDoc->GetEditEngine();
        rEngine.SetText(rStr);
        pDoc->SetEditText(ScAddress(nCol,nRow,nTab), rEngine.CreateTextObject());
    }
    return bMultiLine;
}


static String lcl_GetFixed( const OUString& rLine, sal_Int32 nStart, sal_Int32 nNext,
                     bool& rbIsQuoted, bool& rbOverflowCell )
{
    sal_Int32 nLen = rLine.getLength();
    if (nNext > nLen)
        nNext = nLen;
    if ( nNext <= nStart )
        return EMPTY_STRING;

    const sal_Unicode* pStr = rLine.getStr();

    sal_Int32 nSpace = nNext;
    while ( nSpace > nStart && pStr[nSpace-1] == ' ' )
        --nSpace;

    rbIsQuoted = (pStr[nStart] == sal_Unicode('"') && pStr[nSpace-1] == sal_Unicode('"'));
    if (rbIsQuoted)
    {
        bool bFits = (nSpace - nStart - 3 <= STRING_MAXLEN);
        OSL_ENSURE( bFits, "lcl_GetFixed: line doesn't fit into data");
        if (bFits)
            return rLine.copy(nStart+1, nSpace-nStart-2);
        else
        {
            rbOverflowCell = true;
            return rLine.copy(nStart+1, STRING_MAXLEN);
        }
    }
    else
    {
        bool bFits = (nSpace - nStart <= STRING_MAXLEN);
        OSL_ENSURE( bFits, "lcl_GetFixed: line doesn't fit into data");
        if (bFits)
            return rLine.copy(nStart, nSpace-nStart);
        else
        {
            rbOverflowCell = true;
            return rLine.copy(nStart, STRING_MAXLEN);
        }
    }
}

bool ScImportExport::ExtText2Doc( SvStream& rStrm )
{
    if (!pExtOptions)
        return Text2Doc( rStrm );

    sal_uLong nOldPos = rStrm.Tell();
    rStrm.Seek( STREAM_SEEK_TO_END );
    ::std::auto_ptr<ScProgress> xProgress( new ScProgress( pDocSh,
            ScGlobal::GetRscString( STR_LOAD_DOC ), rStrm.Tell() - nOldPos ));
    rStrm.Seek( nOldPos );
    rStrm.StartReadingUnicodeText( rStrm.GetStreamCharSet() );

    SCCOL nStartCol = aRange.aStart.Col();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCTAB nTab = aRange.aStart.Tab();

    bool    bFixed              = pExtOptions->IsFixedLen();
    const String& rSeps         = pExtOptions->GetFieldSeps();
    const sal_Unicode* pSeps    = rSeps.GetBuffer();
    bool    bMerge              = pExtOptions->IsMergeSeps();
    sal_uInt16  nInfoCount      = pExtOptions->GetInfoCount();
    const sal_Int32* pColStart  = pExtOptions->GetColStart();
    const sal_uInt8* pColFormat = pExtOptions->GetColFormat();
    long nSkipLines             = pExtOptions->GetStartRow();

    LanguageType eDocLang = pExtOptions->GetLanguage();
    SvNumberFormatter aNumFormatter( comphelper::getProcessComponentContext(), eDocLang);
    bool bDetectNumFormat = pExtOptions->IsDetectSpecialNumber();

    // For date recognition
    ::utl::TransliterationWrapper aTransliteration(
        comphelper::getProcessComponentContext(), SC_TRANSLITERATION_IGNORECASE );
    aTransliteration.loadModuleIfNeeded( eDocLang );
    CalendarWrapper aCalendar( comphelper::getProcessComponentContext() );
    aCalendar.loadDefaultCalendar(
        LanguageTag::convertToLocale( eDocLang ) );
    boost::scoped_ptr< ::utl::TransliterationWrapper > pEnglishTransliteration;
    boost::scoped_ptr< CalendarWrapper > pEnglishCalendar;
    if ( eDocLang != LANGUAGE_ENGLISH_US )
    {
        pEnglishTransliteration.reset(new ::utl::TransliterationWrapper (
            comphelper::getProcessComponentContext(), SC_TRANSLITERATION_IGNORECASE ));
        aTransliteration.loadModuleIfNeeded( LANGUAGE_ENGLISH_US );
        pEnglishCalendar.reset(new CalendarWrapper ( comphelper::getProcessComponentContext() ));
        pEnglishCalendar->loadDefaultCalendar(
            LanguageTag::convertToLocale( LANGUAGE_ENGLISH_US ) );
    }

    OUString aLine;
    String aCell;
    sal_uInt16 i;
    SCROW nRow = nStartRow;

    while(--nSkipLines>0)
    {
        aLine = ReadCsvLine(rStrm, !bFixed, rSeps, cStr); // content is ignored
        if ( rStrm.IsEof() )
            break;
    }

    // Determine range for Undo.
    // TODO: we don't need this during import of a file to a new sheet or
    // document, could set bDetermineRange=false then.
    bool bDetermineRange = true;

    // Row heights don't need to be adjusted on the fly if EndPaste() is called
    // afterwards, which happens only if bDetermineRange. This variable also
    // survives the toggle of bDetermineRange down at the end of the do{} loop.
    bool bRangeIsDetermined = bDetermineRange;

    bool bQuotedAsText = pExtOptions && pExtOptions->IsQuotedAsText();

    sal_uLong nOriginalStreamPos = rStrm.Tell();

    do
    {
        for( ;; )
        {
            aLine = ReadCsvLine(rStrm, !bFixed, rSeps, cStr);
            if ( rStrm.IsEof() && aLine.isEmpty() )
                break;

            EmbeddedNullTreatment( aLine);

            sal_Int32 nLineLen = aLine.getLength();
            SCCOL nCol = nStartCol;
            bool bMultiLine = false;
            if ( bFixed )               //  Feste Satzlaenge
            {
                // Yes, the check is nCol<=MAXCOL+1, +1 because it is only an
                // overflow if there is really data following to be put behind
                // the last column, which doesn't happen if info is
                // SC_COL_SKIP.
                for ( i=0; i<nInfoCount && nCol <= MAXCOL+1; i++ )
                {
                    sal_uInt8 nFmt = pColFormat[i];
                    if (nFmt != SC_COL_SKIP)        // sonst auch nCol nicht hochzaehlen
                    {
                        if (nCol > MAXCOL)
                            bOverflowCol = true;    // display warning on import
                        else if (!bDetermineRange)
                        {
                            sal_Int32 nStart = pColStart[i];
                            sal_Int32 nNext = ( i+1 < nInfoCount ) ? pColStart[i+1] : nLineLen;
                            bool bIsQuoted = false;
                            aCell = lcl_GetFixed( aLine, nStart, nNext, bIsQuoted, bOverflowCell );
                            if (bIsQuoted && bQuotedAsText)
                                nFmt = SC_COL_TEXT;

                            bMultiLine |= lcl_PutString(
                                pDoc, nCol, nRow, nTab, aCell, nFmt,
                                &aNumFormatter, bDetectNumFormat, aTransliteration, aCalendar,
                                pEnglishTransliteration.get(), pEnglishCalendar.get());
                        }
                        ++nCol;
                    }
                }
            }
            else                        //  Nach Trennzeichen suchen
            {
                SCCOL nSourceCol = 0;
                sal_uInt16 nInfoStart = 0;
                const sal_Unicode* p = aLine.getStr();
                // Yes, the check is nCol<=MAXCOL+1, +1 because it is only an
                // overflow if there is really data following to be put behind
                // the last column, which doesn't happen if info is
                // SC_COL_SKIP.
                while (*p && nCol <= MAXCOL+1)
                {
                    bool bIsQuoted = false;
                    p = ScImportExport::ScanNextFieldFromString( p, aCell,
                            cStr, pSeps, bMerge, bIsQuoted, bOverflowCell );

                    sal_uInt8 nFmt = SC_COL_STANDARD;
                    for ( i=nInfoStart; i<nInfoCount; i++ )
                    {
                        if ( pColStart[i] == nSourceCol + 1 )       // pColStart ist 1-basiert
                        {
                            nFmt = pColFormat[i];
                            nInfoStart = i + 1;     // ColInfos sind in Reihenfolge
                            break;  // for
                        }
                    }
                    if ( nFmt != SC_COL_SKIP )
                    {
                        if (nCol > MAXCOL)
                            bOverflowCol = true;    // display warning on import
                        else if (!bDetermineRange)
                        {
                            if (bIsQuoted && bQuotedAsText)
                                nFmt = SC_COL_TEXT;

                            bMultiLine |= lcl_PutString(
                                pDoc, nCol, nRow, nTab, aCell, nFmt,
                                &aNumFormatter, bDetectNumFormat, aTransliteration,
                                aCalendar, pEnglishTransliteration.get(), pEnglishCalendar.get());
                        }
                        ++nCol;
                    }

                    ++nSourceCol;
                }
            }
            if (nEndCol < nCol)
                nEndCol = nCol;     //! points to the next free or even MAXCOL+2

            if (!bDetermineRange)
            {
                if (bMultiLine && !bRangeIsDetermined && pDocSh)
                    pDocSh->AdjustRowHeight( nRow, nRow, nTab);
                xProgress->SetStateOnPercent( rStrm.Tell() - nOldPos );
            }
            ++nRow;
            if ( nRow > MAXROW )
            {
                bOverflowRow = true;    // display warning on import
                break;  // for
            }
        }
        // so far nRow/nEndCol pointed to the next free
        if (nRow > nStartRow)
            --nRow;
        if (nEndCol > nStartCol)
            nEndCol = ::std::min( static_cast<SCCOL>(nEndCol - 1), MAXCOL);

        if (bDetermineRange)
        {
            aRange.aEnd.SetCol( nEndCol );
            aRange.aEnd.SetRow( nRow );

            if ( !mbApi && nStartCol != nEndCol &&
                 !pDoc->IsBlockEmpty( nTab, nStartCol + 1, nStartRow, nEndCol, nRow ) )
            {
                ScReplaceWarnBox aBox( pDocSh->GetActiveDialogParent() );
                if ( aBox.Execute() != RET_YES )
                {
                    return false;
                }
            }

            rStrm.Seek( nOriginalStreamPos );
            nRow = nStartRow;
            if (!StartPaste())
            {
                EndPaste();
                return false;
            }
        }

        bDetermineRange = !bDetermineRange;     // toggle
    } while (!bDetermineRange);

    xProgress.reset();    // make room for AdjustRowHeight progress
    if (bRangeIsDetermined)
        EndPaste();

    return true;
}


void ScImportExport::EmbeddedNullTreatment( OUString & rStr )
{
    // A nasty workaround for data with embedded NULL characters. As long as we
    // can't handle them properly as cell content (things assume 0-terminated
    // strings at too many places) simply strip all NULL characters from raw
    // data. Excel does the same. See fdo#57841 for sample data.

    // The normal case is no embedded NULL, check first before de-/allocating
    // ustring stuff.
    sal_Unicode cNull = 0;
    if (rStr.indexOf( cNull) >= 0)
    {
        rStr = rStr.replaceAll( OUString( &cNull, 1), OUString());
    }
}


const sal_Unicode* ScImportExport::ScanNextFieldFromString( const sal_Unicode* p,
        String& rField, sal_Unicode cStr, const sal_Unicode* pSeps, bool bMergeSeps, bool& rbIsQuoted,
        bool& rbOverflowCell )
{
    rbIsQuoted = false;
    rField.Erase();
    const sal_Unicode cBlank = ' ';
    if (!ScGlobal::UnicodeStrChr( pSeps, cBlank))
    {
        // Cope with broken generators that put leading blanks before a quoted
        // field, like "field1", "field2", "..."
        // NOTE: this is not in conformance with http://tools.ietf.org/html/rfc4180
        const sal_Unicode* pb = p;
        while (*pb == cBlank)
            ++pb;
        if (*pb == cStr)
            p = pb;
    }
    if ( *p == cStr )           // String in quotes
    {
        rbIsQuoted = true;
        const sal_Unicode* p1;
        p1 = p = lcl_ScanString( p, rField, pSeps, cStr, DQM_ESCAPE, rbOverflowCell );
        while ( *p && !ScGlobal::UnicodeStrChr( pSeps, *p ) )
            p++;
        // Append remaining unquoted and undelimited data (dirty, dirty) to
        // this field.
        if (p > p1)
        {
            if (!lcl_appendLineData( rField, p1, p))
                rbOverflowCell = true;
        }
        if( *p )
            p++;
    }
    else                        // up to delimiter
    {
        const sal_Unicode* p0 = p;
        while ( *p && !ScGlobal::UnicodeStrChr( pSeps, *p ) )
            p++;
        if (!lcl_appendLineData( rField, p0, p))
            rbOverflowCell = true;
        if( *p )
            p++;
    }
    if ( bMergeSeps )           // skip following delimiters
    {
        while ( *p && ScGlobal::UnicodeStrChr( pSeps, *p ) )
            p++;
    }
    return p;
}

namespace {

/**
 * Check if a given string has any line break characters or separators.
 *
 * @param rStr string to inspect.
 * @param cSep separator character.
 */
bool hasLineBreaksOrSeps( const String& rStr, sal_Unicode cSep )
{
    const sal_Unicode* p = rStr.GetBuffer();
    for (xub_StrLen i = 0, n = rStr.Len(); i < n; ++i, ++p)
    {
        sal_Unicode c = *p;
        if (c == cSep)
            // separator found.
            return true;

        switch (c)
        {
            case '\n':
            case '\r':
                // line break found.
                return true;
            default:
                ;
        }
    }
    return false;
}

}

bool ScImportExport::Doc2Text( SvStream& rStrm )
{
    SCCOL nCol;
    SCROW nRow;
    SCCOL nStartCol = aRange.aStart.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCTAB nStartTab = aRange.aStart.Tab();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nEndRow = aRange.aEnd.Row();
    SCTAB nEndTab = aRange.aEnd.Tab();

    if (!pDoc->GetClipParam().isMultiRange() && nStartTab == nEndTab)
        pDoc->ShrinkToDataArea( nStartTab, nStartCol, nStartRow, nEndCol, nEndRow );

    String aCell;

    bool bConvertLF = (GetSystemLineEnd() != LINEEND_LF);

    for (nRow = nStartRow; nRow <= nEndRow; nRow++)
    {
        if (bIncludeFiltered || !pDoc->RowFiltered( nRow, nStartTab ))
        {
            for (nCol = nStartCol; nCol <= nEndCol; nCol++)
            {
                CellType eType;
                pDoc->GetCellType( nCol, nRow, nStartTab, eType );
                switch (eType)
                {
                    case CELLTYPE_FORMULA:
                    {
                        if (bFormulas)
                        {
                            pDoc->GetFormula( nCol, nRow, nStartTab, aCell );
                            if( aCell.Search( cSep ) != STRING_NOTFOUND )
                                lcl_WriteString( rStrm, aCell, cStr, cStr );
                            else
                                lcl_WriteSimpleString( rStrm, aCell );
                        }
                        else
                        {
                            aCell = pDoc->GetString(nCol, nRow, nStartTab);

                            bool bMultiLineText = ( aCell.Search( '\n' ) != STRING_NOTFOUND );
                            if( bMultiLineText )
                            {
                                if( mExportTextOptions.meNewlineConversion == ScExportTextOptions::ToSpace )
                                    aCell.SearchAndReplaceAll( '\n', ' ' );
                                else if ( mExportTextOptions.meNewlineConversion == ScExportTextOptions::ToSystem && bConvertLF )
                                    aCell = convertLineEnd(aCell, GetSystemLineEnd());
                            }

                            if( mExportTextOptions.mcSeparatorConvertTo && cSep )
                                aCell.SearchAndReplaceAll( cSep, mExportTextOptions.mcSeparatorConvertTo );

                            if( mExportTextOptions.mbAddQuotes && ( aCell.Search( cSep ) != STRING_NOTFOUND ) )
                                lcl_WriteString( rStrm, aCell, cStr, cStr );
                            else
                                lcl_WriteSimpleString( rStrm, aCell );
                        }
                    }
                    break;
                    case CELLTYPE_VALUE:
                    {
                        aCell = pDoc->GetString(nCol, nRow, nStartTab);
                        lcl_WriteSimpleString( rStrm, aCell );
                    }
                    break;
                    case CELLTYPE_NONE:
                    break;
                    default:
                    {
                        aCell = pDoc->GetString(nCol, nRow, nStartTab);

                        bool bMultiLineText = ( aCell.Search( '\n' ) != STRING_NOTFOUND );
                        if( bMultiLineText )
                        {
                            if( mExportTextOptions.meNewlineConversion == ScExportTextOptions::ToSpace )
                                aCell.SearchAndReplaceAll( '\n', ' ' );
                            else if ( mExportTextOptions.meNewlineConversion == ScExportTextOptions::ToSystem && bConvertLF )
                                aCell = convertLineEnd(aCell, GetSystemLineEnd());
                        }

                        if( mExportTextOptions.mcSeparatorConvertTo && cSep )
                            aCell.SearchAndReplaceAll( cSep, mExportTextOptions.mcSeparatorConvertTo );

                        if( mExportTextOptions.mbAddQuotes && hasLineBreaksOrSeps(aCell, cSep) )
                            lcl_WriteString( rStrm, aCell, cStr, cStr );
                        else
                            lcl_WriteSimpleString( rStrm, aCell );
                    }
                }
                if( nCol < nEndCol )
                    lcl_WriteSimpleString( rStrm, OUString(cSep) );
            }
                WriteUnicodeOrByteEndl( rStrm );
            if( rStrm.GetError() != SVSTREAM_OK )
                break;
            if( nSizeLimit && rStrm.Tell() > nSizeLimit )
                break;
        }
    }

    return rStrm.GetError() == SVSTREAM_OK;
}


bool ScImportExport::Sylk2Doc( SvStream& rStrm )
{
    bool bOk = true;
    bool bMyDoc = false;
    SylkVersion eVersion = SYLK_OTHER;

    // US-English separators for StringToDouble
    sal_Unicode cDecSep = '.';
    sal_Unicode cGrpSep = ',';

    SCCOL nStartCol = aRange.aStart.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nEndRow = aRange.aEnd.Row();
    sal_uLong nOldPos = rStrm.Tell();
    bool bData = !bSingle;
    ::std::vector< sal_uInt32 > aFormats;

    if( !bSingle)
        bOk = StartPaste();

    while( bOk )
    {
        String aLine;
        String aText;
        OString aByteLine;
        SCCOL nCol = nStartCol;
        SCROW nRow = nStartRow;
        SCCOL nRefCol = 1;
        SCROW nRefRow = 1;
        rStrm.Seek( nOldPos );
        for( ;; )
        {
            //! allow unicode
            rStrm.ReadLine( aByteLine );
            aLine = OStringToOUString(aByteLine, rStrm.GetStreamCharSet());
            if( rStrm.IsEof() )
                break;
            const sal_Unicode* p = aLine.GetBuffer();
            sal_Unicode cTag = *p++;
            if( cTag == 'C' )       // Content
            {
                if( *p++ != ';' )
                    return false;
                while( *p )
                {
                    sal_Unicode ch = *p++;
                    ch = ScGlobal::ToUpperAlpha( ch );
                    switch( ch )
                    {
                        case 'X':
                            nCol = static_cast<SCCOL>(OUString(p).toInt32()) + nStartCol - 1;
                            break;
                        case 'Y':
                            nRow = OUString(p).toInt32() + nStartRow - 1;
                            break;
                        case 'C':
                            nRefCol = static_cast<SCCOL>(OUString(p).toInt32()) + nStartCol - 1;
                            break;
                        case 'R':
                            nRefRow = OUString(p).toInt32() + nStartRow - 1;
                            break;
                        case 'K':
                        {
                            if( !bSingle &&
                                    ( nCol < nStartCol || nCol > nEndCol
                                      || nRow < nStartRow || nRow > nEndRow
                                      || nCol > MAXCOL || nRow > MAXROW ) )
                                break;
                            if( !bData )
                            {
                                if( nRow > nEndRow )
                                    nEndRow = nRow;
                                if( nCol > nEndCol )
                                    nEndCol = nCol;
                                break;
                            }
                            bool bText;
                            if( *p == '"' )
                            {
                                bText = true;
                                aText.Erase();
                                p = lcl_ScanSylkString( p, aText, eVersion);
                            }
                            else
                                bText = false;
                            const sal_Unicode* q = p;
                            while( *q && *q != ';' )
                                q++;
                            if ( !(*q == ';' && *(q+1) == 'I') )
                            {   // don't ignore value
                                if( bText )
                                {
                                    pDoc->EnsureTable(aRange.aStart.Tab());
                                    pDoc->SetTextCell(
                                        ScAddress(nCol, nRow, aRange.aStart.Tab()), aText);
                                }
                                else
                                {
                                    double fVal = rtl_math_uStringToDouble( p,
                                            aLine.GetBuffer() + aLine.Len(),
                                            cDecSep, cGrpSep, NULL, NULL );
                                    pDoc->SetValue( nCol, nRow, aRange.aStart.Tab(), fVal );
                                }
                            }
                        }
                        break;
                        case 'E':
                        case 'M':
                        {
                            if ( ch == 'M' )
                            {
                                if ( nRefCol < nCol )
                                    nRefCol = nCol;
                                if ( nRefRow < nRow )
                                    nRefRow = nRow;
                                if ( !bData )
                                {
                                    if( nRefRow > nEndRow )
                                        nEndRow = nRefRow;
                                    if( nRefCol > nEndCol )
                                        nEndCol = nRefCol;
                                }
                            }
                            if( !bMyDoc || !bData )
                                break;
                            aText = '=';
                            p = lcl_ScanSylkFormula( p, aText, eVersion);
                            ScAddress aPos( nCol, nRow, aRange.aStart.Tab() );
                            /* FIXME: do we want GRAM_ODFF_A1 instead? At the
                             * end it probably should be GRAM_ODFF_R1C1, since
                             * R1C1 is what Excel writes in SYLK. */
                            const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_PODF_A1;
                            ScCompiler aComp( pDoc, aPos);
                            aComp.SetGrammar(eGrammar);
                            ScTokenArray* pCode = aComp.CompileString( aText );
                            if ( ch == 'M' )
                            {
                                ScMarkData aMark;
                                aMark.SelectTable( aPos.Tab(), true );
                                pDoc->InsertMatrixFormula( nCol, nRow, nRefCol,
                                    nRefRow, aMark, EMPTY_STRING, pCode );
                            }
                            else
                            {
                                ScFormulaCell* pFCell = new ScFormulaCell(
                                        pDoc, aPos, pCode, eGrammar, MM_NONE);
                                pDoc->SetFormulaCell(aPos, pFCell);
                            }
                            delete pCode;   // ctor/InsertMatrixFormula did copy TokenArray
                        }
                        break;
                    }
                    while( *p && *p != ';' )
                        p++;
                    if( *p )
                        p++;
                }
            }
            else if( cTag == 'F' )      // Format
            {
                if( *p++ != ';' )
                    return false;
                sal_Int32 nFormat = -1;
                while( *p )
                {
                    sal_Unicode ch = *p++;
                    ch = ScGlobal::ToUpperAlpha( ch );
                    switch( ch )
                    {
                        case 'X':
                            nCol = static_cast<SCCOL>(OUString(p).toInt32()) + nStartCol - 1;
                            break;
                        case 'Y':
                            nRow = OUString(p).toInt32() + nStartRow - 1;
                            break;
                        case 'P' :
                            if ( bData )
                            {
                                // F;P<n> sets format code of P;P<code> at
                                // current position, or at ;X;Y if specified.
                                // Note that ;X;Y may appear after ;P
                                const sal_Unicode* p0 = p;
                                while( *p && *p != ';' )
                                    p++;
                                OUString aNumber(p0, p - p0);
                                nFormat = aNumber.toInt32();
                            }
                            break;
                    }
                    while( *p && *p != ';' )
                        p++;
                    if( *p )
                        p++;
                }
                if ( !bData )
                {
                    if( nRow > nEndRow )
                        nEndRow = nRow;
                    if( nCol > nEndCol )
                        nEndCol = nCol;
                }
                if ( 0 <= nFormat && nFormat < (sal_Int32)aFormats.size() )
                {
                    sal_uInt32 nKey = aFormats[nFormat];
                    pDoc->ApplyAttr( nCol, nRow, aRange.aStart.Tab(),
                            SfxUInt32Item( ATTR_VALUE_FORMAT, nKey ) );
                }
            }
            else if( cTag == 'P' )
            {
                if ( bData && *p == ';' && *(p+1) == 'P' )
                {
                    OUString aCode( p+2 );
                    // unescape doubled semicolons
                    aCode = aCode.replaceAll(";;", ";");
                    // get rid of Xcl escape characters
                    aCode = aCode.replaceAll(OUString(static_cast<sal_Unicode>(0x1b)), OUString());
                    sal_Int32 nCheckPos;
                    short nType;
                    sal_uInt32 nKey;
                    pDoc->GetFormatTable()->PutandConvertEntry( aCode, nCheckPos, nType, nKey,
                                                                LANGUAGE_ENGLISH_US, ScGlobal::eLnge );
                    if ( nCheckPos )
                        nKey = 0;
                    aFormats.push_back( nKey );
                }
            }
            else if( cTag == 'I' && *p == 'D' )
            {
                aLine.Erase( 0, 4 );
                if (aLine.EqualsAscii( "CALCOOO32" ))
                    eVersion = SYLK_OOO32;
                else if (aLine.EqualsAscii( "SCALC3" ))
                    eVersion = SYLK_SCALC3;
                bMyDoc = (eVersion <= SYLK_OWN);
            }
            else if( cTag == 'E' )                      // Ende
                break;
        }
        if( !bData )
        {
            aRange.aEnd.SetCol( nEndCol );
            aRange.aEnd.SetRow( nEndRow );
            bOk = StartPaste();
            bData = true;
        }
        else
            break;
    }

    EndPaste();
    return bOk;
}


bool ScImportExport::Doc2Sylk( SvStream& rStrm )
{
    SCCOL nCol;
    SCROW nRow;
    SCCOL nStartCol = aRange.aStart.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nEndRow = aRange.aEnd.Row();
    String aCellStr;
    String aValStr;
    lcl_WriteSimpleString( rStrm,
            String( RTL_CONSTASCII_USTRINGPARAM( "ID;PCALCOOO32")));
    WriteUnicodeOrByteEndl( rStrm );

    for (nRow = nStartRow; nRow <= nEndRow; nRow++)
    {
        for (nCol = nStartCol; nCol <= nEndCol; nCol++)
        {
            String aBufStr;
            double nVal;
            bool bForm = false;
            SCROW r = nRow - nStartRow + 1;
            SCCOL c = nCol - nStartCol + 1;
            ScRefCellValue aCell;
            aCell.assign(*pDoc, ScAddress(nCol, nRow, aRange.aStart.Tab()));
            CellType eType = aCell.meType;
            switch( eType )
            {
                case CELLTYPE_FORMULA:
                    bForm = bFormulas;
                    if( pDoc->HasValueData( nCol, nRow, aRange.aStart.Tab()) )
                        goto hasvalue;
                    else
                        goto hasstring;

                case CELLTYPE_VALUE:
                hasvalue:
                    pDoc->GetValue( nCol, nRow, aRange.aStart.Tab(), nVal );

                    aValStr = ::rtl::math::doubleToUString( nVal,
                            rtl_math_StringFormat_Automatic,
                            rtl_math_DecimalPlaces_Max, '.', true );

                    aBufStr.AssignAscii(RTL_CONSTASCII_STRINGPARAM( "C;X" ));
                    aBufStr += OUString::number( c );
                    aBufStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ";Y" ));
                    aBufStr += OUString::number( r );
                    aBufStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ";K" ));
                    aBufStr += aValStr;
                    lcl_WriteSimpleString( rStrm, aBufStr );
                    goto checkformula;

                case CELLTYPE_STRING:
                case CELLTYPE_EDIT:
                hasstring:
                    aCellStr = pDoc->GetString(nCol, nRow, aRange.aStart.Tab());
                    aCellStr.SearchAndReplaceAll( OUString('\n'), OUString(SYLK_LF) );

                    aBufStr.AssignAscii(RTL_CONSTASCII_STRINGPARAM( "C;X" ));
                    aBufStr += OUString::number( c );
                    aBufStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ";Y" ));
                    aBufStr += OUString::number( r );
                    aBufStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ";K" ));
                    lcl_WriteSimpleString( rStrm, aBufStr );
                    lcl_WriteString( rStrm, aCellStr, '"', ';' );

                checkformula:
                    if( bForm )
                    {
                        const ScFormulaCell* pFCell = aCell.mpFormula;
                        switch ( pFCell->GetMatrixFlag() )
                        {
                            case MM_REFERENCE :
                                aCellStr.Erase();
                            break;
                            default:
                                OUString aOUCellStr;
                                pFCell->GetFormula( aOUCellStr,formula::FormulaGrammar::GRAM_PODF_A1);
                                aCellStr = aOUCellStr;
                                /* FIXME: do we want GRAM_ODFF_A1 instead? At
                                 * the end it probably should be
                                 * GRAM_ODFF_R1C1, since R1C1 is what Excel
                                 * writes in SYLK. */
                        }
                        if ( pFCell->GetMatrixFlag() != MM_NONE &&
                                aCellStr.Len() > 2 &&
                                aCellStr.GetChar(0) == '{' &&
                                aCellStr.GetChar(aCellStr.Len()-1) == '}' )
                        {   // cut off matrix {} characters
                            aCellStr.Erase(aCellStr.Len()-1,1);
                            aCellStr.Erase(0,1);
                        }
                        if ( aCellStr.GetChar(0) == '=' )
                            aCellStr.Erase(0,1);
                        String aPrefix;
                        switch ( pFCell->GetMatrixFlag() )
                        {
                            case MM_FORMULA :
                            {   // diff expression with 'M' M$-extension
                                SCCOL nC;
                                SCROW nR;
                                pFCell->GetMatColsRows( nC, nR );
                                nC += c - 1;
                                nR += r - 1;
                                aPrefix.AssignAscii( RTL_CONSTASCII_STRINGPARAM( ";R" ) );
                                aPrefix += OUString::number( nR );
                                aPrefix.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ";C" ) );
                                aPrefix += OUString::number( nC );
                                aPrefix.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ";M" ) );
                            }
                            break;
                            case MM_REFERENCE :
                            {   // diff expression with 'I' M$-extension
                                ScAddress aPos;
                                pFCell->GetMatrixOrigin( aPos );
                                aPrefix.AssignAscii( RTL_CONSTASCII_STRINGPARAM( ";I;R" ) );
                                aPrefix += OUString::number( aPos.Row() - nStartRow + 1 );
                                aPrefix.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ";C" ) );
                                aPrefix += OUString::number( aPos.Col() - nStartCol + 1 );
                            }
                            break;
                            default:
                                // formula Expression
                                aPrefix.AssignAscii( RTL_CONSTASCII_STRINGPARAM( ";E" ) );
                        }
                        lcl_WriteSimpleString( rStrm, aPrefix );
                        if ( aCellStr.Len() )
                            lcl_WriteString( rStrm, aCellStr, 0, ';' );
                    }
                    WriteUnicodeOrByteEndl( rStrm );
                    break;

                default:
                {
                    // added to avoid warnings
                }
            }
        }
    }
    lcl_WriteSimpleString( rStrm, OUString( 'E' ) );
    WriteUnicodeOrByteEndl( rStrm );
    return rStrm.GetError() == SVSTREAM_OK;
}


bool ScImportExport::Doc2HTML( SvStream& rStrm, const String& rBaseURL )
{
    // CharSet is ignored in ScExportHTML, read from Load/Save HTML options
    ScFormatFilter::Get().ScExportHTML( rStrm, rBaseURL, pDoc, aRange, RTL_TEXTENCODING_DONTKNOW, bAll,
        aStreamPath, aNonConvertibleChars );
    return rStrm.GetError() == SVSTREAM_OK;
}

bool ScImportExport::Doc2RTF( SvStream& rStrm )
{
    //  CharSet is ignored in ScExportRTF
    ScFormatFilter::Get().ScExportRTF( rStrm, pDoc, aRange, RTL_TEXTENCODING_DONTKNOW );
    return rStrm.GetError() == SVSTREAM_OK;
}


bool ScImportExport::Doc2Dif( SvStream& rStrm )
{
    // for DIF in the clipboard, IBM_850 is always used
    ScFormatFilter::Get().ScExportDif( rStrm, pDoc, aRange, RTL_TEXTENCODING_IBM_850 );
    return true;
}


bool ScImportExport::Dif2Doc( SvStream& rStrm )
{
    SCTAB nTab = aRange.aStart.Tab();
    ScDocument* pImportDoc = new ScDocument( SCDOCMODE_UNDO );
    pImportDoc->InitUndo( pDoc, nTab, nTab );

    // for DIF in the clipboard, IBM_850 is always used
    ScFormatFilter::Get().ScImportDif( rStrm, pImportDoc, aRange.aStart, RTL_TEXTENCODING_IBM_850 );

    SCCOL nEndCol;
    SCROW nEndRow;
    pImportDoc->GetCellArea( nTab, nEndCol, nEndRow );
    // if there are no cells in the imported content, nEndCol/nEndRow may be before the start
    if ( nEndCol < aRange.aStart.Col() )
        nEndCol = aRange.aStart.Col();
    if ( nEndRow < aRange.aStart.Row() )
        nEndRow = aRange.aStart.Row();
    aRange.aEnd = ScAddress( nEndCol, nEndRow, nTab );

    bool bOk = StartPaste();
    if (bOk)
    {
        sal_uInt16 nFlags = IDF_ALL & ~IDF_STYLES;
        pDoc->DeleteAreaTab( aRange, nFlags );
        pImportDoc->CopyToDocument( aRange, nFlags, false, pDoc );
        EndPaste();
    }

    delete pImportDoc;

    return bOk;
}


bool ScImportExport::RTF2Doc( SvStream& rStrm, const String& rBaseURL )
{
    ScEEAbsImport *pImp = ScFormatFilter::Get().CreateRTFImport( pDoc, aRange );
    if (!pImp)
        return false;
    pImp->Read( rStrm, rBaseURL );
    aRange = pImp->GetRange();

    bool bOk = StartPaste();
    if (bOk)
    {
        sal_uInt16 nFlags = IDF_ALL & ~IDF_STYLES;
        pDoc->DeleteAreaTab( aRange, nFlags );
        pImp->WriteToDocument();
        EndPaste();
    }
    delete pImp;
    return bOk;
}


bool ScImportExport::HTML2Doc( SvStream& rStrm, const String& rBaseURL )
{
    ScEEAbsImport *pImp = ScFormatFilter::Get().CreateHTMLImport( pDoc, rBaseURL, aRange, true);
    if (!pImp)
        return false;
    pImp->Read( rStrm, rBaseURL );
    aRange = pImp->GetRange();

    bool bOk = StartPaste();
    if (bOk)
    {
        // ScHTMLImport may call ScDocument::InitDrawLayer, resulting in
        // a Draw Layer but no Draw View -> create Draw Layer and View here
        if (pDocSh)
            pDocSh->MakeDrawLayer();

        sal_uInt16 nFlags = IDF_ALL & ~IDF_STYLES;
        pDoc->DeleteAreaTab( aRange, nFlags );

        if (pExtOptions)
        {
            // Pick up import options if available.
            LanguageType eLang = pExtOptions->GetLanguage();
            SvNumberFormatter aNumFormatter( comphelper::getProcessComponentContext(), eLang);
            bool bSpecialNumber = pExtOptions->IsDetectSpecialNumber();
            pImp->WriteToDocument(false, 1.0, &aNumFormatter, bSpecialNumber);
        }
        else
            // Regular import, with no options.
            pImp->WriteToDocument();

        EndPaste();
    }
    delete pImp;
    return bOk;
}

#ifndef DISABLE_DYNLOADING

#define RETURN_ERROR { return eERR_INTERN; }
class ScFormatFilterMissing : public ScFormatFilterPlugin {
  public:
    ScFormatFilterMissing()
    {
      OSL_FAIL("Missing file filters");
    }
    virtual ~ScFormatFilterMissing() {}
    virtual FltError ScImportLotus123( SfxMedium&, ScDocument*, CharSet ) RETURN_ERROR
    virtual FltError ScImportQuattroPro( SfxMedium &, ScDocument * ) RETURN_ERROR
    virtual FltError ScImportExcel( SfxMedium&, ScDocument*, const EXCIMPFORMAT ) RETURN_ERROR
    virtual FltError ScImportStarCalc10( SvStream&, ScDocument* ) RETURN_ERROR
    virtual FltError ScImportDif( SvStream&, ScDocument*, const ScAddress&,
                 const CharSet, sal_uInt32 ) RETURN_ERROR
    virtual FltError ScImportRTF( SvStream&, const String&, ScDocument*, ScRange& ) RETURN_ERROR
    virtual FltError ScImportHTML( SvStream&, const String&, ScDocument*, ScRange&, double, bool, SvNumberFormatter*, bool ) RETURN_ERROR

    virtual ScEEAbsImport *CreateRTFImport( ScDocument*, const ScRange& ) { return NULL; }
    virtual ScEEAbsImport *CreateHTMLImport( ScDocument*, const String&, const ScRange&, bool ) { return NULL; }
    virtual String         GetHTMLRangeNameList( ScDocument*, const String& ) { return String(); }

    virtual FltError ScExportExcel5( SfxMedium&, ScDocument*, ExportFormatExcel, CharSet ) RETURN_ERROR
    virtual FltError ScExportDif( SvStream&, ScDocument*, const ScAddress&, const CharSet, sal_uInt32 ) RETURN_ERROR
    virtual FltError ScExportDif( SvStream&, ScDocument*, const ScRange&, const CharSet, sal_uInt32 ) RETURN_ERROR
    virtual FltError ScExportHTML( SvStream&, const String&, ScDocument*, const ScRange&, const CharSet, bool,
                  const String&, String& ) RETURN_ERROR
    virtual FltError ScExportRTF( SvStream&, ScDocument*, const ScRange&, const CharSet ) RETURN_ERROR

    virtual ScOrcusFilters* GetOrcusFilters() { return NULL; }
};

extern "C" { static void SAL_CALL thisModule() {} }

#else

extern "C" {
ScFormatFilterPlugin* ScFilterCreate();
}

#endif

typedef ScFormatFilterPlugin * (*FilterFn)(void);
ScFormatFilterPlugin &ScFormatFilter::Get()
{
    static ScFormatFilterPlugin *plugin;

    if (plugin != NULL)
        return *plugin;

#ifndef DISABLE_DYNLOADING
    OUString sFilterLib(SVLIBRARY("scfilt"));
    static ::osl::Module aModule;
    bool bLoaded = aModule.loadRelative(&thisModule, sFilterLib);
    if (!bLoaded)
        bLoaded = aModule.load(sFilterLib);
    if (bLoaded)
    {
        oslGenericFunction fn = aModule.getFunctionSymbol( OUString( "ScFilterCreate" ) );
        if (fn != NULL)
            plugin = reinterpret_cast<FilterFn>(fn)();
    }
    if (plugin == NULL)
        plugin = new ScFormatFilterMissing();
#else
    plugin = ScFilterCreate();
#endif

    return *plugin;
}

// Precondition: pStr is guaranteed to be non-NULL and points to a 0-terminated
// array.
static inline const sal_Unicode* lcl_UnicodeStrChr( const sal_Unicode* pStr,
        sal_Unicode c )
{
    while (*pStr)
    {
        if (*pStr == c)
            return pStr;
        ++pStr;
    }
    return 0;
}

OUString ReadCsvLine( SvStream &rStream, bool bEmbeddedLineBreak,
        const String& rFieldSeparators, sal_Unicode cFieldQuote )
{
    OUString aStr;
    rStream.ReadUniOrByteStringLine(aStr, rStream.GetStreamCharSet(), nArbitraryLineLengthLimit);

    if (bEmbeddedLineBreak)
    {
        const sal_Unicode* pSeps = rFieldSeparators.GetBuffer();

        QuoteType eQuoteState = FIELDEND_QUOTE;
        bool bFieldStart = true;

        sal_Int32 nLastOffset = 0;
        sal_Int32 nQuotes = 0;
        while (!rStream.IsEof() && aStr.getLength() < nArbitraryLineLengthLimit)
        {
            const sal_Unicode *p, *pStart;
            p = pStart = aStr.getStr();
            p += nLastOffset;
            while (*p)
            {
                if (nQuotes)
                {
                    if (*p == cFieldQuote)
                    {
                        if (bFieldStart)
                        {
                            ++nQuotes;
                            bFieldStart = false;
                            eQuoteState = FIELDSTART_QUOTE;
                        }
                        // Do not detect a FIELDSTART_QUOTE if not in
                        // bFieldStart mode, in which case for unquoted content
                        // we are in FIELDEND_QUOTE state.
                        else if (eQuoteState != FIELDEND_QUOTE)
                        {
                            eQuoteState = lcl_isEscapedOrFieldEndQuote( nQuotes, p, pSeps, cFieldQuote);
                            // DONTKNOW_QUOTE is an embedded unescaped quote we
                            // don't count for pairing.
                            if (eQuoteState != DONTKNOW_QUOTE)
                                ++nQuotes;
                        }
                    }
                    else if (eQuoteState == FIELDEND_QUOTE)
                    {
                        if (bFieldStart)
                            // If blank is a separator it starts a field, if it
                            // is not and thus maybe leading before quote we
                            // are still at start of field regarding quotes.
                            bFieldStart = (*p == ' ' || lcl_UnicodeStrChr( pSeps, *p) != NULL);
                        else
                            bFieldStart = (lcl_UnicodeStrChr( pSeps, *p) != NULL);
                    }
                }
                else
                {
                    if (*p == cFieldQuote && bFieldStart)
                    {
                        nQuotes = 1;
                        eQuoteState = FIELDSTART_QUOTE;
                        bFieldStart = false;
                    }
                    else if (eQuoteState == FIELDEND_QUOTE)
                    {
                        // This also skips leading blanks at beginning of line
                        // if followed by a quote. It's debatable whether we
                        // actually want that or not, but congruent with what
                        // ScanNextFieldFromString() does.
                        if (bFieldStart)
                            bFieldStart = (*p == ' ' || lcl_UnicodeStrChr( pSeps, *p) != NULL);
                        else
                            bFieldStart = (lcl_UnicodeStrChr( pSeps, *p) != NULL);
                    }
                }
                // A quote character inside a field content does not start
                // a quote.
                ++p;
            }

            if (nQuotes % 2 == 0)
                // We still have a (theoretical?) problem here if due to
                // nArbitraryLineLengthLimit we split a string right between a
                // doubled quote pair.
                break;
            else
            {
                nLastOffset = aStr.getLength();
                OUString aNext;
                rStream.ReadUniOrByteStringLine(aNext, rStream.GetStreamCharSet(), nArbitraryLineLengthLimit);
                aStr += OUString( sal_Unicode('\n'));
                aStr += aNext;
            }
        }
    }
    return aStr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
