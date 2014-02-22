/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

namespace dbmm
{

#define STATE_CLOSE_SUB_DOCS    0
#define STATE_BACKUP_DBDOC      1
#define STATE_MIGRATE           2
#define STATE_SUMMARY           3

#define PATH_DEFAULT            1

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

    
    static void lcl_getControllers_throw(const Reference< XModel2 >& _rxDocument,
        ::std::list< Reference< XController2 > >& _out_rControllers )
    {
        _out_rControllers.clear();
        Reference< XEnumeration > xControllerEnum( _rxDocument->getControllers(), UNO_SET_THROW );
        while ( xControllerEnum->hasMoreElements() )
            _out_rControllers.push_back( Reference< XController2 >( xControllerEnum->nextElement(), UNO_QUERY_THROW ) );
    }

    
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

    
    MacroMigrationDialog::MacroMigrationDialog( Window* _pParent, const Reference<XComponentContext>& _rContext,
        const Reference< XOfficeDatabaseDocument >& _rxDocument )
        :MacroMigrationDialog_Base( _pParent, MacroMigrationResId( DLG_MACRO_MIGRATION ) )
        ,m_pData( new MacroMigrationDialog_Data( _rContext, _rxDocument ) )
    {
        OUString sTitlePrepare( MacroMigrationResId( STR_STATE_CLOSE_SUB_DOCS ) );
        OUString sTitleStoreAs( MacroMigrationResId( STR_STATE_BACKUP_DBDOC ) );
        OUString sTitleMigrate( MacroMigrationResId( STR_STATE_MIGRATE ) );
        OUString sTitleSummary( MacroMigrationResId( STR_STATE_SUMMARY ) );
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

    MacroMigrationDialog::~MacroMigrationDialog()
    {
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
            enableButtons( WZB_FINISH, false );
            enableState( STATE_MIGRATE, false );
            enableState( STATE_SUMMARY, false );
            break;

        case STATE_BACKUP_DBDOC:
            enableState( STATE_MIGRATE, true );
            
            
            
            break;

        case STATE_MIGRATE:
        {
            
            
            enableState( STATE_CLOSE_SUB_DOCS, false );
            enableState( STATE_BACKUP_DBDOC, false );
            enableState( STATE_SUMMARY, false );

            enableButtons( WZB_FINISH | WZB_CANCEL | WZB_PREVIOUS | WZB_NEXT, false );

            
            PostUserEvent( LINK( this, MacroMigrationDialog, OnStartMigration ) );
        }
        break;

        case STATE_SUMMARY:
            
            enableState( STATE_MIGRATE, false );
            updateTravelUI();

            
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

    sal_Bool MacroMigrationDialog::leaveState( WizardState _nState )
    {
        return MacroMigrationDialog_Base::leaveState( _nState );
    }

    MacroMigrationDialog::WizardState MacroMigrationDialog::determineNextState( WizardState _nCurrentState ) const
    {
        return MacroMigrationDialog_Base::determineNextState( _nCurrentState );
    }

    sal_Bool MacroMigrationDialog::onFinish()
    {
        return MacroMigrationDialog_Base::onFinish();
    }

    IMPL_LINK( MacroMigrationDialog, OnStartMigration, void*, /*_pNotInterestedIn*/ )
    {
        
        m_pData->bMigrationIsRunning = true;

        
        ProgressPage& rProgressPage( dynamic_cast< ProgressPage& >( *GetPage( STATE_MIGRATE ) ) );
        MigrationEngine aEngine( m_pData->aContext, m_pData->xDocument, rProgressPage, m_pData->aLogger );
        rProgressPage.setDocumentCounts( aEngine.getFormCount(), aEngine.getReportCount() );

        
        m_pData->bMigrationSuccess = aEngine.migrateAll();
        m_pData->bMigrationFailure = !m_pData->bMigrationSuccess;

        
        enableButtons( WZB_FINISH | WZB_NEXT, true );
        enableState( STATE_SUMMARY, true );
        updateTravelUI();

        m_pData->bMigrationIsRunning = false;

        if ( m_pData->bMigrationSuccess )
        {
            rProgressPage.onFinishedSuccessfully();
        }
        else
        {   
            travelNext();
        }

        
        return 0L;
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
            
            ::std::list< Reference< XController2 > > aControllers;
            lcl_getControllers_throw( m_pData->xDocumentModel, aControllers );

            
            for (   ::std::list< Reference< XController2 > >::const_iterator pos = aControllers.begin();
                    pos != aControllers.end() && bSuccess;
                    ++pos
                )
            {
                Reference< XDatabaseDocumentUI > xController( *pos, UNO_QUERY );
                OSL_ENSURE( xController.is(), "MacroMigrationDialog::impl_closeSubDocs_nothrow: unexpected: controller is missing an important interface!" );
                    
                    
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

    namespace
    {
        bool    lcl_equalURLs_nothrow(
            const Reference< XComponentContext >& context,
            const OUString& _lhs, const OUString _rhs )
        {
            
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

    bool MacroMigrationDialog::impl_backupDocument_nothrow() const
    {
        if ( !m_pData->xDocumentModel.is() )
            
            return false;

        SaveDBDocPage& rBackupPage = dynamic_cast< SaveDBDocPage& >( *GetPage( STATE_BACKUP_DBDOC ) );
        OUString sBackupLocation( rBackupPage.getBackupLocation() );

        Any aError;
        try
        {
            
            if ( lcl_equalURLs_nothrow( m_pData->aContext, sBackupLocation, m_pData->xDocumentModel->getURL() ) )
            {
                ErrorBox aErrorBox( const_cast< MacroMigrationDialog* >( this ), MacroMigrationResId( ERR_INVALID_BACKUP_LOCATION ) );
                aErrorBox.Execute();
                rBackupPage.grabLocationFocus();
                return false;
            }

            
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
        typedef ::std::pair< Reference< XFrame >, OUString > ViewDescriptor;
        ::std::list< ViewDescriptor > aViews;

        try
        {
            
            OUString                            sDocumentURL ( m_pData->xDocumentModel->getURL()  );
            ::comphelper::NamedValueCollection  aDocumentArgs( m_pData->xDocumentModel->getArgs() );
            if ( !_bMigrationSuccess )
            {
                
                aDocumentArgs.put( "SalvagedFile", m_pData->sSuccessfulBackupLocation );
                
                Reference< XModifiable > xModify( m_pData->xDocument, UNO_QUERY_THROW );
                xModify->setModified( sal_False );
                
                aDocumentArgs.put( "SuppressMigrationWarning", sal_Bool(sal_True) );
            }

            
            aDocumentArgs.remove( "Model" );
            aDocumentArgs.remove( "Stream" );
            aDocumentArgs.remove( "InputStream" );
            aDocumentArgs.remove( "FileName" );
            aDocumentArgs.remove( "URL" );

            
            ::std::list< Reference< XController2 > > aControllers;
            lcl_getControllers_throw( m_pData->xDocumentModel, aControllers );

            
            while ( !aControllers.empty() )
            {
                Reference< XController2 > xController( aControllers.front(), UNO_SET_THROW );
                aControllers.pop_front();

                Reference< XFrame > xFrame( xController->getFrame(), UNO_SET_THROW );
                OUString sViewName( xController->getViewControllerName() );

                if ( !xController->suspend( sal_True ) )
                {   
                    
                    OSL_FAIL( "MacroMigrationDialog::impl_reloadDocument_nothrow: could not suspend a controller!" );
                    
                    
                    throw CloseVetoException();
                }

                aViews.push_back( ViewDescriptor( xFrame, sViewName ) );
                xFrame->setComponent( NULL, NULL );
                xController->dispose();
            }

            
            

            Reference< XOfficeDatabaseDocument > xNewDocument;

            
            while ( !aViews.empty() )
            {
                ViewDescriptor aView( aViews.front() );
                aViews.pop_front();

                
                Reference< XComponentLoader > xLoader( aView.first, UNO_QUERY_THROW );
                aDocumentArgs.put( "ViewName", aView.second );
                Reference< XInterface > xReloaded( xLoader->loadComponentFromURL(
                    sDocumentURL,
                    OUString( "_self" ),
                    0,
                    aDocumentArgs.getPropertyValues()
                ) );

                OSL_ENSURE( xReloaded != m_pData->xDocumentModel,
                    "MacroMigrationDialog::impl_reloadDocument_nothrow: this should have been a new instance!" );
                    
                    
                if ( !xNewDocument.is() )
                {
                    xNewDocument.set( xReloaded, UNO_QUERY_THROW );
                    
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

            
            
            
            if ( !_bMigrationSuccess )
            {
                Reference< XModifiable > xModify( m_pData->xDocument, UNO_QUERY_THROW );
                xModify->setModified( sal_True );
                    
                Reference< XStorable > xStor( m_pData->xDocument, UNO_QUERY_THROW );
                xStor->store();
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        
        
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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
