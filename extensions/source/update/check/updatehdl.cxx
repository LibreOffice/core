/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updatehdl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 14:37:48 $
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
#include "precompiled_extensions.hxx"

#include "updatehdl.hxx"
#include "extensio.hrc"

#include "osl/diagnose.h"
#include "osl/thread.hxx"
#include "rtl/ustring.hxx"

#include "com/sun/star/uno/Sequence.h"

#include <com/sun/star/style/VerticalAlignment.hpp>

#include "com/sun/star/awt/ActionEvent.hpp"
#include "com/sun/star/awt/PushButtonType.hpp"
#include "com/sun/star/awt/VclWindowPeerAttribute.hpp"
#include "com/sun/star/awt/WindowAttribute.hpp"
#include "com/sun/star/awt/XButton.hpp"
#include "com/sun/star/awt/XControl.hpp"
#include "com/sun/star/awt/XControlContainer.hpp"
#include "com/sun/star/awt/XMessageBox.hpp"
#include "com/sun/star/awt/XThrobber.hpp"
#include "com/sun/star/awt/XTopWindow.hpp"
#include "com/sun/star/awt/XVclWindowPeer.hpp"
#include "com/sun/star/awt/XWindow.hpp"
#include "com/sun/star/awt/XWindow2.hpp"

#include <com/sun/star/beans/PropertyValue.hpp>
#include "com/sun/star/beans/XPropertySet.hpp"

#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/task/InteractionRequestStringResolver.hpp"

#include <com/sun/star/resource/XResourceBundleLoader.hpp>

#include "updatehdl.hrc"

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

#define COMMAND_CLOSE       UNISTRING("close")

#define CTRL_THROBBER       UNISTRING("throbber")
#define CTRL_PROGRESS       UNISTRING("progress")

#define TEXT_STATUS         UNISTRING("text_status")
#define TEXT_PERCENT        UNISTRING("text_percent")
#define TEXT_DESCRIPTION    UNISTRING("text_description")

#define FIXED_LINE_MODEL    UNISTRING("com.sun.star.awt.UnoControlFixedLineModel")
#define FIXED_TEXT_MODEL    UNISTRING("com.sun.star.awt.UnoControlFixedTextModel")
#define EDIT_FIELD_MODEL    UNISTRING("com.sun.star.awt.UnoControlEditModel")
#define BUTTON_MODEL        UNISTRING("com.sun.star.awt.UnoControlButtonModel")
#define GROUP_BOX_MODEL     UNISTRING("com.sun.star.awt.UnoControlGroupBoxModel")

using namespace com::sun::star;

//--------------------------------------------------------------------
UpdateHandler::UpdateHandler( const uno::Reference< uno::XComponentContext > & rxContext,
                              const rtl::Reference< IActionListener > & rxActionListener ) :
    mxContext( rxContext ),
    mxActionListener( rxActionListener ),
    meCurState( UPDATESTATES_COUNT ),
    mnPercent( 0 ),
    mnLastCtrlState( -1 ),
    mbDownloadBtnHasDots( false ),
    mbVisible( false ),
    mbStringsLoaded( false ),
    mbMinimized( false )
{
}

//--------------------------------------------------------------------
UpdateHandler::~UpdateHandler()
{
    mxContext = NULL;
    mxUpdDlg = NULL;
    mxInteractionHdl = NULL;
    mxActionListener = NULL;
}

//--------------------------------------------------------------------
void UpdateHandler::enableControls( short nCtrlState )
{
    osl::MutexGuard aGuard( maMutex );

    if ( nCtrlState == mnLastCtrlState )
        return;

    sal_Bool bEnableControl;

    short nCurStateVal = nCtrlState;
    short nOldStateVal = mnLastCtrlState;

    // the help button should always be the last button in the
    // enum list und must never be disabled
    for ( int i=0; i<HELP_BUTTON; i++ )
    {
        nCurStateVal = (short)(nCtrlState >> i);
        nOldStateVal = (short)(mnLastCtrlState >> i);
        if ( ( nCurStateVal & 0x01 ) != ( nOldStateVal & 0x01 ) )
        {
            bEnableControl = ( ( nCurStateVal & 0x01 ) == 0x01 );
            setControlProperty( msButtonIDs[i], UNISTRING("Enabled"), uno::Any( bEnableControl ) );
        }
    }

    mnLastCtrlState = nCtrlState;
}

//--------------------------------------------------------------------
void UpdateHandler::setDownloadBtnLabel( bool bAppendDots )
{
    osl::MutexGuard aGuard( maMutex );

    if ( mbDownloadBtnHasDots != bAppendDots )
    {
        rtl::OUString aLabel( msDownload );

        if ( bAppendDots )
            aLabel += UNISTRING( "..." );

        setControlProperty( msButtonIDs[DOWNLOAD_BUTTON], UNISTRING("Label"), uno::Any( aLabel ) );
        mbDownloadBtnHasDots = bAppendDots;
    }
}

