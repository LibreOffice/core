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
#include <sqledit.hxx>
#include <QueryTextView.hxx>
#include <querycontainerwindow.hxx>
#include <helpids.h>
#include <undosqledit.hxx>
#include <QueryDesignView.hxx>
#include <svx/svxids.hrc>
#include <cppuhelper/implbase.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/txtattr.hxx>
#include <vcl/textview.hxx>
#include <vcl/xtextedt.hxx>

using namespace dbaui;

class OSqlEdit::ChangesListener:
    public cppu::WeakImplHelper< css::beans::XPropertiesChangeListener >
{
public:
    explicit ChangesListener(OSqlEdit & editor): editor_(editor) {}

private:
    virtual ~ChangesListener() override {}

    virtual void SAL_CALL disposing(css::lang::EventObject const &) override
    {
        osl::MutexGuard g(editor_.m_mutex);
        editor_.m_notifier.clear();
    }

    virtual void SAL_CALL propertiesChange(
        css::uno::Sequence< css::beans::PropertyChangeEvent > const &) override
    {
        SolarMutexGuard g;
        editor_.ImplSetFont();
    }

    OSqlEdit & editor_;
};

OSqlEdit::OSqlEdit(OQueryTextView* pParent)
    : VclMultiLineEdit(pParent, WB_LEFT | WB_VSCROLL | WB_BORDER)
    , aHighlighter(HighlighterLanguage::SQL)
    , m_pView(pParent)
    , m_bAccelAction( false )
    , m_bStopTimer(false )
{
    EnableUpdateData(300);

    SetHelpId( HID_CTL_QRYSQLEDIT );
    SetModifyHdl( LINK(this, OSqlEdit, ModifyHdl) );

    m_timerUndoActionCreation.SetTimeout(1000);
    m_timerUndoActionCreation.SetInvokeHandler(LINK(this, OSqlEdit, OnUndoActionTimer));

    m_timerInvalidate.SetTimeout(200);
    m_timerInvalidate.SetInvokeHandler(LINK(this, OSqlEdit, OnInvalidateTimer));
    m_timerInvalidate.Start();

    ImplSetFont();
    // Listen for change of Font and Color Settings:
    // Using "this" in ctor is a little fishy, but should work here at least as
    // long as there are no derivations:
    m_listener = new ChangesListener(*this);
    css::uno::Reference< css::beans::XMultiPropertySet > n(
        officecfg::Office::Common::Font::SourceViewFont::get(),
        css::uno::UNO_QUERY_THROW);
    {
        osl::MutexGuard g(m_mutex);
        m_notifier = n;
    }
    css::uno::Sequence< OUString > s(2);
    s[0] = "FontHeight";
    s[1] = "FontName";
    n->addPropertiesChangeListener(s, m_listener.get());
    m_ColorConfig.AddListener(this);

    //#i97044#
    EnableFocusSelectionHide( false );
}

void OSqlEdit::DoBracketHilight(sal_uInt16 nKey)
{
    TextSelection aCurrentPos = GetTextView()->GetSelection();
    sal_Int32 nStartPos = aCurrentPos.GetStart().GetIndex();
    const sal_uInt32 nStartPara = aCurrentPos.GetStart().GetPara();
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

    sal_uInt32 nPara = nStartPara;
    do
    {
        if (nPara == nStartPara && nStartPos == 0)
            continue;

        OUString aLine( GetTextEngine()->GetText( nPara ) );

        if (aLine.isEmpty())
            continue;

        for (sal_Int32 i = (nPara==nStartPara) ? nStartPos-1 : aLine.getLength()-1; i>0; --i)
        {
            if (aLine[i] == nChar)
            {
                if (!nCount)
                {
                    GetTextEngine()->SetAttrib( TextAttribFontWeight( WEIGHT_ULTRABOLD ), nPara, i, i+1 );
                    GetTextEngine()->SetAttrib( TextAttribFontColor( Color(0,0,0) ), nPara, i, i+1 );
                    GetTextEngine()->SetAttrib( TextAttribFontWeight( WEIGHT_ULTRABOLD ), nStartPara, nStartPos, nStartPos );
                    GetTextEngine()->SetAttrib( TextAttribFontColor( Color(0,0,0) ), nStartPara, nStartPos, nStartPos );
                    return;
                }
                else
                    --nCount;
            }
            if (aLine[i] == nKey)
                ++nCount;
        }
    } while (nPara--);
}

bool OSqlEdit::PreNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        DoBracketHilight(rNEvt.GetKeyEvent()->GetCharCode());

    return VclMultiLineEdit::PreNotify(rNEvt);
}

Color OSqlEdit::GetSyntaxHighlightColor(const svtools::ColorConfig& rColorConfig, HighlighterLanguage eLanguage, TokenType aToken)
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

Color OSqlEdit::GetColorValue(TokenType aToken)
{
    return GetSyntaxHighlightColor(m_aColorConfig, aHighlighter.GetLanguage(), aToken);
}

void OSqlEdit::UpdateData()
{
    // syntax highlighting
    // this must be possible improved by using notifychange correctly
    bool bTempModified = GetTextEngine()->IsModified();
    for (sal_uInt32 nLine=0; nLine < GetTextEngine()->GetParagraphCount(); ++nLine)
    {
        OUString aLine( GetTextEngine()->GetText( nLine ) );
        GetTextEngine()->RemoveAttribs( nLine );
        std::vector<HighlightPortion> aPortions;
        aHighlighter.getHighlightPortions( aLine, aPortions );
        for (auto const& portion : aPortions)
        {
            GetTextEngine()->SetAttrib( TextAttribFontColor( GetColorValue(portion.tokenType) ), nLine, portion.nBegin, portion.nEnd );
        }
    }
    GetTextView()->ShowCursor( false );
    GetTextEngine()->SetModified(bTempModified);
}

