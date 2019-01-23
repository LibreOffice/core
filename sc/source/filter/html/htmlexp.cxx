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

#include <scitems.hxx>
#include <editeng/eeitem.hxx>

#include <vcl/svapp.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svx/xoutbmp.hxx>
#include <editeng/editeng.hxx>
#include <svtools/htmlcfg.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frmhtmlw.hxx>
#include <sfx2/objsh.hxx>
#include <svl/urihelper.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/parhtml.hxx>
#include <vcl/outdev.hxx>
#include <stdio.h>
#include <osl/diagnose.h>

#include <htmlexp.hxx>
#include <global.hxx>
#include <postit.hxx>
#include <document.hxx>
#include <attrib.hxx>
#include <patattr.hxx>
#include <stlpool.hxx>
#include <scresid.hxx>
#include <formulacell.hxx>
#include <cellform.hxx>
#include <docoptio.hxx>
#include <editutil.hxx>
#include <ftools.hxx>
#include <cellvalue.hxx>

#include <editeng/flditem.hxx>
#include <editeng/borderline.hxx>

// Without strings.hrc: error C2679: binary '=' : no operator defined which takes a
// right-hand operand of type 'const class String (__stdcall *)(class ScResId)'
// at
// const String aStrTable( ScResId( SCSTR_TABLE ) ); aStrOut = aStrTable;
// ?!???
#include <strings.hrc>
#include <globstr.hrc>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <rtl/strbuf.hxx>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;

const static sal_Char sMyBegComment[]   = "<!-- ";
const static sal_Char sMyEndComment[]   = " -->";
const static sal_Char sDisplay[]        = "display:";
const static sal_Char sBorder[]         = "border:";
const static sal_Char sBackground[]     = "background:";

const sal_uInt16 ScHTMLExport::nDefaultFontSize[SC_HTML_FONTSIZES] =
{
    HTMLFONTSZ1_DFLT, HTMLFONTSZ2_DFLT, HTMLFONTSZ3_DFLT, HTMLFONTSZ4_DFLT,
    HTMLFONTSZ5_DFLT, HTMLFONTSZ6_DFLT, HTMLFONTSZ7_DFLT
};

sal_uInt16 ScHTMLExport::nFontSize[SC_HTML_FONTSIZES] = { 0 };

const char* ScHTMLExport::pFontSizeCss[SC_HTML_FONTSIZES] =
{
    "xx-small", "x-small", "small", "medium", "large", "x-large", "xx-large"
};

const sal_uInt16 ScHTMLExport::nCellSpacing = 0;
const sal_Char ScHTMLExport::sIndentSource[nIndentMax+1] =
    "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

// Macros for HTML export

#define TAG_ON( tag )       HTMLOutFuncs::Out_AsciiTag( rStrm, tag )
#define TAG_OFF( tag )      HTMLOutFuncs::Out_AsciiTag( rStrm, tag, false )
#define OUT_STR( str )      HTMLOutFuncs::Out_String( rStrm, str, eDestEnc, &aNonConvertibleChars )
#define OUT_LF()            rStrm.WriteCharPtr( SAL_NEWLINE_STRING ).WriteCharPtr( GetIndentStr() )
#define TAG_ON_LF( tag )    (TAG_ON( tag ).WriteCharPtr( SAL_NEWLINE_STRING ).WriteCharPtr( GetIndentStr() ))
#define TAG_OFF_LF( tag )   (TAG_OFF( tag ).WriteCharPtr( SAL_NEWLINE_STRING ).WriteCharPtr( GetIndentStr() ))
#define OUT_HR()            TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_horzrule )
#define OUT_COMMENT( comment )  (rStrm.WriteCharPtr( sMyBegComment ), OUT_STR( comment ) \
                                .WriteCharPtr( sMyEndComment ).WriteCharPtr( SAL_NEWLINE_STRING ) \
                                .WriteCharPtr( GetIndentStr() ))

#define OUT_SP_CSTR_ASS( s )    rStrm.WriteChar( ' ').WriteCharPtr( s ).WriteChar( '=' )

#define GLOBSTR(id) ScResId( id )

void ScFormatFilterPluginImpl::ScExportHTML( SvStream& rStrm, const OUString& rBaseURL, ScDocument* pDoc,
        const ScRange& rRange, const rtl_TextEncoding /*eNach*/, bool bAll,
        const OUString& rStreamPath, OUString& rNonConvertibleChars, const OUString& rFilterOptions )
{
    ScHTMLExport aEx( rStrm, rBaseURL, pDoc, rRange, bAll, rStreamPath, rFilterOptions );
    aEx.Write();
    rNonConvertibleChars = aEx.GetNonConvertibleChars();
}

static OString lcl_getColGroupString(sal_Int32 nSpan, sal_Int32 nWidth)
{
    OStringBuffer aByteStr(OOO_STRING_SVTOOLS_HTML_colgroup);
    aByteStr.append(' ');
    if( nSpan > 1 )
    {
        aByteStr.append(OOO_STRING_SVTOOLS_HTML_O_span);
        aByteStr.append("=\"");
        aByteStr.append(nSpan);
        aByteStr.append("\" ");
    }
    aByteStr.append(OOO_STRING_SVTOOLS_HTML_O_width);
    aByteStr.append("=\"");
    aByteStr.append(nWidth);
    aByteStr.append('"');
    return aByteStr.makeStringAndClear();
}

static void lcl_AddStamp( OUString& rStr, const OUString& rName,
    const css::util::DateTime& rDateTime,
    const LocaleDataWrapper& rLoc )
{
    Date aD(rDateTime.Day, rDateTime.Month, rDateTime.Year);
    tools::Time aT(rDateTime.Hours, rDateTime.Minutes, rDateTime.Seconds,
            rDateTime.NanoSeconds);
    DateTime aDateTime(aD,aT);

    OUString        aStrDate    = rLoc.getDate( aDateTime );
    OUString        aStrTime    = rLoc.getTime( aDateTime );

    rStr += GLOBSTR( STR_BY ) + " ";
    if (!rName.isEmpty())
        rStr += rName;
    else
        rStr += "???";
    rStr += " " + GLOBSTR( STR_ON ) + " ";
    if (!aStrDate.isEmpty())
        rStr += aStrDate;
    else
        rStr += "???";
    rStr += ", ";
    if (!aStrTime.isEmpty())
        rStr += aStrTime;
    else
        rStr += "???";
}