//--------------------------------------------------------------------
UpdateState UpdateHandler::setState( UpdateState eState )
{
    osl::MutexGuard aGuard( maMutex );

    if ( eState == meCurState )
        return meCurState;

    // osl::MutexGuard();
    if ( !mxUpdDlg.is() )
        createDialog();

    if ( isVisible() )
        {} // ToTop();

    rtl::OUString sText;

    switch ( eState )
    {
        case UPDATESTATE_CHECKING:
            showControls( (1<<CANCEL_BUTTON) + (1<<THROBBER_CTRL) );
            enableControls( 1<<CANCEL_BUTTON );
            setControlProperty( TEXT_STATUS, UNISTRING("Text"), uno::Any( substVariables(msChecking) ) );
            setControlProperty( TEXT_DESCRIPTION, UNISTRING("Text"), uno::Any( rtl::OUString() ) );
            focusControl( CANCEL_BUTTON );
            break;
        case UPDATESTATE_ERROR_CHECKING:
            showControls( 0 );
            enableControls( 1 << CLOSE_BUTTON );
            setControlProperty( TEXT_STATUS, UNISTRING("Text"), uno::Any( substVariables(msCheckingError) ) );
            focusControl( CLOSE_BUTTON );
            break;
        case UPDATESTATE_UPDATE_AVAIL:
            showControls( 0 );
            enableControls( ( 1 << CLOSE_BUTTON )  + ( 1 << DOWNLOAD_BUTTON ) );
            setControlProperty( TEXT_STATUS, UNISTRING("Text"), uno::Any( substVariables(msUpdFound) ) );

            sText = substVariables(msDownloadWarning);
            if ( msDescriptionMsg.getLength() )
                sText += UNISTRING("\n\n") + msDescriptionMsg;
            setControlProperty( TEXT_DESCRIPTION, UNISTRING("Text"), uno::Any( sText ) );

            setDownloadBtnLabel( false );
            focusControl( DOWNLOAD_BUTTON );
            break;
        case UPDATESTATE_UPDATE_NO_DOWNLOAD:
            showControls( 0 );
            enableControls( ( 1 << CLOSE_BUTTON )  + ( 1 << DOWNLOAD_BUTTON ) );
            setControlProperty( TEXT_STATUS, UNISTRING("Text"), uno::Any( substVariables(msUpdFound) ) );

            sText = substVariables(msDownloadNotAvail);
            if ( msDescriptionMsg.getLength() )
                sText += UNISTRING("\n\n") + msDescriptionMsg;
            setControlProperty( TEXT_DESCRIPTION, UNISTRING("Text"), uno::Any( sText ) );

            setDownloadBtnLabel( true );
            focusControl( DOWNLOAD_BUTTON );
            break;
        case UPDATESTATE_NO_UPDATE_AVAIL:
            showControls( 0 );
            enableControls( 1 << CLOSE_BUTTON );
            setControlProperty( TEXT_STATUS, UNISTRING("Text"), uno::Any( substVariables(msNoUpdFound) ) );
            setControlProperty( TEXT_DESCRIPTION, UNISTRING("Text"), uno::Any( rtl::OUString() ) );
            focusControl( CLOSE_BUTTON );
            break;
        case UPDATESTATE_DOWNLOADING:
            showControls( (1<<PROGRESS_CTRL) + (1<<CANCEL_BUTTON) + (1<<PAUSE_BUTTON) + (1<<RESUME_BUTTON) );
            enableControls( (1<<CLOSE_BUTTON) + (1<<CANCEL_BUTTON) + (1<<PAUSE_BUTTON) );
            setControlProperty( TEXT_STATUS, UNISTRING("Text"), uno::Any( substVariables(msDownloading) ) );
            setControlProperty( TEXT_PERCENT, UNISTRING("Text"), uno::Any( substVariables(msPercent) ) );
            //setControlProperty( TEXT_DESCRIPTION, UNISTRING("Text"), uno::Any( substVariables(msDownloadWarning) ) );
            setControlProperty( CTRL_PROGRESS, UNISTRING("ProgressValue"), uno::Any( mnPercent ) );
            focusControl( CLOSE_BUTTON );
            break;
        case UPDATESTATE_DOWNLOAD_PAUSED:
            showControls( (1<<PROGRESS_CTRL) + (1<<CANCEL_BUTTON) + (1<<PAUSE_BUTTON) + (1<<RESUME_BUTTON) );
            enableControls( (1<<CLOSE_BUTTON) + (1<<CANCEL_BUTTON) + (1<<RESUME_BUTTON) );
            setControlProperty( TEXT_STATUS, UNISTRING("Text"), uno::Any( substVariables(msDownloadPause) ) );
            setControlProperty( TEXT_PERCENT, UNISTRING("Text"), uno::Any( substVariables(msPercent) ) );
            //setControlProperty( TEXT_DESCRIPTION, UNISTRING("Text"), uno::Any( substVariables(msDownloadWarning) ) );
            setControlProperty( CTRL_PROGRESS, UNISTRING("ProgressValue"), uno::Any( mnPercent ) );
            focusControl( CLOSE_BUTTON );
            break;
        case UPDATESTATE_ERROR_DOWNLOADING:
            showControls( 0 );
            enableControls( 1 << CLOSE_BUTTON );
            setControlProperty( TEXT_STATUS, UNISTRING("Text"), uno::Any( substVariables(msDownloadError) ) );
            focusControl( CLOSE_BUTTON );
            break;
        case UPDATESTATE_DOWNLOAD_AVAIL:
            showControls( 0 );
            enableControls( (1<<CLOSE_BUTTON) + (1<<INSTALL_BUTTON) );
            setControlProperty( TEXT_STATUS, UNISTRING("Text"), uno::Any( substVariables(msReady2Install) ) );
            setControlProperty( TEXT_DESCRIPTION, UNISTRING("Text"), uno::Any( substVariables(msDownloadDescr) ) );
            focusControl( INSTALL_BUTTON );
            break;
        case UPDATESTATE_AUTO_START:
        case UPDATESTATES_COUNT:
            //do nothing, only count!
            break;
    }

    meCurState = eState;

    return meCurState;
}

//--------------------------------------------------------------------
bool UpdateHandler::isVisible() const
{
    if ( !mxUpdDlg.is() ) return false;

    uno::Reference< awt::XWindow2 > xWindow( mxUpdDlg, uno::UNO_QUERY );

    if ( xWindow.is() )
        return xWindow->isVisible();
    else
        return false;
}

//--------------------------------------------------------------------
void UpdateHandler::setVisible( bool bVisible )
{
    osl::MutexGuard aGuard( maMutex );

    mbVisible = bVisible;

    // osl::MutexGuard();
    if ( !mxUpdDlg.is() )
        createDialog();

    uno::Reference< awt::XWindow > xWindow( mxUpdDlg, uno::UNO_QUERY );

    if ( xWindow.is() )
        xWindow->setVisible( bVisible );

    if ( bVisible )
    {
        uno::Reference< awt::XTopWindow > xTopWindow( mxUpdDlg, uno::UNO_QUERY );
        if ( xTopWindow.is() )
            xTopWindow->toFront();
    }
}

//--------------------------------------------------------------------
void UpdateHandler::setProgress( sal_Int32 nPercent )
{
    if ( nPercent != mnPercent )
    {
        osl::MutexGuard aGuard( maMutex );

        mnPercent = nPercent;
        setControlProperty( CTRL_PROGRESS, UNISTRING("ProgressValue"), uno::Any( nPercent ) );
        setControlProperty( TEXT_PERCENT, UNISTRING("Text"), uno::Any( substVariables(msPercent) ) );
    }
}

//--------------------------------------------------------------------
void UpdateHandler::setErrorMessage( const rtl::OUString& rErrorMsg )
{
    setControlProperty( TEXT_DESCRIPTION, UNISTRING("Text"), uno::Any( rErrorMsg ) );
}

//--------------------------------------------------------------------
void UpdateHandler::setDownloadFile( const rtl::OUString& rFilePath )
{
    sal_Int32 nLast = rFilePath.lastIndexOf( '/' );
    if ( nLast != -1 )
        msDownloadFile = rFilePath.copy( nLast+1 );
}

