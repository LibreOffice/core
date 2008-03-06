/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macromigrationdialog.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:03:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
        virtual sal_Bool        onFinish( sal_Int32 _nResult );

        // Dialog overridables
        virtual BOOL    Close();

    private:
        void    impl_showCloseDocsError( bool _bShow );
        bool    impl_closeSubDocs_nothrow();
        bool    impl_backupDocument_nothrow() const;

    private:
        DECL_LINK( OnStartMigration, void* );

    private:
        ::std::auto_ptr< MacroMigrationDialog_Data >    m_pData;
    };

//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_MACROMIGRATIONDIALOG_HXX
