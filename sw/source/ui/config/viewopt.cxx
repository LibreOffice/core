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

#include <svx/htmlmode.hxx>
#include <svtools/htmlcfg.hxx>

#include <svx/svxids.hrc>
#include <editeng/svxenum.hxx>
#include <editeng/svxacorr.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/region.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <swmodule.hxx>
#include <swtypes.hxx>
#include <viewopt.hxx>
#include <wdocsh.hxx>
#include <swrect.hxx>
#include <crstate.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <unotools/syslocale.hxx>

#include <editeng/acorrcfg.hxx>

#ifdef DBG_UTIL
bool SwViewOption::s_bTest9 = false;        //DrawingLayerNotLoading
#endif
Color SwViewOption::aDocBoundColor(COL_LIGHTGRAY);
Color SwViewOption::aObjectBoundColor(COL_LIGHTGRAY);
Color SwViewOption::aDocColor(COL_LIGHTGRAY);
Color SwViewOption::aAppBackgroundColor(COL_LIGHTGRAY);
Color SwViewOption::aTableBoundColor(COL_LIGHTGRAY);
Color SwViewOption::aIndexShadingsColor(COL_LIGHTGRAY);
Color SwViewOption::aLinksColor(COL_BLUE);
Color SwViewOption::aVisitedLinksColor(COL_RED);
Color SwViewOption::aDirectCursorColor(COL_BLUE);
Color SwViewOption::aTextGridColor(COL_LIGHTGRAY);
Color SwViewOption::aSpellColor(COL_LIGHTRED);
Color SwViewOption::aSmarttagColor(COL_LIGHTMAGENTA);
Color SwViewOption::aFontColor(COL_BLACK);
Color SwViewOption::aFieldShadingsColor(COL_LIGHTGRAY);
Color SwViewOption::aSectionBoundColor(COL_LIGHTGRAY);
Color SwViewOption::aPageBreakColor(COL_BLUE);
Color SwViewOption::aScriptIndicatorColor(COL_GREEN);
Color SwViewOption::aShadowColor(COL_GRAY);
Color SwViewOption::aHeaderFooterMarkColor(COL_BLUE);

sal_Int32 SwViewOption::nAppearanceFlags = VIEWOPT_DOC_BOUNDARIES|VIEWOPT_OBJECT_BOUNDARIES;
sal_uInt16 SwViewOption::nPixelTwips = 0;   // one pixel on the screen

static const char aPostItStr[] = "  ";

sal_Bool SwViewOption::IsEqualFlags( const SwViewOption &rOpt ) const
{
    return  nCoreOptions == rOpt.nCoreOptions
            && nCore2Options == rOpt.nCore2Options
            && aSnapSize    == rOpt.aSnapSize
            && mnViewLayoutColumns == rOpt.mnViewLayoutColumns
            && nDivisionX   == rOpt.GetDivisionX()
            && nDivisionY   == rOpt.GetDivisionY()
            && nPagePrevRow == rOpt.GetPagePrevRow()
            && nPagePrevCol == rOpt.GetPagePrevCol()
            && aRetoucheColor == rOpt.GetRetoucheColor()
            && mbFormView == rOpt.IsFormView()
            && mbBrowseMode == rOpt.getBrowseMode()
            && mbViewLayoutBookMode == rOpt.mbViewLayoutBookMode
            && bShowPlaceHolderFields == rOpt.bShowPlaceHolderFields
            && bIdle == rOpt.bIdle
#ifdef DBG_UTIL
            // correspond to the statements in ui/config/cfgvw.src
            && m_bTest1 == rOpt.IsTest1()
            && m_bTest2 == rOpt.IsTest2()
            && m_bTest3 == rOpt.IsTest3()
            && m_bTest4 == rOpt.IsTest4()
            && m_bTest5 == rOpt.IsTest5()
            && m_bTest6 == rOpt.IsTest6()
            && m_bTest7 == rOpt.IsTest7()
            && m_bTest8 == rOpt.IsTest8()
            && m_bTest10 == rOpt.IsTest10()
#endif
            ;
}

