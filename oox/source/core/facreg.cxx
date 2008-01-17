/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: facreg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:50 $
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

#include "oox/dllapi.h"

#include <string.h>

#include <sal/config.h>

#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <cppuhelper/factory.hxx>
#include <uno/lbnames.h>

using namespace rtl;
using namespace com::sun::star;

#define SERVICE( className )                                        \
extern OUString SAL_CALL className##_getImplementationName() throw();   \
extern uno::Sequence< OUString > SAL_CALL className##_getSupportedServiceNames() throw();\
extern uno::Reference< uno::XInterface > SAL_CALL className##_createInstance(           \
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr )                \
    throw( uno::Exception )

namespace oox {
    namespace core { SERVICE( FilterDetect ); }
    namespace ppt { SERVICE( PowerPointImport ); }
    namespace xls { SERVICE( BiffDetector ); }
    namespace xls { SERVICE( ExcelFilter ); }
    namespace xls { SERVICE( ExcelBiffFilter ); }
    namespace shape { SERVICE( ShapeContextHandler ); }
    namespace shape { SERVICE( FastTokenHandlerService ); }
}

//
#ifdef __cplusplus
extern "C"
{
#endif

OOX_DLLPUBLIC void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

void SAL_CALL writeInfo( registry::XRegistryKey * pRegistryKey, const OUString& rImplementationName, const uno::Sequence< OUString >& rServices )
{
    uno::Reference< registry::XRegistryKey > xNewKey(
        pRegistryKey->createKey(
            OUString( sal_Unicode( '/' ) ) + rImplementationName + OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) ) );

    for( sal_Int32 i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);
}

#define WRITEINFO(className)\
    writeInfo( pKey, className##_getImplementationName(), className##_getSupportedServiceNames() )

OOX_DLLPUBLIC sal_Bool SAL_CALL component_writeInfo( void * , void * pRegistryKey )
{
    if( pRegistryKey )
    {
        try
        {
            registry::XRegistryKey *pKey = reinterpret_cast< registry::XRegistryKey * >( pRegistryKey );

            WRITEINFO( ::oox::core::FilterDetect );
            WRITEINFO( ::oox::ppt::PowerPointImport );
            WRITEINFO( ::oox::xls::BiffDetector );
            WRITEINFO( ::oox::xls::ExcelFilter );
            WRITEINFO( ::oox::xls::ExcelBiffFilter );
            WRITEINFO( ::oox::shape::ShapeContextHandler );
            WRITEINFO( ::oox::shape::FastTokenHandlerService );
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_True;
}

#define SINGLEFACTORY(classname)\
        if( classname##_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )\
        {\
            xFactory = ::cppu::createSingleFactory( xMSF,\
                classname##_getImplementationName(),\
                classname##_createInstance,\
                classname##_getSupportedServiceNames() );\
        }

OOX_DLLPUBLIC void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * )
{
    void * pRet = 0;
    if( pServiceManager )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );

        uno::Reference< lang::XSingleServiceFactory > xFactory;

        const sal_Int32 nImplNameLen = strlen( pImplName );

        // impress oasis import
        SINGLEFACTORY( ::oox::core::FilterDetect )
        else SINGLEFACTORY( oox::ppt::PowerPointImport )
        else SINGLEFACTORY( ::oox::xls::BiffDetector )
        else SINGLEFACTORY( ::oox::xls::ExcelFilter )
        else SINGLEFACTORY( ::oox::xls::ExcelBiffFilter )
        else SINGLEFACTORY( ::oox::shape::ShapeContextHandler)
        else SINGLEFACTORY( ::oox::shape::FastTokenHandlerService)

        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    return pRet;
}

#ifdef __cplusplus
}
#endif

