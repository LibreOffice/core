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


#ifndef _COMPHELPER_WEAK_HXX_
#define _COMPHELPER_WEAK_HXX_

#include "comphelper/comphelperdllapi.h"

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cppuhelper/weak.hxx>

namespace comphelper
{
/** Base class to implement an UNO object supporting types and weak references, i.e. the object can be held
    weakly (by a ::com::sun::star::uno::WeakReference).
    This implementation copes with reference counting.  Upon last release(), the virtual dtor
    is called.

    In addition to the features from cppu::OWeakObject, derivations from this class can
    also used as a base class for ::cppu::ImplInheritanceHelper?
*/
class COMPHELPER_DLLPUBLIC OWeakTypeObject : public ::cppu::OWeakObject, public ::com::sun::star::lang::XTypeProvider
{
public:
    OWeakTypeObject();
    virtual ~OWeakTypeObject();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(const ::com::sun::star::uno::Type & rType )  throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);

};

}

#endif


