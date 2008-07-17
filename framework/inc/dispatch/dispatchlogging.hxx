/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2008 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: dispatchlogging.hxx,v $
*
* $Revision: 1.2 $
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
************************************************************************/

#ifndef __FRAMEWORK_HELPER_DISPATCHLOGGING_HXX_
#define __FRAMEWORK_HELPER_DISPATCHLOGGING_HXX_

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakref.hxx>

namespace framework {
        class DispatchLogging :
            public ::com::sun::star::frame::XDispatchProvider,
            public ::com::sun::star::frame::XDispatchProviderInterception,
            public ::com::sun::star::lang::XEventListener,
            public ::cppu::OWeakObject
        {
            public:
                static ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > createInstance(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>& slave);
            protected:
                DispatchLogging() {};
        };
}

#endif // #ifndef __FRAMEWORK_HELPER_DISPATCHLOGGING_HXX_
