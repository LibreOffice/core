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

#include <sfx2/htmlmode.hxx>
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
#include <unotools/configmgr.hxx>
#include <unotools/syslocale.hxx>

#include <editeng/acorrcfg.hxx>
#include <comphelper/lok.hxx>

#ifdef DBG_UTIL
bool SwViewOption::m_bTest9 = false;        //DrawingLayerNotLoading
#endif
Color SwViewOption::m_aDocBoundColor(COL_LIGHTGRAY);
Color SwViewOption::m_aObjectBoundColor(COL_LIGHTGRAY);
Color SwViewOption::m_aDocColor(COL_LIGHTGRAY);
Color SwViewOption::m_aAppBackgroundColor(COL_LIGHTGRAY);
Color SwViewOption::m_aTableBoundColor(COL_LIGHTGRAY);
Color SwViewOption::m_aIndexShadingsColor(COL_LIGHTGRAY);
Color SwViewOption::m_aLinksColor(COL_BLUE);
Color SwViewOption::m_aVisitedLinksColor(COL_RED);
Color SwViewOption::m_aDirectCursorColor(COL_BLUE);
Color SwViewOption::m_aTextGridColor(COL_LIGHTGRAY);
Color SwViewOption::m_aSpellColor(COL_LIGHTRED);
Color SwViewOption::m_aSmarttagColor(COL_LIGHTMAGENTA);
Color SwViewOption::m_aFontColor(COL_BLACK);
Color SwViewOption::m_aFieldShadingsColor(COL_LIGHTGRAY);
Color SwViewOption::m_aSectionBoundColor(COL_LIGHTGRAY);
Color SwViewOption::m_aPageBreakColor(COL_BLUE);
Color SwViewOption::m_aScriptIndicatorColor(COL_GREEN);
Color SwViewOption::m_aShadowColor(COL_GRAY);
Color SwViewOption::m_aHeaderFooterMarkColor(COL_BLUE);

sal_Int32 SwViewOption::m_nAppearanceFlags = VIEWOPT_DOC_BOUNDARIES|VIEWOPT_OBJECT_BOUNDARIES;
sal_uInt16 SwViewOption::m_nPixelTwips = 0;   // one pixel on the screen

static const char aPostItStr[] = "  ";

bool SwViewOption::IsEqualFlags( const SwViewOption &rOpt ) const
{
    return  m_nCoreOptions == rOpt.m_nCoreOptions
            && m_nCore2Options == rOpt.m_nCore2Options
            && m_aSnapSize    == rOpt.m_aSnapSize
            && mnViewLayoutColumns == rOpt.mnViewLayoutColumns
            && m_nDivisionX   == rOpt.GetDivisionX()
            && m_nDivisionY   == rOpt.GetDivisionY()
            && m_nPagePreviewRow == rOpt.GetPagePrevRow()
            && m_nPagePreviewCol == rOpt.GetPagePrevCol()
            && m_aRetouchColor == rOpt.GetRetoucheColor()
            && mbFormView == rOpt.IsFormView()
            && mbBrowseMode == rOpt.getBrowseMode()
            && mbViewLayoutBookMode == rOpt.mbViewLayoutBookMode
            && mbHideWhitespaceMode == rOpt.mbHideWhitespaceMode
            && m_bShowPlaceHolderFields == rOpt.m_bShowPlaceHolderFields
            && m_bIdle == rOpt.m_bIdle
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
                             const SwRect &rRect, long nCol )
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
                                    const SwRect &rRect )
{
    Color aOldColor(pOut->GetLineColor());
    Color aOldFillColor( pOut->GetFillColor() );
    pOut->SetLineColor( Color(COL_BLACK) );
    pOut->SetFillColor( Color(COL_TRANSPARENT ));
    pOut->DrawRect( rRect.SVRect() );
    pOut->SetFillColor( aOldFillColor );
    pOut->SetLineColor( aOldColor );
}

sal_uInt16 SwViewOption::GetPostItsWidth( const OutputDevice *pOut )
{
    assert(pOut && "no Outdev");
    return sal_uInt16(pOut->GetTextWidth( aPostItStr));
}

void SwViewOption::PaintPostIts( OutputDevice *pOut, const SwRect &rRect, bool bIsScript )
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
        DrawRect( pOut, aRect, m_aScriptIndicatorColor.GetColor() );
    pOut->SetLineColor( aOldLineColor );
    }
}

