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


#include "updatehdl.hxx"
#include <helpids.h>

#include <osl/diagnose.h>
#include <osl/thread.hxx>
#include <osl/file.hxx>
#include <rtl/ustring.hxx>
#include <rtl/bootstrap.hxx>

#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/style/VerticalAlignment.hpp>

#include <com/sun/star/awt/ActionEvent.hpp>
#include <com/sun/star/awt/PushButtonType.hpp>
#include <com/sun/star/awt/UnoControlDialog.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XMessageBox.hpp>
#include <com/sun/star/awt/XAnimation.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XVclContainer.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindow2.hpp>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/configuration/theDefaultProvider.hpp>

#include <com/sun/star/container/XNameContainer.hpp>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/TerminationVetoException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/InteractionRequestStringResolver.hpp>

#include <strings.hrc>
#include <unotools/resmgr.hxx>
#include <tools/urlobj.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#define COMMAND_CLOSE       "close"

#define CTRL_THROBBER       "throbber"
#define CTRL_PROGRESS       "progress"

#define TEXT_STATUS         "text_status"
#define TEXT_PERCENT        "text_percent"
#define TEXT_DESCRIPTION    "text_description"

#define FIXED_LINE_MODEL    "com.sun.star.awt.UnoControlFixedLineModel"
#define FIXED_TEXT_MODEL    "com.sun.star.awt.UnoControlFixedTextModel"
#define EDIT_FIELD_MODEL    "com.sun.star.awt.UnoControlEditModel"
#define BUTTON_MODEL        "com.sun.star.awt.UnoControlButtonModel"
#define GROUP_BOX_MODEL     "com.sun.star.awt.UnoControlGroupBoxModel"

using namespace com::sun::star;


UpdateHandler::UpdateHandler( const uno::Reference< uno::XComponentContext > & rxContext,
                              const rtl::Reference< IActionListener > & rxActionListener ) :
    mxContext( rxContext ),
    mxActionListener( rxActionListener ),
    meCurState( UPDATESTATES_COUNT ),
    meLastState( UPDATESTATES_COUNT ),
    mnPercent( 0 ),
    mnLastCtrlState( -1 ),
    mbDownloadBtnHasDots( false ),
    mbVisible( false ),
    mbStringsLoaded( false ),
    mbMinimized( false ),
    mbListenerAdded(false),
    mbShowsMessageBox(false)
{
}


UpdateHandler::~UpdateHandler()
{
    mxContext = nullptr;
    mxUpdDlg = nullptr;
    mxInteractionHdl = nullptr;
    mxActionListener = nullptr;
}


void UpdateHandler::enableControls( short nCtrlState )
{
    osl::MutexGuard aGuard( maMutex );

    if ( nCtrlState == mnLastCtrlState )
        return;

    // the help button should always be the last button in the
    // enum list and must never be disabled
    for ( int i=0; i<HELP_BUTTON; i++ )
    {
        short nCurStateVal = static_cast<short>(nCtrlState >> i);
        short nOldStateVal = static_cast<short>(mnLastCtrlState >> i);
        if ( ( nCurStateVal & 0x01 ) != ( nOldStateVal & 0x01 ) )
        {
            bool bEnableControl = ( ( nCurStateVal & 0x01 ) == 0x01 );
            setControlProperty( msButtonIDs[i], "Enabled", uno::Any( bEnableControl ) );
        }
    }

    mnLastCtrlState = nCtrlState;
}


void UpdateHandler::setDownloadBtnLabel( bool bAppendDots )
{
    osl::MutexGuard aGuard( maMutex );

    if ( mbDownloadBtnHasDots != bAppendDots )
    {
        OUString aLabel( msDownload );

        if ( bAppendDots )
            aLabel += "...";

        setControlProperty( msButtonIDs[DOWNLOAD_BUTTON], "Label", uno::Any( aLabel ) );
        setControlProperty( msButtonIDs[DOWNLOAD_BUTTON], "HelpURL", uno::makeAny<OUString>( INET_HID_SCHEME HID_CHECK_FOR_UPD_DOWNLOAD2 ) );

        mbDownloadBtnHasDots = bAppendDots;
    }
}


void UpdateHandler::setState( UpdateState eState )
{
    osl::MutexGuard aGuard( maMutex );

    meCurState = eState;

    if ( mxUpdDlg.is() && mbVisible )
        updateState( meCurState );
}


bool UpdateHandler::isVisible() const
{
    if ( !mxUpdDlg.is() ) return false;

    uno::Reference< awt::XWindow2 > xWindow( mxUpdDlg, uno::UNO_QUERY );

    if ( xWindow.is() )
        return xWindow->isVisible();
    else
        return false;
}


void UpdateHandler::setVisible( bool bVisible )
{
    osl::MutexGuard aGuard( maMutex );

    mbVisible = bVisible;

    if ( bVisible )
    {
        if ( !mxUpdDlg.is() )
            createDialog();

        // this should never happen, but if it happens we better return here
        if ( !mxUpdDlg.is() )
            return;

        updateState( meCurState );

        uno::Reference< awt::XWindow > xWindow( mxUpdDlg, uno::UNO_QUERY );

        if ( xWindow.is() )
            xWindow->setVisible( bVisible );

        uno::Reference< awt::XTopWindow > xTopWindow( mxUpdDlg, uno::UNO_QUERY );
        if ( xTopWindow.is() )
        {
            xTopWindow->toFront();
            if ( !mbListenerAdded )
            {
                xTopWindow->addTopWindowListener( this );
                mbListenerAdded = true;
            }
        }
    }
    else if ( mxUpdDlg.is() )
    {
        uno::Reference< awt::XWindow > xWindow( mxUpdDlg, uno::UNO_QUERY );

        if ( xWindow.is() )
            xWindow->setVisible( bVisible );
    }
}


void UpdateHandler::setProgress( sal_Int32 nPercent )
{
    if ( nPercent > 100 )
        nPercent = 100;
    else if ( nPercent < 0 )
        nPercent = 0;

    if ( nPercent != mnPercent )
    {
        osl::MutexGuard aGuard( maMutex );

        mnPercent = nPercent;
        setControlProperty( CTRL_PROGRESS, "ProgressValue", uno::Any( nPercent ) );
        setControlProperty( TEXT_PERCENT, "Text", uno::Any( substVariables(msPercent) ) );
    }
}


void UpdateHandler::setErrorMessage( const OUString& rErrorMsg )
{
    setControlProperty( TEXT_DESCRIPTION, "Text", uno::Any( rErrorMsg ) );
}


void UpdateHandler::setDownloadFile( const OUString& rFilePath )
{
    sal_Int32 nLast = rFilePath.lastIndexOf( '/' );
    if ( nLast != -1 )
    {
        msDownloadFile = rFilePath.copy( nLast+1 );
        const OUString aDownloadURL = rFilePath.copy( 0, nLast );
        osl::FileBase::getSystemPathFromFileURL( aDownloadURL, msDownloadPath );
    }
}


