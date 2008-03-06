/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: facreg.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:59:14 $
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
#include "precompiled_sd.hxx"

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _UNO_LBNAMES_H_
#include <uno/lbnames.h>
#endif

#ifndef SFX2_SFXMODELFACTORY_HXX
#include <sfx2/sfxmodelfactory.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include "osl/diagnose.h"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef INCLUDED_STRING_H
#include <string.h>
#define INCLUDED_STRING_H
#endif
#include <comphelper/stl_types.hxx>
#include <hash_map>
#include <boost/shared_ptr.hpp>

using namespace rtl;
using namespace com::sun::star;

// Forward declarations of the factories.

extern uno::Reference< uno::XInterface > SAL_CALL SdDrawingDocument_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory, const sal_uInt64 _nCreationFlags );
extern OUString SdDrawingDocument_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL SdDrawingDocument_getSupportedServiceNames() throw( uno::RuntimeException );

extern uno::Reference< uno::XInterface > SAL_CALL SdPresentationDocument_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory, const sal_uInt64 _nCreationFlags );
extern OUString SdPresentationDocument_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL SdPresentationDocument_getSupportedServiceNames() throw( uno::RuntimeException );

extern uno::Reference< uno::XInterface > SAL_CALL SdHtmlOptionsDialog_CreateInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory );
extern OUString SdHtmlOptionsDialog_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL SdHtmlOptionsDialog_getSupportedServiceNames() throw( uno::RuntimeException );

extern uno::Reference< uno::XInterface > SAL_CALL SdUnoModule_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory );
extern OUString SdUnoModule_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL SdUnoModule_getSupportedServiceNames() throw( uno::RuntimeException );

namespace sd
{
extern uno::Reference< uno::XInterface > SAL_CALL RandomNode_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory );
extern OUString RandomNode__getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL RandomNode_getSupportedServiceNames() throw( uno::RuntimeException );
}

namespace sd { namespace framework {
extern uno::Reference<uno::XInterface> SAL_CALL PaneController_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString PaneController_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL PaneController_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL ConfigurationController_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString ConfigurationController_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL ConfigurationController_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL ViewController_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString ViewController_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL ViewController_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL ModuleController_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString ModuleController_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL ModuleController_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL ToolbarController_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString ToolbarController_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL ToolbarController_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL CommandController_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString CommandController_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL CommandController_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL BasicPaneFactory_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString BasicPaneFactory_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL BasicPaneFactory_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL BasicToolBarFactory_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString BasicToolBarFactory_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL BasicToolBarFactory_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL BasicViewFactory_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString BasicViewFactory_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL BasicViewFactory_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL TaskPaneService_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString TaskPaneService_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL TaskPaneService_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL ResourceId_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString ResourceId_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL ResourceId_getSupportedServiceNames (void)
    throw (uno::RuntimeException);
} }

using namespace ::sd;
using namespace ::sd::framework;




// ----------------------------------------------------------------------------
// Declaration and initialization of a map from service names to locally
// unique factory identifiers.

enum FactoryId
{
    SdDrawingDocumentFactory,
    SdPresentationDocumentFactory,
    SdHtmlOptionsDialogFactory,
    SdUnoModuleFactory,
    RandomNodeFactory,

    PaneControllerFactory,

    ConfigurationControllerFactory,
    ViewControllerFactory,
    ModuleControllerFactory,
    ToolbarControllerFactory,
    CommandControllerFactory,
    BasicPaneFactoryFactory,
    BasicToolBarFactoryFactory,
    BasicViewFactoryFactory,
    TaskPaneServiceFactory,
    ResourceIdFactory
};
typedef ::std::hash_map<OUString, FactoryId, comphelper::UStringHash, comphelper::UStringEqual> FactoryMap;


