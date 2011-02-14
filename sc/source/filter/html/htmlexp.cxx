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



// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#define _SVSTDARR_STRINGSSORTDTOR
#include <rtl/tencinfo.h>

#include <vcl/svapp.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <svx/xoutbmp.hxx>
#include <editeng/editeng.hxx>
#include <svtools/htmlcfg.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frmhtmlw.hxx>
#include <sfx2/objsh.hxx>
#include <svl/stritem.hxx>
#include <svl/urihelper.hxx>
#ifndef _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTS
#endif
#include <svl/svstdarr.hxx>
#include <svl/zforlist.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/parhtml.hxx>
#include <vcl/outdev.hxx>
#include <stdio.h>

#include "htmlexp.hxx"
#include "filter.hxx"
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
#include "ftools.hxx"


#include <editeng/flditem.hxx>
#include <editeng/borderline.hxx>
#include <unotools/syslocale.hxx>


// ohne sc.hrc: error C2679: binary '=' : no operator defined which takes a
// right-hand operand of type 'const class String (__stdcall *)(class ScResId)'
// bei
// const String aStrTable( ScResId( SCSTR_TABLE ) ); aStrOut = aStrTable;
// ?!???
#include "sc.hrc"
#include "globstr.hrc"

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>


//========================================================================

const static sal_Char __FAR_DATA sMyBegComment[]    = "<!-- ";
const static sal_Char __FAR_DATA sMyEndComment[]    = " -->";
const static sal_Char __FAR_DATA sFontFamily[]      = "font-family:";
const static sal_Char __FAR_DATA sFontSize[]        = "font-size:";

const sal_uInt16 __FAR_DATA ScHTMLExport::nDefaultFontSize[SC_HTML_FONTSIZES] =
{
    HTMLFONTSZ1_DFLT, HTMLFONTSZ2_DFLT, HTMLFONTSZ3_DFLT, HTMLFONTSZ4_DFLT,
    HTMLFONTSZ5_DFLT, HTMLFONTSZ6_DFLT, HTMLFONTSZ7_DFLT
};

sal_uInt16 ScHTMLExport::nFontSize[SC_HTML_FONTSIZES] = { 0 };

const char* __FAR_DATA ScHTMLExport::pFontSizeCss[SC_HTML_FONTSIZES] =
{
    "xx-small", "x-small", "small", "medium", "large", "x-large", "xx-large"
};

const sal_uInt16 ScHTMLExport::nCellSpacing = 0;
const sal_Char __FAR_DATA ScHTMLExport::sIndentSource[nIndentMax+1] =
    "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

//========================================================================
// Makros fuer HTML-Export
//========================================================================
#define OUT_PROLOGUE()      (rStrm << sHTML30_Prologue << ScExportBase::sNewLine \
                                << ScExportBase::sNewLine)
#define TAG_ON( tag )       HTMLOutFuncs::Out_AsciiTag( rStrm, tag )
#define TAG_OFF( tag )      HTMLOutFuncs::Out_AsciiTag( rStrm, tag, sal_False )
#define OUT_STR( str )      HTMLOutFuncs::Out_String( rStrm, str, eDestEnc, &aNonConvertibleChars )
#define OUT_STR_NO_CONV( str )  HTMLOutFuncs::Out_String( rStrm, str, eDestEnc )
#define OUT_LF()            rStrm << ScExportBase::sNewLine << GetIndentStr()
#define lcl_OUT_LF()        rStrm << ScExportBase::sNewLine
#define TAG_ON_LF( tag )    (TAG_ON( tag ) << ScExportBase::sNewLine << GetIndentStr())
#define TAG_OFF_LF( tag )   (TAG_OFF( tag ) << ScExportBase::sNewLine << GetIndentStr())
#define OUT_HR()            TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_horzrule )
#define OUT_COMMENT( comment )  (rStrm << sMyBegComment, OUT_STR( comment ) \
                                << sMyEndComment << ScExportBase::sNewLine \
                                << GetIndentStr())
#define lcl_OUT_COMMENT( comment )  (rStrm << sMyBegComment, OUT_STR_NO_CONV( comment ) \
                                << sMyEndComment << ScExportBase::sNewLine)

#define OUT_SP_CSTR_ASS( s )    rStrm << ' ' << s << '='
#define APPEND_SPACE( s )   s.AppendAscii(" ")

#define GLOBSTR(id) ScGlobal::GetRscString( id )



//========================================================================

FltError ScFormatFilterPluginImpl::ScExportHTML( SvStream& rStrm, const String& rBaseURL, ScDocument* pDoc,
        const ScRange& rRange, const CharSet /*eNach*/, sal_Bool bAll,
        const String& rStreamPath, String& rNonConvertibleChars )
{
    ScHTMLExport aEx( rStrm, rBaseURL, pDoc, rRange, bAll, rStreamPath );
    FltError nErr = aEx.Write();
    rNonConvertibleChars = aEx.GetNonConvertibleChars();
    return nErr;
}


