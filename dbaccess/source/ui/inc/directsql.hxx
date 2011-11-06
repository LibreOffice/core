/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _DBACCESS_UI_DIRECTSQL_HXX_
#define _DBACCESS_UI_DIRECTSQL_HXX_

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SVEDIT_HXX
#include <svtools/editsyntaxhighlighter.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#include <deque>

#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _UNOTOOLS_EVENTLISTENERADAPTER_HXX_
#include <unotools/eventlisteneradapter.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

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
            OSL_ENSURE(sal_False, (ByteString(methodname) += ByteString(": ") += ByteString(pError)).GetBuffer());  \
    }
#else
#define CHECK_INVARIANTS(methodname)
#endif

//........................................................................
}   // namespace dbaui
//........................................................................

#endif // _DBACCESS_UI_DIRECTSQL_HXX_

