/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macromigrationdialog.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:03:38 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "dbmm_global.hrc"
#include "dbmm_module.hxx"
#include "docerrorhandling.hxx"
#include "macromigration.hrc"
#include "macromigrationdialog.hxx"
#include "macromigrationpages.hxx"
#include "migrationengine.hxx"
#include "migrationlog.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XModel.hpp>
/** === end UNO includes === **/

#include <cppuhelper/exc_hlp.hxx>
#include <svtools/filenotation.hxx>
#include <tools/diagnose_ex.h>

//........................................................................
namespace dbmm
{
//........................................................................

#define STATE_CLOSE_SUB_DOCS    0
#define STATE_BACKUP_DBDOC      1
#define STATE_MIGRATE           2
#define STATE_SUMMARY           3

#define PATH_DEFAULT    1

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::sdb::application::XDatabaseDocumentUI;
    using ::com::sun::star::sdb::XOfficeDatabaseDocument;
    using ::com::sun::star::frame::XModel2;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::container::XEnumeration;
    using ::com::sun::star::frame::XStorable;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::frame::XModel;
    /** === end UNO using === **/

    //====================================================================
    //= MacroMigrationDialog_Data
    //====================================================================
    struct MacroMigrationDialog_Data
    {
        ::comphelper::ComponentContext          aContext;
        MigrationLog                            aLogger;
        Reference< XOfficeDatabaseDocument >    xDocument;
        bool                                    bMigrationIsRunning;

        MacroMigrationDialog_Data(
                const ::comphelper::ComponentContext& _rContext,
                const Reference< XOfficeDatabaseDocument >& _rxDocument )
            :aContext( _rContext )
            ,aLogger()
            ,xDocument( _rxDocument )
            ,bMigrationIsRunning( false )
        {
        }
    };


    //====================================================================
    //= MacroMigrationDialog
    //====================================================================
    //--------------------------------------------------------------------
    MacroMigrationDialog::MacroMigrationDialog( Window* _pParent, const ::comphelper::ComponentContext& _rContext,
        const Reference< XOfficeDatabaseDocument >& _rxDocument )
        :MacroMigrationDialog_Base( _pParent, MacroMigrationResId( DLG_MACRO_MIGRATION ) )
        ,m_pData( new MacroMigrationDialog_Data( _rContext, _rxDocument ) )
    {
        String sTitlePrepare( MacroMigrationResId( STR_STATE_CLOSE_SUB_DOCS ) );
        String sTitleStoreAs( MacroMigrationResId( STR_STATE_BACKUP_DBDOC ) );
        String sTitleMigrate( MacroMigrationResId( STR_STATE_MIGRATE ) );
        String sTitleSummary( MacroMigrationResId( STR_STATE_SUMMARY ) );
        FreeResource();

        describeState( STATE_CLOSE_SUB_DOCS,    sTitlePrepare, &PreparationPage::Create   );
        describeState( STATE_BACKUP_DBDOC,      sTitleStoreAs, &SaveDBDocPage::Create     );
        describeState( STATE_MIGRATE,           sTitleMigrate, &ProgressPage::Create      );
        describeState( STATE_SUMMARY,           sTitleSummary, &ResultPage::Create        );

        declarePath( PATH_DEFAULT, STATE_CLOSE_SUB_DOCS, STATE_BACKUP_DBDOC, STATE_MIGRATE, STATE_SUMMARY, WZS_INVALID_STATE );

        SetPageSizePixel( LogicToPixel( ::Size( TAB_PAGE_WIDTH, TAB_PAGE_HEIGHT ), MAP_APPFONT ) );
        ShowButtonFixedLine( true );
        SetRoadmapInteractive( true );
        enableAutomaticNextButtonState();
        defaultButton( WZB_NEXT );
        enableButtons( WZB_FINISH, true );
        ActivatePage();

        OSL_PRECOND( m_pData->xDocument.is(), "MacroMigrationDialog::MacroMigrationDialog: illegal document!" );
    }

    //--------------------------------------------------------------------
    MacroMigrationDialog::~MacroMigrationDialog()
    {
    }

