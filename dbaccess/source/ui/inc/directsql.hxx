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

#pragma once

#include <sal/config.h>

#include <comphelper/syntaxhighlight.hxx>
#include <sal/log.hxx>
#include <svtools/colorcfg.hxx>
#include <vcl/timer.hxx>
#include <vcl/weld.hxx>
#include <deque>

#include <com/sun/star/sdbc/XConnection.hpp>
#include <unotools/eventlisteneradapter.hxx>
#include <osl/mutex.hxx>

#include "sqledit.hxx"

struct ImplSVEvent;

namespace dbaui
{
    // DirectSQLDialog
    class DirectSQLDialog final
            : public weld::GenericDialogController
            , public ::utl::OEventListenerAdapter
    {
        ::osl::Mutex    m_aMutex;

        std::unique_ptr<weld::Button> m_xExecute;
        std::unique_ptr<weld::ComboBox> m_xSQLHistory;
        std::unique_ptr<weld::TextView> m_xStatus;
        std::unique_ptr<weld::CheckButton> m_xShowOutput;
        std::unique_ptr<weld::TextView> m_xOutput;
        std::unique_ptr<weld::Button> m_xClose;
        std::unique_ptr<SQLEditView> m_xSQL;
        std::unique_ptr<weld::CustomWeld> m_xSQLEd;

        typedef std::deque< OUString >  StringQueue;
        StringQueue     m_aStatementHistory;    // previous statements
        StringQueue     m_aNormalizedHistory;   // previous statements, normalized to be used in the list box

        sal_Int32       m_nStatusCount;

        css::uno::Reference< css::sdbc::XConnection >
                        m_xConnection;

        ImplSVEvent* m_pClosingEvent;

    public:
        DirectSQLDialog(
            weld::Window* _pParent,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConn);
        virtual ~DirectSQLDialog() override;

        /// number of history entries
        sal_Int32 getHistorySize() const;

    private:
        void executeCurrent();
        void switchToHistory(sal_Int32 _nHistoryPos);

        // OEventListenerAdapter
        virtual void _disposing( const css::lang::EventObject& _rSource ) override;

        DECL_LINK( OnExecute, weld::Button&, void );
        DECL_LINK( OnClose, void*, void );
        DECL_LINK( OnCloseClick, weld::Button&, void );
        DECL_LINK( OnListEntrySelected, weld::ComboBox&, void );
        DECL_LINK( OnStatementModified, LinkParamNone*, void );

        /// adds a statement to the statement history
        void implAddToStatementHistory(const OUString& _rStatement);

        /// ensures that our history has at most m_nHistoryLimit entries
        void implEnsureHistoryLimit();

        /// executes the statement given, adds the status to the status list
        void implExecuteStatement(const OUString& _rStatement);

        /// adds a status text to the status list
        void addStatusText(const OUString& _rMessage);

        /// adds a status text to the output list
        void addOutputText(const OUString& _rMessage);

        /// displays resultset
        void display(const css::uno::Reference< css::sdbc::XResultSet >& xRS);

#ifdef DBG_UTIL
        const char* impl_CheckInvariants() const;
#endif
    };

#ifdef DBG_UTIL
#define CHECK_INVARIANTS(methodname)    \
    {   \
        const char* pError = impl_CheckInvariants();    \
        if (pError) \
            SAL_WARN("dbaccess.ui", methodname ": " << pError);   \
    }
#else
#define CHECK_INVARIANTS(methodname)
#endif

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
