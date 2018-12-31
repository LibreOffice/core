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
#include <com/sun/star/awt/XSimpleTabController.hpp>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/implbase2.hxx>

namespace com { namespace sun { namespace star { namespace awt { class XTopWindowListener; } } } }

typedef ::cppu::AggImplInheritanceHelper2   <   ControlContainerBase
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
                                virtual ~UnoDialogControl() override;
    OUString             GetComponentServiceName() override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
    void SAL_CALL dispose() override;

    // css::awt::XTopWindow
    void SAL_CALL addTopWindowListener( const css::uno::Reference< css::awt::XTopWindowListener >& xListener ) override;
    void SAL_CALL removeTopWindowListener( const css::uno::Reference< css::awt::XTopWindowListener >& xListener ) override;
    void SAL_CALL toFront(  ) override;
    void SAL_CALL toBack(  ) override;
    void SAL_CALL setMenuBar( const css::uno::Reference< css::awt::XMenuBar >& xMenu ) override;

    // css::awt::XWindowListener
    virtual void SAL_CALL windowResized( const css::awt::WindowEvent& e ) override;
    virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& e ) override;
    virtual void SAL_CALL windowShown( const css::lang::EventObject& e ) override;
    virtual void SAL_CALL windowHidden( const css::lang::EventObject& e ) override;

    // css::awt::XDialog2
    virtual void SAL_CALL endDialog( ::sal_Int32 Result ) override;
    virtual void SAL_CALL setHelpId( const OUString& Id ) override;

    // css::awt::XDialog
    void SAL_CALL setTitle( const OUString& Title ) override;
    OUString SAL_CALL getTitle() override;
    sal_Int16 SAL_CALL execute() override;
    void SAL_CALL endExecute() override;

    // css::awt::XControl
    sal_Bool SAL_CALL setModel( const css::uno::Reference< css::awt::XControlModel >& Model ) override;

    // XModifyListener
    virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) override;

    // resolve some ambiguous methods
    virtual css::uno::Reference<css::awt::XWindowPeer> SAL_CALL getPeer() override
        { return UnoDialogControl_Base::ControlContainerBase::getPeer(); }
    virtual void SAL_CALL addWindowListener(const css::uno::Reference<css::awt::XWindowListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::addWindowListener(p1); }
    virtual css::uno::Reference<css::awt::XControlModel> SAL_CALL getModel() override
        { return UnoDialogControl_Base::ControlContainerBase::getModel(); }
    virtual void SAL_CALL addEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::addEventListener(p1); }
    virtual void SAL_CALL removeEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removeEventListener(p1); }
    virtual void SAL_CALL setContext(const css::uno::Reference<css::uno::XInterface>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::setContext(p1); }
    virtual css::uno::Reference<css::uno::XInterface> SAL_CALL getContext() override
        { return UnoDialogControl_Base::ControlContainerBase::getContext(); }
    virtual css::uno::Reference<css::awt::XView> SAL_CALL getView() override
        { return UnoDialogControl_Base::ControlContainerBase::getView(); }
    virtual void SAL_CALL setDesignMode(sal_Bool p1) override
        { UnoDialogControl_Base::ControlContainerBase::setDesignMode(p1); }
    virtual sal_Bool SAL_CALL isDesignMode() override
        { return UnoDialogControl_Base::ControlContainerBase::isDesignMode(); }
    virtual sal_Bool SAL_CALL isTransparent() override
        { return UnoDialogControl_Base::ControlContainerBase::isTransparent(); }
    virtual void SAL_CALL setPosSize(sal_Int32 p1, sal_Int32 p2, sal_Int32 p3, sal_Int32 p4, sal_Int16 p5) override
        { UnoDialogControl_Base::ControlContainerBase::setPosSize(p1, p2, p3, p4, p5); }
    virtual css::awt::Rectangle SAL_CALL getPosSize() override
        { return UnoDialogControl_Base::ControlContainerBase::getPosSize(); }
    virtual void SAL_CALL setVisible(sal_Bool p1) override
        { UnoDialogControl_Base::ControlContainerBase::setVisible(p1); }
    virtual void SAL_CALL setEnable(sal_Bool p1) override
        { UnoDialogControl_Base::ControlContainerBase::setEnable(p1); }
    virtual void SAL_CALL setFocus() override
        { UnoDialogControl_Base::ControlContainerBase::setFocus(); }
    virtual void SAL_CALL removeWindowListener(const css::uno::Reference<css::awt::XWindowListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removeWindowListener(p1); }
    virtual void SAL_CALL addFocusListener(const css::uno::Reference<css::awt::XFocusListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::addFocusListener(p1); }
    virtual void SAL_CALL removeFocusListener(const css::uno::Reference<css::awt::XFocusListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removeFocusListener(p1); }
    virtual void SAL_CALL addKeyListener(const css::uno::Reference<css::awt::XKeyListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::addKeyListener(p1); }
    virtual void SAL_CALL removeKeyListener(const css::uno::Reference<css::awt::XKeyListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removeKeyListener(p1); }
    virtual void SAL_CALL addMouseListener(const css::uno::Reference<css::awt::XMouseListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::addMouseListener(p1); }
    virtual void SAL_CALL removeMouseListener(const css::uno::Reference<css::awt::XMouseListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removeMouseListener(p1); }
    virtual void SAL_CALL addMouseMotionListener(const css::uno::Reference<css::awt::XMouseMotionListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::addMouseMotionListener(p1); }
    virtual void SAL_CALL removeMouseMotionListener(const css::uno::Reference<css::awt::XMouseMotionListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removeMouseMotionListener(p1); }
    virtual void SAL_CALL addPaintListener(const css::uno::Reference<css::awt::XPaintListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::addPaintListener(p1); }
    virtual void SAL_CALL removePaintListener(const css::uno::Reference<css::awt::XPaintListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removePaintListener(p1); }
    virtual void SAL_CALL setStatusText(const OUString& p1) override
        { UnoDialogControl_Base::ControlContainerBase::setStatusText(p1); }
    virtual css::uno::Sequence<css::uno::Reference<css::awt::XControl> > SAL_CALL getControls() override
        { return UnoDialogControl_Base::ControlContainerBase::getControls(); }
    virtual css::uno::Reference<css::awt::XControl> SAL_CALL getControl(const OUString& p1) override
        { return UnoDialogControl_Base::ControlContainerBase::getControl(p1); }
    virtual void SAL_CALL addControl(const OUString& p1, const css::uno::Reference<css::awt::XControl>& p2) override
        { UnoDialogControl_Base::ControlContainerBase::addControl(p1, p2); }
    virtual void SAL_CALL removeControl(const css::uno::Reference<css::awt::XControl>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removeControl(p1); }


    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

