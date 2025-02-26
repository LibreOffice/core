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

#include <editeng/editids.hrc>
#include <editeng/svxacorr.hxx>
#include <officecfg/Office/Common.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/outdev.hxx>
#include <swmodule.hxx>
#include <viewopt.hxx>
#include <wdocsh.hxx>
#include <swrect.hxx>
#include <viewsh.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <crstate.hxx>
#include <authratr.hxx>
#include <svtools/colorcfg.hxx>
#include <unotools/syslocale.hxx>

#include <editeng/acorrcfg.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/configurationlistener.hxx>

sal_uInt16 SwViewOption::s_nPixelTwips = 0;   // one pixel on the screen
SwViewColors SwViewOption::s_aInitialColorConfig {};

SwViewColors::SwViewColors() :
    m_aDocColor(COL_LIGHTGRAY),
    m_aDocBoundColor(COL_LIGHTGRAY),
    m_aAppBackgroundColor(COL_LIGHTGRAY),
    m_aTableBoundColor(COL_LIGHTGRAY),
    m_aFontColor(COL_BLACK),
    m_aIndexShadingsColor(COL_LIGHTGRAY),
    m_aLinksColor(COL_BLUE),
    m_aVisitedLinksColor(COL_RED),
    m_aTextGridColor(COL_LIGHTGRAY),
    m_aSpellColor(COL_LIGHTRED),
    m_aGrammarColor(COL_LIGHTBLUE),
    m_aSmarttagColor(COL_LIGHTMAGENTA),
    m_aFieldShadingsColor(COL_LIGHTGRAY),
    m_aSectionBoundColor(COL_LIGHTGRAY),
    m_aPageBreakColor(COL_BLUE),
    m_aNonPrintingCharacterColor(Color(0x268bd2)),
    m_aScriptIndicatorColor(COL_GREEN),
    m_aShadowColor(COL_GRAY),
    m_aHeaderFooterMarkColor(COL_BLUE),
    m_nAppearanceFlags(ViewOptFlags::NONE)
{}

SwViewColors::SwViewColors(const svtools::ColorConfig& rConfig)
{
    m_aDocColor = rConfig.GetColorValue(svtools::DOCCOLOR).nColor;

    m_aDocBoundColor = rConfig.GetColorValue(svtools::DOCBOUNDARIES).nColor;
    m_nAppearanceFlags = ViewOptFlags::NONE;
    m_aAppBackgroundColor = rConfig.GetColorValue(svtools::APPBACKGROUND).nColor;
    m_aTableBoundColor = rConfig.GetColorValue(svtools::TABLEBOUNDARIES).nColor;

    svtools::ColorConfigValue aValue;

    aValue = rConfig.GetColorValue(svtools::WRITERIDXSHADINGS);
    m_aIndexShadingsColor = aValue.nColor;
    if(aValue.bIsVisible)
        m_nAppearanceFlags |= ViewOptFlags::IndexShadings;

    aValue = rConfig.GetColorValue(svtools::LINKS);
    m_aLinksColor = aValue.nColor;
    if(aValue.bIsVisible)
        m_nAppearanceFlags |= ViewOptFlags::Links;

    aValue = rConfig.GetColorValue(svtools::LINKSVISITED);
    m_aVisitedLinksColor = aValue.nColor;
    if(aValue.bIsVisible)
        m_nAppearanceFlags |= ViewOptFlags::VisitedLinks;

    aValue = rConfig.GetColorValue(svtools::SHADOWCOLOR);
    m_aShadowColor = aValue.nColor;
    if(aValue.bIsVisible)
        m_nAppearanceFlags |= ViewOptFlags::Shadow;

    m_aTextGridColor = rConfig.GetColorValue(svtools::WRITERTEXTGRID).nColor;

    m_aSpellColor = rConfig.GetColorValue(svtools::SPELL).nColor;
    m_aGrammarColor = rConfig.GetColorValue(svtools::GRAMMAR).nColor;

    m_aSmarttagColor = rConfig.GetColorValue(svtools::SMARTTAGS).nColor;

    m_aFontColor = rConfig.GetColorValue(svtools::FONTCOLOR).nColor;

    aValue = rConfig.GetColorValue(svtools::WRITERFIELDSHADINGS);
    m_aFieldShadingsColor = aValue.nColor;
    // as in initializeForTiledRendering we don't want to enable
    // field shadings for the online case
    if (aValue.bIsVisible && !comphelper::LibreOfficeKit::isActive())
        m_nAppearanceFlags |= ViewOptFlags::FieldShadings;

    m_aSectionBoundColor = rConfig.GetColorValue(svtools::WRITERSECTIONBOUNDARIES).nColor;

    aValue = rConfig.GetColorValue(svtools::WRITERPAGEBREAKS);
    m_aPageBreakColor = aValue.nColor;

    aValue = rConfig.GetColorValue(svtools::WRITERNONPRINTCHARS);
    m_aNonPrintingCharacterColor = aValue.nColor;

    aValue = rConfig.GetColorValue(svtools::WRITERHEADERFOOTERMARK);
    m_aHeaderFooterMarkColor = aValue.nColor;

    m_aScriptIndicatorColor = rConfig.GetColorValue(svtools::WRITERSCRIPTINDICATOR).nColor;
}

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
            && m_nDefaultAnchor == rOpt.m_nDefaultAnchor
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

