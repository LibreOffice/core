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
