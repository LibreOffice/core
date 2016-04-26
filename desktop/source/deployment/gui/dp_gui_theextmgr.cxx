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

#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

#include <osl/mutex.hxx>

#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/frame/Desktop.hpp>

#include "dp_gui_dialog2.hxx"
#include "dp_gui_extensioncmdqueue.hxx"
#include "dp_gui_theextmgr.hxx"
#include "dp_identifier.hxx"
#include "dp_update.hxx"

#define USER_PACKAGE_MANAGER    "user"
#define SHARED_PACKAGE_MANAGER  "shared"

using namespace ::com::sun::star;

namespace dp_gui {


::rtl::Reference< TheExtensionManager > TheExtensionManager::s_ExtMgr;


//                             TheExtensionManager


TheExtensionManager::TheExtensionManager( const uno::Reference< awt::XWindow > &xParent,
                                          const uno::Reference< uno::XComponentContext > &xContext ) :
    m_xContext( xContext ),
    m_xParent( xParent ),
    m_pExtMgrDialog( nullptr ),
    m_pUpdReqDialog( nullptr ),
    m_pExecuteCmdQueue( nullptr )
{
    m_xExtensionManager = deployment::ExtensionManager::get( xContext );
    m_xExtensionManager->addModifyListener( this );

    uno::Reference< lang::XMultiServiceFactory > xConfig(
        configuration::theDefaultProvider::get(xContext));
    uno::Any args[1];
    beans::PropertyValue aValue( OUString("nodepath"), 0, uno::Any( OUString("/org.openoffice.Office.OptionsDialog/Nodes") ),
                                 beans::PropertyState_DIRECT_VALUE );
    args[0] <<= aValue;
    m_xNameAccessNodes.set(
        xConfig->createInstanceWithArguments( "com.sun.star.configuration.ConfigurationAccess",
                                              uno::Sequence< uno::Any >( args, 1 )),
        uno::UNO_QUERY_THROW);

    // get the 'get more extensions here' url
    uno::Reference< container::XNameAccess > xNameAccessRepositories;
    beans::PropertyValue aValue2( OUString("nodepath"), 0, uno::Any( OUString("/org.openoffice.Office.ExtensionManager/ExtensionRepositories") ),
                                  beans::PropertyState_DIRECT_VALUE );
    args[0] <<= aValue2;
    xNameAccessRepositories.set(
        xConfig->createInstanceWithArguments( "com.sun.star.configuration.ConfigurationAccess",
                                              uno::Sequence< uno::Any >( args, 1 )),
        uno::UNO_QUERY_THROW);
    try
    {   //throws css::container::NoSuchElementException, css::lang::WrappedTargetException
        uno::Any value = xNameAccessRepositories->getByName("WebsiteLink");
        m_sGetExtensionsURL = value.get< OUString > ();
     }
    catch ( const uno::Exception& )
    {}

    if ( dp_misc::office_is_running() )
    {
        // the registration should be done after the construction has been ended
        // otherwise an exception prevents object creation, but it is registered as a listener
        m_xDesktop.set( frame::Desktop::create(xContext), uno::UNO_QUERY_THROW );
        m_xDesktop->addTerminateListener( this );
    }
}


TheExtensionManager::~TheExtensionManager()
{
    m_pUpdReqDialog.disposeAndClear();
    m_pExtMgrDialog.disposeAndClear();
    delete m_pExecuteCmdQueue;
}


void TheExtensionManager::createDialog( const bool bCreateUpdDlg )
{
    const SolarMutexGuard guard;

    if ( bCreateUpdDlg )
    {
        if ( !m_pUpdReqDialog )
        {
            m_pUpdReqDialog = VclPtr<UpdateRequiredDialog>::Create( nullptr, this );
            delete m_pExecuteCmdQueue;
            m_pExecuteCmdQueue = new ExtensionCmdQueue( m_pUpdReqDialog.get(), this, m_xContext );
            createPackageList();
        }
    }
    else if ( !m_pExtMgrDialog )
    {
        if (m_xParent.is())
            m_pExtMgrDialog = VclPtr<ExtMgrDialog>::Create( VCLUnoHelper::GetWindow(m_xParent), this );
        else
            m_pExtMgrDialog = VclPtr<ExtMgrDialog>::Create( nullptr, this, Dialog::InitFlag::NoParent );
        delete m_pExecuteCmdQueue;
        m_pExecuteCmdQueue = new ExtensionCmdQueue( m_pExtMgrDialog.get(), this, m_xContext );
        m_pExtMgrDialog->setGetExtensionsURL( m_sGetExtensionsURL );
        createPackageList();
    }
}


void TheExtensionManager::Show()
{
    const SolarMutexGuard guard;

    getDialog()->Show();
}


void TheExtensionManager::SetText( const OUString &rTitle )
{
    const SolarMutexGuard guard;

    getDialog()->SetText( rTitle );
}


void TheExtensionManager::ToTop( ToTopFlags nFlags )
{
    const SolarMutexGuard guard;

    getDialog()->ToTop( nFlags );
}


bool TheExtensionManager::Close()
{
    if ( m_pExtMgrDialog )
        return m_pExtMgrDialog->Close();
    else if ( m_pUpdReqDialog )
        return m_pUpdReqDialog->Close();
    else
        return true;
}


sal_Int16 TheExtensionManager::execute()
{
    sal_Int16 nRet = 0;

    if ( m_pUpdReqDialog )
    {
        nRet = m_pUpdReqDialog->Execute();
        m_pUpdReqDialog.disposeAndClear();
    }

    return nRet;
}


bool TheExtensionManager::isVisible()
{
    return getDialog()->IsVisible();
}


bool TheExtensionManager::checkUpdates( bool /* bShowUpdateOnly */, bool /*bParentVisible*/ )
{
    std::vector< uno::Reference< deployment::XPackage >  > vEntries;
    uno::Sequence< uno::Sequence< uno::Reference< deployment::XPackage > > > xAllPackages;

    try {
        xAllPackages = m_xExtensionManager->getAllExtensions( uno::Reference< task::XAbortChannel >(),
                                                              uno::Reference< ucb::XCommandEnvironment >() );
    } catch ( const deployment::DeploymentException & ) {
        return false;
    } catch ( const ucb::CommandFailedException & ) {
        return false;
    } catch ( const ucb::CommandAbortedException & ) {
        return false;
    } catch ( const lang::IllegalArgumentException & e ) {
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


bool TheExtensionManager::installPackage( const OUString &rPackageURL, bool bWarnUser )
{
    if ( rPackageURL.isEmpty() )
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


bool TheExtensionManager::queryTermination()
{
    if ( dp_misc::office_is_running() )
        return true;
    // the standalone application unopkg must not close ( and quit ) the dialog
    // when there are still actions in the queue
    return true;
}


void TheExtensionManager::terminateDialog()
{
    if ( ! dp_misc::office_is_running() )
    {
        const SolarMutexGuard guard;
        m_pExtMgrDialog.disposeAndClear();
        m_pUpdReqDialog.disposeAndClear();
        Application::Quit();
    }
}


void TheExtensionManager::createPackageList()
{
    uno::Sequence< uno::Sequence< uno::Reference< deployment::XPackage > > > xAllPackages;

    try {
        xAllPackages = m_xExtensionManager->getAllExtensions( uno::Reference< task::XAbortChannel >(),
                                                              uno::Reference< ucb::XCommandEnvironment >() );
    } catch ( const deployment::DeploymentException & ) {
        return;
    } catch ( const ucb::CommandFailedException & ) {
        return;
    } catch ( const ucb::CommandAbortedException & ) {
        return;
    } catch ( const lang::IllegalArgumentException & e ) {
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


PackageState TheExtensionManager::getPackageState( const uno::Reference< deployment::XPackage > &xPackage )
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
    catch ( const uno::RuntimeException & ) {
        throw;
    }
    catch (const uno::Exception & exc) {
        (void) exc;
        OSL_FAIL( OUStringToOString( exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        return NOT_AVAILABLE;
    }
}


bool TheExtensionManager::isReadOnly( const uno::Reference< deployment::XPackage > &xPackage ) const
{
    if ( m_xExtensionManager.is() && xPackage.is() )
    {
        return m_xExtensionManager->isReadOnlyRepository( xPackage->getRepositoryName() );
    }
    else
        return true;
}


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
        //If we have a node then it must contain the set of leaves. This is part of OptionsDialog.xcs
        uno::Reference< XInterface> xIntNode = anyNode.get< uno::Reference< XInterface > >();
        uno::Reference< container::XNameAccess > xNode( xIntNode, uno::UNO_QUERY_THROW );

        uno::Any anyLeaves = xNode->getByName("Leaves");
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
            uno::Any anyValue = xLeaf->getPropertyValue("Id");

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


// XEventListener
void TheExtensionManager::disposing( lang::EventObject const & rEvt )
    throw ( uno::RuntimeException, std::exception )
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
            m_pExtMgrDialog.disposeAndClear();
            m_pUpdReqDialog.disposeAndClear();
        }
        s_ExtMgr.clear();
    }
}


// XTerminateListener
void TheExtensionManager::queryTermination( ::lang::EventObject const & )
    throw ( frame::TerminationVetoException, uno::RuntimeException, std::exception )
{
    DialogHelper *pDialogHelper = getDialogHelper();

    if ( m_pExecuteCmdQueue->isBusy() || ( pDialogHelper && pDialogHelper->isBusy() ) )
    {
        ToTop( ToTopFlags::RestoreWhenMin );
        throw frame::TerminationVetoException(
            "The office cannot be closed while the Extension Manager is running",
            static_cast<frame::XTerminateListener*>(this));
    }
    else
    {
        if ( m_pExtMgrDialog )
            m_pExtMgrDialog->Close();
        if ( m_pUpdReqDialog )
            m_pUpdReqDialog->Close();
    }
}


void TheExtensionManager::notifyTermination( ::lang::EventObject const & rEvt )
    throw ( uno::RuntimeException, std::exception )
{
    disposing( rEvt );
}


// XModifyListener
void TheExtensionManager::modified( ::lang::EventObject const & /*rEvt*/ )
    throw ( uno::RuntimeException, std::exception )
{
    getDialogHelper()->prepareChecking();
    createPackageList();
    getDialogHelper()->checkEntries();
}


::rtl::Reference< TheExtensionManager > TheExtensionManager::get( const uno::Reference< uno::XComponentContext > &xContext,
                                                                  const uno::Reference< awt::XWindow > &xParent,
                                                                  const OUString & extensionURL )
{
    if ( s_ExtMgr.is() )
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        if ( !extensionURL.isEmpty() )
            s_ExtMgr->installPackage( extensionURL, true );
        return s_ExtMgr;
    }

    ::rtl::Reference<TheExtensionManager> that( new TheExtensionManager( xParent, xContext ) );

    const SolarMutexGuard guard;
    if ( ! s_ExtMgr.is() )
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        s_ExtMgr = that;
    }

    if ( !extensionURL.isEmpty() )
        s_ExtMgr->installPackage( extensionURL, true );

    return s_ExtMgr;
}

} //namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