SwViewOption::SwViewOption() :
    m_sSymbolFont( "symbol" ),
    m_aRetouchColor( COL_TRANSPARENT ),
    mnViewLayoutColumns( 0 ),
    m_nPagePreviewRow( 1 ),
    m_nPagePreviewCol( 2 ),
    m_nShadowCrsrFillMode( FILL_TAB ),
    m_bReadonly(false),
    m_bStarOneSetting(false),
    m_bIsPagePreview(false),
    m_bSelectionInReadonly(false),
    mbFormView(false),
    mbBrowseMode(false),
    mbBookView(false),
    mbViewLayoutBookMode(false),
    mbHideWhitespaceMode(false),
    m_bShowPlaceHolderFields( true ),
    m_nZoom( 100 ),
    m_eZoom( SvxZoomType::PERCENT ),
    m_nTableDestination(TBL_DEST_CELL)
{
    // Initialisation is a little simpler now
    // all Bits to 0
    m_nCoreOptions =
        VIEWOPT_1_HARDBLANK |
        VIEWOPT_1_SOFTHYPH |
        VIEWOPT_1_REF |
        VIEWOPT_1_GRAPHIC |
        VIEWOPT_1_TABLE |
        VIEWOPT_1_DRAW |
        VIEWOPT_1_CONTROL |
        VIEWOPT_1_PAGEBACK |
        VIEWOPT_1_POSTITS;

    m_nCore2Options =
        VIEWOPT_CORE2_BLACKFONT |
        VIEWOPT_CORE2_HIDDENPARA;

    m_nUIOptions =
        VIEWOPT_2_MODIFIED |
        VIEWOPT_2_GRFKEEPZOOM |
        VIEWOPT_2_ANY_RULER;

    if (!utl::ConfigManager::IsAvoidConfig() && MEASURE_METRIC != SvtSysLocale().GetLocaleData().getMeasurementSystemEnum())
        m_aSnapSize.Width() = m_aSnapSize.Height() = 720;   // 1/2"
    else
        m_aSnapSize.Width() = m_aSnapSize.Height() = 567;   // 1 cm
    m_nDivisionX = m_nDivisionY = 1;

    m_bSelectionInReadonly = !utl::ConfigManager::IsAvoidConfig() && SW_MOD()->GetAccessibilityOptions().IsSelectionInReadonly();

    m_bIdle = true;

#ifdef DBG_UTIL
    // correspond to the statements in ui/config/cfgvw.src
    m_bTest1 = m_bTest2 = m_bTest3 = m_bTest4 =
             m_bTest5 = m_bTest6 = m_bTest7 = m_bTest8 = m_bTest10 = false;
#endif
    if (comphelper::LibreOfficeKit::isActive())
        m_aAppBackgroundColor = COL_TRANSPARENT;
}

