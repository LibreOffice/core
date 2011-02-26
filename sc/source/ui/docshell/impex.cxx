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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// System - Includes -----------------------------------------------------

class StarBASIC;



#ifndef PCH
#include "sc.hrc"
#define GLOBALOVERFLOW
#endif

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <osl/endian.h>
#include <i18npool/mslangid.hxx>
#include <tools/string.hxx>
#include <rtl/math.hxx>
#include <svtools/htmlout.hxx>
#include <svl/zforlist.hxx>
#define _SVSTDARR_ULONGS
#include <svl/svstdarr.hxx>
#include <sot/formats.hxx>
#include <sfx2/mieclip.hxx>
#include <unotools/charclass.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <unotools/transliterationwrapper.hxx>

#include "global.hxx"
#include "scerrors.hxx"
#include "docsh.hxx"
#include "undoblk.hxx"
#include "rangenam.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "filter.hxx"
#include "asciiopt.hxx"
#include "cell.hxx"
#include "docoptio.hxx"
#include "progress.hxx"
#include "scitems.hxx"
#include "editable.hxx"
#include "compiler.hxx"
#include "warnbox.hxx"

#include "impex.hxx"

// ause
#include "editutil.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "stringutil.hxx"

#include "globstr.hrc"
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <osl/module.hxx>

//========================================================================

namespace
{
    const String SYLK_LF = String::CreateFromAscii("\x1b :");
    const String DOUBLE_SEMICOLON = String::CreateFromAscii(";;");
    const String DOUBLE_DOUBLEQUOTE = String::CreateFromAscii("\"\"");
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
      bFormulas( FALSE ), bIncludeFiltered( TRUE ),
      bAll( TRUE ), bSingle( TRUE ), bUndo( FALSE ),
      bOverflow( FALSE ), mbApi( true ), mExportTextOptions()
{
    pUndoDoc = NULL;
    pExtOptions = NULL;
}

// Insert am Punkt ohne Bereichschecks


ScImportExport::ScImportExport( ScDocument* p, const ScAddress& rPt )
    : pDocSh( PTR_CAST(ScDocShell,p->GetDocumentShell()) ), pDoc( p ),
      aRange( rPt ),
      nSizeLimit( 0 ), cSep( '\t' ), cStr( '"' ),
      bFormulas( FALSE ), bIncludeFiltered( TRUE ),
      bAll( FALSE ), bSingle( TRUE ), bUndo( BOOL( pDocSh != NULL ) ),
      bOverflow( FALSE ), mbApi( true ), mExportTextOptions()
{
    pUndoDoc = NULL;
    pExtOptions = NULL;
}


//  ctor with a range is only used for export
//! ctor with a string (and bSingle=TRUE) is also used for DdeSetData

ScImportExport::ScImportExport( ScDocument* p, const ScRange& r )
    : pDocSh( PTR_CAST(ScDocShell,p->GetDocumentShell()) ), pDoc( p ),
      aRange( r ),
      nSizeLimit( 0 ), cSep( '\t' ), cStr( '"' ),
      bFormulas( FALSE ), bIncludeFiltered( TRUE ),
      bAll( FALSE ), bSingle( FALSE ), bUndo( BOOL( pDocSh != NULL ) ),
      bOverflow( FALSE ), mbApi( true ), mExportTextOptions()
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
      bFormulas( FALSE ), bIncludeFiltered( TRUE ),
      bAll( FALSE ), bSingle( TRUE ), bUndo( BOOL( pDocSh != NULL ) ),
      bOverflow( FALSE ), mbApi( true ), mExportTextOptions()
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
        USHORT nPos;
        if( pRange->SearchName( aPos, nPos ) )
        {
            ScRangeData* pData = (*pRange)[ nPos ];
            if( pData->HasType( RT_REFAREA )
                || pData->HasType( RT_ABSAREA )
                || pData->HasType( RT_ABSPOS ) )
                pData->GetSymbol( aPos );                   // mit dem Inhalt weitertesten
        }
    }
    formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    // Bereich?
    if( aRange.Parse( aPos, pDoc, eConv ) & SCA_VALID )
        bSingle = FALSE;
    // Zelle?
    else if( aRange.aStart.Parse( aPos, pDoc, eConv ) & SCA_VALID )
        aRange.aEnd = aRange.aStart;
    else
        bAll = TRUE;
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


BOOL ScImportExport::IsFormatSupported( ULONG nFormat )
{
    return BOOL( nFormat == FORMAT_STRING
              || nFormat == SOT_FORMATSTR_ID_SYLK
              || nFormat == SOT_FORMATSTR_ID_LINK
              || nFormat == SOT_FORMATSTR_ID_HTML
              || nFormat == SOT_FORMATSTR_ID_HTML_SIMPLE
              || nFormat == SOT_FORMATSTR_ID_DIF );
}


//////////////////////////////////////////////////////////////////////////////

// Vorbereitung fuer Undo: Undo-Dokument erzeugen


BOOL ScImportExport::StartPaste()
{
    if ( !bAll )
    {
        ScEditableTester aTester( pDoc, aRange );
        if ( !aTester.IsEditable() )
        {
            InfoBox aInfoBox(Application::GetDefDialogParent(),
                                ScGlobal::GetRscString( aTester.GetMessageId() ) );
            aInfoBox.Execute();
            return FALSE;
        }
    }
    if( bUndo && pDocSh && pDoc->IsUndoEnabled())
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, aRange.aStart.Tab(), aRange.aEnd.Tab() );
        pDoc->CopyToDocument( aRange, IDF_ALL | IDF_NOCAPTIONS, FALSE, pUndoDoc );
    }
    return TRUE;
}

// Nachbereitung Insert: Undo/Redo-Aktionen erzeugen, Invalidate/Repaint


