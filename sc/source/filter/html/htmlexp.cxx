/*************************************************************************
 *
 *  $RCSfile: htmlexp.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-13 18:05:05 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#define _SVSTDARR_STRINGSSORTDTOR

#ifndef _RTL_TENCINFO_H //autogen wg. rtl_getBestMimeCharsetFromTextEncoding, rtl_getTextEncodingFromMimeCharset
#include <rtl/tencinfo.h>
#endif

#include <svx/algitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/colritem.hxx>
#include <svx/fhgtitem.hxx>
#include <svx/fontitem.hxx>
#include <svx/postitem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/wghtitem.hxx>
#include <svx/xoutbmp.hxx>
#ifndef _MyEDITENG_HXX //autogen wg. EditEngine
#include <svx/editeng.hxx>
#endif
#ifndef _EEITEM_HXX //autogen wg. EE_FEATURE_FIELD
#include <svx/eeitem.hxx>
#endif
#include <offmgr/app.hxx>
#include <offmgr/htmlcfg.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docinf.hxx>
#include <sfx2/frmhtmlw.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/stritem.hxx>
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTS
#endif
#include <svtools/svstdarr.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/parhtml.hxx>
#include <vcl/outdev.hxx>
#include <tools/intn.hxx>
#include <stdio.h>

#if defined(GetNumberFormat) && SUPD<356
// xoutbmp.hxx -> svimbase.hxx -> sysdep.hxx -> windows.h ->
// define GetNumberFormat GetNumberFormatA
#undef GetNumberFormat
#endif

#include "htmlexp.hxx"
#include "filter.hxx"
#include "flttools.hxx"
#include "global.hxx"
#include "document.hxx"
#include "scitems.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "stlpool.hxx"
#include "scresid.hxx"
#include "cell.hxx"
#include "cellform.hxx"
#include "docoptio.hxx"
#include "editutil.hxx"

#define ITEMID_FIELD EE_FEATURE_FIELD
#include <svx/flditem.hxx>
#undef ITEMID_FIELD


// ohne sc.hrc: error C2679: binary '=' : no operator defined which takes a
// right-hand operand of type 'const class String (__stdcall *)(class ScResId)'
// bei
// const String aStrTable( ScResId( SCSTR_TABLE ) ); aStrOut = aStrTable;
// ?!???
#include "sc.hrc"
#include "globstr.hrc"

//========================================================================

const static sal_Char __FAR_DATA sMyBegComment[]    = "<!-- ";
const static sal_Char __FAR_DATA sMyEndComment[]    = " -->";
const static sal_Char __FAR_DATA sFontFamily[]      = "font-family: ";
const static sal_Char __FAR_DATA sFontSize[]        = "font-size: ";

const USHORT __FAR_DATA ScHTMLExport::nDefaultFontSize[SC_HTML_FONTSIZES] =
{
    HTMLFONTSZ1_DFLT, HTMLFONTSZ2_DFLT, HTMLFONTSZ3_DFLT, HTMLFONTSZ4_DFLT,
    HTMLFONTSZ5_DFLT, HTMLFONTSZ6_DFLT, HTMLFONTSZ7_DFLT
};

USHORT ScHTMLExport::nFontSize[SC_HTML_FONTSIZES] = { 0 };

const USHORT ScHTMLExport::nCellSpacing = 0;
const sal_Char __FAR_DATA ScHTMLExport::sIndentSource[nIndentMax+1] =
    "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

//========================================================================
// Makros fuer HTML-Export
//========================================================================
#define OUT_PROLOGUE()      (rStrm << sHTML30_Prologue << ScExportBase::sNewLine \
                                << ScExportBase::sNewLine)
#define TAG_ON( tag )       HTMLOutFuncs::Out_AsciiTag( rStrm, tag )
#define TAG_OFF( tag )      HTMLOutFuncs::Out_AsciiTag( rStrm, tag, FALSE )
#define OUT_STR( str )      HTMLOutFuncs::Out_String( rStrm, str, eDestEnc )
#define OUT_LF()            rStrm << ScExportBase::sNewLine << GetIndentStr()
#define lcl_OUT_LF()        rStrm << ScExportBase::sNewLine
#define TAG_ON_LF( tag )    (TAG_ON( tag ) << ScExportBase::sNewLine << GetIndentStr())
#define TAG_OFF_LF( tag )   (TAG_OFF( tag ) << ScExportBase::sNewLine << GetIndentStr())
#define OUT_HR()            TAG_ON_LF( sHTML_horzrule )
#define OUT_COMMENT( comment )  (rStrm << sMyBegComment, OUT_STR( comment ) \
                                << sMyEndComment << ScExportBase::sNewLine \
                                << GetIndentStr())
#define lcl_OUT_COMMENT( comment )  (rStrm << sMyBegComment, OUT_STR( comment ) \
                                << sMyEndComment << ScExportBase::sNewLine)

#define OUT_SP_CSTR_ASS( s )    rStrm << ' ' << s << '='
#define APPEND_SPACE( s )   s.AppendAscii(" ")

extern BOOL bOderSo;

#define GLOBSTR(id) ScGlobal::GetRscString( id )



//========================================================================

FltError ScExportHTML( SvStream& rStrm, ScDocument* pDoc,
        const ScRange& rRange, const CharSet eNach, BOOL bAll,
        const String& rStreamPath )
{
    ScHTMLExport aEx( rStrm, pDoc, rRange, bAll, rStreamPath );
    return aEx.Write();
}


void lcl_AddStamp( String& rStr, const SfxStamp& rStamp, International& rInter )
{
    const DateTime& rDateTime   = rStamp.GetTime();
    String          aStrDate    = rInter.GetDate( rDateTime );
    String          aStrTime    = rInter.GetTime( rDateTime );

    rStr += GLOBSTR( STR_BY );
    APPEND_SPACE( rStr );
    if (rStamp.GetName().Len())
        rStr += rStamp.GetName();
    else
        rStr.AppendAscii( "???" );
    APPEND_SPACE( rStr );
    rStr += GLOBSTR( STR_ON );
    APPEND_SPACE( rStr );
    if (aStrDate.Len())
        rStr += aStrDate;
    else
        rStr.AppendAscii( "???" );
    rStr.AppendAscii( ", " );
    if (aStrTime.Len())
        rStr += aStrTime;
    else
        rStr.AppendAscii( "???" );
}


void lcl_AppendHTMLColorTripel( ByteString& rStr, const Color& rColor )
{
    // <font COLOR="#00FF40">hallo</font>
    sal_Char    buf[64];
    sal_Char*   p = buf;

    rStr += "\"#";
    p += sprintf( p, "%02X", rColor.GetRed() );
    p += sprintf( p, "%02X", rColor.GetGreen() );
    p += sprintf( p, "%02X", rColor.GetBlue() );
    rStr += buf;
    rStr += '\"';
}


/*void lcl_TagOn( String& rResult, const String& rTag, const String* pStrOpt )
{
    rResult  = '<';
    rResult += rTag;
    if ( pStrOpt )
    {
        rResult += ' ';
        rResult += *pStrOpt;
    }
    rResult += '>';
}
*/

