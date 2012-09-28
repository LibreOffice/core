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


#include "dbmm_global.hrc"
#include "dbmm_module.hxx"
#include "docinteraction.hxx"
#include "macromigration.hrc"
#include "macromigrationdialog.hxx"
#include "macromigrationpages.hxx"
#include "migrationengine.hxx"
#include "migrationerror.hxx"
#include "migrationlog.hxx"

#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/XContent.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase1.hxx>
#include <rtl/ref.hxx>
#include <svl/filenotation.hxx>
#include <tools/diagnose_ex.h>
#include <ucbhelper/content.hxx>
#include <vcl/msgbox.hxx>

#include <list>

//........................................................................
namespace dbmm
{
//........................................................................

#define STATE_CLOSE_SUB_DOCS    0
#define STATE_BACKUP_DBDOC      1
#define STATE_MIGRATE           2
#define STATE_SUMMARY           3

#define PATH_DEFAULT            1

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
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
    using ::com::sun::star::frame::XController2;
    using ::com::sun::star::container::XEnumeration;
    using ::com::sun::star::frame::XStorable;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::util::XCloseable;
    using ::com::sun::star::util::XCloseListener;
    using ::com::sun::star::util::CloseVetoException;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::frame::XComponentLoader;
    using ::com::sun::star::util::XModifiable;
    using ::com::sun::star::ucb::UniversalContentBroker;
    using ::com::sun::star::ucb::XCommandEnvironment;
    using ::com::sun::star::ucb::XContent;
    using ::com::sun::star::ucb::XContentIdentifier;
    /** === end UNO using === **/

    //====================================================================
    //= helper
    //====================================================================
    //--------------------------------------------------------------------
    static void lcl_getControllers_throw(const Reference< XModel2 >& _rxDocument,
        ::std::list< Reference< XController2 > >& _out_rControllers )
    {
        _out_rControllers.clear();
        Reference< XEnumeration > xControllerEnum( _rxDocument->getControllers(), UNO_SET_THROW );
        while ( xControllerEnum->hasMoreElements() )
            _out_rControllers.push_back( Reference< XController2 >( xControllerEnum->nextElement(), UNO_QUERY_THROW ) );
    }

    //====================================================================
    //= MacroMigrationDialog_Data
    //====================================================================
    struct MacroMigrationDialog_Data
    {
        ::comphelper::ComponentContext          aContext;
        MigrationLog                            aLogger;
        Reference< XOfficeDatabaseDocument >    xDocument;
        Reference< XModel2 >                    xDocumentModel;
        ::rtl::OUString                         sSuccessfulBackupLocation;
        bool                                    bMigrationIsRunning;
        bool                                    bMigrationFailure;
        bool                                    bMigrationSuccess;

        MacroMigrationDialog_Data(
                const ::comphelper::ComponentContext& _rContext,
                const Reference< XOfficeDatabaseDocument >& _rxDocument )
            :aContext( _rContext )
            ,aLogger()
            ,xDocument( _rxDocument )
            ,xDocumentModel( _rxDocument, UNO_QUERY )
            ,bMigrationIsRunning( false )
            ,bMigrationFailure( false )
            ,bMigrationSuccess( false )
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

        OSL_PRECOND( m_pData->xDocumentModel.is(), "MacroMigrationDialog::MacroMigrationDialog: illegal document!" );
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
    short MacroMigrationDialog::Execute()
    {
        short nResult = MacroMigrationDialog_Base::Execute();
        if ( !m_pData->bMigrationFailure && !m_pData->bMigrationSuccess )
            // migration did not even start
            return nResult;

        OSL_ENSURE( !m_pData->bMigrationFailure || !m_pData->bMigrationSuccess,
            "MacroMigrationDialog::Execute: success *and* failure at the same time?!" );
        impl_reloadDocument_nothrow( m_pData->bMigrationSuccess );

        return nResult;
    }

    //--------------------------------------------------------------------
    sal_Bool MacroMigrationDialog::Close()
    {
        if ( m_pData->bMigrationIsRunning )
            return sal_False;
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

            // start the migration asynchronously
            PostUserEvent( LINK( this, MacroMigrationDialog, OnStartMigration ) );
        }
        break;

        case STATE_SUMMARY:
            // disable the previous step - we can't return to the actual migration, it already happened (or failed)
            enableState( STATE_MIGRATE, false );
            updateTravelUI();

            // display the results
            dynamic_cast< ResultPage& >( *GetPage( STATE_SUMMARY ) ).displayMigrationLog(
                m_pData->bMigrationSuccess, m_pData->aLogger.getCompleteLog() );

            enableButtons( WZB_FINISH, m_pData->bMigrationSuccess );
            enableButtons( WZB_CANCEL, m_pData->bMigrationFailure );
            defaultButton( m_pData->bMigrationSuccess ? WZB_FINISH : WZB_CANCEL );
            break;