namespace {
static ::boost::shared_ptr<FactoryMap> spFactoryMap;
::boost::shared_ptr<FactoryMap> GetFactoryMap (void)
{
    if (spFactoryMap.get() == NULL)
    {
        spFactoryMap.reset(new FactoryMap);
        (*spFactoryMap)[SdDrawingDocument_getImplementationName()] = SdDrawingDocumentFactory;
        (*spFactoryMap)[SdPresentationDocument_getImplementationName()] = SdPresentationDocumentFactory;
        (*spFactoryMap)[SdHtmlOptionsDialog_getImplementationName()] = SdHtmlOptionsDialogFactory;
        (*spFactoryMap)[SdUnoModule_getImplementationName()] = SdUnoModuleFactory;
        (*spFactoryMap)[RandomNode__getImplementationName()] = RandomNodeFactory;
        (*spFactoryMap)[PaneController_getImplementationName()] = PaneControllerFactory;
        (*spFactoryMap)[ConfigurationController_getImplementationName()] = ConfigurationControllerFactory;
        (*spFactoryMap)[ViewController_getImplementationName()] = ViewControllerFactory;
        (*spFactoryMap)[ModuleController_getImplementationName()] = ModuleControllerFactory;
        (*spFactoryMap)[ToolbarController_getImplementationName()] = ToolbarControllerFactory;
        (*spFactoryMap)[CommandController_getImplementationName()] = CommandControllerFactory;
        (*spFactoryMap)[BasicPaneFactory_getImplementationName()] = BasicPaneFactoryFactory;
        (*spFactoryMap)[BasicToolBarFactory_getImplementationName()] = BasicToolBarFactoryFactory;
        (*spFactoryMap)[BasicViewFactory_getImplementationName()] = BasicViewFactoryFactory;
        (*spFactoryMap)[TaskPaneService_getImplementationName()] = TaskPaneServiceFactory;
        (*spFactoryMap)[ResourceId_getImplementationName()] = ResourceIdFactory;

        for (FactoryMap::const_iterator iEntry (spFactoryMap->begin());
             iEntry != spFactoryMap->end();
             ++iEntry)
        {
            OSL_TRACE("    %s -> %d",
                OUStringToOString(iEntry->first,RTL_TEXTENCODING_UTF8).getStr(),
                iEntry->second);
        }
    }
    return spFactoryMap;
};
} // end of anonymous namespace


#ifdef __cplusplus
extern "C"
{
#endif

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    const sal_Char  ** ppEnvTypeName,
    uno_Environment **  )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

