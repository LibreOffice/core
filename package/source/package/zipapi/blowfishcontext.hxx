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


#ifndef _BLOWFISHCONTEXT_HXX
#define _BLOWFISHCONTEXT_HXX

#include <com/sun/star/xml/crypto/XCipherContext.hpp>

#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>

class BlowfishCFB8CipherContext : public cppu::WeakImplHelper1< ::com::sun::star::xml::crypto::XCipherContext >
{
    ::osl::Mutex m_aMutex;
    void* m_pCipher;
    bool m_bEncrypt;

    BlowfishCFB8CipherContext()
    : m_pCipher( NULL )
    , m_bEncrypt( false )
    {}

public:

    virtual ~BlowfishCFB8CipherContext();

    static ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XCipherContext >
        Create( const ::com::sun::star::uno::Sequence< sal_Int8 >& aDerivedKey, const ::com::sun::star::uno::Sequence< sal_Int8 >& aInitVector, bool bEncrypt );

    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL convertWithCipherContext( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::DisposedException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL finalizeCipherContextAndDispose(  ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::uno::RuntimeException);
};

#endif // _BLOWFISHCONTEXT_HXX