/*void lcl_TagOff( String& rResult, const String& rTag )
{
    rResult = '<'; rResult += rTag; rResult += '>';
}
*/

void lcl_WriteTeamInfo( SvStream& rStrm, rtl_TextEncoding eDestEnc )
{
    if ( !bOderSo ) return;
    lcl_OUT_LF();
    lcl_OUT_COMMENT( _STRINGCONST( "Sascha Ballach                     " ) );
    lcl_OUT_COMMENT( _STRINGCONST( "Michael Daeumling (aka Bitsau)     " ) );
    lcl_OUT_COMMENT( _STRINGCONST( "Michael Hagen                      " ) );
    lcl_OUT_COMMENT( _STRINGCONST( "Roland Jakobs                      " ) );
    lcl_OUT_COMMENT( _STRINGCONST( "Andreas Krebs                      " ) );
    lcl_OUT_COMMENT( _STRINGCONST( "Niklas Nebel                       " ) );
    lcl_OUT_COMMENT( _STRINGCONST( "Jacques Nietsch                    " ) );
    lcl_OUT_COMMENT( _STRINGCONST( "Marcus Olk                         " ) );
    lcl_OUT_COMMENT( _STRINGCONST( "Eike Rathke                        " ) );
    lcl_OUT_COMMENT( _STRINGCONST( "Daniel Rentz                       " ) );
    lcl_OUT_COMMENT( _STRINGCONST( "Stephan Templin                    " ) );
    lcl_OUT_COMMENT( _STRINGCONST( "Gunnar Timm                        " ) );
    lcl_OUT_COMMENT( _STRINGCONST( "*** Man kann nicht ALLES haben! ***" ) );
    lcl_OUT_LF();
}


//////////////////////////////////////////////////////////////////////////////

ScHTMLExport::ScHTMLExport( SvStream& rStrmP, ScDocument* pDocP,
                            const ScRange& rRangeP,
                            BOOL bAllP, const String& rStreamPathP ) :
    ScExportBase( rStrmP, pDocP, rRangeP ),
    aStreamPath( rStreamPathP ),
    pAppWin( Application::GetDefaultDevice() ),
    pSrcArr( NULL ),
    pDestArr( NULL ),
    nUsedTables( 0 ),
    nIndent( 0 ),
    bAll( bAllP ),
    bTabHasGraphics( FALSE ),
    bCalcAsShown( pDocP->GetDocOptions().IsCalcAsShown() )
{
    const sal_Char* pCharSet = rtl_getBestMimeCharsetFromTextEncoding( gsl_getSystemTextEncoding() );
    eDestEnc = rtl_getTextEncodingFromMimeCharset( pCharSet );

    strcpy( sIndent, sIndentSource );
    sIndent[0] = 0;
    const USHORT nCount = pDoc->GetTableCount();
    for ( USHORT nTab = 0; nTab < nCount; nTab++ )
    {
        if ( !IsEmptyTable( nTab ) )
            nUsedTables++;
    }

    // neue Konfiguration setzen
    OfaHtmlOptions* pHtmlOptions = ((OfficeApplication*)SFX_APP())->GetHtmlOptions();
    bCopyLocalFileToINet = pHtmlOptions->IsSaveGraphicsLocal();
    for ( USHORT j=0; j < SC_HTML_FONTSIZES; j++ )
    {
        USHORT nSize = pHtmlOptions->GetFontSize( j );
        // in Twips merken, wie unser SvxFontHeightItem
        if ( nSize )
            nFontSize[j] = nSize * 20;
        else
            nFontSize[j] = nDefaultFontSize[j] * 20;
    }

    // Content-Id fuer Mail-Export?
    SfxObjectShell* pDocSh = pDoc->GetDocumentShell();
    if ( pDocSh )
    {
        const SfxPoolItem* pItem = pDocSh->GetItem( SID_ORIGURL );
        if( pItem )
        {
            aCId = ((const SfxStringItem *)pItem)->GetValue();
            DBG_ASSERT( aCId.Len(), "CID ohne Laenge!" );
        }
    }
}


ScHTMLExport::~ScHTMLExport()
{
    for ( ScHTMLGraphEntry* pE = aGraphList.First(); pE; pE = aGraphList.Next() )
        delete pE;
    delete pSrcArr;
    delete pDestArr;
}


USHORT ScHTMLExport::GetFontSizeNumber( USHORT nHeight )
{
    USHORT nSize = 1;
    for ( USHORT j=SC_HTML_FONTSIZES-1; j>0; j-- )
    {
        if( nHeight > (nFontSize[j] + nFontSize[j-1]) / 2 )
        {   // der naechstgelegene
            nSize = j+1;
            break;
        }
    }
    return nSize;
}


USHORT ScHTMLExport::ToPixel( USHORT nVal )
{
    if( nVal )
    {
        nVal = (USHORT)pAppWin->LogicToPixel(
                    Size( nVal, nVal ), MapMode( MAP_TWIP ) ).Width();
        if( !nVal )     // wo ein Twip ist sollte auch ein Pixel sein
            nVal = 1;
    }
    return nVal;
}


Size ScHTMLExport::MMToPixel( const Size& rSize )
{
    Size aSize( rSize );
    aSize = pAppWin->LogicToPixel( rSize, MapMode( MAP_100TH_MM ) );
    // wo etwas ist sollte auch ein Pixel sein
    if ( !aSize.Width() && rSize.Width() )
        aSize.Width() = 1;
    if ( !aSize.Height() && rSize.Height() )
        aSize.Height() = 1;
    return aSize;
}


ULONG ScHTMLExport::Write()
{
    rStrm << '<' << sHTML_doctype << ' ' << sHTML_doctype32 << '>'
        << sNewLine << sNewLine;
    TAG_ON_LF( sHTML_html );
    if ( bAll )
    {
        WriteHeader();
        OUT_LF();
    }
    WriteBody();
    OUT_LF();
    TAG_OFF_LF( sHTML_html );

    return rStrm.GetError();
}