void lcl_AddStamp( String& rStr, const String& rName,
    const ::com::sun::star::util::DateTime& rDateTime,
    const LocaleDataWrapper& rLoc )
{
    Date aD(rDateTime.Day, rDateTime.Month, rDateTime.Year);
    Time aT(rDateTime.Hours, rDateTime.Minutes, rDateTime.Seconds,
            rDateTime.HundredthSeconds);
    DateTime aDateTime(aD,aT);

    String          aStrDate    = rLoc.getDate( aDateTime );
    String          aStrTime    = rLoc.getTime( aDateTime );

    rStr += GLOBSTR( STR_BY );
    APPEND_SPACE( rStr );
    if (rName.Len())
        rStr += rName;
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
    p += sprintf( p, "%02X", rColor.GetRed() );     // #100211# - checked
    p += sprintf( p, "%02X", rColor.GetGreen() );   // #100211# - checked
    p += sprintf( p, "%02X", rColor.GetBlue() );    // #100211# - checked
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

//////////////////////////////////////////////////////////////////////////////

ScHTMLExport::ScHTMLExport( SvStream& rStrmP, const String& rBaseURL, ScDocument* pDocP,
                            const ScRange& rRangeP,
                            sal_Bool bAllP, const String& rStreamPathP ) :
    ScExportBase( rStrmP, pDocP, rRangeP ),
    aBaseURL( rBaseURL ),
    aStreamPath( rStreamPathP ),
    pAppWin( Application::GetDefaultDevice() ),
    pSrcArr( NULL ),
    pDestArr( NULL ),
    nUsedTables( 0 ),
    nIndent( 0 ),
    bAll( bAllP ),
    bTabHasGraphics( sal_False ),
    bCalcAsShown( pDocP->GetDocOptions().IsCalcAsShown() ),
    bTableDataWidth( sal_True ),
    bTableDataHeight( sal_True )
{
    strcpy( sIndent, sIndentSource );       // #100211# - checked
    sIndent[0] = 0;

    // set HTML configuration
    SvxHtmlOptions* pHtmlOptions = SvxHtmlOptions::Get();
    eDestEnc = (pDoc->IsClipOrUndo() ? RTL_TEXTENCODING_UTF8 : pHtmlOptions->GetTextEncoding());
    bCopyLocalFileToINet = pHtmlOptions->IsSaveGraphicsLocal();
    for ( sal_uInt16 j=0; j < SC_HTML_FONTSIZES; j++ )
    {
        sal_uInt16 nSize = pHtmlOptions->GetFontSize( j );
        // remember in Twips, like our SvxFontHeightItem
        if ( nSize )
            nFontSize[j] = nSize * 20;
        else
            nFontSize[j] = nDefaultFontSize[j] * 20;
    }

    const SCTAB nCount = pDoc->GetTableCount();
    for ( SCTAB nTab = 0; nTab < nCount; nTab++ )
    {
        if ( !IsEmptyTable( nTab ) )
            nUsedTables++;
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


sal_uInt16 ScHTMLExport::GetFontSizeNumber( sal_uInt16 nHeight )
{
    sal_uInt16 nSize = 1;
    for ( sal_uInt16 j=SC_HTML_FONTSIZES-1; j>0; j-- )
    {
        if( nHeight > (nFontSize[j] + nFontSize[j-1]) / 2 )
        {   // der naechstgelegene
            nSize = j+1;
            break;
        }
    }
    return nSize;
}

const char* ScHTMLExport::GetFontSizeCss( sal_uInt16 nHeight )
{
    sal_uInt16 nSize = GetFontSizeNumber( nHeight );
    return pFontSizeCss[ nSize-1 ];
}


sal_uInt16 ScHTMLExport::ToPixel( sal_uInt16 nVal )
{
    if( nVal )
    {
        nVal = (sal_uInt16)pAppWin->LogicToPixel(
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


sal_uLong ScHTMLExport::Write()
{
    rStrm << '<' << OOO_STRING_SVTOOLS_HTML_doctype << ' ' << OOO_STRING_SVTOOLS_HTML_doctype32 << '>'
        << sNewLine << sNewLine;
    TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_html );
    WriteHeader();
    OUT_LF();
    WriteBody();
    OUT_LF();
    TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_html );

    return rStrm.GetError();
}


void ScHTMLExport::WriteHeader()
{
    IncIndent(1); TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_head );

    if ( pDoc->IsClipOrUndo() )
    {   // no real DocInfo available, but some META information like charset needed
        SfxFrameHTMLWriter::Out_DocInfo( rStrm, aBaseURL, NULL, sIndent, eDestEnc, &aNonConvertibleChars );
    }
    else
    {
        using namespace ::com::sun::star;
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDoc->GetDocumentShell()->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps
            = xDPS->getDocumentProperties();
        SfxFrameHTMLWriter::Out_DocInfo( rStrm, aBaseURL, xDocProps,
            sIndent, eDestEnc, &aNonConvertibleChars );
        OUT_LF();

        //----------------------------------------------------------
        if (!xDocProps->getPrintedBy().equalsAscii(""))
        {
            OUT_COMMENT( GLOBSTR( STR_DOC_INFO ) );
            String aStrOut( GLOBSTR( STR_DOC_PRINTED ) );
            aStrOut.AppendAscii( ": " );
            lcl_AddStamp( aStrOut, xDocProps->getPrintedBy(),
                xDocProps->getPrintDate(), *ScGlobal::pLocaleData );
            OUT_COMMENT( aStrOut );
        }
        //----------------------------------------------------------
    }
    OUT_LF();

    // CSS1 StyleSheet
    PageDefaults( bAll ? 0 : aRange.aStart.Tab() );
    IncIndent(1); TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_style );
    rStrm << sMyBegComment; OUT_LF();
    rStrm << OOO_STRING_SVTOOLS_HTML_body << "," << OOO_STRING_SVTOOLS_HTML_division << "," << OOO_STRING_SVTOOLS_HTML_table << ","
        << OOO_STRING_SVTOOLS_HTML_thead << "," << OOO_STRING_SVTOOLS_HTML_tbody << "," << OOO_STRING_SVTOOLS_HTML_tfoot << ","
        << OOO_STRING_SVTOOLS_HTML_tablerow << "," << OOO_STRING_SVTOOLS_HTML_tableheader << ","
        << OOO_STRING_SVTOOLS_HTML_tabledata << "," << OOO_STRING_SVTOOLS_HTML_parabreak << " { " << sFontFamily;
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
        << GetFontSizeCss( ( sal_uInt16 ) aHTMLStyle.nFontHeight ) << " }";
    OUT_LF();
    rStrm << sMyEndComment;
    IncIndent(-1); OUT_LF(); TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_style );

    IncIndent(-1); OUT_LF(); TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_head );
}