void SwViewOption::DrawRect( OutputDevice *pOut,
                             const SwRect &rRect, long nCol ) const
{
    if ( pOut->GetOutDevType() != OUTDEV_PRINTER )
    {
        const Color aCol( nCol );
        const Color aOldColor( pOut->GetFillColor() );
        pOut->SetFillColor( aCol );
        pOut->DrawRect( rRect.SVRect() );
        pOut->SetFillColor( aOldColor );
    }
    else
        DrawRectPrinter( pOut, rRect );
}

void SwViewOption::DrawRectPrinter( OutputDevice *pOut,
                                    const SwRect &rRect ) const
{
    Color aOldColor(pOut->GetLineColor());
    Color aOldFillColor( pOut->GetFillColor() );
    pOut->SetLineColor( Color(COL_BLACK) );
    pOut->SetFillColor( Color(COL_TRANSPARENT ));
    pOut->DrawRect( rRect.SVRect() );
    pOut->SetFillColor( aOldFillColor );
    pOut->SetLineColor( aOldColor );
}

sal_uInt16 SwViewOption::GetPostItsWidth( const OutputDevice *pOut ) const
{
    OSL_ENSURE( pOut, "no Outdev" );
    return sal_uInt16(pOut->GetTextWidth( rtl::OUString(aPostItStr )));
}

void SwViewOption::PaintPostIts( OutputDevice *pOut, const SwRect &rRect, sal_Bool bIsScript ) const
{
    if( pOut && bIsScript )
    {
            Color aOldLineColor( pOut->GetLineColor() );
        pOut->SetLineColor( Color(COL_GRAY ) );
        // to make it look nice, we subtract two pixels everywhere
        sal_uInt16 nPix = GetPixelTwips() * 2;
        if( rRect.Width() <= 2 * nPix || rRect.Height() <= 2 * nPix )
            nPix = 0;
        const Point aTopLeft(  rRect.Left()  + nPix, rRect.Top()    + nPix );
        const Point aBotRight( rRect.Right() - nPix, rRect.Bottom() - nPix );
        const SwRect aRect( aTopLeft, aBotRight );
        DrawRect( pOut, aRect, aScriptIndicatorColor.GetColor() );
    pOut->SetLineColor( aOldLineColor );
    }
}

SwViewOption::SwViewOption() :
    sSymbolFont( RTL_CONSTASCII_USTRINGPARAM( "symbol" ) ),
    aRetoucheColor( COL_TRANSPARENT ),
    mnViewLayoutColumns( 0 ),
    nPagePrevRow( 1 ),
    nPagePrevCol( 2 ),
    nShdwCrsrFillMode( FILL_TAB ),
    bReadonly(sal_False),
    bStarOneSetting(sal_False),
    bIsPagePreview(sal_False),
    bSelectionInReadonly(sal_False),
    mbFormView(sal_False),
    mbBrowseMode(sal_False),
    mbBookView(sal_False),
    mbViewLayoutBookMode(sal_False),
    bShowPlaceHolderFields( sal_True ),
    nZoom( 100 ),
    eZoom( SVX_ZOOM_PERCENT ),
    nTblDest(TBL_DEST_CELL)
{
    // Initialisation is a little simpler now
    // all Bits to 0
    nCoreOptions =  VIEWOPT_1_HARDBLANK | VIEWOPT_1_SOFTHYPH |
                    VIEWOPT_1_REF |
                    VIEWOPT_1_GRAPHIC |
                    VIEWOPT_1_TABLE    | VIEWOPT_1_DRAW | VIEWOPT_1_CONTROL |
                    VIEWOPT_1_PAGEBACK |
                    VIEWOPT_1_POSTITS;
    nCore2Options = VIEWOPT_CORE2_BLACKFONT | VIEWOPT_CORE2_HIDDENPARA;
    nUIOptions    = VIEWOPT_2_MODIFIED | VIEWOPT_2_GRFKEEPZOOM |VIEWOPT_2_ANY_RULER;

    if(MEASURE_METRIC != SvtSysLocale().GetLocaleData().getMeasurementSystemEnum())
        aSnapSize.Width() = aSnapSize.Height() = 720;   // 1/2"
    else
        aSnapSize.Width() = aSnapSize.Height() = 567;   // 1 cm
    nDivisionX = nDivisionY = 1;

    bSelectionInReadonly = SW_MOD()->GetAccessibilityOptions().IsSelectionInReadonly();

    bIdle = true;

#ifdef DBG_UTIL
    // correspond to the statements in ui/config/cfgvw.src
    m_bTest1 = m_bTest2 = m_bTest3 = m_bTest4 =
             m_bTest5 = m_bTest6 = m_bTest7 = m_bTest8 = m_bTest10 = false;
#endif
}