static OString lcl_makeHTMLColorTriplet(const Color& rColor)
{
    sal_Char    buf[24];

    // <font COLOR="#00FF40">hello</font>
    snprintf( buf, 24, "\"#%02X%02X%02X\"", rColor.GetRed(), rColor.GetGreen(), rColor.GetBlue() );

    return OString(buf);
}

ScHTMLExport::ScHTMLExport( SvStream& rStrmP, const OUString& rBaseURL, ScDocument* pDocP,
                            const ScRange& rRangeP, bool bAllP,
                            const OUString& rStreamPathP, const OUString& rFilterOptions ) :
    ScExportBase( rStrmP, pDocP, rRangeP ),
    aBaseURL( rBaseURL ),
    aStreamPath( rStreamPathP ),
    pAppWin( Application::GetDefaultDevice() ),
    nUsedTables( 0 ),
    nIndent( 0 ),
    bAll( bAllP ),
    bTabHasGraphics( false ),
    bTabAlignedLeft( false ),
    bCalcAsShown( pDocP->GetDocOptions().IsCalcAsShown() ),
    bTableDataHeight( true ),
    mbSkipImages ( false ),
    mbSkipHeaderFooter( false )
{
    strcpy( sIndent, sIndentSource );
    sIndent[0] = 0;

    // set HTML configuration
    SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();
    eDestEnc = (pDoc->IsClipOrUndo() ? RTL_TEXTENCODING_UTF8 : rHtmlOptions.GetTextEncoding());
    bCopyLocalFileToINet = rHtmlOptions.IsSaveGraphicsLocal();

    if (rFilterOptions == "SkipImages")
    {
        mbSkipImages = true;
    }
    else if (rFilterOptions == "SkipHeaderFooter")
    {
        mbSkipHeaderFooter = true;
    }

    for ( sal_uInt16 j=0; j < SC_HTML_FONTSIZES; j++ )
    {
        sal_uInt16 nSize = rHtmlOptions.GetFontSize( j );
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
}

ScHTMLExport::~ScHTMLExport()
{
    aGraphList.clear();
}

sal_uInt16 ScHTMLExport::GetFontSizeNumber( sal_uInt16 nHeight )
{
    sal_uInt16 nSize = 1;
    for ( sal_uInt16 j=SC_HTML_FONTSIZES-1; j>0; j-- )
    {
        if( nHeight > (nFontSize[j] + nFontSize[j-1]) / 2 )
        {   // The one next to it
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
        nVal = static_cast<sal_uInt16>(pAppWin->LogicToPixel(
                    Size( nVal, nVal ), MapMode( MapUnit::MapTwip ) ).Width());
        if( !nVal ) // If there's a Twip there should also be a Pixel
            nVal = 1;
    }
    return nVal;
}

Size ScHTMLExport::MMToPixel( const Size& rSize )
{
    Size aSize( rSize );
    aSize = pAppWin->LogicToPixel( rSize, MapMode( MapUnit::Map100thMM ) );
    // If there's something there should also be a Pixel
    if ( !aSize.Width() && rSize.Width() )
        aSize.setWidth( 1 );
    if ( !aSize.Height() && rSize.Height() )
        aSize.setHeight( 1 );
    return aSize;
}

void ScHTMLExport::Write()
{
    if (!mbSkipHeaderFooter)
    {
        rStrm.WriteChar( '<' ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_doctype ).WriteChar( ' ' ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_doctype40 ).WriteChar( '>' )
           .WriteCharPtr( SAL_NEWLINE_STRING ).WriteCharPtr( SAL_NEWLINE_STRING );
        TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_html );
        WriteHeader();
        OUT_LF();
    }
    WriteBody();
    OUT_LF();
    if (!mbSkipHeaderFooter)
        TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_html );
}

