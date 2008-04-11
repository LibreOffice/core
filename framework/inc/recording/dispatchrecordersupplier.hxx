/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dispatchrecordersupplier.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_RECORDING_DISPATCHRECORDERSUPPLIER_HXX_
#define __FRAMEWORK_RECORDING_DISPATCHRECORDERSUPPLIER_HXX_

//_________________________________________________________________________________________________________________
// include own things

#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <macros/debug.hxx>
#include <macros/generic.hxx>
#include <general.h>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
// include interfaces
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.HPP>
#endif
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

//_________________________________________________________________________________________________________________
// include other projects
#include <cppuhelper/weak.hxx>

//_________________________________________________________________________________________________________________
//  namespace

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const

//_________________________________________________________________________________________________________________
//  exported definitions

//_______________________________________________

/** @short          implement a supplier for dispatch recorder
    @descr          This supplier can be set on property "DispatchRecorderSupplier" on a frame.
                    By using of this supplier and his internal XDispatchRecorder it's possible to
                    record XDispatch::dispatch() requests.

    @threadsafe     yes
 */
class DispatchRecorderSupplier  :   // interfaces
                                    public  css::lang::XTypeProvider                ,
                                    public  css::lang::XServiceInfo                 ,
                                    public  css::frame::XDispatchRecorderSupplier   ,
                                    // baseclasses
                                    // Order is neccessary for right initialization!
                                    private ThreadHelpBase                          ,
                                    public  ::cppu::OWeakObject
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

        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
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