void ScHTMLExport::WriteOverview()
{
    if ( nUsedTables > 1 )
    {
        IncIndent(1);
        OUT_HR();
        IncIndent(1); TAG_ON( OOO_STRING_SVTOOLS_HTML_parabreak ); TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_center );
        TAG_ON( OOO_STRING_SVTOOLS_HTML_head1 );
        OUT_STR( ScGlobal::GetRscString( STR_OVERVIEW ) );
        TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_head1 );

        String       aStr;

        const SCTAB nCount = pDoc->GetTableCount();
        for ( SCTAB nTab = 0; nTab < nCount; nTab++ )
        {
            if ( !IsEmptyTable( nTab ) )
            {
                pDoc->GetName( nTab, aStr );
                rStrm << "<A HREF=\"#table"
                    << ByteString::CreateFromInt32( nTab ).GetBuffer()
                    << "\">";
                OUT_STR( aStr );
                rStrm << "</A>";
                TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_linebreak );
            }
        }

        IncIndent(-1); OUT_LF();
        IncIndent(-1); TAG_OFF( OOO_STRING_SVTOOLS_HTML_center ); TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_parabreak );
    }
}


const SfxItemSet& ScHTMLExport::PageDefaults( SCTAB nTab )
{
    SfxStyleSheetBasePool*  pStylePool  = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase*      pStyleSheet = NULL;
    DBG_ASSERT( pStylePool, "StylePool not found! :-(" );

    // remember defaults for compare in WriteCell
    if ( !aHTMLStyle.bInitialized )
    {
        pStylePool->SetSearchMask( SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL );
        pStyleSheet = pStylePool->Find(
                ScGlobal::GetRscString(STR_STYLENAME_STANDARD),
                SFX_STYLE_FAMILY_PARA );
        DBG_ASSERT( pStyleSheet, "ParaStyle not found! :-(" );
        if (!pStyleSheet)
            pStyleSheet = pStylePool->First();
        const SfxItemSet& rSetPara = pStyleSheet->GetItemSet();

        aHTMLStyle.nDefaultScriptType = ScGlobal::GetDefaultScriptType();
        aHTMLStyle.aFontFamilyName = ((const SvxFontItem&)(rSetPara.Get(
                        ScGlobal::GetScriptedWhichID(
                            aHTMLStyle.nDefaultScriptType, ATTR_FONT
                            )))).GetFamilyName();
        aHTMLStyle.nFontHeight = ((const SvxFontHeightItem&)(rSetPara.Get(
                        ScGlobal::GetScriptedWhichID(
                            aHTMLStyle.nDefaultScriptType, ATTR_FONT_HEIGHT
                            )))).GetHeight();
        aHTMLStyle.nFontSizeNumber = GetFontSizeNumber( static_cast< sal_uInt16 >( aHTMLStyle.nFontHeight ) );
    }

    // Page style sheet printer settings, e.g. for background graphics.
    // There's only one background graphic in HTML!
    pStylePool->SetSearchMask( SFX_STYLE_FAMILY_PAGE, SFXSTYLEBIT_ALL );
    pStyleSheet = pStylePool->Find( pDoc->GetPageStyle( nTab ), SFX_STYLE_FAMILY_PAGE );
    DBG_ASSERT( pStyleSheet, "PageStyle not found! :-(" );
    if (!pStyleSheet)
        pStyleSheet = pStylePool->First();
    const SfxItemSet& rSet = pStyleSheet->GetItemSet();
    if ( !aHTMLStyle.bInitialized )
    {
        const SvxBrushItem* pBrushItem = (const SvxBrushItem*)&rSet.Get( ATTR_BACKGROUND );
        aHTMLStyle.aBackgroundColor = pBrushItem->GetColor();
        aHTMLStyle.bInitialized = sal_True;
    }
    return rSet;
}


void ScHTMLExport::BorderToStyle( ByteString& rOut, const char* pBorderName,
        const SvxBorderLine* pLine, bool& bInsertSemicolon )
{
    if ( pLine )
    {
        if ( bInsertSemicolon )
            rOut += "; ";

        // which border
        ((rOut += "border-") += pBorderName) += ": ";

        // thickness
        int nWidth = pLine->GetOutWidth();
        int nPxWidth = ( nWidth > 0 )? std::max( int( nWidth / TWIPS_PER_PIXEL ), 1 ): 0;
        (rOut += ByteString::CreateFromInt32( nPxWidth )) += "px solid #";

        // color
        char hex[7];
        snprintf( hex, 7, "%06x", static_cast< unsigned int >( pLine->GetColor().GetRGBColor() ) );
        hex[6] = 0;

        rOut += hex;

        bInsertSemicolon = true;
    }
}