static void SAL_CALL writeInfo(
    registry::XRegistryKey          * pRegistryKey,
    const OUString                  & rImplementationName,
    const uno::Sequence< OUString > & rServices )
{
    uno::Reference< registry::XRegistryKey > xNewKey(
        pRegistryKey->createKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + rImplementationName + OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) ) );

    for( sal_Int32 i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(
    void * ,
    void * pRegistryKey )
{
    if( pRegistryKey )
    {
        try
        {
            registry::XRegistryKey *pKey = reinterpret_cast< registry::XRegistryKey * >( pRegistryKey );

            writeInfo( pKey, SdHtmlOptionsDialog_getImplementationName(), SdHtmlOptionsDialog_getSupportedServiceNames() );
            writeInfo( pKey, SdDrawingDocument_getImplementationName(), SdDrawingDocument_getSupportedServiceNames() );
            writeInfo( pKey, SdPresentationDocument_getImplementationName(), SdPresentationDocument_getSupportedServiceNames() );
            writeInfo( pKey, SdUnoModule_getImplementationName(), SdUnoModule_getSupportedServiceNames() );
            writeInfo( pKey, sd::RandomNode__getImplementationName(), sd::RandomNode_getSupportedServiceNames() );
            writeInfo(
                pKey,
                sd::framework::PaneController_getImplementationName(),
                sd::framework::PaneController_getSupportedServiceNames());
            writeInfo(
                pKey,
                sd::framework::ConfigurationController_getImplementationName(),
                sd::framework::ConfigurationController_getSupportedServiceNames());
            writeInfo(
                pKey,
                sd::framework::ViewController_getImplementationName(),
                sd::framework::ViewController_getSupportedServiceNames());
            writeInfo(
                pKey,
                sd::framework::ToolbarController_getImplementationName(),
                sd::framework::ToolbarController_getSupportedServiceNames());
            writeInfo(
                pKey,
                sd::framework::CommandController_getImplementationName(),
                sd::framework::CommandController_getSupportedServiceNames());
            writeInfo(
                pKey,
                sd::framework::ModuleController_getImplementationName(),
                sd::framework::ModuleController_getSupportedServiceNames());
            writeInfo(
                pKey,
                sd::framework::BasicPaneFactory_getImplementationName(),
                sd::framework::BasicPaneFactory_getSupportedServiceNames());
            writeInfo(
                pKey,
                sd::framework::BasicToolBarFactory_getImplementationName(),
                sd::framework::BasicToolBarFactory_getSupportedServiceNames());
            writeInfo(
                pKey,
                sd::framework::BasicViewFactory_getImplementationName(),
                sd::framework::BasicViewFactory_getSupportedServiceNames());
            writeInfo(
                pKey,
                sd::framework::TaskPaneService_getImplementationName(),
                sd::framework::TaskPaneService_getSupportedServiceNames());
            writeInfo(
                pKey,
                sd::framework::ResourceId_getImplementationName(),
                sd::framework::ResourceId_getSupportedServiceNames());
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_True;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName,
    void           * pServiceManager,
    void           *  )
{
    void * pRet = 0;

    if( pServiceManager )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );

        uno::Reference<lang::XSingleServiceFactory> xFactory;
        uno::Reference<lang::XSingleComponentFactory> xComponentFactory;

        ::boost::shared_ptr<FactoryMap> pFactoryMap (GetFactoryMap());
        OUString sImplementationName (OUString::createFromAscii(pImplName));
        FactoryMap::const_iterator iFactory (pFactoryMap->find(sImplementationName));
        if (iFactory != pFactoryMap->end())
        {
            switch (iFactory->second)
            {
                case SdHtmlOptionsDialogFactory:
                    xFactory = ::cppu::createSingleFactory(
                        xMSF,
                        SdHtmlOptionsDialog_getImplementationName(),
                        SdHtmlOptionsDialog_CreateInstance,
                        SdHtmlOptionsDialog_getSupportedServiceNames());
                    break;

                case SdDrawingDocumentFactory:
                    xFactory = ::sfx2::createSfxModelFactory(
                        xMSF,
                        SdDrawingDocument_getImplementationName(),
                        SdDrawingDocument_createInstance,
                        SdDrawingDocument_getSupportedServiceNames());
                    break;

                case SdPresentationDocumentFactory:
                    xFactory = ::sfx2::createSfxModelFactory(
                        xMSF,
                        SdPresentationDocument_getImplementationName(),
                        SdPresentationDocument_createInstance,
                        SdPresentationDocument_getSupportedServiceNames());
                    break;

                case SdUnoModuleFactory:
                    xFactory = ::cppu::createSingleFactory(
                        xMSF,
                        SdUnoModule_getImplementationName(),
                        SdUnoModule_createInstance,
                        SdUnoModule_getSupportedServiceNames());
                    break;

                case RandomNodeFactory:
                    xFactory = ::cppu::createSingleFactory(
                        xMSF,
                        sd::RandomNode__getImplementationName(),
                        sd::RandomNode_createInstance,
                        sd::RandomNode_getSupportedServiceNames());
                    break;

                case PaneControllerFactory:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::PaneController_createInstance,
                        sd::framework::PaneController_getImplementationName(),
                        sd::framework::PaneController_getSupportedServiceNames());
                    break;

                case ConfigurationControllerFactory:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::ConfigurationController_createInstance,
                        sd::framework::ConfigurationController_getImplementationName(),
                        sd::framework::ConfigurationController_getSupportedServiceNames());
                    break;

                case ViewControllerFactory:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::ViewController_createInstance,
                        sd::framework::ViewController_getImplementationName(),
                        sd::framework::ViewController_getSupportedServiceNames());
                    break;

                case ModuleControllerFactory:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::ModuleController_createInstance,
                        sd::framework::ModuleController_getImplementationName(),
                        sd::framework::ModuleController_getSupportedServiceNames());
                    break;

                case ToolbarControllerFactory:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::ToolbarController_createInstance,
                        sd::framework::ToolbarController_getImplementationName(),
                        sd::framework::ToolbarController_getSupportedServiceNames());
                    break;

                case CommandControllerFactory:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::CommandController_createInstance,
                        sd::framework::CommandController_getImplementationName(),
                        sd::framework::CommandController_getSupportedServiceNames());
                    break;

                case BasicPaneFactoryFactory:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::BasicPaneFactory_createInstance,
                        sd::framework::BasicPaneFactory_getImplementationName(),
                        sd::framework::BasicPaneFactory_getSupportedServiceNames());
                    break;

                case BasicToolBarFactoryFactory:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::BasicToolBarFactory_createInstance,
                        sd::framework::BasicToolBarFactory_getImplementationName(),
                        sd::framework::BasicToolBarFactory_getSupportedServiceNames());
                    break;

                case BasicViewFactoryFactory:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::BasicViewFactory_createInstance,
                        sd::framework::BasicViewFactory_getImplementationName(),
                        sd::framework::BasicViewFactory_getSupportedServiceNames());
                    break;

                case TaskPaneServiceFactory:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::TaskPaneService_createInstance,
                        sd::framework::TaskPaneService_getImplementationName(),
                        sd::framework::TaskPaneService_getSupportedServiceNames());
                    break;

                case ResourceIdFactory:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::ResourceId_createInstance,
                        sd::framework::ResourceId_getImplementationName(),
                        sd::framework::ResourceId_getSupportedServiceNames());
                    break;

                default:
                    break;
            }
            if (xComponentFactory.is())
            {
                xComponentFactory->acquire();
                pRet =  xComponentFactory.get();
            }
            else if (xFactory.is())
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
    }
    return pRet;
}

} // end of extern "C"
