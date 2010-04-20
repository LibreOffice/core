/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_gui_theextmgr.cxx,v $
 *
 * $Revision: 1.3.14.2 $
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

#include "vos/mutex.hxx"

#include "toolkit/helper/vclunohelper.hxx"

#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/deployment/XPackageManagerFactory.hpp"
#include "com/sun/star/deployment/thePackageManagerFactory.hpp"

#include "dp_gui_dialog2.hxx"
#include "dp_gui_extensioncmdqueue.hxx"
#include "dp_gui_theextmgr.hxx"
#include "dp_gui_theextmgr.hxx"
#include "dp_identifier.hxx"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

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
    m_pUpdReqDialog( NULL )
{
    if ( dp_misc::office_is_running() )
    {
        m_xDesktop.set( xContext->getServiceManager()->createInstanceWithContext(
                            OUSTR("com.sun.star.frame.Desktop"), xContext ), uno::UNO_QUERY );
        if ( m_xDesktop.is() )
            m_xDesktop->addTerminateListener( this );
    }

    m_sPackageManagers.realloc(2);
    m_sPackageManagers[0] = deployment::thePackageManagerFactory::get( m_xContext )->getPackageManager( OUSTR("user") );
    m_sPackageManagers[1] = deployment::thePackageManagerFactory::get( m_xContext )->getPackageManager( OUSTR("shared") );;

    for ( sal_Int32 i = 0; i < m_sPackageManagers.getLength(); ++i )
    {
        m_sPackageManagers[i]->addModifyListener( this );
    }

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
}

//------------------------------------------------------------------------------
TheExtensionManager::~TheExtensionManager()
{
    if ( m_pUpdReqDialog )
        delete m_pUpdReqDialog;
    if ( m_pExtMgrDialog )
        delete m_pExtMgrDialog;
}

//------------------------------------------------------------------------------
void TheExtensionManager::createDialog( const bool bCreateUpdDlg )
{
    const ::vos::OGuard guard( Application::GetSolarMutex() );

    if ( bCreateUpdDlg )
    {
        if ( !m_pUpdReqDialog )
        {
            m_pUpdReqDialog = new UpdateRequiredDialog( NULL, this );
            m_pExecuteCmdQueue.reset( new ExtensionCmdQueue( (DialogHelper*) m_pUpdReqDialog, this, m_xContext ) );
            createPackageList();
        }
    }
    else if ( !m_pExtMgrDialog )
    {
        m_pExtMgrDialog = new ExtMgrDialog( m_pParent, this );
        m_pExecuteCmdQueue.reset( new ExtensionCmdQueue( (DialogHelper*) m_pExtMgrDialog, this, m_xContext ) );
        m_pExtMgrDialog->setGetExtensionsURL( m_sGetExtensionsURL );
        createPackageList();
    }
}

//------------------------------------------------------------------------------
void TheExtensionManager::Show()
{
    const ::vos::OGuard guard( Application::GetSolarMutex() );

    getDialog()->Show();
}

//------------------------------------------------------------------------------
void TheExtensionManager::SetText( const ::rtl::OUString &rTitle )
{
    const ::vos::OGuard guard( Application::GetSolarMutex() );

    getDialog()->SetText( rTitle );
}

//------------------------------------------------------------------------------
void TheExtensionManager::ToTop( USHORT nFlags )
{
    const ::vos::OGuard guard( Application::GetSolarMutex() );

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
    std::vector< TUpdateListEntry > vEntries;

    for ( sal_Int32 i = 0; i < m_sPackageManagers.getLength(); ++i )
    {
        uno::Sequence< uno::Reference< deployment::XPackage > > xPackages;
        try {
            xPackages = m_sPackageManagers[i]->getDeployedPackages( uno::Reference< task::XAbortChannel >(),
                                                                    uno::Reference< ucb::XCommandEnvironment >() );
            for ( sal_Int32 k = 0; k < xPackages.getLength(); ++k )
            {
                TUpdateListEntry pEntry( new UpdateListEntry( xPackages[k], m_sPackageManagers[i] ) );
                vEntries.push_back( pEntry );
            }
        } catch ( deployment::DeploymentException & ) {
            continue;
        } catch ( ucb::CommandFailedException & ) {
            continue;
        } catch ( ucb::CommandAbortedException & ) {
            return true;
        } catch ( lang::IllegalArgumentException & e ) {
            throw uno::RuntimeException( e.Message, e.Context );
        }
    }

    m_pExecuteCmdQueue->checkForUpdates( vEntries );
    return true;
}