SwViewOption::SwViewOption(const SwViewOption& rVOpt)
{
    bReadonly = sal_False;
    bSelectionInReadonly = sal_False;
    // #114856# Formular view
    mbFormView       = rVOpt.mbFormView;
    nZoom           = rVOpt.nZoom       ;
    aSnapSize       = rVOpt.aSnapSize   ;
    mnViewLayoutColumns = rVOpt.mnViewLayoutColumns ;
    nDivisionX      = rVOpt.nDivisionX  ;
    nDivisionY      = rVOpt.nDivisionY  ;
    nPagePrevRow    = rVOpt.nPagePrevRow;
    nPagePrevCol    = rVOpt.nPagePrevCol;
    bIsPagePreview  = rVOpt.bIsPagePreview;
    eZoom           = rVOpt.eZoom       ;
    nTblDest        = rVOpt.nTblDest    ;
    nUIOptions      = rVOpt.nUIOptions  ;
    nCoreOptions    = rVOpt.nCoreOptions  ;
    nCore2Options   = rVOpt.nCore2Options  ;
    aRetoucheColor  = rVOpt.GetRetoucheColor();
    sSymbolFont     = rVOpt.sSymbolFont;
    nShdwCrsrFillMode = rVOpt.nShdwCrsrFillMode;
    bStarOneSetting = rVOpt.bStarOneSetting;
    mbBookView      = rVOpt.mbBookView;
    mbBrowseMode    = rVOpt.mbBrowseMode;
    mbViewLayoutBookMode = rVOpt.mbViewLayoutBookMode;
    bShowPlaceHolderFields = rVOpt.bShowPlaceHolderFields;
    bIdle           = rVOpt.bIdle;

#ifdef DBG_UTIL
    m_bTest1  = rVOpt.m_bTest1;
    m_bTest2  = rVOpt.m_bTest2;
    m_bTest3  = rVOpt.m_bTest3;
    m_bTest4  = rVOpt.m_bTest4;
    m_bTest5  = rVOpt.m_bTest5;
    m_bTest6  = rVOpt.m_bTest6;
    m_bTest7  = rVOpt.m_bTest7;
    m_bTest8  = rVOpt.m_bTest8;
    m_bTest10 = rVOpt.m_bTest10;
#endif
}


SwViewOption& SwViewOption::operator=( const SwViewOption &rVOpt )
{
    // #114856# Formular view
    mbFormView       = rVOpt.mbFormView   ;
    nZoom           = rVOpt.nZoom       ;
    aSnapSize       = rVOpt.aSnapSize   ;
    mnViewLayoutColumns = rVOpt.mnViewLayoutColumns ;
    nDivisionX      = rVOpt.nDivisionX  ;
    nDivisionY      = rVOpt.nDivisionY  ;
    nPagePrevRow    = rVOpt.nPagePrevRow;
    nPagePrevCol    = rVOpt.nPagePrevCol;
    bIsPagePreview  = rVOpt.bIsPagePreview;
    eZoom           = rVOpt.eZoom       ;
    nTblDest        = rVOpt.nTblDest    ;
    nUIOptions      = rVOpt.nUIOptions  ;
    nCoreOptions    = rVOpt.nCoreOptions;
    nCore2Options   = rVOpt.nCore2Options;
    aRetoucheColor  = rVOpt.GetRetoucheColor();
    sSymbolFont     = rVOpt.sSymbolFont;
    nShdwCrsrFillMode = rVOpt.nShdwCrsrFillMode;
    bStarOneSetting = rVOpt.bStarOneSetting;
    mbBookView      = rVOpt.mbBookView;
    mbBrowseMode    = rVOpt.mbBrowseMode;
    mbViewLayoutBookMode = rVOpt.mbViewLayoutBookMode;
    bShowPlaceHolderFields = rVOpt.bShowPlaceHolderFields;
    bIdle           = rVOpt.bIdle;

#ifdef DBG_UTIL
    m_bTest1  = rVOpt.m_bTest1;
    m_bTest2  = rVOpt.m_bTest2;
    m_bTest3  = rVOpt.m_bTest3;
    m_bTest4  = rVOpt.m_bTest4;
    m_bTest5  = rVOpt.m_bTest5;
    m_bTest6  = rVOpt.m_bTest6;
    m_bTest7  = rVOpt.m_bTest7;
    m_bTest8  = rVOpt.m_bTest8;
    m_bTest10 = rVOpt.m_bTest10;
#endif
    return *this;
}


