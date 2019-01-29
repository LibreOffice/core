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

#include "docinteraction.hxx"
#include <core_resource.hxx>
#include <strings.hrc>
#include "macromigrationdialog.hxx"
#include "macromigrationpages.hxx"
#include "migrationengine.hxx"
#include "migrationerror.hxx"
#include "migrationlog.hxx"

#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/XContent.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ref.hxx>
#include <svl/filenotation.hxx>
#include <tools/diagnose_ex.h>
#include <ucbhelper/content.hxx>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>

#include <vector>

namespace dbmm
{

#define STATE_CLOSE_SUB_DOCS    0
#define STATE_BACKUP_DBDOC      1
#define STATE_MIGRATE           2
#define STATE_SUMMARY           3

#define PATH_DEFAULT            1

// height and width of tab pages
#define TAB_PAGE_WIDTH  280
#define TAB_PAGE_HEIGHT 185

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::sdb::application::XDatabaseDocumentUI;
    using ::com::sun::star::sdb::XOfficeDatabaseDocument;
    using ::com::sun::star::frame::XModel2;
    using ::com::sun::star::frame::XController2;
    using ::com::sun::star::container::XEnumeration;
    using ::com::sun::star::frame::XStorable;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::util::XCloseable;
    using ::com::sun::star::util::CloseVetoException;
    using ::com::sun::star::frame::XComponentLoader;
    using ::com::sun::star::util::XModifiable;
    using ::com::sun::star::ucb::UniversalContentBroker;
    using ::com::sun::star::ucb::XCommandEnvironment;
    using ::com::sun::star::ucb::XContent;
    using ::com::sun::star::ucb::XContentIdentifier;

    // helper
    static void lcl_getControllers_throw(const Reference< XModel2 >& _rxDocument,
        std::vector< Reference< XController2 > >& _out_rControllers )
    {
        _out_rControllers.clear();
        Reference< XEnumeration > xControllerEnum( _rxDocument->getControllers(), UNO_SET_THROW );
        while ( xControllerEnum->hasMoreElements() )
            _out_rControllers.emplace_back( xControllerEnum->nextElement(), UNO_QUERY_THROW );
    }

    // MacroMigrationDialog_Data
    struct MacroMigrationDialog_Data
    {
        Reference<XComponentContext>          aContext;
        MigrationLog                            aLogger;
        Reference< XOfficeDatabaseDocument >    xDocument;
        Reference< XModel2 >                    xDocumentModel;
        OUString                         sSuccessfulBackupLocation;
        bool                                    bMigrationIsRunning;
        bool                                    bMigrationFailure;
        bool                                    bMigrationSuccess;