//------------------------------------------------------------------------------
bool TheExtensionManager::enablePackage( const uno::Reference< deployment::XPackage > &xPackage,
                                         bool bEnable )
{
    m_pExecuteCmdQueue->enableExtension( xPackage, bEnable );

    return true;
}

//------------------------------------------------------------------------------
bool TheExtensionManager::removePackage( const uno::Reference< deployment::XPackageManager > &xPackageManager,
                                         const uno::Reference< deployment::XPackage > &xPackage )
{
    m_pExecuteCmdQueue->removeExtension( xPackageManager, xPackage );

    return true;
}

//------------------------------------------------------------------------------
bool TheExtensionManager::updatePackages( const std::vector< TUpdateListEntry > &vList )
{
    m_pExecuteCmdQueue->checkForUpdates( vList );

    return true;
}

//------------------------------------------------------------------------------
bool TheExtensionManager::installPackage( const OUString &rPackageURL, bool bWarnUser )
{
    if ( rPackageURL.getLength() == 0 )
        return false;

    createDialog( false );

    uno::Reference< deployment::XPackageManager > xUserPkgMgr = getUserPkgMgr();
    uno::Reference< deployment::XPackageManager > xSharedPkgMgr = getSharedPkgMgr();

    bool bInstall = true;
    bool bInstallForAll = false;

    if ( !bWarnUser && ! xSharedPkgMgr->isReadOnly() )
        bInstall = getDialogHelper()->installForAllUsers( bInstallForAll );

    if ( !bInstall )
        return false;

    if ( bInstallForAll )
        m_pExecuteCmdQueue->addExtension( xSharedPkgMgr, rPackageURL, false );
    else
        m_pExecuteCmdQueue->addExtension( xUserPkgMgr, rPackageURL, bWarnUser );

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
        const ::vos::OGuard guard( Application::GetSolarMutex() );
        delete m_pExtMgrDialog;
        m_pExtMgrDialog = NULL;
        delete m_pUpdReqDialog;
        m_pUpdReqDialog = NULL;
        Application::Quit();
    }
}

//------------------------------------------------------------------------------
bool TheExtensionManager::createPackageList( const uno::Reference< deployment::XPackageManager > &xPackageManager )
{
    uno::Sequence< uno::Reference< deployment::XPackage > > packages;

    try {
        packages = xPackageManager->getDeployedPackages( uno::Reference< task::XAbortChannel >(),
                                                         uno::Reference< ucb::XCommandEnvironment >() );
    } catch ( deployment::DeploymentException & ) {
        //handleGeneralError(e.Cause);
        return true;
    } catch ( ucb::CommandFailedException & ) {
        //handleGeneralError(e.Reason);
        return true;
    } catch ( ucb::CommandAbortedException & ) {
        return false;
    } catch ( lang::IllegalArgumentException & e ) {
        throw uno::RuntimeException( e.Message, e.Context );
    }

    for ( sal_Int32 j = 0; j < packages.getLength(); ++j )
    {
        getDialogHelper()->addPackageToList( packages[j], xPackageManager );
    }

    return true;
}

//------------------------------------------------------------------------------
void TheExtensionManager::createPackageList()
{
    for ( sal_Int32 i = 0; i < m_sPackageManagers.getLength(); ++i )
    {
        if ( ! createPackageList( m_sPackageManagers[i] ) )
            break;
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
            const ::vos::OGuard guard( Application::GetSolarMutex() );
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
void TheExtensionManager::modified( ::lang::EventObject const & rEvt )
    throw ( uno::RuntimeException )
{
    uno::Reference< deployment::XPackageManager > xPackageManager( rEvt.Source, uno::UNO_QUERY );
    if ( xPackageManager.is() )
    {
        getDialogHelper()->prepareChecking( xPackageManager );
        createPackageList( xPackageManager );
        getDialogHelper()->checkEntries();
    }
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

    const ::vos::OGuard guard( Application::GetSolarMutex() );
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