void ScImportExport::EndPaste()
{
    BOOL bHeight = pDocSh && pDocSh->AdjustRowHeight(
                    aRange.aStart.Row(), aRange.aEnd.Row(), aRange.aStart.Tab() );

    if( pUndoDoc && pDoc->IsUndoEnabled() )
    {
        ScDocument* pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
        pRedoDoc->InitUndo( pDoc, aRange.aStart.Tab(), aRange.aEnd.Tab() );
        pDoc->CopyToDocument( aRange, IDF_ALL | IDF_NOCAPTIONS, FALSE, pRedoDoc );
        ScMarkData aDestMark;
        aDestMark.SelectOneTable( aRange.aStart.Tab() );
        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoPaste( pDocSh,
                aRange.aStart.Col(), aRange.aStart.Row(), aRange.aStart.Tab(),
                aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aEnd.Tab(), aDestMark,
                pUndoDoc, pRedoDoc, IDF_ALL, NULL,NULL,NULL,NULL ) );
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

BOOL ScImportExport::ImportData( const String& /* rMimeType */,
                     const ::com::sun::star::uno::Any & /* rValue */ )
{
    DBG_ASSERT( !this, "Implementation is missing" );
    return FALSE;
}

BOOL ScImportExport::ExportData( const String& rMimeType,
                                 ::com::sun::star::uno::Any & rValue )
{
    SvMemoryStream aStrm;
    // mba: no BaseURL for data exchange
    if( ExportStream( aStrm, String(),
                SotExchange::GetFormatIdFromMimeType( rMimeType ) ))
    {
        aStrm << (BYTE) 0;
        rValue <<= ::com::sun::star::uno::Sequence< sal_Int8 >(
                                        (sal_Int8*)aStrm.GetData(),
                                        aStrm.Seek( STREAM_SEEK_TO_END ) );
        return TRUE;
    }
    return FALSE;
}


BOOL ScImportExport::ImportString( const ::rtl::OUString& rText, ULONG nFmt )
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
            rtl_TextEncoding eEnc = gsl_getSystemTextEncoding();
            ::rtl::OString aTmp( rText.getStr(), rText.getLength(), eEnc );
            SvMemoryStream aStrm( (void*)aTmp.getStr(), aTmp.getLength() * sizeof(sal_Char), STREAM_READ );
            aStrm.SetStreamCharSet( eEnc );
            SetNoEndianSwap( aStrm );       //! no swapping in memory
            return ImportStream( aStrm, String(), nFmt );
        }
    }
}


BOOL ScImportExport::ExportString( ::rtl::OUString& rText, ULONG nFmt )
{
    DBG_ASSERT( nFmt == FORMAT_STRING, "ScImportExport::ExportString: Unicode not supported for other formats than FORMAT_STRING" );
    if ( nFmt != FORMAT_STRING )
    {
        rtl_TextEncoding eEnc = gsl_getSystemTextEncoding();
        ByteString aTmp;
        BOOL bOk = ExportByteString( aTmp, eEnc, nFmt );
        rText = UniString( aTmp, eEnc );
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

        rText = rtl::OUString( (const sal_Unicode*) aStrm.GetData() );
        return TRUE;
    }
    rText = rtl::OUString();
    return FALSE;

    // ExportStream must handle RTL_TEXTENCODING_UNICODE
}


BOOL ScImportExport::ExportByteString( ByteString& rText, rtl_TextEncoding eEnc, ULONG nFmt )
{
    DBG_ASSERT( eEnc != RTL_TEXTENCODING_UNICODE, "ScImportExport::ExportByteString: Unicode not supported" );
    if ( eEnc == RTL_TEXTENCODING_UNICODE )
        eEnc = gsl_getSystemTextEncoding();

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
        if( aStrm.Tell() <= (ULONG) STRING_MAXLEN )
        {
            rText = (const sal_Char*) aStrm.GetData();
            return TRUE;
        }
    }
    rText.Erase();
    return FALSE;
}


BOOL ScImportExport::ImportStream( SvStream& rStrm, const String& rBaseURL, ULONG nFmt )
{
    if( nFmt == FORMAT_STRING )
    {
        if( ExtText2Doc( rStrm ) )      // pExtOptions auswerten
            return TRUE;
    }
    if( nFmt == SOT_FORMATSTR_ID_SYLK )
    {
        if( Sylk2Doc( rStrm ) )
            return TRUE;
    }
    if( nFmt == SOT_FORMATSTR_ID_DIF )
    {
        if( Dif2Doc( rStrm ) )
            return TRUE;
    }
    if( nFmt == FORMAT_RTF )
    {
        if( RTF2Doc( rStrm, rBaseURL ) )
            return TRUE;
    }
    if( nFmt == SOT_FORMATSTR_ID_LINK )
        return TRUE;            // Link-Import?
    if ( nFmt == SOT_FORMATSTR_ID_HTML )
    {
        if( HTML2Doc( rStrm, rBaseURL ) )
            return TRUE;
    }
    if ( nFmt == SOT_FORMATSTR_ID_HTML_SIMPLE )
    {
        MSE40HTMLClipFormatObj aMSE40ClpObj;                // needed to skip the header data
        SvStream* pHTML = aMSE40ClpObj.IsValid( rStrm );
        if ( pHTML && HTML2Doc( *pHTML, rBaseURL ) )
            return TRUE;
    }

    return FALSE;
}


BOOL ScImportExport::ExportStream( SvStream& rStrm, const String& rBaseURL, ULONG nFmt )
{
    if( nFmt == FORMAT_STRING )
    {
        if( Doc2Text( rStrm ) )
            return TRUE;
    }
    if( nFmt == SOT_FORMATSTR_ID_SYLK )
    {
        if( Doc2Sylk( rStrm ) )
            return TRUE;
    }
    if( nFmt == SOT_FORMATSTR_ID_DIF )
    {
        if( Doc2Dif( rStrm ) )
            return TRUE;
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

        DBG_ASSERT( aDocName.Len(), "ClipBoard document has no name! :-/" );
        if( aDocName.Len() )
        {
            String aRefName;
            USHORT nFlags = SCA_VALID | SCA_TAB_3D;
            if( bSingle )
                aRange.aStart.Format( aRefName, nFlags, pDoc, pDoc->GetAddressConvention() );
            else
            {
                if( aRange.aStart.Tab() != aRange.aEnd.Tab() )
                    nFlags |= SCA_TAB2_3D;
                aRange.Format( aRefName, nFlags, pDoc );
            }
            String aAppName = Application::GetAppName();

            WriteUnicodeOrByteString( rStrm, aAppName, TRUE );
            WriteUnicodeOrByteString( rStrm, aDocName, TRUE );
            WriteUnicodeOrByteString( rStrm, aRefName, TRUE );
            if ( rStrm.GetStreamCharSet() == RTL_TEXTENCODING_UNICODE )
                rStrm << sal_Unicode(0);
            else
                rStrm << sal_Char(0);
            return BOOL( rStrm.GetError() == SVSTREAM_OK );
        }
    }
    if( nFmt == SOT_FORMATSTR_ID_HTML )
    {
        if( Doc2HTML( rStrm, rBaseURL ) )
            return TRUE;
    }
    if( nFmt == FORMAT_RTF )
    {
        if( Doc2RTF( rStrm ) )
            return TRUE;
    }

    return FALSE;
}


