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



#ifndef _COMENUMWRAPPER_HXX
#define _COMENUMWRAPPER_HXX

#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/script/XInvocation.hpp>

#include <cppuhelper/implbase1.hxx>

class ComEnumerationWrapper : public ::cppu::WeakImplHelper1< ::com::sun::star::container::XEnumeration >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XInvocation > m_xInvocation;
    sal_Int32 m_nCurInd;

public:
    ComEnumerationWrapper( const ::com::sun::star::uno::Reference< ::com::sun::star::script::XInvocation >& xInvocation )
    : m_xInvocation( xInvocation )
    , m_nCurInd( 0 )
    {
    }

    // container::XEnumeration
    virtual ::sal_Bool SAL_CALL hasMoreElements() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement() throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};

#endif // _COMENUMWRAPPER_HXX