void ScHTMLExport::WriteHeader()
{
    IncIndent(1); TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_head );

    if ( pDoc->IsClipOrUndo() )
    {   // no real DocInfo available, but some META information like charset needed
        SfxFrameHTMLWriter::Out_DocInfo( rStrm, aBaseURL, nullptr, sIndent, eDestEnc, &aNonConvertibleChars );
    }
    else
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDoc->GetDocumentShell()->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps
            = xDPS->getDocumentProperties();
        SfxFrameHTMLWriter::Out_DocInfo( rStrm, aBaseURL, xDocProps,
            sIndent, eDestEnc, &aNonConvertibleChars );
        OUT_LF();

        if (!xDocProps->getPrintedBy().isEmpty())
        {
            OUT_COMMENT( GLOBSTR( STR_DOC_INFO ) );
            OUString aStrOut = ( GLOBSTR( STR_DOC_PRINTED ) ) + ": ";
            lcl_AddStamp( aStrOut, xDocProps->getPrintedBy(),
                xDocProps->getPrintDate(), *ScGlobal::pLocaleData );
            OUT_COMMENT( aStrOut );
        }

    }
    OUT_LF();

    // CSS1 StyleSheet
    PageDefaults( bAll ? 0 : aRange.aStart.Tab() );
    IncIndent(1);
    rStrm.WriteCharPtr( "<" ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_style ).WriteCharPtr( " " ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_O_type ).WriteCharPtr( "=\"text/css\">" );

    OUT_LF();
    rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_HTML_body ).WriteCharPtr( "," ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_division ).WriteCharPtr( "," ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_table ).WriteCharPtr( "," )
       .WriteCharPtr( OOO_STRING_SVTOOLS_HTML_thead ).WriteCharPtr( "," ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_tbody ).WriteCharPtr( "," ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_tfoot ).WriteCharPtr( "," )
       .WriteCharPtr( OOO_STRING_SVTOOLS_HTML_tablerow ).WriteCharPtr( "," ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_tableheader ).WriteCharPtr( "," )
       .WriteCharPtr( OOO_STRING_SVTOOLS_HTML_tabledata ).WriteCharPtr( "," ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_parabreak )
       .WriteCharPtr( " { " ).WriteCharPtr( "font-family:" );
    if (!aHTMLStyle.aFontFamilyName.isEmpty())
    {
        const OUString& rList = aHTMLStyle.aFontFamilyName;
        for(sal_Int32 nPos {0};;)
        {
            rStrm.WriteChar( '\"' );
            OUT_STR( rList.getToken( 0, ';', nPos ) );
            rStrm.WriteChar( '\"' );
            if (nPos<0)
                break;
            rStrm.WriteCharPtr( ", " );
        }
    }
    rStrm.WriteCharPtr( "; " ).WriteCharPtr( "font-size:" )
       .WriteCharPtr( GetFontSizeCss( static_cast<sal_uInt16>(aHTMLStyle.nFontHeight) ) ).WriteCharPtr( " }" );

    OUT_LF();

    // write the style for the comments to make them stand out from normal cell content
    // this is done through only showing the cell contents when the custom indicator is hovered
    rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_HTML_anchor ).WriteCharPtr(".comment-indicator:hover")
       .WriteCharPtr(" + ").WriteCharPtr( OOO_STRING_SVTOOLS_HTML_comment2 ).WriteCharPtr(" { ")
       .WriteCharPtr(sBackground).WriteCharPtr("#ffd").WriteCharPtr("; ")
       .WriteCharPtr("position:").WriteCharPtr("absolute").WriteCharPtr("; ")
       .WriteCharPtr(sDisplay).WriteCharPtr("block").WriteCharPtr("; ")
       .WriteCharPtr(sBorder).WriteCharPtr("1px solid black").WriteCharPtr("; ")
       .WriteCharPtr("padding:").WriteCharPtr("0.5em").WriteCharPtr("; ")
       .WriteCharPtr(" } ");

    OUT_LF();

    rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_HTML_anchor ).WriteCharPtr(".comment-indicator")
        .WriteCharPtr(" { ")
        .WriteCharPtr(sBackground).WriteCharPtr("red").WriteCharPtr("; ")
        .WriteCharPtr(sDisplay).WriteCharPtr("inline-block").WriteCharPtr("; ")
        .WriteCharPtr(sBorder).WriteCharPtr("1px solid black").WriteCharPtr("; ")
        .WriteCharPtr("width:").WriteCharPtr("0.5em").WriteCharPtr("; ")
        .WriteCharPtr("height:").WriteCharPtr("0.5em").WriteCharPtr("; ")
        .WriteCharPtr(" } ");

    OUT_LF();

    rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_HTML_comment2  ).WriteCharPtr(" { ")
       .WriteCharPtr(sDisplay).WriteCharPtr("none").WriteCharPtr("; ")
       .WriteCharPtr(" } ");


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
        OUT_STR( ScResId( STR_OVERVIEW ) );
        TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_head1 );

        OUString aStr;

        const SCTAB nCount = pDoc->GetTableCount();
        for ( SCTAB nTab = 0; nTab < nCount; nTab++ )
        {
            if ( !IsEmptyTable( nTab ) )
            {
                pDoc->GetName( nTab, aStr );
                rStrm.WriteCharPtr( "<A HREF=\"#table" )
                   .WriteCharPtr( OString::number(nTab).getStr() )
                   .WriteCharPtr( "\">" );
                OUT_STR( aStr );
                rStrm.WriteCharPtr( "</A>" );
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
    SfxStyleSheetBase*      pStyleSheet = nullptr;
    OSL_ENSURE( pStylePool, "StylePool not found! :-(" );

    // remember defaults for compare in WriteCell
    if ( !aHTMLStyle.bInitialized )
    {
        pStylePool->SetSearchMask( SfxStyleFamily::Para );
        pStyleSheet = pStylePool->Find(
                ScResId(STR_STYLENAME_STANDARD),
                SfxStyleFamily::Para );
        OSL_ENSURE( pStyleSheet, "ParaStyle not found! :-(" );
        if (!pStyleSheet)
            pStyleSheet = pStylePool->First();
        const SfxItemSet& rSetPara = pStyleSheet->GetItemSet();

        aHTMLStyle.nDefaultScriptType = ScGlobal::GetDefaultScriptType();
        aHTMLStyle.aFontFamilyName = static_cast<const SvxFontItem&>((rSetPara.Get(
                        ScGlobal::GetScriptedWhichID(
                            aHTMLStyle.nDefaultScriptType, ATTR_FONT
                            )))).GetFamilyName();
        aHTMLStyle.nFontHeight = static_cast<const SvxFontHeightItem&>((rSetPara.Get(
                        ScGlobal::GetScriptedWhichID(
                            aHTMLStyle.nDefaultScriptType, ATTR_FONT_HEIGHT
                            )))).GetHeight();
        aHTMLStyle.nFontSizeNumber = GetFontSizeNumber( static_cast< sal_uInt16 >( aHTMLStyle.nFontHeight ) );
    }

    // Page style sheet printer settings, e.g. for background graphics.
    // There's only one background graphic in HTML!
    pStylePool->SetSearchMask( SfxStyleFamily::Page );
    pStyleSheet = pStylePool->Find( pDoc->GetPageStyle( nTab ), SfxStyleFamily::Page );
    OSL_ENSURE( pStyleSheet, "PageStyle not found! :-(" );
    if (!pStyleSheet)
        pStyleSheet = pStylePool->First();
    const SfxItemSet& rSet = pStyleSheet->GetItemSet();
    if ( !aHTMLStyle.bInitialized )
    {
        const SvxBrushItem* pBrushItem = &rSet.Get( ATTR_BACKGROUND );
        aHTMLStyle.aBackgroundColor = pBrushItem->GetColor();
        aHTMLStyle.bInitialized = true;
    }
    return rSet;
}

OString ScHTMLExport::BorderToStyle(const char* pBorderName,
        const SvxBorderLine* pLine, bool& bInsertSemicolon)
{
    OStringBuffer aOut;

    if ( pLine )
    {
        if ( bInsertSemicolon )
            aOut.append("; ");

        // which border
        aOut.append("border-").append(pBorderName).append(": ");

        // thickness
        int nWidth = pLine->GetWidth();
        int nPxWidth = (nWidth > 0) ?
            std::max(int(nWidth / TWIPS_PER_PIXEL), 1) : 0;
        aOut.append(static_cast<sal_Int32>(nPxWidth)).
            append("px ");
        switch (pLine->GetBorderLineStyle())
        {
            case SvxBorderLineStyle::SOLID:
                aOut.append("solid");
                break;
            case SvxBorderLineStyle::DOTTED:
                aOut.append("dotted");
                break;
            case SvxBorderLineStyle::DASHED:
            case SvxBorderLineStyle::DASH_DOT:
            case SvxBorderLineStyle::DASH_DOT_DOT:
                aOut.append("dashed");
                break;
            case SvxBorderLineStyle::DOUBLE:
            case SvxBorderLineStyle::DOUBLE_THIN:
            case SvxBorderLineStyle::THINTHICK_SMALLGAP:
            case SvxBorderLineStyle::THINTHICK_MEDIUMGAP:
            case SvxBorderLineStyle::THINTHICK_LARGEGAP:
            case SvxBorderLineStyle::THICKTHIN_SMALLGAP:
            case SvxBorderLineStyle::THICKTHIN_MEDIUMGAP:
            case SvxBorderLineStyle::THICKTHIN_LARGEGAP:
                aOut.append("double");
                break;
            case SvxBorderLineStyle::EMBOSSED:
                aOut.append("ridge");
                break;
            case SvxBorderLineStyle::ENGRAVED:
                aOut.append("groove");
                break;
            case SvxBorderLineStyle::OUTSET:
                aOut.append("outset");
                break;
            case SvxBorderLineStyle::INSET:
                aOut.append("inset");
                break;
            default:
                aOut.append("hidden");
        }
        aOut.append(" #");

        // color
        char hex[7];
        snprintf( hex, 7, "%06" SAL_PRIxUINT32, static_cast<sal_uInt32>( pLine->GetColor().GetRGBColor() ) );
        hex[6] = 0;

        aOut.append(hex);

        bInsertSemicolon = true;
    }

    return aOut.makeStringAndClear();
}

void ScHTMLExport::WriteBody()
{
    const SfxItemSet& rSet = PageDefaults( bAll ? 0 : aRange.aStart.Tab() );
    const SvxBrushItem* pBrushItem = &rSet.Get( ATTR_BACKGROUND );

    // default text color black
    if (!mbSkipHeaderFooter)
    {
        rStrm.WriteChar( '<' ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_body );

        if (!mbSkipImages)
        {
            if ( bAll && GPOS_NONE != pBrushItem->GetGraphicPos() )
            {
                OUString aLink = pBrushItem->GetGraphicLink();
                OUString aGrfNm;

                // Embedded graphic -> write using WriteGraphic
                if( aLink.isEmpty() )
                {
                    const Graphic* pGrf = pBrushItem->GetGraphic();
                    if( pGrf )
                    {
                        // Save graphic as (JPG) file
                        aGrfNm = aStreamPath;
                        ErrCode nErr = XOutBitmap::WriteGraphic( *pGrf, aGrfNm,
                            "JPG", XOutFlags::UseNativeIfPossible );
                        if( !nErr ) // Contains errors, as we have nothing to output
                        {
                            aGrfNm = URIHelper::SmartRel2Abs(
                                    INetURLObject(aBaseURL),
                                    aGrfNm, URIHelper::GetMaybeFileHdl());
                            aLink = aGrfNm;
                        }
                    }
                }
                else
                {
                    aGrfNm = aLink;
                    if( bCopyLocalFileToINet )
                    {
                        CopyLocalFileToINet( aGrfNm, aStreamPath );
                    }
                    else
                        aGrfNm = URIHelper::SmartRel2Abs(
                                INetURLObject(aBaseURL),
                                aGrfNm, URIHelper::GetMaybeFileHdl());
                    aLink = aGrfNm;
                }
                if( !aLink.isEmpty() )
                {
                    rStrm.WriteChar( ' ' ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_O_background ).WriteCharPtr( "=\"" );
                    OUT_STR( URIHelper::simpleNormalizedMakeRelative(
                                aBaseURL,
                                aLink ) ).WriteChar( '\"' );
                }
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

        rStrm.WriteChar( '>' ); OUT_LF();
    }

    if ( bAll )
        WriteOverview();

    WriteTables();

    if (!mbSkipHeaderFooter)
        TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_body );
}

void ScHTMLExport::WriteTables()
{
    const SCTAB nTabCount = pDoc->GetTableCount();
    const OUString  aStrTable( ScResId( SCSTR_TABLE ) );
    OUString   aStr;
    OUString        aStrOut;
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
                aStrOut = aStrTable + " "  + OUString::number( nTableStrNum++ ) + ": ";

                OUT_HR();

                // Write anchor
                rStrm.WriteCharPtr( "<A NAME=\"table" )
                   .WriteCharPtr( OString::number(nTab).getStr() )
                   .WriteCharPtr( "\">" );
                TAG_ON( OOO_STRING_SVTOOLS_HTML_head1 );
                OUT_STR( aStrOut );
                TAG_ON( OOO_STRING_SVTOOLS_HTML_emphasis );

                pDoc->GetName( nTab, aStr );
                OUT_STR( aStr );

                TAG_OFF( OOO_STRING_SVTOOLS_HTML_emphasis );
                TAG_OFF( OOO_STRING_SVTOOLS_HTML_head1 );
                rStrm.WriteCharPtr( "</A>" ); OUT_LF();
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
        OStringBuffer aByteStrOut(OOO_STRING_SVTOOLS_HTML_table);

        bTabHasGraphics = bTabAlignedLeft = false;
        if ( bAll && pDrawLayer )
            PrepareGraphics( pDrawLayer, nTab, nStartCol, nStartRow,
                nEndCol, nEndRow );

        // more <TABLE ...>
        if ( bTabAlignedLeft )
        {
            aByteStrOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).
                append("=\"").
                append(OOO_STRING_SVTOOLS_HTML_AL_left).append('"');
        }
            // ALIGN=LEFT allow text and graphics to flow around
        // CELLSPACING
        aByteStrOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_cellspacing).
            append("=\"").
            append(static_cast<sal_Int32>(nCellSpacing)).append('"');

        // BORDER=0, we do the styling of the cells in <TD>
        aByteStrOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_border).
            append("=\"0\"");
        IncIndent(1); TAG_ON_LF( aByteStrOut.makeStringAndClear().getStr() );

        // --- <COLGROUP> ----
        {
            SCCOL nCol = nStartCol;
            sal_Int32 nWidth = 0;
            sal_Int32 nSpan = 0;
            while( nCol <= nEndCol )
            {
                if( pDoc->ColHidden(nCol, nTab) )
                {
                    ++nCol;
                    continue;
                }

                if( nWidth != ToPixel( pDoc->GetColWidth( nCol, nTab ) ) )
                {
                    if( nSpan != 0 )
                    {
                        TAG_ON(lcl_getColGroupString(nSpan, nWidth).getStr());
                        TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_colgroup );
                    }
                    nWidth = ToPixel( pDoc->GetColWidth( nCol, nTab ) );
                    nSpan = 1;
                }
                else
                    nSpan++;
                nCol++;
            }
            if( nSpan )
            {
                TAG_ON(lcl_getColGroupString(nSpan, nWidth).getStr());
                TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_colgroup );
            }
        }

        // <TBODY> // Re-enable only when THEAD and TFOOT are exported
        // IncIndent(1); TAG_ON_LF( OOO_STRING_SVTOOLS_HTML_tbody );
        // At least old (3.x, 4.x?) Netscape doesn't follow <TABLE COLS=n> and
        // <COL WIDTH=x> specified, but needs a width at every column.
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
            bTableDataHeight = true;  // height at every first cell of each row
            for ( SCCOL nCol2=nStartCol; nCol2<=nEndCol; nCol2++ )
            {
                if ( pDoc->ColHidden(nCol2, nTab) )
                    continue;   // for

                if ( nCol2 == nEndCol )
                    IncIndent(-1);
                WriteCell( nCol2, nRow, nTab );
                bTableDataHeight = false;
            }

            if ( nRow == nEndRow )
                IncIndent(-1);
            TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_tablerow );
        }
        // TODO: Uncomment later
        // IncIndent(-1); TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_tbody );

        IncIndent(-1); TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_table );

        if ( bTabHasGraphics && !mbSkipImages )
        {
            // the rest that is not in a cell
            size_t ListSize = aGraphList.size();
            for ( size_t i = 0; i < ListSize; ++i )
            {
                ScHTMLGraphEntry* pE = &aGraphList[ i ];
                if ( !pE->bWritten )
                    WriteGraphEntry( pE );
            }
            aGraphList.clear();
            if ( bTabAlignedLeft )
            {
                // clear <TABLE ALIGN=LEFT> with <BR CLEAR=LEFT>
                aByteStrOut.append(OOO_STRING_SVTOOLS_HTML_linebreak);
                aByteStrOut.append(' ').
                    append(OOO_STRING_SVTOOLS_HTML_O_clear).append('=').
                    append(OOO_STRING_SVTOOLS_HTML_AL_left);
                TAG_ON_LF( aByteStrOut.makeStringAndClear().getStr() );
            }
        }

        if ( bAll )
            OUT_COMMENT( "**************************************************************************" );
    }
}

