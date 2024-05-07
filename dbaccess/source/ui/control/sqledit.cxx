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

#include <sal/config.h>

#include <cassert>

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <officecfg/Office/Common.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/wghtitem.hxx>
#include <sqledit.hxx>
#include <cppuhelper/implbase.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/specialchars.hxx>
#include <vcl/svapp.hxx>

using namespace dbaui;

class SQLEditView::ChangesListener:
    public cppu::WeakImplHelper< css::beans::XPropertiesChangeListener >
{
public:
    explicit ChangesListener(SQLEditView& editor): editor_(editor) {}

private:
    virtual ~ChangesListener() override {}

    virtual void SAL_CALL disposing(css::lang::EventObject const &) override
    {
        std::unique_lock g(editor_.m_mutex);
        editor_.m_notifier.clear();
    }

    virtual void SAL_CALL propertiesChange(
        css::uno::Sequence< css::beans::PropertyChangeEvent > const &) override
    {
        SolarMutexGuard g;
        editor_.ImplSetFont();
    }

    SQLEditView& editor_;
};

SQLEditView::SQLEditView(std::unique_ptr<weld::ScrolledWindow> xScrolledWindow)
    : m_xScrolledWindow(std::move(xScrolledWindow))
    , m_aUpdateDataTimer("dbaccess SQLEditView m_aUpdateDataTimer")
    , m_aHighlighter(HighlighterLanguage::SQL)
    , m_bInUpdate(false)
    , m_bDisableInternalUndo(false)
{
    m_xScrolledWindow->connect_vadjustment_changed(LINK(this, SQLEditView, ScrollHdl));
}

void SQLEditView::DisableInternalUndo()
{
    GetEditEngine()->EnableUndo(false);
    m_bDisableInternalUndo = true;
}

void SQLEditView::SetItemPoolFont(SfxItemPool* pItemPool)
{
    OUString sFontName(officecfg::Office::Common::Font::SourceViewFont::FontName::get().value_or(OUString()));
    if (sFontName.isEmpty())
    {
        vcl::Font aTmpFont(OutputDevice::GetDefaultFont(DefaultFontType::FIXED, Application::GetSettings().GetUILanguageTag().getLanguageType(), GetDefaultFontFlags::OnlyOne));
        sFontName = aTmpFont.GetFamilyName();
    }

    Size aFontSize(0, officecfg::Office::Common::Font::SourceViewFont::FontHeight::get());
    vcl::Font aAppFont(sFontName, aFontSize);

    pItemPool->SetUserDefaultItem(SvxFontItem(aAppFont.GetFamilyType(), aAppFont.GetFamilyName(),
                                              u""_ustr, PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW,
                                              EE_CHAR_FONTINFO));
    pItemPool->SetUserDefaultItem(SvxFontItem(aAppFont.GetFamilyType(), aAppFont.GetFamilyName(),
                                              u""_ustr, PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW,
                                              EE_CHAR_FONTINFO_CJK));
    pItemPool->SetUserDefaultItem(SvxFontItem(aAppFont.GetFamilyType(), aAppFont.GetFamilyName(),
                                              u""_ustr, PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW,
                                              EE_CHAR_FONTINFO_CTL));

    pItemPool->SetUserDefaultItem(
        SvxFontHeightItem(aAppFont.GetFontHeight() * 20, 100, EE_CHAR_FONTHEIGHT));
    pItemPool->SetUserDefaultItem(
        SvxFontHeightItem(aAppFont.GetFontHeight() * 20, 100, EE_CHAR_FONTHEIGHT_CJK));
    pItemPool->SetUserDefaultItem(
        SvxFontHeightItem(aAppFont.GetFontHeight() * 20, 100, EE_CHAR_FONTHEIGHT_CTL));
}

void SQLEditView::makeEditEngine()
{
    assert(!m_pItemPool);
    m_pItemPool = EditEngine::CreatePool();
    SetItemPoolFont(m_pItemPool.get());
    m_xEditEngine.reset(new EditEngine(m_pItemPool.get()));
}