    //--------------------------------------------------------------------
    const ::comphelper::ComponentContext& MacroMigrationDialog::getComponentContext() const
    {
        return m_pData->aContext;
    }

    //--------------------------------------------------------------------
    const Reference< XOfficeDatabaseDocument >& MacroMigrationDialog::getDocument() const
    {
        return m_pData->xDocument;
    }

    //--------------------------------------------------------------------
    BOOL MacroMigrationDialog::Close()
    {
        if ( m_pData->bMigrationIsRunning )
            return FALSE;
        return MacroMigrationDialog_Base::Close();
    }

    //--------------------------------------------------------------------
    void MacroMigrationDialog::enterState( WizardState _nState )
    {
        MacroMigrationDialog_Base::enterState( _nState );

        switch ( _nState )
        {
        case STATE_CLOSE_SUB_DOCS:
            enableButtons( WZB_FINISH, false );
            enableState( STATE_MIGRATE, false );
            enableState( STATE_SUMMARY, false );
            break;

        case STATE_BACKUP_DBDOC:
            enableState( STATE_MIGRATE, true );
            // Note that the state is automatically disabled if the current page
            // (SaveDBDocPage) returns false in its canAdvance, not caring that
            // we enabled it here.
            break;

        case STATE_MIGRATE:
        {
            // disable everything. The process we will start here cannot be cancelled, the user
            // needs to wait 'til it's finished.
            enableState( STATE_CLOSE_SUB_DOCS, false );
            enableState( STATE_BACKUP_DBDOC, false );
            enableState( STATE_SUMMARY, false );

            enableButtons( WZB_FINISH | WZB_CANCEL | WZB_PREVIOUS | WZB_NEXT, false );

            // prevent closing
            m_pData->bMigrationIsRunning = true;
            // start the migration asynchronously
            PostUserEvent( LINK( this, MacroMigrationDialog, OnStartMigration ) );
        }
        break;

        case STATE_SUMMARY:
            // enable the previous step - we can't return to the actual migration, it already happened (or failed)
            enableState( STATE_MIGRATE, false );
            updateTravelUI();
            dynamic_cast< ResultPage& >( *GetPage( STATE_SUMMARY ) ).displaySummary( m_pData->aLogger.getCompleteLog() );
            break;

        default:
            OSL_ENSURE( false, "MacroMigrationDialog::enterState: unhandled state!" );
        }
    }

    //--------------------------------------------------------------------
    sal_Bool MacroMigrationDialog::prepareLeaveCurrentState( CommitPageReason _eReason )
    {
        if ( !MacroMigrationDialog_Base::prepareLeaveCurrentState( _eReason ) )
            return sal_False;

        switch ( getCurrentState() )
        {
        case STATE_CLOSE_SUB_DOCS:
            if ( !impl_closeSubDocs_nothrow() )
                return sal_False;
            break;
        case STATE_BACKUP_DBDOC:
            if ( !impl_backupDocument_nothrow() )
                return sal_False;
            break;
        case STATE_MIGRATE:
            break;
        case STATE_SUMMARY:
            break;
        default:
            OSL_ENSURE( false, "MacroMigrationDialog::prepareLeaveCurrentState: unhandled state!" );
        }

        return sal_True;
    }

    //--------------------------------------------------------------------
    sal_Bool MacroMigrationDialog::leaveState( WizardState _nState )
    {
        return MacroMigrationDialog_Base::leaveState( _nState );
    }

    //--------------------------------------------------------------------
    MacroMigrationDialog::WizardState MacroMigrationDialog::determineNextState( WizardState _nCurrentState ) const
    {
        return MacroMigrationDialog_Base::determineNextState( _nCurrentState );
    }

    //--------------------------------------------------------------------
    sal_Bool MacroMigrationDialog::onFinish( sal_Int32 _nResult )
    {
        return MacroMigrationDialog_Base::onFinish( _nResult );
    }

