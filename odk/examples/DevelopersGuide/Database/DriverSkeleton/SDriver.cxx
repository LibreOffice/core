/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "SDriver.hxx"
#include "SConnection.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace connectivity::skeleton;

namespace connectivity
{
    namespace skeleton
    {
        //------------------------------------------------------------------
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL SkeletonDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
        {
            return *(new SkeletonDriver());
        }
    }
}
// --------------------------------------------------------------------------------
SkeletonDriver::SkeletonDriver()
    : ODriver_BASE(m_aMutex)
{
}
// --------------------------------------------------------------------------------
void SkeletonDriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // when driver will be destroied so all our connections have to be destroied as well
    for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_xConnections.clear();

    ODriver_BASE::disposing();
}

// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString SkeletonDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.comp.sdbc.SkeletonDriver");
        // this name is referenced in the configuration and in the skeleton.xml
        // Please take care when changing it.
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > SkeletonDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    // which service is supported
    // for more information @see com.sun.star.sdbc.Driver
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.Driver");
    return aSNS;
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL SkeletonDriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------
sal_Bool SAL_CALL SkeletonDriver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL SkeletonDriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL SkeletonDriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    // create a new connection with the given properties and append it to our vector
    OConnection* pCon = new OConnection(this);
    Reference< XConnection > xCon = pCon;   // important here because otherwise the connection could be deleted inside (refcount goes -> 0)
    pCon->construct(url,info);              // late constructor call which can throw exception and allows a correct dtor call when so
    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL SkeletonDriver::acceptsURL( const ::rtl::OUString& url )
        throw(SQLException, RuntimeException)
{
    // here we have to look if we support this url format
    // change the URL format to your needs, but please aware that the first on who accepts the URl wins.
    return (!url.compareTo(::rtl::OUString::createFromAscii("sdbc:skeleton:"),14));
}
// --------------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL SkeletonDriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    // if you have somthing special to say, return it here :-)
    return Sequence< DriverPropertyInfo >();
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL SkeletonDriver::getMajorVersion(  ) throw(RuntimeException)
{
    return 0; // depends on you
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL SkeletonDriver::getMinorVersion(  ) throw(RuntimeException)
{
    return 1; // depends on you
}
// --------------------------------------------------------------------------------

//.........................................................................
namespace connectivity
{
    namespace skeleton
    {
//.........................................................................

void release(oslInterlockedCount& _refCount,
             ::cppu::OBroadcastHelper& rBHelper,
             ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
             ::com::sun::star::lang::XComponent* _pObject)
{
    if (osl_decrementInterlockedCount( &_refCount ) == 0)
    {
        osl_incrementInterlockedCount( &_refCount );

        if (!rBHelper.bDisposed && !rBHelper.bInDispose)
        {
            // remember the parent
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xParent;
            {
                ::osl::MutexGuard aGuard( rBHelper.rMutex );
                xParent = _xInterface;
                _xInterface = NULL;
            }

            // First dispose
            _pObject->dispose();

            // only the alive ref holds the object
            OSL_ASSERT( _refCount == 1 );

            // release the parent in the ~
            if (xParent.is())
            {
                ::osl::MutexGuard aGuard( rBHelper.rMutex );
                _xInterface = xParent;
            }
        }
    }
    else
        osl_incrementInterlockedCount( &_refCount );
}

void checkDisposed(sal_Bool _bThrow) throw ( DisposedException )
{
    if (_bThrow)
        throw DisposedException();

}
//.........................................................................
    }
}
//.........................................................................