OUString UpdateHandler::getBubbleText( UpdateState eState )
{
    osl::MutexGuard aGuard( maMutex );

    OUString sText;
    sal_Int32 nIndex = static_cast<sal_Int32>(eState);

    loadStrings();

    if ( ( UPDATESTATE_UPDATE_AVAIL <= nIndex ) && ( nIndex < UPDATESTATES_COUNT ) )
        sText = substVariables( msBubbleTexts[ nIndex - UPDATESTATE_UPDATE_AVAIL ] );

    return sText;
}


OUString UpdateHandler::getBubbleTitle( UpdateState eState )
{
    osl::MutexGuard aGuard( maMutex );

    OUString sText;
    sal_Int32 nIndex = static_cast<sal_Int32>(eState);

    loadStrings();

    if ( ( UPDATESTATE_UPDATE_AVAIL <= nIndex ) && ( nIndex < UPDATESTATES_COUNT ) )
        sText = substVariables( msBubbleTitles[ nIndex - UPDATESTATE_UPDATE_AVAIL] );

    return sText;
}


OUString UpdateHandler::getDefaultInstErrMsg()
{
    osl::MutexGuard aGuard( maMutex );

    loadStrings();

    return substVariables( msInstallError );
}

// XActionListener

void SAL_CALL UpdateHandler::disposing( const lang::EventObject& rEvt )
{
    if ( rEvt.Source == mxUpdDlg )
        mxUpdDlg.clear();
}


void SAL_CALL UpdateHandler::actionPerformed( awt::ActionEvent const & rEvent )
{
    DialogControls eButton = BUTTON_COUNT;
    for ( int i = 0; i < BUTTON_COUNT; i++ )
    {
        if ( rEvent.ActionCommand == msButtonIDs[i] )
        {
            eButton = static_cast<DialogControls>(i);
            break;
        }
    }

    if ( rEvent.ActionCommand == COMMAND_CLOSE )
    {
        if ( ( mnLastCtrlState & ( 1 << CLOSE_BUTTON ) ) == ( 1 << CLOSE_BUTTON ) )
            eButton = CLOSE_BUTTON;
        else
            eButton = CANCEL_BUTTON;
    }

    switch ( eButton ) {
        case CANCEL_BUTTON:
        {
            bool bCancel = true;

            if ( ( meCurState == UPDATESTATE_DOWNLOADING ) ||
                 ( meCurState == UPDATESTATE_DOWNLOAD_PAUSED ) ||
                 ( meCurState == UPDATESTATE_ERROR_DOWNLOADING ) )
                bCancel = showWarning( msCancelMessage );

            if ( bCancel )
            {
                mxActionListener->cancel();
                setVisible( false );
            }
            break;
        }
        case CLOSE_BUTTON:
            setVisible( false );
            if ( meCurState == UPDATESTATE_ERROR_CHECKING )
                mxActionListener->closeAfterFailure();
            break;
        case DOWNLOAD_BUTTON:
            mxActionListener->download();
            break;
        case INSTALL_BUTTON:
            if ( showWarning( msInstallMessage ) )
                mxActionListener->install();
            break;
        case PAUSE_BUTTON:
            mxActionListener->pause();
            break;
        case RESUME_BUTTON:
            mxActionListener->resume();
            break;
        case HELP_BUTTON:
            break;
        default:
            OSL_FAIL( "UpdateHandler::actionPerformed: unknown command!" );
    }
}

// XTopWindowListener

void SAL_CALL UpdateHandler::windowOpened( const lang::EventObject& )
{
}


void SAL_CALL UpdateHandler::windowClosing( const lang::EventObject& e )
{
    awt::ActionEvent aActionEvt;
    aActionEvt.ActionCommand = COMMAND_CLOSE;
    aActionEvt.Source = e.Source;

    actionPerformed( aActionEvt );
}


void SAL_CALL UpdateHandler::windowClosed( const lang::EventObject& )
{
}


void SAL_CALL UpdateHandler::windowMinimized( const lang::EventObject& )
{
    mbMinimized = true;
}


void SAL_CALL UpdateHandler::windowNormalized( const lang::EventObject& )
{
    mbMinimized = false;
}


void SAL_CALL UpdateHandler::windowActivated( const lang::EventObject& )
{
}


void SAL_CALL UpdateHandler::windowDeactivated( const lang::EventObject& )
{
}

// XInteractionHandler

void SAL_CALL UpdateHandler::handle( uno::Reference< task::XInteractionRequest > const & rRequest)
{
    if ( !mxInteractionHdl.is() )
    {
        if( !mxContext.is() )
            throw uno::RuntimeException( "UpdateHandler:: empty component context", *this );

        uno::Reference< lang::XMultiComponentFactory > xServiceManager(mxContext->getServiceManager());

        if( !xServiceManager.is() )
            throw uno::RuntimeException( "UpdateHandler: unable to obtain service manager from component context", *this );

        mxInteractionHdl.set(
            task::InteractionHandler::createWithParent(mxContext, nullptr),
            uno::UNO_QUERY_THROW);
    }
    uno::Reference< task::XInteractionRequestStringResolver > xStrResolver =
            task::InteractionRequestStringResolver::create( mxContext );
    beans::Optional< OUString > aErrorText = xStrResolver->getStringFromInformationalRequest( rRequest );
    if ( aErrorText.IsPresent )
    {
        setControlProperty( TEXT_DESCRIPTION, "Text", uno::Any( aErrorText.Value ) );

        uno::Sequence< uno::Reference< task::XInteractionContinuation > > xContinuations = rRequest->getContinuations();
        if ( xContinuations.getLength() == 1 )
        {
            if ( meCurState == UPDATESTATE_CHECKING )
                setState( UPDATESTATE_ERROR_CHECKING );
            else if ( meCurState == UPDATESTATE_DOWNLOADING )
                setState( UPDATESTATE_ERROR_DOWNLOADING );

            xContinuations[0]->select();
        }
        else
            mxInteractionHdl->handle( rRequest );
    }
    else
        mxInteractionHdl->handle( rRequest );
}


// XTerminateListener

void SAL_CALL UpdateHandler::queryTermination( const lang::EventObject& )
{
    if ( mbShowsMessageBox )
    {
        uno::Reference< awt::XTopWindow > xTopWindow( mxUpdDlg, uno::UNO_QUERY );
        if ( xTopWindow.is() )
            xTopWindow->toFront();

        throw frame::TerminationVetoException(
            "The office cannot be closed while displaying a warning!",
            static_cast<frame::XTerminateListener*>(this));
    }
    else
        setVisible( false );
}


void SAL_CALL UpdateHandler::notifyTermination( const lang::EventObject& )
{
    osl::MutexGuard aGuard( maMutex );

    if ( mxUpdDlg.is() )
    {
        uno::Reference< awt::XTopWindow > xTopWindow( mxUpdDlg, uno::UNO_QUERY );
        if ( xTopWindow.is() )
            xTopWindow->removeTopWindowListener( this );

        uno::Reference< lang::XComponent > xComponent( mxUpdDlg, uno::UNO_QUERY );
        if ( xComponent.is() )
            xComponent->dispose();

        mxUpdDlg.clear();
    }
}


