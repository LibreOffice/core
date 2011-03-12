/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "vcl/svapp.hxx"
#include "vcl/msgbox.hxx"

#include "osl/mutex.hxx"

#include "toolkit/helper/vclunohelper.hxx"

#include "com/sun/star/beans/XPropertySet.hpp"

#include "dp_gui_dialog2.hxx"
#include "dp_gui_extensioncmdqueue.hxx"
#include "dp_gui_theextmgr.hxx"
#include "dp_gui_theextmgr.hxx"
#include "dp_identifier.hxx"
#include "dp_update.hxx"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

#define USER_PACKAGE_MANAGER    OUSTR("user")
#define SHARED_PACKAGE_MANAGER  OUSTR("shared")
#define BUNDLED_PACKAGE_MANAGER OUSTR("bundled")

using namespace ::com::sun::star;
using ::rtl::OUString;

namespace dp_gui {

//------------------------------------------------------------------------------

::rtl::Reference< TheExtensionManager > TheExtensionManager::s_ExtMgr;

//------------------------------------------------------------------------------
//                             TheExtensionManager
//------------------------------------------------------------------------------

TheExtensionManager::TheExtensionManager( Window *pParent,
                                          const uno::Reference< uno::XComponentContext > &xContext ) :
    m_xContext( xContext ),
    m_pParent( pParent ),
    m_pExtMgrDialog( NULL ),
    m_pUpdReqDialog( NULL ),
    m_pExecuteCmdQueue( NULL )
{
    m_xExtensionManager = deployment::ExtensionManager::get( xContext );
    m_xExtensionManager->addModifyListener( this );

    uno::Reference< lang::XMultiServiceFactory > xConfig(
        xContext->getServiceManager()->createInstanceWithContext(
            OUSTR("com.sun.star.configuration.ConfigurationProvider"), xContext ), uno::UNO_QUERY_THROW);
    uno::Any args[1];
    beans::PropertyValue aValue( OUSTR("nodepath"), 0, uno::Any( OUSTR("/org.openoffice.Office.OptionsDialog/Nodes") ),
                                 beans::PropertyState_DIRECT_VALUE );
    args[0] <<= aValue;
    m_xNameAccessNodes = uno::Reference< container::XNameAccess >(
        xConfig->createInstanceWithArguments( OUSTR("com.sun.star.configuration.ConfigurationAccess"),
                                              uno::Sequence< uno::Any >( args, 1 )), uno::UNO_QUERY_THROW);

    // get the 'get more extensions here' url
    uno::Reference< container::XNameAccess > xNameAccessRepositories;
    beans::PropertyValue aValue2( OUSTR("nodepath"), 0, uno::Any( OUSTR("/org.openoffice.Office.ExtensionManager/ExtensionRepositories") ),
                                  beans::PropertyState_DIRECT_VALUE );
    args[0] <<= aValue2;
    xNameAccessRepositories = uno::Reference< container::XNameAccess > (
        xConfig->createInstanceWithArguments( OUSTR("com.sun.star.configuration.ConfigurationAccess"),
                                              uno::Sequence< uno::Any >( args, 1 )), uno::UNO_QUERY_THROW);
    try
    {   //throws css::container::NoSuchElementException, css::lang::WrappedTargetException
        uno::Any value = xNameAccessRepositories->getByName( OUSTR( "WebsiteLink" ) );
        m_sGetExtensionsURL = value.get< OUString > ();
     }
    catch ( uno::Exception& )
    {}

    if ( dp_misc::office_is_running() )
    {
        // the registration should be done after the construction has been ended
        // otherwise an exception prevents object creation, but it is registered as a listener
        m_xDesktop.set( xContext->getServiceManager()->createInstanceWithContext(
                            OUSTR("com.sun.star.frame.Desktop"), xContext ), uno::UNO_QUERY );
        if ( m_xDesktop.is() )
            m_xDesktop->addTerminateListener( this );
    }
}

//------------------------------------------------------------------------------
TheExtensionManager::~TheExtensionManager()
{
    if ( m_pUpdReqDialog )
        delete m_pUpdReqDialog;
    if ( m_pExtMgrDialog )
        delete m_pExtMgrDialog;
    if ( m_pExecuteCmdQueue )
        delete m_pExecuteCmdQueue;
}

//------------------------------------------------------------------------------
void TheExtensionManager::createDialog( const bool bCreateUpdDlg )
{
    const SolarMutexGuard guard;

    if ( bCreateUpdDlg )
    {
        if ( !m_pUpdReqDialog )
        {
            m_pUpdReqDialog = new UpdateRequiredDialog( NULL, this );
            delete m_pExecuteCmdQueue;
            m_pExecuteCmdQueue = new ExtensionCmdQueue( (DialogHelper*) m_pUpdReqDialog, this, m_xContext );
            createPackageList();
        }
    }
    else if ( !m_pExtMgrDialog )
    {
        m_pExtMgrDialog = new ExtMgrDialog( m_pParent, this );
        delete m_pExecuteCmdQueue;
        m_pExecuteCmdQueue = new ExtensionCmdQueue( (DialogHelper*) m_pExtMgrDialog, this, m_xContext );
        m_pExtMgrDialog->setGetExtensionsURL( m_sGetExtensionsURL );
        createPackageList();
    }
}

//------------------------------------------------------------------------------
void TheExtensionManager::Show()
{
    const SolarMutexGuard guard;

    getDialog()->Show();
}

//------------------------------------------------------------------------------
void TheExtensionManager::SetText( const ::rtl::OUString &rTitle )
{
    const SolarMutexGuard guard;

    getDialog()->SetText( rTitle );
}

//------------------------------------------------------------------------------
void TheExtensionManager::ToTop( sal_uInt16 nFlags )
{
    const SolarMutexGuard guard;

    getDialog()->ToTop( nFlags );
}

//------------------------------------------------------------------------------
bool TheExtensionManager::Close()
{
    if ( m_pExtMgrDialog )
        return m_pExtMgrDialog->Close();
    else if ( m_pUpdReqDialog )
        return m_pUpdReqDialog->Close();
    else
        return true;
}

//------------------------------------------------------------------------------
sal_Int16 TheExtensionManager::execute()
{
    sal_Int16 nRet = 0;

    if ( m_pUpdReqDialog )
    {
        nRet = m_pUpdReqDialog->Execute();
        delete m_pUpdReqDialog;
        m_pUpdReqDialog = NULL;
    }

    return nRet;
}

//------------------------------------------------------------------------------
bool TheExtensionManager::isVisible()
{
    return getDialog()->IsVisible();
}

//------------------------------------------------------------------------------
bool TheExtensionManager::checkUpdates( bool /* bShowUpdateOnly */, bool /*bParentVisible*/ )
{
    std::vector< uno::Reference< deployment::XPackage >  > vEntries;
    uno::Sequence< uno::Sequence< uno::Reference< deployment::XPackage > > > xAllPackages;

    try {
        xAllPackages = m_xExtensionManager->getAllExtensions( uno::Reference< task::XAbortChannel >(),
                                                              uno::Reference< ucb::XCommandEnvironment >() );
    } catch ( deployment::DeploymentException & ) {
        return false;
    } catch ( ucb::CommandFailedException & ) {
        return false;
    } catch ( ucb::CommandAbortedException & ) {
        return false;
    } catch ( lang::IllegalArgumentException & e ) {
        throw uno::RuntimeException( e.Message, e.Context );
    }

    for ( sal_Int32 i = 0; i < xAllPackages.getLength(); ++i )
    {
        uno::Reference< deployment::XPackage > xPackage = dp_misc::getExtensionWithHighestVersion(xAllPackages[i]);
        OSL_ASSERT(xPackage.is());
        if ( xPackage.is() )
        {
            vEntries.push_back( xPackage );
        }
    }

    m_pExecuteCmdQueue->checkForUpdates( vEntries );
    return true;
}

//------------------------------------------------------------------------------
bool TheExtensionManager::installPackage( const OUString &rPackageURL, bool bWarnUser )
{
    if ( rPackageURL.getLength() == 0 )
        return false;

    createDialog( false );

    bool bInstall = true;
    bool bInstallForAll = false;

    // DV! missing function is read only repository from extension manager
    if ( !bWarnUser && ! m_xExtensionManager->isReadOnlyRepository( SHARED_PACKAGE_MANAGER ) )
        bInstall = getDialogHelper()->installForAllUsers( bInstallForAll );

    if ( !bInstall )
        return false;

    if ( bInstallForAll )
        m_pExecuteCmdQueue->addExtension( rPackageURL, SHARED_PACKAGE_MANAGER, false );
    else
        m_pExecuteCmdQueue->addExtension( rPackageURL, USER_PACKAGE_MANAGER, bWarnUser );

    return true;
}

//------------------------------------------------------------------------------
bool TheExtensionManager::queryTermination()
{
    if ( dp_misc::office_is_running() )
        return true;
    // the standalone application unopkg must not close ( and quit ) the dialog
    // when there are still actions in the queue
    return true;
}

//------------------------------------------------------------------------------
void TheExtensionManager::terminateDialog()
{
    if ( ! dp_misc::office_is_running() )
    {
        const SolarMutexGuard guard;
        delete m_pExtMgrDialog;
        m_pExtMgrDialog = NULL;
        delete m_pUpdReqDialog;
        m_pUpdReqDialog = NULL;
        Application::Quit();
    }
}

//------------------------------------------------------------------------------
void TheExtensionManager::createPackageList()
{
    uno::Sequence< uno::Sequence< uno::Reference< deployment::XPackage > > > xAllPackages;

    try {
        xAllPackages = m_xExtensionManager->getAllExtensions( uno::Reference< task::XAbortChannel >(),
                                                              uno::Reference< ucb::XCommandEnvironment >() );
    } catch ( deployment::DeploymentException & ) {
        return;
    } catch ( ucb::CommandFailedException & ) {
        return;
    } catch ( ucb::CommandAbortedException & ) {
        return;
    } catch ( lang::IllegalArgumentException & e ) {
        throw uno::RuntimeException( e.Message, e.Context );
    }

    for ( sal_Int32 i = 0; i < xAllPackages.getLength(); ++i )
    {
        uno::Sequence< uno::Reference< deployment::XPackage > > xPackageList = xAllPackages[i];

        for ( sal_Int32 j = 0; j < xPackageList.getLength(); ++j )
        {
            uno::Reference< deployment::XPackage > xPackage = xPackageList[j];
            if ( xPackage.is() )
            {
                PackageState eState = getPackageState( xPackage );
                getDialogHelper()->addPackageToList( xPackage );
                // When the package is enabled, we can stop here, otherwise we have to look for
                // another version of this package
                if ( ( eState == REGISTERED ) || ( eState == NOT_AVAILABLE ) )
                    break;
            }
        }
    }

    uno::Sequence< uno::Reference< deployment::XPackage > > xNoLicPackages;
    xNoLicPackages = m_xExtensionManager->getExtensionsWithUnacceptedLicenses( SHARED_PACKAGE_MANAGER,
                                                                               uno::Reference< ucb::XCommandEnvironment >() );
    for ( sal_Int32 i = 0; i < xNoLicPackages.getLength(); ++i )
    {
        uno::Reference< deployment::XPackage > xPackage = xNoLicPackages[i];
        if ( xPackage.is() )
        {
            getDialogHelper()->addPackageToList( xPackage, true );
        }
    }
}

//------------------------------------------------------------------------------
PackageState TheExtensionManager::getPackageState( const uno::Reference< deployment::XPackage > &xPackage ) const
{
    try {
        beans::Optional< beans::Ambiguous< sal_Bool > > option(
            xPackage->isRegistered( uno::Reference< task::XAbortChannel >(),
                                    uno::Reference< ucb::XCommandEnvironment >() ) );
        if ( option.IsPresent )
        {
            ::beans::Ambiguous< sal_Bool > const & reg = option.Value;
            if ( reg.IsAmbiguous )
                return AMBIGUOUS;
            else
                return reg.Value ? REGISTERED : NOT_REGISTERED;
        }
        else
            return NOT_AVAILABLE;
    }
    catch ( uno::RuntimeException & ) {
        throw;
    }
    catch ( uno::Exception & exc) {
        (void) exc;
        OSL_ENSURE( 0, ::rtl::OUStringToOString( exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        return NOT_AVAILABLE;
    }
}

//------------------------------------------------------------------------------
bool TheExtensionManager::isReadOnly( const uno::Reference< deployment::XPackage > &xPackage ) const
{
    if ( m_xExtensionManager.is() && xPackage.is() )
    {
        return m_xExtensionManager->isReadOnlyRepository( xPackage->getRepositoryName() );
    }
    else
        return true;
}

//------------------------------------------------------------------------------
// The function investigates if the extension supports options.
bool TheExtensionManager::supportsOptions( const uno::Reference< deployment::XPackage > &xPackage ) const
{
    bool bOptions = false;

    if ( ! xPackage->isBundle() )
        return false;

    beans::Optional< OUString > aId = xPackage->getIdentifier();

    //a bundle must always have an id
    OSL_ASSERT( aId.IsPresent );

    //iterate over all available nodes
    uno::Sequence< OUString > seqNames = m_xNameAccessNodes->getElementNames();

    for ( int i = 0; i < seqNames.getLength(); i++ )
    {
        uno::Any anyNode = m_xNameAccessNodes->getByName( seqNames[i] );
        //If we have a node then then it must contain the set of leaves. This is part of OptionsDialog.xcs
        uno::Reference< XInterface> xIntNode = anyNode.get< uno::Reference< XInterface > >();
        uno::Reference< container::XNameAccess > xNode( xIntNode, uno::UNO_QUERY_THROW );

        uno::Any anyLeaves = xNode->getByName( OUSTR("Leaves") );
        uno::Reference< XInterface > xIntLeaves = anyLeaves.get< uno::Reference< XInterface > >();
        uno::Reference< container::XNameAccess > xLeaves( xIntLeaves, uno::UNO_QUERY_THROW );

        //iterate over all available leaves
        uno::Sequence< OUString > seqLeafNames = xLeaves->getElementNames();
        for ( int j = 0; j < seqLeafNames.getLength(); j++ )
        {
            uno::Any anyLeaf = xLeaves->getByName( seqLeafNames[j] );
            uno::Reference< XInterface > xIntLeaf = anyLeaf.get< uno::Reference< XInterface > >();
            uno::Reference< beans::XPropertySet > xLeaf( xIntLeaf, uno::UNO_QUERY_THROW );
            //investigate the Id property if it matches the extension identifier which
            //has been passed in.
            uno::Any anyValue = xLeaf->getPropertyValue( OUSTR("Id") );

            OUString sId = anyValue.get< OUString >();
            if ( sId == aId.Value )
            {
                bOptions = true;
                break;
            }
        }
        if ( bOptions )
            break;
    }
    return bOptions;
}

//------------------------------------------------------------------------------
// XEventListener
void TheExtensionManager::disposing( lang::EventObject const & rEvt )
    throw ( uno::RuntimeException )
{
    bool shutDown = (rEvt.Source == m_xDesktop);

    if ( shutDown && m_xDesktop.is() )
    {
        m_xDesktop->removeTerminateListener( this );
        m_xDesktop.clear();
    }

    if ( shutDown )
    {
        if ( dp_misc::office_is_running() )
        {
            const SolarMutexGuard guard;
            delete m_pExtMgrDialog;
            m_pExtMgrDialog = NULL;
            delete m_pUpdReqDialog;
            m_pUpdReqDialog = NULL;
        }
        s_ExtMgr.clear();
    }
}

//------------------------------------------------------------------------------
// XTerminateListener
void TheExtensionManager::queryTermination( ::lang::EventObject const & )
    throw ( frame::TerminationVetoException, uno::RuntimeException )
{
    DialogHelper *pDialogHelper = getDialogHelper();

    if ( m_pExecuteCmdQueue->isBusy() || ( pDialogHelper && pDialogHelper->isBusy() ) )
    {
        ToTop( TOTOP_RESTOREWHENMIN );
        throw frame::TerminationVetoException(
            OUSTR("The office cannot be closed while the Extension Manager is running"),
            uno::Reference<XInterface>(static_cast<frame::XTerminateListener*>(this), uno::UNO_QUERY));
    }
    else
    {
        if ( m_pExtMgrDialog )
            m_pExtMgrDialog->Close();
        if ( m_pUpdReqDialog )
            m_pUpdReqDialog->Close();
    }
}

//------------------------------------------------------------------------------
void TheExtensionManager::notifyTermination( ::lang::EventObject const & rEvt )
    throw ( uno::RuntimeException )
{
    disposing( rEvt );
}

//------------------------------------------------------------------------------
// XModifyListener
void TheExtensionManager::modified( ::lang::EventObject const & /*rEvt*/ )
    throw ( uno::RuntimeException )
{
    getDialogHelper()->prepareChecking();
    createPackageList();
    getDialogHelper()->checkEntries();
}

//------------------------------------------------------------------------------
::rtl::Reference< TheExtensionManager > TheExtensionManager::get( const uno::Reference< uno::XComponentContext > &xContext,
                                                                  const uno::Reference< awt::XWindow > &xParent,
                                                                  const OUString & extensionURL )
{
    if ( s_ExtMgr.is() )
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        if ( extensionURL.getLength() )
            s_ExtMgr->installPackage( extensionURL, true );
        return s_ExtMgr;
    }

    Window * pParent = DIALOG_NO_PARENT;
    if ( xParent.is() )
        pParent = VCLUnoHelper::GetWindow(xParent);

    ::rtl::Reference<TheExtensionManager> that( new TheExtensionManager( pParent, xContext ) );

    const SolarMutexGuard guard;
    if ( ! s_ExtMgr.is() )
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        s_ExtMgr = that;
    }

    if ( extensionURL.getLength() )
        s_ExtMgr->installPackage( extensionURL, true );

    return s_ExtMgr;
}

} //namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
