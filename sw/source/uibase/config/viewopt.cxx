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
#include <authratr.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/syslocale.hxx>

#include <editeng/acorrcfg.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/configurationlistener.hxx>

Color SwViewOption::s_aDocBoundColor(COL_LIGHTGRAY);
Color SwViewOption::s_aObjectBoundColor(COL_LIGHTGRAY);
Color SwViewOption::s_aDocColor(COL_LIGHTGRAY);
Color SwViewOption::s_aAppBackgroundColor(COL_LIGHTGRAY);
Color SwViewOption::s_aTableBoundColor(COL_LIGHTGRAY);
Color SwViewOption::s_aIndexShadingsColor(COL_LIGHTGRAY);
Color SwViewOption::s_aLinksColor(COL_BLUE);
Color SwViewOption::s_aVisitedLinksColor(COL_RED);
Color SwViewOption::s_aDirectCursorColor(COL_BLUE);
Color SwViewOption::s_aTextGridColor(COL_LIGHTGRAY);
Color SwViewOption::s_aSpellColor(COL_LIGHTRED);
Color SwViewOption::s_aSmarttagColor(COL_LIGHTMAGENTA);
Color SwViewOption::s_aFontColor(COL_BLACK);
Color SwViewOption::s_aFieldShadingsColor(COL_LIGHTGRAY);
Color SwViewOption::s_aSectionBoundColor(COL_LIGHTGRAY);
Color SwViewOption::s_aPageBreakColor(COL_BLUE);
Color SwViewOption::s_aScriptIndicatorColor(COL_GREEN);
Color SwViewOption::s_aShadowColor(COL_GRAY);
Color SwViewOption::s_aHeaderFooterMarkColor(COL_BLUE);

ViewOptFlags SwViewOption::s_nAppearanceFlags = ViewOptFlags::DocBoundaries|ViewOptFlags::ObjectBoundaries;
sal_uInt16 SwViewOption::s_nPixelTwips = 0;   // one pixel on the screen

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
                             const SwRect &rRect, ::Color nCol )
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
    pOut->SetLineColor( COL_BLACK );
    pOut->SetFillColor( COL_TRANSPARENT);
    pOut->DrawRect( rRect.SVRect() );
    pOut->SetFillColor( aOldFillColor );
    pOut->SetLineColor( aOldColor );
}

sal_uInt16 SwViewOption::GetPostItsWidth( const OutputDevice *pOut )
{
    assert(pOut && "no Outdev");
    return sal_uInt16(pOut->GetTextWidth("  "));
}

void SwViewOption::PaintPostIts( OutputDevice *pOut, const SwRect &rRect, bool bIsScript )
{
    if( pOut && bIsScript )
    {
        Color aOldLineColor( pOut->GetLineColor() );
        pOut->SetLineColor( COL_GRAY );
        // to make it look nice, we subtract two pixels everywhere
        sal_uInt16 nPix = s_nPixelTwips * 2;
        if( rRect.Width() <= 2 * nPix || rRect.Height() <= 2 * nPix )
            nPix = 0;
        const Point aTopLeft(  rRect.Left()  + nPix, rRect.Top()    + nPix );
        const Point aBotRight( rRect.Right() - nPix, rRect.Bottom() - nPix );
        const SwRect aRect( aTopLeft, aBotRight );
        DrawRect( pOut, aRect, s_aScriptIndicatorColor );
        pOut->SetLineColor( aOldLineColor );
    }
}

SwViewOption::SwViewOption() :
    m_sSymbolFont( "symbol" ),
    m_aRetouchColor( COL_TRANSPARENT ),
    mnViewLayoutColumns( 0 ),
    m_nPagePreviewRow( 1 ),
    m_nPagePreviewCol( 2 ),
    m_nShadowCursorFillMode( FILL_TAB ),
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
        ViewOptFlags1::HardBlank |
        ViewOptFlags1::SoftHyph |
        ViewOptFlags1::Ref |
        ViewOptFlags1::Graphic |
        ViewOptFlags1::Table |
        ViewOptFlags1::Draw |
        ViewOptFlags1::Control |
        ViewOptFlags1::Pageback |
        ViewOptFlags1::Postits;

    m_nCore2Options =
        ViewOptCoreFlags2::BlackFont |
        ViewOptCoreFlags2::HiddenPara;

    m_nUIOptions =
        ViewOptFlags2::Modified |
        ViewOptFlags2::GrfKeepZoom |
        ViewOptFlags2::AnyRuler;

    if (!utl::ConfigManager::IsFuzzing() && MeasurementSystem::Metric != SvtSysLocale().GetLocaleData().getMeasurementSystemEnum())
    {
        m_aSnapSize.setWidth(720);   // 1/2"
        m_aSnapSize.setHeight(720);   // 1/2"

    }
    else
    {
        m_aSnapSize.setWidth(567);   // 1 cm
        m_aSnapSize.setHeight(567);   // 1 cm
    }
    m_nDivisionX = m_nDivisionY = 1;

    m_bSelectionInReadonly = !utl::ConfigManager::IsFuzzing() && SW_MOD()->GetAccessibilityOptions().IsSelectionInReadonly();

    m_bIdle = true;

