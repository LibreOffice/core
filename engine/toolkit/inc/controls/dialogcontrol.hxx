/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <controls/controlmodelcontainerbase.hxx>
#include <com/sun/star/awt/UnoControlDialog.hpp>
#include <com/sun/star/awt/XSimpleTabController.hpp>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/implbase2.hxx>

typedef ::cppu::AggImplInheritanceHelper2   <   ControlContainerBase
                                            ,   css::awt::XUnoControlDialog
                                            ,   css::awt::XWindowListener
                                            >   UnoDialogControl_Base;
class UnoDialogControl final : public UnoDialogControl_Base
{
private:
    css::uno::Reference< css::awt::XMenuBar >         mxMenuBar;
    TopWindowListenerMultiplexer                                                maTopWindowListeners;
    bool                                                                        mbWindowListener;

public:

                                UnoDialogControl( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                                virtual ~UnoDialogControl() override;
    OUString             GetComponentServiceName() const override;

    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void disposing( const css::lang::EventObject& Source ) override;
    void dispose() override;

    // css::awt::XTopWindow
    void addTopWindowListener( const css::uno::Reference< css::awt::XTopWindowListener >& xListener ) override;
    void removeTopWindowListener( const css::uno::Reference< css::awt::XTopWindowListener >& xListener ) override;
    void toFront(  ) override;
    void toBack(  ) override;
    void setMenuBar( const css::uno::Reference< css::awt::XMenuBar >& xMenu ) override;

    // css::awt::XWindowListener
    virtual void windowResized( const css::awt::WindowEvent& e ) override;
    virtual void windowMoved( const css::awt::WindowEvent& e ) override;
    virtual void windowShown( const css::lang::EventObject& e ) override;
    virtual void windowHidden( const css::lang::EventObject& e ) override;

    // css::awt::XDialog2
    virtual void endDialog( ::sal_Int32 Result ) override;
    virtual void setHelpId( const OUString& Id ) override;

    // css::awt::XDialog
    void setTitle( const OUString& Title ) override;
    OUString getTitle() override;
    sal_Int16 execute() override;
    void endExecute() override;

    // css::awt::XControl
    bool setModel( const css::uno::Reference< css::awt::XControlModel >& Model ) override;

    // XModifyListener
    virtual void modified( const css::lang::EventObject& aEvent ) override;

    // resolve some ambiguous methods
    virtual css::uno::Reference<css::awt::XWindowPeer> getPeer() override
        { return UnoDialogControl_Base::ControlContainerBase::getPeer(); }
    virtual void addWindowListener(const css::uno::Reference<css::awt::XWindowListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::addWindowListener(p1); }
    virtual css::uno::Reference<css::awt::XControlModel> getModel() override
        { return UnoDialogControl_Base::ControlContainerBase::getModel(); }
    virtual void addEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::addEventListener(p1); }
    virtual void removeEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removeEventListener(p1); }
    virtual void setContext(const css::uno::Reference<css::uno::XInterface>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::setContext(p1); }
    virtual css::uno::Reference<css::uno::XInterface> getContext() override
        { return UnoDialogControl_Base::ControlContainerBase::getContext(); }
    virtual css::uno::Reference<css::awt::XView> getView() override
        { return UnoDialogControl_Base::ControlContainerBase::getView(); }
    virtual void setDesignMode(bool p1) override
        { UnoDialogControl_Base::ControlContainerBase::setDesignMode(p1); }
    virtual bool isDesignMode() override
        { return UnoDialogControl_Base::ControlContainerBase::isDesignMode(); }
    virtual bool isTransparent() override
        { return UnoDialogControl_Base::ControlContainerBase::isTransparent(); }
    virtual void setPosSize(sal_Int32 p1, sal_Int32 p2, sal_Int32 p3, sal_Int32 p4, sal_Int16 p5) override
        { UnoDialogControl_Base::ControlContainerBase::setPosSize(p1, p2, p3, p4, p5); }
    virtual css::awt::Rectangle getPosSize() override
        { return UnoDialogControl_Base::ControlContainerBase::getPosSize(); }
    virtual void setVisible(bool p1) override
        { UnoDialogControl_Base::ControlContainerBase::setVisible(p1); }
    virtual void setEnable(bool p1) override
        { UnoDialogControl_Base::ControlContainerBase::setEnable(p1); }
    virtual void setFocus() override
        { UnoDialogControl_Base::ControlContainerBase::setFocus(); }
    virtual void removeWindowListener(const css::uno::Reference<css::awt::XWindowListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removeWindowListener(p1); }
    virtual void addFocusListener(const css::uno::Reference<css::awt::XFocusListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::addFocusListener(p1); }
    virtual void removeFocusListener(const css::uno::Reference<css::awt::XFocusListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removeFocusListener(p1); }
    virtual void addKeyListener(const css::uno::Reference<css::awt::XKeyListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::addKeyListener(p1); }
    virtual void removeKeyListener(const css::uno::Reference<css::awt::XKeyListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removeKeyListener(p1); }
    virtual void addMouseListener(const css::uno::Reference<css::awt::XMouseListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::addMouseListener(p1); }
    virtual void removeMouseListener(const css::uno::Reference<css::awt::XMouseListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removeMouseListener(p1); }
    virtual void addMouseMotionListener(const css::uno::Reference<css::awt::XMouseMotionListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::addMouseMotionListener(p1); }
    virtual void removeMouseMotionListener(const css::uno::Reference<css::awt::XMouseMotionListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removeMouseMotionListener(p1); }
    virtual void addPaintListener(const css::uno::Reference<css::awt::XPaintListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::addPaintListener(p1); }
    virtual void removePaintListener(const css::uno::Reference<css::awt::XPaintListener>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removePaintListener(p1); }
    virtual void setStatusText(const OUString& p1) override
        { UnoDialogControl_Base::ControlContainerBase::setStatusText(p1); }
    virtual cpo::uno::Sequence<css::uno::Reference<css::awt::XControl> > getControls() override
        { return UnoDialogControl_Base::ControlContainerBase::getControls(); }
    virtual css::uno::Reference<css::awt::XControl> getControl(const OUString& p1) override
        { return UnoDialogControl_Base::ControlContainerBase::getControl(p1); }
    virtual void addControl(const OUString& p1, const css::uno::Reference<css::awt::XControl>& p2) override
        { UnoDialogControl_Base::ControlContainerBase::addControl(p1, p2); }
    virtual void removeControl(const css::uno::Reference<css::awt::XControl>& p1) override
        { UnoDialogControl_Base::ControlContainerBase::removeControl(p1); }


    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    bool supportsService(OUString const & ServiceName) override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