//--------------------------------------------------------------------
rtl::OUString UpdateHandler::getBubbleText( UpdateState eState )
{
    osl::MutexGuard aGuard( maMutex );

    rtl::OUString sText;
    sal_Int32 nIndex = (sal_Int32) eState;

    loadStrings();

    if ( ( UPDATESTATE_UPDATE_AVAIL <= nIndex ) && ( nIndex < UPDATESTATES_COUNT ) )
        sText = substVariables( msBubbleTexts[ nIndex - UPDATESTATE_UPDATE_AVAIL ] );

    return sText;
}

//--------------------------------------------------------------------
rtl::OUString UpdateHandler::getBubbleTitle( UpdateState eState )
{
    osl::MutexGuard aGuard( maMutex );

    rtl::OUString sText;
    sal_Int32 nIndex = (sal_Int32) eState;

    loadStrings();

    if ( ( UPDATESTATE_UPDATE_AVAIL <= nIndex ) && ( nIndex < UPDATESTATES_COUNT ) )
        sText = substVariables( msBubbleTitles[ nIndex - UPDATESTATE_UPDATE_AVAIL] );

    return sText;
}

//--------------------------------------------------------------------
rtl::OUString UpdateHandler::getDefaultInstErrMsg()
{
    osl::MutexGuard aGuard( maMutex );

    loadStrings();

    return substVariables( msInstallError );
}

// XActionListener
//--------------------------------------------------------------------
void SAL_CALL UpdateHandler::disposing( const lang::EventObject& )
    throw( uno::RuntimeException )
{
    OSL_TRACE("ButtonActionListener::disposing(): %p\n", this);
}

//--------------------------------------------------------------------
void SAL_CALL UpdateHandler::actionPerformed( awt::ActionEvent const & rEvent )
    throw( uno::RuntimeException )
{
    DialogControls eButton = BUTTON_COUNT;
    for ( int i = 0; i < BUTTON_COUNT; i++ )
    {
        if ( rEvent.ActionCommand.equals( msButtonIDs[i] ) )
        {
            eButton = (DialogControls) i;
            break;
        }
    }

    if ( rEvent.ActionCommand.equals( COMMAND_CLOSE ) )
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
                 ( meCurState == UPDATESTATE_DOWNLOAD_PAUSED ) )
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
            OSL_ENSURE( false, "UpdateHandler::actionPerformed: unknown command!" );
    }
}

// XTopWindowListener
//--------------------------------------------------------------------
void SAL_CALL UpdateHandler::windowOpened( const lang::EventObject& )
    throw( uno::RuntimeException )
{
}

//--------------------------------------------------------------------
void SAL_CALL UpdateHandler::windowClosing( const lang::EventObject& e )
    throw( uno::RuntimeException )
{
    awt::ActionEvent aActionEvt;
    aActionEvt.ActionCommand = COMMAND_CLOSE;
    aActionEvt.Source = e.Source;

    actionPerformed( aActionEvt );
}

//--------------------------------------------------------------------
void SAL_CALL UpdateHandler::windowClosed( const lang::EventObject& )
    throw( uno::RuntimeException )
{
}

//--------------------------------------------------------------------
void SAL_CALL UpdateHandler::windowMinimized( const lang::EventObject& )
    throw( uno::RuntimeException )
{
    mbMinimized = true;
}

//--------------------------------------------------------------------
void SAL_CALL UpdateHandler::windowNormalized( const lang::EventObject& )
    throw( uno::RuntimeException )
{
    mbMinimized = false;
}

//--------------------------------------------------------------------
void SAL_CALL UpdateHandler::windowActivated( const lang::EventObject& )
    throw( uno::RuntimeException )
{
}

//--------------------------------------------------------------------
void SAL_CALL UpdateHandler::windowDeactivated( const lang::EventObject& )
    throw( uno::RuntimeException )
{
}