#ifdef DBG_UTIL
    // correspond to the statements in ui/config/cfgvw.src
    m_bTest1 = m_bTest2 = m_bTest3 = m_bTest4 =
             m_bTest5 = m_bTest6 = m_bTest7 = m_bTest8 = m_bTest10 = false;
#endif
    if (comphelper::LibreOfficeKit::isActive())
        s_aAppBackgroundColor = COL_TRANSPARENT;
}

SwViewOption::SwViewOption(const SwViewOption& rVOpt)
{
    m_bReadonly = false;
    m_bSelectionInReadonly = false;
    // #114856# Form view
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
    m_nShadowCursorFillMode = rVOpt.m_nShadowCursorFillMode;
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
    // #114856# Form view
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
    m_nShadowCursorFillMode = rVOpt.m_nShadowCursorFillMode;
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

void SwViewOption::Init( vcl::Window const *pWin )
{
    if( !s_nPixelTwips && pWin )
    {
        s_nPixelTwips = static_cast<sal_uInt16>(pWin->PixelToLogic( Size(1,1) ).Height());
    }
}

bool SwViewOption::IsAutoCompleteWords()
{
    const SvxSwAutoFormatFlags& rFlags = SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags();
    return rFlags.bAutoCmpltCollectWords;
}

void SwViewOption::SetOnlineSpell(bool b)
{
    if (b)
        m_nCoreOptions |= ViewOptFlags1::OnlineSpell;
    else
        m_nCoreOptions &= ~ViewOptFlags1::OnlineSpell;
}

AuthorCharAttr::AuthorCharAttr() :
    m_nItemId (SID_ATTR_CHAR_UNDERLINE),
    m_nAttr   (LINESTYLE_SINGLE),
    m_nColor  (COL_TRANSPARENT)
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
    return s_aDocColor;
}

Color&   SwViewOption::GetDocBoundariesColor()
{
    return s_aDocBoundColor;
}

Color&   SwViewOption::GetObjectBoundariesColor()
{
    return s_aObjectBoundColor;
}

Color& SwViewOption::GetAppBackgroundColor()
{
    return s_aAppBackgroundColor;
}

Color&   SwViewOption::GetTableBoundariesColor()
{
    return s_aTableBoundColor;
}

Color&   SwViewOption::GetIndexShadingsColor()
{
    return s_aIndexShadingsColor;
}

Color&   SwViewOption::GetLinksColor()
{
    return s_aLinksColor;
}

Color&   SwViewOption::GetVisitedLinksColor()
{
    return s_aVisitedLinksColor;
}

Color&   SwViewOption::GetDirectCursorColor()
{
    return s_aDirectCursorColor;
}

Color&   SwViewOption::GetTextGridColor()
{
    return s_aTextGridColor;
}

Color&   SwViewOption::GetSpellColor()
{
    return s_aSpellColor;
}

Color&   SwViewOption::GetSmarttagColor()
{
    return s_aSmarttagColor;
}

Color&   SwViewOption::GetShadowColor()
{
    return s_aShadowColor;
}

Color&   SwViewOption::GetFontColor()
{
    return s_aFontColor;
}

Color&   SwViewOption::GetFieldShadingsColor()
{
    return s_aFieldShadingsColor;
}

Color&   SwViewOption::GetSectionBoundColor()
{
    return s_aSectionBoundColor;
}

Color& SwViewOption::GetPageBreakColor()
{
    return s_aPageBreakColor;
}

Color& SwViewOption::GetHeaderFooterMarkColor()
{
    return s_aHeaderFooterMarkColor;
}