void ScHTMLExport::WriteHeader()
{
    International&   rInter     = *ScGlobal::pScInternational;
    SfxDocumentInfo& rInfo      = pDoc->GetDocumentShell()->GetDocInfo();
    String           aStrOut;

    IncIndent(1); TAG_ON_LF( sHTML_head );

    SfxFrameHTMLWriter::Out_DocInfo( rStrm, &rInfo, sIndent, eDestEnc );
    OUT_LF();

    //----------------------------------------------------------
    if ( rInfo.GetPrinted().GetName().Len() )
    {
        OUT_COMMENT( GLOBSTR( STR_DOC_INFO ) );
        aStrOut  = GLOBSTR( STR_DOC_PRINTED );
        aStrOut.AppendAscii( ": " );
        lcl_AddStamp( aStrOut, rInfo.GetPrinted(), rInter );
        OUT_COMMENT( aStrOut );
    }
    //----------------------------------------------------------

    lcl_WriteTeamInfo( rStrm, eDestEnc );

    IncIndent(-1); OUT_LF(); TAG_OFF_LF( sHTML_head );
}


void ScHTMLExport::WriteOverview()
{
    if ( nUsedTables > 1 )
    {
        IncIndent(1);
        OUT_HR();
        IncIndent(1); TAG_ON( sHTML_parabreak ); TAG_ON_LF( sHTML_center );
        TAG_ON( sHTML_head1 );
        OUT_STR( ScGlobal::GetRscString( STR_OVERVIEW ) );
        TAG_OFF_LF( sHTML_head1 );

        String       aStr;

        const USHORT nCount = pDoc->GetTableCount();
        for ( USHORT nTab = 0; nTab < nCount; nTab++ )
        {
            if ( !IsEmptyTable( nTab ) )
            {
                pDoc->GetName( nTab, aStr );
                rStrm << "<A HREF=\"#table"
                    << ByteString::CreateFromInt32( nTab ).GetBuffer()
                    << "\">";
                OUT_STR( aStr );
                rStrm << "</A>";
                TAG_ON_LF( sHTML_linebreak );
            }
        }

        IncIndent(-1); OUT_LF();
        IncIndent(-1); TAG_OFF( sHTML_center ); TAG_OFF_LF( sHTML_parabreak );
    }
}


const SfxItemSet& ScHTMLExport::PageDefaults( USHORT nTab )
{
    SfxStyleSheetBasePool*  pStylePool  = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase*      pStyleSheet = NULL;
    DBG_ASSERT( pStylePool, "StylePool not found! :-(" );
    pStylePool->SetSearchMask( SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL );
    pStyleSheet = pStylePool->Find( ScGlobal::GetRscString(STR_STYLENAME_STANDARD), SFX_STYLE_FAMILY_PARA );
    DBG_ASSERT( pStyleSheet, "ParaStyle not found! :-(" );
    if (!pStyleSheet)
        pStyleSheet = pStylePool->First();
    const SfxItemSet& rSetPara = pStyleSheet->GetItemSet();

    // Defaults fuer Vergleich in WriteCell merken
    aHTMLStyle.aFontFamilyName =
        ((const SvxFontItem&)(rSetPara.Get( ATTR_FONT ))).GetFamilyName();
    aHTMLStyle.nFontHeight =
        ((const SvxFontHeightItem&)(rSetPara.Get( ATTR_FONT_HEIGHT ))).GetHeight();

    // Seitenvorlage Druckeinstellungen fuer z.B. Hintergrundgrafik,
    // es gibt nur eine Hintergrundgrafik in HTML!
    // aHTMLStyle.aBackgroundColor wird nicht hier sondern in WriteBody gesetzt
    pStylePool->SetSearchMask( SFX_STYLE_FAMILY_PAGE, SFXSTYLEBIT_ALL );
    pStyleSheet = pStylePool->Find( pDoc->GetPageStyle( nTab ), SFX_STYLE_FAMILY_PAGE );
    DBG_ASSERT( pStyleSheet, "PageStyle not found! :-(" );
    if (!pStyleSheet)
        pStyleSheet = pStylePool->First();
    const SfxItemSet& rSet = pStyleSheet->GetItemSet();
    return rSet;
}


BOOL ScHTMLExport::HasBottomBorder( USHORT nRow, USHORT nTab,
        USHORT nStartCol, USHORT nEndCol )
{
    BOOL bHas = TRUE;
    for ( USHORT nCol=nStartCol; nCol<=nEndCol && bHas; nCol++ )
    {
        SvxBoxItem* pBorder = (SvxBoxItem*)
            pDoc->GetAttr( nCol, nRow, nTab, ATTR_BORDER );
        if ( !pBorder || !pBorder->GetBottom() )
        {   // vielleicht obere Border an Zelle darunter?
            if ( nRow < MAXROW )
            {
                pBorder = (SvxBoxItem*) pDoc->GetAttr( nCol, nRow+1, nTab, ATTR_BORDER );
                if ( !pBorder || !pBorder->GetTop() )
                    bHas = FALSE;
            }
            else
                bHas = FALSE;
        }
    }
    return bHas;
}


BOOL ScHTMLExport::HasLeftBorder( USHORT nCol, USHORT nTab,
        USHORT nStartRow, USHORT nEndRow )
{
    BOOL bHas = TRUE;
    for ( USHORT nRow=nStartRow; nRow<=nEndRow && bHas; nRow++ )
    {
        SvxBoxItem* pBorder = (SvxBoxItem*)
            pDoc->GetAttr( nCol, nRow, nTab, ATTR_BORDER );
        if ( !pBorder || !pBorder->GetLeft() )
        {   // vielleicht rechte Border an Zelle links daneben?
            if ( nCol > 0 )
            {
                pBorder = (SvxBoxItem*) pDoc->GetAttr( nCol-1, nRow, nTab, ATTR_BORDER );
                if ( !pBorder || !pBorder->GetRight() )
                    bHas = FALSE;
            }
            else
                bHas = FALSE;
        }
    }
    return bHas;
}


BOOL ScHTMLExport::HasTopBorder( USHORT nRow, USHORT nTab,
        USHORT nStartCol, USHORT nEndCol )
{
    BOOL bHas = TRUE;
    for ( USHORT nCol=nStartCol; nCol<=nEndCol && bHas; nCol++ )
    {
        SvxBoxItem* pBorder = (SvxBoxItem*)
            pDoc->GetAttr( nCol, nRow, nTab, ATTR_BORDER );
        if ( !pBorder || !pBorder->GetTop() )
        {   // vielleicht untere Border an Zelle darueber?
            if ( nRow > 0 )
            {
                pBorder = (SvxBoxItem*) pDoc->GetAttr( nCol, nRow-1, nTab, ATTR_BORDER );
                if ( !pBorder || !pBorder->GetBottom() )
                    bHas = FALSE;
            }
            else
                bHas = FALSE;
        }
    }
    return bHas;
}


