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

#pragma once

#include <com/sun/star/lang/XConnectionPointContainer.hpp>
#include <cppuhelper/propshlp.hxx>
#include <rtl/ref.hxx>

#include <basecontrol.hxx>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::frame { class XFrame2; }
namespace unocontrols { class OConnectionPointContainerHelper; }

namespace unocontrols {

class FrameControl final : public css::awt::XControlModel
                    , public css::lang::XConnectionPointContainer
                    , public BaseControl                                // This order is necessary for right initialization of m_aMutex!
                    , public ::cppu::OBroadcastHelper
                    , public ::cppu::OPropertySetHelper
{
public:

    FrameControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual ~FrameControl() override;

    //  XInterface

    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type& aType
    ) override;

    /**
        @short      increment refcount
        @seealso    XInterface
        @seealso    release()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() noexcept override;

    /**
        @short      decrement refcount
        @seealso    XInterface
        @seealso    acquire()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() noexcept override;

    //  XTypeProvider

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    //  XAggregation

    css::uno::Any SAL_CALL queryAggregation(
        const css::uno::Type& aType
    ) override;

    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    //  XControl

    virtual void SAL_CALL createPeer(
        const css::uno::Reference< css::awt::XToolkit >&      xToolkit ,
        const css::uno::Reference< css::awt::XWindowPeer >&   xParent
    ) override;

    virtual sal_Bool SAL_CALL setModel(
        const css::uno::Reference< css::awt::XControlModel >& xModel
    ) override;

    virtual css::uno::Reference< css::awt::XControlModel > SAL_CALL getModel() override;

    //  XComponent

    virtual void SAL_CALL dispose() override;

    //  XView

    virtual sal_Bool SAL_CALL setGraphics(
        const css::uno::Reference< css::awt::XGraphics >& xDevice
    ) override;

    virtual css::uno::Reference< css::awt::XGraphics > SAL_CALL getGraphics() override;

    //  XConnectionPointContainer

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getConnectionPointTypes() override;

    virtual css::uno::Reference< css::lang::XConnectionPoint > SAL_CALL queryConnectionPoint(
        const css::uno::Type& aType
    ) override;

    virtual void SAL_CALL advise(
        const css::uno::Type&                                aType       ,
        const css::uno::Reference< css::uno::XInterface >&    xListener
    ) override;

    virtual void SAL_CALL unadvise(
        const css::uno::Type&                                aType ,
        const css::uno::Reference< css::uno::XInterface >&    xListener
    ) override;

private:

    using OPropertySetHelper::getFastPropertyValue;

    //  OPropertySetHelper

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        css::uno::Any&       rConvertedValue   ,
        css::uno::Any&       rOldValue         ,
        sal_Int32           nHandle           ,
        const css::uno::Any& rValue
    ) override;

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle ,
        const css::uno::Any& rValue
    ) override;

    virtual void SAL_CALL getFastPropertyValue( css::uno::Any&   rValue  ,
                                                sal_Int32       nHandle ) const override;

    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    //  XPropertySet

    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;

    //  BaseControl

    virtual css::awt::WindowDescriptor impl_getWindowDescriptor(
        const css::uno::Reference< css::awt::XWindowPeer >& xParentPeer
    ) override;

    void impl_createFrame(  const css::uno::Reference< css::awt::XWindowPeer >&       xPeer           ,
                            const OUString&                                         sURL            ,
                            const css::uno::Sequence< css::beans::PropertyValue >&    seqArguments    );

    void impl_deleteFrame();

    css::uno::Reference< css::frame::XFrame2 >              m_xFrame;
    OUString                                                m_sComponentURL;
    css::uno::Sequence< css::beans::PropertyValue >         m_seqLoaderArguments;
    rtl::Reference<OConnectionPointContainerHelper>         m_aConnectionPointContainer;

};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
