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
#include <com/sun/star/frame/XFrameControl.hpp>
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

//______________________________________________________________________________________________________________
//  namespaces
//______________________________________________________________________________________________________________

namespace unocontrols{

//______________________________________________________________________________________________________________
//  class
//______________________________________________________________________________________________________________

class FrameControl  : public ::com::sun::star::awt::XControlModel
                    , public ::com::sun::star::lang::XConnectionPointContainer
                    , public ::com::sun::star::frame::XFrameControl
                    , public BaseControl                                // This order is necessary for right initialization of m_aMutex!
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

    FrameControl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

    virtual ~FrameControl();

    //__________________________________________________________________________________________________________
    //  XInterface
    //__________________________________________________________________________________________________________

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

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XAggregation
    //__________________________________________________________________________________________________________

    ::com::sun::star::uno::Any SAL_CALL queryAggregation(
        const ::com::sun::star::uno::Type& aType
    ) throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XControl
    //__________________________________________________________________________________________________________

    virtual void SAL_CALL createPeer(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >&      xToolkit ,
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >&   xParent
    ) throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL setModel(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xModel
    ) throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel()
        throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XComponent
    //__________________________________________________________________________________________________________

    virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XView
    //__________________________________________________________________________________________________________

    virtual sal_Bool SAL_CALL setGraphics(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xDevice
    ) throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > SAL_CALL getGraphics()
        throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XConnectionPointContainer
    //__________________________________________________________________________________________________________

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


    //__________________________________________________________________________________________________________
    //  XFrameControl
    //__________________________________________________________________________________________________________