BOOL ScHTMLExport::HasRightBorder( USHORT nCol, USHORT nTab,
        USHORT nStartRow, USHORT nEndRow )
{
    BOOL bHas = TRUE;
    for ( USHORT nRow=nStartRow; nRow<=nEndRow && bHas; nRow++ )
    {
        SvxBoxItem* pBorder = (SvxBoxItem*)
            pDoc->GetAttr( nCol, nRow, nTab, ATTR_BORDER );
        if ( !pBorder || !pBorder->GetRight() )
        {   // vielleicht linke Border an Zelle rechts daneben?
            if ( nCol < MAXCOL )
            {
                pBorder = (SvxBoxItem*) pDoc->GetAttr( nCol+1, nRow, nTab, ATTR_BORDER );
                if ( !pBorder || !pBorder->GetLeft() )
                    bHas = FALSE;
            }
            else
                bHas = FALSE;
        }
    }
    return bHas;
}


void ScHTMLExport::WriteBody()
{
    const SfxItemSet& rSet = PageDefaults( bAll ? 0 : aRange.aStart.Tab() );
    const SvxBrushItem* pBrushItem = (const SvxBrushItem*)&rSet.Get( ATTR_BACKGROUND );

    aHTMLStyle.aBackgroundColor = pBrushItem->GetColor();

    // CSS1 StyleSheet
    IncIndent(1); TAG_ON_LF( sHTML_style );
    rStrm << sMyBegComment; OUT_LF();
    rStrm << sHTML_body << " { " << sFontFamily;
    xub_StrLen nFonts = aHTMLStyle.aFontFamilyName.GetTokenCount( ';' );
    if ( nFonts == 1 )
    {
        rStrm << '\"';
        OUT_STR( aHTMLStyle.aFontFamilyName );
        rStrm << '\"';
    }
    else
    {   // Fontliste, VCL: Semikolon als Separator,
        // CSS1: Komma als Separator und jeder einzelne Fontname quoted
        const String& rList = aHTMLStyle.aFontFamilyName;
        for ( xub_StrLen j = 0, nPos = 0; j < nFonts; j++ )
        {
            rStrm << '\"';
            OUT_STR( rList.GetToken( 0, ';', nPos ) );
            rStrm << '\"';
            if ( j < nFonts-1 )
                rStrm << ", ";
        }
    }
    rStrm << "; " << sFontSize
        << ByteString::CreateFromInt32( GetFontSizeNumber( ( USHORT ) aHTMLStyle.nFontHeight ) ).GetBuffer()
        << " }";
    OUT_LF();
    rStrm << sMyEndComment;
    IncIndent(-1); OUT_LF(); TAG_OFF_LF( sHTML_style );
    OUT_LF();

    // default Textfarbe schwarz
    rStrm << '<' << sHTML_body << ' ' << sHTML_O_text << "=\"#000000\"";

    if ( bAll && GPOS_NONE != pBrushItem->GetGraphicPos() )
    {
        const String* pLink = pBrushItem->GetGraphicLink();
        String aGrfNm;

        // embeddete Grafik -> via WriteGraphic schreiben
        if( !pLink )
        {
            const Graphic* pGrf = pBrushItem->GetGraphic();
            if( pGrf )
            {
                // Grafik als (JPG-)File speichern
                aGrfNm = aStreamPath;
                USHORT nErr = XOutBitmap::WriteGraphic( *pGrf, aGrfNm, _STRINGCONST( "JPG" ) );
                if( !nErr )     // fehlerhaft, da ist nichts auszugeben
                {
                    aGrfNm = URIHelper::SmartRelToAbs( aGrfNm );
                    if ( HasCId() )
                        MakeCIdURL( aGrfNm );
                    pLink = &aGrfNm;
                }
            }
        }
        else
        {
            aGrfNm = *pLink;
            if( bCopyLocalFileToINet || HasCId() )
            {
                CopyLocalFileToINet( aGrfNm, aStreamPath );
                if ( HasCId() )
                    MakeCIdURL( aGrfNm );
            }
            else
                aGrfNm = URIHelper::SmartRelToAbs( aGrfNm );
            pLink = &aGrfNm;
        }
        if( pLink )
        {
            rStrm << ' ' << sHTML_O_background << "=\"";
            OUT_STR( INetURLObject::AbsToRel( *pLink ) ) << '\"';
        }
    }
    OUT_SP_CSTR_ASS( sHTML_O_bgcolor );
    HTMLOutFuncs::Out_Color( rStrm, aHTMLStyle.aBackgroundColor );

    rStrm << '>'; OUT_LF();

    if ( bAll )
        WriteOverview();

    WriteTables();

    TAG_OFF_LF( sHTML_body );
}


void ScHTMLExport::WriteTables()
{
    const USHORT    nTabCount = pDoc->GetTableCount();
    const String    aStrTable( ScResId( SCSTR_TABLE ) );
    String          aStr;
    String          aStrOut;
    USHORT          nStartCol, nStartRow, nStartTab;
    USHORT          nEndCol, nEndRow, nEndTab;
    USHORT          nStartColFix, nStartRowFix, nEndColFix, nEndRowFix;
    ScDrawLayer*    pDrawLayer = pDoc->GetDrawLayer();
    if ( bAll )
    {
        nStartTab = 0;
        nEndTab = nTabCount - 1;
    }
    else
    {
        nStartCol = nStartColFix = aRange.aStart.Col();
        nStartRow = nStartRowFix = aRange.aStart.Row();
        nStartTab = aRange.aStart.Tab();
        nEndCol = nEndColFix = aRange.aEnd.Col();
        nEndRow = nEndRowFix = aRange.aEnd.Row();
        nEndTab = aRange.aEnd.Tab();
    }
    for ( USHORT nTab=nStartTab; nTab<=nEndTab; nTab++ )
    {
        if ( bAll )
        {
            if ( !GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow ) )
                continue;   // for

            if ( nUsedTables > 1 )
            {
                aStrOut  = aStrTable;
                aStrOut.AppendAscii( " " );
                aStrOut += String::CreateFromInt32( nTab + 1 );
                aStrOut.AppendAscii( ": " );

                OUT_HR();

                // Anker festlegen:
                rStrm << "<A NAME=\"table"
                    << ByteString::CreateFromInt32( nTab ).GetBuffer()
                    << "\">";
                TAG_ON( sHTML_head1 );
                OUT_STR( aStrOut );
                TAG_ON( sHTML_emphasis );

                pDoc->GetName( nTab, aStr );
                OUT_STR( aStr );

                TAG_OFF( sHTML_emphasis );
                TAG_OFF( sHTML_head1 );
                rStrm << "</A>"; OUT_LF();
            }
        }
        else
        {
            nStartCol = nStartColFix;
            nStartRow = nStartRowFix;
            nEndCol = nEndColFix;
            nEndRow = nEndRowFix;
            if ( !TrimDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow ) )
                continue;   // for
        }

        // <TABLE ...>
        ByteString  aByteStrOut = sHTML_table;
