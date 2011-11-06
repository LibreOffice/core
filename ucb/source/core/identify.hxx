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



#ifndef _IDENTIFY_HXX
#define _IDENTIFY_HXX

#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>

//=========================================================================

class ContentIdentifier :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                  public com::sun::star::ucb::XContentIdentifier
{
public:
    ContentIdentifier( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                       const rtl::OUString& ContentId );
    virtual ~ContentIdentifier();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XContentIdentifier
    virtual rtl::OUString SAL_CALL getContentIdentifier()
        throw( com::sun::star::uno::RuntimeException );
    virtual rtl::OUString SAL_CALL getContentProviderScheme()
        throw( com::sun::star::uno::RuntimeException );

private:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    rtl::OUString m_aContentId;
    rtl::OUString m_aProviderScheme;
};

#endif /* !_IDENTIFY_HXX */