        MacroMigrationDialog_Data(
                const Reference<XComponentContext>& _rContext,
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

    // MacroMigrationDialog
    MacroMigrationDialog::MacroMigrationDialog(vcl::Window* _pParent, const Reference<XComponentContext>& _rContext,
        const Reference< XOfficeDatabaseDocument >& _rxDocument)
        : MacroMigrationDialog_Base(_pParent)
        , m_pData( new MacroMigrationDialog_Data( _rContext, _rxDocument ) )
    {
        OUString sTitlePrepare( DBA_RES( STR_STATE_CLOSE_SUB_DOCS ) );
        OUString sTitleStoreAs( DBA_RES( STR_STATE_BACKUP_DBDOC ) );
        OUString sTitleMigrate( DBA_RES( STR_STATE_MIGRATE ) );
        OUString sTitleSummary( DBA_RES( STR_STATE_SUMMARY ) );

        describeState( STATE_CLOSE_SUB_DOCS,    sTitlePrepare, &PreparationPage::Create   );
        describeState( STATE_BACKUP_DBDOC,      sTitleStoreAs, &SaveDBDocPage::Create     );
        describeState( STATE_MIGRATE,           sTitleMigrate, &ProgressPage::Create      );
        describeState( STATE_SUMMARY,           sTitleSummary, &ResultPage::Create        );

        declarePath( PATH_DEFAULT, {STATE_CLOSE_SUB_DOCS, STATE_BACKUP_DBDOC, STATE_MIGRATE, STATE_SUMMARY} );

        SetPageSizePixel(LogicToPixel(::Size(TAB_PAGE_WIDTH, TAB_PAGE_HEIGHT), MapMode(MapUnit::MapAppFont)));
        SetRoadmapInteractive( true );
        enableAutomaticNextButtonState();
        defaultButton( WizardButtonFlags::NEXT );
        enableButtons( WizardButtonFlags::FINISH, true );
        ActivatePage();

        OSL_PRECOND( m_pData->xDocumentModel.is(), "MacroMigrationDialog::MacroMigrationDialog: illegal document!" );
    }

    const Reference<XComponentContext>& MacroMigrationDialog::getComponentContext() const
    {
        return m_pData->aContext;
    }

    const Reference< XOfficeDatabaseDocument >& MacroMigrationDialog::getDocument() const
    {
        return m_pData->xDocument;
    }

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

    bool MacroMigrationDialog::Close()
    {
        if ( m_pData->bMigrationIsRunning )
            return false;
        return MacroMigrationDialog_Base::Close();
    }

    void MacroMigrationDialog::enterState( WizardState _nState )
    {
        MacroMigrationDialog_Base::enterState( _nState );

        switch ( _nState )
        {
        case STATE_CLOSE_SUB_DOCS:
            enableButtons( WizardButtonFlags::FINISH, false );
            enableState( STATE_MIGRATE, false );
            enableState( STATE_SUMMARY, false );
            break;

        case STATE_BACKUP_DBDOC:
            enableState( STATE_MIGRATE );
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

            enableButtons( WizardButtonFlags::FINISH | WizardButtonFlags::CANCEL | WizardButtonFlags::PREVIOUS | WizardButtonFlags::NEXT, false );

            // start the migration asynchronously
            PostUserEvent( LINK( this, MacroMigrationDialog, OnStartMigration ), nullptr, true );
        }
        break;

        case STATE_SUMMARY:
            // disable the previous step - we can't return to the actual migration, it already happened (or failed)
            enableState( STATE_MIGRATE, false );
            updateTravelUI();

            // display the results
            dynamic_cast< ResultPage& >( *GetPage( STATE_SUMMARY ) ).displayMigrationLog(
                m_pData->bMigrationSuccess, m_pData->aLogger.getCompleteLog() );

            enableButtons( WizardButtonFlags::FINISH, m_pData->bMigrationSuccess );
            enableButtons( WizardButtonFlags::CANCEL, m_pData->bMigrationFailure );
            defaultButton( m_pData->bMigrationSuccess ? WizardButtonFlags::FINISH : WizardButtonFlags::CANCEL );
            break;

        default:
            OSL_FAIL( "MacroMigrationDialog::enterState: unhandled state!" );
        }
    }

    bool MacroMigrationDialog::prepareLeaveCurrentState( CommitPageReason _eReason )
    {
        if ( !MacroMigrationDialog_Base::prepareLeaveCurrentState( _eReason ) )
            return false;

        switch ( getCurrentState() )
        {
        case STATE_CLOSE_SUB_DOCS:
            if ( !impl_closeSubDocs_nothrow() )
                return false;
            break;
        case STATE_BACKUP_DBDOC:
            if ( !impl_backupDocument_nothrow() )
                return false;
            break;
        case STATE_MIGRATE:
            break;
        case STATE_SUMMARY:
            break;
        default:
            OSL_FAIL( "MacroMigrationDialog::prepareLeaveCurrentState: unhandled state!" );
        }

        return true;
    }

    IMPL_LINK_NOARG( MacroMigrationDialog, OnStartMigration, void*, void )
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
        enableButtons( WizardButtonFlags::FINISH | WizardButtonFlags::NEXT, true );
        enableState( STATE_SUMMARY );
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
    }

    void MacroMigrationDialog::impl_showCloseDocsError( bool _bShow )
    {
        PreparationPage* pPreparationPage = dynamic_cast< PreparationPage* >( GetPage( STATE_CLOSE_SUB_DOCS ) );
        OSL_ENSURE( pPreparationPage, "MacroMigrationDialog::impl_showCloseDocsError: did not find the page!" );
        if ( pPreparationPage )
            pPreparationPage->showCloseDocsError( _bShow );
    }

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
            std::vector< Reference< XController2 > > aControllers;
            lcl_getControllers_throw( m_pData->xDocumentModel, aControllers );

            // close all sub documents of all controllers
            for (auto const& controller : aControllers)
            {
                Reference< XDatabaseDocumentUI > xController( controller, UNO_QUERY );
                OSL_ENSURE( xController.is(), "MacroMigrationDialog::impl_closeSubDocs_nothrow: unexpected: controller is missing an important interface!" );
                    // at the moment, only one implementation for a DBDoc's controller exists, which should
                    // support this interface
                if ( !xController.is() )
                    continue;

                bSuccess = xController->closeSubComponents();
                if (!bSuccess)
                    break;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
            bSuccess = false;
        }

