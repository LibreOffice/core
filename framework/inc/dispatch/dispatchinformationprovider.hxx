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

#ifndef INCLUDED_FRAMEWORK_INC_DISPATCH_DISPATCHINFORMATIONPROVIDER_HXX
#define INCLUDED_FRAMEWORK_INC_DISPATCH_DISPATCHINFORMATIONPROVIDER_HXX

#include <general.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider2.hpp>

#include <cppuhelper/weakref.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>
#include <vcl/svapp.hxx>

namespace framework{

/*-************************************************************************************************************
    @short          a helper to merge dispatch information of different sources together.
*//*-*************************************************************************************************************/
class DispatchInformationProvider : public  ::cppu::WeakImplHelper< css::frame::XDispatchInformationProvider2 >
{

    // member
    private:

        css::uno::Reference< css::uno::XComponentContext > m_xContext;
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

    // interface
    public:

        DispatchInformationProvider(const css::uno::Reference< css::uno::XComponentContext >& xContext ,
                                    const css::uno::Reference< css::frame::XFrame >&          xFrame);

        virtual ~DispatchInformationProvider() override;

        virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedCommandGroups() override;

        virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation(sal_Int16 nCommandGroup) override;
        virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation2(sal_Int16 nCommandGroup, sal_uInt32 nSlotMode) override;

    // helper
    private:

        css::uno::Sequence< css::uno::Reference< css::frame::XDispatchInformationProvider > > implts_getAllSubProvider();

}; // class DispatchInformationProvider

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_DISPATCH_DISPATCHINFORMATIONPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