void SQLEditView::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    WeldEditView::SetDrawingArea(pDrawingArea);

    EditEngine& rEditEngine = *GetEditEngine();

    rEditEngine.SetDefaultHorizontalTextDirection(EEHorizontalTextDirection::L2R);
    rEditEngine.SetModifyHdl(LINK(this, SQLEditView, ModifyHdl));
    rEditEngine.SetStatusEventHdl(LINK(this, SQLEditView, EditStatusHdl));

    m_aUpdateDataTimer.SetTimeout(150);
    m_aUpdateDataTimer.SetInvokeHandler(LINK(this, SQLEditView, ImplUpdateDataHdl));

    ImplSetFont();

    // Listen for change of Font and Color Settings:
    // Using "this" in ctor is a little fishy, but should work here at least as
    // long as there are no derivations:
    m_listener = new ChangesListener(*this);
    css::uno::Reference< css::beans::XMultiPropertySet > n(
        officecfg::Office::Common::Font::SourceViewFont::get(),
        css::uno::UNO_QUERY_THROW);
    {
        std::unique_lock g(m_mutex);
        m_notifier = n;
    }
    css::uno::Sequence< OUString > s { u"FontHeight"_ustr, u"FontName"_ustr };
    n->addPropertiesChangeListener(s, m_listener);
    m_ColorConfig.AddListener(this);
}

SQLEditView::~SQLEditView()
{
    css::uno::Reference< css::beans::XMultiPropertySet > n;
    {
        std::unique_lock g(m_mutex);
        n = m_notifier;
    }
    if (n.is()) {
        n->removePropertiesChangeListener(m_listener);
    }
    m_ColorConfig.RemoveListener(this);
}

void SQLEditView::SetTextAndUpdate(const OUString& rNewText)
{
    SetText(rNewText);
    UpdateData();
}

IMPL_LINK_NOARG(SQLEditView, ModifyHdl, LinkParamNone*, void)
{
    if (m_bInUpdate)
        return;
    m_aUpdateDataTimer.Start();
}

IMPL_LINK_NOARG(SQLEditView, ImplUpdateDataHdl, Timer*, void)
{
    UpdateData();
}

Color SQLEditView::GetColorValue(TokenType aToken)
{
    return GetSyntaxHighlightColor(m_aColorConfig, m_aHighlighter.GetLanguage(), aToken);
}

void SQLEditView::UpdateData()
{
    m_bInUpdate = true;
    EditEngine& rEditEngine = *GetEditEngine();

    bool bModified = rEditEngine.IsModified();
    bool bUndoEnabled = rEditEngine.IsUndoEnabled();
    rEditEngine.EnableUndo(false);

    // syntax highlighting
    for (sal_Int32 nLine=0; nLine < rEditEngine.GetParagraphCount(); ++nLine)
    {
        OUString aLine( rEditEngine.GetText( nLine ) );

        ESelection aAllLine(nLine, 0, nLine, EE_TEXTPOS_ALL);
        rEditEngine.RemoveAttribs(aAllLine, false, EE_CHAR_COLOR);
        rEditEngine.RemoveAttribs(aAllLine, false, EE_CHAR_WEIGHT);
        rEditEngine.RemoveAttribs(aAllLine, false, EE_CHAR_WEIGHT_CJK);
        rEditEngine.RemoveAttribs(aAllLine, false, EE_CHAR_WEIGHT_CTL);

        std::vector<HighlightPortion> aPortions;
        m_aHighlighter.getHighlightPortions( aLine, aPortions );
        for (auto const& portion : aPortions)
        {
            SfxItemSet aSet(rEditEngine.GetEmptyItemSet());
            aSet.Put(SvxColorItem(GetColorValue(portion.tokenType), EE_CHAR_COLOR));
            rEditEngine.QuickSetAttribs(aSet, ESelection(nLine, portion.nBegin, nLine, portion.nEnd));
        }
    }

    rEditEngine.ClearModifyFlag();

    m_bInUpdate = false;

    rEditEngine.EnableUndo(bUndoEnabled);

    if (bModified)
        m_aModifyLink.Call(nullptr);

    Invalidate();
}