void UpdateHandler::updateState( UpdateState eState )
{
    if ( meLastState == eState )
        return;

    OUString sText;

    switch ( eState )
    {
        case UPDATESTATE_CHECKING:
            showControls( (1<<CANCEL_BUTTON) + (1<<THROBBER_CTRL) );
            enableControls( 1<<CANCEL_BUTTON );
            setControlProperty( TEXT_STATUS, "Text", uno::Any( substVariables(msChecking) ) );
            setControlProperty( TEXT_DESCRIPTION, "Text", uno::Any( OUString() ) );
            focusControl( CANCEL_BUTTON );
            break;
        case UPDATESTATE_ERROR_CHECKING:
            showControls( 0 );
            enableControls( 1 << CLOSE_BUTTON );
            setControlProperty( TEXT_STATUS, "Text", uno::Any( substVariables(msCheckingError) ) );
            focusControl( CLOSE_BUTTON );
            break;
        case UPDATESTATE_UPDATE_AVAIL:
            showControls( 0 );
            enableControls( ( 1 << CLOSE_BUTTON )  + ( 1 << DOWNLOAD_BUTTON ) );
            setControlProperty( TEXT_STATUS, "Text", uno::Any( substVariables(msUpdFound) ) );

            sText = substVariables(msDownloadWarning);
            if ( !msDescriptionMsg.isEmpty() )
                sText += "\n\n" + msDescriptionMsg;
            setControlProperty( TEXT_DESCRIPTION, "Text", uno::Any( sText ) );

            setDownloadBtnLabel( false );
            focusControl( DOWNLOAD_BUTTON );
            break;
        case UPDATESTATE_UPDATE_NO_DOWNLOAD:
            showControls( 0 );
            enableControls( ( 1 << CLOSE_BUTTON )  + ( 1 << DOWNLOAD_BUTTON ) );
            setControlProperty( TEXT_STATUS, "Text", uno::Any( substVariables(msUpdFound) ) );

            sText = substVariables(msDownloadNotAvail);
            if ( !msDescriptionMsg.isEmpty() )
                sText += "\n\n" + msDescriptionMsg;
            setControlProperty( TEXT_DESCRIPTION, "Text", uno::Any( sText ) );

            setDownloadBtnLabel( true );
            focusControl( DOWNLOAD_BUTTON );
            break;
        case UPDATESTATE_NO_UPDATE_AVAIL:
        case UPDATESTATE_EXT_UPD_AVAIL:     // will only be set, when there are no office updates avail
            showControls( 0 );
            enableControls( 1 << CLOSE_BUTTON );
            setControlProperty( TEXT_STATUS, "Text", uno::Any( substVariables(msNoUpdFound) ) );
            setControlProperty( TEXT_DESCRIPTION, "Text", uno::Any( OUString() ) );
            focusControl( CLOSE_BUTTON );
            break;
        case UPDATESTATE_DOWNLOADING:
            showControls( (1<<PROGRESS_CTRL) + (1<<CANCEL_BUTTON) + (1<<PAUSE_BUTTON) + (1<<RESUME_BUTTON) );
            enableControls( (1<<CLOSE_BUTTON) + (1<<CANCEL_BUTTON) + (1<<PAUSE_BUTTON) );
            setControlProperty( TEXT_STATUS, "Text", uno::Any( substVariables(msDownloading) ) );
            setControlProperty( TEXT_PERCENT, "Text", uno::Any( substVariables(msPercent) ) );
            setControlProperty( TEXT_DESCRIPTION, "Text", uno::Any( substVariables(msDownloadWarning) ) );
            setControlProperty( CTRL_PROGRESS, "ProgressValue", uno::Any( mnPercent ) );
            focusControl( CLOSE_BUTTON );
            break;
        case UPDATESTATE_DOWNLOAD_PAUSED:
            showControls( (1<<PROGRESS_CTRL) + (1<<CANCEL_BUTTON) + (1<<PAUSE_BUTTON) + (1<<RESUME_BUTTON) );
            enableControls( (1<<CLOSE_BUTTON) + (1<<CANCEL_BUTTON) + (1<<RESUME_BUTTON) );
            setControlProperty( TEXT_STATUS, "Text", uno::Any( substVariables(msDownloadPause) ) );
            setControlProperty( TEXT_PERCENT, "Text", uno::Any( substVariables(msPercent) ) );
            setControlProperty( TEXT_DESCRIPTION, "Text", uno::Any( substVariables(msDownloadWarning) ) );
            setControlProperty( CTRL_PROGRESS, "ProgressValue", uno::Any( mnPercent ) );
            focusControl( CLOSE_BUTTON );
            break;
        case UPDATESTATE_ERROR_DOWNLOADING:
            showControls( (1<<PROGRESS_CTRL) + (1<<CANCEL_BUTTON) + (1<<PAUSE_BUTTON) + (1<<RESUME_BUTTON) );
            enableControls( (1<<CLOSE_BUTTON) + (1<<CANCEL_BUTTON) );
            setControlProperty( TEXT_STATUS, "Text", uno::Any( substVariables(msDownloadError) ) );
            focusControl( CLOSE_BUTTON );
            break;
        case UPDATESTATE_DOWNLOAD_AVAIL:
            showControls( 0 );
            enableControls( (1<<CLOSE_BUTTON) + (1<<INSTALL_BUTTON) );
            setControlProperty( TEXT_STATUS, "Text", uno::Any( substVariables(msReady2Install) ) );
            setControlProperty( TEXT_DESCRIPTION, "Text", uno::Any( substVariables(msDownloadDescr) ) );
            focusControl( INSTALL_BUTTON );
            break;
        case UPDATESTATE_AUTO_START:
        case UPDATESTATES_COUNT:
            //do nothing, only count!
            break;
    }

    meLastState = eState;
}

OUString UpdateHandler::loadString(const std::locale& rLocale,
                                   const char* pResourceId)
{
    return Translate::get(pResourceId, rLocale);
}

OUString UpdateHandler::substVariables( const OUString &rSource ) const
{
    return rSource
        .replaceAll( "%NEXTVERSION", msNextVersion )
        .replaceAll( "%DOWNLOAD_PATH", msDownloadPath )
        .replaceAll( "%FILE_NAME", msDownloadFile )
        .replaceAll( "%PERCENT", OUString::number( mnPercent ) );
}