// XInteractionHandler
//------------------------------------------------------------------------------
void SAL_CALL UpdateHandler::handle( uno::Reference< task::XInteractionRequest > const & rRequest)
    throw (uno::RuntimeException)
{
    if ( !mxInteractionHdl.is() )
    {
        if( !mxContext.is() )
            throw uno::RuntimeException( UNISTRING( "UpdateHandler:: empty component context" ), *this );

        uno::Reference< lang::XMultiComponentFactory > xServiceManager(mxContext->getServiceManager());

        if( !xServiceManager.is() )
            throw uno::RuntimeException( UNISTRING( "UpdateHandler: unable to obtain service manager from component context" ), *this );

        mxInteractionHdl = uno::Reference<task::XInteractionHandler> (
                                xServiceManager->createInstanceWithContext(
                                    UNISTRING( "com.sun.star.task.InteractionHandler" ),
                                    mxContext),
                                uno::UNO_QUERY_THROW);
        if( !mxInteractionHdl.is() )
            throw uno::RuntimeException( UNISTRING( "UpdateHandler:: could not get default interaction handler" ), *this );
    }
    uno::Reference< task::XInteractionRequestStringResolver > xStrResolver =
            task::InteractionRequestStringResolver::create( mxContext );
    beans::Optional< ::rtl::OUString > aErrorText = xStrResolver->getStringFromInformationalRequest( rRequest );
    if ( aErrorText.IsPresent )
    {
        setControlProperty( TEXT_DESCRIPTION, UNISTRING("Text"), uno::Any( aErrorText.Value ) );

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


//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
void UpdateHandler::searchAndReplaceAll( rtl::OUString &rText,
                                         const rtl::OUString &rWhat,
                                         const rtl::OUString &rWith ) const
{
    sal_Int32 nIndex = rText.indexOf( rWhat );

    while ( nIndex != -1 )
    {
        rText = rText.replaceAt( nIndex, rWhat.getLength(), rWith );
        nIndex = rText.indexOf( rWhat, nIndex );
    }
}

//--------------------------------------------------------------------
rtl::OUString UpdateHandler::loadString( const uno::Reference< resource::XResourceBundle > xBundle,
                                         sal_Int32 nResourceId ) const
{
    rtl::OUString sString;
    rtl::OUString sKey = UNISTRING( "string:" ) + rtl::OUString::valueOf( nResourceId );

    try
    {
        OSL_VERIFY( xBundle->getByName( sKey ) >>= sString );
    }
    catch( const uno::Exception& )
    {
        OSL_ENSURE( false, "UpdateHandler::loadString: caught an exception!" );
        sString = UNISTRING("Missing ") + sKey;
    }

    searchAndReplaceAll( sString, UNISTRING( "%PRODUCTNAME" ), msProductName );
    searchAndReplaceAll( sString, UNISTRING( "%PRODUCTVERSION" ), msProductVersion );

    return sString;
}

rtl::OUString UpdateHandler::substVariables( const rtl::OUString &rSource ) const
{
    rtl::OUString sString( rSource );

    searchAndReplaceAll( sString, UNISTRING( "%NEXTVERSION" ), msNextVersion );
    searchAndReplaceAll( sString, UNISTRING( "%DOWNLOAD_PATH" ), msDownloadPath );
    searchAndReplaceAll( sString, UNISTRING( "%FILE_NAME" ), msDownloadFile );
    searchAndReplaceAll( sString, UNISTRING( "%PERCENT" ), rtl::OUString::valueOf( mnPercent ) );

    return sString;
}

//--------------------------------------------------------------------
void UpdateHandler::loadStrings()
{
    if ( mbStringsLoaded )
        return;
    else
        mbStringsLoaded = true;

    uno::Reference< resource::XResourceBundleLoader > xLoader;
    try
    {
        uno::Any aValue( mxContext->getValueByName(
                UNISTRING( "/singletons/com.sun.star.resource.OfficeResourceLoader" ) ) );
        OSL_VERIFY( aValue >>= xLoader );
    }
    catch( const uno::Exception& )
    {
        OSL_ENSURE( false, "UpdateHandler::loadStrings: could not create the resource loader!" );
    }

    if ( !xLoader.is() ) return;

    uno::Reference< resource::XResourceBundle > xBundle;

    try
    {
        xBundle = xLoader->loadBundle_Default( UNISTRING( "upd" ) );
    }
    catch( const resource::MissingResourceException& )
    {
        OSL_ENSURE( false, "UpdateHandler::loadStrings: missing the resource bundle!" );
    }

    if ( !xBundle.is() ) return;

    getProductName();

    msChecking      = loadString( xBundle, RID_UPDATE_STR_CHECKING );
    msCheckingError = loadString( xBundle, RID_UPDATE_STR_CHECKING_ERR );
    msNoUpdFound    = loadString( xBundle, RID_UPDATE_STR_NO_UPD_FOUND );
    msUpdFound      = loadString( xBundle, RID_UPDATE_STR_UPD_FOUND );
    msDlgTitle      = loadString( xBundle, RID_UPDATE_STR_DLG_TITLE );
    msDownloadPause = loadString( xBundle, RID_UPDATE_STR_DOWNLOAD_PAUSE );
    msDownloadError = loadString( xBundle, RID_UPDATE_STR_DOWNLOAD_ERR );
    msDownloadWarning = loadString( xBundle, RID_UPDATE_STR_DOWNLOAD_WARN );
    msDownloadDescr =  loadString( xBundle, RID_UPDATE_STR_DOWNLOAD_DESCR );
    msDownloadNotAvail = loadString( xBundle, RID_UPDATE_STR_DOWNLOAD_UNAVAIL );
    msDownloading   = loadString( xBundle, RID_UPDATE_STR_DOWNLOADING );
    msReady2Install = loadString( xBundle, RID_UPDATE_STR_READY_INSTALL );
    msCancelTitle   = loadString( xBundle, RID_UPDATE_STR_CANCEL_TITLE );
    msCancelMessage = loadString( xBundle, RID_UPDATE_STR_CANCEL_DOWNLOAD );
    msInstallMessage = loadString( xBundle, RID_UPDATE_STR_BEGIN_INSTALL );
    msInstallNow    = loadString( xBundle, RID_UPDATE_STR_INSTALL_NOW );
    msInstallLater  = loadString( xBundle, RID_UPDATE_STR_INSTALL_LATER );
    msInstallError  = loadString( xBundle, RID_UPDATE_STR_INSTALL_ERROR );
    msOverwriteWarning = loadString( xBundle, RID_UPDATE_STR_OVERWRITE_WARNING );
    msPercent       = loadString( xBundle, RID_UPDATE_STR_PERCENT );

    msStatusFL      = loadString( xBundle, RID_UPDATE_FT_STATUS );
    msDescription   = loadString( xBundle, RID_UPDATE_FT_DESCRIPTION );

    msClose         = loadString( xBundle, RID_UPDATE_BTN_CLOSE );
    msDownload      = loadString( xBundle, RID_UPDATE_BTN_DOWNLOAD );
    msInstall       = loadString( xBundle, RID_UPDATE_BTN_INSTALL );
    msPauseBtn      = loadString( xBundle, RID_UPDATE_BTN_PAUSE );
    msResumeBtn     = loadString( xBundle, RID_UPDATE_BTN_RESUME );
    msCancelBtn     = loadString( xBundle, RID_UPDATE_BTN_CANCEL );

    // all update states before UPDATESTATE_UPDATE_AVAIL don't have a bubble
    // so we can ignore them
    for ( int i=0; i < (int)(UPDATESTATES_COUNT - UPDATESTATE_UPDATE_AVAIL); i++ )
    {
        msBubbleTexts[ i ] = loadString( xBundle, RID_UPDATE_BUBBLE_TEXT_START + i );
        msBubbleTitles[ i ] = loadString( xBundle, RID_UPDATE_BUBBLE_T_TEXT_START + i );
    }

    for ( int i=0; i < BUTTON_COUNT; i++ )
    {
        msButtonIDs[ i ] = UNISTRING("BUTTON_") + rtl::OUString::valueOf( (sal_Int32) i );
    }
}


//--------------------------------------------------------------------
void UpdateHandler::startThrobber( bool bStart )
{
    uno::Reference< awt::XControlContainer > xContainer( mxUpdDlg, uno::UNO_QUERY );
    uno::Reference< awt::XThrobber > xThrobber( xContainer->getControl( CTRL_THROBBER ), uno::UNO_QUERY );

    if ( xThrobber.is() )
    {
        if ( bStart )
            xThrobber->start();
        else
            xThrobber->stop();
    }

    uno::Reference< awt::XWindow > xWindow( xContainer->getControl( CTRL_THROBBER ), uno::UNO_QUERY );
    if (xWindow.is() )
        xWindow->setVisible( bStart );
}

//--------------------------------------------------------------------
void UpdateHandler::setControlProperty( const rtl::OUString &rCtrlName,
                                        const rtl::OUString &rPropName,
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
        OSL_ENSURE( sal_False, "UpdateHandler::setControlProperty: caught an exception!" );
    }
}

//--------------------------------------------------------------------
void UpdateHandler::showControl( const rtl::OUString &rCtrlName, sal_Bool bShow )
{
    uno::Reference< awt::XControlContainer > xContainer( mxUpdDlg, uno::UNO_QUERY );

    if ( !xContainer.is() )
    {
        OSL_ENSURE( sal_False, "UpdateHandler::showControl: could not get control container!" );
        return;
    }

    uno::Reference< awt::XWindow > xWindow( xContainer->getControl( rCtrlName ), uno::UNO_QUERY );
    if ( xWindow.is() )
        xWindow->setVisible( bShow );
}

//--------------------------------------------------------------------
void UpdateHandler::focusControl( DialogControls eID )
{
    uno::Reference< awt::XControlContainer > xContainer( mxUpdDlg, uno::UNO_QUERY );

    if ( !xContainer.is() )
    {
        OSL_ENSURE( sal_False, "UpdateHandler::focusControl: could not get control container!" );
        return;
    }

    OSL_ENSURE( (eID < BUTTON_COUNT), "UpdateHandler::focusControl: id to big!" );

    uno::Reference< awt::XWindow > xWindow( xContainer->getControl( msButtonIDs[(short)eID] ), uno::UNO_QUERY );
    if ( xWindow.is() )
        xWindow->setFocus();
}

//--------------------------------------------------------------------
void UpdateHandler::insertControlModel( uno::Reference< awt::XControlModel > & rxDialogModel,
                                        rtl::OUString const & rServiceName,
                                        rtl::OUString const & rControlName,
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
    xPropSet->setPropertyValue( UNISTRING("Name"), uno::Any (rControlName) );
    xPropSet->setPropertyValue( UNISTRING("PositionX"), uno::Any (rPosSize.X) );
    xPropSet->setPropertyValue( UNISTRING("PositionY"), uno::Any (rPosSize.Y) );
    xPropSet->setPropertyValue( UNISTRING("Height"), uno::Any (rPosSize.Height) );
    xPropSet->setPropertyValue( UNISTRING("Width"), uno::Any (rPosSize.Width) );

    // insert by Name into DialogModel container
    uno::Reference< container::XNameContainer > xContainer (rxDialogModel, uno::UNO_QUERY_THROW);
    xContainer->insertByName( rControlName, uno::Any (uno::Reference< uno::XInterface >(xModel, uno::UNO_QUERY)));
}

//--------------------------------------------------------------------
void UpdateHandler::getProductName()
{
    if( !mxContext.is() )
        throw uno::RuntimeException( UNISTRING( "getProductName: empty component context" ), *this );

    uno::Reference< lang::XMultiComponentFactory > xServiceManager( mxContext->getServiceManager() );

    if( !xServiceManager.is() )
        throw uno::RuntimeException( UNISTRING( "getProductName: unable to obtain service manager from component context" ), *this );

    uno::Reference< lang::XMultiServiceFactory > xConfigurationProvider(
        xServiceManager->createInstanceWithContext( UNISTRING( "com.sun.star.configuration.ConfigurationProvider" ), mxContext ),
        uno::UNO_QUERY_THROW);

    beans::PropertyValue aProperty;
    aProperty.Name  = UNISTRING( "nodepath" );
    aProperty.Value = uno::makeAny( UNISTRING("org.openoffice.Setup/Product") );

    uno::Sequence< uno::Any > aArgumentList( 1 );
    aArgumentList[0] = uno::makeAny( aProperty );

    uno::Reference< uno::XInterface > xConfigAccess;
    xConfigAccess = xConfigurationProvider->createInstanceWithArguments( UNISTRING("com.sun.star.configuration.ConfigurationAccess"),
                                                                         aArgumentList );

    uno::Reference< container::XNameAccess > xNameAccess( xConfigAccess, uno::UNO_QUERY_THROW );
    rtl::OUString aProductName;

    xNameAccess->getByName(UNISTRING("ooName")) >>= msProductName;
    xNameAccess->getByName(UNISTRING("ooSetupVersion")) >>= msProductVersion;
}

//--------------------------------------------------------------------
bool UpdateHandler::showWarning( const rtl::OUString &rWarningText ) const
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
    aDescriptor.WindowServiceName = UNISTRING( "warningbox" );
    aDescriptor.ParentIndex       = -1;
    aDescriptor.Parent            = xPeer;
    aDescriptor.Bounds            = awt::Rectangle( 10, 10, 250, 150 );
    aDescriptor.WindowAttributes  = nWindowAttributes;

    uno::Reference< awt::XMessageBox > xMsgBox( xToolkit->createWindow( aDescriptor ), uno::UNO_QUERY );
    if ( xMsgBox.is() )
    {
        sal_Int16 nRet;
        // xMsgBox->setCaptionText( msCancelTitle );
        xMsgBox->setMessageText( rWarningText );
        nRet = xMsgBox->execute();
        if ( nRet == 2 ) // RET_YES == 2
            bRet = true;
    }

    return bRet;
}