void ScHTMLExport::WriteBody()
{
    const SfxItemSet& rSet = PageDefaults( bAll ? 0 : aRange.aStart.Tab() );
    const SvxBrushItem* pBrushItem = (const SvxBrushItem*)&rSet.Get( ATTR_BACKGROUND );

    // default Textfarbe schwarz
    rStrm << '<' << OOO_STRING_SVTOOLS_HTML_body << ' ' << OOO_STRING_SVTOOLS_HTML_O_text << "=\"#000000\"";

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
                sal_uInt16 nErr = XOutBitmap::WriteGraphic( *pGrf, aGrfNm,
                    CREATE_STRING( "JPG" ), XOUTBMP_USE_NATIVE_IF_POSSIBLE );
                if( !nErr )     // fehlerhaft, da ist nichts auszugeben
                {
                    aGrfNm = URIHelper::SmartRel2Abs(
                            INetURLObject(aBaseURL),
                            aGrfNm, URIHelper::GetMaybeFileHdl(), true, false);
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
                aGrfNm = URIHelper::SmartRel2Abs(
                        INetURLObject(aBaseURL),
                        aGrfNm, URIHelper::GetMaybeFileHdl(), true, false);
            pLink = &aGrfNm;
        }
        if( pLink )
        {
            rStrm << ' ' << OOO_STRING_SVTOOLS_HTML_O_background << "=\"";
            OUT_STR( URIHelper::simpleNormalizedMakeRelative(
                        aBaseURL,
                        *pLink ) ) << '\"';
        }
    }
    if ( !aHTMLStyle.aBackgroundColor.GetTransparency() )
    {   // A transparent background color should always result in default
        // background of the browser. Also, HTMLOutFuncs::Out_Color() writes
        // black #000000 for COL_AUTO which is the same as white #ffffff with
        // transparency set to 0xff, our default background.
        OUT_SP_CSTR_ASS( OOO_STRING_SVTOOLS_HTML_O_bgcolor );
        HTMLOutFuncs::Out_Color( rStrm, aHTMLStyle.aBackgroundColor );
    }

    rStrm << '>'; OUT_LF();

    if ( bAll )
        WriteOverview();

    WriteTables();

    TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_body );
}


