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

#ifndef _DBACCESS_UI_DIRECTSQL_HXX_
#define _DBACCESS_UI_DIRECTSQL_HXX_

#include <vcl/dialog.hxx>
#include <svtools/editsyntaxhighlighter.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <comphelper/stl_types.hxx>
#include <deque>

#include <com/sun/star/sdbc/XConnection.hpp>
#include <unotools/eventlisteneradapter.hxx>
#include "moduledbu.hxx"
#include <osl/mutex.hxx>

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= DirectSQLDialog
    //====================================================================
    class DirectSQLDialog
            :public ModalDialog
            ,public ::utl::OEventListenerAdapter
    {
    protected:
        OModuleClient m_aModuleClient;
        ::osl::Mutex    m_aMutex;

        FixedLine       m_aFrame;
        FixedText       m_aSQLLabel;
        MultiLineEditSyntaxHighlight    m_aSQL;
        PushButton      m_aExecute;
        FixedText       m_aHistoryLabel;
        ListBox*        m_pSQLHistory;
        FixedLine       m_aStatusFrame;
        MultiLineEdit   m_aStatus;
        FixedLine       m_aButtonSeparator;
        HelpButton      m_aHelp;
        PushButton      m_aClose;

        typedef ::std::deque< String >  StringQueue;
        StringQueue     m_aStatementHistory;    // previous statements
        StringQueue     m_aNormalizedHistory;   // previous statements, normalized to be used in the list box

        sal_Int32       m_nHistoryLimit;
        sal_Int32       m_nStatusCount;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                        m_xConnection;

    public:
        DirectSQLDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn);
        ~DirectSQLDialog();

        /// number of history entries
        sal_Int32 getHistorySize() const;

    protected:
        void executeCurrent();
        void switchToHistory(sal_Int32 _nHistoryPos, sal_Bool _bUpdateListBox = sal_True);

        // OEventListenerAdapter
        virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource );

    protected:
        DECL_LINK( OnExecute, void* );
        DECL_LINK( OnClose, void* );
        DECL_LINK( OnListEntrySelected, void* );
        DECL_LINK( OnStatementModified, void* );

    private:
        /// adds a statement to the statement history
        void implAddToStatementHistory(const String& _rStatement);

        /// ensures that our history has at most m_nHistoryLimit entries
        void implEnsureHistoryLimit();

        /// executes the statement given, adds the status to the status list
        void implExecuteStatement(const String& _rStatement);

        /// adds a status text to the status list
        void addStatusText(const String& _rMessage);

#ifdef DBG_UTIL
        const sal_Char* impl_CheckInvariants() const;
#endif
    };

    //====================================================================
#ifdef DBG_UTIL
#define CHECK_INVARIANTS(methodname)    \
    {   \
        const sal_Char* pError = impl_CheckInvariants();    \
        if (pError) \
            OSL_FAIL((ByteString(methodname) += ByteString(": ") += ByteString(pError)).GetBuffer());   \
    }
#else
#define CHECK_INVARIANTS(methodname)
#endif

//........................................................................
}   // namespace dbaui
//........................................................................

#endif // _DBACCESS_UI_DIRECTSQL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