        impl_showCloseDocsError( !bSuccess );
        return bSuccess;
    }

    namespace
    {
        bool    lcl_equalURLs_nothrow(
            const Reference< XComponentContext >& context,
            const OUString& _lhs, const OUString& _rhs )
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
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
            return bEqual;
        }
    }

    bool MacroMigrationDialog::impl_backupDocument_nothrow() const
    {
        if ( !m_pData->xDocumentModel.is() )
            // should never happen, but has been reported as assertion before
            return false;

        SaveDBDocPage& rBackupPage = dynamic_cast< SaveDBDocPage& >( *GetPage( STATE_BACKUP_DBDOC ) );
        OUString sBackupLocation( rBackupPage.getBackupLocation() );

        Any aError;
        try
        {
            // check that the backup location isn't the same as the document itself
            if ( lcl_equalURLs_nothrow( m_pData->aContext, sBackupLocation, m_pData->xDocumentModel->getURL() ) )
            {
                std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                               VclMessageType::Warning, VclButtonsType::Ok, DBA_RES(STR_INVALID_BACKUP_LOCATION)));
                xErrorBox->run();
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

    void MacroMigrationDialog::impl_reloadDocument_nothrow( bool _bMigrationSuccess )
    {
        typedef std::pair< Reference< XFrame >, OUString > ViewDescriptor;
        std::vector< ViewDescriptor > aViews;

        try
        {
            // the information which is necessary to reload the document
            OUString                            sDocumentURL ( m_pData->xDocumentModel->getURL()  );
            ::comphelper::NamedValueCollection  aDocumentArgs( m_pData->xDocumentModel->getArgs() );
            if ( !_bMigrationSuccess )
            {
                // if the migration was not successful, then reload from the backup
                aDocumentArgs.put( "SalvagedFile", m_pData->sSuccessfulBackupLocation );
                // reset the modified flag of the document, so the controller can be suspended later
                Reference< XModifiable > xModify( m_pData->xDocument, UNO_QUERY_THROW );
                xModify->setModified( false );
                // after this reload, don't show the migration warning, again
                aDocumentArgs.put( "SuppressMigrationWarning", true );
            }

            // remove anything from the args which might refer to the old document
            aDocumentArgs.remove( "Model" );
            aDocumentArgs.remove( "Stream" );
            aDocumentArgs.remove( "InputStream" );
            aDocumentArgs.remove( "FileName" );
            aDocumentArgs.remove( "URL" );

            // collect all controllers of our document
            std::vector< Reference< XController2 > > aControllers;
            lcl_getControllers_throw( m_pData->xDocumentModel, aControllers );

            // close all those controllers
            for (auto const& controller : aControllers)
            {
                Reference< XController2 > xController( controller, UNO_SET_THROW );

                Reference< XFrame > xFrame( xController->getFrame(), UNO_SET_THROW );
                OUString sViewName( xController->getViewControllerName() );

                if ( !xController->suspend( true ) )
                {   // ouch. There shouldn't be any modal dialogs and such, so there
                    // really is no reason why suspending shouldn't work.
                    OSL_FAIL( "MacroMigrationDialog::impl_reloadDocument_nothrow: could not suspend a controller!" );
                    // ignoring this would be at the cost of a crash (potentially)
                    // so, we cannot continue here.
                    throw CloseVetoException();
                }

                aViews.emplace_back( xFrame, sViewName );
                xFrame->setComponent( nullptr, nullptr );
                xController->dispose();
            }
            aControllers.clear();

            // Note the document is closed now - disconnecting the last controller
            // closes it automatically.

            Reference< XOfficeDatabaseDocument > xNewDocument;

            // re-create the views
            for (auto const& view : aViews)
            {
                // load the document into this frame
                Reference< XComponentLoader > xLoader( view.first, UNO_QUERY_THROW );
                aDocumentArgs.put( "ViewName", view.second );
                Reference< XInterface > xReloaded( xLoader->loadComponentFromURL(
                    sDocumentURL,
                    "_self",
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
            aViews.clear();

            m_pData->xDocument = xNewDocument;
            m_pData->xDocumentModel.set( xNewDocument, UNO_QUERY );

            // finally, now that the document has been reloaded - if the migration was not successful,
            // then it was reloaded from the backup, but the real document still is broken. So, save
            // the document once, which will write the content loaded from the backup to the real docfile.
            if ( !_bMigrationSuccess )
            {
                Reference< XModifiable > xModify( m_pData->xDocument, UNO_QUERY_THROW );
                xModify->setModified( true );
                    // this is just paranoia - in case saving the doc fails, perhaps the user is tempted to do so
                Reference< XStorable > xStor( m_pData->xDocument, UNO_QUERY_THROW );
                xStor->store();
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }

        // close all frames from aViews - the respective controllers have been closed, but
        // reloading didn't work, so the frames are zombies now.
        for (auto const& view : aViews)
        {
            try
            {
                Reference< XCloseable > xFrameClose( view.first, UNO_QUERY_THROW );
                xFrameClose->close( true );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
        }
        aViews.clear();
    }

} // namespace dbmm

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
