/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __FRAMEWORK_RECORDING_DISPATCHRECORDERSUPPLIER_HXX_
#define __FRAMEWORK_RECORDING_DISPATCHRECORDERSUPPLIER_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <macros/debug.hxx>
#include <macros/generic.hxx>
#include <general.h>
#include <stdtypes.h>

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <cppuhelper/implbase2.hxx>

namespace framework{

/** @short          implement a supplier for dispatch recorder
    @descr          This supplier can be set on property "DispatchRecorderSupplier" on a frame.
                    By using of this supplier and his internal XDispatchRecorder it's possible to
                    record XDispatch::dispatch() requests.

    @threadsafe     yes
 */
class DispatchRecorderSupplier  :   // baseclasses
                                    // Order is necessary for right initialization!
                                    private ThreadHelpBase                          ,
                                    // interfaces
                                    public  ::cppu::WeakImplHelper2<
                                                css::lang::XServiceInfo             ,
                                                css::frame::XDispatchRecorderSupplier >
{
    //___________________________________________
    // member

    private:

        //_______________________________________
        /** provided dispatch recorder of this supplier instance

            @life   Is controled from outside. Because this variable is setted
                    from there and not created internaly. But we release our
                    reference to it if we die.
         */
        css::uno::Reference< css::frame::XDispatchRecorder > m_xDispatchRecorder;

        //_______________________________________
        /** reference to the global uno service manager
         */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;

    //___________________________________________
    // uno interface

    public:

        //_______________________________________
        // XInterface, XTypeProvider, XServiceInfo

        DECLARE_XSERVICEINFO

        //_______________________________________
        // XDispatchRecorderSupplier

        virtual void                                                 SAL_CALL setDispatchRecorder( const css::uno::Reference< css::frame::XDispatchRecorder >& xRecorder   ) throw (css::uno::RuntimeException);
        virtual css::uno::Reference< css::frame::XDispatchRecorder > SAL_CALL getDispatchRecorder(                                                                         ) throw (css::uno::RuntimeException);
        virtual void                                                 SAL_CALL dispatchAndRecord  ( const css::util::URL&                                       aURL        ,
                                                                                                   const css::uno::Sequence< css::beans::PropertyValue >&      lArguments  ,
                                                                                                   const css::uno::Reference< css::frame::XDispatch >&         xDispatcher ) throw (css::uno::RuntimeException);

    //___________________________________________
    // native interface

    public:

         DispatchRecorderSupplier( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
        ~DispatchRecorderSupplier();

};      //  class DispatchRecorderSupplier

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_RECORDING_DISPATCHRECORDERSUPPLIER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