void ScHTMLExport::WriteTables()
{
    const SCTAB nTabCount = pDoc->GetTableCount();
    const String    aStrTable( ScResId( SCSTR_TABLE ) );
    String          aStr;
    String          aStrOut;
    SCCOL           nStartCol;
    SCROW           nStartRow;
    SCTAB           nStartTab;
    SCCOL           nEndCol;
    SCROW           nEndRow;
    SCTAB           nEndTab;
    SCCOL           nStartColFix = 0;
    SCROW           nStartRowFix = 0;
    SCCOL           nEndColFix = 0;
    SCROW           nEndRowFix = 0;
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
    SCTAB nTableStrNum = 1;
    for ( SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++ )
    {
        if ( !pDoc->IsVisible( nTab ) )
            continue;   // for

        if ( bAll )
        {
            if ( !GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow ) )
                continue;   // for

            if ( nUsedTables > 1 )
            {
                aStrOut  = aStrTable;
                aStrOut.AppendAscii( " " );
                aStrOut += String::CreateFromInt32( nTableStrNum++ );
                aStrOut.AppendAscii( ": " );

                OUT_HR();

                // Anker festlegen:
                rStrm << "<A NAME=\"table"
                    << ByteString::CreateFromInt32( nTab ).GetBuffer()
                    << "\">";
                TAG_ON( OOO_STRING_SVTOOLS_HTML_head1 );
                OUT_STR( aStrOut );
                TAG_ON( OOO_STRING_SVTOOLS_HTML_emphasis );

                pDoc->GetName( nTab, aStr );
                OUT_STR( aStr );

                TAG_OFF( OOO_STRING_SVTOOLS_HTML_emphasis );
                TAG_OFF( OOO_STRING_SVTOOLS_HTML_head1 );
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
        ByteString  aByteStrOut = OOO_STRING_SVTOOLS_HTML_table;
//      aStrOut  = OOO_STRING_SVTOOLS_HTML_table;

        // FRAME=VOID, we do the styling of the cells in <TD>
        (((aByteStrOut += ' ') += OOO_STRING_SVTOOLS_HTML_frame) += '=') += OOO_STRING_SVTOOLS_HTML_TF_void;

        bTabHasGraphics = bTabAlignedLeft = sal_False;
        if ( bAll && pDrawLayer )
            PrepareGraphics( pDrawLayer, nTab, nStartCol, nStartRow,
                nEndCol, nEndRow );

        // more <TABLE ...>
        if ( bTabAlignedLeft )
            (((aByteStrOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_align) += '=') += OOO_STRING_SVTOOLS_HTML_AL_left;
            // ALIGN=LEFT allow text and graphics to flow around
        // CELLSPACING
        (((aByteStrOut += ' ' ) += OOO_STRING_SVTOOLS_HTML_O_cellspacing ) += '=') +=
                                    ByteString::CreateFromInt32( nCellSpacing );
        // COLS=n
        SCCOL nColCnt = 0;
        SCCOL nCol;
        for ( nCol=nStartCol; nCol<=nEndCol; nCol++ )
        {
            if ( !pDoc->ColHidden(nCol, nTab) )
                ++nColCnt;
        }
        (((aByteStrOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_cols) += '=') += ByteString::CreateFromInt32( nColCnt );

        // RULES=NONE, we do the styling of the cells in <TD>
        (((aByteStrOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_rules) += '=') += OOO_STRING_SVTOOLS_HTML_TR_none;

        // BORDER=0, we do the styling of the cells in <TD>
        ((aByteStrOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_border) += "=0";
        IncIndent(1); TAG_ON_LF( aByteStrOut.GetBuffer() );

        // <COLGROUP>
        TAG_ON( OOO_STRING_SVTOOLS_HTML_colgroup );
        // <COL WIDTH=x> as pre-info for long tables
        ByteString  aByteStr = OOO_STRING_SVTOOLS_HTML_col;
        aByteStr += ' ';
        aByteStr += OOO_STRING_SVTOOLS_HTML_O_width;
        aByteStr += '=';
        for ( nCol=nStartCol; nCol<=nEndCol; nCol++ )
        {
            if ( pDoc->ColHidden(nCol, nTab) )
                continue;   // for

            aByteStrOut  = aByteStr;
            aByteStrOut += ByteString::CreateFromInt32(
                                ToPixel( pDoc->GetColWidth( nCol, nTab ) ) );
            TAG_ON( aByteStrOut.GetBuffer() );
        }
        TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_colgroup );

        // <TBODY>
        IncIndent(1); TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_tbody );
        // At least old (3.x, 4.x?) Netscape doesn't follow <TABLE COLS=n> and
        // <COL WIDTH=x> specified, but needs a width at every column.
        bTableDataWidth = sal_True;     // widths in first row
        bool bHasHiddenRows = pDoc->HasHiddenRows(nStartRow, nEndRow, nTab);
        for ( SCROW nRow=nStartRow; nRow<=nEndRow; nRow++ )
        {
            if ( bHasHiddenRows && pDoc->RowHidden(nRow, nTab) )
            {
                nRow = pDoc->FirstVisibleRow(nRow+1, nEndRow, nTab);
                --nRow;
                continue;   // for
            }

            IncIndent(1); TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_tablerow );
            bTableDataHeight = sal_True;  // height at every first cell of each row
            for ( SCCOL nCol2=nStartCol; nCol2<=nEndCol; nCol2++ )
            {
                if ( pDoc->ColHidden(nCol2, nTab) )
                    continue;   // for

                if ( nCol2 == nEndCol )
                    IncIndent(-1);
                WriteCell( nCol2, nRow, nTab );
                bTableDataHeight = sal_False;
            }
            bTableDataWidth = sal_False;    // widths only in first row

            if ( nRow == nEndRow )
                IncIndent(-1);
            TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_tablerow );
        }
        IncIndent(-1); TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_tbody );

        IncIndent(-1); TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_table );

        if ( bTabHasGraphics )
        {
            // the rest that is not in a cell
            for ( ScHTMLGraphEntry* pE = aGraphList.First(); pE; pE = aGraphList.Next() )
            {
                if ( !pE->bWritten )
                    WriteGraphEntry( pE );
                delete pE;
            }
            aGraphList.Clear();
            if ( bTabAlignedLeft )
            {   // clear <TABLE ALIGN=LEFT> with <BR CLEAR=LEFT>
                aByteStrOut = OOO_STRING_SVTOOLS_HTML_linebreak;
                (((aByteStrOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_clear) += '=') += OOO_STRING_SVTOOLS_HTML_AL_left;
                TAG_ON_LF( aByteStrOut.GetBuffer() );
            }
        }

        if ( bAll )
            OUT_COMMENT( CREATE_STRING( "**************************************************************************" ) );
    }
}