void UpdateHandler::loadStrings()
{
    if ( mbStringsLoaded )
        return;
    else
        mbStringsLoaded = true;

    std::locale loc = Translate::Create("pcr");

    msChecking      = loadString( loc, RID_UPDATE_STR_CHECKING );
    msCheckingError = loadString( loc, RID_UPDATE_STR_CHECKING_ERR );
    msNoUpdFound    = loadString( loc, RID_UPDATE_STR_NO_UPD_FOUND );

    msUpdFound      = loadString( loc, RID_UPDATE_STR_UPD_FOUND );
    setFullVersion( msUpdFound );

    msDlgTitle      = loadString( loc, RID_UPDATE_STR_DLG_TITLE );
    msDownloadPause = loadString( loc, RID_UPDATE_STR_DOWNLOAD_PAUSE );
    msDownloadError = loadString( loc, RID_UPDATE_STR_DOWNLOAD_ERR );
    msDownloadWarning = loadString( loc, RID_UPDATE_STR_DOWNLOAD_WARN );
    msDownloadDescr =  loadString( loc, RID_UPDATE_STR_DOWNLOAD_DESCR );
    msDownloadNotAvail = loadString( loc, RID_UPDATE_STR_DOWNLOAD_UNAVAIL );
    msDownloading   = loadString( loc, RID_UPDATE_STR_DOWNLOADING );
    msReady2Install = loadString( loc, RID_UPDATE_STR_READY_INSTALL );
    msCancelMessage = loadString( loc, RID_UPDATE_STR_CANCEL_DOWNLOAD );
    msInstallMessage = loadString( loc, RID_UPDATE_STR_BEGIN_INSTALL );
    msInstallError  = loadString( loc, RID_UPDATE_STR_INSTALL_ERROR );
    msOverwriteWarning = loadString( loc, RID_UPDATE_STR_OVERWRITE_WARNING );
    msPercent       = loadString( loc, RID_UPDATE_STR_PERCENT );
    msReloadWarning = loadString( loc, RID_UPDATE_STR_RELOAD_WARNING );
    msReloadReload  = loadString( loc, RID_UPDATE_STR_RELOAD_RELOAD );
    msReloadContinue = loadString( loc, RID_UPDATE_STR_RELOAD_CONTINUE );

    msStatusFL      = loadString( loc, RID_UPDATE_FT_STATUS );
    msDescription   = loadString( loc, RID_UPDATE_FT_DESCRIPTION );

    msClose         = loadString( loc, RID_UPDATE_BTN_CLOSE );
    msDownload      = loadString( loc, RID_UPDATE_BTN_DOWNLOAD );
    msInstall       = loadString( loc, RID_UPDATE_BTN_INSTALL );
    msPauseBtn      = loadString( loc, RID_UPDATE_BTN_PAUSE );
    msResumeBtn     = loadString( loc, RID_UPDATE_BTN_RESUME );
    msCancelBtn     = loadString( loc, RID_UPDATE_BTN_CANCEL );

    std::pair<const char*, const char*> RID_UPDATE_BUBBLE[] =
    {
        { RID_UPDATE_BUBBLE_UPDATE_AVAIL, RID_UPDATE_BUBBLE_T_UPDATE_AVAIL },
        { RID_UPDATE_BUBBLE_UPDATE_NO_DOWN, RID_UPDATE_BUBBLE_T_UPDATE_NO_DOWN },
        { RID_UPDATE_BUBBLE_AUTO_START, RID_UPDATE_BUBBLE_T_AUTO_START },
        { RID_UPDATE_BUBBLE_DOWNLOADING, RID_UPDATE_BUBBLE_T_DOWNLOADING },
        { RID_UPDATE_BUBBLE_DOWNLOAD_PAUSED, RID_UPDATE_BUBBLE_T_DOWNLOAD_PAUSED },
        { RID_UPDATE_BUBBLE_ERROR_DOWNLOADING, RID_UPDATE_BUBBLE_T_ERROR_DOWNLOADING },
        { RID_UPDATE_BUBBLE_DOWNLOAD_AVAIL, RID_UPDATE_BUBBLE_T_DOWNLOAD_AVAIL },
        { RID_UPDATE_BUBBLE_EXT_UPD_AVAIL, RID_UPDATE_BUBBLE_T_EXT_UPD_AVAIL }
    };

    static_assert(SAL_N_ELEMENTS(RID_UPDATE_BUBBLE) == UPDATESTATES_COUNT - UPDATESTATE_UPDATE_AVAIL, "mismatch");

    // all update states before UPDATESTATE_UPDATE_AVAIL don't have a bubble
    // so we can ignore them
    for (size_t i = 0; i < SAL_N_ELEMENTS(RID_UPDATE_BUBBLE); ++i)
    {
        msBubbleTexts[i] = loadString(loc, RID_UPDATE_BUBBLE[i].first);
        msBubbleTitles[i] = loadString(loc, RID_UPDATE_BUBBLE[i].second);
    }

    for ( int i=0; i < BUTTON_COUNT; i++ )
    {
        msButtonIDs[ i ] = "BUTTON_" + OUString::number( i );
    }
}


void UpdateHandler::startThrobber( bool bStart )
{
    uno::Reference< awt::XControlContainer > xContainer( mxUpdDlg, uno::UNO_QUERY );
    uno::Reference< awt::XAnimation > xThrobber( xContainer->getControl( CTRL_THROBBER ), uno::UNO_QUERY );

    if ( xThrobber.is() )
    {
        if ( bStart )
            xThrobber->startAnimation();
        else
            xThrobber->stopAnimation();
    }

    uno::Reference< awt::XWindow > xWindow( xContainer->getControl( CTRL_THROBBER ), uno::UNO_QUERY );
    if (xWindow.is() )
        xWindow->setVisible( bStart );
}


void UpdateHandler::setControlProperty( const OUString &rCtrlName,
                                        const OUString &rPropName,
                                        const uno::Any &rPropValue )
{
    if ( !mxUpdDlg.is() ) return;

    uno::Reference< awt::XControlContainer > xContainer( mxUpdDlg, uno::UNO_QUERY );
    uno::Reference< awt::XControl > xControl( xContainer->getControl( rCtrlName ), uno::UNO_QUERY_THROW );
    uno::Reference< awt::XControlModel > xControlModel( xControl->getModel(), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY_THROW );

    try {
        xPropSet->setPropertyValue( rPropName, rPropValue );
    }
    catch( const beans::UnknownPropertyException& )
    {
        OSL_FAIL( "UpdateHandler::setControlProperty: caught an exception!" );
    }
}


void UpdateHandler::showControl( const OUString &rCtrlName, bool bShow )
{
    uno::Reference< awt::XControlContainer > xContainer( mxUpdDlg, uno::UNO_QUERY );

    if ( !xContainer.is() )
    {
        OSL_FAIL( "UpdateHandler::showControl: could not get control container!" );
        return;
    }

    uno::Reference< awt::XWindow > xWindow( xContainer->getControl( rCtrlName ), uno::UNO_QUERY );
    if ( xWindow.is() )
        xWindow->setVisible( bShow );
}