void ScHTMLExport::WriteCell( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    const ScPatternAttr* pAttr = pDoc->GetPattern( nCol, nRow, nTab );
    const SfxItemSet* pCondItemSet = pDoc->GetCondResult( nCol, nRow, nTab );

    const ScMergeFlagAttr& rMergeFlagAttr = pAttr->GetItem( ATTR_MERGE_FLAG, pCondItemSet );
    if ( rMergeFlagAttr.IsOverlapped() )
        return ;

    ScAddress aPos( nCol, nRow, nTab );
    ScHTMLGraphEntry* pGraphEntry = nullptr;
    if ( bTabHasGraphics && !mbSkipImages )
    {
        size_t ListSize = aGraphList.size();
        for ( size_t i = 0; i < ListSize; ++i )
        {
            ScHTMLGraphEntry* pE = &aGraphList[ i ];
            if ( pE->bInCell && pE->aRange.In( aPos ) )
            {
                if ( pE->aRange.aStart == aPos )
                {
                    pGraphEntry = pE;
                    break;  // for
                }
                else
                    return ; // Is a Col/RowSpan, Overlapped
            }
        }
    }

    ScRefCellValue aCell(*pDoc, aPos);

    sal_uInt32 nFormat = pAttr->GetNumberFormat( pFormatter );
    bool bValueData = aCell.hasNumeric();
    SvtScriptType nScriptType = SvtScriptType::NONE;
    if (!aCell.isEmpty())
        nScriptType = pDoc->GetScriptType(nCol, nRow, nTab);

    if ( nScriptType == SvtScriptType::NONE )
        nScriptType = aHTMLStyle.nDefaultScriptType;

    OStringBuffer aStrTD(OOO_STRING_SVTOOLS_HTML_tabledata);

    // border of the cells
    const SvxBoxItem* pBorder = pDoc->GetAttr( nCol, nRow, nTab, ATTR_BORDER );
    if ( pBorder && (pBorder->GetTop() || pBorder->GetBottom() || pBorder->GetLeft() || pBorder->GetRight()) )
    {
        aStrTD.append(' ').append(OOO_STRING_SVTOOLS_HTML_style).
            append("=\"");

        bool bInsertSemicolon = false;
        aStrTD.append(BorderToStyle("top", pBorder->GetTop(),
            bInsertSemicolon));
        aStrTD.append(BorderToStyle("bottom", pBorder->GetBottom(),
            bInsertSemicolon));
        aStrTD.append(BorderToStyle("left", pBorder->GetLeft(),
            bInsertSemicolon));
        aStrTD.append(BorderToStyle("right", pBorder->GetRight(),
            bInsertSemicolon));

        aStrTD.append('"');
    }

    const sal_Char* pChar;
    sal_uInt16 nHeightPixel;

    const ScMergeAttr& rMergeAttr = pAttr->GetItem( ATTR_MERGE, pCondItemSet );
    if ( pGraphEntry || rMergeAttr.IsMerged() )
    {
        SCCOL nC, jC;
        SCROW nR;
        sal_uLong v;
        if ( pGraphEntry )
            nC = std::max( SCCOL(pGraphEntry->aRange.aEnd.Col() - nCol + 1),
                           rMergeAttr.GetColMerge() );
        else
            nC = rMergeAttr.GetColMerge();
        if ( nC > 1 )
        {
            aStrTD.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_colspan).
                append('=').append(static_cast<sal_Int32>(nC));
            nC = nC + nCol;
            for ( jC=nCol, v=0; jC<nC; jC++ )
                v += pDoc->GetColWidth( jC, nTab );
        }

        if ( pGraphEntry )
            nR = std::max( SCROW(pGraphEntry->aRange.aEnd.Row() - nRow + 1),
                           rMergeAttr.GetRowMerge() );
        else
            nR = rMergeAttr.GetRowMerge();
        if ( nR > 1 )
        {
            aStrTD.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_rowspan).
                append('=').append(static_cast<sal_Int32>(nR));
            nR += nRow;
            v = pDoc->GetRowHeight( nRow, nR-1, nTab );
            nHeightPixel = ToPixel( static_cast< sal_uInt16 >( v ) );
        }
        else
            nHeightPixel = ToPixel( pDoc->GetRowHeight( nRow, nTab ) );
    }
    else
        nHeightPixel = ToPixel( pDoc->GetRowHeight( nRow, nTab ) );

    if ( bTableDataHeight )
    {
        aStrTD.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_height).
            append("=\"").
            append(static_cast<sal_Int32>(nHeightPixel)).append('"');
    }

    const SvxFontItem& rFontItem = static_cast<const SvxFontItem&>( pAttr->GetItem(
            ScGlobal::GetScriptedWhichID( nScriptType, ATTR_FONT),
            pCondItemSet) );

    const SvxFontHeightItem& rFontHeightItem = static_cast<const SvxFontHeightItem&>(
        pAttr->GetItem( ScGlobal::GetScriptedWhichID( nScriptType,
                    ATTR_FONT_HEIGHT), pCondItemSet) );

    const SvxWeightItem& rWeightItem = static_cast<const SvxWeightItem&>( pAttr->GetItem(
            ScGlobal::GetScriptedWhichID( nScriptType, ATTR_FONT_WEIGHT),
            pCondItemSet) );

    const SvxPostureItem& rPostureItem = static_cast<const SvxPostureItem&>(
        pAttr->GetItem( ScGlobal::GetScriptedWhichID( nScriptType,
                    ATTR_FONT_POSTURE), pCondItemSet) );

    const SvxUnderlineItem& rUnderlineItem =
        pAttr->GetItem( ATTR_FONT_UNDERLINE, pCondItemSet );

    const SvxCrossedOutItem& rCrossedOutItem =
        pAttr->GetItem( ATTR_FONT_CROSSEDOUT, pCondItemSet );

    const SvxColorItem& rColorItem = pAttr->GetItem(
            ATTR_FONT_COLOR, pCondItemSet );

    const SvxHorJustifyItem& rHorJustifyItem =
        pAttr->GetItem( ATTR_HOR_JUSTIFY, pCondItemSet );

    const SvxVerJustifyItem& rVerJustifyItem =
        pAttr->GetItem( ATTR_VER_JUSTIFY, pCondItemSet );

    const SvxBrushItem& rBrushItem = pAttr->GetItem(
            ATTR_BACKGROUND, pCondItemSet );

    Color aBgColor;
    if ( rBrushItem.GetColor().GetTransparency() == 255 )
        aBgColor = aHTMLStyle.aBackgroundColor; // No unwanted background color
    else
        aBgColor = rBrushItem.GetColor();

    bool bBold          = ( WEIGHT_BOLD      <= rWeightItem.GetWeight() );
    bool bItalic        = ( ITALIC_NONE      != rPostureItem.GetPosture() );
    bool bUnderline     = ( LINESTYLE_NONE   != rUnderlineItem.GetLineStyle() );
    bool bCrossedOut    = ( STRIKEOUT_SINGLE <= rCrossedOutItem.GetStrikeout() );
    bool bSetFontColor  = ( COL_AUTO         != rColorItem.GetValue() );  // default is AUTO now
    bool bSetFontName   = ( aHTMLStyle.aFontFamilyName  != rFontItem.GetFamilyName() );
    sal_uInt16 nSetFontSizeNumber = 0;
    sal_uInt32 nFontHeight = rFontHeightItem.GetHeight();
    if ( nFontHeight != aHTMLStyle.nFontHeight )
    {
        nSetFontSizeNumber = GetFontSizeNumber( static_cast<sal_uInt16>(nFontHeight) );
        if ( nSetFontSizeNumber == aHTMLStyle.nFontSizeNumber )
            nSetFontSizeNumber = 0;   // no difference, don't set
    }

    bool bSetFont = (bSetFontColor || bSetFontName || nSetFontSizeNumber);

    //! TODO: we could entirely use CSS1 here instead, but that would exclude
    //! Netscape 3.0 and Netscape 4.x without JavaScript enabled.
    //! Do we want that?

    switch( rHorJustifyItem.GetValue() )
    {
        case SvxCellHorJustify::Standard:
            pChar = (bValueData ? OOO_STRING_SVTOOLS_HTML_AL_right : OOO_STRING_SVTOOLS_HTML_AL_left);
            break;
        case SvxCellHorJustify::Center:    pChar = OOO_STRING_SVTOOLS_HTML_AL_center;  break;
        case SvxCellHorJustify::Block:     pChar = OOO_STRING_SVTOOLS_HTML_AL_justify; break;
        case SvxCellHorJustify::Right:     pChar = OOO_STRING_SVTOOLS_HTML_AL_right;   break;
        case SvxCellHorJustify::Left:
        case SvxCellHorJustify::Repeat:
        default:                        pChar = OOO_STRING_SVTOOLS_HTML_AL_left;    break;
    }

    aStrTD.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).
        append("=\"").append(pChar).append('"');

    switch( rVerJustifyItem.GetValue() )
    {
        case SvxCellVerJustify::Top:       pChar = OOO_STRING_SVTOOLS_HTML_VA_top;     break;
        case SvxCellVerJustify::Center:    pChar = OOO_STRING_SVTOOLS_HTML_VA_middle;  break;
        case SvxCellVerJustify::Bottom:    pChar = OOO_STRING_SVTOOLS_HTML_VA_bottom;  break;
        case SvxCellVerJustify::Standard:
        default:                        pChar = nullptr;
    }
    if ( pChar )
    {
        aStrTD.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_valign).
            append('=').append(pChar);
    }

    if ( aHTMLStyle.aBackgroundColor != aBgColor )
    {
        aStrTD.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_bgcolor).
            append('=');
        aStrTD.append(lcl_makeHTMLColorTriplet(aBgColor));
    }

    double fVal = 0.0;
    if ( bValueData )
    {
        switch (aCell.meType)
        {
            case CELLTYPE_VALUE:
                fVal = aCell.mfValue;
                if ( bCalcAsShown && fVal != 0.0 )
                    fVal = pDoc->RoundValueAsShown( fVal, nFormat );
                break;
            case CELLTYPE_FORMULA:
                fVal = aCell.mpFormula->GetValue();
                break;
            default:
                OSL_FAIL( "value data with unsupported cell type" );
        }
    }

    aStrTD.append(HTMLOutFuncs::CreateTableDataOptionsValNum(bValueData, fVal,
        nFormat, *pFormatter, eDestEnc, &aNonConvertibleChars));

    TAG_ON(aStrTD.makeStringAndClear().getStr());

    //write the note for this as the first thing in the tag
    if (pDoc->HasNote(aPos))
    {
        ScPostIt* pNote = pDoc->GetNote(aPos);

        //create the comment indicator
        OStringBuffer aStr(OOO_STRING_SVTOOLS_HTML_anchor);
        aStr.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_class)
           .append("=\"").append("comment-indicator").append("\"");
        TAG_ON(aStr.makeStringAndClear().getStr());
        TAG_OFF(OOO_STRING_SVTOOLS_HTML_anchor);
        OUT_LF();

        //create the element holding the contents
        //this is a bit naive, since it doesn't separate
        //lines into html breaklines yet
        TAG_ON(OOO_STRING_SVTOOLS_HTML_comment2);
        OUT_STR( pNote->GetText() );
        TAG_OFF(OOO_STRING_SVTOOLS_HTML_comment2);
        OUT_LF();
    }

    if ( bBold )        TAG_ON( OOO_STRING_SVTOOLS_HTML_bold );
    if ( bItalic )      TAG_ON( OOO_STRING_SVTOOLS_HTML_italic );
    if ( bUnderline )   TAG_ON( OOO_STRING_SVTOOLS_HTML_underline );
    if ( bCrossedOut )  TAG_ON( OOO_STRING_SVTOOLS_HTML_strikethrough );

    if ( bSetFont )
    {
        OStringBuffer aStr(OOO_STRING_SVTOOLS_HTML_font);
        if ( bSetFontName )
        {
            aStr.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_face).
                append("=\"");

            if (!rFontItem.GetFamilyName().isEmpty())
            {
                const OUString& rList = rFontItem.GetFamilyName();
                for (sal_Int32 nPos {0};;)
                {
                    OString aTmpStr = HTMLOutFuncs::ConvertStringToHTML(
                        rList.getToken( 0, ';', nPos ), eDestEnc,
                        &aNonConvertibleChars);
                    aStr.append(aTmpStr);
                    if (nPos<0)
                        break;
                    aStr.append(',');
                }
            }

            aStr.append('\"');
        }
        if ( nSetFontSizeNumber )
        {
            aStr.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_size).
                append('=').append(static_cast<sal_Int32>(nSetFontSizeNumber));
        }
        if ( bSetFontColor )
        {
            Color   aColor = rColorItem.GetValue();

            //  always export automatic text color as black
            if ( aColor == COL_AUTO )
                aColor = COL_BLACK;

            aStr.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_color).
                append('=').append(lcl_makeHTMLColorTriplet(aColor));
        }
        TAG_ON(aStr.makeStringAndClear().getStr());
    }

    OUString aStrOut;
    bool bFieldText = false;

    Color* pColor;
    switch (aCell.meType)
    {
        case CELLTYPE_EDIT :
            bFieldText = WriteFieldText(aCell.mpEditText);
            if ( bFieldText )
                break;
            [[fallthrough]];
        default:
            ScCellFormat::GetString(aCell, nFormat, aStrOut, &pColor, *pFormatter, pDoc);
    }

    if ( !bFieldText )
    {
        if ( aStrOut.isEmpty() )
        {
            TAG_ON( OOO_STRING_SVTOOLS_HTML_linebreak ); // No completely empty line
        }
        else
        {
            sal_Int32 nPos = aStrOut.indexOf( '\n' );
            if ( nPos == -1 )
            {
                OUT_STR( aStrOut );
            }
            else
            {
                sal_Int32 nStartPos = 0;
                do
                {
                    OUString aSingleLine = aStrOut.copy( nStartPos, nPos - nStartPos );
                    OUT_STR( aSingleLine );
                    TAG_ON( OOO_STRING_SVTOOLS_HTML_linebreak );
                    nStartPos = nPos + 1;
                }
                while( ( nPos = aStrOut.indexOf( '\n', nStartPos ) ) != -1 );
                OUString aSingleLine = aStrOut.copy( nStartPos );
                OUT_STR( aSingleLine );
            }
        }
    }
    if ( pGraphEntry )
        WriteGraphEntry( pGraphEntry );

    if ( bSetFont )     TAG_OFF( OOO_STRING_SVTOOLS_HTML_font );
    if ( bCrossedOut )  TAG_OFF( OOO_STRING_SVTOOLS_HTML_strikethrough );
    if ( bUnderline )   TAG_OFF( OOO_STRING_SVTOOLS_HTML_underline );
    if ( bItalic )      TAG_OFF( OOO_STRING_SVTOOLS_HTML_italic );
    if ( bBold )        TAG_OFF( OOO_STRING_SVTOOLS_HTML_bold );

    TAG_OFF_LF( OOO_STRING_SVTOOLS_HTML_tabledata );
}