        default:
            OSL_FAIL( "MacroMigrationDialog::enterState: unhandled state!" );
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
            OSL_FAIL( "MacroMigrationDialog::prepareLeaveCurrentState: unhandled state!" );
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
    sal_Bool MacroMigrationDialog::onFinish()
    {
        return MacroMigrationDialog_Base::onFinish();
    }

    //--------------------------------------------------------------------
    IMPL_LINK( MacroMigrationDialog, OnStartMigration, void*, /*_pNotInterestedIn*/ )
    {
        // prevent closing
        m_pData->bMigrationIsRunning = true;

        // initialize migration engine and progress
        ProgressPage& rProgressPage( dynamic_cast< ProgressPage& >( *GetPage( STATE_MIGRATE ) ) );
        MigrationEngine aEngine( m_pData->aContext, m_pData->xDocument, rProgressPage, m_pData->aLogger );
        rProgressPage.setDocumentCounts( aEngine.getFormCount(), aEngine.getReportCount() );

        // do the migration
        m_pData->bMigrationSuccess = aEngine.migrateAll();
        m_pData->bMigrationFailure = !m_pData->bMigrationSuccess;

        // re-enable the UI
        enableButtons( WZB_FINISH | WZB_NEXT, true );
        enableState( STATE_SUMMARY, true );
        updateTravelUI();

        m_pData->bMigrationIsRunning = false;

        if ( m_pData->bMigrationSuccess )
        {
            rProgressPage.onFinishedSuccessfully();
        }
        else
        {   // if there was an error, show the summary automatically
            travelNext();
        }

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
            // collect all controllers of our document
            ::std::list< Reference< XController2 > > aControllers;
            lcl_getControllers_throw( m_pData->xDocumentModel, aControllers );

            // close all sub documents of all controllers
            for (   ::std::list< Reference< XController2 > >::const_iterator pos = aControllers.begin();
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
    namespace
    {
        bool    lcl_equalURLs_nothrow(
            const Reference< XComponentContext >& context,
            const ::rtl::OUString& _lhs, const ::rtl::OUString _rhs )
        {
            // the cheap situation: the URLs are equal
            if ( _lhs == _rhs )
                return true;

            bool bEqual = true;
            try
            {
                ::ucbhelper::Content aContentLHS = ::ucbhelper::Content( _lhs, Reference< XCommandEnvironment >(), context );
                ::ucbhelper::Content aContentRHS = ::ucbhelper::Content( _rhs, Reference< XCommandEnvironment >(), context );
                Reference< XContent > xContentLHS( aContentLHS.get(), UNO_SET_THROW );
                Reference< XContent > xContentRHS( aContentRHS.get(), UNO_SET_THROW );
                Reference< XContentIdentifier > xID1( xContentLHS->getIdentifier(), UNO_SET_THROW );
                Reference< XContentIdentifier > xID2( xContentRHS->getIdentifier(), UNO_SET_THROW );

                bEqual = UniversalContentBroker::create(context)->compareContentIds( xID1, xID2 ) == 0;
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return bEqual;
        }
    }

    //--------------------------------------------------------------------
    bool MacroMigrationDialog::impl_backupDocument_nothrow() const
    {
        if ( !m_pData->xDocumentModel.is() )
            // should never happen, but has been reported as assertion before
            return false;

        SaveDBDocPage& rBackupPage = dynamic_cast< SaveDBDocPage& >( *GetPage( STATE_BACKUP_DBDOC ) );
        ::rtl::OUString sBackupLocation( rBackupPage.getBackupLocation() );

        Any aError;
        try
        {
            // check that the backup location isn't the same as the document itself
            if ( lcl_equalURLs_nothrow( m_pData->aContext.getUNOContext(), sBackupLocation, m_pData->xDocumentModel->getURL() ) )
            {
                ErrorBox aErrorBox( const_cast< MacroMigrationDialog* >( this ), MacroMigrationResId( ERR_INVALID_BACKUP_LOCATION ) );
                aErrorBox.Execute();
                rBackupPage.grabLocationFocus();
                return false;
            }

            // store to the backup location
            const Reference< XStorable > xDocument( getDocument(), UNO_QUERY_THROW );
            xDocument->storeToURL( sBackupLocation, Sequence< PropertyValue >() );
            m_pData->sSuccessfulBackupLocation = sBackupLocation;
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
        InteractionHandler aHandler( m_pData->aContext, m_pData->xDocumentModel.get() );
        aHandler.reportError( aError );

        m_pData->aLogger.logFailure( MigrationError(
            ERR_DOCUMENT_BACKUP_FAILED,
            sBackupLocation,
            aError
        ) );

        return false;
    }

    //--------------------------------------------------------------------
    void MacroMigrationDialog::impl_reloadDocument_nothrow( bool _bMigrationSuccess )
    {
        typedef ::std::pair< Reference< XFrame >, ::rtl::OUString > ViewDescriptor;
        ::std::list< ViewDescriptor > aViews;

        try
        {
            // the information which is necessary to reload the document
            ::rtl::OUString                     sDocumentURL ( m_pData->xDocumentModel->getURL()  );
            ::comphelper::NamedValueCollection  aDocumentArgs( m_pData->xDocumentModel->getArgs() );
            if ( !_bMigrationSuccess )
            {
                // if the migration was not successful, then reload from the backup
                aDocumentArgs.put( "SalvagedFile", m_pData->sSuccessfulBackupLocation );
                // reset the modified flag of the document, so the controller can be suspended later
                Reference< XModifiable > xModify( m_pData->xDocument, UNO_QUERY_THROW );
                xModify->setModified( sal_False );
                // after this reload, don't show the migration warning, again
                aDocumentArgs.put( "SuppressMigrationWarning", sal_Bool(sal_True) );
            }

            // remove anything from the args which might refer to the old document
            aDocumentArgs.remove( "Model" );
            aDocumentArgs.remove( "Stream" );
            aDocumentArgs.remove( "InputStream" );
            aDocumentArgs.remove( "FileName" );
            aDocumentArgs.remove( "URL" );

            // collect all controllers of our document
            ::std::list< Reference< XController2 > > aControllers;
            lcl_getControllers_throw( m_pData->xDocumentModel, aControllers );

            // close all those controllers
            while ( !aControllers.empty() )
            {
                Reference< XController2 > xController( aControllers.front(), UNO_SET_THROW );
                aControllers.pop_front();

                Reference< XFrame > xFrame( xController->getFrame(), UNO_SET_THROW );
                ::rtl::OUString sViewName( xController->getViewControllerName() );

                if ( !xController->suspend( sal_True ) )
                {   // ouch. There shouldn't be any modal dialogs and such, so there
                    // really is no reason why suspending shouldn't work.
                    OSL_FAIL( "MacroMigrationDialog::impl_reloadDocument_nothrow: could not suspend a controller!" );
                    // ignoring this would be at the cost of a crash (potentially)
                    // so, we cannot continue here.
                    throw CloseVetoException();
                }

                aViews.push_back( ViewDescriptor( xFrame, sViewName ) );
                xFrame->setComponent( NULL, NULL );
                xController->dispose();
            }

            // Note the document is closed now - disconnecting the last controller
            // closes it automatically.

            Reference< XOfficeDatabaseDocument > xNewDocument;

            // re-create the views
            while ( !aViews.empty() )
            {
                ViewDescriptor aView( aViews.front() );
                aViews.pop_front();

                // load the document into this frame
                Reference< XComponentLoader > xLoader( aView.first, UNO_QUERY_THROW );
                aDocumentArgs.put( "ViewName", aView.second );
                Reference< XInterface > xReloaded( xLoader->loadComponentFromURL(
                    sDocumentURL,
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_self" ) ),
                    0,
                    aDocumentArgs.getPropertyValues()
                ) );

                OSL_ENSURE( xReloaded != m_pData->xDocumentModel,
                    "MacroMigrationDialog::impl_reloadDocument_nothrow: this should have been a new instance!" );
                    // this would be unexpected, but recoverable: The loader should at least have done
                    // this: really *load* the document, even if it loaded it into the old document instance
                if ( !xNewDocument.is() )
                {
                    xNewDocument.set( xReloaded, UNO_QUERY_THROW );
                    // for subsequent loads, into different frames, put the document into the load args
                    aDocumentArgs.put( "Model", xNewDocument );
                }
                #if OSL_DEBUG_LEVEL > 0
                else
                {
                    OSL_ENSURE( xNewDocument == xReloaded,
                        "MacroMigrationDialog::impl_reloadDocument_nothrow: unexpected: subsequent load attempt returned a wrong document!" );
                }
                #endif
            }

            m_pData->xDocument = xNewDocument;
            m_pData->xDocumentModel.set( xNewDocument, UNO_QUERY );

            // finally, now that the document has been reloaded - if the migration was not successful,
            // then it was reloaded from the backup, but the real document still is broken. So, save
            // the document once, which will write the content loaded from the backup to the real docfile.
            if ( !_bMigrationSuccess )
            {
                Reference< XModifiable > xModify( m_pData->xDocument, UNO_QUERY_THROW );
                xModify->setModified( sal_True );
                    // this is just parnoia - in case saving the doc fails, perhaps the user is tempted to do so
                Reference< XStorable > xStor( m_pData->xDocument, UNO_QUERY_THROW );
                xStor->store();
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        // close all frames from aViews - the respective controllers have been closed, but
        // reloading didn't work, so the frames are zombies now.
        while ( !aViews.empty() )
        {
            ViewDescriptor aView( aViews.front() );
            aViews.pop_front();
            try
            {
                Reference< XCloseable > xFrameClose( aView.first, UNO_QUERY_THROW );
                xFrameClose->close( sal_True );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

//........................................................................
} // namespace dbmm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