void SQLEditView::DoBracketHilight(sal_uInt16 nKey)
{
    ESelection aCurrentPos = m_xEditView->GetSelection();
    sal_Int32 nStartPos = aCurrentPos.nStartPos;
    const sal_uInt32 nStartPara = aCurrentPos.nStartPara;
    sal_uInt16 nCount = 0;
    int nChar = -1;

    switch (nKey)
    {
        case '\'':  // no break
        case '"':
        {
            nChar = nKey;
            break;
        }
        case '}' :
        {
            nChar = '{';
            break;
        }
        case ')':
        {
            nChar = '(';
            break;
        }
        case ']':
        {
            nChar = '[';
            break;
        }
    }

    if (nChar == -1)
        return;

    bool bUndoEnabled = m_xEditEngine->IsUndoEnabled();
    m_xEditEngine->EnableUndo(false);

    sal_uInt32 nPara = nStartPara;
    do
    {
        if (nPara == nStartPara && nStartPos == 0)
            continue;

        OUString aLine( m_xEditEngine->GetText( nPara ) );

        if (aLine.isEmpty())
            continue;

        for (sal_Int32 i = (nPara==nStartPara) ? nStartPos-1 : aLine.getLength()-1; i>0; --i)
        {
            if (aLine[i] == nChar)
            {
                if (!nCount)
                {
                    SfxItemSet aSet(m_xEditEngine->GetEmptyItemSet());
                    aSet.Put(SvxColorItem(Color(0,0,0), EE_CHAR_COLOR));
                    aSet.Put(SvxWeightItem(WEIGHT_ULTRABOLD, EE_CHAR_WEIGHT));
                    aSet.Put(SvxWeightItem(WEIGHT_ULTRABOLD, EE_CHAR_WEIGHT_CJK));
                    aSet.Put(SvxWeightItem(WEIGHT_ULTRABOLD, EE_CHAR_WEIGHT_CTL));

                    m_xEditEngine->QuickSetAttribs(aSet, ESelection(nPara, i, nPara, i + 1));
                    m_xEditEngine->QuickSetAttribs(aSet, ESelection(nStartPara, nStartPos, nStartPara, nStartPos));
                    return;
                }
                else
                    --nCount;
            }
            if (aLine[i] == nKey)
                ++nCount;
        }
    } while (nPara--);

    m_xEditEngine->EnableUndo(bUndoEnabled);
}

Color SQLEditView::GetSyntaxHighlightColor(const svtools::ColorConfig& rColorConfig, HighlighterLanguage eLanguage, TokenType aToken)
{
    Color aColor;
    switch (eLanguage)
    {
        case HighlighterLanguage::SQL:
        {
            switch (aToken)
            {
                case TokenType::Identifier: aColor = rColorConfig.GetColorValue(svtools::SQLIDENTIFIER).nColor; break;
                case TokenType::Number:     aColor = rColorConfig.GetColorValue(svtools::SQLNUMBER).nColor; break;
                case TokenType::String:     aColor = rColorConfig.GetColorValue(svtools::SQLSTRING).nColor; break;
                case TokenType::Operator:   aColor = rColorConfig.GetColorValue(svtools::SQLOPERATOR).nColor; break;
                case TokenType::Keywords:   aColor = rColorConfig.GetColorValue(svtools::SQLKEYWORD).nColor; break;
                case TokenType::Parameter:  aColor = rColorConfig.GetColorValue(svtools::SQLPARAMETER).nColor; break;
                case TokenType::Comment:    aColor = rColorConfig.GetColorValue(svtools::SQLCOMMENT).nColor; break;
                default:            aColor = Color(0,0,0);
            }
            break;
        }
        case HighlighterLanguage::Basic:
        {
            switch (aToken)
            {
                case TokenType::Identifier: aColor = Color(255,0,0); break;
                case TokenType::Comment:    aColor = Color(0,0,45); break;
                case TokenType::Number:     aColor = Color(204,102,204); break;
                case TokenType::String:     aColor = Color(0,255,45); break;
                case TokenType::Operator:   aColor = Color(0,0,100); break;
                case TokenType::Keywords:   aColor = Color(0,0,255); break;
                case TokenType::Error :     aColor = Color(0,255,255); break;
                default:            aColor = Color(0,0,0);
            }
            break;
        }
        default: aColor = Color(0,0,0);

    }
    return aColor;
}

bool SQLEditView::KeyInput(const KeyEvent& rKEvt)
{
    DoBracketHilight(rKEvt.GetCharCode());

    if (m_bDisableInternalUndo)
    {
        KeyFuncType eFunc = rKEvt.GetKeyCode().GetFunction();
        if (eFunc == KeyFuncType::UNDO || eFunc == KeyFuncType::REDO)
            return false;
    }

    return WeldEditView::KeyInput(rKEvt);
}

