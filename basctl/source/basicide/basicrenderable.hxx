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


#ifndef _BASICRENDERABLE_HXX
#define _BASICRENDERABLE_HXX

#include "com/sun/star/view/XRenderable.hpp"
#include "cppuhelper/compbase1.hxx"

#include "vcl/print.hxx"

class IDEBaseWindow;

namespace basicide
{
class BasicRenderable :
        public cppu::WeakComponentImplHelper1< com::sun::star::view::XRenderable >,
        public vcl::PrinterOptionsHelper
{
    IDEBaseWindow*      mpWindow;
    osl::Mutex          maMutex;

    Printer* getPrinter();
public:
    BasicRenderable( IDEBaseWindow* pWin );
    virtual ~BasicRenderable();

    // XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount (
        const com::sun::star::uno::Any& aSelection,
        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue >& xOptions)
        throw (com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> SAL_CALL getRenderer (
        sal_Int32 nRenderer,
        const com::sun::star::uno::Any& rSelection,
        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rxOptions)
        throw (com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL render (
        sal_Int32 nRenderer,
        const com::sun::star::uno::Any& rSelection,
        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rxOptions)
        throw (com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException);

};

} // namespace

#endif
