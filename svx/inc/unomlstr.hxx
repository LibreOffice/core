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


#ifndef _SVX_UNOMLSTR_HXX
#define _SVX_UNOMLSTR_HXX

#include <com/sun/star/util/XModifyListener.hpp>

#include <cppuhelper/implbase1.hxx>

class SdrObject;

class SvxUnoShapeModifyListener : public ::cppu::WeakAggImplHelper1< ::com::sun::star::util::XModifyListener >
{
    SdrObject*  mpObj;

public:
    SvxUnoShapeModifyListener( SdrObject* pObj ) throw();
    virtual ~SvxUnoShapeModifyListener() throw();

    // ::com::sun::star::util::XModifyListener
    virtual void SAL_CALL modified(const ::com::sun::star::lang::EventObject& aEvent) throw(  ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(  ::com::sun::star::uno::RuntimeException);

    // internal
    void invalidate() throw();
};


#endif


