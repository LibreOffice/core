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



#ifndef _ENUMERATION_HXX
#define _ENUMERATION_HXX


#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/uno/Reference.hxx>

namespace com { namespace sun { namespace star {
    namespace container { class XIndexAccess; }
    namespace uno { class Any; }
    namespace container { class NoSuchElementException; }
    namespace lang { class WrappedTargetException; }
    namespace uno { class RuntimeException; }
} } }

/** implement XEnumeration based on container::XIndexAccess */
class Enumeration
    : public cppu::WeakImplHelper1<com::sun::star::container::XEnumeration>
{
    com::sun::star::uno::Reference<com::sun::star::container::XIndexAccess> mxContainer;
    sal_Int32 mnIndex;

public:
    Enumeration( com::sun::star::container::XIndexAccess* );

    virtual sal_Bool SAL_CALL hasMoreElements()
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Any SAL_CALL nextElement()
        throw( com::sun::star::container::NoSuchElementException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );
};

#endif