    //--------------------------------------------------------------------
    IMPL_LINK( MacroMigrationDialog, OnStartMigration, void*, /*_pNotInterestedIn*/ )
    {
        // initialize migration engine and progress
        ProgressPage& rProgressPage( dynamic_cast< ProgressPage& >( *GetPage( STATE_MIGRATE ) ) );
        MigrationEngine aEngine( m_pData->aContext, m_pData->xDocument, rProgressPage, m_pData->aLogger );
        rProgressPage.setDocumentCounts( aEngine.getFormCount(), aEngine.getReportCount() );

        // do the migration
        bool bSuccess = aEngine.migrateAll();

        // re-enable the UI
        enableButtons( ( bSuccess ? WZB_FINISH | WZB_NEXT : 0 ), true );
        enableState( STATE_SUMMARY, bSuccess );
        updateTravelUI();

        m_pData->bMigrationIsRunning = false;

        // outta here
        return 0L;
    }

    //--------------------------------------------------------------------
    void MacroMigrationDialog::impl_showCloseDocsError( bool _bShow )
    {
        PreparationPage* pPreparationPage = dynamic_cast< PreparationPage* >( GetPage( STATE_CLOSE_SUB_DOCS ) );
        OSL_ENSURE( pPreparationPage, "MacroMigrationDialog::impl_showCloseDocsError: did not find the page!" );
        if ( pPreparationPage )
            pPreparationPage->showCloseDocsError( _bShow );
    }

    //--------------------------------------------------------------------
    bool MacroMigrationDialog::impl_closeSubDocs_nothrow()
    {
        OSL_PRECOND( m_pData->xDocument.is(), "MacroMigrationDialog::impl_closeSubDocs_nothrow: no document!" );
        if ( !m_pData->xDocument.is() )
            return false;

        impl_showCloseDocsError( false );

        bool bSuccess = true;
        try
        {
            ::std::vector< Reference< XController > > aControllers;

            // collect all controllers of our document
            Reference< XModel2 > xDocument( m_pData->xDocument, UNO_QUERY_THROW );
            Reference< XEnumeration > xControllerEnum( xDocument->getControllers(), UNO_SET_THROW );
            while ( xControllerEnum->hasMoreElements() )
                aControllers.push_back( Reference< XController >( xControllerEnum->nextElement(), UNO_QUERY_THROW ) );

            // close all sub documents of all controllers
            for (   ::std::vector< Reference< XController > >::const_iterator pos = aControllers.begin();
                    pos != aControllers.end() && bSuccess;
                    ++pos
                )
            {
                Reference< XDatabaseDocumentUI > xController( *pos, UNO_QUERY );
                OSL_ENSURE( xController.is(), "MacroMigrationDialog::impl_closeSubDocs_nothrow: unexpected: controller is missing an important interface!" );
                    // at the moment, only one implementation for a DBDoc's controller exists, which should
                    // support this interface
                if ( !xController.is() )
                    continue;

                bSuccess = xController->closeSubComponents();
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            bSuccess = false;
        }

        impl_showCloseDocsError( !bSuccess );
        return bSuccess;
    }

    //--------------------------------------------------------------------
    bool MacroMigrationDialog::impl_backupDocument_nothrow() const
    {
        const SaveDBDocPage& rBackupPage = dynamic_cast< const SaveDBDocPage& >( *GetPage( STATE_BACKUP_DBDOC ) );
        ::rtl::OUString sBackupLocation( rBackupPage.getBackupLocation() );

        Any aError;
        try
        {
            const Reference< XStorable > xDocument( getDocument(), UNO_QUERY_THROW );
            xDocument->storeToURL( sBackupLocation, Sequence< PropertyValue >() );
        }
        catch( const Exception& )
        {
            aError = ::cppu::getCaughtException();
        }
        if ( !aError.hasValue() )
        {
            ::svt::OFileNotation aFileNotation( sBackupLocation );
            m_pData->aLogger.backedUpDocument( aFileNotation.get( ::svt::OFileNotation::N_SYSTEM ) );
            return true;
        }

        // display the error to the user
        DocumentErrorHandling::reportError( m_pData->aContext, m_pData->xDocument, aError );

        // TODO: log the error

        return false;
    }

//........................................................................
} // namespace dbmm
//........................................................................
