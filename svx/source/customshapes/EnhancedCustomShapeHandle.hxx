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



#ifndef _ENHANCED_CUSTOMSHAPE_HANDLE_HXX
#define _ENHANCED_CUSTOMSHAPE_HANDLE_HXX

#include <com/sun/star/uno/RuntimeException.hpp>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XCustomShapeHandle.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <cppuhelper/weakref.hxx>

class EnhancedCustomShapeHandle : public cppu::WeakImplHelper2
<
    com::sun::star::drawing::XCustomShapeHandle,
    com::sun::star::lang::XInitialization
>
{
    sal_uInt32                                                          mnIndex;
    com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxCustomShape;

public:

            EnhancedCustomShapeHandle( com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xCustomShape, sal_uInt32 nIndex );
    virtual ~EnhancedCustomShapeHandle();

    // XInterface
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XCustomShapeHandle
    virtual com::sun::star::awt::Point SAL_CALL getPosition()
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setControllerPosition( const com::sun::star::awt::Point& )
        throw ( com::sun::star::uno::RuntimeException );

    // XInitialization
    virtual void SAL_CALL initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
        throw ( com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException );
};

#endif