//--------------------------------------------------------------------
bool UpdateHandler::showOverwriteWarning() const
{
    return showWarning( msOverwriteWarning );
}

//--------------------------------------------------------------------
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
#define THROBBER_WIDTH      14
#define THROBBER_HEIGHT     14
#define THROBBER_X_POS      ( DIALOG_BORDER + 8 )
#define THROBBER_Y_POS      ( DIALOG_BORDER + 23 )
#define BUTTON_BAR_HEIGHT   24
#define LABEL_OFFSET        ( LABEL_HEIGHT + 4 )
#define DIALOG_HEIGHT       ( BOX_HEIGHT1 + BOX_HEIGHT2 + LABEL_OFFSET + BUTTON_BAR_HEIGHT + 3 * DIALOG_BORDER )
#define LABEL_Y_POS         ( 2 * DIALOG_BORDER + BOX_HEIGHT1 )
#define EDIT2_Y_POS         ( LABEL_Y_POS + LABEL_OFFSET )
#define BUTTON_BAR_Y_POS    ( EDIT2_Y_POS + DIALOG_BORDER + BOX_HEIGHT2 )
#define BUTTON_Y_POS        ( BUTTON_BAR_Y_POS + 5 )
#define CLOSE_BTN_X         ( DIALOG_WIDTH - DIALOG_BORDER - BUTTON_WIDTH )
#define INSTALL_BTN_X       ( CLOSE_BTN_X - 2 * BUTTON_X_OFFSET - BUTTON_WIDTH )
#define DOWNLOAD_BTN_X      ( INSTALL_BTN_X - BUTTON_X_OFFSET - BUTTON_WIDTH )
#define PROGRESS_WIDTH      80
#define PROGRESS_HEIGHT     10
#define PROGRESS_X_POS      ( DIALOG_BORDER + 8 )
#define PROGRESS_Y_POS      ( DIALOG_BORDER + 2*LABEL_OFFSET )

