/*************************************************************************
 *
 *  $RCSfile: impex.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-28 15:44:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// System - Includes -----------------------------------------------------

class StarBASIC;

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include "sc.hrc"
#define GLOBALOVERFLOW
#endif

// INCLUDE ---------------------------------------------------------------

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include <tools/list.hxx>
#include <tools/string.hxx>
#include <rtl/math.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/zforlist.hxx>
#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>
#include <sot/formats.hxx>
#include <sfx2/mieclip.hxx>
#include <unotools/charclass.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#ifndef _COM_SUN_STAR_I18N_CALENDARFIELDINDEX_HPP_
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#endif
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif

#include "global.hxx"
#include "docsh.hxx"
#include "undoblk.hxx"
#include "rangenam.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "filter.hxx"
#include "asciiopt.hxx"
#include "cell.hxx"
#include "rtfimp.hxx"
#include "htmlimp.hxx"
#include "docoptio.hxx"
#include "progress.hxx"
#include "scitems.hxx"
#include "editable.hxx"

#include "impex.hxx"

#include "globstr.hrc"

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

//========================================================================


// Gesamtdokument ohne Undo


ScImportExport::ScImportExport( ScDocument* p )
    : pDoc( p ), pDocSh( PTR_CAST(ScDocShell,p->GetDocumentShell()) ),
      nSizeLimit( 0 ), bSingle( TRUE ), bAll( TRUE ), bUndo( FALSE ),
      cSep( '\t' ), cStr( '"' ), bFormulas( FALSE ), bIncludeFiltered( TRUE ),
      bOverflow( FALSE )
{
    pUndoDoc = NULL;
    pExtOptions = NULL;
}

// Insert am Punkt ohne Bereichschecks


ScImportExport::ScImportExport( ScDocument* p, const ScAddress& rPt )
    : pDoc( p ), pDocSh( PTR_CAST(ScDocShell,p->GetDocumentShell()) ),
      aRange( rPt ),
      nSizeLimit( 0 ), bSingle( TRUE ), bAll( FALSE ), bUndo( BOOL( pDocSh != NULL ) ),
      cSep( '\t' ), cStr( '"' ), bFormulas( FALSE ), bIncludeFiltered( TRUE ),
      bOverflow( FALSE )
{
    pUndoDoc = NULL;
    pExtOptions = NULL;
}


//  ctor with a range is only used for export
//! ctor with a string (and bSingle=TRUE) is also used for DdeSetData

ScImportExport::ScImportExport( ScDocument* p, const ScRange& r )
    : pDoc( p ), pDocSh( PTR_CAST(ScDocShell,p->GetDocumentShell()) ),
      aRange( r ),
      nSizeLimit( 0 ), bSingle( FALSE ), bAll( FALSE ), bUndo( BOOL( pDocSh != NULL ) ),
      cSep( '\t' ), cStr( '"' ), bFormulas( FALSE ), bIncludeFiltered( TRUE ),
      bOverflow( FALSE )
{
    pUndoDoc = NULL;
    pExtOptions = NULL;
    // Zur Zeit nur in einer Tabelle!
    aRange.aEnd.SetTab( aRange.aStart.Tab() );
}

// String auswerten: Entweder Bereich, Punkt oder Gesamtdoc (bei Fehler)
// Falls eine View existiert, wird die TabNo der View entnommen!


ScImportExport::ScImportExport( ScDocument* p, const String& rPos )
    : pDoc( p ), pDocSh( PTR_CAST(ScDocShell,p->GetDocumentShell()) ),
      nSizeLimit( 0 ), bSingle( TRUE ), bAll( FALSE ), bUndo( BOOL( pDocSh != NULL ) ),
      cSep( '\t' ), cStr( '"' ), bFormulas( FALSE ), bIncludeFiltered( TRUE ),
      bOverflow( FALSE )
{
    pUndoDoc = NULL;
    pExtOptions = NULL;

    USHORT nTab = ScDocShell::GetCurTab();
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
    // Bereich?
    if( aRange.Parse( aPos, pDoc ) & SCA_VALID )
        bSingle = FALSE;
    // Zelle?
    else if( aRange.aStart.Parse( aPos, pDoc ) & SCA_VALID )
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
        pDoc->CopyToDocument( aRange, IDF_ALL, FALSE, pUndoDoc );
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
        pDoc->CopyToDocument( aRange, IDF_ALL, FALSE, pRedoDoc );
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


#if 0
BOOL ScImportExport::ImportData( SvData& rData )
{
    ULONG nFmt = rData.GetFormat();
    if ( nFmt == SOT_FORMATSTR_ID_HTML_SIMPLE )
    {
        MSE40HTMLClipFormatObj aMSE40ClpObj;
        if ( aMSE40ClpObj.GetData( rData ) )
        {
            SvStream* pStream = aMSE40ClpObj.GetStream();
            return ImportStream( *pStream, nFmt );
        }
        return FALSE;
    }
    else
    {
        void* pMem;
        ULONG nSize = rData.GetMinMemorySize();
        rData.GetData( &pMem, TRANSFER_REFERENCE );
        if( nFmt == FORMAT_STRING
                 || nFmt == FORMAT_RTF
                 || nFmt == SOT_FORMATSTR_ID_SYLK
                 || nFmt == SOT_FORMATSTR_ID_HTML
                 || nFmt == SOT_FORMATSTR_ID_DIF )
        {
            //! String? Unicode??

            // Stringende ermitteln!
            sal_Char* pBegin = (sal_Char*) pMem;
            sal_Char* pEnd   = (sal_Char*) pMem + nSize;

            nSize = 0;
            while( pBegin != pEnd && *pBegin != '\0' )
                pBegin++, nSize++;
            // #72909# MT says only STRING has to be zero-terminated
            DBG_ASSERT( pBegin != pEnd || nFmt != FORMAT_STRING, "non zero-terminated String" )
        }
        SvMemoryStream aStrm( pMem, nSize, STREAM_READ );
        return ImportStream( aStrm, nFmt );
    }
}

#endif

BOOL ScImportExport::ImportData( const String& rMimeType,
                     const ::com::sun::star::uno::Any & rValue )
{
    DBG_ASSERT( !this, "Implementation is missing" );
    return FALSE;
}

BOOL ScImportExport::ExportData( const String& rMimeType,
                                 ::com::sun::star::uno::Any & rValue )
{
    SvMemoryStream aStrm;
    if( ExportStream( aStrm,
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


// static
inline void ScImportExport::SetNoEndianSwap( SvStream& rStrm )
{
#ifdef __BIGENDIAN
    rStrm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
#else
    rStrm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
#endif
}


BOOL ScImportExport::ImportString( const ::rtl::OUString& rText, ULONG nFmt )
{
    switch ( nFmt )
    {
        // formats supporting unicode
        case FORMAT_STRING :
        {
            SvMemoryStream aStrm( (void*)rText.getStr(), rText.getLength() * sizeof(sal_Unicode), STREAM_READ );
            aStrm.SetStreamCharSet( RTL_TEXTENCODING_UNICODE );
            SetNoEndianSwap( aStrm );       //! no swapping in memory
            return ImportStream( aStrm, nFmt );
            // ImportStream must handle RTL_TEXTENCODING_UNICODE
        }
        break;
        default:
        {
            rtl_TextEncoding eEnc = gsl_getSystemTextEncoding();
            ::rtl::OString aTmp( rText.getStr(), rText.getLength(), eEnc );
            SvMemoryStream aStrm( (void*)aTmp.getStr(), aTmp.getLength() * sizeof(sal_Char), STREAM_READ );
            aStrm.SetStreamCharSet( eEnc );
            SetNoEndianSwap( aStrm );       //! no swapping in memory
            return ImportStream( aStrm, nFmt );
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
    if( ExportStream( aStrm, nFmt ) )
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
    if( ExportStream( aStrm, nFmt ) )
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


BOOL ScImportExport::ImportStream( SvStream& rStrm, ULONG nFmt )
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
        if( RTF2Doc( rStrm ) )
            return TRUE;
    }
    if( nFmt == SOT_FORMATSTR_ID_LINK )
        return TRUE;            // Link-Import?
    if ( nFmt == SOT_FORMATSTR_ID_HTML )
    {
        if( HTML2Doc( rStrm ) )
            return TRUE;
    }
    if ( nFmt == SOT_FORMATSTR_ID_HTML_SIMPLE )
    {
        MSE40HTMLClipFormatObj aMSE40ClpObj;                // needed to skip the header data
        SvStream* pHTML = aMSE40ClpObj.IsValid( rStrm );
        if ( pHTML && HTML2Doc( *pHTML ) )
            return TRUE;
    }

    return FALSE;
}


BOOL ScImportExport::ExportStream( SvStream& rStrm, ULONG nFmt )
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
                aRange.aStart.Format( aRefName, nFlags, pDoc );
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
        if( Doc2HTML( rStrm ) )
            return TRUE;
    }
    if( nFmt == FORMAT_RTF )
    {
        if( Doc2RTF( rStrm ) )
            return TRUE;
    }

    return FALSE;
}


//static
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


// static
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

const sal_Unicode* lcl_ScanString( const sal_Unicode* p, String& rString,
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
                            rString.Append( p0, (p-1) - p0 );   // first part
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
            rString.Append( p0, ((*p || *(p-1) == cStr) ? p-1 : p) - p0 );
    } while ( bCont );
    return p;
}


void lcl_WriteString( SvStream& rStrm, String& rString, sal_Unicode cStr )
{
    xub_StrLen n = 0;
    while( ( n = rString.Search( cStr, n ) ) != STRING_NOTFOUND )
    {
        rString.Insert( cStr, n );
        n += 2;
    }

    rString.Insert( cStr, 0 );
    rString.Append( cStr );

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

    USHORT nStartCol = aRange.aStart.Col();
    USHORT nStartRow = aRange.aStart.Row();
    USHORT nEndCol = aRange.aEnd.Col();
    USHORT nEndRow = aRange.aEnd.Row();
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
        USHORT nRow = nStartRow;
        rStrm.Seek( nOldPos );
        for( ;; )
        {
            rStrm.ReadUniOrByteStringLine( aLine );
            if( rStrm.IsEof() )
                break;
            USHORT nCol = nStartCol;
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
                    aCell.Assign( q, p - q );
                    if( *p )
                        p++;
                }
                if (nCol<=MAXCOL && nRow<=MAXROW )
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


void lcl_PutString( ScDocument* pDoc, USHORT nCol, USHORT nRow, USHORT nTab,
                    const String& rStr, BYTE nColFormat,
                    ::utl::TransliterationWrapper& rTransliteration,
                    CalendarWrapper& rCalendar,
                    ::utl::TransliterationWrapper* pSecondTransliteration,
                    CalendarWrapper* pSecondCalendar )
{
    if ( nColFormat == SC_COL_SKIP || !rStr.Len() || nCol > MAXCOL || nRow > MAXROW )
        return;

    if ( nColFormat == SC_COL_TEXT )
    {
        pDoc->PutCell( nCol, nRow, nTab, new ScStringCell( rStr ) );
        return;
    }

    if ( nColFormat == SC_COL_ENGLISH )
    {
        //! SetString mit Extra-Flag ???

        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
        ULONG nEnglish = pFormatter->GetStandardIndex(LANGUAGE_ENGLISH_US);
        double fVal;
        if ( pFormatter->IsNumberFormat( rStr, nEnglish, fVal ) )
        {
            //  Zahlformat wird nicht auf englisch gesetzt
            pDoc->SetValue( nCol, nRow, nTab, fVal );
            return;
        }
        //  sonst weiter mit SetString
    }
    else if ( nColFormat != SC_COL_STANDARD )                   // Datumsformate
    {
        //  nach genau drei Teilen suchen

        xub_StrLen nLen = rStr.Len();
        xub_StrLen nStart[3];
        xub_StrLen nEnd[3];
        USHORT nFound = 0;
        BOOL bInNum = FALSE;
        for ( xub_StrLen nPos=0; nPos<nLen; nPos++ )
        {
            if ( ScGlobal::pCharClass->isLetterNumeric( rStr, nPos ) )
            {
                if (!bInNum)
                {
                    if ( nFound >= 3 )
                        break;                  // zuviele Teile
                    bInNum = TRUE;
                    nStart[nFound] = nPos;
                    ++nFound;
                }
                nEnd[nFound-1] = nPos;
            }
            else
                bInNum = FALSE;
        }

        USHORT nDP, nMP, nYP;
        switch ( nColFormat )
        {
            case SC_COL_YMD: nDP = 2; nMP = 1; nYP = 0; break;
            case SC_COL_MDY: nDP = 1; nMP = 0; nYP = 2; break;
            case SC_COL_DMY:
            default:         nDP = 0; nMP = 1; nYP = 2; break;
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

        if ( nFound == 3 )              // exactly 3 parts found?
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
                sal_Int32 i, nLen;
                //  first test all month names from local international
                xMonths = rCalendar.getMonths();
                nLen = xMonths.getLength();
                for (i=0; i<nLen && !nMonth; i++)
                {
                    if ( rTransliteration.isEqual( aMStr, xMonths[i].FullName ) ||
                         rTransliteration.isEqual( aMStr, xMonths[i].AbbrevName ) )
                        nMonth = i+1;
                    else if ( i == 8 && rTransliteration.isEqual( aSeptCorrect,
                                xMonths[i].AbbrevName ) &&
                            rTransliteration.isEqual( aMStr, aSepShortened ) )
                    {   // #102136# correct English abbreviation is SEPT,
                        // but data mostly contains SEP only
                        nMonth = i+1;
                    }
                }
                //  if none found, then test english month names
                if ( !nMonth && pSecondCalendar && pSecondTransliteration )
                {
                    xMonths = pSecondCalendar->getMonths();
                    nLen = xMonths.getLength();
                    for (i=0; i<nLen && !nMonth; i++)
                    {
                        if ( pSecondTransliteration->isEqual( aMStr, xMonths[i].FullName ) ||
                             pSecondTransliteration->isEqual( aMStr, xMonths[i].AbbrevName ) )
                        {
                            nMonth = i+1;
                            bSecondCal = TRUE;
                        }
                        else if ( i == 8 && pSecondTransliteration->isEqual(
                                    aMStr, aSepShortened ) )
                        {   // #102136# correct English abbreviation is SEPT,
                            // but data mostly contains SEP only
                            nMonth = i+1;
                            bSecondCal = TRUE;
                        }
                    }
                }
            }

            SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
            if ( nYear < 100 )
                nYear = pFormatter->ExpandTwoDigitYear( nYear );

            CalendarWrapper* pCalendar = (bSecondCal ? pSecondCalendar : &rCalendar);
            sal_Int16 nNumMonths = pCalendar->getNumberOfMonthsInYear();
            if ( nDay && nMonth && nDay<=31 && nMonth<=nNumMonths )
            {
                --nMonth;
                pCalendar->setValue( i18n::CalendarFieldIndex::DAY_OF_MONTH, nDay );
                pCalendar->setValue( i18n::CalendarFieldIndex::MONTH, nMonth );
                pCalendar->setValue( i18n::CalendarFieldIndex::YEAR, nYear );
                if ( pCalendar->isValid() )
                {
                    double fDiff = DateTime(*pFormatter->GetNullDate()) -
                        pCalendar->getEpochStart();
                    double fDays = pCalendar->getDateTime();
                    fDays -= fDiff;

                    LanguageType eLatin, eCjk, eCtl;
                    pDoc->GetLanguage( eLatin, eCjk, eCtl );
                    LanguageType eDocLang = eLatin;     //! which language for date formats?

                    long nFormat = pFormatter->GetStandardFormat( NUMBERFORMAT_DATE, eDocLang );

                    pDoc->PutCell( nCol, nRow, nTab, new ScValueCell(fDays), nFormat, FALSE );

                    return;     // success
                }
            }
        }
    }

    //  Standard oder Datum nicht erkannt -> SetString

    pDoc->SetString( nCol, nRow, nTab, rStr );
}


String lcl_GetFixed( const String& rLine, xub_StrLen nStart, xub_StrLen nNext )
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

    return rLine.Copy( nStart, nSpace-nStart );
}


BOOL ScImportExport::ExtText2Doc( SvStream& rStrm )
{
    if (!pExtOptions)
        return Text2Doc( rStrm );

    ULONG nOldPos = rStrm.Tell();
    rStrm.Seek( STREAM_SEEK_TO_END );
    ScProgress aProgress( pDocSh, ScGlobal::GetRscString( STR_LOAD_DOC ), rStrm.Tell() - nOldPos );
    rStrm.Seek( nOldPos );
    if ( rStrm.GetStreamCharSet() == RTL_TEXTENCODING_UNICODE )
        rStrm.StartReadingUnicodeText();

    BOOL bOld = ScColumn::bDoubleAlloc;
    ScColumn::bDoubleAlloc = TRUE;

    DBG_ASSERT( !bUndo, "ExtText2Doc mit Undo noch nicht implementiert!" );
    USHORT nStartCol = aRange.aStart.Col();
    USHORT nStartRow = aRange.aStart.Row();
    USHORT nTab = aRange.aStart.Tab();

    BOOL    bFixed          = pExtOptions->IsFixedLen();
    const sal_Unicode* pSeps = pExtOptions->GetFieldSeps().GetBuffer();
    BOOL    bMerge          = pExtOptions->IsMergeSeps();
    USHORT  nInfoCount      = pExtOptions->GetInfoCount();
    const xub_StrLen* pColStart = pExtOptions->GetColStart();
    const BYTE* pColFormat  = pExtOptions->GetColFormat();
    long nSkipLines = pExtOptions->GetStartRow();

    LanguageType eLatin, eCjk, eCtl;
    pDoc->GetLanguage( eLatin, eCjk, eCtl );
    LanguageType eDocLang = eLatin;                 //! which language for date formats?

    // For date recognition
    ::utl::TransliterationWrapper aTransliteration(
        pDoc->GetServiceManager(), SC_TRANSLITERATION_IGNORECASE );
    aTransliteration.loadModuleIfNeeded( eDocLang );
    CalendarWrapper aCalendar( pDoc->GetServiceManager() );
    aCalendar.loadDefaultCalendar(
        SvNumberFormatter::ConvertLanguageToLocale( eDocLang ) );
    ::utl::TransliterationWrapper* pEnglishTransliteration = NULL;
    CalendarWrapper* pEnglishCalendar = NULL;
    if ( eDocLang != LANGUAGE_ENGLISH_US )
    {
        pEnglishTransliteration = new ::utl::TransliterationWrapper (
            pDoc->GetServiceManager(), SC_TRANSLITERATION_IGNORECASE );
        aTransliteration.loadModuleIfNeeded( LANGUAGE_ENGLISH_US );
        pEnglishCalendar = new CalendarWrapper ( pDoc->GetServiceManager() );
        pEnglishCalendar->loadDefaultCalendar(
            SvNumberFormatter::ConvertLanguageToLocale( LANGUAGE_ENGLISH_US ) );
    }

    String aLine, aCell;
    USHORT i;
    USHORT nRow = nStartRow;

    while(--nSkipLines>0)
    {
        rStrm.ReadUniOrByteStringLine( aLine );     // content is ignored
        if ( rStrm.IsEof() )
            break;
    }
    for( ;; )
    {
        rStrm.ReadUniOrByteStringLine( aLine );
        if ( rStrm.IsEof() )
            break;

        xub_StrLen nLineLen = aLine.Len();
        USHORT nCol = nStartCol;
        if ( bFixed )               //  Feste Satzlaenge
        {
            for ( i=0; i<nInfoCount; i++ )
            {
                if ( pColFormat[i] != SC_COL_SKIP )     // sonst auch nCol nicht hochzaehlen
                {
                    xub_StrLen nStart = pColStart[i];
                    xub_StrLen nNext = ( i+1 < nInfoCount ) ? pColStart[i+1] : nLineLen;
                    aCell = lcl_GetFixed( aLine, nStart, nNext );
                    lcl_PutString( pDoc, nCol, nRow, nTab, aCell, pColFormat[i],
                        aTransliteration, aCalendar, pEnglishTransliteration, pEnglishCalendar );
                    ++nCol;
                }
            }
        }
        else                        //  Nach Trennzeichen suchen
        {
            USHORT nSourceCol = 0;
            USHORT nInfoStart = 0;
            const sal_Unicode* p = aLine.GetBuffer();
            while (*p)
            {
                p = ScImportExport::ScanNextFieldFromString( p, aCell, cStr, pSeps, bMerge );

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
                    lcl_PutString( pDoc, nCol, nRow, nTab, aCell, nFmt,
                        aTransliteration, aCalendar, pEnglishTransliteration, pEnglishCalendar );
                    ++nCol;
                }

                ++nSourceCol;
            }
        }

        aProgress.SetStateOnPercent( rStrm.Tell() - nOldPos );
        ++nRow;
        if ( nRow > MAXROW )
        {
            bOverflow = TRUE;           // beim Import Warnung ausgeben
            break;
        }
    }

    ScColumn::bDoubleAlloc = bOld;
    pDoc->DoColResize( nTab, 0, MAXCOL, 0 );

    delete pEnglishTransliteration;
    delete pEnglishCalendar;

    return TRUE;
}


// static
const sal_Unicode* ScImportExport::ScanNextFieldFromString( const sal_Unicode* p,
        String& rField, sal_Unicode cStr, const sal_Unicode* pSeps, BOOL bMergeSeps )
{
    rField.Erase();
    if ( *p == cStr )           // String in Anfuehrungszeichen
    {
        p = lcl_ScanString( p, rField, cStr, DQM_ESCAPE );
        while ( *p && !ScGlobal::UnicodeStrChr( pSeps, *p ) )
            p++;
        if( *p )
            p++;
    }
    else                        // bis zum Trennzeichen
    {
        const sal_Unicode* p0 = p;
        while ( *p && !ScGlobal::UnicodeStrChr( pSeps, *p ) )
            p++;
        rField.Append( p0, p - p0 );
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

        //
        //
        //


BOOL ScImportExport::Doc2Text( SvStream& rStrm )
{
    USHORT nCol;
    USHORT nRow;
    USHORT nStartCol = aRange.aStart.Col();
    USHORT nStartRow = aRange.aStart.Row();
    USHORT nEndCol = aRange.aEnd.Col();
    USHORT nEndRow = aRange.aEnd.Row();
    String aCell;

    for (nRow = nStartRow; nRow <= nEndRow; nRow++)
    {
        if (bIncludeFiltered || !pDoc->IsFiltered( nRow, aRange.aStart.Tab() ))
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
                            pDoc->GetFormula( nCol, nRow, aRange.aStart.Tab(), aCell, TRUE );
                            if( aCell.Search( cSep ) != STRING_NOTFOUND )
                                lcl_WriteString( rStrm, aCell, cStr );
                            else
                                lcl_WriteSimpleString( rStrm, aCell );
                        }
                        else
                        {
                            pDoc->GetString( nCol, nRow, aRange.aStart.Tab(), aCell );
                            if( aCell.Search( cSep ) != STRING_NOTFOUND )
                                lcl_WriteString( rStrm, aCell, cStr );
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
                        if( aCell.Search( cSep ) != STRING_NOTFOUND )
                            lcl_WriteString( rStrm, aCell, cStr );
                        else
                            lcl_WriteSimpleString( rStrm, aCell );
                    }
                }
                if( nCol < nEndCol )
                    lcl_WriteSimpleString( rStrm, String(cSep) );
            }
//          if( nRow < nEndRow )
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

    // US-English separators for StringToDouble
    sal_Unicode cDecSep = '.';
    sal_Unicode cGrpSep = ',';

    USHORT nStartCol = aRange.aStart.Col();
    USHORT nStartRow = aRange.aStart.Row();
    USHORT nEndCol = aRange.aEnd.Col();
    USHORT nEndRow = aRange.aEnd.Row();
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
        USHORT nCol = nStartCol;
        USHORT nRow = nStartRow;
        USHORT nRefCol, nRefRow;
        nRefCol = nRefRow = 1;
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
                            nCol = String( p ).ToInt32() + nStartCol - 1;
                            break;
                        case 'Y':
                            nRow = String( p ).ToInt32() + nStartRow - 1;
                            break;
                        case 'C':
                            nRefCol = String( p ).ToInt32() + nStartCol - 1;
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
                                aText = '\'';       // force string cell
                                p = lcl_ScanString( p, aText, '"', DQM_ESCAPE );
                            }
                            else
                                bText = FALSE;
                            const sal_Unicode* q = p;
                            while( *q && *q != ';' )
                                q++;
                            if ( !(*q == ';' && *(q+1) == 'I') )
                            {   // don't ignore value
                                if( bText )
                                    pDoc->SetString( nCol, nRow, aRange.aStart.Tab(), aText );
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
                            if( *p == '"' )
                                p = lcl_ScanString( p, aText, '"', DQM_ESCAPE );
                            else
                            {
                                const sal_Unicode* q = p;
                                while( *p && *p != ';' )
                                    p++;
                                aText.Append( q, p-q );
                            }
                            ScAddress aPos( nCol, nRow, aRange.aStart.Tab() );
                            ScCompiler aComp( pDoc, aPos );
                            aComp.SetCompileEnglish( TRUE );
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
                                ScFormulaCell* pFCell = new ScFormulaCell( pDoc, aPos, pCode, 0 );
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
                            nCol = String( p ).ToInt32() + nStartCol - 1;
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
                                String aNumber( p0, p - p0 );
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
                    ULONG nKey;
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
                bMyDoc = aLine.EqualsAscii( "SCALC3" );
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
    USHORT nCol;
    USHORT nRow;
    USHORT nStartCol = aRange.aStart.Col();
    USHORT nStartRow = aRange.aStart.Row();
    USHORT nEndCol = aRange.aEnd.Col();
    USHORT nEndRow = aRange.aEnd.Row();
    String aCellStr;
    String aValStr;
    lcl_WriteSimpleString( rStrm,
            String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM( "ID;PSCALC3" )) );
    WriteUnicodeOrByteEndl( rStrm );

    for (nRow = nStartRow; nRow <= nEndRow; nRow++)
    {
        for (nCol = nStartCol; nCol <= nEndCol; nCol++)
        {
            String aBufStr;
            double nVal;
            BOOL bForm = FALSE;
            USHORT r = nRow - nStartRow + 1;
            USHORT c = nCol - nStartCol + 1;
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

                    aBufStr.AssignAscii(RTL_CONSTASCII_STRINGPARAM( "C;X" ));
                    aBufStr += String::CreateFromInt32( c );
                    aBufStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ";Y" ));
                    aBufStr += String::CreateFromInt32( r );
                    aBufStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ";K" ));
                    lcl_WriteSimpleString( rStrm, aBufStr );
                    lcl_WriteString( rStrm, aCellStr, '"' );

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
                                pFCell->GetEnglishFormula( aCellStr );
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
                                USHORT nC, nR;
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
                        {
                            if( aCellStr.Search( ';' ) != STRING_NOTFOUND )
                                lcl_WriteString( rStrm, aCellStr, '"' );
                            else
                                lcl_WriteSimpleString( rStrm, aCellStr );
                        }
                    }
                    WriteUnicodeOrByteEndl( rStrm );
                    break;
            }
        }
    }
    lcl_WriteSimpleString( rStrm, String( 'E' ) );
    WriteUnicodeOrByteEndl( rStrm );
    return BOOL( rStrm.GetError() == SVSTREAM_OK );
}


BOOL ScImportExport::Doc2HTML( SvStream& rStrm )
{
    // CharSet is ignored in ScExportHTML, read from Load/Save HTML options
    ScExportHTML( rStrm, pDoc, aRange, RTL_TEXTENCODING_DONTKNOW, bAll,
        aStreamPath, aNonConvertibleChars );
    return BOOL( rStrm.GetError() == SVSTREAM_OK );
}

BOOL ScImportExport::Doc2RTF( SvStream& rStrm )
{
    //  CharSet is ignored in ScExportRTF
    ScExportRTF( rStrm, pDoc, aRange, RTL_TEXTENCODING_DONTKNOW );
    return BOOL( rStrm.GetError() == SVSTREAM_OK );
}


BOOL ScImportExport::Doc2Dif( SvStream& rStrm )
{
    // for DIF in the clipboard, IBM_850 is always used
    ScExportDif( rStrm, pDoc, aRange, RTL_TEXTENCODING_IBM_850 );
    return TRUE;
}


BOOL ScImportExport::Dif2Doc( SvStream& rStrm )
{
    USHORT nTab = aRange.aStart.Tab();
    ScDocument* pImportDoc = new ScDocument( SCDOCMODE_UNDO );
    pImportDoc->InitUndo( pDoc, nTab, nTab );

    // for DIF in the clipboard, IBM_850 is always used
    ScImportDif( rStrm, pImportDoc, aRange.aStart, RTL_TEXTENCODING_IBM_850 );

    USHORT nEndCol, nEndRow;
    pImportDoc->GetCellArea( nTab, nEndCol, nEndRow );
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


BOOL ScImportExport::RTF2Doc( SvStream& rStrm )
{
    ScRTFImport aImp( pDoc, aRange );
    aImp.Read( rStrm );
    aRange = aImp.GetRange();

    BOOL bOk = StartPaste();
    if (bOk)
    {
        USHORT nFlags = IDF_ALL & ~IDF_STYLES;
        pDoc->DeleteAreaTab( aRange, nFlags );
        aImp.WriteToDocument();
        EndPaste();
    }

    return bOk;
}


BOOL ScImportExport::HTML2Doc( SvStream& rStrm )
{
    ScHTMLImport aImp( pDoc, aRange );
    aImp.Read( rStrm );
    aRange = aImp.GetRange();

    BOOL bOk = StartPaste();
    if (bOk)
    {
        //  ScHTMLImport may call ScDocument::InitDrawLayer, resulting in
        //  a DrawLayer but no DrawView -> create DrawLayer and View here
        if (pDocSh)
            pDocSh->MakeDrawLayer();

        USHORT nFlags = IDF_ALL & ~IDF_STYLES;
        pDoc->DeleteAreaTab( aRange, nFlags );
        aImp.WriteToDocument();
        EndPaste();
    }

    return bOk;
}


