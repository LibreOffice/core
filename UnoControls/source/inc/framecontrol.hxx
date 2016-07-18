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

#ifndef INCLUDED_UNOCONTROLS_SOURCE_INC_FRAMECONTROL_HXX
#define INCLUDED_UNOCONTROLS_SOURCE_INC_FRAMECONTROL_HXX

#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame2.hpp>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XConnectionPointContainer.hpp>
#include <cppuhelper/propshlp.hxx>
#include <rtl/ref.hxx>

#include "basecontrol.hxx"
#include "OConnectionPointContainerHelper.hxx"

//  namespaces

namespace unocontrols{

#define SERVICENAME_FRAMECONTROL                        "com.sun.star.frame.FrameControl"
#define IMPLEMENTATIONNAME_FRAMECONTROL                 "stardiv.UnoControls.FrameControl"
#define PROPERTYNAME_LOADERARGUMENTS                    "LoaderArguments"
#define PROPERTYNAME_COMPONENTURL                       "ComponentURL"
#define PROPERTYNAME_FRAME                              "Frame"
#define PROPERTY_COUNT                                  3                        // you must count the properties
#define PROPERTYHANDLE_COMPONENTURL                     0                        // Id must be the index into the array
#define PROPERTYHANDLE_FRAME                            1
#define PROPERTYHANDLE_LOADERARGUMENTS                  2

//  class

class FrameControl  : public css::awt::XControlModel
                    , public css::lang::XConnectionPointContainer
                    , public BaseControl                                // This order is necessary for right initialization of m_aMutex!
                    , public ::cppu::OBroadcastHelper
                    , public ::cppu::OPropertySetHelper
{

//  public methods

public:

    //  construct/destruct

    FrameControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual ~FrameControl();

    //  XInterface

    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type& aType
    ) throw( css::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
        @short      increment refcount
        @seealso    XInterface
        @seealso    release()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw() override;

    /**_______________________________________________________________________________________________________
        @short      decrement refcount
        @seealso    XInterface
        @seealso    acquire()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw() override;

    //  XTypeProvider

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    //  XAggregation

    css::uno::Any SAL_CALL queryAggregation(
        const css::uno::Type& aType
    ) throw( css::uno::RuntimeException, std::exception ) override;

    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    //  XControl

    virtual void SAL_CALL createPeer(
        const css::uno::Reference< css::awt::XToolkit >&      xToolkit ,
        const css::uno::Reference< css::awt::XWindowPeer >&   xParent
    ) throw ( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL setModel(
        const css::uno::Reference< css::awt::XControlModel >& xModel
    ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::awt::XControlModel > SAL_CALL getModel()
        throw( css::uno::RuntimeException, std::exception ) override;

    //  XComponent

    virtual void SAL_CALL dispose() throw( css::uno::RuntimeException, std::exception ) override;

    //  XView

    virtual sal_Bool SAL_CALL setGraphics(
        const css::uno::Reference< css::awt::XGraphics >& xDevice
    ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::awt::XGraphics > SAL_CALL getGraphics()
        throw( css::uno::RuntimeException, std::exception ) override;

    //  XConnectionPointContainer

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getConnectionPointTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::lang::XConnectionPoint > SAL_CALL queryConnectionPoint(
        const css::uno::Type& aType
    ) throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL advise(
        const css::uno::Type&                                aType       ,
        const css::uno::Reference< css::uno::XInterface >&    xListener
    ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL unadvise(
        const css::uno::Type&                                aType ,
        const css::uno::Reference< css::uno::XInterface >&    xListener
    ) throw( css::uno::RuntimeException, std::exception ) override;

    //  impl but public methods to register service!

    static const css::uno::Sequence< OUString > impl_getStaticSupportedServiceNames();

    static const OUString impl_getStaticImplementationName();

//  protected methods

protected:
    using OPropertySetHelper::getFastPropertyValue;

    //  OPropertySetHelper

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        css::uno::Any&       rConvertedValue   ,
        css::uno::Any&       rOldValue         ,
        sal_Int32           nHandle           ,
        const css::uno::Any& rValue
    ) throw( css::lang::IllegalArgumentException ) override;

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle ,
        const css::uno::Any& rValue
    ) throw ( css::uno::Exception, std::exception ) override;

    virtual void SAL_CALL getFastPropertyValue( css::uno::Any&   rValue  ,
                                                sal_Int32       nHandle ) const override;

    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    //  XPropertySet

    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw( css::uno::RuntimeException, std::exception ) override;

    //  BaseControl

    virtual css::awt::WindowDescriptor* impl_getWindowDescriptor(
        const css::uno::Reference< css::awt::XWindowPeer >& xParentPeer
    ) override;

//  private methods

private:

    void impl_createFrame(  const css::uno::Reference< css::awt::XWindowPeer >&       xPeer           ,
                            const OUString&                                         sURL            ,
                            const css::uno::Sequence< css::beans::PropertyValue >&    seqArguments    );

    void impl_deleteFrame();

    static const css::uno::Sequence< css::beans::Property >& impl_getStaticPropertyDescriptor();

//  private variables

private:

    css::uno::Reference< css::frame::XFrame2 >              m_xFrame;
    OUString                                                m_sComponentURL;
    css::uno::Sequence< css::beans::PropertyValue >         m_seqLoaderArguments;
    rtl::Reference<OConnectionPointContainerHelper>         m_aConnectionPointContainer;

};  // class FrameControl

}   // namespace unocontrols

#endif // INCLUDED_UNOCONTROLS_SOURCE_INC_FRAMECONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