//      aStrOut  = sHTML_table;
        aByteStrOut += ' ';
        aByteStrOut += sHTML_frame;
        aByteStrOut += '=';
        USHORT nFrame = 0;
        if ( HasBottomBorder( nEndRow, nTab, nStartCol, nEndCol ) )
            nFrame |= 2;
        if ( HasLeftBorder( nStartCol, nTab, nStartRow, nEndRow ) )
            nFrame |= 4;
        if ( HasTopBorder( nStartRow, nTab, nStartCol, nEndCol ) )
            nFrame |= 8;
        if ( HasRightBorder( nEndCol, nTab, nStartRow, nEndRow ) )
            nFrame |= 16;
        if ( nFrame )
        {
            // nicht alle Kombinationen sind in HTML moeglich
            // nur void, above, below, lhs, rhs, hsides, vsides, box
            const USHORT nAll = 2 | 4 | 8 | 16;
            USHORT nBit;
            for ( nBit=2; nBit<=16; nBit <<= 1 )
            {
                if ( (nFrame | nBit) == nAll )
                {   // mindestens drei Seiten => vier
                    aByteStrOut += sHTML_TF_box;
                    nFrame = 0;
                    break;
                }
            }
            if ( nFrame )
            {   // ein oder zwei Seiten
                for ( nBit=2; nBit<=16; nBit <<= 1 )
                {
                    if ( (nFrame & nBit) == nFrame )
                    {   // eine Seite
                        switch ( nBit )
                        {
                            case 2:
                                aByteStrOut += sHTML_TF_below;
                            break;
                            case 4:
                                aByteStrOut += sHTML_TF_lhs;
                            break;
                            case 8:
                                aByteStrOut += sHTML_TF_above;
                            break;
                            case 16:
                                aByteStrOut += sHTML_TF_rhs;
                            break;
                        }
                        nFrame = 0;
                        break;
                    }
                }
                if ( nFrame )
                {   // zwei Seiten
                    // horizontale bevorzugt
                    if ( nFrame & 8 )
                    {
                        if ( nFrame & 2 )
                            aByteStrOut += sHTML_TF_hsides;
                        else
                            aByteStrOut += sHTML_TF_above;
                    }
                    else if ( nFrame & 2 )
                        aByteStrOut += sHTML_TF_below;
                    else if ( nFrame & 4 )
                    {
                        if ( nFrame & 16 )
                            aByteStrOut += sHTML_TF_vsides;
                        else
                            aByteStrOut += sHTML_TF_lhs;
                    }
                    else    // if ( nFrame & 16 )
                        aByteStrOut += sHTML_TF_rhs;
                }
            }
        }
        else
            aByteStrOut += sHTML_TF_void;

        bTabHasGraphics = bTabAlignedLeft = FALSE;
        if ( bAll && pDrawLayer )
            PrepareGraphics( pDrawLayer, nTab, nStartCol, nStartRow,
                nEndCol, nEndRow );

        // mehr <TABLE ...>
        if ( bTabAlignedLeft )
            (((aByteStrOut += ' ') += sHTML_O_align) += '=') += sHTML_AL_left;
            // ALIGN=LEFT allow text and graphics to flow around
        // CELLSPACING
        (((aByteStrOut += ' ' ) += sHTML_O_cellspacing ) += '=') +=
                                    ByteString::CreateFromInt32( nCellSpacing );
        // COLS=n
        USHORT nColCnt = 0;
        USHORT nCol;
        for ( nCol=nStartCol; nCol<=nEndCol; nCol++ )
        {
            if ( !(pDoc->GetColFlags( nCol, nTab ) & CR_HIDDEN) )
                ++nColCnt;
        }
        (((aByteStrOut += ' ') += sHTML_O_cols) += '=') += ByteString::CreateFromInt32( nColCnt );
        // RULES=GROUPS
        (((aByteStrOut += ' ') += sHTML_O_rules) += '=') += sHTML_TR_groups;
        // Netscape und M$IE brauchen ein BORDER=n um ueberhaupt ein Rule zu zeichnen
        ((aByteStrOut += ' ') += sHTML_O_border) += "=1";
        IncIndent(1); TAG_ON_LF( aByteStrOut.GetBuffer() );

        // <COLGROUP> fuer RULES=GROUPS
        TAG_ON( sHTML_colgroup );
        // <COL WIDTH=x> als Vorabinformation fuer lange Tabellen
        ByteString  aByteStr = sHTML_col;
        aByteStr += ' ';
        aByteStr += sHTML_O_width;
        aByteStr += '=';
        for ( nCol=nStartCol; nCol<=nEndCol; nCol++ )
        {
            if ( pDoc->GetColFlags( nCol, nTab ) & CR_HIDDEN )
                continue;   // for

            aByteStrOut  = aByteStr;
            aByteStrOut += ByteString::CreateFromInt32(
                                ToPixel( pDoc->GetColWidth( nCol, nTab ) ) );
            TAG_ON( aByteStrOut.GetBuffer() );

            if ( nCol < nEndCol && HasRightBorder( nCol, nTab,
                    nStartRow, nEndRow ) )
            {   // neue ColGroup fuer RULES
                TAG_OFF_LF( sHTML_colgroup );
                TAG_ON( sHTML_colgroup );
            }
        }
        TAG_OFF_LF( sHTML_colgroup );

        // <TBODY> fuer RULES=GROUPS
        IncIndent(1); TAG_ON_LF( sHTML_tbody );
        for ( USHORT nRow=nStartRow; nRow<=nEndRow; nRow++ )
        {
            if ( pDoc->GetRowFlags( nRow, nTab ) & CR_HIDDEN )
                continue;   // for

            IncIndent(1); TAG_ON_LF( sHTML_tablerow );
            for ( USHORT nCol=nStartCol; nCol<=nEndCol; nCol++ )
            {
                if ( pDoc->GetColFlags( nCol, nTab ) & CR_HIDDEN )
                    continue;   // for

                if ( nCol == nEndCol )
                    IncIndent(-1);
                WriteCell( nCol, nRow, nTab );
            }

            if ( nRow < nEndRow && HasBottomBorder( nRow, nTab,
                    nStartCol, nEndCol ) )
            {   // neuer TBody fuer RULES
                IncIndent(-1); TAG_OFF_LF( sHTML_tablerow );
                TAG_OFF_LF( sHTML_tbody );
                IncIndent(1); TAG_ON_LF( sHTML_tbody );
            }
            else
            {
                if ( nRow == nEndRow )
                    IncIndent(-1);
                TAG_OFF_LF( sHTML_tablerow );
            }
        }
        IncIndent(-1); TAG_OFF_LF( sHTML_tbody );

        IncIndent(-1); TAG_OFF_LF( sHTML_table );

        if ( bTabHasGraphics )
        {
            // der Rest, der nicht in Zellen ist
            for ( ScHTMLGraphEntry* pE = aGraphList.First(); pE; pE = aGraphList.Next() )
            {
                if ( !pE->bWritten )
                    WriteGraphEntry( pE );
                delete pE;
            }
            aGraphList.Clear();
            if ( bTabAlignedLeft )
            {   // mit <BR CLEAR=LEFT> das <TABLE ALIGN=LEFT> wieder ausschalten
                aByteStrOut = sHTML_linebreak;
                (((aByteStrOut += ' ') += sHTML_O_clear) += '=') += sHTML_AL_left;
                TAG_ON_LF( aByteStrOut.GetBuffer() );
            }
        }

        if ( bAll )
            OUT_COMMENT( _STRINGCONST( "**************************************************************************" ) );
    }
}