private:
    virtual void PrepareWindowDescriptor( css::awt::WindowDescriptor& rDesc ) override;
    virtual void ImplModelPropertiesChanged( const cpo::uno::Sequence< css::beans::PropertyChangeEvent >& rEvents ) override;
};

class UnoMultiPageModel final : public ControlModelContainerBase
{
public:
    UnoMultiPageModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
    UnoMultiPageModel(const UnoMultiPageModel& rOther) : ControlModelContainerBase(rOther) {}
    virtual ~UnoMultiPageModel() override;

    rtl::Reference<UnoControlModel> Clone() const override;

    DECLIMPL_SERVICEINFO_DERIVED( UnoMultiPageModel, ControlModelContainerBase, u"com.sun.star.awt.UnoMultiPageModel"_ustr )

    virtual OUString getServiceName() override;
    virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;
    // XNamedContainer
    void insertByName( const OUString& aName, const cpo::uno::Any& aElement ) override;

    // Override the method of parent class
    virtual bool getGroupControl(  ) override;
private:
    virtual cpo::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

};

class UnoMultiPageControl final : public ControlContainerBase
                            ,public css::awt::XSimpleTabController
                            ,public css::awt::XTabListener
{
    TabListenerMultiplexer maTabListeners;
    void bindPage( const css::uno::Reference< css::awt::XControl >& _rxControl );
public:
    UnoMultiPageControl( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
    virtual ~UnoMultiPageControl() override;
    OUString     GetComponentServiceName() const override;

    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoMultiPageControl, ControlContainerBase, u"com.sun.star.awt.UnoControlMultiPage"_ustr )
    cpo::uno::Any  queryInterface( const cpo::uno::Type & rType ) override { return ControlContainerBase::queryInterface(rType); }
    cpo::uno::Any  queryAggregation( const cpo::uno::Type & rType ) override;
    void                        acquire() noexcept override  { OWeakAggObject::acquire(); }
    void                        release() noexcept override  { OWeakAggObject::release(); }
    // css::lang::XTypeProvider
    cpo::uno::Sequence< cpo::uno::Type >  getTypes() override;
    cpo::uno::Sequence< sal_Int8 >                     getImplementationId() override;
    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    // css::awt::XSimpleTabController
    virtual ::sal_Int32 insertTab() override;
    virtual void removeTab( ::sal_Int32 ID ) override;

    virtual void setTabProps( ::sal_Int32 ID, const cpo::uno::Sequence< css::beans::NamedValue >& Properties ) override;
    virtual cpo::uno::Sequence< css::beans::NamedValue > getTabProps( ::sal_Int32 ID ) override;

    virtual void activateTab( ::sal_Int32 ID ) override;
    virtual ::sal_Int32 getActiveTabID() override;

    virtual void addTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) override;
    virtual void removeTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) override;
    // XTabListener
    virtual void inserted( ::sal_Int32 ID ) override;
    virtual void removed( ::sal_Int32 ID ) override;
    virtual void changed( ::sal_Int32 ID, const cpo::uno::Sequence< css::beans::NamedValue >& Properties ) override;
    virtual void activated( ::sal_Int32 ID ) override;
    virtual void deactivated( ::sal_Int32 ID ) override;
    virtual void disposing( const css::lang::EventObject& evt ) override;
    // XComponent
    void dispose(  ) override;

