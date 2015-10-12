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
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <list>

typedef ::cppu::AggImplInheritanceHelper2   <   ControlContainerBase
                                            ,   ::com::sun::star::awt::XUnoControlDialog
                                            ,   ::com::sun::star::awt::XWindowListener
                                            >   UnoDialogControl_Base;
class UnoDialogControl : public UnoDialogControl_Base
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar >         mxMenuBar;
    TopWindowListenerMultiplexer                                                maTopWindowListeners;
    bool                                                                        mbWindowListener;

public:

                                UnoDialogControl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                                virtual ~UnoDialogControl();
    OUString             GetComponentServiceName() override;

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XTopWindow
    void SAL_CALL addTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL toFront(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL toBack(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMenuBar( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar >& xMenu ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XWindowListener
    virtual void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XDialog2
    virtual void SAL_CALL endDialog( ::sal_Int32 Result ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHelpId( const OUString& Id ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XDialog
    void SAL_CALL setTitle( const OUString& Title ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getTitle() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL execute() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL endExecute() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // resolve some ambigous methods
    virtual com::sun::star::uno::Reference<com::sun::star::awt::XWindowPeer> SAL_CALL getPeer() throw (com::sun::star::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::getPeer(); }
    virtual void SAL_CALL addWindowListener(const com::sun::star::uno::Reference<com::sun::star::awt::XWindowListener>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addWindowListener(p1); }
    virtual com::sun::star::uno::Reference<com::sun::star::awt::XControlModel> SAL_CALL getModel() throw (com::sun::star::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::getModel(); }
    virtual void SAL_CALL addEventListener(const com::sun::star::uno::Reference<com::sun::star::lang::XEventListener>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addEventListener(p1); }
    virtual void SAL_CALL removeEventListener(const com::sun::star::uno::Reference<com::sun::star::lang::XEventListener>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removeEventListener(p1); }
    virtual void SAL_CALL setContext(const com::sun::star::uno::Reference<com::sun::star::uno::XInterface>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::setContext(p1); }
    virtual com::sun::star::uno::Reference<com::sun::star::uno::XInterface> SAL_CALL getContext() throw (com::sun::star::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::getContext(); }
    virtual com::sun::star::uno::Reference<com::sun::star::awt::XView> SAL_CALL getView() throw (com::sun::star::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::getView(); }
    virtual void SAL_CALL setDesignMode(sal_Bool p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::setDesignMode(p1); }
    virtual sal_Bool SAL_CALL isDesignMode() throw (com::sun::star::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::isDesignMode(); }
    virtual sal_Bool SAL_CALL isTransparent() throw (com::sun::star::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::isTransparent(); }
    virtual void SAL_CALL setPosSize(sal_Int32 p1, sal_Int32 p2, sal_Int32 p3, sal_Int32 p4, sal_Int16 p5) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::setPosSize(p1, p2, p3, p4, p5); }
    virtual com::sun::star::awt::Rectangle SAL_CALL getPosSize() throw (com::sun::star::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::getPosSize(); }
    virtual void SAL_CALL setVisible(sal_Bool p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::setVisible(p1); }
    virtual void SAL_CALL setEnable(sal_Bool p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::setEnable(p1); }
    virtual void SAL_CALL setFocus() throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::setFocus(); }
    virtual void SAL_CALL removeWindowListener(const com::sun::star::uno::Reference<com::sun::star::awt::XWindowListener>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removeWindowListener(p1); }
    virtual void SAL_CALL addFocusListener(const com::sun::star::uno::Reference<com::sun::star::awt::XFocusListener>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addFocusListener(p1); }
    virtual void SAL_CALL removeFocusListener(const com::sun::star::uno::Reference<com::sun::star::awt::XFocusListener>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removeFocusListener(p1); }
    virtual void SAL_CALL addKeyListener(const com::sun::star::uno::Reference<com::sun::star::awt::XKeyListener>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addKeyListener(p1); }
    virtual void SAL_CALL removeKeyListener(const com::sun::star::uno::Reference<com::sun::star::awt::XKeyListener>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removeKeyListener(p1); }
    virtual void SAL_CALL addMouseListener(const com::sun::star::uno::Reference<com::sun::star::awt::XMouseListener>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addMouseListener(p1); }
    virtual void SAL_CALL removeMouseListener(const com::sun::star::uno::Reference<com::sun::star::awt::XMouseListener>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removeMouseListener(p1); }
    virtual void SAL_CALL addMouseMotionListener(const com::sun::star::uno::Reference<com::sun::star::awt::XMouseMotionListener>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addMouseMotionListener(p1); }
    virtual void SAL_CALL removeMouseMotionListener(const com::sun::star::uno::Reference<com::sun::star::awt::XMouseMotionListener>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removeMouseMotionListener(p1); }
    virtual void SAL_CALL addPaintListener(const com::sun::star::uno::Reference<com::sun::star::awt::XPaintListener>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addPaintListener(p1); }
    virtual void SAL_CALL removePaintListener(const com::sun::star::uno::Reference<com::sun::star::awt::XPaintListener>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removePaintListener(p1); }
    virtual void SAL_CALL setStatusText(const rtl::OUString& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::setStatusText(p1); }
    virtual com::sun::star::uno::Sequence<com::sun::star::uno::Reference<com::sun::star::awt::XControl> > SAL_CALL getControls() throw (com::sun::star::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::getControls(); }
    virtual com::sun::star::uno::Reference<com::sun::star::awt::XControl> SAL_CALL getControl(const rtl::OUString& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { return UnoDialogControl_Base::ControlContainerBase::getControl(p1); }
    virtual void SAL_CALL addControl(const rtl::OUString& p1, const com::sun::star::uno::Reference<com::sun::star::awt::XControl>& p2) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::addControl(p1, p2); }
    virtual void SAL_CALL removeControl(const com::sun::star::uno::Reference<com::sun::star::awt::XControl>& p1) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { UnoDialogControl_Base::ControlContainerBase::removeControl(p1); }


    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    virtual void PrepareWindowDescriptor( ::com::sun::star::awt::WindowDescriptor& rDesc ) override;
    virtual void ImplModelPropertiesChanged( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& rEvents ) throw(::com::sun::star::uno::RuntimeException) override;
protected:
};

class UnoMultiPageModel : public ControlModelContainerBase
{
public:
    UnoMultiPageModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~UnoMultiPageModel();
    UnoMultiPageModel( const UnoMultiPageModel& rModel );

    UnoControlModel*    Clone() const override;

    DECLIMPL_SERVICEINFO_DERIVED( UnoMultiPageModel, ControlModelContainerBase, "com.sun.star.awt.UnoMultiPageModel" )

    virtual OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw( ::com::sun::star::uno::RuntimeException, std::exception) override;
    // XNamedContainer
    void SAL_CALL insertByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // Override the method of parent Class
    virtual sal_Bool SAL_CALL getGroupControl(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
protected:
    virtual ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper() override;

};

class UnoMultiPageControl :  public ControlContainerBase
                            ,public ::com::sun::star::awt::XSimpleTabController
                            ,public ::com::sun::star::awt::XTabListener
{
    TabListenerMultiplexer maTabListeners;
    void bindPage( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl );
public:
    UnoMultiPageControl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~UnoMultiPageControl();
    OUString     GetComponentServiceName() override;

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoMultiPageControl, ControlContainerBase, "com.sun.star.awt.UnoControlMultiPage" )
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) override { return ControlContainerBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }
    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    // com::sun::star::awt::XSimpleTabController
    virtual ::sal_Int32 SAL_CALL insertTab() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTab( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setTabProps( ::sal_Int32 ID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Properties ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > SAL_CALL getTabProps( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL activateTab( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getActiveTabID() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    // XTabListener
    virtual void SAL_CALL inserted( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removed( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL changed( ::sal_Int32 ID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Properties ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL activated( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL deactivated( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& evt ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    // XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    virtual void    impl_createControlPeerIfNecessary(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl
    ) override;

};


class UnoPageModel : public ControlModelContainerBase
{
public:
    UnoPageModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~UnoPageModel();
    UnoPageModel( const UnoPageModel& rModel );

    UnoControlModel*    Clone() const override;

    DECLIMPL_SERVICEINFO_DERIVED( UnoPageModel, ControlModelContainerBase, "com.sun.star.awt.UnoPageModel" )

    virtual OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw( ::com::sun::star::uno::RuntimeException, std::exception) override;

    // Override the method of parent Class
    virtual sal_Bool SAL_CALL getGroupControl(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
protected:
    virtual ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper() override;

};

class UnoPageControl :  public ControlContainerBase
{
public:
    UnoPageControl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~UnoPageControl();
    OUString     GetComponentServiceName() override;


    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoPageControl, ControlContainerBase, "com.sun.star.awt.UnoControlPage" )
};

class UnoFrameModel : public ControlModelContainerBase
{
public:
    UnoFrameModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~UnoFrameModel();
    UnoFrameModel( const UnoFrameModel& rModel );

    UnoControlModel*    Clone() const override;

    DECLIMPL_SERVICEINFO_DERIVED( UnoFrameModel, ControlModelContainerBase, "com.sun.star.awt.UnoFrameModel" )

    virtual OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw( ::com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    virtual ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper() override;
};

class UnoFrameControl :  public ControlContainerBase
{
protected:
    virtual void        ImplSetPosSize( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& rxCtrl ) override;
public:
    UnoFrameControl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~UnoFrameControl();
    OUString     GetComponentServiceName() override;

// ::com::sun::star::lang::XServiceInfo
DECLIMPL_SERVICEINFO_DERIVED( UnoFrameControl, ControlContainerBase, "com.sun.star.awt.UnoControlFrame" )
};

#endif // INCLUDED_TOOLKIT_CONTROLS_DIALOGCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
