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



#ifndef _DIGESTCONTEXT_HXX
#define _DIGESTCONTEXT_HXX

#include <com/sun/star/xml/crypto/XDigestContext.hpp>

#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>

class ODigestContext : public cppu::WeakImplHelper1< ::com::sun::star::xml::crypto::XDigestContext >
{
private:
    ::osl::Mutex m_aMutex;

    PK11Context* m_pContext;
    sal_Int32 m_nDigestLength;
    bool m_b1KData;
    sal_Int32 m_nDigested;

    bool m_bDisposed;
    bool m_bBroken;

public:
    ODigestContext( PK11Context* pContext, sal_Int32 nDigestLength, bool b1KData )
    : m_pContext( pContext )
    , m_nDigestLength( nDigestLength )
    , m_b1KData( b1KData )
    , m_nDigested( 0 )
    , m_bDisposed( false )
    , m_bBroken( false )
    {}

    virtual ~ODigestContext();


    // XDigestContext
    virtual void SAL_CALL updateDigest( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL finalizeDigestAndDispose() throw (::com::sun::star::lang::DisposedException, ::com::sun::star::uno::RuntimeException);
};

#endif