bool SQLEditView::Command(const CommandEvent& rCEvt)
{
    if (rCEvt.GetCommand() == CommandEventId::ContextMenu)
    {
        ::tools::Rectangle aRect(rCEvt.GetMousePosPixel(), Size(1, 1));
        weld::Widget* pPopupParent = GetDrawingArea();
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(pPopupParent, u"vcl/ui/editmenu.ui"_ustr));
        std::unique_ptr<weld::Menu> xContextMenu(xBuilder->weld_menu(u"menu"_ustr));

        bool bEnableCut = true;
        bool bEnableCopy = true;
        bool bEnableDelete = true;
        bool bEnablePaste = true;
        bool bEnableSpecialChar = true;

        EditView* pEditView = GetEditView();

        if (!pEditView->HasSelection())
        {
            bEnableCut = false;
            bEnableCopy = false;
            bEnableDelete = false;
        }

        if (pEditView->IsReadOnly())
        {
            bEnableCut = false;
            bEnablePaste = false;
            bEnableDelete = false;
            bEnableSpecialChar = false;
        }

        xContextMenu->set_sensitive(u"cut"_ustr, bEnableCut);
        xContextMenu->set_sensitive(u"copy"_ustr, bEnableCopy);
        xContextMenu->set_sensitive(u"delete"_ustr, bEnableDelete);
        xContextMenu->set_sensitive(u"paste"_ustr, bEnablePaste);
        xContextMenu->set_sensitive(u"specialchar"_ustr, bEnableSpecialChar);
        xContextMenu->set_visible(u"undo"_ustr, false);
        xContextMenu->set_visible(u"specialchar"_ustr, vcl::GetGetSpecialCharsFunction() != nullptr);

        OUString sCommand = xContextMenu->popup_at_rect(pPopupParent, aRect);

        if (sCommand == "cut")
            pEditView->Cut();
        else if (sCommand == "copy")
            pEditView->Copy();
        else if (sCommand == "paste")
            pEditView->Paste();
        else if (sCommand == "delete")
            pEditView->DeleteSelected();
        else if (sCommand == "selectall")
        {
            sal_Int32 nPar = m_xEditEngine->GetParagraphCount();
            if (nPar)
            {
                sal_Int32 nLen = m_xEditEngine->GetTextLen(nPar - 1);
                pEditView->SetSelection(ESelection(0, 0, nPar - 1, nLen));
            }
        }
        else if (sCommand == "specialchar")
        {
            OUString aChars = vcl::GetGetSpecialCharsFunction()(pPopupParent, m_xEditEngine->GetStandardFont(0));
            if (!aChars.isEmpty())
            {
                pEditView->InsertText(aChars);
            }
        }

        return true;
    }
    return WeldEditView::Command(rCEvt);
}

void SQLEditView::EditViewScrollStateChange()
{
    // editengine height has changed or editview scroll pos has changed
    SetScrollBarRange();
}

void SQLEditView::SetScrollBarRange()
{
    EditEngine *pEditEngine = GetEditEngine();
    if (!pEditEngine)
        return;
    if (!m_xScrolledWindow)
        return;
    EditView* pEditView = GetEditView();
    if (!pEditView)
        return;

    int nVUpper = pEditEngine->GetTextHeight();
    int nVCurrentDocPos = pEditView->GetVisArea().Top();
    const Size aOut(pEditView->GetOutputArea().GetSize());
    int nVStepIncrement = aOut.Height() * 2 / 10;
    int nVPageIncrement = aOut.Height() * 8 / 10;
    int nVPageSize = aOut.Height();

    /* limit the page size to below nUpper because gtk's gtk_scrolled_window_start_deceleration has
       effectively...

       lower = gtk_adjustment_get_lower
       upper = gtk_adjustment_get_upper - gtk_adjustment_get_page_size

       and requires that upper > lower or the deceleration animation never ends
    */
    nVPageSize = std::min(nVPageSize, nVUpper);

    m_xScrolledWindow->vadjustment_configure(nVCurrentDocPos, 0, nVUpper,
                                             nVStepIncrement, nVPageIncrement, nVPageSize);
}

IMPL_LINK_NOARG(SQLEditView, ScrollHdl, weld::ScrolledWindow&, void)
{
    DoScroll();
}

IMPL_LINK_NOARG(SQLEditView, EditStatusHdl, EditStatus&, void)
{
    Resize();
}

void SQLEditView::DoScroll()
{
    if (m_xEditView)
    {
        auto currentDocPos = m_xEditView->GetVisArea().Top();
        auto nDiff = currentDocPos - m_xScrolledWindow->vadjustment_get_value();
        // we expect SetScrollBarRange callback to be triggered by Scroll
        // to set where we ended up
        m_xEditView->Scroll(0, nDiff);
    }
}

void SQLEditView::ConfigurationChanged(utl::ConfigurationBroadcaster*, ConfigurationHints)
{
    UpdateData();
}

void SQLEditView::ImplSetFont()
{
    // see SmEditWindow::DataChanged for a similar case
    SetItemPoolFont(m_pItemPool.get()); // change default font
    // re-create with the new font
    EditEngine& rEditEngine = *GetEditEngine();
    OUString aTxt(rEditEngine.GetText());
    rEditEngine.Clear();
    SetTextAndUpdate(aTxt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