void ScHTMLExport::WriteCell( USHORT nCol, USHORT nRow, USHORT nTab )
{
    const ScPatternAttr* pAttr = pDoc->GetPattern( nCol, nRow, nTab );

    const ScMergeFlagAttr& rMergeFlagAttr = (const ScMergeFlagAttr&) pAttr->GetItem( ATTR_MERGE_FLAG );
    if ( rMergeFlagAttr.IsOverlapped() )
        return ;

    ScAddress aPos( nCol, nRow, nTab );
    BOOL bIsGraphHere = FALSE;
    ScHTMLGraphEntry* pGraphEntry;
    if ( bTabHasGraphics )
    {
        for ( pGraphEntry = aGraphList.First(); pGraphEntry;
              pGraphEntry = aGraphList.Next() )
        {

            if ( pGraphEntry->bInCell && pGraphEntry->aRange.In( aPos ) )
            {
                if ( pGraphEntry->aRange.aStart == aPos )
                    bIsGraphHere = TRUE;
                else
                    return ;        // ist ein Col/RowSpan, Overlapped
                break;
            }
        }
    }

    ScBaseCell* pCell = pDoc->GetCell( aPos );
    ULONG nFormat = pAttr->GetNumberFormat( pFormatter );
    BOOL bValueData;
    if ( pCell )
        bValueData = pCell->HasValueData();
    else
        bValueData = FALSE;

    ByteString aStrTD = sHTML_tabledata;

    String      aStr;
    const sal_Char* pChar;
    USHORT nWidthPixel;
    USHORT nHeightPixel;

    const ScMergeAttr& rMergeAttr = (const ScMergeAttr&) pAttr->GetItem( ATTR_MERGE );
    if ( bIsGraphHere || rMergeAttr.IsMerged() )
    {
        USHORT j, n, v;
        if ( bIsGraphHere )
            n = pGraphEntry->aRange.aEnd.Col() - nCol + 1;
        else
            n = rMergeAttr.GetColMerge();
        if ( n > 1 )
        {
            (((aStrTD += ' ') += sHTML_O_colspan) += '=') += ByteString::CreateFromInt32( n );
            n += nCol;
            for ( j=nCol, v=0; j<n; j++ )
                v += pDoc->GetColWidth( j, nTab );
            nWidthPixel = ToPixel( v );
        }
        else
            nWidthPixel = ToPixel( pDoc->GetColWidth( nCol, nTab ) );

        if ( bIsGraphHere )
            n = pGraphEntry->aRange.aEnd.Row() - nRow + 1;
        else
            n = rMergeAttr.GetRowMerge();
        if ( n > 1 )
        {
            (((aStrTD += ' ') += sHTML_O_rowspan) += '=') += ByteString::CreateFromInt32( n );
            n += nRow;
            for ( j=nRow, v=0; j<n; j++ )
                v += pDoc->GetRowHeight( j, nTab );
            nHeightPixel = ToPixel( v );
        }
        else
            nHeightPixel = ToPixel( pDoc->GetRowHeight( nRow, nTab ) );
    }
    else
    {
        nWidthPixel = ToPixel( pDoc->GetColWidth( nCol, nTab ) );
        nHeightPixel = ToPixel( pDoc->GetRowHeight( nRow, nTab ) );
    }

    // trotz der <TABLE COLS=n> und <COL WIDTH=x> Angaben noetig,
    // da die nicht von Netscape beachtet werden..
    // Spaltenbreite
    (((aStrTD += ' ') += sHTML_O_width) += '=') += ByteString::CreateFromInt32( nWidthPixel );
    // Zeilenhoehe
    (((aStrTD += ' ') += sHTML_O_height) += '=') += ByteString::CreateFromInt32( nHeightPixel );

    const SvxFontItem&          rFontItem       = (const SvxFontItem&)      pAttr->GetItem( ATTR_FONT );
    const SvxFontHeightItem&    rFontHeightItem = (const SvxFontHeightItem&)pAttr->GetItem( ATTR_FONT_HEIGHT );
    const SvxWeightItem&        rWeightItem     = (const SvxWeightItem&)    pAttr->GetItem( ATTR_FONT_WEIGHT );
    const SvxPostureItem&       rPostureItem    = (const SvxPostureItem&)   pAttr->GetItem( ATTR_FONT_POSTURE );
    const SvxUnderlineItem&     rUnderlineItem  = (const SvxUnderlineItem&) pAttr->GetItem( ATTR_FONT_UNDERLINE );
    const SvxColorItem&         rColorItem      = (const SvxColorItem&)     pAttr->GetItem( ATTR_FONT_COLOR );
    const SvxHorJustifyItem&    rHorJustifyItem = (const SvxHorJustifyItem&)pAttr->GetItem( ATTR_HOR_JUSTIFY );
    const SvxVerJustifyItem&    rVerJustifyItem = (const SvxVerJustifyItem&)pAttr->GetItem( ATTR_VER_JUSTIFY );
    const SvxBrushItem&         rBrushItem      = (const SvxBrushItem&)     pAttr->GetItem( ATTR_BACKGROUND );
    Color aBgColor;
    if ( rBrushItem.GetColor().GetTransparency() == 255 )
        aBgColor = aHTMLStyle.aBackgroundColor;     // #55121# keine ungewollte Hintergrundfarbe
    else
        aBgColor = rBrushItem.GetColor();

    BOOL bBold          = ( WEIGHT_BOLD     <= rWeightItem.GetWeight() );
    BOOL bItalic        = ( ITALIC_NONE     != rPostureItem.GetPosture() );
    BOOL bUnderline     = ( UNDERLINE_NONE  != rUnderlineItem.GetUnderline() );
    BOOL bSetFontColor  = ( COL_BLACK       != rColorItem.GetValue().GetColor() );
#if 0
// keine StyleSheet-Fontangaben: hart fuer jede Zelle
    BOOL bSetFontName   = TRUE;
    BOOL bSetFontHeight = TRUE;
#else
    BOOL bSetFontName   = ( aHTMLStyle.aFontFamilyName  != rFontItem.GetFamilyName() );
    BOOL bSetFontHeight = ( aHTMLStyle.nFontHeight      != rFontHeightItem.GetHeight() );
#endif
    BOOL bSetFont = (bSetFontColor || bSetFontName || bSetFontHeight);

    switch( rHorJustifyItem.GetValue() )
    {
        case SVX_HOR_JUSTIFY_STANDARD:
            pChar = (bValueData ? sHTML_AL_right : sHTML_AL_left);
            break;
        case SVX_HOR_JUSTIFY_CENTER:    pChar = sHTML_AL_center;    break;
        case SVX_HOR_JUSTIFY_BLOCK:     pChar = sHTML_AL_justify;   break;
        case SVX_HOR_JUSTIFY_RIGHT:     pChar = sHTML_AL_right;     break;
        case SVX_HOR_JUSTIFY_LEFT:
        case SVX_HOR_JUSTIFY_REPEAT:
        default:                        pChar = sHTML_AL_left;      break;
    }

    (((aStrTD += ' ') += sHTML_O_align) += '=') += pChar;

    switch( rVerJustifyItem.GetValue() )
    {
        case SVX_VER_JUSTIFY_TOP:       pChar = sHTML_VA_top;       break;
        case SVX_VER_JUSTIFY_CENTER:    pChar = sHTML_VA_middle;    break;
        case SVX_VER_JUSTIFY_BOTTOM:    pChar = sHTML_VA_bottom;    break;
        case SVX_VER_JUSTIFY_STANDARD:
        default:                        pChar = NULL;
    }
    if ( pChar )
        (((aStrTD += ' ') += sHTML_O_valign) += '=') += pChar;

    if ( aHTMLStyle.aBackgroundColor != aBgColor )
    {
        ((aStrTD += ' ') += sHTML_O_bgcolor) += '=';
        lcl_AppendHTMLColorTripel( aStrTD, aBgColor );
    }

    double fVal = 0.0;
    if ( bValueData )
    {
        if ( pCell )
        {
            switch ( pCell->GetCellType() )
            {
                case CELLTYPE_VALUE:
                    fVal = ((ScValueCell*)pCell)->GetValue();
                    if ( bCalcAsShown && fVal != 0.0 )
                        fVal = pDoc->RoundValueAsShown( fVal, nFormat );
                    break;
                case CELLTYPE_FORMULA:
                    fVal = ((ScFormulaCell*)pCell)->GetValue();
                    if ( (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
                        nFormat = ScGlobal::GetStandardFormat( fVal, *pFormatter,
                            nFormat, ((ScFormulaCell*)pCell)->GetFormatType() );
                    break;
                default:
                    DBG_ERRORFILE( "value data with unsupported cell type" );
            }
        }
    }
    HTMLOutFuncs::CreateTableDataOptionsValNum( aStrTD, bValueData, fVal,
        nFormat, *pFormatter, eDestEnc );

    TAG_ON( aStrTD.GetBuffer() );

    if ( bBold )        TAG_ON( sHTML_bold );
    if ( bItalic )      TAG_ON( sHTML_italic );
    if ( bUnderline )   TAG_ON( sHTML_underline );


    if ( bSetFont )
    {
        ByteString  aStr = sHTML_font;
        if ( bSetFontName )
        {
            ((aStr += ' ') += sHTML_O_face) += "=\"";
            xub_StrLen nFonts = rFontItem.GetFamilyName().GetTokenCount( ';' );
            if ( nFonts == 1 )
            {
                ByteString  aTmpStr;
                HTMLOutFuncs::ConvertStringToHTML( rFontItem.GetFamilyName(), aTmpStr, eDestEnc );
                aStr += aTmpStr;
            }
            else
            {   // Fontliste, VCL: Semikolon als Separator, HTML: Komma
                const String& rList = rFontItem.GetFamilyName();
                for ( xub_StrLen j = 0, nPos = 0; j < nFonts; j++ )
                {
                    ByteString  aTmpStr;
                    HTMLOutFuncs::ConvertStringToHTML( rList.GetToken( 0, ';', nPos ), aTmpStr, eDestEnc );
                    aStr += aTmpStr;
                    if ( j < nFonts-1 )
                        aStr += ',';
                }
            }
            aStr += '\"';
        }
        if ( bSetFontHeight )
        {
            (((aStr += ' ') += sHTML_O_size) += '=')
                += ByteString::CreateFromInt32( GetFontSizeNumber( (USHORT)rFontHeightItem.GetHeight() ) );
        }
        if ( bSetFontColor )
        {
            Color   aColor = rColorItem.GetValue();
            ((aStr += ' ') += sHTML_O_color) += '=';
            lcl_AppendHTMLColorTripel( aStr, aColor );
        }
        TAG_ON( aStr.GetBuffer() );
    }

    String aStrOut;
    BOOL bFieldText = FALSE;
    if ( pCell )
    {   // cell content
        Color* pColor;
        switch ( pCell->GetCellType() )
        {
            case CELLTYPE_NOTE :
                // nothing
            break;
            case CELLTYPE_EDIT :
                bFieldText = WriteFieldText( (const ScEditCell*) pCell );
                if ( bFieldText )
                    break;
                //! else: fallthru
            default:
                ScCellFormat::GetString( pCell, nFormat, aStrOut, &pColor, *pFormatter );
        }
    }
    if ( !bFieldText )
    {
        if ( !aStrOut.Len() )
            TAG_ON( sHTML_linebreak );      // #42573# keine komplett leere Zelle
        else
            OUT_STR( aStrOut );
    }
    if ( bIsGraphHere )
        WriteGraphEntry( pGraphEntry );

    if ( bSetFont )     TAG_OFF( sHTML_font );
    if ( bUnderline )   TAG_OFF( sHTML_underline );
    if ( bItalic )      TAG_OFF( sHTML_italic );
    if ( bBold )        TAG_OFF( sHTML_bold );

    TAG_OFF_LF( sHTML_tabledata );
}


BOOL ScHTMLExport::WriteFieldText( const ScEditCell* pCell )
{
    BOOL bFields = FALSE;
    const EditTextObject* pData;
    pCell->GetData( pData );
    // text and anchor of URL fields, Doc-Engine is a ScFieldEditEngine
    EditEngine& rEngine = pDoc->GetEditEngine();
    rEngine.SetText( *pData );
    USHORT nParas = rEngine.GetParagraphCount();
    if ( nParas )
    {
        ESelection aSel( 0, 0, nParas-1, rEngine.GetTextLen( nParas-1 ) );
        SfxItemSet aSet( rEngine.GetAttribs( aSel ) );
        SfxItemState eFieldState = aSet.GetItemState( EE_FEATURE_FIELD, FALSE );
        if ( eFieldState == SFX_ITEM_DONTCARE || eFieldState == SFX_ITEM_SET )
            bFields = TRUE;
    }
    if ( bFields )
    {
        BOOL bOldUpdateMode = rEngine.GetUpdateMode();
        rEngine.SetUpdateMode( TRUE );      // no portions if not formatted
        for ( USHORT nPar=0; nPar < nParas; nPar++ )
        {
            if ( nPar > 0 )
                rStrm << ' ';       // blank between paragraphs
            SvUShorts aPortions;
            rEngine.GetPortions( nPar, aPortions );
            USHORT nCnt = aPortions.Count();
            USHORT nStart = 0;
            for ( USHORT nPos = 0; nPos < nCnt; nPos++ )
            {
                USHORT nEnd = aPortions.GetObject( nPos );
                ESelection aSel( nPar, nStart, nPar, nEnd );
                BOOL bUrl = FALSE;
                // fields are single characters
                if ( nEnd == nStart+1 )
                {
                    const SfxPoolItem* pItem;
                    SfxItemSet aSet = rEngine.GetAttribs( aSel );
                    if ( aSet.GetItemState( EE_FEATURE_FIELD, FALSE, &pItem ) == SFX_ITEM_ON )
                    {
                        const SvxFieldData* pField = ((const SvxFieldItem*)pItem)->GetField();
                        if ( pField && pField->ISA(SvxURLField) )
                        {
                            bUrl = TRUE;
                            const SvxURLField*  pURLField = (const SvxURLField*)pField;
//                          String              aFieldText = rEngine.GetText( aSel );
                            rStrm << '<' << sHTML_anchor << ' ' << sHTML_O_href << "=\"";
                            OUT_STR( pURLField->GetURL() );
                            rStrm << "\">";
                            OUT_STR( pURLField->GetRepresentation() );
                            rStrm << "</" << sHTML_anchor << '>';
                        }
                    }
                }
                if ( !bUrl )
                    OUT_STR( rEngine.GetText( aSel ) );
                nStart = nEnd;
            }
        }
        rEngine.SetUpdateMode( bOldUpdateMode );
    }
    return bFields;
}


BOOL ScHTMLExport::CopyLocalFileToINet( String& rFileNm,
        const String& rTargetNm, BOOL bFileToFile )
{
    BOOL bRet = FALSE;
    INetURLObject aFileUrl, aTargetUrl;
    aFileUrl.SetSmartURL( rFileNm );
    aTargetUrl.SetSmartURL( rTargetNm );
    if( INET_PROT_FILE == aFileUrl.GetProtocol() &&
        ( (bFileToFile && INET_PROT_FILE == aTargetUrl.GetProtocol()) ||
          (!bFileToFile && INET_PROT_FILE != aTargetUrl.GetProtocol() &&
                           INET_PROT_FTP <= aTargetUrl.GetProtocol() &&
                           INET_PROT_NEWS >= aTargetUrl.GetProtocol()) ) )
    {
        if( pSrcArr )
        {
            // wurde die Datei schon verschoben
            USHORT nPos;
            if( pSrcArr->Seek_Entry( &rFileNm, &nPos ))
            {
                rFileNm = *(*pDestArr)[ nPos ];
                return TRUE;
            }
        }
        else
        {
            pSrcArr = new SvStringsSortDtor( 4, 4 );
            pDestArr = new SvStringsSortDtor( 4, 4 );
        }

        String* pSrc = new String( rFileNm );
        SvFileStream aTmp( aFileUrl.PathToFileName(), STREAM_READ );

        String* pDest = new String( aTargetUrl.GetPartBeforeLastName() );
        *pDest += aFileUrl.GetName();

        if( bFileToFile )
        {
            INetURLObject aCpyURL( *pDest );
            SvFileStream aCpy( aCpyURL.PathToFileName(), STREAM_WRITE );
            aCpy << aTmp;

            aCpy.Close();
            bRet = SVSTREAM_OK == aCpy.GetError();
        }
        else
        {
            SfxMedium aMedium( *pDest, STREAM_WRITE | STREAM_SHARE_DENYNONE,
                                FALSE );

            // temp. File anlegen
    //      aMedium.DownLoad();

            {
                SvFileStream aCpy( aMedium.GetPhysicalName(), STREAM_WRITE );
                aCpy << aTmp;
            }

            // uebertragen
            aMedium.Close();
            aMedium.Commit();

            bRet = 0 == aMedium.GetError();
        }

        if( bRet )
        {
            pSrcArr->Insert( pSrc );
            pDestArr->Insert( pDest );
            rFileNm = *pDest;
        }
        else
        {
            delete pSrc;
            delete pDest;
        }
    }

    return bRet;
}


void ScHTMLExport::MakeCIdURL( String& rURL )
{
    if( !aCId.Len() )
        return;

    INetURLObject aURLObj( rURL );
    if( INET_PROT_FILE != aURLObj.GetProtocol() )
        return;

    String aLastName( aURLObj.GetLastName() );
    DBG_ASSERT( aLastName.Len(), "Dateiname ohne Laenge!" );
    aLastName.ToLowerAscii();

    rURL.AssignAscii( "cid:" );
    rURL += aLastName;
    rURL.AppendAscii( "." );
    rURL += aCId;
}


void ScHTMLExport::IncIndent( short nVal )
{
    sIndent[nIndent] = '\t';
    nIndent += nVal;
    if ( nIndent < 0 )
        nIndent = 0;
    else if ( nIndent > nIndentMax )
        nIndent = nIndentMax;
    sIndent[nIndent] = 0;
}