SwViewOption::SwViewOption(const SwViewOption& rVOpt)
{
    m_bReadonly = false;
    m_bSelectionInReadonly = false;
    // #114856# Formular view
    mbFormView       = rVOpt.mbFormView;
    m_nZoom           = rVOpt.m_nZoom       ;
    m_aSnapSize       = rVOpt.m_aSnapSize   ;
    mnViewLayoutColumns = rVOpt.mnViewLayoutColumns ;
    m_nDivisionX      = rVOpt.m_nDivisionX  ;
    m_nDivisionY      = rVOpt.m_nDivisionY  ;
    m_nPagePreviewRow    = rVOpt.m_nPagePreviewRow;
    m_nPagePreviewCol    = rVOpt.m_nPagePreviewCol;
    m_bIsPagePreview  = rVOpt.m_bIsPagePreview;
    m_eZoom           = rVOpt.m_eZoom       ;
    m_nTableDestination        = rVOpt.m_nTableDestination    ;
    m_nUIOptions      = rVOpt.m_nUIOptions  ;
    m_nCoreOptions    = rVOpt.m_nCoreOptions  ;
    m_nCore2Options   = rVOpt.m_nCore2Options  ;
    m_aRetouchColor  = rVOpt.GetRetoucheColor();
    m_sSymbolFont     = rVOpt.m_sSymbolFont;
    m_nShadowCrsrFillMode = rVOpt.m_nShadowCrsrFillMode;
    m_bStarOneSetting = rVOpt.m_bStarOneSetting;
    mbBookView      = rVOpt.mbBookView;
    mbBrowseMode    = rVOpt.mbBrowseMode;
    mbViewLayoutBookMode = rVOpt.mbViewLayoutBookMode;
    mbHideWhitespaceMode = rVOpt.mbHideWhitespaceMode;
    m_bShowPlaceHolderFields = rVOpt.m_bShowPlaceHolderFields;
    m_bIdle           = rVOpt.m_bIdle;

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
    m_nZoom           = rVOpt.m_nZoom       ;
    m_aSnapSize       = rVOpt.m_aSnapSize   ;
    mnViewLayoutColumns = rVOpt.mnViewLayoutColumns ;
    m_nDivisionX      = rVOpt.m_nDivisionX  ;
    m_nDivisionY      = rVOpt.m_nDivisionY  ;
    m_nPagePreviewRow    = rVOpt.m_nPagePreviewRow;
    m_nPagePreviewCol    = rVOpt.m_nPagePreviewCol;
    m_bIsPagePreview  = rVOpt.m_bIsPagePreview;
    m_eZoom           = rVOpt.m_eZoom       ;
    m_nTableDestination        = rVOpt.m_nTableDestination    ;
    m_nUIOptions      = rVOpt.m_nUIOptions  ;
    m_nCoreOptions    = rVOpt.m_nCoreOptions;
    m_nCore2Options   = rVOpt.m_nCore2Options;
    m_aRetouchColor  = rVOpt.GetRetoucheColor();
    m_sSymbolFont     = rVOpt.m_sSymbolFont;
    m_nShadowCrsrFillMode = rVOpt.m_nShadowCrsrFillMode;
    m_bStarOneSetting = rVOpt.m_bStarOneSetting;
    mbBookView      = rVOpt.mbBookView;
    mbBrowseMode    = rVOpt.mbBrowseMode;
    mbViewLayoutBookMode = rVOpt.mbViewLayoutBookMode;
    mbHideWhitespaceMode = rVOpt.mbHideWhitespaceMode;
    m_bShowPlaceHolderFields = rVOpt.m_bShowPlaceHolderFields;
    m_bIdle           = rVOpt.m_bIdle;

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

void SwViewOption::Init( vcl::Window *pWin )
{
    if( !m_nPixelTwips && pWin )
    {
        m_nPixelTwips = (sal_uInt16)pWin->PixelToLogic( Size(1,1) ).Height();
    }
}

bool SwViewOption::IsAutoCompleteWords()
{
    const SvxSwAutoFormatFlags& rFlags = SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags();
    return rFlags.bAutoCmpltCollectWords;
}

void SwViewOption::SetOnlineSpell(bool b)
{
    if (comphelper::LibreOfficeKit::isActive())
        return;

    b ? (m_nCoreOptions |= VIEWOPT_1_ONLINESPELL ) : ( m_nCoreOptions &= ~VIEWOPT_1_ONLINESPELL);
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
    if(!pShell || dynamic_cast<const SwWebDocShell*>( pShell) )
    {
        nRet = HTMLMODE_ON | HTMLMODE_SOME_STYLES;
        SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
        switch ( rHtmlOpt.GetExportMode() )
        {
            case HTML_CFG_MSIE:
                nRet |= HTMLMODE_FULL_STYLES;
            break;
            case HTML_CFG_NS40:
                // no special features for this browser
            break;
            case HTML_CFG_WRITER:
                nRet |= HTMLMODE_FULL_STYLES;
            break;
        }
    }
    return nRet;
}

Color&   SwViewOption::GetDocColor()
{
    return m_aDocColor;
}

Color&   SwViewOption::GetDocBoundariesColor()
{
    return m_aDocBoundColor;
}

Color&   SwViewOption::GetObjectBoundariesColor()
{
    return m_aObjectBoundColor;
}

Color& SwViewOption::GetAppBackgroundColor()
{
    return m_aAppBackgroundColor;
}

Color&   SwViewOption::GetTableBoundariesColor()
{
    return m_aTableBoundColor;
}

Color&   SwViewOption::GetIndexShadingsColor()
{
    return m_aIndexShadingsColor;
}

Color&   SwViewOption::GetLinksColor()
{
    return m_aLinksColor;
}

Color&   SwViewOption::GetVisitedLinksColor()
{
    return m_aVisitedLinksColor;
}

Color&   SwViewOption::GetDirectCursorColor()
{
    return m_aDirectCursorColor;
}

Color&   SwViewOption::GetTextGridColor()
{
    return m_aTextGridColor;
}

Color&   SwViewOption::GetSpellColor()
{
    return m_aSpellColor;
}

Color&   SwViewOption::GetSmarttagColor()
{
    return m_aSmarttagColor;
}

Color&   SwViewOption::GetShadowColor()
{
    return m_aShadowColor;
}

Color&   SwViewOption::GetFontColor()
{
    return m_aFontColor;
}

Color&   SwViewOption::GetFieldShadingsColor()
{
    return m_aFieldShadingsColor;
}

Color&   SwViewOption::GetSectionBoundColor()
{
    return m_aSectionBoundColor;
}

Color& SwViewOption::GetPageBreakColor()
{
    return m_aPageBreakColor;
}

Color& SwViewOption::GetHeaderFooterMarkColor()
{
    return m_aHeaderFooterMarkColor;
}

void SwViewOption::ApplyColorConfigValues(const svtools::ColorConfig& rConfig )
{
    m_aDocColor.SetColor(rConfig.GetColorValue(svtools::DOCCOLOR).nColor);

    svtools::ColorConfigValue aValue = rConfig.GetColorValue(svtools::DOCBOUNDARIES);
    m_aDocBoundColor.SetColor(aValue.nColor);
    m_nAppearanceFlags = 0;
    if(aValue.bIsVisible)
        m_nAppearanceFlags |= VIEWOPT_DOC_BOUNDARIES;

    m_aAppBackgroundColor.SetColor(rConfig.GetColorValue(svtools::APPBACKGROUND).nColor);

    aValue = rConfig.GetColorValue(svtools::OBJECTBOUNDARIES);
    m_aObjectBoundColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        m_nAppearanceFlags |= VIEWOPT_OBJECT_BOUNDARIES;

    aValue = rConfig.GetColorValue(svtools::TABLEBOUNDARIES);
    m_aTableBoundColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        m_nAppearanceFlags |= VIEWOPT_TABLE_BOUNDARIES;

    aValue = rConfig.GetColorValue(svtools::WRITERIDXSHADINGS);
    m_aIndexShadingsColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        m_nAppearanceFlags |= VIEWOPT_INDEX_SHADINGS;

    aValue = rConfig.GetColorValue(svtools::LINKS);
    m_aLinksColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        m_nAppearanceFlags |= VIEWOPT_LINKS;

    aValue = rConfig.GetColorValue(svtools::LINKSVISITED);
    m_aVisitedLinksColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        m_nAppearanceFlags |= VIEWOPT_VISITED_LINKS;

    aValue = rConfig.GetColorValue(svtools::SHADOWCOLOR);
    m_aShadowColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        m_nAppearanceFlags |= VIEWOPT_SHADOW;

    m_aDirectCursorColor.SetColor(rConfig.GetColorValue(svtools::WRITERDIRECTCURSOR).nColor);

    m_aTextGridColor.SetColor(rConfig.GetColorValue(svtools::WRITERTEXTGRID).nColor);

    m_aSpellColor.SetColor(rConfig.GetColorValue(svtools::SPELL).nColor);

    m_aSmarttagColor.SetColor(rConfig.GetColorValue(svtools::SMARTTAGS).nColor);

    m_aFontColor.SetColor(rConfig.GetColorValue(svtools::FONTCOLOR).nColor);

    aValue = rConfig.GetColorValue(svtools::WRITERFIELDSHADINGS);
    m_aFieldShadingsColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        m_nAppearanceFlags |= VIEWOPT_FIELD_SHADINGS;

    aValue = rConfig.GetColorValue(svtools::WRITERSECTIONBOUNDARIES);
    m_aSectionBoundColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        m_nAppearanceFlags |= VIEWOPT_SECTION_BOUNDARIES;

    aValue = rConfig.GetColorValue(svtools::WRITERPAGEBREAKS);
    m_aPageBreakColor.SetColor(aValue.nColor);

    aValue = rConfig.GetColorValue(svtools::WRITERHEADERFOOTERMARK);
    m_aHeaderFooterMarkColor.SetColor(aValue.nColor);

    m_aScriptIndicatorColor.SetColor(rConfig.GetColorValue(svtools::WRITERSCRIPTINDICATOR).nColor);
}

void SwViewOption::SetAppearanceFlag(sal_Int32 nFlag, bool bSet, bool bSaveInConfig )
{
    if(bSet)
        m_nAppearanceFlags |= nFlag;
    else
        m_nAppearanceFlags &= ~nFlag;
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

bool SwViewOption::IsAppearanceFlag(sal_Int32 nFlag)
{
    return 0 != (m_nAppearanceFlags & nFlag);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