void SwViewOption::ApplyColorConfigValues(const svtools::ColorConfig& rConfig )
{
    s_aDocColor = rConfig.GetColorValue(svtools::DOCCOLOR).nColor;

    svtools::ColorConfigValue aValue = rConfig.GetColorValue(svtools::DOCBOUNDARIES);
    s_aDocBoundColor = aValue.nColor;
    s_nAppearanceFlags = ViewOptFlags::NONE;
    if(aValue.bIsVisible)
        s_nAppearanceFlags |= ViewOptFlags::DocBoundaries;

    s_aAppBackgroundColor = rConfig.GetColorValue(svtools::APPBACKGROUND).nColor;

    aValue = rConfig.GetColorValue(svtools::OBJECTBOUNDARIES);
    s_aObjectBoundColor = aValue.nColor;
    if(aValue.bIsVisible)
        s_nAppearanceFlags |= ViewOptFlags::ObjectBoundaries;

    aValue = rConfig.GetColorValue(svtools::TABLEBOUNDARIES);
    s_aTableBoundColor = aValue.nColor;
    if(aValue.bIsVisible)
        s_nAppearanceFlags |= ViewOptFlags::TableBoundaries;

    aValue = rConfig.GetColorValue(svtools::WRITERIDXSHADINGS);
    s_aIndexShadingsColor = aValue.nColor;
    if(aValue.bIsVisible)
        s_nAppearanceFlags |= ViewOptFlags::IndexShadings;

    aValue = rConfig.GetColorValue(svtools::LINKS);
    s_aLinksColor = aValue.nColor;
    if(aValue.bIsVisible)
        s_nAppearanceFlags |= ViewOptFlags::Links;

    aValue = rConfig.GetColorValue(svtools::LINKSVISITED);
    s_aVisitedLinksColor = aValue.nColor;
    if(aValue.bIsVisible)
        s_nAppearanceFlags |= ViewOptFlags::VisitedLinks;

    aValue = rConfig.GetColorValue(svtools::SHADOWCOLOR);
    s_aShadowColor = aValue.nColor;
    if(aValue.bIsVisible)
        s_nAppearanceFlags |= ViewOptFlags::Shadow;

    s_aDirectCursorColor = rConfig.GetColorValue(svtools::WRITERDIRECTCURSOR).nColor;

    s_aTextGridColor = rConfig.GetColorValue(svtools::WRITERTEXTGRID).nColor;

    s_aSpellColor = rConfig.GetColorValue(svtools::SPELL).nColor;

    s_aSmarttagColor = rConfig.GetColorValue(svtools::SMARTTAGS).nColor;

    s_aFontColor = rConfig.GetColorValue(svtools::FONTCOLOR).nColor;

    aValue = rConfig.GetColorValue(svtools::WRITERFIELDSHADINGS);
    s_aFieldShadingsColor = aValue.nColor;
    if(aValue.bIsVisible)
        s_nAppearanceFlags |= ViewOptFlags::FieldShadings;

    aValue = rConfig.GetColorValue(svtools::WRITERSECTIONBOUNDARIES);
    s_aSectionBoundColor = aValue.nColor;
    if(aValue.bIsVisible)
        s_nAppearanceFlags |= ViewOptFlags::SectionBoundaries;

    aValue = rConfig.GetColorValue(svtools::WRITERPAGEBREAKS);
    s_aPageBreakColor = aValue.nColor;

    aValue = rConfig.GetColorValue(svtools::WRITERHEADERFOOTERMARK);
    s_aHeaderFooterMarkColor = aValue.nColor;

    s_aScriptIndicatorColor = rConfig.GetColorValue(svtools::WRITERSCRIPTINDICATOR).nColor;
}

void SwViewOption::SetAppearanceFlag(ViewOptFlags nFlag, bool bSet, bool bSaveInConfig )
{
    if(bSet)
        s_nAppearanceFlags |= nFlag;
    else
        s_nAppearanceFlags &= ~nFlag;
    if(bSaveInConfig)
    {
        //create an editable svtools::ColorConfig and store the change
        svtools::EditableColorConfig aEditableConfig;
        struct FlagToConfig_Impl
        {
            ViewOptFlags const                nFlag;
            svtools::ColorConfigEntry const   eEntry;
        };
        static const FlagToConfig_Impl aFlags[] =
        {
            { ViewOptFlags::DocBoundaries     ,   svtools::DOCBOUNDARIES },
            { ViewOptFlags::ObjectBoundaries  ,   svtools::OBJECTBOUNDARIES },
            { ViewOptFlags::TableBoundaries   ,   svtools::TABLEBOUNDARIES },
            { ViewOptFlags::IndexShadings     ,   svtools::WRITERIDXSHADINGS },
            { ViewOptFlags::Links             ,   svtools::LINKS },
            { ViewOptFlags::VisitedLinks      ,   svtools::LINKSVISITED },
            { ViewOptFlags::FieldShadings     ,   svtools::WRITERFIELDSHADINGS },
            { ViewOptFlags::SectionBoundaries ,   svtools::WRITERSECTIONBOUNDARIES },
            { ViewOptFlags::Shadow            ,   svtools::SHADOWCOLOR },
            { ViewOptFlags::NONE              ,   svtools::ColorConfigEntryCount }
        };
        sal_uInt16 nPos = 0;
        while(aFlags[nPos].nFlag != ViewOptFlags::NONE)
        {
            if(nFlag & aFlags[nPos].nFlag)
            {
                svtools::ColorConfigValue aValue = aEditableConfig.GetColorValue(aFlags[nPos].eEntry);
                aValue.bIsVisible = bSet;
                aEditableConfig.SetColorValue(aFlags[nPos].eEntry, aValue);
            }
            nPos++;
        }
    }
}

bool SwViewOption::IsAppearanceFlag(ViewOptFlags nFlag)
{
    return bool(s_nAppearanceFlags & nFlag);
}

namespace{
rtl::Reference<comphelper::ConfigurationListener> const & getWCOptionListener()
{
    static rtl::Reference<comphelper::ConfigurationListener> xListener(new comphelper::ConfigurationListener("/org.openoffice.Office.Writer/Cursor/Option"));
    return xListener;
}
}

bool SwViewOption::IsIgnoreProtectedArea()
{
    static comphelper::ConfigurationListenerProperty<bool> gIgnoreProtectedArea(getWCOptionListener(), "IgnoreProtectedArea");
    return gIgnoreProtectedArea.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