void UpdateHandler::focusControl( DialogControls eID )
{
    uno::Reference< awt::XControlContainer > xContainer( mxUpdDlg, uno::UNO_QUERY );

    if ( !xContainer.is() )
    {
        OSL_FAIL( "UpdateHandler::focusControl: could not get control container!" );
        return;
    }

    OSL_ENSURE( (eID < BUTTON_COUNT), "UpdateHandler::focusControl: id to big!" );

    uno::Reference< awt::XWindow > xWindow( xContainer->getControl( msButtonIDs[static_cast<short>(eID)] ), uno::UNO_QUERY );
    if ( xWindow.is() )
        xWindow->setFocus();
}


void UpdateHandler::insertControlModel( uno::Reference< awt::XControlModel > const & rxDialogModel,
                                        OUString const & rServiceName,
                                        OUString const & rControlName,
                                        awt::Rectangle const & rPosSize,
                                        uno::Sequence< beans::NamedValue > const & rProps )
{
    uno::Reference< lang::XMultiServiceFactory > xFactory (rxDialogModel, uno::UNO_QUERY_THROW);
    uno::Reference< awt::XControlModel > xModel (xFactory->createInstance (rServiceName), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xPropSet (xModel, uno::UNO_QUERY_THROW);

    for (sal_Int32 i = 0, n = rProps.getLength(); i < n; i++)
    {
        xPropSet->setPropertyValue (rProps[i].Name, rProps[i].Value);
    }

    // @see awt/UnoControlDialogElement.idl
    xPropSet->setPropertyValue( "Name", uno::Any (rControlName) );
    xPropSet->setPropertyValue( "PositionX", uno::Any (rPosSize.X) );
    xPropSet->setPropertyValue( "PositionY", uno::Any (rPosSize.Y) );
    xPropSet->setPropertyValue( "Height", uno::Any (rPosSize.Height) );
    xPropSet->setPropertyValue( "Width", uno::Any (rPosSize.Width) );

    // insert by Name into DialogModel container
    uno::Reference< container::XNameContainer > xContainer (rxDialogModel, uno::UNO_QUERY_THROW);
    xContainer->insertByName( rControlName, uno::Any (uno::Reference< uno::XInterface >(xModel, uno::UNO_QUERY)));
}


void UpdateHandler::setFullVersion( OUString& rString )
{
    uno::Reference< lang::XMultiServiceFactory > xConfigurationProvider(
        css::configuration::theDefaultProvider::get( mxContext ) );

    beans::PropertyValue aProperty;
    aProperty.Name  = "nodepath";
    aProperty.Value <<= OUString("org.openoffice.Setup/Product");

    uno::Sequence< uno::Any > aArgumentList( 1 );
    aArgumentList[0] <<= aProperty;

    uno::Reference< uno::XInterface > xConfigAccess;
    xConfigAccess = xConfigurationProvider->createInstanceWithArguments( "com.sun.star.configuration.ConfigurationAccess",
                                                                         aArgumentList );

    uno::Reference< container::XNameAccess > xNameAccess( xConfigAccess, uno::UNO_QUERY_THROW );

    OUString aProductVersion;
    xNameAccess->getByName("ooSetupVersion") >>= aProductVersion;
    OUString aProductFullVersion;
    xNameAccess->getByName("ooSetupVersionAboutBox") >>= aProductFullVersion;
    rString = rString.replaceFirst( aProductVersion, aProductFullVersion );
}


bool UpdateHandler::showWarning( const OUString &rWarningText ) const
{
    bool bRet = false;

    uno::Reference< awt::XControl > xControl( mxUpdDlg, uno::UNO_QUERY );
    if ( !xControl.is() ) return bRet;

    uno::Reference< awt::XWindowPeer > xPeer = xControl->getPeer();
    if ( !xPeer.is() ) return bRet;

    uno::Reference< awt::XToolkit > xToolkit = xPeer->getToolkit();
    if ( !xToolkit.is() ) return bRet;

    awt::WindowDescriptor aDescriptor;

    sal_Int32 nWindowAttributes = awt::WindowAttribute::BORDER | awt::WindowAttribute::MOVEABLE | awt::WindowAttribute::CLOSEABLE;
    nWindowAttributes |= awt::VclWindowPeerAttribute::YES_NO;
    nWindowAttributes |= awt::VclWindowPeerAttribute::DEF_NO;

    aDescriptor.Type              = awt::WindowClass_MODALTOP;
    aDescriptor.WindowServiceName = "warningbox";
    aDescriptor.ParentIndex       = -1;
    aDescriptor.Parent            = xPeer;
    aDescriptor.Bounds            = awt::Rectangle( 10, 10, 250, 150 );
    aDescriptor.WindowAttributes  = nWindowAttributes;

    uno::Reference< awt::XMessageBox > xMsgBox( xToolkit->createWindow( aDescriptor ), uno::UNO_QUERY );
    if ( xMsgBox.is() )
    {
        mbShowsMessageBox = true;
        sal_Int16 nRet;
        // xMsgBox->setCaptionText( msCancelTitle );
        xMsgBox->setMessageText( rWarningText );
        nRet = xMsgBox->execute();
        if ( nRet == 2 ) // RET_YES == 2
            bRet = true;
        mbShowsMessageBox = false;
    }

    uno::Reference< lang::XComponent > xComponent( xMsgBox, uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();

    return bRet;
}


bool UpdateHandler::showWarning( const OUString &rWarningText,
                                 const OUString &rBtnText_1,
                                 const OUString &rBtnText_2 ) const
{
    bool bRet = false;

    uno::Reference< awt::XControl > xControl( mxUpdDlg, uno::UNO_QUERY );
    if ( !xControl.is() ) return bRet;

    uno::Reference< awt::XWindowPeer > xPeer = xControl->getPeer();
    if ( !xPeer.is() ) return bRet;

    uno::Reference< awt::XToolkit > xToolkit = xPeer->getToolkit();
    if ( !xToolkit.is() ) return bRet;

    awt::WindowDescriptor aDescriptor;

    sal_Int32 nWindowAttributes = awt::WindowAttribute::BORDER | awt::WindowAttribute::MOVEABLE | awt::WindowAttribute::CLOSEABLE;
    nWindowAttributes |= awt::VclWindowPeerAttribute::YES_NO;
    nWindowAttributes |= awt::VclWindowPeerAttribute::DEF_NO;

    aDescriptor.Type              = awt::WindowClass_MODALTOP;
    aDescriptor.WindowServiceName = "warningbox";
    aDescriptor.ParentIndex       = -1;
    aDescriptor.Parent            = xPeer;
    aDescriptor.Bounds            = awt::Rectangle( 10, 10, 250, 150 );
    aDescriptor.WindowAttributes  = nWindowAttributes;

    uno::Reference< awt::XMessageBox > xMsgBox( xToolkit->createWindow( aDescriptor ), uno::UNO_QUERY );
    if ( xMsgBox.is() )
    {
        uno::Reference< awt::XVclContainer > xMsgBoxCtrls( xMsgBox, uno::UNO_QUERY );
        if ( xMsgBoxCtrls.is() )
        {
            uno::Sequence< uno::Reference< awt::XWindow > > xChildren = xMsgBoxCtrls->getWindows();

            for ( long i=0; i < xChildren.getLength(); i++ )
            {
                uno::Reference< awt::XVclWindowPeer > xMsgBoxCtrl( xChildren[i], uno::UNO_QUERY );
                if ( xMsgBoxCtrl.is() )
                {
                    bool bIsDefault = true;
                    uno::Any aValue = xMsgBoxCtrl->getProperty( "DefaultButton" );
                    aValue >>= bIsDefault;
                    if ( bIsDefault )
                        xMsgBoxCtrl->setProperty( "Text", uno::Any( rBtnText_1 ) );
                    else
                        xMsgBoxCtrl->setProperty( "Text", uno::Any( rBtnText_2 ) );
                }
            }
        }

        sal_Int16 nRet;
        // xMsgBox->setCaptionText( msCancelTitle );
        mbShowsMessageBox = true;
        xMsgBox->setMessageText( rWarningText );
        nRet = xMsgBox->execute();
        if ( nRet == 2 ) // RET_YES == 2
            bRet = true;

        mbShowsMessageBox = false;
    }

    uno::Reference< lang::XComponent > xComponent( xMsgBox, uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();

    return bRet;
}


bool UpdateHandler::showOverwriteWarning( const OUString& rFileName ) const
{
    return showWarning(
        (msReloadWarning
         .replaceAll( "%FILENAME", rFileName )
         .replaceAll( "%DOWNLOAD_PATH", msDownloadPath )),
        msReloadContinue, msReloadReload );
}


bool UpdateHandler::showOverwriteWarning() const
{
    return showWarning( msOverwriteWarning );
}


#define BUTTON_HEIGHT       14
#define BUTTON_WIDTH        50
#define BUTTON_X_OFFSET      7
#define BUTTON_Y_OFFSET      3
#define LABEL_HEIGHT        10

#define DIALOG_WIDTH       300
#define DIALOG_BORDER        5
#define INNER_BORDER         3
#define TEXT_OFFSET          1
#define BOX_HEIGHT1          ( LABEL_HEIGHT + 3*BUTTON_HEIGHT + 2*BUTTON_Y_OFFSET + 2*INNER_BORDER )
#define BOX_HEIGHT2         50
#define EDIT_WIDTH          ( DIALOG_WIDTH - 2 * DIALOG_BORDER )
#define BOX1_BTN_X          ( DIALOG_BORDER + EDIT_WIDTH - BUTTON_WIDTH - INNER_BORDER )
#define BOX1_BTN_Y          ( DIALOG_BORDER + LABEL_HEIGHT + INNER_BORDER)
#define THROBBER_WIDTH      16
#define THROBBER_HEIGHT     16
#define THROBBER_X_POS      ( DIALOG_BORDER + 8 )
#define THROBBER_Y_POS      ( DIALOG_BORDER + 23 )
#define BUTTON_BAR_HEIGHT   24
#define LABEL_OFFSET        ( LABEL_HEIGHT + 4 )
#define DIALOG_HEIGHT       ( BOX_HEIGHT1 + BOX_HEIGHT2 + LABEL_OFFSET + BUTTON_BAR_HEIGHT + 3 * DIALOG_BORDER )
#define LABEL_Y_POS         ( 2 * DIALOG_BORDER + BOX_HEIGHT1 )
#define EDIT2_Y_POS         ( LABEL_Y_POS + LABEL_HEIGHT )
#define BUTTON_BAR_Y_POS    ( EDIT2_Y_POS + DIALOG_BORDER + BOX_HEIGHT2 )
#define BUTTON_Y_POS        ( BUTTON_BAR_Y_POS + 8 )
#define CLOSE_BTN_X         ( DIALOG_WIDTH - DIALOG_BORDER - BUTTON_WIDTH )
#define INSTALL_BTN_X       ( CLOSE_BTN_X - 2 * BUTTON_X_OFFSET - BUTTON_WIDTH )
#define DOWNLOAD_BTN_X      ( INSTALL_BTN_X - BUTTON_X_OFFSET - BUTTON_WIDTH )
#define PROGRESS_WIDTH      80
#define PROGRESS_HEIGHT     10
#define PROGRESS_X_POS      ( DIALOG_BORDER + 8 )
#define PROGRESS_Y_POS      ( DIALOG_BORDER + 2*LABEL_OFFSET )


void UpdateHandler::showControls( short nControls )
{
    // The buttons from CANCEL_BUTTON to RESUME_BUTTON will be shown or
    // hidden on demand
    short nShiftMe;
    for ( int i = 0; i <= int(RESUME_BUTTON); i++ )
    {
        nShiftMe = static_cast<short>(nControls >> i);
        showControl( msButtonIDs[i], static_cast<bool>(nShiftMe & 0x01) );
    }

    nShiftMe = static_cast<short>(nControls >> THROBBER_CTRL);
    startThrobber( static_cast<bool>(nShiftMe & 0x01) );

    nShiftMe = static_cast<short>(nControls >> PROGRESS_CTRL);
    showControl( CTRL_PROGRESS, static_cast<bool>(nShiftMe & 0x01) );
    showControl( TEXT_PERCENT, static_cast<bool>(nShiftMe & 0x01) );

    // Status text needs to be smaller, when there are buttons at the right side of the dialog
    if ( ( nControls & ( (1<<CANCEL_BUTTON) + (1<<PAUSE_BUTTON) + (1<<RESUME_BUTTON) ) )  != 0 )
        setControlProperty( TEXT_STATUS, "Width", uno::Any( sal_Int32(EDIT_WIDTH - BUTTON_WIDTH - 2*INNER_BORDER - TEXT_OFFSET ) ) );
    else
        setControlProperty( TEXT_STATUS, "Width", uno::Any( sal_Int32(EDIT_WIDTH - 2*TEXT_OFFSET ) ) );

    // Status text needs to be taller, when we show the progress bar
    if ( ( nControls & ( 1<<PROGRESS_CTRL ) ) != 0 )
        setControlProperty( TEXT_STATUS, "Height", uno::Any( sal_Int32(LABEL_HEIGHT) ) );
    else
        setControlProperty( TEXT_STATUS, "Height", uno::Any( sal_Int32(BOX_HEIGHT1 - 4*TEXT_OFFSET - LABEL_HEIGHT ) ) );
}


void UpdateHandler::createDialog()
{
    if ( !mxContext.is() )
    {
        OSL_ASSERT( false );
        return;
    }

    if( mxContext.is() )
    {
        uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create( mxContext );
        xDesktop->addTerminateListener( this );
    }

    loadStrings();

    uno::Reference< lang::XMultiComponentFactory > xFactory( mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference< awt::XControlModel > xControlModel( xFactory->createInstanceWithContext(
                                                         "com.sun.star.awt.UnoControlDialogModel",
                                                         mxContext), uno::UNO_QUERY_THROW );
    {
        // @see awt/UnoControlDialogModel.idl
        uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY_THROW );

        xPropSet->setPropertyValue( "Title", uno::Any( msDlgTitle ) );
        xPropSet->setPropertyValue( "Closeable", uno::Any( true ) );
        xPropSet->setPropertyValue( "Enabled", uno::Any( true ) );
        xPropSet->setPropertyValue( "Moveable", uno::Any( true ) );
        xPropSet->setPropertyValue( "Sizeable", uno::Any( true ) );
        xPropSet->setPropertyValue( "DesktopAsParent", uno::Any( true ) );
        xPropSet->setPropertyValue( "PositionX", uno::Any(sal_Int32( 100 )) );
        xPropSet->setPropertyValue( "PositionY", uno::Any(sal_Int32( 100 )) );
        xPropSet->setPropertyValue( "Width", uno::Any(sal_Int32( DIALOG_WIDTH )) );
        xPropSet->setPropertyValue( "Height", uno::Any(sal_Int32( DIALOG_HEIGHT )) );
        xPropSet->setPropertyValue( "HelpURL", uno::makeAny<OUString>( INET_HID_SCHEME HID_CHECK_FOR_UPD_DLG ) );
    }
    {   // Label (fixed text) <status>
        uno::Sequence< beans::NamedValue > aProps { { "Label", uno::Any( msStatusFL ) } };

        insertControlModel( xControlModel, FIXED_TEXT_MODEL, "fixedLineStatus",
                            awt::Rectangle( DIALOG_BORDER+1, DIALOG_BORDER, EDIT_WIDTH-2, LABEL_HEIGHT ),
                            aProps );
    }
    {   // box around <status> text
        uno::Sequence< beans::NamedValue > aProps;

        insertControlModel( xControlModel, GROUP_BOX_MODEL, "StatusBox",
                            awt::Rectangle( DIALOG_BORDER, DIALOG_BORDER + LABEL_HEIGHT, EDIT_WIDTH, BOX_HEIGHT1 - LABEL_HEIGHT ),
                            aProps );
    }
    {   // Text (multiline edit) <status>
        uno::Sequence< beans::NamedValue > aProps
        {
            { "Text", uno::Any( substVariables(msChecking) ) },
            { "Border", uno::Any( sal_Int16( 0 ) ) },
            { "PaintTransparent", uno::Any( true ) },
            { "MultiLine", uno::Any( true ) },
            { "ReadOnly", uno::Any( true ) },
            { "AutoVScroll", uno::Any( true ) },
            { "HelpURL", uno::makeAny<OUString>( INET_HID_SCHEME HID_CHECK_FOR_UPD_STATUS ) }
        };

        insertControlModel( xControlModel, EDIT_FIELD_MODEL, TEXT_STATUS,
                            awt::Rectangle( DIALOG_BORDER + TEXT_OFFSET,
                                            DIALOG_BORDER + LABEL_HEIGHT + TEXT_OFFSET,
                                            EDIT_WIDTH - 2*TEXT_OFFSET,
                                            BOX_HEIGHT1 - 4*TEXT_OFFSET - LABEL_HEIGHT ),
                            aProps );
    }
    {   // Text (edit) <percent>
        uno::Sequence< beans::NamedValue > aProps
        {
            { "Text", uno::Any( msPercent ) },
            { "Border", uno::Any( sal_Int16( 0 ) ) },
            { "PaintTransparent", uno::Any( true ) },
            { "ReadOnly", uno::Any( true ) },
        };

        insertControlModel( xControlModel, EDIT_FIELD_MODEL, TEXT_PERCENT,
                            awt::Rectangle( PROGRESS_X_POS + PROGRESS_WIDTH + DIALOG_BORDER,
                                            PROGRESS_Y_POS,
                                            EDIT_WIDTH - PROGRESS_WIDTH - BUTTON_WIDTH - 2*DIALOG_BORDER,
                                            LABEL_HEIGHT ),
                            aProps );
    }
    {   // pause button
        uno::Sequence< beans::NamedValue > aProps
        {
            { "DefaultButton", uno::Any( false ) },
            { "Enabled", uno::Any( true ) },
            { "PushButtonType", uno::Any( sal_Int16(awt::PushButtonType_STANDARD) ) },
            { "Label", uno::Any( msPauseBtn ) },
            { "HelpURL", uno::makeAny<OUString>( INET_HID_SCHEME HID_CHECK_FOR_UPD_PAUSE ) }
        };

        insertControlModel ( xControlModel, BUTTON_MODEL, msButtonIDs[PAUSE_BUTTON],
                             awt::Rectangle( BOX1_BTN_X, BOX1_BTN_Y, BUTTON_WIDTH, BUTTON_HEIGHT ),
                             aProps );
    }
    {   // resume button
        uno::Sequence< beans::NamedValue > aProps
        {
            { "DefaultButton", uno::Any( false ) },
            { "Enabled", uno::Any( true ) },
            { "PushButtonType", uno::Any( sal_Int16(awt::PushButtonType_STANDARD) ) },
            { "Label", uno::Any( msResumeBtn ) },
            { "HelpURL", uno::makeAny<OUString>( INET_HID_SCHEME HID_CHECK_FOR_UPD_RESUME ) }
        };

        insertControlModel ( xControlModel, BUTTON_MODEL, msButtonIDs[RESUME_BUTTON],
                             awt::Rectangle( BOX1_BTN_X,
                                             BOX1_BTN_Y + BUTTON_Y_OFFSET + BUTTON_HEIGHT,
                                             BUTTON_WIDTH,
                                             BUTTON_HEIGHT ),
                             aProps );
    }
    {   // abort button
        uno::Sequence< beans::NamedValue > aProps
        {
            { "DefaultButton", uno::Any( false ) },
            { "Enabled", uno::Any( true ) },
            { "PushButtonType", uno::Any( sal_Int16(awt::PushButtonType_STANDARD) ) },
            { "Label", uno::Any( msCancelBtn ) },
            { "HelpURL", uno::makeAny<OUString>( INET_HID_SCHEME HID_CHECK_FOR_UPD_CANCEL ) }
        };

        insertControlModel ( xControlModel, BUTTON_MODEL, msButtonIDs[CANCEL_BUTTON],
                             awt::Rectangle( BOX1_BTN_X,
                                             BOX1_BTN_Y + (2*(BUTTON_HEIGHT+BUTTON_Y_OFFSET)),
                                             BUTTON_WIDTH,
                                             BUTTON_HEIGHT ),
                             aProps );
    }
    {   // Label (FixedText) <description>
        uno::Sequence< beans::NamedValue > aProps { { "Label", uno::Any( msDescription ) } };

        insertControlModel( xControlModel, FIXED_TEXT_MODEL, "fixedTextDescription",
                            awt::Rectangle( DIALOG_BORDER+1, LABEL_Y_POS, EDIT_WIDTH-2, LABEL_HEIGHT ),
                            aProps );
    }
    {   // box around <description> text
        uno::Sequence< beans::NamedValue > aProps;

        insertControlModel( xControlModel, GROUP_BOX_MODEL, "DescriptionBox",
                            awt::Rectangle( DIALOG_BORDER, EDIT2_Y_POS, EDIT_WIDTH, BOX_HEIGHT2 ),
                            aProps );
    }
    {   // Text (MultiLineEdit) <description>
        uno::Sequence< beans::NamedValue > aProps
        {
            { "Text", uno::Any( OUString() ) },
            { "Border", uno::Any( sal_Int16( 0 ) ) },
            { "PaintTransparent", uno::Any( true ) },
            { "MultiLine", uno::Any( true ) },
            { "ReadOnly", uno::Any( true ) },
            { "AutoVScroll", uno::Any( true ) },
            { "HelpURL", uno::makeAny<OUString>( INET_HID_SCHEME HID_CHECK_FOR_UPD_DESCRIPTION ) }
        };

        insertControlModel( xControlModel, EDIT_FIELD_MODEL, TEXT_DESCRIPTION,
                            awt::Rectangle( DIALOG_BORDER + TEXT_OFFSET,
                                            EDIT2_Y_POS + 2*TEXT_OFFSET,
                                            EDIT_WIDTH - 3*TEXT_OFFSET,
                                            BOX_HEIGHT2 - 3*TEXT_OFFSET ),
                            aProps );
    }
    {   // @see awt/UnoControlFixedLineModel.idl
        uno::Sequence< beans::NamedValue > aProps { { "Orientation", uno::Any( sal_Int32( 0 ) ) } };

        insertControlModel( xControlModel, FIXED_LINE_MODEL, "fixedLine",
                            awt::Rectangle( 0, BUTTON_BAR_Y_POS, DIALOG_WIDTH, 5 ),
                            aProps );
    }
    {   // close button // @see awt/UnoControlButtonModel.idl
        uno::Sequence< beans::NamedValue > aProps
        {
            { "DefaultButton", uno::Any( false ) },
            { "Enabled", uno::Any( true ) },
            // [property] short PushButtonType
            // with own "ButtonActionListener"
            { "PushButtonType", uno::Any( sal_Int16(awt::PushButtonType_STANDARD) ) },
            // with default ActionListener => endDialog().
            // setProperty( aProps, 2, "PushButtonType", uno::Any( sal_Int16(awt::PushButtonType_CANCEL) ) );
            // [property] string Label // only if PushButtonType_STANDARD
            { "Label", uno::Any( msClose ) },
            { "HelpURL", uno::makeAny<OUString>( INET_HID_SCHEME HID_CHECK_FOR_UPD_CLOSE ) }
        };

        insertControlModel ( xControlModel, BUTTON_MODEL, msButtonIDs[ CLOSE_BUTTON ],
                             awt::Rectangle( CLOSE_BTN_X, BUTTON_Y_POS, BUTTON_WIDTH, BUTTON_HEIGHT ),
                             aProps );
    }
    {   // install button
        uno::Sequence< beans::NamedValue > aProps
        {
            { "DefaultButton", uno::Any( false ) },
            { "Enabled", uno::Any( true ) },
            { "PushButtonType", uno::Any( sal_Int16(awt::PushButtonType_STANDARD) ) },
            { "Label", uno::Any( msInstall ) },
            { "HelpURL", uno::makeAny<OUString>( INET_HID_SCHEME HID_CHECK_FOR_UPD_INSTALL ) }
        };

        insertControlModel ( xControlModel, BUTTON_MODEL, msButtonIDs[INSTALL_BUTTON],
                             awt::Rectangle( INSTALL_BTN_X, BUTTON_Y_POS, BUTTON_WIDTH, BUTTON_HEIGHT ),
                             aProps );
    }
    {   // download button
        uno::Sequence< beans::NamedValue > aProps
        {
            { "DefaultButton", uno::Any( false ) },
            { "Enabled", uno::Any( true ) },
            { "PushButtonType", uno::Any( sal_Int16(awt::PushButtonType_STANDARD) ) },
            { "Label", uno::Any( msDownload ) },
            { "HelpURL", uno::makeAny<OUString>( INET_HID_SCHEME HID_CHECK_FOR_UPD_DOWNLOAD ) }
        };

        insertControlModel ( xControlModel, BUTTON_MODEL, msButtonIDs[DOWNLOAD_BUTTON],
                             awt::Rectangle( DOWNLOAD_BTN_X, BUTTON_Y_POS, BUTTON_WIDTH, BUTTON_HEIGHT ),
                             aProps );
    }
    {   // help button
        uno::Sequence< beans::NamedValue > aProps
        {
            { "DefaultButton", uno::Any( false ) },
            { "Enabled", uno::Any( true ) },
            { "PushButtonType", uno::Any( sal_Int16(awt::PushButtonType_HELP) ) }
        };

        insertControlModel( xControlModel, BUTTON_MODEL, msButtonIDs[HELP_BUTTON],
                            awt::Rectangle( DIALOG_BORDER, BUTTON_Y_POS, BUTTON_WIDTH, BUTTON_HEIGHT ),
                            aProps );
    }
    {   // @see awt/UnoControlThrobberModel.idl
        uno::Sequence< beans::NamedValue > aProps;

        insertControlModel( xControlModel, "com.sun.star.awt.SpinningProgressControlModel", CTRL_THROBBER,
                            awt::Rectangle( THROBBER_X_POS, THROBBER_Y_POS, THROBBER_WIDTH, THROBBER_HEIGHT),
                            aProps );
    }
    {    // @see awt/UnoControlProgressBarModel.idl
        uno::Sequence< beans::NamedValue > aProps
        {
            { "Enabled", uno::Any( true ) },
            { "ProgressValue", uno::Any( sal_Int32( 0 ) ) },
            { "ProgressValueMax", uno::Any( sal_Int32( 100 ) ) },
            { "ProgressValueMin", uno::Any( sal_Int32( 0 ) ) },
        };
        insertControlModel( xControlModel, "com.sun.star.awt.UnoControlProgressBarModel", CTRL_PROGRESS,
                            awt::Rectangle( PROGRESS_X_POS, PROGRESS_Y_POS, PROGRESS_WIDTH, PROGRESS_HEIGHT ),
                            aProps);
    }

    uno::Reference< awt::XUnoControlDialog > xControl = awt::UnoControlDialog::create( mxContext );
    xControl->setModel( xControlModel );

    if ( !mbVisible )
    {
        xControl->setVisible( false );
    }

    xControl->createPeer( nullptr, nullptr );
    {
        for ( int i = 0; i < HELP_BUTTON; i++ )
        {
            uno::Reference< awt::XButton > xButton ( xControl->getControl( msButtonIDs[i] ), uno::UNO_QUERY);
            if (xButton.is())
            {
                xButton->setActionCommand( msButtonIDs[i] );
                xButton->addActionListener( this );
            }
        }
    }

    mxUpdDlg.set( xControl, uno::UNO_QUERY_THROW );
    mnLastCtrlState = -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
