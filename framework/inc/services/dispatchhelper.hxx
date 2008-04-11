/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dispatchhelper.hxx,v $
 * $Revision: 1.5 $
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

#ifndef __FRAMEWORK_SERVICES_DISPATCHHELPER_HXX_
#define __FRAMEWORK_SERVICES_DISPATCHHELPER_HXX_

//_______________________________________________
// my own includes

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>

//_______________________________________________
// interface includes
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/DispatchResultEvent.hpp>

//_______________________________________________
// other includes
#include <cppuhelper/weak.hxx>
#include <osl/conditn.hxx>

//_______________________________________________
// namespace

namespace framework{

//_______________________________________________
// exported const

//_______________________________________________
// exported definitions

/**
    @short      implements an easy way for dispatches
    @descr      Dispatches are splitted into different parts:
                    - parsing of the URL
                    - searching for a dispatcgh object
                    - dispatching of the URL
                All these steps are done inside one method call here.
*/

class DispatchHelper : public css::lang::XTypeProvider
                     , public css::lang::XServiceInfo
                     , public css::frame::XDispatchHelper
                     , public css::frame::XDispatchResultListener // => XEventListener
                     , public ThreadHelpBase                      // must be the first base class!
                     , public ::cppu::OWeakObject                 // => XInterface, XWeak
{

    //-------------------------------------------
    // member

    private:

        /** global uno service manager.
            Can be used to create own needed services. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** used to wait for asynchronous listener callbacks. */
        ::osl::Condition m_aBlock;

        css::uno::Any m_aResult;

        css::uno::Reference< css::uno::XInterface > m_xBroadcaster;

    //-------------------------------------------
    // interface

    public:

        //---------------------------------------
        // ctor/dtor

                 DispatchHelper( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR );
        virtual ~DispatchHelper(                                                                     );

        //---------------------------------------
        // XInterface, XTypeProvider, XServiceInfo

        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------
        // XDispatchHelper
        virtual css::uno::Any SAL_CALL executeDispatch(
                                        const css::uno::Reference< css::frame::XDispatchProvider >& xDispatchProvider ,
                                        const ::rtl::OUString&                                      sURL              ,
                                        const ::rtl::OUString&                                      sTargetFrameName  ,
                                              sal_Int32                                             nSearchFlags      ,
                                        const css::uno::Sequence< css::beans::PropertyValue >&      lArguments        )
        throw(css::uno::RuntimeException);

        //---------------------------------------
        // XDispatchResultListener
        virtual void SAL_CALL dispatchFinished(
                                const css::frame::DispatchResultEvent& aResult )
        throw(css::uno::RuntimeException);

        //---------------------------------------
        // XEventListener
        virtual void SAL_CALL disposing(
                                const css::lang::EventObject& aEvent )
        throw(css::uno::RuntimeException);
};

}

#endif