SwViewOption::~SwViewOption()
{
}

void SwViewOption::Init( Window *pWin )
{
    if( !nPixelTwips && pWin )
    {
        nPixelTwips = (sal_uInt16)pWin->PixelToLogic( Size(1,1) ).Height();
    }
}

sal_Bool SwViewOption::IsAutoCompleteWords() const
{
    const SvxSwAutoFmtFlags& rFlags = SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags();
    return rFlags.bAutoCmpltCollectWords;
}

AuthorCharAttr::AuthorCharAttr() :
    nItemId (SID_ATTR_CHAR_UNDERLINE),
    nAttr   (UNDERLINE_SINGLE),
    nColor  (COL_TRANSPARENT)
{
}

sal_uInt16      GetHtmlMode(const SwDocShell* pShell)
{
    sal_uInt16 nRet = 0;
    if(!pShell || PTR_CAST(SwWebDocShell, pShell))
    {
        nRet = HTMLMODE_ON | HTMLMODE_SOME_STYLES;
        SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
        switch ( rHtmlOpt.GetExportMode() )
        {
            case HTML_CFG_MSIE:
                nRet |= HTMLMODE_FULL_STYLES;
            break;
            case HTML_CFG_NS40:
                nRet |= HTMLMODE_FRM_COLUMNS;
            break;
            case HTML_CFG_WRITER:
                nRet |= HTMLMODE_FRM_COLUMNS|HTMLMODE_FULL_STYLES;
            break;
        }
    }
    return nRet;
}

Color&   SwViewOption::GetDocColor()
{
    return aDocColor;
}

Color&   SwViewOption::GetDocBoundariesColor()
{
    return aDocBoundColor;
}

Color&   SwViewOption::GetObjectBoundariesColor()
{
    return aObjectBoundColor;
}

Color& SwViewOption::GetAppBackgroundColor()
{
    return aAppBackgroundColor;
}

Color&   SwViewOption::GetTableBoundariesColor()
{
    return aTableBoundColor;
}

Color&   SwViewOption::GetIndexShadingsColor()
{
    return aIndexShadingsColor;
}

Color&   SwViewOption::GetLinksColor()
{
    return aLinksColor;
}

Color&   SwViewOption::GetVisitedLinksColor()
{
    return aVisitedLinksColor;
}

Color&   SwViewOption::GetDirectCursorColor()
{
    return aDirectCursorColor;
}

Color&   SwViewOption::GetTextGridColor()
{
    return aTextGridColor;
}

Color&   SwViewOption::GetSpellColor()
{
    return aSpellColor;
}

Color&   SwViewOption::GetSmarttagColor()
{
    return aSmarttagColor;
}

Color&   SwViewOption::GetShadowColor()
{
    return aShadowColor;
}

Color&   SwViewOption::GetFontColor()
{
    return aFontColor;
}

Color&   SwViewOption::GetFieldShadingsColor()
{
    return aFieldShadingsColor;
}

Color&   SwViewOption::GetSectionBoundColor()
{
    return aSectionBoundColor;
}

Color& SwViewOption::GetPageBreakColor()
{
    return aPageBreakColor;
}

Color& SwViewOption::GetHeaderFooterMarkColor()
{
    return aHeaderFooterMarkColor;
}