OSqlEdit::~OSqlEdit()
{
    disposeOnce();
}

void OSqlEdit::dispose()
{
    if (m_timerUndoActionCreation.IsActive())
        m_timerUndoActionCreation.Stop();
    css::uno::Reference< css::beans::XMultiPropertySet > n;
    {
        osl::MutexGuard g(m_mutex);
        n = m_notifier;
    }
    if (n.is()) {
        n->removePropertiesChangeListener(m_listener.get());
    }
    m_ColorConfig.RemoveListener(this);
    m_pView.clear();
    VclMultiLineEdit::dispose();
}

void OSqlEdit::KeyInput( const KeyEvent& rKEvt )
{
    OJoinController& rController = m_pView->getContainerWindow()->getDesignView()->getController();
    rController.InvalidateFeature(SID_CUT);
    rController.InvalidateFeature(SID_COPY);

    // Is this a cut, copy, paste event?
    KeyFuncType aKeyFunc = rKEvt.GetKeyCode().GetFunction();
    if( (aKeyFunc==KeyFuncType::CUT)||(aKeyFunc==KeyFuncType::COPY)||(aKeyFunc==KeyFuncType::PASTE) )
        m_bAccelAction = true;

    VclMultiLineEdit::KeyInput( rKEvt );

    if( m_bAccelAction )
        m_bAccelAction = false;
}

void OSqlEdit::GetFocus()
{
    m_strOrigText  =GetText();
    VclMultiLineEdit::GetFocus();
}

IMPL_LINK_NOARG(OSqlEdit, OnUndoActionTimer, Timer *, void)
{
    OUString aText = GetText();
    if(aText == m_strOrigText)
        return;

    OJoinController& rController = m_pView->getContainerWindow()->getDesignView()->getController();
    SfxUndoManager& rUndoMgr = rController.GetUndoManager();
    std::unique_ptr<OSqlEditUndoAct> pUndoAct(new OSqlEditUndoAct( this ));

    pUndoAct->SetOriginalText( m_strOrigText );
    rUndoMgr.AddUndoAction( std::move(pUndoAct) );

    rController.InvalidateFeature(SID_UNDO);
    rController.InvalidateFeature(SID_REDO);

    m_strOrigText  =aText;
}

IMPL_LINK_NOARG(OSqlEdit, OnInvalidateTimer, Timer *, void)
{
    OJoinController& rController = m_pView->getContainerWindow()->getDesignView()->getController();
    rController.InvalidateFeature(SID_CUT);
    rController.InvalidateFeature(SID_COPY);
    if(!m_bStopTimer)
        m_timerInvalidate.Start();
}

IMPL_LINK_NOARG(OSqlEdit, ModifyHdl, Edit&, void)
{
    if (m_timerUndoActionCreation.IsActive())
        m_timerUndoActionCreation.Stop();
    m_timerUndoActionCreation.Start();

    OJoinController& rController = m_pView->getContainerWindow()->getDesignView()->getController();
    if (!rController.isModified())
        rController.setModified( true );

    rController.InvalidateFeature(SID_SBA_QRY_EXECUTE);
    rController.InvalidateFeature(SID_CUT);
    rController.InvalidateFeature(SID_COPY);
}

void OSqlEdit::SetText(const OUString& rNewText)
{
    if (m_timerUndoActionCreation.IsActive())
    {   // create the trailing undo-actions
        m_timerUndoActionCreation.Stop();
        LINK(this, OSqlEdit, OnUndoActionTimer).Call(nullptr);
    }

    VclMultiLineEdit::SetText(rNewText);
    UpdateData();

    m_strOrigText  =rNewText;
}

void OSqlEdit::stopTimer()
{
    m_bStopTimer = true;
    if (m_timerInvalidate.IsActive())
        m_timerInvalidate.Stop();
}

void OSqlEdit::startTimer()
{
    m_bStopTimer = false;
    if (!m_timerInvalidate.IsActive())
        m_timerInvalidate.Start();
}

void OSqlEdit::ConfigurationChanged( utl::ConfigurationBroadcaster* pOption, ConfigurationHints )
{
    assert( pOption == &m_ColorConfig );
    (void) pOption; // avoid warnings
    VclMultiLineEdit::UpdateData();
}

void OSqlEdit::ImplSetFont()
{
    AllSettings aSettings = GetSettings();
    StyleSettings aStyleSettings = aSettings.GetStyleSettings();
    OUString sFontName(
        officecfg::Office::Common::Font::SourceViewFont::FontName::get().
        value_or( OUString() ) );
    if ( sFontName.isEmpty() )
    {
        vcl::Font aTmpFont( OutputDevice::GetDefaultFont( DefaultFontType::FIXED, Application::GetSettings().GetUILanguageTag().getLanguageType(), GetDefaultFontFlags::NONE, this ) );
        sFontName = aTmpFont.GetFamilyName();
    }
    Size aFontSize(
        0, officecfg::Office::Common::Font::SourceViewFont::FontHeight::get() );
    vcl::Font aFont( sFontName, aFontSize );
    aStyleSettings.SetFieldFont(aFont);
    aSettings.SetStyleSettings(aStyleSettings);
    SetSettings(aSettings);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