bool SwViewOption::IsShowOutlineContentVisibilityButton() const
{
    return m_nCoreOptions.bShowOutlineContentVisibilityButton;
}

bool SwViewOption::IsTreatSubOutlineLevelsAsContent() const
{
    return m_nCoreOptions.bTreatSubOutlineLevelsAsContent;
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

SwTwips SwViewOption::GetPostItsWidth(const OutputDevice* pOut)
{
    assert(pOut && "no Outdev");
    return pOut->GetTextWidth(u"  "_ustr);
}

void SwViewOption::PaintPostIts( OutputDevice *pOut, const SwRect &rRect, bool bIsScript ) const
{
    if( !(pOut && bIsScript) )
        return;

    Color aOldLineColor( pOut->GetLineColor() );
    pOut->SetLineColor( COL_GRAY );
    // to make it look nice, we subtract two pixels everywhere
    SwTwips nPix = s_nPixelTwips * 2;
    if( rRect.Width() <= 2 * nPix || rRect.Height() <= 2 * nPix )
        nPix = 0;
    const Point aTopLeft(  rRect.Left()  + nPix, rRect.Top()    + nPix );
    const Point aBotRight( rRect.Right() - nPix, rRect.Bottom() - nPix );
    const SwRect aRect( aTopLeft, aBotRight );
    DrawRect( pOut, aRect,  m_aColorConfig.m_aScriptIndicatorColor );
    pOut->SetLineColor( aOldLineColor );
}

SwViewOption::SwViewOption() :
    m_sThemeName( u"Default"_ustr ),
    m_sSymbolFont( u"symbol"_ustr ),
    m_aRetouchColor( COL_TRANSPARENT ),
    mnViewLayoutColumns( 0 ),
    m_nPagePreviewRow( 1 ),
    m_nPagePreviewCol( 2 ),
    m_nShadowCursorFillMode( SwFillMode::Tab ),
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
    m_bEncloseWithCharactersOn( true ),
    m_nZoom( 100 ),
    m_eZoom( SvxZoomType::PERCENT ),
    m_nTableDestination(TBL_DEST_CELL)
{
    m_nCore2Options =
        ViewOptCoreFlags2::BlackFont |
        ViewOptCoreFlags2::HiddenPara;

    m_nUIOptions =
        ViewOptFlags2::Modified |
        ViewOptFlags2::GrfKeepZoom |
        ViewOptFlags2::ResolvedPostits |
        ViewOptFlags2::AnyRuler;

    if (!comphelper::IsFuzzing() && MeasurementSystem::Metric != SvtSysLocale().GetLocaleData().getMeasurementSystemEnum())
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

    m_bSelectionInReadonly = officecfg::Office::Common::Accessibility::IsSelectionInReadonly::get();

    m_bIdle = true;

    m_nDefaultAnchor = 1; //FLY_TO_CHAR

    // tdf#135266 - tox dialog: remember last used entry level depending on the index type
    m_nTocEntryLvl = 0;
    m_nIdxEntryLvl = 1;

#ifdef DBG_UTIL
    // correspond to the statements in ui/config/cfgvw.src
    m_bTest1 = m_bTest2 = m_bTest3 = m_bTest4 =
             m_bTest5 = m_bTest6 = m_bTest7 = m_bTest8 = m_bTest10 = false;
#endif
    m_aColorConfig = s_aInitialColorConfig;
    if (comphelper::LibreOfficeKit::isActive())
        m_aColorConfig.m_aAppBackgroundColor = COL_TRANSPARENT;
}

SwViewOption::SwViewOption(const SwViewOption& rVOpt)
{
    m_bReadonly = false;
    m_bSelectionInReadonly = false;
    // #114856# Form view
    m_sThemeName = rVOpt.m_sThemeName;
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
    m_bEncloseWithCharactersOn = rVOpt.m_bEncloseWithCharactersOn;
    m_bIdle           = rVOpt.m_bIdle;
    m_nDefaultAnchor  = rVOpt.m_nDefaultAnchor;
    m_nTocEntryLvl = rVOpt.m_nTocEntryLvl;
    m_nIdxEntryLvl = rVOpt.m_nIdxEntryLvl;
    m_aColorConfig    = rVOpt.m_aColorConfig;

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
    m_sThemeName = rVOpt.m_sThemeName;
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
    m_bEncloseWithCharactersOn = rVOpt.m_bEncloseWithCharactersOn;
    m_bIdle           = rVOpt.m_bIdle;
    m_nDefaultAnchor  = rVOpt.m_nDefaultAnchor;
    m_aColorConfig    = rVOpt.m_aColorConfig;

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

void SwViewOption::Init(const OutputDevice* pWin)
{
    if( !s_nPixelTwips && pWin )
    {
        s_nPixelTwips = o3tl::narrowing<sal_uInt16>(pWin->PixelToLogic( Size(1,1) ).Height());
    }
}

bool SwViewOption::IsAutoCompleteWords()
{
    const SvxSwAutoFormatFlags& rFlags = SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags();
    return rFlags.bAutoCmpltCollectWords;
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
        switch ( SvxHtmlOptions::GetExportMode() )
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

RndStdIds SwViewOption::GetDefaultAnchorType() const
{
    switch ( m_nDefaultAnchor )
    {
      case 0:
        return RndStdIds::FLY_AT_PARA; //0
      case 1:
        return RndStdIds::FLY_AT_CHAR; //4
      case 2:
        return RndStdIds::FLY_AS_CHAR; //1
      default:
        return RndStdIds::FLY_AT_CHAR; //4
    }//switch
}

const Color& SwViewOption::GetDocColor() const
{
    return m_aColorConfig.m_aDocColor;
}

const Color& SwViewOption::GetDocBoundariesColor() const
{
    return m_aColorConfig.m_aDocBoundColor;
}

const Color& SwViewOption::GetAppBackgroundColor() const
{
    return m_aColorConfig.m_aAppBackgroundColor;
}

const Color& SwViewOption::GetTableBoundariesColor() const
{
    return m_aColorConfig.m_aTableBoundColor;
}

const Color& SwViewOption::GetIndexShadingsColor() const
{
    return m_aColorConfig.m_aIndexShadingsColor;
}

const Color& SwViewOption::GetLinksColor() const
{
    return m_aColorConfig.m_aLinksColor;
}

const Color& SwViewOption::GetVisitedLinksColor() const
{
    return m_aColorConfig.m_aVisitedLinksColor;
}

const Color& SwViewOption::GetTextGridColor() const
{
    return m_aColorConfig.m_aTextGridColor;
}

const Color& SwViewOption::GetSpellColor() const
{
    return m_aColorConfig.m_aSpellColor;
}

const Color& SwViewOption::GetGrammarColor() const
{
    return m_aColorConfig.m_aGrammarColor;
}

const Color& SwViewOption::GetSmarttagColor() const
{
    return m_aColorConfig.m_aSmarttagColor;
}

const Color& SwViewOption::GetShadowColor() const
{
    return m_aColorConfig.m_aShadowColor;
}

const Color& SwViewOption::GetFontColor() const
{
    return m_aColorConfig.m_aFontColor;
}

const Color& SwViewOption::GetFieldShadingsColor() const
{
    return m_aColorConfig.m_aFieldShadingsColor;
}

const Color& SwViewOption::GetSectionBoundColor() const
{
    return m_aColorConfig.m_aSectionBoundColor;
}

const Color& SwViewOption::GetPageBreakColor() const
{
    return m_aColorConfig.m_aPageBreakColor;
}

const Color& SwViewOption::GetNonPrintingCharacterColor() const
{
    return m_aColorConfig.m_aNonPrintingCharacterColor;
}

const Color& SwViewOption::GetHeaderFooterMarkColor() const
{
    return m_aColorConfig.m_aHeaderFooterMarkColor;
}

void SwViewOption::SetAppearanceFlag(ViewOptFlags nFlag, bool bSet, bool bSaveInConfig )
{
    if(bSet)
        m_aColorConfig.m_nAppearanceFlags |= nFlag;
    else
        m_aColorConfig.m_nAppearanceFlags &= ~nFlag;
    if(!bSaveInConfig)
        return;

    //create an editable svtools::ColorConfig and store the change
    svtools::EditableColorConfig aEditableConfig;
    struct FlagToConfig_Impl
    {
        ViewOptFlags                nFlag;
        svtools::ColorConfigEntry   eEntry;
    };
    static const FlagToConfig_Impl aFlags[] =
    {
        { ViewOptFlags::IndexShadings     ,   svtools::WRITERIDXSHADINGS },
        { ViewOptFlags::Links             ,   svtools::LINKS },
        { ViewOptFlags::VisitedLinks      ,   svtools::LINKSVISITED },
        { ViewOptFlags::FieldShadings     ,   svtools::WRITERFIELDSHADINGS },
        { ViewOptFlags::Shadow            ,   svtools::SHADOWCOLOR },
    };
    for (auto& item : aFlags)
    {
        if (nFlag & item.nFlag)
        {
            svtools::ColorConfigValue aValue = aEditableConfig.GetColorValue(item.eEntry);
            aValue.bIsVisible = bSet;
            aEditableConfig.SetColorValue(item.eEntry, aValue);
        }
    }
}

bool SwViewOption::IsAppearanceFlag(ViewOptFlags nFlag) const
{
    return bool(m_aColorConfig.m_nAppearanceFlags & nFlag);
}

namespace{
rtl::Reference<comphelper::ConfigurationListener> const & getWCOptionListener()
{
    static rtl::Reference<comphelper::ConfigurationListener> xListener(new comphelper::ConfigurationListener(u"/org.openoffice.Office.Writer/Cursor/Option"_ustr));
    return xListener;
}
}

bool SwViewOption::IsIgnoreProtectedArea()
{
    if (comphelper::IsFuzzing())
        return false;
    static comphelper::ConfigurationListenerProperty<bool> gIgnoreProtectedArea(getWCOptionListener(), u"IgnoreProtectedArea"_ustr);
    return gIgnoreProtectedArea.get();
}

void SwViewOption::SyncLayoutRelatedViewOptions(const SwViewOption& rOpt)
{
    SetFieldName(rOpt.IsFieldName());
    SetShowHiddenField(rOpt.IsShowHiddenField());
    SetShowHiddenPara(rOpt.IsShowHiddenPara());
    SetShowHiddenChar(rOpt.IsShowHiddenChar());
    SetViewLayoutBookMode(rOpt.IsViewLayoutBookMode());
    SetHideWhitespaceMode(rOpt.IsHideWhitespaceMode());
    SetViewLayoutColumns(rOpt.GetViewLayoutColumns());
    SetPostIts(rOpt.IsPostIts());
}

const SwViewOption& SwViewOption::GetCurrentViewOptions()
{
    SfxViewShell* pCurrentShell = SfxViewShell::Current();
    SwView* pView = dynamic_cast<SwView*>(pCurrentShell);
    if(pView)
    {
        return *pView->GetWrtShell().GetViewOptions();
    }

    // Some unit tests don't have a SfxViewShell, so we need to return something
    static SwViewOption aDefaultViewOptions;
    return aDefaultViewOptions;
}

void SwViewOption::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwViewOption"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    m_nCoreOptions.dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}

void ViewOptFlags1::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("ViewOptFlags1"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bUseHeaderFooterMenu"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bUseHeaderFooterMenu).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bTab"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bTab).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bBlank"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bBlank).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bHardBlank"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bHardBlank).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bParagraph"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bParagraph).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bLinebreak"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bLinebreak).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bPagebreak"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bPagebreak).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bColumnbreak"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bColumnbreak).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bSoftHyph"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bSoftHyph).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bBookmarks"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bBookmarks).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bRef"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bRef).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bFieldName"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bFieldName).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bPostits"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bPostits).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bFieldHidden"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bFieldHidden).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bCharHidden"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bCharHidden).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bGraphic"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bGraphic).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bTable"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bTable).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bDraw"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bDraw).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bControl"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bControl).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bCrosshair"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bCrosshair).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bSnap"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bSnap).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bSynchronize"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bSynchronize).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bGridVisible"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bGridVisible).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bOnlineSpell"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bOnlineSpell).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bTreatSubOutlineLevelsAsContent"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bTreatSubOutlineLevelsAsContent).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bShowInlineTooltips"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bShowInlineTooltips).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bViewMetachars"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bViewMetachars).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bRedlineRecordingOn"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bRedlineRecordingOn).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bPageback"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bPageback).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bShowOutlineContentVisibilityButton"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bShowOutlineContentVisibilityButton).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bShowChangesInMargin"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bShowChangesInMargin).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bShowChangesInMargin2"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bShowChangesInMargin2).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
