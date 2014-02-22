/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include <cppuhelper/supportsservice.hxx>
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
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL SkeletonDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
        {
            return *(new SkeletonDriver());
        }
    }
}

SkeletonDriver::SkeletonDriver()
    : ODriver_BASE(m_aMutex)
{
}

void SkeletonDriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    
    for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_xConnections.clear();

    ODriver_BASE::disposing();
}



rtl::OUString SkeletonDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString("com.sun.star.comp.sdbc.SkeletonDriver");
        
        
}

Sequence< ::rtl::OUString > SkeletonDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    
    
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = ::rtl::OUString("com.sun.star.sdbc.Driver");
    return aSNS;
}

::rtl::OUString SAL_CALL SkeletonDriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL SkeletonDriver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< ::rtl::OUString > SAL_CALL SkeletonDriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

Reference< XConnection > SAL_CALL SkeletonDriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    
    OConnection* pCon = new OConnection(this);
    Reference< XConnection > xCon = pCon;   
    pCon->construct(url,info);              
    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}

sal_Bool SAL_CALL SkeletonDriver::acceptsURL( const ::rtl::OUString& url )
        throw(SQLException, RuntimeException)
{
    
    
    return url.startsWith("sdbc:skeleton:");
}

Sequence< DriverPropertyInfo > SAL_CALL SkeletonDriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    
    return Sequence< DriverPropertyInfo >();
}

sal_Int32 SAL_CALL SkeletonDriver::getMajorVersion(  ) throw(RuntimeException)
{
    return 0; 
}

sal_Int32 SAL_CALL SkeletonDriver::getMinorVersion(  ) throw(RuntimeException)
{
    return 1; 
}



namespace connectivity
{
    namespace skeleton
    {


void release(oslInterlockedCount& _refCount,
             ::cppu::OBroadcastHelper& rBHelper,
             ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
             ::com::sun::star::lang::XComponent* _pObject)
{
    if (osl_atomic_decrement( &_refCount ) == 0)
    {
        osl_atomic_increment( &_refCount );

        if (!rBHelper.bDisposed && !rBHelper.bInDispose)
        {
            
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xParent;
            {
                ::osl::MutexGuard aGuard( rBHelper.rMutex );
                xParent = _xInterface;
                _xInterface = NULL;
            }

            
            _pObject->dispose();

            
            OSL_ASSERT( _refCount == 1 );

            
            if (xParent.is())
            {
                ::osl::MutexGuard aGuard( rBHelper.rMutex );
                _xInterface = xParent;
            }
        }
    }
    else
        osl_atomic_increment( &_refCount );
}

void checkDisposed(sal_Bool _bThrow) throw ( DisposedException )
{
    if (_bThrow)
        throw DisposedException();

}

    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
