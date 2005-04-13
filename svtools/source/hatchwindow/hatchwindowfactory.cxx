/*************************************************************************
 *
 *  $RCSfile: hatchwindowfactory.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:02:50 $
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

#include "hatchwindowfactory.hxx"
#include "hatchwindow.hxx"

#ifndef _CPPUHELPER_FACTORY_HXX_
#include "cppuhelper/factory.hxx"
#endif

using namespace ::com::sun::star;

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OHatchWindowFactory::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString::createFromAscii("com.sun.star.embed.HatchWindowFactory");
    aRet[1] = ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.HatchWindowFactory");
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OHatchWindowFactory::impl_staticGetImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.HatchWindowFactory");
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OHatchWindowFactory::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new OHatchWindowFactory( xServiceManager ) );
}


//-------------------------------------------------------------------------
uno::Reference< embed::XHatchWindow > SAL_CALL OHatchWindowFactory::createHatchWindowInstance(
                const uno::Reference< awt::XWindowPeer >& xParent,
                const awt::Rectangle& aBounds,
                const awt::Size& aHandlerSize )
    throw (uno::RuntimeException)
{
    if ( !xParent.is() )
        throw lang::IllegalArgumentException(); // TODO

    VCLXHatchWindow* pResult = new VCLXHatchWindow();
    pResult->initializeWindow( xParent, aBounds, aHandlerSize );
    return uno::Reference< embed::XHatchWindow >( static_cast< embed::XHatchWindow* >( pResult ) );
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OHatchWindowFactory::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL OHatchWindowFactory::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OHatchWindowFactory::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

//-------------------------------------------------------------------------

extern "C"
{

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo (
    void * /* pServiceManager */, void * pRegistryKey)
{
    if (pRegistryKey)
    {
        uno::Reference< registry::XRegistryKey> xRegistryKey (
            reinterpret_cast< registry::XRegistryKey* >(pRegistryKey));
        uno::Reference< registry::XRegistryKey> xNewKey;

        xNewKey = xRegistryKey->createKey (
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
            OHatchWindowFactory::impl_staticGetImplementationName() +
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) );

        const uno::Sequence< ::rtl::OUString > aServices (
            OHatchWindowFactory::impl_staticGetSupportedServiceNames());
        for (sal_Int32 i = 0, n = aServices.getLength(); i < n; i++ )
            xNewKey->createKey( aServices.getConstArray()[i] );

        return sal_True;
    }
    return sal_False;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory (
    const sal_Char * pImplementationName, void * pServiceManager, void * /* pRegistryKey */)
{
    void * pResult = 0;
    if (pServiceManager)
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory;
        if (OHatchWindowFactory::impl_staticGetImplementationName().compareToAscii (pImplementationName ) == 0)
        {
            xFactory = cppu::createOneInstanceFactory(
                reinterpret_cast< lang::XMultiServiceFactory* >(pServiceManager),
                OHatchWindowFactory::impl_staticGetImplementationName(),
                OHatchWindowFactory::impl_staticCreateSelfInstance,
                OHatchWindowFactory::impl_staticGetSupportedServiceNames());
        }
        if (xFactory.is())
        {
            xFactory->acquire();
            pResult = xFactory.get();
        }
    }
    return pResult;
}

} // extern "C"
