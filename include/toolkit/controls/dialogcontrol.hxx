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

#ifndef INCLUDED_TOOLKIT_CONTROLS_DIALOGCONTROL_HXX
#define INCLUDED_TOOLKIT_CONTROLS_DIALOGCONTROL_HXX

#include <toolkit/controls/controlmodelcontainerbase.hxx>
#include <com/sun/star/awt/UnoControlDialog.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XDialog2.hpp>
#include <com/sun/star/awt/XSimpleTabController.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <com/sun/star/graphic/XGraphicObject.hpp>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/controls/unocontrolcontainer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/implbase.hxx>
#include <list>

typedef ::cppu::AggImplInheritanceHelper   <   ControlContainerBase
                                           ,   css::awt::XUnoControlDialog
                                           ,   css::awt::XWindowListener
                                           >   UnoDialogControl_Base;
class UnoDialogControl : public UnoDialogControl_Base
{
private:
    css::uno::Reference< css::awt::XMenuBar >         mxMenuBar;
    TopWindowListenerMultiplexer                                                maTopWindowListeners;
    bool                                                                        mbWindowListener;

public:

                                UnoDialogControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                                virtual ~UnoDialogControl();
    OUString             GetComponentServiceName() override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTopWindow
    void SAL_CALL addTopWindowListener( const css::uno::Reference< css::awt::XTopWindowListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeTopWindowListener( const css::uno::Reference< css::awt::XTopWindowListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL toFront(  ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL toBack(  ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMenuBar( const css::uno::Reference< css::awt::XMenuBar >& xMenu ) throw (css::uno::RuntimeException, std::exception) override;

    // css::awt::XWindowListener
    virtual void SAL_CALL windowResized( const css::awt::WindowEvent& e ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& e ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowShown( const css::lang::EventObject& e ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowHidden( const css::lang::EventObject& e ) throw (css::uno::RuntimeException, std::exception) override;

    // css::awt::XDialog2
    virtual void SAL_CALL endDialog( ::sal_Int32 Result ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHelpId( const OUString& Id ) throw (css::uno::RuntimeException, std::exception) override;

    // css::awt::XDialog
    void SAL_CALL setTitle( const OUString& Title ) throw(css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getTitle() throw(css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL execute() throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL endExecute() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XControl
    sal_Bool SAL_CALL setModel( const css::uno::Reference< css::awt::XControlModel >& Model ) throw(css::uno::RuntimeException, std::exception) override;

    // XModifyListener
    virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) throw (css::uno::RuntimeException, std::exception) override;

    // resolve some ambigous methods
    virtual css::uno::Reference<css::awt::XWindowPeer> SAL_CALL getPeer() throw (css::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::getPeer(); }
    virtual void SAL_CALL addWindowListener(const css::uno::Reference<css::awt::XWindowListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addWindowListener(p1); }
    virtual css::uno::Reference<css::awt::XControlModel> SAL_CALL getModel() throw (css::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::getModel(); }
    virtual void SAL_CALL addEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addEventListener(p1); }
    virtual void SAL_CALL removeEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removeEventListener(p1); }
    virtual void SAL_CALL setContext(const css::uno::Reference<css::uno::XInterface>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::setContext(p1); }
    virtual css::uno::Reference<css::uno::XInterface> SAL_CALL getContext() throw (css::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::getContext(); }
    virtual css::uno::Reference<css::awt::XView> SAL_CALL getView() throw (css::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::getView(); }
    virtual void SAL_CALL setDesignMode(sal_Bool p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::setDesignMode(p1); }
    virtual sal_Bool SAL_CALL isDesignMode() throw (css::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::isDesignMode(); }
    virtual sal_Bool SAL_CALL isTransparent() throw (css::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::isTransparent(); }
    virtual void SAL_CALL setPosSize(sal_Int32 p1, sal_Int32 p2, sal_Int32 p3, sal_Int32 p4, sal_Int16 p5) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::setPosSize(p1, p2, p3, p4, p5); }
    virtual css::awt::Rectangle SAL_CALL getPosSize() throw (css::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::getPosSize(); }
    virtual void SAL_CALL setVisible(sal_Bool p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::setVisible(p1); }
    virtual void SAL_CALL setEnable(sal_Bool p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::setEnable(p1); }
    virtual void SAL_CALL setFocus() throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::setFocus(); }
    virtual void SAL_CALL removeWindowListener(const css::uno::Reference<css::awt::XWindowListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removeWindowListener(p1); }
    virtual void SAL_CALL addFocusListener(const css::uno::Reference<css::awt::XFocusListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addFocusListener(p1); }
    virtual void SAL_CALL removeFocusListener(const css::uno::Reference<css::awt::XFocusListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removeFocusListener(p1); }
    virtual void SAL_CALL addKeyListener(const css::uno::Reference<css::awt::XKeyListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addKeyListener(p1); }
    virtual void SAL_CALL removeKeyListener(const css::uno::Reference<css::awt::XKeyListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removeKeyListener(p1); }
    virtual void SAL_CALL addMouseListener(const css::uno::Reference<css::awt::XMouseListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addMouseListener(p1); }
    virtual void SAL_CALL removeMouseListener(const css::uno::Reference<css::awt::XMouseListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removeMouseListener(p1); }
    virtual void SAL_CALL addMouseMotionListener(const css::uno::Reference<css::awt::XMouseMotionListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addMouseMotionListener(p1); }
    virtual void SAL_CALL removeMouseMotionListener(const css::uno::Reference<css::awt::XMouseMotionListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removeMouseMotionListener(p1); }
    virtual void SAL_CALL addPaintListener(const css::uno::Reference<css::awt::XPaintListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addPaintListener(p1); }
    virtual void SAL_CALL removePaintListener(const css::uno::Reference<css::awt::XPaintListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removePaintListener(p1); }
    virtual void SAL_CALL setStatusText(const rtl::OUString& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::setStatusText(p1); }
    virtual css::uno::Sequence<css::uno::Reference<css::awt::XControl> > SAL_CALL getControls() throw (css::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::getControls(); }
    virtual css::uno::Reference<css::awt::XControl> SAL_CALL getControl(const rtl::OUString& p1) throw (css::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::getControl(p1); }
    virtual void SAL_CALL addControl(const rtl::OUString& p1, const css::uno::Reference<css::awt::XControl>& p2) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addControl(p1, p2); }
    virtual void SAL_CALL removeControl(const css::uno::Reference<css::awt::XControl>& p1) throw (css::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removeControl(p1); }


    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    virtual void PrepareWindowDescriptor( css::awt::WindowDescriptor& rDesc ) override;
    virtual void ImplModelPropertiesChanged( const css::uno::Sequence< css::beans::PropertyChangeEvent >& rEvents ) throw(css::uno::RuntimeException, std::exception) override;
protected:
};

class UnoMultiPageModel : public ControlModelContainerBase
{
public:
    UnoMultiPageModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~UnoMultiPageModel();
    UnoMultiPageModel( const UnoMultiPageModel& rModel );

    UnoControlModel*    Clone() const override;

    DECLIMPL_SERVICEINFO_DERIVED( UnoMultiPageModel, ControlModelContainerBase, "com.sun.star.awt.UnoMultiPageModel" )

    virtual OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw( css::uno::RuntimeException, std::exception) override;
    // XNamedContainer
    void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // Override the method of parent Class
    virtual sal_Bool SAL_CALL getGroupControl(  ) throw (css::uno::RuntimeException, std::exception) override;
protected:
    virtual css::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper() override;

};

class UnoMultiPageControl :  public ControlContainerBase
                            ,public css::awt::XSimpleTabController
                            ,public css::awt::XTabListener
{
    TabListenerMultiplexer maTabListeners;
    void bindPage( const css::uno::Reference< css::awt::XControl >& _rxControl );
public:
    UnoMultiPageControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~UnoMultiPageControl();
    OUString     GetComponentServiceName() override;

    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoMultiPageControl, ControlContainerBase, "com.sun.star.awt.UnoControlMultiPage" )
    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException) override { return ControlContainerBase::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }
    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;
    // css::awt::XSimpleTabController
    virtual ::sal_Int32 SAL_CALL insertTab() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTab( ::sal_Int32 ID ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setTabProps( ::sal_Int32 ID, const css::uno::Sequence< css::beans::NamedValue >& Properties ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::NamedValue > SAL_CALL getTabProps( ::sal_Int32 ID ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL activateTab( ::sal_Int32 ID ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getActiveTabID() throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;
    // XTabListener
    virtual void SAL_CALL inserted( ::sal_Int32 ID ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removed( ::sal_Int32 ID ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL changed( ::sal_Int32 ID, const css::uno::Sequence< css::beans::NamedValue >& Properties ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL activated( ::sal_Int32 ID ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL deactivated( ::sal_Int32 ID ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& evt ) throw (css::uno::RuntimeException, std::exception) override;
    // XComponent
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

protected:
    virtual void    impl_createControlPeerIfNecessary(
        const css::uno::Reference< css::awt::XControl >& _rxControl
    ) override;

};


class UnoPageModel : public ControlModelContainerBase
{
public:
    UnoPageModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~UnoPageModel();
    UnoPageModel( const UnoPageModel& rModel );

    UnoControlModel*    Clone() const override;

    DECLIMPL_SERVICEINFO_DERIVED( UnoPageModel, ControlModelContainerBase, "com.sun.star.awt.UnoPageModel" )

    virtual OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw( css::uno::RuntimeException, std::exception) override;

    // Override the method of parent Class
    virtual sal_Bool SAL_CALL getGroupControl(  ) throw (css::uno::RuntimeException, std::exception) override;
protected:
    virtual css::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper() override;

};

class UnoPageControl :  public ControlContainerBase
{
public:
    UnoPageControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~UnoPageControl();
    OUString     GetComponentServiceName() override;


    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoPageControl, ControlContainerBase, "com.sun.star.awt.UnoControlPage" )
};

class UnoFrameModel : public ControlModelContainerBase
{
public:
    UnoFrameModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~UnoFrameModel();
    UnoFrameModel( const UnoFrameModel& rModel );

    UnoControlModel*    Clone() const override;

    DECLIMPL_SERVICEINFO_DERIVED( UnoFrameModel, ControlModelContainerBase, "com.sun.star.awt.UnoFrameModel" )

    virtual OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw( css::uno::RuntimeException, std::exception) override;

protected:
    virtual css::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper() override;
};

class UnoFrameControl :  public ControlContainerBase
{
protected:
    virtual void        ImplSetPosSize( css::uno::Reference< css::awt::XControl >& rxCtrl ) override;
public:
    UnoFrameControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~UnoFrameControl();
    OUString     GetComponentServiceName() override;

// css::lang::XServiceInfo
DECLIMPL_SERVICEINFO_DERIVED( UnoFrameControl, ControlContainerBase, "com.sun.star.awt.UnoControlFrame" )
};

#endif // INCLUDED_TOOLKIT_CONTROLS_DIALOGCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
