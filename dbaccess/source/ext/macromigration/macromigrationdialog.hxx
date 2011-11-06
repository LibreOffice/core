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



#ifndef DBACCESS_MACROMIGRATIONDIALOG_HXX
#define DBACCESS_MACROMIGRATIONDIALOG_HXX

/** === begin UNO includes === **/
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <svtools/roadmapwizard.hxx>

#include <memory>

//........................................................................
namespace dbmm
{
//........................................................................

    //====================================================================
    //= MacroMigrationDialog
    //====================================================================
    struct MacroMigrationDialog_Data;
    typedef ::svt::RoadmapWizard    MacroMigrationDialog_Base;
    class MacroMigrationDialog : public MacroMigrationDialog_Base
    {
    public:
        MacroMigrationDialog(
            Window* _pParent,
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XOfficeDatabaseDocument >& _rxDocument
        );
        virtual ~MacroMigrationDialog();

        const ::comphelper::ComponentContext&
                getComponentContext() const;
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XOfficeDatabaseDocument >&
                getDocument() const;

    protected:
        // OWizardMachine overridables
        virtual void            enterState( WizardState _nState );
        virtual sal_Bool        prepareLeaveCurrentState( CommitPageReason _eReason );
        virtual sal_Bool        leaveState( WizardState _nState );
        virtual WizardState     determineNextState( WizardState _nCurrentState ) const;
        virtual sal_Bool        onFinish();

        // Dialog overridables
        virtual sal_Bool    Close();
        virtual short   Execute();

    private:
        void    impl_showCloseDocsError( bool _bShow );
        bool    impl_closeSubDocs_nothrow();
        bool    impl_backupDocument_nothrow() const;
        void    impl_reloadDocument_nothrow( bool _bMigrationSuccess );

    private:
        DECL_LINK( OnStartMigration, void* );

    private:
        ::std::auto_ptr< MacroMigrationDialog_Data >    m_pData;
    };

//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_MACROMIGRATIONDIALOG_HXX
