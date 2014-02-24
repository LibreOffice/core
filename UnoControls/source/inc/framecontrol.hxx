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

#ifndef _UNOCONTROLS_FRAMECONTROL_CTRL_HXX
#define _UNOCONTROLS_FRAMECONTROL_CTRL_HXX

#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame2.hpp>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XConnectionPointContainer.hpp>
#include <cppuhelper/propshlp.hxx>

#include "basecontrol.hxx"
#include "OConnectionPointContainerHelper.hxx"


//  namespaces


namespace unocontrols{

#define SERVICENAME_FRAMECONTROL                        "com.sun.star.frame.FrameControl"
#define IMPLEMENTATIONNAME_FRAMECONTROL                 "stardiv.UnoControls.FrameControl"
#define PROPERTYNAME_LOADERARGUMENTS                    "LoaderArguments"
#define PROPERTYNAME_COMPONENTURL                       "ComponentURL"
#define PROPERTYNAME_FRAME                              "Frame"
#define ERRORTEXT_VOSENSHURE                            "This is an invalid property handle."
#define PROPERTY_COUNT                                  3                                                       // you must count the propertys
#define PROPERTYHANDLE_COMPONENTURL                     0                                                       // Id must be the index into the array
#define PROPERTYHANDLE_FRAME                            1
#define PROPERTYHANDLE_LOADERARGUMENTS                  2


//  class


class FrameControl  : public ::com::sun::star::awt::XControlModel
                    , public ::com::sun::star::lang::XConnectionPointContainer
                    , public BaseControl                                // This order is necessary for right initialization of m_aMutex!
                    , public ::cppu::OBroadcastHelper
                    , public ::cppu::OPropertySetHelper
{


//  public methods


public:


    //  construct/destruct


    FrameControl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

    virtual ~FrameControl();


    //  XInterface


    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type& aType
    ) throw( ::com::sun::star::uno::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      increment refcount
        @descr      -

        @seealso    XInterface
        @seealso    release()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw();

    /**_______________________________________________________________________________________________________
        @short      decrement refcount
        @descr      -

        @seealso    XInterface
        @seealso    acquire()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw();


    //  XTypeProvider


    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException );


    //  XAggregation


    ::com::sun::star::uno::Any SAL_CALL queryAggregation(
        const ::com::sun::star::uno::Type& aType
    ) throw( ::com::sun::star::uno::RuntimeException );


    //  XControl


    virtual void SAL_CALL createPeer(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >&      xToolkit ,
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >&   xParent
    ) throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL setModel(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xModel
    ) throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel()
        throw( ::com::sun::star::uno::RuntimeException );


    //  XComponent


    virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );


    //  XView


    virtual sal_Bool SAL_CALL setGraphics(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xDevice
    ) throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > SAL_CALL getGraphics()
        throw( ::com::sun::star::uno::RuntimeException );


    //  XConnectionPointContainer


    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getConnectionPointTypes()
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XConnectionPoint > SAL_CALL queryConnectionPoint(
        const ::com::sun::star::uno::Type& aType
    ) throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL advise(
        const ::com::sun::star::uno::Type&                                aType       ,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&    xListener
    ) throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL unadvise(
        const ::com::sun::star::uno::Type&                                aType ,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&    xListener
    ) throw( ::com::sun::star::uno::RuntimeException );


    //  impl but public methods to register service!


    static const ::com::sun::star::uno::Sequence< OUString > impl_getStaticSupportedServiceNames();

    static const OUString impl_getStaticImplementationName();


//  protected methods


protected:
    using OPropertySetHelper::getFastPropertyValue;

    //  OPropertySetHelper


    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        ::com::sun::star::uno::Any&       rConvertedValue   ,
        ::com::sun::star::uno::Any&       rOldValue         ,
        sal_Int32           nHandle           ,
        const ::com::sun::star::uno::Any& rValue
    ) throw( ::com::sun::star::lang::IllegalArgumentException );

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle ,
        const ::com::sun::star::uno::Any& rValue
    ) throw ( ::com::sun::star::uno::Exception );

    virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any&   rValue  ,
                                                sal_Int32       nHandle ) const ;

    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();


    //  XPropertySet


    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw( ::com::sun::star::uno::RuntimeException );


    //  BaseControl


    virtual ::com::sun::star::awt::WindowDescriptor* impl_getWindowDescriptor(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParentPeer
    );


//  private methods


private:

    void impl_createFrame(  const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >&       xPeer           ,
                            const OUString&                                         sURL            ,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&    seqArguments    );

    void impl_deleteFrame();

    static const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > impl_getStaticPropertyDescriptor();



//  private variables


private:

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame2 >        m_xFrame                    ;
    OUString                                        m_sComponentURL             ;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   m_seqLoaderArguments        ;
    ::cppu::OMultiTypeInterfaceContainerHelper      m_aInterfaceContainer       ;
    OConnectionPointContainerHelper                 m_aConnectionPointContainer ;

};  // class FrameControl

}   // namespace unocontrols

#endif  // #ifndef _UNOCONTROLS_FRAMECONTROL_CTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
