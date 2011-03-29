/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef __FRAMEWORK_DISPATCH_DISPATCHINFORMATIONPROVIDER_HXX_
#define __FRAMEWORK_DISPATCH_DISPATCHINFORMATIONPROVIDER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <macros/xinterface.hxx>
#include <general.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weakref.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <vcl/svapp.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          a helper to merge dispatch informations of different sources together.
*//*-*************************************************************************************************************/
class DispatchInformationProvider : public  css::frame::XDispatchInformationProvider
                                  , private ThreadHelpBase
                                  , public  ::cppu::OWeakObject
{
    //_______________________
    // member
    private:

        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

    //_______________________
    // interface
    public:

        DispatchInformationProvider(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ,
                                    const css::uno::Reference< css::frame::XFrame >&              xFrame);

        virtual ~DispatchInformationProvider();

        FWK_DECLARE_XINTERFACE

        virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedCommandGroups()
            throw (css::uno::RuntimeException);

        virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation(sal_Int16 nCommandGroup)
            throw (css::uno::RuntimeException);

    //_______________________
    // helper
    private:

        css::uno::Sequence< css::uno::Reference< css::frame::XDispatchInformationProvider > > implts_getAllSubProvider();

}; // class DispatchInformationProvider

} // namespace framework

#endif // #ifndef __FRAMEWORK_DISPATCH_DISPATCHINFORMATIONPROVIDER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