protected:
    virtual void PrepareWindowDescriptor( css::awt::WindowDescriptor& rDesc ) override;
    virtual void ImplModelPropertiesChanged( const css::uno::Sequence< css::beans::PropertyChangeEvent >& rEvents ) override;
protected:
};

class UnoMultiPageModel : public ControlModelContainerBase
{
public:
    UnoMultiPageModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~UnoMultiPageModel() override;

    UnoMultiPageModel(UnoMultiPageModel const &) = default;
    UnoMultiPageModel(UnoMultiPageModel &&) = default;
    UnoMultiPageModel & operator =(UnoMultiPageModel const &) = delete; // due to ControlModelContainerBase
    UnoMultiPageModel & operator =(UnoMultiPageModel &&) = delete; // due to ControlModelContainerBase

    rtl::Reference<UnoControlModel> Clone() const override;

    DECLIMPL_SERVICEINFO_DERIVED( UnoMultiPageModel, ControlModelContainerBase, "com.sun.star.awt.UnoMultiPageModel" )

    virtual OUString SAL_CALL getServiceName() override;
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    // XNamedContainer
    void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) override;

    // Override the method of parent class
    virtual sal_Bool SAL_CALL getGroupControl(  ) override;
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
    virtual ~UnoMultiPageControl() override;
    OUString     GetComponentServiceName() override;

    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoMultiPageControl, ControlContainerBase, "com.sun.star.awt.UnoControlMultiPage" )
    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override { return ControlContainerBase::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }
    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;
    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    // css::awt::XSimpleTabController
    virtual ::sal_Int32 SAL_CALL insertTab() override;
    virtual void SAL_CALL removeTab( ::sal_Int32 ID ) override;

    virtual void SAL_CALL setTabProps( ::sal_Int32 ID, const css::uno::Sequence< css::beans::NamedValue >& Properties ) override;
    virtual css::uno::Sequence< css::beans::NamedValue > SAL_CALL getTabProps( ::sal_Int32 ID ) override;

    virtual void SAL_CALL activateTab( ::sal_Int32 ID ) override;
    virtual ::sal_Int32 SAL_CALL getActiveTabID() override;

    virtual void SAL_CALL addTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) override;
    virtual void SAL_CALL removeTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) override;
    // XTabListener
    virtual void SAL_CALL inserted( ::sal_Int32 ID ) override;
    virtual void SAL_CALL removed( ::sal_Int32 ID ) override;
    virtual void SAL_CALL changed( ::sal_Int32 ID, const css::uno::Sequence< css::beans::NamedValue >& Properties ) override;
    virtual void SAL_CALL activated( ::sal_Int32 ID ) override;
    virtual void SAL_CALL deactivated( ::sal_Int32 ID ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& evt ) override;
    // XComponent
    void SAL_CALL dispose(  ) override;