private:
    virtual void    impl_createControlPeerIfNecessary(
        const css::uno::Reference< css::awt::XControl >& _rxControl
    ) override;

};


class UnoPageModel final : public ControlModelContainerBase
{
public:
    UnoPageModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
    UnoPageModel(const UnoPageModel& rOther) : ControlModelContainerBase(rOther) {}
    virtual ~UnoPageModel() override;

    rtl::Reference<UnoControlModel> Clone() const override;

    DECLIMPL_SERVICEINFO_DERIVED( UnoPageModel, ControlModelContainerBase, u"com.sun.star.awt.UnoPageModel"_ustr )

    virtual OUString getServiceName() override;
    virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // Override the method of parent class
    virtual bool getGroupControl(  ) override;
private:
    virtual cpo::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

};

class UnoPageControl final : public ControlContainerBase
{
public:
    UnoPageControl( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
    virtual ~UnoPageControl() override;
    OUString     GetComponentServiceName() const override;


    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoPageControl, ControlContainerBase, u"com.sun.star.awt.UnoControlPage"_ustr )
};

class UnoFrameModel final : public ControlModelContainerBase
{
public:
    UnoFrameModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
    UnoFrameModel(const UnoFrameModel& rOther) : ControlModelContainerBase(rOther) {}
    virtual ~UnoFrameModel() override;

    rtl::Reference<UnoControlModel> Clone() const override;

    DECLIMPL_SERVICEINFO_DERIVED( UnoFrameModel, ControlModelContainerBase, u"com.sun.star.awt.UnoFrameModel"_ustr )

    virtual OUString getServiceName() override;
    virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

private:
    virtual cpo::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;
};

class UnoFrameControl final : public ControlContainerBase
{
    virtual void        ImplSetPosSize( css::uno::Reference< css::awt::XControl >& rxCtrl ) override;
public:
    UnoFrameControl( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
    virtual ~UnoFrameControl() override;
    OUString     GetComponentServiceName() const override;

// css::lang::XServiceInfo
DECLIMPL_SERVICEINFO_DERIVED( UnoFrameControl, ControlContainerBase, u"com.sun.star.awt.UnoControlFrame"_ustr )
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
