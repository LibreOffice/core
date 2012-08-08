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
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XConnectionPointContainer.hpp>
#include <cppuhelper/propshlp.hxx>

#include "basecontrol.hxx"
#include "OConnectionPointContainerHelper.hxx"

//______________________________________________________________________________________________________________
//  namespaces
//______________________________________________________________________________________________________________

namespace unocontrols{

//______________________________________________________________________________________________________________
//  defines
//______________________________________________________________________________________________________________

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

//______________________________________________________________________________________________________________
//  class
//______________________________________________________________________________________________________________

class FrameControl  : public ::com::sun::star::awt::XControlModel
                    , public ::com::sun::star::lang::XConnectionPointContainer
                    , public BaseControl                                // This order is neccessary for right initialization of m_aMutex!
                    , public ::cppu::OBroadcastHelper
                    , public ::cppu::OPropertySetHelper
{

//______________________________________________________________________________________________________________
//  public methods
//______________________________________________________________________________________________________________

public:

    //__________________________________________________________________________________________________________
    //  construct/destruct
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    FrameControl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ~FrameControl();

    //__________________________________________________________________________________________________________
    //  XInterface
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

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

    //__________________________________________________________________________________________________________
    //  XTypeProvider
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XAggregation
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    ::com::sun::star::uno::Any SAL_CALL queryAggregation(
        const ::com::sun::star::uno::Type& aType
    ) throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XControl
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL createPeer(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >&      xToolkit ,
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >&   xParent
    ) throw ( ::com::sun::star::uno::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual sal_Bool SAL_CALL setModel(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xModel
    ) throw( ::com::sun::star::uno::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel()
        throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XComponent
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XView
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual sal_Bool SAL_CALL setGraphics(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xDevice
    ) throw( ::com::sun::star::uno::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > SAL_CALL getGraphics()
        throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XConnectionPointContainer
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getConnectionPointTypes()
        throw( ::com::sun::star::uno::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XConnectionPoint > SAL_CALL queryConnectionPoint(
        const ::com::sun::star::uno::Type& aType
    ) throw ( ::com::sun::star::uno::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL advise(
        const ::com::sun::star::uno::Type&                                aType       ,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&    xListener
    ) throw( ::com::sun::star::uno::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL unadvise(
        const ::com::sun::star::uno::Type&                                aType ,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&    xListener
    ) throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  impl but public methods to register service!
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    static const ::com::sun::star::uno::Sequence< ::rtl::OUString > impl_getStaticSupportedServiceNames();

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    static const ::rtl::OUString impl_getStaticImplementationName();

//______________________________________________________________________________________________________________
//  protected methods
//______________________________________________________________________________________________________________

protected:
    using OPropertySetHelper::getFastPropertyValue;
    //__________________________________________________________________________________________________________
    //  OPropertySetHelper
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        ::com::sun::star::uno::Any&       rConvertedValue   ,
        ::com::sun::star::uno::Any&       rOldValue         ,
        sal_Int32           nHandle           ,
        const ::com::sun::star::uno::Any& rValue
    ) throw( ::com::sun::star::lang::IllegalArgumentException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle ,
        const ::com::sun::star::uno::Any& rValue
    ) throw ( ::com::sun::star::uno::Exception );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any&   rValue  ,
                                                sal_Int32       nHandle ) const ;

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    //__________________________________________________________________________________________________________
    //  XPropertySet
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  BaseControl
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::com::sun::star::awt::WindowDescriptor* impl_getWindowDescriptor(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParentPeer
    );

//______________________________________________________________________________________________________________
//  private methods
//______________________________________________________________________________________________________________

private:

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    void impl_createFrame(  const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >&       xPeer           ,
                            const ::rtl::OUString&                                  sURL            ,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&    seqArguments    );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    void impl_deleteFrame();

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    static const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > impl_getStaticPropertyDescriptor();


//______________________________________________________________________________________________________________
//  private variables
//______________________________________________________________________________________________________________

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >         m_xFrame                    ;
    ::rtl::OUString                                 m_sComponentURL             ;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   m_seqLoaderArguments        ;
    ::cppu::OMultiTypeInterfaceContainerHelper      m_aInterfaceContainer       ;
    OConnectionPointContainerHelper                 m_aConnectionPointContainer ;

};  // class FrameControl

}   // namespace unocontrols

#endif  // #ifndef _UNOCONTROLS_FRAMECONTROL_CTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