protected:
    virtual void    impl_createControlPeerIfNecessary(
        const css::uno::Reference< css::awt::XControl >& _rxControl
    ) override;

};


class UnoPageModel : public ControlModelContainerBase
{
public:
    UnoPageModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~UnoPageModel() override;

    UnoPageModel(UnoPageModel const &) = default;
    UnoPageModel(UnoPageModel &&) = default;
    UnoPageModel & operator =(UnoPageModel const &) = delete; // due to ControlModelContainerBase
    UnoPageModel & operator =(UnoPageModel &&) = delete; // due to ControlModelContainerBase

    rtl::Reference<UnoControlModel> Clone() const override;

    DECLIMPL_SERVICEINFO_DERIVED( UnoPageModel, ControlModelContainerBase, "com.sun.star.awt.UnoPageModel" )

    virtual OUString SAL_CALL getServiceName() override;
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // Override the method of parent class
    virtual sal_Bool SAL_CALL getGroupControl(  ) override;
protected:
    virtual css::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper() override;

};

class UnoPageControl :  public ControlContainerBase
{
public:
    UnoPageControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~UnoPageControl() override;
    OUString     GetComponentServiceName() override;


    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoPageControl, ControlContainerBase, "com.sun.star.awt.UnoControlPage" )
};

class UnoFrameModel : public ControlModelContainerBase
{
public:
    UnoFrameModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~UnoFrameModel() override;

    UnoFrameModel(UnoFrameModel const &) = default;
    UnoFrameModel(UnoFrameModel &&) = default;
    UnoFrameModel & operator =(UnoFrameModel const &) = delete; // due to ControlModelContainerBase
    UnoFrameModel & operator =(UnoFrameModel &&) = delete; // due to ControlModelContainerBase

    rtl::Reference<UnoControlModel> Clone() const override;

    DECLIMPL_SERVICEINFO_DERIVED( UnoFrameModel, ControlModelContainerBase, "com.sun.star.awt.UnoFrameModel" )

    virtual OUString SAL_CALL getServiceName() override;
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

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
    virtual ~UnoFrameControl() override;
    OUString     GetComponentServiceName() override;

// css::lang::XServiceInfo
DECLIMPL_SERVICEINFO_DERIVED( UnoFrameControl, ControlContainerBase, "com.sun.star.awt.UnoControlFrame" )
};

#endif // INCLUDED_TOOLKIT_CONTROLS_DIALOGCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