void ScImportExport::WriteUnicodeOrByteString( SvStream& rStrm, const String& rString, BOOL bZero )
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
        ByteString aByteStr( rString, eEnc );
        rStrm << aByteStr.GetBuffer();
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
                rStrm << sal_Unicode(_CR);
            break;
            case LINEEND_LF :
                rStrm << sal_Unicode(_LF);
            break;
            default:
                rStrm << sal_Unicode(_CR) << sal_Unicode(_LF);
        }
    }
    else
        endl( rStrm );
}


enum DoubledQuoteMode
{
    DQM_KEEP,       // both are taken
    DQM_ESCAPE,     // escaped quote, one is taken, one ignored
    DQM_CONCAT,     // first is end, next is start, both ignored => strings combined
    DQM_SEPARATE    // end one string and begin next
};

static const sal_Unicode* lcl_ScanString( const sal_Unicode* p, String& rString,
            sal_Unicode cStr, DoubledQuoteMode eMode )
{
    p++;    //! jump over opening quote
    BOOL bCont;
    do
    {
        bCont = FALSE;
        const sal_Unicode* p0 = p;
        for( ;; )
        {
            if( !*p )
                break;
            if( *p == cStr )
            {
                if ( *++p != cStr )
                    break;
                // doubled quote char
                switch ( eMode )
                {
                    case DQM_KEEP :
                        p++;            // both for us (not breaking for-loop)
                    break;
                    case DQM_ESCAPE :
                        p++;            // one for us (breaking for-loop)
                        bCont = TRUE;   // and more
                    break;
                    case DQM_CONCAT :
                        if ( p0+1 < p )
                            rString.Append( p0, sal::static_int_cast<xub_StrLen>( (p-1) - p0 ) );  // first part
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
            rString.Append( p0, sal::static_int_cast<xub_StrLen>( ((*p || *(p-1) == cStr) ? p-1 : p) - p0 ) );
    } while ( bCont );
    return p;
}

void lcl_UnescapeSylk( String & rString, SylkVersion eVersion )
{
    // Older versions didn't escape the semicolon.
    // Older versions quoted the string and doubled embedded quotes, but not
    // the semicolons, which was plain wrong.
    if (eVersion >= SYLK_OOO32)
        rString.SearchAndReplaceAll( DOUBLE_SEMICOLON, ';' );
    else
        rString.SearchAndReplaceAll( DOUBLE_DOUBLEQUOTE, '"' );

    rString.SearchAndReplaceAll( SYLK_LF, _LF );
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

inline void lcl_WriteSimpleString( SvStream& rStrm, const String& rString )
{
    ScImportExport::WriteUnicodeOrByteString( rStrm, rString );
}

//////////////////////////////////////////////////////////////////////////////


BOOL ScImportExport::Text2Doc( SvStream& rStrm )
{
    BOOL bOk = TRUE;

    SCCOL nStartCol = aRange.aStart.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nEndRow = aRange.aEnd.Row();
    ULONG  nOldPos = rStrm.Tell();
    if ( rStrm.GetStreamCharSet() == RTL_TEXTENCODING_UNICODE )
        rStrm.StartReadingUnicodeText();
    BOOL   bData = BOOL( !bSingle );
    if( !bSingle)
        bOk = StartPaste();

    while( bOk )
    {
        ByteString aByteLine;
        String aLine, aCell;
        SCROW nRow = nStartRow;
        rStrm.Seek( nOldPos );
        for( ;; )
        {
            rStrm.ReadUniOrByteStringLine( aLine );
            if( rStrm.IsEof() )
                break;
            SCCOL nCol = nStartCol;
            const sal_Unicode* p = aLine.GetBuffer();
            while( *p )
            {
                aCell.Erase();
                if( *p == cStr )
                {
                    p = lcl_ScanString( p, aCell, cStr, DQM_KEEP );
                    while( *p && *p != cSep )
                        p++;
                    if( *p )
                        p++;
                }
                else
                {
                    const sal_Unicode* q = p;
                    while( *p && *p != cSep )
                        p++;
                    aCell.Assign( q, sal::static_int_cast<xub_StrLen>( p - q ) );
                    if( *p )
                        p++;
                }
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
                    bOverflow = TRUE;           // beim Import Warnung ausgeben
                ++nCol;
            }
            ++nRow;
        }

        if( !bData )
        {
            aRange.aEnd.SetCol( nEndCol );
            aRange.aEnd.SetRow( nEndRow );
            bOk = StartPaste();
            bData = TRUE;
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
    ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rStr, BYTE nColFormat,
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
        sal_uInt32 nIndex;
        if (pFormatter->IsNumberFormat(rStr, nIndex, fDummy))
        {
            // Set the format of this cell to Text.
            sal_uInt32 nFormat = pFormatter->GetStandardFormat(NUMBERFORMAT_TEXT);
            ScPatternAttr aNewAttrs(pDoc->GetPool());
            SfxItemSet& rSet = aNewAttrs.GetItemSet();
            rSet.Put( SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat) );
            pDoc->ApplyPattern(nCol, nRow, nTab, aNewAttrs);

        }
        pDoc->PutCell( nCol, nRow, nTab, ScBaseCell::CreateTextCell( rStr, pDoc ) );
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
        const USHORT nMaxNumberParts = 7;   // Y-M-D h:m:s.t
        xub_StrLen nLen = rStr.Len();
        xub_StrLen nStart[nMaxNumberParts];
        xub_StrLen nEnd[nMaxNumberParts];

        USHORT nDP, nMP, nYP;
        switch ( nColFormat )
        {
            case SC_COL_YMD: nDP = 2; nMP = 1; nYP = 0; break;
            case SC_COL_MDY: nDP = 1; nMP = 0; nYP = 2; break;
            case SC_COL_DMY:
            default:         nDP = 0; nMP = 1; nYP = 2; break;
        }

        USHORT nFound = 0;
        BOOL bInNum = FALSE;
        for ( xub_StrLen nPos=0; nPos<nLen && (bInNum ||
                    nFound<nMaxNumberParts); nPos++ )
        {
            if (bInNum && nFound == 3 && nColFormat == SC_COL_YMD &&
                    nPos <= nStart[nFound]+2 && rStr.GetChar(nPos) == 'T')
                bInNum = FALSE;     // ISO-8601: YYYY-MM-DDThh:mm...
            else if ((((!bInNum && nFound==nMP) || (bInNum && nFound==nMP+1))
                        && ScGlobal::pCharClass->isLetterNumeric( rStr, nPos))
                    || ScGlobal::pCharClass->isDigit( rStr, nPos))
            {
                if (!bInNum)
                {
                    bInNum = TRUE;
                    nStart[nFound] = nPos;
                    ++nFound;
                }
                nEnd[nFound-1] = nPos;
            }
            else
                bInNum = FALSE;
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

                BOOL bLongYear = ( nDateLen >= 7 );
                BOOL bShortFirst = ( nDateLen == 5 || nDateLen == 7 );

                USHORT nFieldStart = nDateStart;
                for (USHORT nPos=0; nPos<3; nPos++)
                {
                    USHORT nFieldEnd = nFieldStart + 1;     // default: 2 digits
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
            BOOL bSecondCal = FALSE;
            USHORT nDay  = (USHORT) rStr.Copy( nStart[nDP], nEnd[nDP]+1-nStart[nDP] ).ToInt32();
            USHORT nYear = (USHORT) rStr.Copy( nStart[nYP], nEnd[nYP]+1-nStart[nYP] ).ToInt32();
            String aMStr = rStr.Copy( nStart[nMP], nEnd[nMP]+1-nStart[nMP] );
            sal_Int16 nMonth = (sal_Int16) aMStr.ToInt32();
            if (!nMonth)
            {
                static const String aSeptCorrect( RTL_CONSTASCII_USTRINGPARAM( "SEPT" ) );
                static const String aSepShortened( RTL_CONSTASCII_USTRINGPARAM( "SEP" ) );
                uno::Sequence< i18n::CalendarItem > xMonths;
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
                    {   // #102136# correct English abbreviation is SEPT,
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
                            bSecondCal = TRUE;
                        }
                        else if ( i == 8 && pSecondTransliteration->isEqual(
                                    aMStr, aSepShortened ) )
                        {   // #102136# correct English abbreviation is SEPT,
                            // but data mostly contains SEP only
                            nMonth = sal::static_int_cast<sal_Int16>( i+1 );
                            bSecondCal = TRUE;
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
                    rtl::OUString aT( &cDec, 1);
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
                    ULONG nFormat = pDocFormatter->GetStandardFormat( nType, eDocLang );
                    // maybe there is a special format including seconds or milliseconds
                    if (nFound > 5)
                        nFormat = pDocFormatter->GetStandardFormat( fDays, nFormat, nType, eDocLang);

                    pDoc->PutCell( nCol, nRow, nTab, new ScValueCell(fDays), nFormat, FALSE );

                    return bMultiLine;     // success
                }
            }
        }
    }

    // Standard or date not determined -> SetString / EditCell
    if( rStr.Search( _LF ) == STRING_NOTFOUND )
    {
        ScSetStringParam aParam;
        aParam.mpNumFormatter = pFormatter;
        aParam.mbDetectNumberFormat = bDetectNumFormat;
        aParam.mbSetTextCellFormat = true;
        pDoc->SetString( nCol, nRow, nTab, rStr, &aParam );
    }
    else
    {
        bMultiLine = true;
        pDoc->PutCell( nCol, nRow, nTab, new ScEditCell( rStr, pDoc ) );
    }
    return bMultiLine;
}


String lcl_GetFixed( const String& rLine, xub_StrLen nStart, xub_StrLen nNext, bool& rbIsQuoted )
{
    xub_StrLen nLen = rLine.Len();
    if (nNext > nLen)
        nNext = nLen;
    if ( nNext <= nStart )
        return EMPTY_STRING;

    const sal_Unicode* pStr = rLine.GetBuffer();

    xub_StrLen nSpace = nNext;
    while ( nSpace > nStart && pStr[nSpace-1] == ' ' )
        --nSpace;

    rbIsQuoted = (pStr[nStart] == sal_Unicode('"') && pStr[nSpace-1] == sal_Unicode('"'));
    if (rbIsQuoted)
        return rLine.Copy(nStart+1, nSpace-nStart-2);
    else
        return rLine.Copy(nStart, nSpace-nStart);
}

BOOL ScImportExport::ExtText2Doc( SvStream& rStrm )
{
    if (!pExtOptions)
        return Text2Doc( rStrm );

    ULONG nOldPos = rStrm.Tell();
    rStrm.Seek( STREAM_SEEK_TO_END );
    ::std::auto_ptr<ScProgress> xProgress( new ScProgress( pDocSh,
            ScGlobal::GetRscString( STR_LOAD_DOC ), rStrm.Tell() - nOldPos ));
    rStrm.Seek( nOldPos );
    if ( rStrm.GetStreamCharSet() == RTL_TEXTENCODING_UNICODE )
        rStrm.StartReadingUnicodeText();

    ScColumn::DoubleAllocSwitch aAllocSwitch(true);

    SCCOL nStartCol = aRange.aStart.Col();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCTAB nTab = aRange.aStart.Tab();

    BOOL    bFixed          = pExtOptions->IsFixedLen();
    const String& rSeps     = pExtOptions->GetFieldSeps();
    const sal_Unicode* pSeps = rSeps.GetBuffer();
    BOOL    bMerge          = pExtOptions->IsMergeSeps();
    USHORT  nInfoCount      = pExtOptions->GetInfoCount();
    const xub_StrLen* pColStart = pExtOptions->GetColStart();
    const BYTE* pColFormat  = pExtOptions->GetColFormat();
    long nSkipLines = pExtOptions->GetStartRow();

    LanguageType eDocLang = pExtOptions->GetLanguage();
    SvNumberFormatter aNumFormatter(pDoc->GetServiceManager(), eDocLang);
    bool bDetectNumFormat = pExtOptions->IsDetectSpecialNumber();

    // For date recognition
    ::utl::TransliterationWrapper aTransliteration(
        pDoc->GetServiceManager(), SC_TRANSLITERATION_IGNORECASE );
    aTransliteration.loadModuleIfNeeded( eDocLang );
    CalendarWrapper aCalendar( pDoc->GetServiceManager() );
    aCalendar.loadDefaultCalendar(
        MsLangId::convertLanguageToLocale( eDocLang ) );
    ::utl::TransliterationWrapper* pEnglishTransliteration = NULL;
    CalendarWrapper* pEnglishCalendar = NULL;
    if ( eDocLang != LANGUAGE_ENGLISH_US )
    {
        pEnglishTransliteration = new ::utl::TransliterationWrapper (
            pDoc->GetServiceManager(), SC_TRANSLITERATION_IGNORECASE );
        aTransliteration.loadModuleIfNeeded( LANGUAGE_ENGLISH_US );
        pEnglishCalendar = new CalendarWrapper ( pDoc->GetServiceManager() );
        pEnglishCalendar->loadDefaultCalendar(
            MsLangId::convertLanguageToLocale( LANGUAGE_ENGLISH_US ) );
    }

    String aLine, aCell;
    USHORT i;
    SCROW nRow = nStartRow;

    while(--nSkipLines>0)
    {
        rStrm.ReadCsvLine( aLine, !bFixed, rSeps, cStr); // content is ignored
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

    ULONG nOriginalStreamPos = rStrm.Tell();

    do
    {
        for( ;; )
        {
            rStrm.ReadCsvLine( aLine, !bFixed, rSeps, cStr);
            if ( rStrm.IsEof() )
                break;

            xub_StrLen nLineLen = aLine.Len();
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
                    BYTE nFmt = pColFormat[i];
                    if (nFmt != SC_COL_SKIP)        // sonst auch nCol nicht hochzaehlen
                    {
                        if (nCol > MAXCOL)
                            bOverflow = TRUE;       // display warning on import
                        else if (!bDetermineRange)
                        {
                            xub_StrLen nStart = pColStart[i];
                            xub_StrLen nNext = ( i+1 < nInfoCount ) ? pColStart[i+1] : nLineLen;
                            bool bIsQuoted = false;
                            aCell = lcl_GetFixed( aLine, nStart, nNext, bIsQuoted );
                            if (bIsQuoted && bQuotedAsText)
                                nFmt = SC_COL_TEXT;

                            bMultiLine |= lcl_PutString(
                                pDoc, nCol, nRow, nTab, aCell, nFmt,
                                &aNumFormatter, bDetectNumFormat, aTransliteration, aCalendar,
                                pEnglishTransliteration, pEnglishCalendar);
                        }
                        ++nCol;
                    }
                }
            }
            else                        //  Nach Trennzeichen suchen
            {
                SCCOL nSourceCol = 0;
                USHORT nInfoStart = 0;
                const sal_Unicode* p = aLine.GetBuffer();
                // Yes, the check is nCol<=MAXCOL+1, +1 because it is only an
                // overflow if there is really data following to be put behind
                // the last column, which doesn't happen if info is
                // SC_COL_SKIP.
                while (*p && nCol <= MAXCOL+1)
                {
                    bool bIsQuoted = false;
                    p = ScImportExport::ScanNextFieldFromString( p, aCell, cStr, pSeps, bMerge, bIsQuoted );

                    BYTE nFmt = SC_COL_STANDARD;
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
                            bOverflow = TRUE;       // display warning on import
                        else if (!bDetermineRange)
                        {
                            if (bIsQuoted && bQuotedAsText)
                                nFmt = SC_COL_TEXT;

                            bMultiLine |= lcl_PutString(
                                pDoc, nCol, nRow, nTab, aCell, nFmt,
                                &aNumFormatter, bDetectNumFormat, aTransliteration,
                                aCalendar, pEnglishTransliteration, pEnglishCalendar);
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
                bOverflow = TRUE;       // display warning on import
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
                    delete pEnglishTransliteration;
                    delete pEnglishCalendar;
                    return FALSE;
                }
            }

            rStrm.Seek( nOriginalStreamPos );
            nRow = nStartRow;
            if (!StartPaste())
            {
                EndPaste();
                return FALSE;
            }
        }

        bDetermineRange = !bDetermineRange;     // toggle
    } while (!bDetermineRange);

    pDoc->DoColResize( nTab, nStartCol, nEndCol, 0 );

    delete pEnglishTransliteration;
    delete pEnglishCalendar;

    xProgress.reset();    // make room for AdjustRowHeight progress
    if (bRangeIsDetermined)
        EndPaste();

    return TRUE;
}


const sal_Unicode* ScImportExport::ScanNextFieldFromString( const sal_Unicode* p,
        String& rField, sal_Unicode cStr, const sal_Unicode* pSeps, bool bMergeSeps, bool& rbIsQuoted )
{
    rbIsQuoted = false;
    rField.Erase();
    if ( *p == cStr )           // String in Anfuehrungszeichen
    {
        rbIsQuoted = true;
        const sal_Unicode* p1;
        p1 = p = lcl_ScanString( p, rField, cStr, DQM_ESCAPE );
        while ( *p && !ScGlobal::UnicodeStrChr( pSeps, *p ) )
            p++;
        // Append remaining unquoted and undelimited data (dirty, dirty) to
        // this field.
        if (p > p1)
            rField.Append( p1, sal::static_int_cast<xub_StrLen>( p - p1 ) );
        if( *p )
            p++;
    }
    else                        // bis zum Trennzeichen
    {
        const sal_Unicode* p0 = p;
        while ( *p && !ScGlobal::UnicodeStrChr( pSeps, *p ) )
            p++;
        rField.Append( p0, sal::static_int_cast<xub_StrLen>( p - p0 ) );
        if( *p )
            p++;
    }
    if ( bMergeSeps )           // folgende Trennzeichen ueberspringen
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
            case _LF:
            case _CR:
                // line break found.
                return true;
            default:
                ;
        }
    }
    return false;
}

}

BOOL ScImportExport::Doc2Text( SvStream& rStrm )
{
    SCCOL nCol;
    SCROW nRow;
    SCCOL nStartCol = aRange.aStart.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nEndRow = aRange.aEnd.Row();
    String aCell;
    bool bConvertLF = (GetSystemLineEnd() != LINEEND_LF);

    for (nRow = nStartRow; nRow <= nEndRow; nRow++)
    {
        if (bIncludeFiltered || !pDoc->RowFiltered( nRow, aRange.aStart.Tab() ))
        {
            for (nCol = nStartCol; nCol <= nEndCol; nCol++)
            {
                CellType eType;
                pDoc->GetCellType( nCol, nRow, aRange.aStart.Tab(), eType );
                switch (eType)
                {
                    case CELLTYPE_FORMULA:
                    {
                        if (bFormulas)
                        {
                            pDoc->GetFormula( nCol, nRow, aRange.aStart.Tab(), aCell );
                            if( aCell.Search( cSep ) != STRING_NOTFOUND )
                                lcl_WriteString( rStrm, aCell, cStr, cStr );
                            else
                                lcl_WriteSimpleString( rStrm, aCell );
                        }
                        else
                        {
                            pDoc->GetString( nCol, nRow, aRange.aStart.Tab(), aCell );

                            bool bMultiLineText = ( aCell.Search( _LF ) != STRING_NOTFOUND );
                            if( bMultiLineText )
                            {
                                if( mExportTextOptions.meNewlineConversion == ScExportTextOptions::ToSpace )
                                    aCell.SearchAndReplaceAll( _LF, ' ' );
                                else if ( mExportTextOptions.meNewlineConversion == ScExportTextOptions::ToSystem && bConvertLF )
                                    aCell.ConvertLineEnd();
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
                        pDoc->GetString( nCol, nRow, aRange.aStart.Tab(), aCell );
                        lcl_WriteSimpleString( rStrm, aCell );
                    }
                    break;
                    case CELLTYPE_NOTE:
                    case CELLTYPE_NONE:
                    break;
                    default:
                    {
                        pDoc->GetString( nCol, nRow, aRange.aStart.Tab(), aCell );

                        bool bMultiLineText = ( aCell.Search( _LF ) != STRING_NOTFOUND );
                        if( bMultiLineText )
                        {
                            if( mExportTextOptions.meNewlineConversion == ScExportTextOptions::ToSpace )
                                aCell.SearchAndReplaceAll( _LF, ' ' );
                            else if ( mExportTextOptions.meNewlineConversion == ScExportTextOptions::ToSystem && bConvertLF )
                                aCell.ConvertLineEnd();
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
                    lcl_WriteSimpleString( rStrm, String(cSep) );
            }
                WriteUnicodeOrByteEndl( rStrm );
            if( rStrm.GetError() != SVSTREAM_OK )
                break;
            if( nSizeLimit && rStrm.Tell() > nSizeLimit )
                break;
        }
    }

    return BOOL( rStrm.GetError() == SVSTREAM_OK );
}


BOOL ScImportExport::Sylk2Doc( SvStream& rStrm )
{
    BOOL bOk = TRUE;
    BOOL bMyDoc = FALSE;
    SylkVersion eVersion = SYLK_OTHER;

    // US-English separators for StringToDouble
    sal_Unicode cDecSep = '.';
    sal_Unicode cGrpSep = ',';

    SCCOL nStartCol = aRange.aStart.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nEndRow = aRange.aEnd.Row();
    ULONG nOldPos = rStrm.Tell();
    BOOL bData = BOOL( !bSingle );
    SvULongs aFormats;

    if( !bSingle)
        bOk = StartPaste();

    while( bOk )
    {
        String aLine;
        String aText;
        ByteString aByteLine;
        SCCOL nCol = nStartCol;
        SCROW nRow = nStartRow;
        SCCOL nRefCol = 1;
        SCROW nRefRow = 1;
        rStrm.Seek( nOldPos );
        for( ;; )
        {
            //! allow unicode
            rStrm.ReadLine( aByteLine );
            aLine = String( aByteLine, rStrm.GetStreamCharSet() );
            if( rStrm.IsEof() )
                break;
            const sal_Unicode* p = aLine.GetBuffer();
            sal_Unicode cTag = *p++;
            if( cTag == 'C' )       // Content
            {
                if( *p++ != ';' )
                    return FALSE;
                while( *p )
                {
                    sal_Unicode ch = *p++;
                    ch = ScGlobal::ToUpperAlpha( ch );
                    switch( ch )
                    {
                        case 'X':
                            nCol = static_cast<SCCOL>(String( p ).ToInt32()) + nStartCol - 1;
                            break;
                        case 'Y':
                            nRow = String( p ).ToInt32() + nStartRow - 1;
                            break;
                        case 'C':
                            nRefCol = static_cast<SCCOL>(String( p ).ToInt32()) + nStartCol - 1;
                            break;
                        case 'R':
                            nRefRow = String( p ).ToInt32() + nStartRow - 1;
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
                            BOOL bText;
                            if( *p == '"' )
                            {
                                bText = TRUE;
                                aText.Erase();
                                p = lcl_ScanSylkString( p, aText, eVersion);
                            }
                            else
                                bText = FALSE;
                            const sal_Unicode* q = p;
                            while( *q && *q != ';' )
                                q++;
                            if ( !(*q == ';' && *(q+1) == 'I') )
                            {   // don't ignore value
                                if( bText )
                                {
                                    pDoc->PutCell( nCol, nRow, aRange.aStart.Tab(),
                                            ScBaseCell::CreateTextCell( aText, pDoc),
                                            (BOOL) TRUE);
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
                                aMark.SelectTable( aPos.Tab(), TRUE );
                                pDoc->InsertMatrixFormula( nCol, nRow, nRefCol,
                                    nRefRow, aMark, EMPTY_STRING, pCode );
                            }
                            else
                            {
                                ScFormulaCell* pFCell = new ScFormulaCell(
                                        pDoc, aPos, pCode, eGrammar, MM_NONE);
                                pDoc->PutCell( aPos, pFCell );
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
                    return FALSE;
                sal_Int32 nFormat = -1;
                while( *p )
                {
                    sal_Unicode ch = *p++;
                    ch = ScGlobal::ToUpperAlpha( ch );
                    switch( ch )
                    {
                        case 'X':
                            nCol = static_cast<SCCOL>(String( p ).ToInt32()) + nStartCol - 1;
                            break;
                        case 'Y':
                            nRow = String( p ).ToInt32() + nStartRow - 1;
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
                                String aNumber( p0, sal::static_int_cast<xub_StrLen>( p - p0 ) );
                                nFormat = aNumber.ToInt32();
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
                if ( 0 <= nFormat && nFormat < aFormats.Count() )
                {
                    ULONG nKey = aFormats[(USHORT)nFormat];
                    pDoc->ApplyAttr( nCol, nRow, aRange.aStart.Tab(),
                            SfxUInt32Item( ATTR_VALUE_FORMAT, nKey ) );
                }
            }
            else if( cTag == 'P' )
            {
                if ( bData && *p == ';' && *(p+1) == 'P' )
                {
                    String aCode( p+2 );
                    // unescape doubled semicolons
                    xub_StrLen nPos = 0;
                    String aSemicolon( RTL_CONSTASCII_USTRINGPARAM(";;"));
                    while ( (nPos = aCode.Search( aSemicolon, nPos )) != STRING_NOTFOUND )
                        aCode.Erase( nPos++, 1 );
                    // get rid of Xcl escape characters
                    nPos = 0;
                    while ( (nPos = aCode.Search( sal_Unicode(0x1b), nPos )) != STRING_NOTFOUND )
                        aCode.Erase( nPos, 1 );
                    xub_StrLen nCheckPos;
                    short nType;
                    sal_uInt32 nKey;
                    pDoc->GetFormatTable()->PutandConvertEntry(
                        aCode, nCheckPos, nType, nKey, LANGUAGE_ENGLISH_US,
                        ScGlobal::eLnge );
                    if ( nCheckPos )
                        nKey = 0;
                    aFormats.Insert( nKey, aFormats.Count() );
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
            bData = TRUE;
        }
        else
            break;
    }

    EndPaste();
    return bOk;
}


BOOL ScImportExport::Doc2Sylk( SvStream& rStrm )
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
            BOOL bForm = FALSE;
            SCROW r = nRow - nStartRow + 1;
            SCCOL c = nCol - nStartCol + 1;
            ScBaseCell* pCell;
            pDoc->GetCell( nCol, nRow, aRange.aStart.Tab(), pCell );
            CellType eType = (pCell ? pCell->GetCellType() : CELLTYPE_NONE);
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
                            rtl_math_DecimalPlaces_Max, '.', TRUE );

                    aBufStr.AssignAscii(RTL_CONSTASCII_STRINGPARAM( "C;X" ));
                    aBufStr += String::CreateFromInt32( c );
                    aBufStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ";Y" ));
                    aBufStr += String::CreateFromInt32( r );
                    aBufStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ";K" ));
                    aBufStr += aValStr;
                    lcl_WriteSimpleString( rStrm, aBufStr );
                    goto checkformula;

                case CELLTYPE_STRING:
                case CELLTYPE_EDIT:
                hasstring:
                    pDoc->GetString( nCol, nRow, aRange.aStart.Tab(), aCellStr );
                    aCellStr.SearchAndReplaceAll( _LF, SYLK_LF );

                    aBufStr.AssignAscii(RTL_CONSTASCII_STRINGPARAM( "C;X" ));
                    aBufStr += String::CreateFromInt32( c );
                    aBufStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ";Y" ));
                    aBufStr += String::CreateFromInt32( r );
                    aBufStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ";K" ));
                    lcl_WriteSimpleString( rStrm, aBufStr );
                    lcl_WriteString( rStrm, aCellStr, '"', ';' );

                checkformula:
                    if( bForm )
                    {
                        const ScFormulaCell* pFCell =
                            static_cast<const ScFormulaCell*>(pCell);
                        switch ( pFCell->GetMatrixFlag() )
                        {
                            case MM_REFERENCE :
                                aCellStr.Erase();
                            break;
                            default:
                                pFCell->GetFormula( aCellStr,formula::FormulaGrammar::GRAM_PODF_A1);
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
                                aPrefix += String::CreateFromInt32( nR );
                                aPrefix.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ";C" ) );
                                aPrefix += String::CreateFromInt32( nC );
                                aPrefix.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ";M" ) );
                            }
                            break;
                            case MM_REFERENCE :
                            {   // diff expression with 'I' M$-extension
                                ScAddress aPos;
                                pFCell->GetMatrixOrigin( aPos );
                                aPrefix.AssignAscii( RTL_CONSTASCII_STRINGPARAM( ";I;R" ) );
                                aPrefix += String::CreateFromInt32( aPos.Row() - nStartRow + 1 );
                                aPrefix.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ";C" ) );
                                aPrefix += String::CreateFromInt32( aPos.Col() - nStartCol + 1 );
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
    lcl_WriteSimpleString( rStrm, String( 'E' ) );
    WriteUnicodeOrByteEndl( rStrm );
    return BOOL( rStrm.GetError() == SVSTREAM_OK );
}


BOOL ScImportExport::Doc2HTML( SvStream& rStrm, const String& rBaseURL )
{
    // CharSet is ignored in ScExportHTML, read from Load/Save HTML options
    ScFormatFilter::Get().ScExportHTML( rStrm, rBaseURL, pDoc, aRange, RTL_TEXTENCODING_DONTKNOW, bAll,
        aStreamPath, aNonConvertibleChars );
    return BOOL( rStrm.GetError() == SVSTREAM_OK );
}

BOOL ScImportExport::Doc2RTF( SvStream& rStrm )
{
    //  CharSet is ignored in ScExportRTF
    ScFormatFilter::Get().ScExportRTF( rStrm, pDoc, aRange, RTL_TEXTENCODING_DONTKNOW );
    return BOOL( rStrm.GetError() == SVSTREAM_OK );
}


BOOL ScImportExport::Doc2Dif( SvStream& rStrm )
{
    // for DIF in the clipboard, IBM_850 is always used
    ScFormatFilter::Get().ScExportDif( rStrm, pDoc, aRange, RTL_TEXTENCODING_IBM_850 );
    return TRUE;
}


BOOL ScImportExport::Dif2Doc( SvStream& rStrm )
{
    SCTAB nTab = aRange.aStart.Tab();
    ScDocument* pImportDoc = new ScDocument( SCDOCMODE_UNDO );
    pImportDoc->InitUndo( pDoc, nTab, nTab );

    // for DIF in the clipboard, IBM_850 is always used
    ScFormatFilter::Get().ScImportDif( rStrm, pImportDoc, aRange.aStart, RTL_TEXTENCODING_IBM_850 );

    SCCOL nEndCol;
    SCROW nEndRow;
    pImportDoc->GetCellArea( nTab, nEndCol, nEndRow );
    // #131247# if there are no cells in the imported content, nEndCol/nEndRow may be before the start
    if ( nEndCol < aRange.aStart.Col() )
        nEndCol = aRange.aStart.Col();
    if ( nEndRow < aRange.aStart.Row() )
        nEndRow = aRange.aStart.Row();
    aRange.aEnd = ScAddress( nEndCol, nEndRow, nTab );

    BOOL bOk = StartPaste();
    if (bOk)
    {
        USHORT nFlags = IDF_ALL & ~IDF_STYLES;
        pDoc->DeleteAreaTab( aRange, nFlags );
        pImportDoc->CopyToDocument( aRange, nFlags, FALSE, pDoc );
        EndPaste();
    }

    delete pImportDoc;

    return bOk;
}


BOOL ScImportExport::RTF2Doc( SvStream& rStrm, const String& rBaseURL )
{
    ScEEAbsImport *pImp = ScFormatFilter::Get().CreateRTFImport( pDoc, aRange );
    if (!pImp)
        return false;
    pImp->Read( rStrm, rBaseURL );
    aRange = pImp->GetRange();

    BOOL bOk = StartPaste();
    if (bOk)
    {
        USHORT nFlags = IDF_ALL & ~IDF_STYLES;
        pDoc->DeleteAreaTab( aRange, nFlags );
        pImp->WriteToDocument();
        EndPaste();
    }
    delete pImp;
    return bOk;
}


BOOL ScImportExport::HTML2Doc( SvStream& rStrm, const String& rBaseURL )
{
    ScEEAbsImport *pImp = ScFormatFilter::Get().CreateHTMLImport( pDoc, rBaseURL, aRange, TRUE);
    if (!pImp)
        return false;
    pImp->Read( rStrm, rBaseURL );
    aRange = pImp->GetRange();

    BOOL bOk = StartPaste();
    if (bOk)
    {
        // ScHTMLImport may call ScDocument::InitDrawLayer, resulting in
        // a Draw Layer but no Draw View -> create Draw Layer and View here
        if (pDocSh)
            pDocSh->MakeDrawLayer();

        USHORT nFlags = IDF_ALL & ~IDF_STYLES;
        pDoc->DeleteAreaTab( aRange, nFlags );

        if (pExtOptions)
        {
            // Pick up import options if available.
            LanguageType eLang = pExtOptions->GetLanguage();
            SvNumberFormatter aNumFormatter(pDoc->GetServiceManager(), eLang);
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

#define RETURN_ERROR { return eERR_INTERN; }
class ScFormatFilterMissing : public ScFormatFilterPlugin {
  public:
    ScFormatFilterMissing()
    {
      OSL_FAIL("Missing file filters");
    }
    virtual FltError ScImportLotus123( SfxMedium&, ScDocument*, CharSet ) RETURN_ERROR
    virtual FltError ScImportQuattroPro( SfxMedium &, ScDocument * ) RETURN_ERROR
    virtual FltError ScImportExcel( SfxMedium&, ScDocument*, const EXCIMPFORMAT ) RETURN_ERROR
    virtual FltError ScImportStarCalc10( SvStream&, ScDocument* ) RETURN_ERROR
    virtual FltError ScImportDif( SvStream&, ScDocument*, const ScAddress&,
                 const CharSet, UINT32 ) RETURN_ERROR
    virtual FltError ScImportRTF( SvStream&, const String&, ScDocument*, ScRange& ) RETURN_ERROR
    virtual FltError ScImportHTML( SvStream&, const String&, ScDocument*, ScRange&, double, BOOL, SvNumberFormatter*, bool ) RETURN_ERROR

    virtual ScEEAbsImport *CreateRTFImport( ScDocument*, const ScRange& ) { return NULL; }
    virtual ScEEAbsImport *CreateHTMLImport( ScDocument*, const String&, const ScRange&, BOOL ) { return NULL; }
    virtual String         GetHTMLRangeNameList( ScDocument*, const String& ) { return String(); }

#if ENABLE_LOTUS123_EXPORT
    virtual FltError ScExportLotus123( SvStream&, ScDocument*, ExportFormatLotus, CharSet ) RETURN_ERROR
#endif
    virtual FltError ScExportExcel5( SfxMedium&, ScDocument*, ExportFormatExcel, CharSet ) RETURN_ERROR
    virtual FltError ScExportDif( SvStream&, ScDocument*, const ScAddress&, const CharSet, UINT32 ) RETURN_ERROR
    virtual FltError ScExportDif( SvStream&, ScDocument*, const ScRange&, const CharSet, UINT32 ) RETURN_ERROR
    virtual FltError ScExportHTML( SvStream&, const String&, ScDocument*, const ScRange&, const CharSet, BOOL,
                  const String&, String& ) RETURN_ERROR
    virtual FltError ScExportRTF( SvStream&, ScDocument*, const ScRange&, const CharSet ) RETURN_ERROR
};

extern "C" { static void SAL_CALL thisModule() {} }
typedef ScFormatFilterPlugin * (*FilterFn)(void);
ScFormatFilterPlugin &ScFormatFilter::Get()
{
    static ScFormatFilterPlugin *plugin;

    if (plugin != NULL)
        return *plugin;

    static ::osl::Module aModule;
    if ( aModule.loadRelative( &thisModule,
                   ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SVLIBRARY( "scfilt" ) ) ) ) )
    {
    oslGenericFunction fn = aModule.getFunctionSymbol( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ScFilterCreate" )) );
    if (fn != NULL)
        plugin = reinterpret_cast<FilterFn>(fn)();
    }
    if (plugin == NULL)
        plugin = new ScFormatFilterMissing();

    return *plugin;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
