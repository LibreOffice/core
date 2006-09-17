/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLFilterRegistration.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:29:48 $
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
#include "precompiled_xmloff.hxx"

#include <string.h>

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

using namespace ::com::sun::star;

#define DECLARE_SERVICE( classname )    \
    extern ::rtl::OUString SAL_CALL classname##_getImplementationName() throw();   \
    extern uno::Sequence< ::rtl::OUString > SAL_CALL classname##_getSupportedServiceNames() throw();   \
    extern uno::Reference< uno::XInterface > SAL_CALL classname##_createInstance(   \
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );

#define ENUMERATE_SERVICE( classname ) \
    { classname##_getImplementationName, classname##_getSupportedServiceNames, classname##_createInstance }

// ============================================================================
DECLARE_SERVICE( OOo2OasisTransformer )
DECLARE_SERVICE( Oasis2OOoTransformer )

DECLARE_SERVICE( XMLAutoTextEventImportOOO )
DECLARE_SERVICE( XMLMetaImportOOO )
DECLARE_SERVICE( XMLMathSettingsImportOOO )
DECLARE_SERVICE( XMLMathMetaImportOOO )
DECLARE_SERVICE( XMLCalcSettingsImportOOO )
DECLARE_SERVICE( XMLCalcMetaImportOOO )
DECLARE_SERVICE( XMLCalcContentImportOOO )
DECLARE_SERVICE( XMLCalcStylesImportOOO )
DECLARE_SERVICE( XMLCalcImportOOO )
DECLARE_SERVICE( XMLWriterSettingsImportOOO )
DECLARE_SERVICE( XMLWriterMetaImportOOO )
DECLARE_SERVICE( XMLWriterContentImportOOO )
DECLARE_SERVICE( XMLWriterStylesImportOOO )
DECLARE_SERVICE( XMLWriterImportOOO )
DECLARE_SERVICE( XMLChartContentImportOOO )
DECLARE_SERVICE( XMLChartStylesImportOOO )
DECLARE_SERVICE( XMLChartImportOOO )
DECLARE_SERVICE( XMLDrawSettingsImportOOO )
DECLARE_SERVICE( XMLDrawMetaImportOOO )
DECLARE_SERVICE( XMLDrawContentImportOOO )
DECLARE_SERVICE( XMLDrawStylesImportOOO )
DECLARE_SERVICE( XMLDrawImportOOO )
DECLARE_SERVICE( XMLImpressSettingsImportOOO )
DECLARE_SERVICE( XMLImpressMetaImportOOO )
DECLARE_SERVICE( XMLImpressContentImportOOO )
DECLARE_SERVICE( XMLImpressStylesImportOOO )
DECLARE_SERVICE( XMLImpressImportOOO )

// ============================================================================

// ----------------------------------------------------------------------------
namespace
{
    typedef ::rtl::OUString (SAL_CALL * GetImplementationName)();
    typedef uno::Sequence< ::rtl::OUString > (SAL_CALL * GetSupportedServiceNames)();
    typedef uno::Reference< ::uno::XInterface > (SAL_CALL * CreateInstance)(
        const uno::Reference< lang::XMultiServiceFactory >& );

    struct ServiceDescriptor
    {
        GetImplementationName       getImplementationName;
        GetSupportedServiceNames    getSupportedServiceNames;
        CreateInstance              createInstance;
    };

    // ------------------------------------------------------------------------
    static const ServiceDescriptor* getServiceDescriptors()
    {
        static const ServiceDescriptor aDescriptors[] =
        {
            // ================================================================
            ENUMERATE_SERVICE( OOo2OasisTransformer ),
            ENUMERATE_SERVICE( Oasis2OOoTransformer ),

            ENUMERATE_SERVICE( XMLAutoTextEventImportOOO ),
            ENUMERATE_SERVICE( XMLMetaImportOOO ),
            ENUMERATE_SERVICE( XMLMathSettingsImportOOO ),
            ENUMERATE_SERVICE( XMLMathMetaImportOOO ),
            ENUMERATE_SERVICE( XMLCalcSettingsImportOOO ),
            ENUMERATE_SERVICE( XMLCalcMetaImportOOO ),
            ENUMERATE_SERVICE( XMLCalcContentImportOOO ),
            ENUMERATE_SERVICE( XMLCalcStylesImportOOO ),
            ENUMERATE_SERVICE( XMLCalcImportOOO ),
            ENUMERATE_SERVICE( XMLWriterSettingsImportOOO ),
            ENUMERATE_SERVICE( XMLWriterMetaImportOOO ),
            ENUMERATE_SERVICE( XMLWriterContentImportOOO ),
            ENUMERATE_SERVICE( XMLWriterStylesImportOOO ),
            ENUMERATE_SERVICE( XMLWriterImportOOO ),
            ENUMERATE_SERVICE( XMLChartContentImportOOO ),
            ENUMERATE_SERVICE( XMLChartStylesImportOOO ),
            ENUMERATE_SERVICE( XMLChartImportOOO ),
            ENUMERATE_SERVICE( XMLDrawSettingsImportOOO ),
            ENUMERATE_SERVICE( XMLDrawMetaImportOOO ),
            ENUMERATE_SERVICE( XMLDrawContentImportOOO ),
            ENUMERATE_SERVICE( XMLDrawStylesImportOOO ),
            ENUMERATE_SERVICE( XMLDrawImportOOO ),
            ENUMERATE_SERVICE( XMLImpressSettingsImportOOO ),
            ENUMERATE_SERVICE( XMLImpressMetaImportOOO ),
            ENUMERATE_SERVICE( XMLImpressContentImportOOO ),
            ENUMERATE_SERVICE( XMLImpressStylesImportOOO ),
            ENUMERATE_SERVICE( XMLImpressImportOOO ),
            // ================================================================
            { NULL, NULL, NULL }
        };
        return aDescriptors;
    };
}
// ----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif

void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo( void * /*pServiceManager*/, void * pRegistryKey )
{
    if( pRegistryKey )
    {
        try
        {
            uno::Reference< registry::XRegistryKey > xMasterKey( reinterpret_cast< registry::XRegistryKey * >( pRegistryKey ) );

            const ServiceDescriptor* pDescriptor = getServiceDescriptors();
            while ( pDescriptor->getImplementationName )
            {
                ::rtl::OUString sNewKeyName( RTL_CONSTASCII_USTRINGPARAM("/") );
                sNewKeyName += pDescriptor->getImplementationName();
                sNewKeyName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") );

                uno::Reference< registry::XRegistryKey > xNewKey( xMasterKey->createKey( sNewKeyName ) );

                uno::Sequence< ::rtl::OUString > aServices = pDescriptor->getSupportedServiceNames();
                const ::rtl::OUString* pServices = aServices.getConstArray();
                for( sal_Int32 i = 0; i < aServices.getLength(); ++i, ++pServices )
                    xNewKey->createKey( *pServices);

                ++pDescriptor;
            }
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "xof::component_writeInfo: InvalidRegistryException!" );
        }
    }
    return sal_True;
}

void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = NULL;
    if( pServiceManager )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xMSF( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );

            const sal_Int32 nImplNameLen = strlen( pImplName );

            const ServiceDescriptor* pDescriptor = getServiceDescriptors();
            while ( pDescriptor->getImplementationName )
            {
                if ( pDescriptor->getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
                {
                    uno::Reference< lang::XSingleServiceFactory > xFactory =
                        ::cppu::createSingleFactory( xMSF,
                            pDescriptor->getImplementationName(),
                            pDescriptor->createInstance,
                            pDescriptor->getSupportedServiceNames()
                        );

                    if ( xFactory.is() )
                    {
                        xFactory->acquire();
                        pRet = xFactory.get();
                        break;
                    }
                }

                ++pDescriptor;
            }
        }
        catch( uno::Exception& )
        {
            OSL_ENSURE( sal_False, "xof::component_getFactory: Exception!" );
        }
    }

    return pRet;
}

#ifdef __cplusplus
}
#endif