//--------------------------------------------------------------------
void UpdateHandler::showControls( short nControls )
{
    // The buttons from CANCEL_BUTTON to RESUME_BUTTON will be shown or
    // hidden on demand
    short nShiftMe;
    for ( int i = 0; i <= (int)RESUME_BUTTON; i++ )
    {
        nShiftMe = (short)(nControls >> i);
        showControl( msButtonIDs[i], (sal_Bool)(nShiftMe & 0x01) );
    }

    nShiftMe = (short)(nControls >> THROBBER_CTRL);
    startThrobber( (bool)(nShiftMe & 0x01) );

    nShiftMe = (short)(nControls >> PROGRESS_CTRL);
    showControl( CTRL_PROGRESS, (sal_Bool)(nShiftMe & 0x01) );
    showControl( TEXT_PERCENT, (sal_Bool)(nShiftMe & 0x01) );

    // Status text needs to be smaller, when there are buttons at the right side of the dialog
    if ( ( nControls & ( (1<<CANCEL_BUTTON) + (1<<PAUSE_BUTTON) + (1<<RESUME_BUTTON) ) )  != 0 )
        setControlProperty( TEXT_STATUS, UNISTRING("Width"), uno::Any( sal_Int32(EDIT_WIDTH - BUTTON_WIDTH - 2*INNER_BORDER - TEXT_OFFSET ) ) );
    else
        setControlProperty( TEXT_STATUS, UNISTRING("Width"), uno::Any( sal_Int32(EDIT_WIDTH - 2*TEXT_OFFSET ) ) );

    // Status text needs to be taller, when we show the progress bar
    if ( ( nControls & ( 1<<PROGRESS_CTRL ) ) != 0 )
        setControlProperty( TEXT_STATUS, UNISTRING("Height"), uno::Any( sal_Int32(LABEL_HEIGHT) ) );
    else
        setControlProperty( TEXT_STATUS, UNISTRING("Height"), uno::Any( sal_Int32(BOX_HEIGHT1 - 4*TEXT_OFFSET - LABEL_HEIGHT ) ) );
}