bool ScHTMLExport::WriteFieldText( const EditTextObject* pData )
{
    bool bFields = false;
    // text and anchor of URL fields, Doc-Engine is a ScFieldEditEngine
    EditEngine& rEngine = pDoc->GetEditEngine();
    rEngine.SetText( *pData );
    sal_Int32 nParas = rEngine.GetParagraphCount();
    if ( nParas )
    {
        ESelection aSel( 0, 0, nParas-1, rEngine.GetTextLen( nParas-1 ) );
        SfxItemSet aSet( rEngine.GetAttribs( aSel ) );
        SfxItemState eFieldState = aSet.GetItemState( EE_FEATURE_FIELD, false );
        if ( eFieldState == SfxItemState::DONTCARE || eFieldState == SfxItemState::SET )
            bFields = true;
    }
    if ( bFields )
    {
        bool bOldUpdateMode = rEngine.GetUpdateMode();
        rEngine.SetUpdateMode( true );      // no portions if not formatted
        for ( sal_Int32 nPar=0; nPar < nParas; nPar++ )
        {
            if ( nPar > 0 )
                TAG_ON( OOO_STRING_SVTOOLS_HTML_linebreak );
            std::vector<sal_Int32> aPortions;
            rEngine.GetPortions( nPar, aPortions );
            sal_Int32 nStart = 0;
            for ( const sal_Int32 nEnd : aPortions )
            {
                ESelection aSel( nPar, nStart, nPar, nEnd );
                bool bUrl = false;
                // fields are single characters
                if ( nEnd == nStart+1 )
                {
                    const SfxPoolItem* pItem;
                    SfxItemSet aSet = rEngine.GetAttribs( aSel );
                    if ( aSet.GetItemState( EE_FEATURE_FIELD, false, &pItem ) == SfxItemState::SET )
                    {
                        const SvxFieldData* pField = static_cast<const SvxFieldItem*>(pItem)->GetField();
                        if (const SvxURLField* pURLField = dynamic_cast<const SvxURLField*>(pField))
                        {
                            bUrl = true;
                            rStrm.WriteChar( '<' ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_anchor ).WriteChar( ' ' ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_O_href ).WriteCharPtr( "=\"" );
                            OUT_STR( pURLField->GetURL() );
                            rStrm.WriteCharPtr( "\">" );
                            OUT_STR( pURLField->GetRepresentation() );
                            rStrm.WriteCharPtr( "</" ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_anchor ).WriteChar( '>' );
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

void ScHTMLExport::CopyLocalFileToINet( OUString& rFileNm,
        const OUString& rTargetNm )
{
    INetURLObject aFileUrl, aTargetUrl;
    aFileUrl.SetSmartURL( rFileNm );
    aTargetUrl.SetSmartURL( rTargetNm );
    if( INetProtocol::File == aFileUrl.GetProtocol() &&
        ( INetProtocol::File != aTargetUrl.GetProtocol() &&
          INetProtocol::Ftp <= aTargetUrl.GetProtocol() &&
          INetProtocol::Javascript >= aTargetUrl.GetProtocol())  )
    {
        if( pFileNameMap )
        {
            // Did we already move the file?
            std::map<OUString, OUString>::iterator it = pFileNameMap->find( rFileNm );
            if( it != pFileNameMap->end() )
            {
                rFileNm = it->second;
                return;
            }
        }
        else
        {
            pFileNameMap.reset( new std::map<OUString, OUString> );
        }

        bool bRet = false;
        SvFileStream aTmp( aFileUrl.PathToFileName(), StreamMode::READ );

        OUString aSrc = rFileNm;
        OUString aDest = aTargetUrl.GetPartBeforeLastName();
        aDest += aFileUrl.GetName();

        SfxMedium aMedium( aDest, StreamMode::WRITE | StreamMode::SHARE_DENYNONE );

        {
            SvFileStream aCpy( aMedium.GetPhysicalName(), StreamMode::WRITE );
            aCpy.WriteStream( aTmp );
        }

        // Take over
        aMedium.Close();
        aMedium.Commit();

        bRet = ERRCODE_NONE == aMedium.GetError();

        if( bRet )
        {
            pFileNameMap->insert( std::make_pair( aSrc, aDest ) );
            rFileNm = aDest;
        }
    }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
