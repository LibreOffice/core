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

#include "sal/config.h"

#include <rtl/ref.hxx>
#include <svtools/editsyntaxhighlighter.hxx>
#include <svl/lstner.hxx>
#include <svtools/colorcfg.hxx>

namespace com { namespace sun { namespace star { namespace beans {
    class XMultiPropertySet;
} } } }

namespace dbaui
{
    class OQueryTextView;
    class OSqlEdit : public MultiLineEditSyntaxHighlight, utl::ConfigurationListener
    {
    private:
        class ChangesListener;
        friend class ChangesListener;

        Timer                   m_timerInvalidate;
        Timer                   m_timerUndoActionCreation;
        Link<>                  m_lnkTextModifyHdl;
        OUString                m_strOrigText;      // is restored on undo
        VclPtr<OQueryTextView>         m_pView;
        bool                m_bAccelAction;     // is set on cut, copy, paste
        bool                m_bStopTimer;
        svtools::ColorConfig    m_ColorConfig;

        rtl::Reference< ChangesListener > m_listener;
        osl::Mutex m_mutex;
        com::sun::star::uno::Reference<
            com::sun::star::beans::XMultiPropertySet > m_notifier;

        DECL_LINK_TYPED(OnUndoActionTimer, Timer*, void);
        DECL_LINK_TYPED(OnInvalidateTimer, Timer*, void);

    private:
        void            ImplSetFont();

    protected:
        virtual void KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
        virtual void GetFocus() SAL_OVERRIDE;

        DECL_LINK(ModifyHdl, void*);

    public:
        OSqlEdit( OQueryTextView* pParent,  WinBits nWinStyle = WB_LEFT | WB_VSCROLL |WB_BORDER);
        virtual ~OSqlEdit();
        virtual void dispose() SAL_OVERRIDE;

        // Edit overridables
        virtual void SetText(const OUString& rNewText) SAL_OVERRIDE;
        using MultiLineEditSyntaxHighlight::SetText;

        // own functionality
        // Cut, Copy, Paste by Accel. runs the action in the Edit but also the
        // corresponding slot in the View. Therefore, the action occurs twice.
       // To prevent this, SlotExec in View can call this function.
        bool IsInAccelAct() { return m_bAccelAction; }

        void SetTextModifyHdl(const Link<>& lnk) { m_lnkTextModifyHdl = lnk; }
            // please don't use SetModifyHdl, I need it for myself, this here is called from the handler set with that
            // the link gets a pointer-to-string, which is invalid after the link

        void stopTimer();
        void startTimer();

        virtual void    ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 ) SAL_OVERRIDE;
        using MultiLineEditSyntaxHighlight::Notify;
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_SQLEDIT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
