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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_DIRECTSQL_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_DIRECTSQL_HXX

#include <vcl/dialog.hxx>
#include <svtools/editsyntaxhighlighter.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <deque>

#include <com/sun/star/sdbc/XConnection.hpp>
#include <unotools/eventlisteneradapter.hxx>
#include "moduledbu.hxx"
#include <osl/mutex.hxx>

#include <svtools/editbrowsebox.hxx>

namespace dbaui
{

    // DirectSQLDialog
    class DirectSQLDialog
            :public ModalDialog
            ,public ::utl::OEventListenerAdapter
    {
    protected:
        OModuleClient m_aModuleClient;
        ::osl::Mutex    m_aMutex;

        MultiLineEditSyntaxHighlight*    m_pSQL;
        PushButton*       m_pExecute;
        ListBox*          m_pSQLHistory;
        VclMultiLineEdit* m_pStatus;
        CheckBox*         m_pShowOutput;
        VclMultiLineEdit* m_pOutput;
        PushButton*       m_pClose;

        typedef ::std::deque< OUString >  StringQueue;
        StringQueue     m_aStatementHistory;    // previous statements
        StringQueue     m_aNormalizedHistory;   // previous statements, normalized to be used in the list box

        sal_Int32       m_nHistoryLimit;
        sal_Int32       m_nStatusCount;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                        m_xConnection;

    public:
        DirectSQLDialog(
            vcl::Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn);
        virtual ~DirectSQLDialog();

        /// number of history entries
        sal_Int32 getHistorySize() const;

    protected:
        void executeCurrent();
        void switchToHistory(sal_Int32 _nHistoryPos, bool _bUpdateListBox = true);

        // OEventListenerAdapter
        virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource ) SAL_OVERRIDE;

    protected:
        DECL_LINK( OnExecute, void* );
        DECL_LINK( OnClose, void* );
        DECL_LINK( OnListEntrySelected, void* );
        DECL_LINK( OnStatementModified, void* );

    private:
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

#ifdef DBG_UTIL
        const sal_Char* impl_CheckInvariants() const;
#endif
    };

#ifdef DBG_UTIL
#define CHECK_INVARIANTS(methodname)    \
    {   \
        const sal_Char* pError = impl_CheckInvariants();    \
        if (pError) \
            SAL_WARN("dbaccess.ui", methodname ": " << pError);   \
    }
#else
#define CHECK_INVARIANTS(methodname)
#endif

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_DIRECTSQL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