//--------------------------------------------------------------------
void UpdateHandler::createDialog()
{
    loadStrings();

    uno::Reference< lang::XMultiComponentFactory > xFactory( mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference< awt::XControlModel > xControlModel( xFactory->createInstanceWithContext(
                                                         UNISTRING("com.sun.star.awt.UnoControlDialogModel"),
                                                         mxContext), uno::UNO_QUERY_THROW );
    {
        // @see awt/UnoControlDialogModel.idl
        uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY_THROW );

        xPropSet->setPropertyValue( UNISTRING("Title"), uno::Any( msDlgTitle ) );
        xPropSet->setPropertyValue( UNISTRING("Closeable"), uno::Any(sal_Bool(sal_True)) );
        xPropSet->setPropertyValue( UNISTRING("Enabled"), uno::Any(sal_Bool(sal_True)) );
        xPropSet->setPropertyValue( UNISTRING("Moveable"), uno::Any(sal_Bool(sal_True)) );
        xPropSet->setPropertyValue( UNISTRING("Sizeable"), uno::Any(sal_Bool(sal_True)) );
        xPropSet->setPropertyValue( UNISTRING("DesktopAsParent"), uno::Any(sal_Bool(sal_True)) );
        xPropSet->setPropertyValue( UNISTRING("Width"), uno::Any(sal_Int32( DIALOG_WIDTH )) );
        xPropSet->setPropertyValue( UNISTRING("Height"), uno::Any(sal_Int32( DIALOG_HEIGHT )) );
    }
    {   // Label (fixed text) <status>
        uno::Sequence< beans::NamedValue > aProps(1);

        setProperty( aProps, 0, UNISTRING("Label"), uno::Any( msStatusFL ) );

        insertControlModel( xControlModel, FIXED_TEXT_MODEL, UNISTRING( "fixedLineStatus" ),
                            awt::Rectangle( DIALOG_BORDER, DIALOG_BORDER, EDIT_WIDTH, LABEL_HEIGHT ),
                            aProps );
    }
    {   // box around <status> text
        uno::Sequence< beans::NamedValue > aProps;

        insertControlModel( xControlModel, GROUP_BOX_MODEL, UNISTRING( "StatusBox" ),
                            awt::Rectangle( DIALOG_BORDER, DIALOG_BORDER + LABEL_HEIGHT, EDIT_WIDTH, BOX_HEIGHT1 - LABEL_HEIGHT ),
                            aProps );
    }
    {   // Text (multiline edit) <status>
        uno::Sequence< beans::NamedValue > aProps(6);

        setProperty( aProps, 0, UNISTRING("Text"), uno::Any( substVariables(msChecking) ) );
        setProperty( aProps, 1, UNISTRING("Border"), uno::Any( sal_Int16( 0 ) ) );
        setProperty( aProps, 2, UNISTRING("PaintTransparent"), uno::Any( sal_Bool(sal_True) ) );
        setProperty( aProps, 3, UNISTRING("MultiLine"), uno::Any( sal_Bool( sal_True ) ) );
        setProperty( aProps, 4, UNISTRING("ReadOnly"), uno::Any( sal_Bool( sal_True ) ) );
        setProperty( aProps, 5, UNISTRING("AutoVScroll"), uno::Any( sal_Bool( sal_True ) ) );

        insertControlModel( xControlModel, EDIT_FIELD_MODEL, TEXT_STATUS,
                            awt::Rectangle( DIALOG_BORDER + TEXT_OFFSET,
                                            DIALOG_BORDER + LABEL_HEIGHT + TEXT_OFFSET,
                                            EDIT_WIDTH /*- BUTTON_WIDTH */ - 2*TEXT_OFFSET,
                                            BOX_HEIGHT1 - 4*TEXT_OFFSET - LABEL_HEIGHT ),
                            aProps );
    }
    {   // Text (edit) <percent>
        uno::Sequence< beans::NamedValue > aProps(4);

        setProperty( aProps, 0, UNISTRING("Text"), uno::Any( msPercent ) );
        setProperty( aProps, 1, UNISTRING("Border"), uno::Any( sal_Int16( 0 ) ) );
        setProperty( aProps, 2, UNISTRING("PaintTransparent"), uno::Any( sal_Bool(sal_True) ) );
        setProperty( aProps, 3, UNISTRING("ReadOnly"), uno::Any( sal_Bool( sal_True ) ) );

        insertControlModel( xControlModel, EDIT_FIELD_MODEL, TEXT_PERCENT,
                            awt::Rectangle( PROGRESS_X_POS + PROGRESS_WIDTH + DIALOG_BORDER,
                                            PROGRESS_Y_POS,
                                            EDIT_WIDTH - PROGRESS_WIDTH - 2*DIALOG_BORDER,
                                            LABEL_HEIGHT ),
                            aProps );
    }
    {   // pause button
        uno::Sequence< beans::NamedValue > aProps(4);

        setProperty( aProps, 0, UNISTRING("DefaultButton"), uno::Any( sal_Bool(sal_False) ) );
        setProperty( aProps, 1, UNISTRING("Enabled"), uno::Any( sal_Bool(sal_True) ) );
        setProperty( aProps, 2, UNISTRING("PushButtonType"), uno::Any( sal_Int16(awt::PushButtonType_STANDARD) ) );
        setProperty( aProps, 3, UNISTRING("Label"), uno::Any( msPauseBtn ) );

        insertControlModel ( xControlModel, BUTTON_MODEL, msButtonIDs[PAUSE_BUTTON],
                             awt::Rectangle( BOX1_BTN_X, BOX1_BTN_Y, BUTTON_WIDTH, BUTTON_HEIGHT ),
                             aProps );
    }
    {   // resume button
        uno::Sequence< beans::NamedValue > aProps(4);

        setProperty( aProps, 0, UNISTRING("DefaultButton"), uno::Any( sal_Bool(sal_False) ) );
        setProperty( aProps, 1, UNISTRING("Enabled"), uno::Any( sal_Bool(sal_True) ) );
        setProperty( aProps, 2, UNISTRING("PushButtonType"), uno::Any( sal_Int16(awt::PushButtonType_STANDARD) ) );
        setProperty( aProps, 3, UNISTRING("Label"), uno::Any( msResumeBtn ) );

        insertControlModel ( xControlModel, BUTTON_MODEL, msButtonIDs[RESUME_BUTTON],
                             awt::Rectangle( BOX1_BTN_X,
                                             BOX1_BTN_Y + BUTTON_Y_OFFSET + BUTTON_HEIGHT,
                                             BUTTON_WIDTH,
                                             BUTTON_HEIGHT ),
                             aProps );
    }
    {   // abort button
        uno::Sequence< beans::NamedValue > aProps(4);

        setProperty( aProps, 0, UNISTRING("DefaultButton"), uno::Any( sal_Bool(sal_False) ) );
        setProperty( aProps, 1, UNISTRING("Enabled"), uno::Any( sal_Bool(sal_True) ) );
        setProperty( aProps, 2, UNISTRING("PushButtonType"), uno::Any( sal_Int16(awt::PushButtonType_STANDARD) ) );
        setProperty( aProps, 3, UNISTRING("Label"), uno::Any( msCancelBtn ) );

        insertControlModel ( xControlModel, BUTTON_MODEL, msButtonIDs[CANCEL_BUTTON],
                             awt::Rectangle( BOX1_BTN_X,
                                             BOX1_BTN_Y + (2*(BUTTON_HEIGHT+BUTTON_Y_OFFSET)),
                                             BUTTON_WIDTH,
                                             BUTTON_HEIGHT ),
                             aProps );
    }
    {   // Label (FixedText) <description>
        uno::Sequence< beans::NamedValue > aProps(1);

        setProperty( aProps, 0, UNISTRING("Label"), uno::Any( msDescription ) );

        insertControlModel( xControlModel, FIXED_TEXT_MODEL, UNISTRING( "fixedTextDescription" ),
                            awt::Rectangle( DIALOG_BORDER, LABEL_Y_POS, EDIT_WIDTH, LABEL_HEIGHT ),
                            aProps );
    }
    {   // box around <description> text
        uno::Sequence< beans::NamedValue > aProps;

        insertControlModel( xControlModel, GROUP_BOX_MODEL, UNISTRING( "DescriptionBox" ),
                            awt::Rectangle( DIALOG_BORDER, EDIT2_Y_POS, EDIT_WIDTH, BOX_HEIGHT2 ),
                            aProps );
    }
    {   // Text (MultiLineEdit) <description>
        uno::Sequence< beans::NamedValue > aProps(6);

        setProperty( aProps, 0, UNISTRING("Text"), uno::Any( rtl::OUString() ) );
        setProperty( aProps, 1, UNISTRING("Border"), uno::Any( sal_Int16( 0 ) ) );
        setProperty( aProps, 2, UNISTRING("PaintTransparent"), uno::Any( sal_Bool(sal_True) ) );
        setProperty( aProps, 3, UNISTRING("MultiLine"), uno::Any( sal_Bool( sal_True ) ) );
        setProperty( aProps, 4, UNISTRING("ReadOnly"), uno::Any( sal_Bool( sal_True ) ) );
        setProperty( aProps, 5, UNISTRING("AutoVScroll"), uno::Any( sal_Bool( sal_True ) ) );

        insertControlModel( xControlModel, EDIT_FIELD_MODEL, TEXT_DESCRIPTION,
                            awt::Rectangle( DIALOG_BORDER + TEXT_OFFSET,
                                            EDIT2_Y_POS + 2*TEXT_OFFSET,
                                            EDIT_WIDTH - 3*TEXT_OFFSET,
                                            BOX_HEIGHT2 - 3*TEXT_OFFSET ),
                            aProps );
    }
    {   // @see awt/UnoControlFixedLineModel.idl
        uno::Sequence< beans::NamedValue > aProps(1);

        setProperty( aProps, 0, UNISTRING("Orientation"), uno::Any( sal_Int32( 0 ) ) );

        insertControlModel( xControlModel, FIXED_LINE_MODEL, UNISTRING("fixedLine"),
                            awt::Rectangle( DIALOG_BORDER, BUTTON_BAR_Y_POS, EDIT_WIDTH, 5 ),
                            aProps );
    }
    {   // close button // @see awt/UnoControlButtonModel.idl
        uno::Sequence< beans::NamedValue > aProps(4);

        setProperty( aProps, 0, UNISTRING("DefaultButton"), uno::Any( sal_Bool(sal_False) ) );
        setProperty( aProps, 1, UNISTRING("Enabled"), uno::Any( sal_Bool(sal_True) ) );
        // [property] short PushButtonType
        // with own "ButtonActionListener"
        setProperty( aProps, 2, UNISTRING("PushButtonType"), uno::Any( sal_Int16(awt::PushButtonType_STANDARD) ) );
        // with default ActionListener => endDialog().
        // setProperty( aProps, 2, UNISTRING("PushButtonType"), uno::Any( sal_Int16(awt::PushButtonType_CANCEL) ) );
        // [property] string Label // only if PushButtonType_STANDARD
        setProperty( aProps, 3, UNISTRING("Label"), uno::Any( msClose ) );

        insertControlModel ( xControlModel, BUTTON_MODEL, msButtonIDs[ CLOSE_BUTTON ],
                             awt::Rectangle( CLOSE_BTN_X, BUTTON_Y_POS, BUTTON_WIDTH, BUTTON_HEIGHT ),
                             aProps );
    }
    {   // install button
        uno::Sequence< beans::NamedValue > aProps(4);

        setProperty( aProps, 0, UNISTRING("DefaultButton"), uno::Any( sal_Bool(sal_False) ) );
        setProperty( aProps, 1, UNISTRING("Enabled"), uno::Any( sal_Bool(sal_True) ) );
        setProperty( aProps, 2, UNISTRING("PushButtonType"), uno::Any( sal_Int16(awt::PushButtonType_STANDARD) ) );
        setProperty( aProps, 3, UNISTRING("Label"), uno::Any( msInstall ) );

        insertControlModel ( xControlModel, BUTTON_MODEL, msButtonIDs[INSTALL_BUTTON],
                             awt::Rectangle( INSTALL_BTN_X, BUTTON_Y_POS, BUTTON_WIDTH, BUTTON_HEIGHT ),
                             aProps );
    }
    {   // download button
        uno::Sequence< beans::NamedValue > aProps(4);

        setProperty( aProps, 0, UNISTRING("DefaultButton"), uno::Any( sal_Bool(sal_False) ) );
        setProperty( aProps, 1, UNISTRING("Enabled"), uno::Any( sal_Bool(sal_True) ) );
        setProperty( aProps, 2, UNISTRING("PushButtonType"), uno::Any( sal_Int16(awt::PushButtonType_STANDARD) ) );
        setProperty( aProps, 3, UNISTRING("Label"), uno::Any( msDownload ) );

        insertControlModel ( xControlModel, BUTTON_MODEL, msButtonIDs[DOWNLOAD_BUTTON],
                             awt::Rectangle( DOWNLOAD_BTN_X, BUTTON_Y_POS, BUTTON_WIDTH, BUTTON_HEIGHT ),
                             aProps );
    }
    {   // help button
        uno::Sequence< beans::NamedValue > aProps(4);

        setProperty( aProps, 0, UNISTRING("DefaultButton"), uno::Any( sal_Bool(sal_False) ) );
        setProperty( aProps, 1, UNISTRING("Enabled"), uno::Any( sal_Bool(sal_True) ) );
        setProperty( aProps, 2, UNISTRING("PushButtonType"), uno::Any( sal_Int16(awt::PushButtonType_HELP) ) );
        setProperty( aProps, 3, UNISTRING("HelpURL"), uno::Any( UNISTRING( "HID:" ) + rtl::OUString::valueOf( (sal_Int32) HID_CHECK_FOR_UPD_DLG ) ) );

        insertControlModel( xControlModel, BUTTON_MODEL, msButtonIDs[HELP_BUTTON],
                            awt::Rectangle( DIALOG_BORDER, BUTTON_Y_POS, BUTTON_WIDTH, BUTTON_HEIGHT ),
                            aProps );
    }
    {   // @see awt/UnoControlThrobberModel.idl
        uno::Sequence< beans::NamedValue > aProps;

        insertControlModel( xControlModel, UNISTRING("com.sun.star.awt.UnoThrobberControlModel"), CTRL_THROBBER,
                            awt::Rectangle( THROBBER_X_POS, THROBBER_Y_POS, THROBBER_WIDTH, THROBBER_HEIGHT),
                            aProps );
    }
    {    // @see awt/UnoControlProgressBarModel.idl
        uno::Sequence< beans::NamedValue > aProps(4);
        setProperty( aProps, 0, UNISTRING("Enabled"), uno::Any( sal_Bool( sal_True ) ) );
        setProperty( aProps, 1, UNISTRING("ProgressValue"), uno::Any( sal_Int32( 0 ) ) );
        setProperty( aProps, 2, UNISTRING("ProgressValueMax"), uno::Any( sal_Int32( 100 ) ) );
        setProperty( aProps, 3, UNISTRING("ProgressValueMin"), uno::Any( sal_Int32( 0 ) ) );

        insertControlModel( xControlModel, UNISTRING("com.sun.star.awt.UnoControlProgressBarModel"), CTRL_PROGRESS,
                            awt::Rectangle( PROGRESS_X_POS, PROGRESS_Y_POS, PROGRESS_WIDTH, PROGRESS_HEIGHT ),
                            aProps);
    }

    uno::Reference< awt::XControl > xControl(
        xFactory->createInstanceWithContext( UNISTRING("com.sun.star.awt.UnoControlDialog"), mxContext),
        uno::UNO_QUERY_THROW );
    xControl->setModel( xControlModel );

    if ( mbVisible == false )
    {
        uno::Reference< awt::XWindow > xWindow( xControl, uno::UNO_QUERY );

        if ( xWindow.is() )
            xWindow->setVisible( false );
    }

    xControl->createPeer( NULL, NULL );
    {
        uno::Reference< awt::XControlContainer > xContainer (xControl, uno::UNO_QUERY);
        for ( int i = 0; i < HELP_BUTTON; i++ )
        {
            uno::Reference< awt::XButton > xButton ( xContainer->getControl( msButtonIDs[i] ), uno::UNO_QUERY);
            if (xButton.is())
            {
                xButton->setActionCommand( msButtonIDs[i] );
                xButton->addActionListener( this );
            }
        }
    }

    mxUpdDlg.set( xControl, uno::UNO_QUERY_THROW );

    uno::Reference< awt::XTopWindow > xTopWindow( xControl, uno::UNO_QUERY );
    if ( xTopWindow.is() )
        xTopWindow->addTopWindowListener( this );
}