    virtual rtl::OUString SAL_CALL getComponentURL()
        throw( ::com::sun::star::uno::RuntimeException )
    { return m_sComponentURL; }
    virtual void SAL_CALL setComponentURL(const rtl::OUString& rVal)
        throw( ::com::sun::star::uno::RuntimeException )
    { m_sComponentURL = rVal; }
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame2 > SAL_CALL getFrame()
        throw( ::com::sun::star::uno::RuntimeException )
    { return m_xFrame; }
    virtual void SAL_CALL setFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame2 > & rxFrame)
        throw( ::com::sun::star::uno::RuntimeException )
    { m_xFrame = rxFrame; }
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getLoaderArguments()
        throw( ::com::sun::star::uno::RuntimeException )
    { return m_seqLoaderArguments; }
    virtual void SAL_CALL setLoaderArguments(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & rVal)
        throw( ::com::sun::star::uno::RuntimeException )
    { m_seqLoaderArguments = rVal; }

    //__________________________________________________________________________________________________________
    //  overrides to remove inheritance ambiguity
    //__________________________________________________________________________________________________________
    virtual css::uno::Reference<css::awt::XWindowPeer> SAL_CALL getPeer() throw (css::uno::RuntimeException)
        { return BaseControl::getPeer(); }
    virtual css::awt::Rectangle SAL_CALL getPosSize() throw (css::uno::RuntimeException)
        { return BaseControl::getPosSize(); }
    virtual void SAL_CALL setPosSize(sal_Int32 p1, sal_Int32 p2, sal_Int32 p3, sal_Int32 p4, sal_Int16 p5) throw (css::uno::RuntimeException)
        { return BaseControl::setPosSize(p1, p2, p3, p4, p5); }
    virtual void SAL_CALL addEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) throw (css::uno::RuntimeException)
        { return BaseControl::addEventListener(p1); }
    virtual void SAL_CALL removeEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) throw (css::uno::RuntimeException)
        { return BaseControl::removeEventListener(p1); }
    virtual void SAL_CALL setVisible(sal_Bool p1) throw (css::uno::RuntimeException)
       { return BaseControl::setVisible(p1); }
    virtual void SAL_CALL setEnable(sal_Bool p1) throw (css::uno::RuntimeException)
        { return BaseControl::setEnable(p1); }
    virtual void SAL_CALL setFocus() throw (css::uno::RuntimeException)
        { return BaseControl::setFocus(); }
    virtual void SAL_CALL addWindowListener(const css::uno::Reference<css::awt::XWindowListener>& p1) throw (css::uno::RuntimeException)
        { return BaseControl::addWindowListener(p1); }
    virtual void SAL_CALL removeWindowListener(const css::uno::Reference<css::awt::XWindowListener>& p1) throw (css::uno::RuntimeException)
        { return BaseControl::removeWindowListener(p1); }
    virtual void SAL_CALL addFocusListener(const css::uno::Reference<css::awt::XFocusListener>& p1) throw (css::uno::RuntimeException)
        { return BaseControl::addFocusListener(p1); }
    virtual void SAL_CALL removeFocusListener(const css::uno::Reference<css::awt::XFocusListener>& p1) throw (css::uno::RuntimeException)
        { return BaseControl::removeFocusListener(p1); }
    virtual void SAL_CALL addKeyListener(const css::uno::Reference<css::awt::XKeyListener>& p1) throw (css::uno::RuntimeException)
        { return BaseControl::addKeyListener(p1); }
    virtual void SAL_CALL removeKeyListener(const css::uno::Reference<css::awt::XKeyListener>& p1) throw (css::uno::RuntimeException)
        { return BaseControl::removeKeyListener(p1); }
    virtual void SAL_CALL addMouseListener(const css::uno::Reference<css::awt::XMouseListener>& p1) throw (css::uno::RuntimeException)
        { return BaseControl::addMouseListener(p1); }
    virtual void SAL_CALL removeMouseListener(const css::uno::Reference<css::awt::XMouseListener>& p1) throw (css::uno::RuntimeException)
        { return BaseControl::removeMouseListener(p1); }
    virtual void SAL_CALL addMouseMotionListener(const css::uno::Reference<css::awt::XMouseMotionListener>& p1) throw (css::uno::RuntimeException)
        { return BaseControl::addMouseMotionListener(p1); }
    virtual void SAL_CALL removeMouseMotionListener(const css::uno::Reference<css::awt::XMouseMotionListener>& p1) throw (css::uno::RuntimeException)
        { return BaseControl::removeMouseMotionListener(p1); }
    virtual void SAL_CALL addPaintListener(const css::uno::Reference<css::awt::XPaintListener>& p1) throw (css::uno::RuntimeException)
        { return BaseControl::addPaintListener(p1); }
    virtual void SAL_CALL removePaintListener(const css::uno::Reference<css::awt::XPaintListener>& p1) throw (css::uno::RuntimeException)
        { return BaseControl::removePaintListener(p1); }
    virtual css::awt::Size SAL_CALL getSize() throw (css::uno::RuntimeException)
        { return BaseControl::getSize(); }
    virtual void SAL_CALL draw(sal_Int32 p1, sal_Int32 p2) throw (css::uno::RuntimeException)
        { return BaseControl::draw(p1, p2); }
    virtual void SAL_CALL setZoom(float p1, float p2) throw (css::uno::RuntimeException)
        { return BaseControl::setZoom(p1, p2); }
    virtual void SAL_CALL setPropertyValue(const rtl::OUString& p1, const css::uno::Any& p2) throw (css::uno::RuntimeException)
        { return ::cppu::OPropertySetHelper::setPropertyValue(p1, p2); }
    virtual css::uno::Any SAL_CALL getPropertyValue(const rtl::OUString& p1) throw (css::uno::RuntimeException)
        { return ::cppu::OPropertySetHelper::getPropertyValue(p1); }
    virtual void SAL_CALL SAL_CALL addPropertyChangeListener(const rtl::OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) throw (css::uno::RuntimeException)
        { return ::cppu::OPropertySetHelper::addPropertyChangeListener(p1, p2); }
    virtual void SAL_CALL removePropertyChangeListener(const rtl::OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) throw (css::uno::RuntimeException)
        { return ::cppu::OPropertySetHelper::removePropertyChangeListener(p1, p2); }
    virtual void SAL_CALL addVetoableChangeListener(const rtl::OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) throw (css::uno::RuntimeException)
        { return ::cppu::OPropertySetHelper::addVetoableChangeListener(p1, p2); }
    virtual void SAL_CALL removeVetoableChangeListener(const rtl::OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) throw (css::uno::RuntimeException)
        { return ::cppu::OPropertySetHelper::removeVetoableChangeListener(p1, p2); }
    virtual void SAL_CALL setContext(const css::uno::Reference<css::uno::XInterface>& p1) throw (css::uno::RuntimeException)
        { return BaseControl::setContext(p1); }
    virtual css::uno::Reference<css::uno::XInterface> SAL_CALL getContext() throw (css::uno::RuntimeException)
        { return BaseControl::getContext(); }
    virtual css::uno::Reference<css::awt::XView> SAL_CALL getView() throw (css::uno::RuntimeException)
        { return BaseControl::getView(); }
    virtual void SAL_CALL setDesignMode(sal_Bool p1) throw (css::uno::RuntimeException)
        { return BaseControl::setDesignMode(p1); }
    virtual sal_Bool SAL_CALL isDesignMode() throw (css::uno::RuntimeException)
        { return BaseControl::isDesignMode(); }
    virtual sal_Bool SAL_CALL isTransparent() throw (css::uno::RuntimeException)
        { return BaseControl::isTransparent(); }

    //__________________________________________________________________________________________________________
    //  impl but public methods to register service!
    //__________________________________________________________________________________________________________

    static const ::com::sun::star::uno::Sequence< OUString > impl_getStaticSupportedServiceNames();

    static const OUString impl_getStaticImplementationName();

//______________________________________________________________________________________________________________
//  protected methods
//______________________________________________________________________________________________________________

protected:
    using OPropertySetHelper::getFastPropertyValue;
    //__________________________________________________________________________________________________________
    //  OPropertySetHelper
    //__________________________________________________________________________________________________________

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

    //__________________________________________________________________________________________________________
    //  XPropertySet
    //__________________________________________________________________________________________________________

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  BaseControl
    //__________________________________________________________________________________________________________

    virtual ::com::sun::star::awt::WindowDescriptor* impl_getWindowDescriptor(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParentPeer
    );

//______________________________________________________________________________________________________________
//  private methods
//______________________________________________________________________________________________________________

private:

    void impl_createFrame(  const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >&       xPeer           ,
                            const OUString&                                         sURL            ,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&    seqArguments    );

    void impl_deleteFrame();

    static const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > impl_getStaticPropertyDescriptor();


//______________________________________________________________________________________________________________
//  private variables
//______________________________________________________________________________________________________________

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
