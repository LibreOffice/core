/*************************************************************************
 *
 *  $RCSfile: dlgprov.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-15 13:27:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SCRIPTING_DLGPROV_HXX
#include "dlgprov.hxx"
#endif

#ifndef SCRIPTING_DLGEVTATT_HXX
#include "dlgevtatt.hxx"
#endif

#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOWPEER_HPP_
#include <com/sun/star/awt/XWindowPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAMPROVIDER_HXX_
#include <com/sun/star/io/XInputStreamProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTICOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINER_HPP_
#include <com/sun/star/script/XLibraryContainer.hpp>
#endif
#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _XMLSCRIPT_XMLDLG_IMEXP_HXX_
#include <xmlscript/xmldlg_imexp.hxx>
#endif

#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;


//.........................................................................
namespace dlgprov
{
//.........................................................................

    // =============================================================================
    // component operations
    // =============================================================================

    static ::rtl::OUString getImplementationName_DialogProviderImpl()
    {
        static ::rtl::OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static ::rtl::OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.scripting.DialogProvider" ) );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    // -----------------------------------------------------------------------------

    static Sequence< ::rtl::OUString > getSupportedServiceNames_DialogProviderImpl()
    {
        static Sequence< ::rtl::OUString >* pNames = 0;
        if ( !pNames )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pNames )
            {
                static Sequence< ::rtl::OUString > aNames(1);
                aNames.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.DialogProvider" ) );
                pNames = &aNames;
            }
        }
        return *pNames;
    }


    // =============================================================================
    // mutex
    // =============================================================================

    ::osl::Mutex& getMutex()
    {
        static ::osl::Mutex* s_pMutex = 0;
        if ( !s_pMutex )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !s_pMutex )
            {
                static ::osl::Mutex s_aMutex;
                s_pMutex = &s_aMutex;
            }
        }
        return *s_pMutex;
    }


    // =============================================================================
    // DialogProviderImpl
    // =============================================================================

    DialogProviderImpl::DialogProviderImpl( const Reference< XComponentContext >& rxContext )
        :m_xContext( rxContext )
        ,m_xModel( 0 )
    {
    }

    // -----------------------------------------------------------------------------

    DialogProviderImpl::~DialogProviderImpl()
    {
    }

    // -----------------------------------------------------------------------------


    Reference< XControlModel > DialogProviderImpl::createDialogModel( const ::rtl::OUString& sURL )
    {
        // parse URL
        // TODO: use URL parsing class
        // TODO: decoding of location
        Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager(), UNO_QUERY );

        if ( !xSMgr.is() )
        {
            throw RuntimeException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DialogProviderImpl::getDialogModel: Couldn't instantiate MultiComponent factory" ) ),
                    Reference< XInterface >() );
        }

        Reference< uri::XUriReferenceFactory > xFac (
            xSMgr->createInstanceWithContext( rtl::OUString::createFromAscii(
            "com.sun.star.uri.UriReferenceFactory"), m_xContext ) , UNO_QUERY );

        if  ( !xFac.is() )
        {
            throw RuntimeException(
                ::rtl::OUString::createFromAscii( "DialogProviderImpl::getDialogModel(), could not instatiate UriReferenceFactory." ),
                Reference< XInterface >() );
        }

        Reference<  uri::XUriReference > uriRef(
            xFac->parse( sURL ), UNO_QUERY );

        Reference < uri::XVndSunStarScriptUrl > sfUri( uriRef, UNO_QUERY );

        if ( !uriRef.is() || !sfUri.is() )
        {
            ::rtl::OUString errorMsg = ::rtl::OUString::createFromAscii( "DialogProviderImpl::getDialogModel: failed to parse URI: " );
            errorMsg.concat( sURL );
            throw IllegalArgumentException( errorMsg,
                Reference< XInterface >(), 1 );
        }

        ::rtl::OUString sDescription = sfUri->getName();

        sal_Int32 nIndex = 0;

        ::rtl::OUString sLibName = sDescription.getToken( 0, (sal_Unicode)'.', nIndex );
        ::rtl::OUString sDlgName;
        if ( nIndex != -1 )
            sDlgName = sDescription.getToken( 0, (sal_Unicode)'.', nIndex );

        ::rtl::OUString sLocation = sfUri->getParameter(
            ::rtl::OUString::createFromAscii( "location" ) );


        // get dialog library container
        // TODO: dialogs in packages
        Reference< XLibraryContainer > xLibContainer;

        if ( sLocation == ::rtl::OUString::createFromAscii( "application" ) )
        {
            xLibContainer = Reference< XLibraryContainer >( SFX_APP()->GetDialogContainer(), UNO_QUERY );
        }
        else if ( sLocation == ::rtl::OUString::createFromAscii( "document" ) )
        {
            if ( m_xModel.is() )
            {
                for ( SfxObjectShell* pObjShell = SfxObjectShell::GetFirst(); pObjShell; pObjShell = SfxObjectShell::GetNext( *pObjShell ) )
                {
                    if ( m_xModel == pObjShell->GetModel() )
                    {
                        xLibContainer = Reference< XLibraryContainer >( pObjShell->GetDialogContainer(), UNO_QUERY );
                        break;
                    }
                }
            }
        }
        else
        {
            for ( SfxObjectShell* pObjShell = SfxObjectShell::GetFirst(); pObjShell; pObjShell = SfxObjectShell::GetNext( *pObjShell ) )
            {
                Reference< frame::XModel > xModel( pObjShell->GetModel(), UNO_QUERY );
                if ( xModel.is() )
                {
                    ::rtl::OUString sDocURL = xModel->getURL();
                    if ( sDocURL.getLength() == 0 )
                    {
                        Sequence < beans::PropertyValue > aProps = xModel->getArgs();
                        sal_Int32 nProps = aProps.getLength();
                        const beans::PropertyValue* pProps = aProps.getConstArray();
                        for ( sal_Int32 i = 0; i < nProps; ++i )
                        {
                            // TODO: according to MBA the property 'Title' may change in future
                            if ( pProps[i].Name == ::rtl::OUString::createFromAscii( "Title" ) )
                            {
                                pProps[i].Value >>= sDocURL;
                                break;
                            }
                        }
                    }
                    if ( sLocation == sDocURL )
                    {
                        xLibContainer = Reference< XLibraryContainer >( pObjShell->GetDialogContainer(), UNO_QUERY );
                        break;
                    }
                }
            }
        }

        // get input stream provider
        Reference< io::XInputStreamProvider > xISP;

        if ( xLibContainer.is() )
        {
            // load dialog library
            if ( !xLibContainer->isLibraryLoaded( sLibName ) )
                xLibContainer->loadLibrary( sLibName );

            // get dialog library
            Reference< container::XNameContainer > xLib;
            if ( xLibContainer->hasByName( sLibName ) )
            {
                Any aElement = xLibContainer->getByName( sLibName );
                aElement >>= xLib;
            }

            if ( xLib.is() )
            {
                // get input stream provider
                if ( xLib->hasByName( sDlgName ) )
                {
                    Any aElement = xLib->getByName( sDlgName );
                    aElement >>= xISP;
                }

                if ( !xISP.is() )
                {
                    throw IllegalArgumentException(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DialogProviderImpl::getDialogModel: dialog not found!" ) ),
                        Reference< XInterface >(), 1 );
                }
            }
            else
            {
                throw IllegalArgumentException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DialogProviderImpl::getDialogModel: library not found!" ) ),
                    Reference< XInterface >(), 1 );
            }
        }
        else
        {
            throw IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DialogProviderImpl::getDialog: library container not found!" ) ),
                Reference< XInterface >(), 1 );
        }

        // import dialog model
        Reference< XControlModel > xCtrlModel;
        if ( xISP.is() )
        {
            Reference< io::XInputStream > xInput( xISP->createInputStream() );
            if ( xInput.is() && m_xContext.is() )
            {
                Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager() );
                if ( xSMgr.is() )
                {
                    Reference< container::XNameContainer > xDialogModel( xSMgr->createInstanceWithContext(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ), m_xContext ), UNO_QUERY );

                    if ( xDialogModel.is() )
                    {
                        ::xmlscript::importDialogModel( xInput, xDialogModel, m_xContext );
                        xCtrlModel = Reference< XControlModel >( xDialogModel, UNO_QUERY );
                    }
                }
            }
        }

        return xCtrlModel;
    }

    // -----------------------------------------------------------------------------

    Reference< XControl > DialogProviderImpl::createDialogControl( const Reference< XControlModel >& rxDialogModel )
    {
        OSL_ENSURE( rxDialogModel.is(), "DialogProviderImpl::getDialogControl: no dialog model" );

        Reference< XControl > xDialogControl;

        if ( m_xContext.is() )
        {
            Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager() );

            if ( xSMgr.is() )
            {
                xDialogControl = Reference< XControl >( xSMgr->createInstanceWithContext(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialog" ) ), m_xContext ), UNO_QUERY );

                if ( xDialogControl.is() )
                {
                    // set the model
                    if ( rxDialogModel.is() )
                        xDialogControl->setModel( rxDialogModel );

                    // set visible
                    Reference< XWindow > xW( xDialogControl, UNO_QUERY );
                    if ( xW.is() )
                        xW->setVisible( sal_False );

                    // get the parent of the dialog control
                    Reference< XWindowPeer > xPeer;
                    if ( m_xModel.is() )
                    {
                        Reference< frame::XController > xController( m_xModel->getCurrentController(), UNO_QUERY );
                        if ( xController.is() )
                        {
                            Reference< frame::XFrame > xFrame( xController->getFrame(), UNO_QUERY );
                            if ( xFrame.is() )
                                xPeer = Reference< XWindowPeer>( xFrame->getContainerWindow(), UNO_QUERY );
                        }
                    }

                    // create a peer
                    Reference< XToolkit> xToolkit( xSMgr->createInstanceWithContext(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.Toolkit" ) ), m_xContext ), UNO_QUERY );
                    if ( xToolkit.is() )
                        xDialogControl->createPeer( xToolkit, xPeer );
                }
            }
        }

        return xDialogControl;
    }

    // -----------------------------------------------------------------------------

    void DialogProviderImpl::attachDialogEvents( const Reference< XControl >& rxDialogControl )
    {
        if ( rxDialogControl.is() )
        {
            Reference< XControlContainer > xControlContainer( rxDialogControl, UNO_QUERY );

            if ( xControlContainer.is() )
            {
                Sequence< Reference< XControl > > aControls = xControlContainer->getControls();
                const Reference< XControl >* pControls = aControls.getConstArray();
                sal_Int32 nControlCount = aControls.getLength();

                Sequence< Reference< XInterface > > aObjects( nControlCount + 1 );
                Reference< XInterface >* pObjects = aObjects.getArray();
                for ( sal_Int32 i = 0; i < nControlCount; ++i )
                {
                    pObjects[i] = Reference< XInterface >( pControls[i], UNO_QUERY );
                }

                // also add the dialog control itself to the sequence
                pObjects[nControlCount] = Reference< XInterface >( rxDialogControl, UNO_QUERY );

                Reference< XScriptListener > xScriptListener = new DialogScriptListenerImpl( m_xContext, m_xModel );

                if ( xScriptListener.is() )
                {
                    if ( !m_xScriptEventsAttacher.is() )
                        m_xScriptEventsAttacher = new DialogEventsAttacherImpl( m_xContext );

                    if ( m_xScriptEventsAttacher.is() )
                    {
                        Any aHelper;
                        m_xScriptEventsAttacher->attachEvents( aObjects, xScriptListener, aHelper );
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    ::rtl::OUString DialogProviderImpl::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_DialogProviderImpl();
    }

    // -----------------------------------------------------------------------------

    sal_Bool DialogProviderImpl::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
        const ::rtl::OUString* pNames = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pNames + aNames.getLength();
        for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
            ;

        return pNames != pEnd;
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > DialogProviderImpl::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_DialogProviderImpl();
    }

    // -----------------------------------------------------------------------------
    // XInitialization
    // -----------------------------------------------------------------------------

    void DialogProviderImpl::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( getMutex() );

        if ( aArguments.getLength() == 1 )
        {
            aArguments[0] >>= m_xModel;

            if ( !m_xModel.is() )
            {
                throw RuntimeException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DialogProviderImpl::initialize: invalid argument format!" ) ),
                    Reference< XInterface >() );
            }
        }
        else if ( aArguments.getLength() > 1 )
        {
            throw RuntimeException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DialogProviderImpl::initialize: invalid number of arguments!" ) ),
                Reference< XInterface >() );
        }
    }

    // -----------------------------------------------------------------------------
    // XDialogProvider
    // -----------------------------------------------------------------------------

    Reference < XDialog > DialogProviderImpl::createDialog( const ::rtl::OUString& URL )
        throw (IllegalArgumentException, RuntimeException)
    {
        // if the dialog is located in a document, the document must already be open!

        ::osl::MutexGuard aGuard( getMutex() );

        OSL_ENSURE( URL.getLength(), "DialogProviderImpl::getDialog: no URL!" );

        Reference< XDialog > xDialog;
        Reference< XControlModel > xCtrlMod( createDialogModel( URL ) );
        if ( xCtrlMod.is() )
        {
            Reference< XControl > xCtrl( createDialogControl( xCtrlMod ) );
            if ( xCtrl.is() )
            {
                attachDialogEvents( xCtrl );
                xDialog = Reference< XDialog >( xCtrl, UNO_QUERY );
            }
        }

        return xDialog;
    }

    // =============================================================================
    // component operations
    // =============================================================================

    static Reference< XInterface > SAL_CALL create_DialogProviderImpl(
        Reference< XComponentContext > const & xContext )
        SAL_THROW( () )
    {
        return static_cast< lang::XTypeProvider * >( new DialogProviderImpl( xContext ) );
    }

    // -----------------------------------------------------------------------------

    static struct ::cppu::ImplementationEntry s_component_entries [] =
    {
        {
            create_DialogProviderImpl, getImplementationName_DialogProviderImpl,
            getSupportedServiceNames_DialogProviderImpl, ::cppu::createSingleComponentFactory,
            0, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace dlgprov
//.........................................................................


// =============================================================================
// component exports
// =============================================================================

extern "C"
{
    void SAL_CALL component_getImplementationEnvironment(
        const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL component_writeInfo(
        lang::XMultiServiceFactory * pServiceManager, registry::XRegistryKey * pRegistryKey )
    {
        return ::cppu::component_writeInfoHelper(
            pServiceManager, pRegistryKey, ::dlgprov::s_component_entries );
    }

    void * SAL_CALL component_getFactory(
        const sal_Char * pImplName, lang::XMultiServiceFactory * pServiceManager,
        registry::XRegistryKey * pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper(
            pImplName, pServiceManager, pRegistryKey, ::dlgprov::s_component_entries );
    }
}