void SwViewOption::ApplyColorConfigValues(const svtools::ColorConfig& rConfig )
{
    aDocColor.SetColor(rConfig.GetColorValue(svtools::DOCCOLOR).nColor);

    svtools::ColorConfigValue aValue = rConfig.GetColorValue(svtools::DOCBOUNDARIES);
    aDocBoundColor.SetColor(aValue.nColor);
    nAppearanceFlags = 0;
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_DOC_BOUNDARIES;

    aAppBackgroundColor.SetColor(rConfig.GetColorValue(svtools::APPBACKGROUND).nColor);

    aValue = rConfig.GetColorValue(svtools::OBJECTBOUNDARIES);
    aObjectBoundColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_OBJECT_BOUNDARIES;

    aValue = rConfig.GetColorValue(svtools::TABLEBOUNDARIES);
    aTableBoundColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_TABLE_BOUNDARIES;

    aValue = rConfig.GetColorValue(svtools::WRITERIDXSHADINGS);
    aIndexShadingsColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_INDEX_SHADINGS;

    aValue = rConfig.GetColorValue(svtools::LINKS);
    aLinksColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_LINKS;

    aValue = rConfig.GetColorValue(svtools::LINKSVISITED);
    aVisitedLinksColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_VISITED_LINKS;

    aValue = rConfig.GetColorValue(svtools::SHADOWCOLOR);
    aShadowColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_SHADOW;

    aDirectCursorColor.SetColor(rConfig.GetColorValue(svtools::WRITERDIRECTCURSOR).nColor);

    aTextGridColor.SetColor(rConfig.GetColorValue(svtools::WRITERTEXTGRID).nColor);

    aSpellColor.SetColor(rConfig.GetColorValue(svtools::SPELL).nColor);

    aSmarttagColor.SetColor(rConfig.GetColorValue(svtools::SMARTTAGS).nColor);

    aFontColor.SetColor(rConfig.GetColorValue(svtools::FONTCOLOR).nColor);

    aValue = rConfig.GetColorValue(svtools::WRITERFIELDSHADINGS);
    aFieldShadingsColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_FIELD_SHADINGS;

    aValue = rConfig.GetColorValue(svtools::WRITERSECTIONBOUNDARIES);
    aSectionBoundColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_SECTION_BOUNDARIES;

    aValue = rConfig.GetColorValue(svtools::WRITERPAGEBREAKS);
    aPageBreakColor.SetColor(aValue.nColor);

    aValue = rConfig.GetColorValue(svtools::WRITERHEADERFOOTERMARK);
    aHeaderFooterMarkColor.SetColor(aValue.nColor);

    aScriptIndicatorColor.SetColor(rConfig.GetColorValue(svtools::WRITERSCRIPTINDICATOR).nColor);
}

void SwViewOption::SetAppearanceFlag(sal_Int32 nFlag, sal_Bool bSet, sal_Bool bSaveInConfig )
{
    if(bSet)
        nAppearanceFlags |= nFlag;
    else
        nAppearanceFlags &= ~nFlag;
    if(bSaveInConfig)
    {
        //create an editable svtools::ColorConfig and store the change
        svtools::EditableColorConfig aEditableConfig;
        struct FlagToConfig_Impl
        {
            sal_Int32               nFlag;
            svtools::ColorConfigEntry   eEntry;
        };
        static const FlagToConfig_Impl aFlags[] =
        {
            { VIEWOPT_DOC_BOUNDARIES     ,   svtools::DOCBOUNDARIES },
            { VIEWOPT_OBJECT_BOUNDARIES  ,   svtools::OBJECTBOUNDARIES },
            { VIEWOPT_TABLE_BOUNDARIES   ,   svtools::TABLEBOUNDARIES },
            { VIEWOPT_INDEX_SHADINGS     ,   svtools::WRITERIDXSHADINGS },
            { VIEWOPT_LINKS              ,   svtools::LINKS },
            { VIEWOPT_VISITED_LINKS      ,   svtools::LINKSVISITED },
            { VIEWOPT_FIELD_SHADINGS     ,   svtools::WRITERFIELDSHADINGS },
            { VIEWOPT_SECTION_BOUNDARIES ,   svtools::WRITERSECTIONBOUNDARIES },
            { VIEWOPT_SHADOW             ,   svtools::SHADOWCOLOR },
            { 0                          ,   svtools::ColorConfigEntryCount }
        };
        sal_uInt16 nPos = 0;
        while(aFlags[nPos].nFlag)
        {
            if(0 != (nFlag&aFlags[nPos].nFlag))
            {
                svtools::ColorConfigValue aValue = aEditableConfig.GetColorValue(aFlags[nPos].eEntry);
                aValue.bIsVisible = bSet;
                aEditableConfig.SetColorValue(aFlags[nPos].eEntry, aValue);
            }
            nPos++;
        }
    }
}

sal_Bool SwViewOption::IsAppearanceFlag(sal_Int32 nFlag)
{
    return 0 != (nAppearanceFlags & nFlag);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