void ScHTMLExport::WriteCell( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    const ScPatternAttr* pAttr = pDoc->GetPattern( nCol, nRow, nTab );
    const SfxItemSet* pCondItemSet = pDoc->GetCondResult( nCol, nRow, nTab );

    const ScMergeFlagAttr& rMergeFlagAttr = (const ScMergeFlagAttr&) pAttr->GetItem( ATTR_MERGE_FLAG, pCondItemSet );
    if ( rMergeFlagAttr.IsOverlapped() )
        return ;

    ScAddress aPos( nCol, nRow, nTab );
    ScHTMLGraphEntry* pGraphEntry = NULL;
    if ( bTabHasGraphics )
    {
        for ( pGraphEntry = aGraphList.First(); pGraphEntry;
              pGraphEntry = aGraphList.Next() )
        {
            if ( pGraphEntry->bInCell && pGraphEntry->aRange.In( aPos ) )
            {
                if ( pGraphEntry->aRange.aStart == aPos )
                    break;  // for
                else
                    return ;        // ist ein Col/RowSpan, Overlapped
            }
        }
    }

    ScBaseCell* pCell = pDoc->GetCell( aPos );
    sal_uLong nFormat = pAttr->GetNumberFormat( pFormatter );
    sal_Bool bValueData;
    sal_uInt8 nScriptType;
    if ( pCell )
    {
        bValueData = pCell->HasValueData();
        nScriptType = pDoc->GetScriptType( nCol, nRow, nTab, pCell );
    }
    else
    {
        bValueData = sal_False;
        nScriptType = 0;
    }
    if ( nScriptType == 0 )
        nScriptType = aHTMLStyle.nDefaultScriptType;


    ByteString aStrTD = OOO_STRING_SVTOOLS_HTML_tabledata;

    // border of the cells
    SvxBoxItem* pBorder = (SvxBoxItem*) pDoc->GetAttr( nCol, nRow, nTab, ATTR_BORDER );
    if ( pBorder && (pBorder->GetTop() || pBorder->GetBottom() || pBorder->GetLeft() || pBorder->GetRight()) )
    {
        ((aStrTD += ' ') += OOO_STRING_SVTOOLS_HTML_style) += "=\"";

        bool bInsertSemicolon = false;
        BorderToStyle( aStrTD, "top", pBorder->GetTop(), bInsertSemicolon );
        BorderToStyle( aStrTD, "bottom", pBorder->GetBottom(), bInsertSemicolon );
        BorderToStyle( aStrTD, "left", pBorder->GetLeft(), bInsertSemicolon );
        BorderToStyle( aStrTD, "right", pBorder->GetRight(), bInsertSemicolon );

        aStrTD += '"';
    }

    const sal_Char* pChar;
    sal_uInt16 nWidthPixel;
    sal_uInt16 nHeightPixel;

    const ScMergeAttr& rMergeAttr = (const ScMergeAttr&) pAttr->GetItem( ATTR_MERGE, pCondItemSet );
    if ( pGraphEntry || rMergeAttr.IsMerged() )
    {
        SCCOL nC, jC;
        SCROW nR;
        sal_uLong v;
        if ( pGraphEntry )
            nC = Max( SCCOL(pGraphEntry->aRange.aEnd.Col() - nCol + 1),
                SCCOL(rMergeAttr.GetColMerge()) );
        else
            nC = rMergeAttr.GetColMerge();
        if ( nC > 1 )
        {
            (((aStrTD += ' ') += OOO_STRING_SVTOOLS_HTML_O_colspan) += '=') += ByteString::CreateFromInt32( nC );
            nC = nC + nCol;
            for ( jC=nCol, v=0; jC<nC; jC++ )
                v += pDoc->GetColWidth( jC, nTab );
            nWidthPixel = ToPixel( static_cast< sal_uInt16 >( v ) );
        }
        else
            nWidthPixel = ToPixel( pDoc->GetColWidth( nCol, nTab ) );

        if ( pGraphEntry )
            nR = Max( SCROW(pGraphEntry->aRange.aEnd.Row() - nRow + 1),
                SCROW(rMergeAttr.GetRowMerge()) );
        else
            nR = rMergeAttr.GetRowMerge();
        if ( nR > 1 )
        {
            (((aStrTD += ' ') += OOO_STRING_SVTOOLS_HTML_O_rowspan) += '=') += ByteString::CreateFromInt32( nR );
            nR += nRow;
            v = pDoc->GetRowHeight( nRow, nR-1, nTab );
            nHeightPixel = ToPixel( static_cast< sal_uInt16 >( v ) );
        }
        else
            nHeightPixel = ToPixel( pDoc->GetRowHeight( nRow, nTab ) );
    }
    else
    {
        nWidthPixel = ToPixel( pDoc->GetColWidth( nCol, nTab ) );
        nHeightPixel = ToPixel( pDoc->GetRowHeight( nRow, nTab ) );
    }

    if ( bTableDataWidth )
        (((aStrTD += ' ') += OOO_STRING_SVTOOLS_HTML_O_width) += '=') += ByteString::CreateFromInt32( nWidthPixel );
    if ( bTableDataHeight )
        (((aStrTD += ' ') += OOO_STRING_SVTOOLS_HTML_O_height) += '=') += ByteString::CreateFromInt32( nHeightPixel );

    const SvxFontItem& rFontItem = (const SvxFontItem&) pAttr->GetItem(
            ScGlobal::GetScriptedWhichID( nScriptType, ATTR_FONT),
            pCondItemSet);

    const SvxFontHeightItem& rFontHeightItem = (const SvxFontHeightItem&)
        pAttr->GetItem( ScGlobal::GetScriptedWhichID( nScriptType,
                    ATTR_FONT_HEIGHT), pCondItemSet);

    const SvxWeightItem& rWeightItem = (const SvxWeightItem&) pAttr->GetItem(
            ScGlobal::GetScriptedWhichID( nScriptType, ATTR_FONT_WEIGHT),
            pCondItemSet);

    const SvxPostureItem& rPostureItem = (const SvxPostureItem&)
        pAttr->GetItem( ScGlobal::GetScriptedWhichID( nScriptType,
                    ATTR_FONT_POSTURE), pCondItemSet);

    const SvxUnderlineItem& rUnderlineItem = (const SvxUnderlineItem&)
        pAttr->GetItem( ATTR_FONT_UNDERLINE, pCondItemSet );

    const SvxColorItem& rColorItem = (const SvxColorItem&) pAttr->GetItem(
            ATTR_FONT_COLOR, pCondItemSet );

    const SvxHorJustifyItem& rHorJustifyItem = (const SvxHorJustifyItem&)
        pAttr->GetItem( ATTR_HOR_JUSTIFY, pCondItemSet );

    const SvxVerJustifyItem& rVerJustifyItem = (const SvxVerJustifyItem&)
        pAttr->GetItem( ATTR_VER_JUSTIFY, pCondItemSet );

    const SvxBrushItem& rBrushItem = (const SvxBrushItem&) pAttr->GetItem(
            ATTR_BACKGROUND, pCondItemSet );

    Color aBgColor;
    if ( rBrushItem.GetColor().GetTransparency() == 255 )
        aBgColor = aHTMLStyle.aBackgroundColor;     // #55121# keine ungewollte Hintergrundfarbe
    else
        aBgColor = rBrushItem.GetColor();

    sal_Bool bBold          = ( WEIGHT_BOLD     <= rWeightItem.GetWeight() );
    sal_Bool bItalic        = ( ITALIC_NONE     != rPostureItem.GetPosture() );
    sal_Bool bUnderline     = ( UNDERLINE_NONE  != rUnderlineItem.GetLineStyle() );
    sal_Bool bSetFontColor  = ( COL_AUTO        != rColorItem.GetValue().GetColor() );  // #97650# default is AUTO now
#if 0
// keine StyleSheet-Fontangaben: hart fuer jede Zelle
    sal_Bool bSetFontName   = sal_True;
    sal_uInt16 nSetFontSizeNumber = GetFontSizeNumber( (sal_uInt16)rFontHeightItem.GetHeight() );
#else
    sal_Bool bSetFontName   = ( aHTMLStyle.aFontFamilyName  != rFontItem.GetFamilyName() );
    sal_uInt16 nSetFontSizeNumber = 0;
    sal_uInt32 nFontHeight = rFontHeightItem.GetHeight();
    if ( nFontHeight != aHTMLStyle.nFontHeight )
    {
        nSetFontSizeNumber = GetFontSizeNumber( (sal_uInt16) nFontHeight );
        if ( nSetFontSizeNumber == aHTMLStyle.nFontSizeNumber )
            nSetFontSizeNumber = 0;   // no difference, don't set
    }
#endif
    sal_Bool bSetFont = (bSetFontColor || bSetFontName || nSetFontSizeNumber);

    //! TODO: we could entirely use CSS1 here instead, but that would exclude
    //! Netscape 3.0 and Netscape 4.x without JavaScript enabled.
    //! Do we want that?

    switch( rHorJustifyItem.GetValue() )
    {
        case SVX_HOR_JUSTIFY_STANDARD:
            pChar = (bValueData ? OOO_STRING_SVTOOLS_HTML_AL_right : OOO_STRING_SVTOOLS_HTML_AL_left);
            break;
        case SVX_HOR_JUSTIFY_CENTER:    pChar = OOO_STRING_SVTOOLS_HTML_AL_center;  break;
        case SVX_HOR_JUSTIFY_BLOCK:     pChar = OOO_STRING_SVTOOLS_HTML_AL_justify; break;
        case SVX_HOR_JUSTIFY_RIGHT:     pChar = OOO_STRING_SVTOOLS_HTML_AL_right;       break;
        case SVX_HOR_JUSTIFY_LEFT:
        case SVX_HOR_JUSTIFY_REPEAT:
        default:                        pChar = OOO_STRING_SVTOOLS_HTML_AL_left;        break;
    }

    (((aStrTD += ' ') += OOO_STRING_SVTOOLS_HTML_O_align) += '=') += pChar;

    switch( rVerJustifyItem.GetValue() )
    {
        case SVX_VER_JUSTIFY_TOP:       pChar = OOO_STRING_SVTOOLS_HTML_VA_top;     break;
        case SVX_VER_JUSTIFY_CENTER:    pChar = OOO_STRING_SVTOOLS_HTML_VA_middle;  break;
        case SVX_VER_JUSTIFY_BOTTOM:    pChar = OOO_STRING_SVTOOLS_HTML_VA_bottom;  break;
        case SVX_VER_JUSTIFY_STANDARD:
        default:                        pChar = NULL;
    }
    if ( pChar )
        (((aStrTD += ' ') += OOO_STRING_SVTOOLS_HTML_O_valign) += '=') += pChar;

    if ( aHTMLStyle.aBackgroundColor != aBgColor )
    {
        ((aStrTD += ' ') += OOO_STRING_SVTOOLS_HTML_O_bgcolor) += '=';
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
        nFormat, *pFormatter, eDestEnc, &aNonConvertibleChars );

    TAG_ON( aStrTD.GetBuffer() );

    if ( bBold )        TAG_ON( OOO_STRING_SVTOOLS_HTML_bold );
    if ( bItalic )      TAG_ON( OOO_STRING_SVTOOLS_HTML_italic );
    if ( bUnderline )   TAG_ON( OOO_STRING_SVTOOLS_HTML_underline );


    if ( bSetFont )
    {
        ByteString  aStr = OOO_STRING_SVTOOLS_HTML_font;
        if ( bSetFontName )
        {
            ((aStr += ' ') += OOO_STRING_SVTOOLS_HTML_O_face) += "=\"";
            xub_StrLen nFonts = rFontItem.GetFamilyName().GetTokenCount( ';' );
            if ( nFonts == 1 )
            {
                ByteString  aTmpStr;
                HTMLOutFuncs::ConvertStringToHTML( rFontItem.GetFamilyName(),
                    aTmpStr, eDestEnc, &aNonConvertibleChars );
                aStr += aTmpStr;
            }
            else
            {   // Fontliste, VCL: Semikolon als Separator, HTML: Komma
                const String& rList = rFontItem.GetFamilyName();
                for ( xub_StrLen j = 0, nPos = 0; j < nFonts; j++ )
                {
                    ByteString  aTmpStr;
                    HTMLOutFuncs::ConvertStringToHTML(
                        rList.GetToken( 0, ';', nPos ), aTmpStr, eDestEnc,
                        &aNonConvertibleChars );
                    aStr += aTmpStr;
                    if ( j < nFonts-1 )
                        aStr += ',';
                }
            }
            aStr += '\"';
        }
        if ( nSetFontSizeNumber )
        {
            (((aStr += ' ') += OOO_STRING_SVTOOLS_HTML_O_size) += '=')
                += ByteString::CreateFromInt32( nSetFontSizeNumber );
        }
        if ( bSetFontColor )
        {
            Color   aColor = rColorItem.GetValue();

            //  always export automatic text color as black
            if ( aColor.GetColor() == COL_AUTO )
                aColor.SetColor( COL_BLACK );

            ((aStr += ' ') += OOO_STRING_SVTOOLS_HTML_O_color) += '=';
            lcl_AppendHTMLColorTripel( aStr, aColor );
        }
        TAG_ON( aStr.GetBuffer() );
    }

    String aStrOut;
    sal_Bool bFieldText = sal_False;
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
        {
            TAG_ON( OOO_STRING_SVTOOLS_HTML_linebreak );        // #42573# keine komplett leere Zelle
        }
        else
        {
            xub_StrLen nPos = aStrOut.Search( _LF );
            if ( nPos == STRING_NOTFOUND )
            {
                OUT_STR( aStrOut );
            }
            else
            {
                xub_StrLen nStartPos = 0;
                do
                {
                    String aSingleLine( aStrOut, nStartPos, nPos - nStartPos );
                    OUT_STR( aSingleLine );
                    TAG_ON( OOO_STRING_SVTOOLS_HTML_linebreak );
                    nStartPos = nPos + 1;
                }
                while( ( nPos = aStrOut.Search( _LF, nStartPos ) ) != STRING_NOTFOUND );
                String aSingleLine( aStrOut, nStartPos, aStrOut.Len() - nStartPos );
                OUT_STR( aSingleLine );
            }
        }
    }
    if ( pGraphEntry )
        WriteGraphEntry( pGraphEntry );

    if ( bSetFont )     TAG_OFF( OOO_STRING_SVTOOLS_HTML_font );
    if ( bUnderline )   TAG_OFF( OOO_STRING_SVTOOLS_HTML_underline );
    if ( bItalic )      TAG_OFF( OOO_STRING_SVTOOLS_HTML_italic );
    if ( bBold )        TAG_OFF( OOO_STRING_SVTOOLS_HTML_bold );

    TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_tabledata );
}


