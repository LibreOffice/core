/*************************************************************************
 *
 *  $RCSfile: excomp1.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 15:28:04 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#include <example/XTest.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace example;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace cppu;
using namespace osl;
using namespace rtl;

#define SERVICENAME1 "example.ExampleComponent1"
#define IMPLNAME1   "example.ExampleComponent1.Impl"

namespace excomp_impl {

//*************************************************************************
// ExampleComponent1Impl
//*************************************************************************
class ExampleComponent1Impl : public WeakImplHelper2< XTest, XServiceInfo >
{
public:
    ExampleComponent1Impl( const Reference<XMultiServiceFactory> & rXSMgr );

    ~ExampleComponent1Impl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

    // XSimpleRegistry
    virtual OUString SAL_CALL getMessage() throw(RuntimeException);

protected:
    Mutex       m_mutex;

    Reference<XMultiServiceFactory> m_xSMgr;
};

//*************************************************************************
ExampleComponent1Impl::ExampleComponent1Impl( const Reference<XMultiServiceFactory> & rXSMgr )
    : m_xSMgr(rXSMgr)
{
}

//*************************************************************************
ExampleComponent1Impl::~ExampleComponent1Impl()
{
}

//*************************************************************************
OUString SAL_CALL ExampleComponent1Impl::getImplementationName(  )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME1) );
}

//*************************************************************************
sal_Bool SAL_CALL ExampleComponent1Impl::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

//*************************************************************************
Sequence<OUString> SAL_CALL ExampleComponent1Impl::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return getSupportedServiceNames_Static();
}

//*************************************************************************
Sequence<OUString> SAL_CALL ExampleComponent1Impl::getSupportedServiceNames_Static(  )
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME1) );
    return Sequence< OUString >( &aName, 1 );
}

//*************************************************************************
OUString SAL_CALL ExampleComponent1Impl::getMessage() throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return OUString::createFromAscii("Lalelu nur der Mann im Mond schaut zu ...");
}


//*************************************************************************
Reference<XInterface> SAL_CALL ExampleComponent1_CreateInstance( const Reference<XMultiServiceFactory>& rSMgr )
{
    Reference<XInterface> xRet;

    XTest *pXTest = (XTest*) new ExampleComponent1Impl(rSMgr);

    if (pXTest)
    {
        xRet = Reference< XInterface >::query(pXTest);
    }

    return xRet;
}

} // excomp_impl


extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            // ExampleComponent1
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME1 "/UNO/SERVICES") ) ) );

            Sequence< OUString > & rSNL =
                ::excomp_impl::ExampleComponent1Impl::getSupportedServiceNames_Static();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (rtl_str_compare( pImplName, IMPLNAME1 ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME1) ),
            ::excomp_impl::ExampleComponent1_CreateInstance,
            ::excomp_impl::ExampleComponent1Impl::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}



