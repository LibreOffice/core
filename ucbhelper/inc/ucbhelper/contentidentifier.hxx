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



#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#define _UCBHELPER_CONTENTIDENTIFIER_HXX

#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include "ucbhelper/ucbhelperdllapi.h"

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
} } } }

namespace rtl {
    class OUString;
}

namespace ucbhelper
{

struct ContentIdentifier_Impl;

//=========================================================================

/**
  * This class implements a simple identifier object for UCB contents.
  * It mainly stores and returns the URL as it was passed to the constructor -
  * The only difference is that the URL scheme will be lower cased. This can
  * be done, because URL schemes are never case sensitive.
  */
class UCBHELPER_DLLPUBLIC ContentIdentifier :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                  public com::sun::star::ucb::XContentIdentifier
{
public:
    ContentIdentifier( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                       const rtl::OUString& rURL );
    ContentIdentifier( const rtl::OUString& rURL );
    virtual ~ContentIdentifier();

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type & rType )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    acquire() throw();
    virtual void SAL_CALL
    release() throw();

    // XTypeProvider
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw( com::sun::star::uno::RuntimeException );

    // XContentIdentifier
    virtual rtl::OUString SAL_CALL
    getContentIdentifier()
        throw( com::sun::star::uno::RuntimeException );
    virtual rtl::OUString SAL_CALL
    getContentProviderScheme()
        throw( com::sun::star::uno::RuntimeException );

private:
    ContentIdentifier_Impl* m_pImpl;
};

} /* namespace ucbhelper */

#endif /* !_UCBHELPER_CONTENTIDENTIFIER_HXX */