sal_Bool ScHTMLExport::WriteFieldText( const ScEditCell* pCell )
{
    sal_Bool bFields = sal_False;
    const EditTextObject* pData;
    pCell->GetData( pData );
    // text and anchor of URL fields, Doc-Engine is a ScFieldEditEngine
    EditEngine& rEngine = pDoc->GetEditEngine();
    rEngine.SetText( *pData );
    sal_uInt16 nParas = rEngine.GetParagraphCount();
    if ( nParas )
    {
        ESelection aSel( 0, 0, nParas-1, rEngine.GetTextLen( nParas-1 ) );
        SfxItemSet aSet( rEngine.GetAttribs( aSel ) );
        SfxItemState eFieldState = aSet.GetItemState( EE_FEATURE_FIELD, sal_False );
        if ( eFieldState == SFX_ITEM_DONTCARE || eFieldState == SFX_ITEM_SET )
            bFields = sal_True;
    }
    if ( bFields )
    {
        sal_Bool bOldUpdateMode = rEngine.GetUpdateMode();
        rEngine.SetUpdateMode( sal_True );      // no portions if not formatted
        for ( sal_uInt16 nPar=0; nPar < nParas; nPar++ )
        {
            if ( nPar > 0 )
                TAG_ON( OOO_STRING_SVTOOLS_HTML_linebreak );
            SvUShorts aPortions;
            rEngine.GetPortions( nPar, aPortions );
            sal_uInt16 nCnt = aPortions.Count();
            sal_uInt16 nStart = 0;
            for ( sal_uInt16 nPos = 0; nPos < nCnt; nPos++ )
            {
                sal_uInt16 nEnd = aPortions.GetObject( nPos );
                ESelection aSel( nPar, nStart, nPar, nEnd );
                sal_Bool bUrl = sal_False;
                // fields are single characters
                if ( nEnd == nStart+1 )
                {
                    const SfxPoolItem* pItem;
                    SfxItemSet aSet = rEngine.GetAttribs( aSel );
                    if ( aSet.GetItemState( EE_FEATURE_FIELD, sal_False, &pItem ) == SFX_ITEM_ON )
                    {
                        const SvxFieldData* pField = ((const SvxFieldItem*)pItem)->GetField();
                        if ( pField && pField->ISA(SvxURLField) )
                        {
                            bUrl = sal_True;
                            const SvxURLField*  pURLField = (const SvxURLField*)pField;
//                          String              aFieldText = rEngine.GetText( aSel );
                            rStrm << '<' << OOO_STRING_SVTOOLS_HTML_anchor << ' ' << OOO_STRING_SVTOOLS_HTML_O_href << "=\"";
                            OUT_STR( pURLField->GetURL() );
                            rStrm << "\">";
                            OUT_STR( pURLField->GetRepresentation() );
                            rStrm << "</" << OOO_STRING_SVTOOLS_HTML_anchor << '>';
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


sal_Bool ScHTMLExport::CopyLocalFileToINet( String& rFileNm,
        const String& rTargetNm, sal_Bool bFileToFile )
{
    sal_Bool bRet = sal_False;
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
            sal_uInt16 nPos;
            if( pSrcArr->Seek_Entry( &rFileNm, &nPos ))
            {
                rFileNm = *(*pDestArr)[ nPos ];
                return sal_True;
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
        *pDest += String(aFileUrl.GetName());

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
                                sal_False );

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
    nIndent = nIndent + nVal;
    if ( nIndent < 0 )
        nIndent = 0;
    else if ( nIndent > nIndentMax )
        nIndent = nIndentMax;
    sIndent[nIndent] = 0;
}



