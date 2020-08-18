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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_SQLEDIT_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_SQLEDIT_HXX

#include <sal/config.h>

#include <comphelper/syntaxhighlight.hxx>
#include <rtl/ref.hxx>
#include <svtools/colorcfg.hxx>
#include <vcl/vclmedit.hxx>

namespace com::sun::star::beans { class XMultiPropertySet; }

namespace dbaui
{
    class OQueryTextView;
    class OSqlEdit final : public VclMultiLineEdit, public utl::ConfigurationListener
    {
    private:
        class ChangesListener;
        friend class ChangesListener;

        SyntaxHighlighter    aHighlighter;
        svtools::ColorConfig m_aColorConfig;

        Timer                   m_timerInvalidate;
        Timer                   m_timerUndoActionCreation;
        OUString                m_strOrigText;      // is restored on undo
        VclPtr<OQueryTextView>  m_pView;
        bool                    m_bAccelAction;     // is set on cut, copy, paste
        bool                    m_bStopTimer;
        svtools::ColorConfig    m_ColorConfig;

        rtl::Reference< ChangesListener > m_listener;
        osl::Mutex              m_mutex;
        css::uno::Reference<  css::beans::XMultiPropertySet > m_notifier;

        DECL_LINK(OnUndoActionTimer, Timer*, void);
        DECL_LINK(OnInvalidateTimer, Timer*, void);

        void ImplSetFont();
        void DoBracketHilight(sal_uInt16 aKey);

        virtual bool PreNotify( NotifyEvent& rNEvt ) override;
        virtual void KeyInput( const KeyEvent& rKEvt ) override;
        virtual void GetFocus() override;

        DECL_LINK(ModifyHdl, Edit&, void);

    public:
        OSqlEdit( OQueryTextView* pParent);
        virtual ~OSqlEdit() override;
        virtual void dispose() override;

        // Edit overridables
        virtual void SetText(const OUString& rNewText) override;
        virtual void SetText( const OUString& rStr, const Selection& rNewSelection ) override
                    { SetText( rStr ); SetSelection( rNewSelection ); }

        virtual void UpdateData() override;

        static Color GetSyntaxHighlightColor(const svtools::ColorConfig& rColorConfig, HighlighterLanguage eLanguage, TokenType aToken);

        Color GetColorValue(TokenType aToken);

        // own functionality
        // Cut, Copy, Paste by Accel. runs the action in the Edit but also the
        // corresponding slot in the View. Therefore, the action occurs twice.
        // To prevent this, SlotExec in View can call this function.
        bool IsInAccelAct() const { return m_bAccelAction; }

        void stopTimer();
        void startTimer();

        virtual void    ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints ) override;
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_SQLEDIT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
