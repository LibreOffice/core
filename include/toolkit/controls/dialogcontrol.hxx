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

#ifndef TOOLKIT_DIALOG_CONTROL_HXX
#define TOOLKIT_DIALOG_CONTROL_HXX

#include <toolkit/controls/controlmodelcontainerbase.hxx>
#include <com/sun/star/awt/XUnoControlDialog.hpp>
#include <com/sun/star/awt/XUnoControlDialogModel.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XDialog2.hpp>
#include <com/sun/star/awt/XSimpleTabController.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <com/sun/star/graphic/XGraphicObject.hpp>
#include "toolkit/helper/servicenames.hxx"
#include "toolkit/helper/macros.hxx"
#include <toolkit/controls/unocontrolcontainer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <list>

//  ----------------------------------------------------
//  class UnoControlDialogModel
//  ----------------------------------------------------

typedef ::cppu::AggImplInheritanceHelper1   <   ControlModelContainerBase
                                            ,   ::com::sun::star::awt::XUnoControlDialogModel
                                            >   UnoControlDialogModel_Base;
class UnoControlDialogModel :   public UnoControlDialogModel_Base
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphicObject > mxGrfObj;
    ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper();
    // ::cppu::OPropertySetHelper
	void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception);

    OUString getPropertyString(const OUString& p1) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool getPropertyBool(const OUString& p1) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 getPropertyInt16(const OUString& p1) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 getPropertyInt32(const OUString& p1) throw(::com::sun::star::uno::RuntimeException);
    void setPropertyString(const OUString& p1, const OUString& p2) throw(::com::sun::star::uno::RuntimeException)
        { setPropertyValue( p1, css::uno::Any(p2) ); }
    void setPropertyBool(const OUString& p1, sal_Bool p2) throw(::com::sun::star::uno::RuntimeException)
        { setPropertyValue( p1, css::uno::Any(p2) ); }
    void setPropertyInt16(const OUString& p1, sal_Int16 p2) throw(::com::sun::star::uno::RuntimeException)
        { setPropertyValue( p1, css::uno::Any(p2) ); }
    void setPropertyInt32(const OUString& p1, sal_Int32 p2) throw(::com::sun::star::uno::RuntimeException)
        { setPropertyValue( p1, css::uno::Any(p2) ); }
public:
                        UnoControlDialogModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                        UnoControlDialogModel( const UnoControlDialogModel& rModel );
                        ~UnoControlDialogModel();

    UnoControlModel*    Clone() const;
    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlDialogModel, ControlModelContainerBase, szServiceName2_UnoControlDialogModel )


    // XUnoControlDialogModel attributes
    virtual rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyString("Name"); }
    virtual void SAL_CALL setName(const rtl::OUString& p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyString("Name", p1); }
    virtual sal_Bool SAL_CALL getDecoration() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyBool("Decoration"); }
    virtual void SAL_CALL setDecoration(sal_Bool p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyBool("Decoration", p1); }
    virtual sal_Int32 SAL_CALL getPositionX() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyInt32("PositionX"); }
    virtual void SAL_CALL setPositionX(sal_Int32 p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyInt32("PositionX", p1); }
    virtual sal_Int32 SAL_CALL getPositionY() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyInt32("PositionY"); }
    virtual void SAL_CALL setPositionY(sal_Int32 p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyInt32("PositionY", p1); }
    virtual sal_Int32 SAL_CALL getWidth() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyInt32("Width"); }
    virtual void SAL_CALL setWidth(sal_Int32 p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyInt32("Width", p1); }
    virtual sal_Int32 SAL_CALL getHeight() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyInt32("Height"); }
    virtual void SAL_CALL setHeight(sal_Int32 p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyInt32("Height", p1); }
    virtual rtl::OUString SAL_CALL getDialogSourceURL() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyString("DialogSourceURL"); }
    virtual void SAL_CALL setDialogSourceURL(const rtl::OUString& p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyString("DialogSourceURL", p1); }
    virtual rtl::OUString SAL_CALL getTitle() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyString("Title"); }
    virtual void SAL_CALL setTitle(const rtl::OUString& p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyString("Title", p1); }
    virtual sal_Bool SAL_CALL getCloseable() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyBool("Closeable"); }
    virtual void SAL_CALL setCloseable(sal_Bool p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyBool("Closeable", p1); }
    virtual sal_Bool SAL_CALL getEnabled() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyBool("Enabled"); }
    virtual void SAL_CALL setEnabled(sal_Bool p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyBool("Enabled", p1); }
    virtual sal_Bool SAL_CALL getMoveable() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyBool("Moveable"); }
    virtual void SAL_CALL setMoveable(sal_Bool p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyBool("Moveable", p1); }
    virtual sal_Bool SAL_CALL getSizeable() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyBool("Sizeable"); }
    virtual void SAL_CALL setSizeable(sal_Bool p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyBool("Sizeable", p1); }
    virtual sal_Bool SAL_CALL getDesktopAsParent() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyBool("DesktopAsParent"); }
    virtual void SAL_CALL setDesktopAsParent(sal_Bool p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyBool("DesktopAsParent", p1); }
    virtual rtl::OUString SAL_CALL getHelpURL() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyString("HelpURL"); }
    virtual void SAL_CALL setHelpURL(const rtl::OUString& p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyString("HelpURL", p1); }
    virtual sal_Int32 SAL_CALL getBackgroundColor() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyInt32("BackgroundColor"); }
    virtual void SAL_CALL setBackgroundColor(sal_Int32 p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyInt32("BackgroundColor", p1); }
    virtual sal_Int16 SAL_CALL getFontEmphasisMark() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyInt16("FontEmphasisMark"); }
    virtual void SAL_CALL setFontEmphasisMark(sal_Int16 p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyInt16("FontEmphasisMark", p1); }
    virtual sal_Int16 SAL_CALL getFontRelief() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyInt16("FontRelief"); }
    virtual void SAL_CALL setFontRelief(sal_Int16 p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyInt16("FontRelief", p1); }
    virtual rtl::OUString SAL_CALL getHelpText() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyString("HelpText"); }
    virtual void SAL_CALL setHelpText(const rtl::OUString& p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyString("HelpText", p1); }
    virtual sal_Int32 SAL_CALL getTextColor() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyInt32("TextColor"); }
    virtual void SAL_CALL setTextColor(sal_Int32 p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyInt32("TextColor", p1); }
    virtual sal_Int32 SAL_CALL getTextLineColor() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyInt32("TextLineColor"); }
    virtual void SAL_CALL setTextLineColor(sal_Int32 p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyInt32("TextLineColor", p1); }
    virtual rtl::OUString SAL_CALL getImageURL() throw(::com::sun::star::uno::RuntimeException)
       { return getPropertyString("ImageURL"); }
    virtual void SAL_CALL setImageURL(const rtl::OUString& p1) throw(::com::sun::star::uno::RuntimeException)
       { setPropertyString("ImageURL", p1); }
    virtual com::sun::star::awt::FontDescriptor SAL_CALL getFontDescriptor() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFontDescriptor(const com::sun::star::awt::FontDescriptor& p1) throw(::com::sun::star::uno::RuntimeException)
        { setPropertyValue( "FontDescriptor", css::uno::Any(p1) ); }
    virtual com::sun::star::uno::Reference<com::sun::star::graphic::XGraphic> SAL_CALL getGraphic() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setGraphic(const com::sun::star::uno::Reference<com::sun::star::graphic::XGraphic>& p1) throw(::com::sun::star::uno::RuntimeException)
        { setPropertyValue( "Graphic", css::uno::Any(p1) ); }


    // overrides to resolve ambiguity
    virtual com::sun::star::uno::Any SAL_CALL getPropertyValue(const rtl::OUString& p1) throw (com::sun::star::beans::UnknownPropertyException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException)
        { return UnoControlDialogModel_Base::ControlModelContainerBase::getPropertyValue(p1); }
    virtual void SAL_CALL setPropertyValue(const OUString& p1, const com::sun::star::uno::Any& p2) throw (com::sun::star::beans::UnknownPropertyException, com::sun::star::beans::PropertyVetoException, com::sun::star::lang::IllegalArgumentException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException)
        { return UnoControlDialogModel_Base::ControlModelContainerBase::setPropertyValue(p1, p2); }
    virtual void SAL_CALL addPropertyChangeListener(const OUString& p1, const com::sun::star::uno::Reference<com::sun::star::beans::XPropertyChangeListener>& p2) throw (com::sun::star::beans::UnknownPropertyException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException)
        { return UnoControlDialogModel_Base::ControlModelContainerBase::addPropertyChangeListener(p1, p2); }
    virtual void SAL_CALL removePropertyChangeListener(const OUString& p1, const com::sun::star::uno::Reference<com::sun::star::beans::XPropertyChangeListener>& p2) throw (com::sun::star::beans::UnknownPropertyException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException)
        { return UnoControlDialogModel_Base::ControlModelContainerBase::removePropertyChangeListener(p1, p2); }
    virtual void SAL_CALL addVetoableChangeListener(const OUString& p1, const com::sun::star::uno::Reference<com::sun::star::beans::XVetoableChangeListener>& p2) throw (com::sun::star::beans::UnknownPropertyException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException)
        { return UnoControlDialogModel_Base::ControlModelContainerBase::addVetoableChangeListener(p1, p2); }
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& p1, const com::sun::star::uno::Reference<com::sun::star::beans::XVetoableChangeListener>& p2) throw (com::sun::star::beans::UnknownPropertyException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException)
        { return UnoControlDialogModel_Base::ControlModelContainerBase::removeVetoableChangeListener(p1, p2); }
    virtual com::sun::star::uno::Reference<com::sun::star::uno::XInterface> SAL_CALL createInstance(const OUString& p1) throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
        { return UnoControlDialogModel_Base::ControlModelContainerBase::createInstance(p1); }
    virtual com::sun::star::uno::Reference<com::sun::star::uno::XInterface> SAL_CALL createInstanceWithArguments(const OUString& p1, const com::sun::star::uno::Sequence<com::sun::star::uno::Any>& p2) throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
        { return UnoControlDialogModel_Base::ControlModelContainerBase::createInstanceWithArguments(p1, p2); }
    virtual com::sun::star::uno::Sequence<rtl::OUString> SAL_CALL getAvailableServiceNames() throw (::com::sun::star::uno::RuntimeException)
        { return UnoControlDialogModel_Base::ControlModelContainerBase::getAvailableServiceNames(); }


    virtual com::sun::star::uno::Type SAL_CALL getElementType() throw (::com::sun::star::uno::RuntimeException)
        { return UnoControlDialogModel_Base::ControlModelContainerBase::getElementType(); }
    virtual sal_Bool SAL_CALL hasElements() throw (::com::sun::star::uno::RuntimeException)
        { return UnoControlDialogModel_Base::ControlModelContainerBase::hasElements(); }
    virtual com::sun::star::uno::Any SAL_CALL getByName(const rtl::OUString& p1) throw (com::sun::star::container::NoSuchElementException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException)
        { return UnoControlDialogModel_Base::ControlModelContainerBase::getByName(p1); }
    virtual com::sun::star::uno::Sequence<rtl::OUString> SAL_CALL getElementNames() throw (::com::sun::star::uno::RuntimeException)
        { return UnoControlDialogModel_Base::ControlModelContainerBase::getElementNames(); }
    virtual sal_Bool SAL_CALL hasByName(const rtl::OUString& p1) throw (::com::sun::star::uno::RuntimeException)
        { return UnoControlDialogModel_Base::ControlModelContainerBase::hasByName(p1); }
    virtual void SAL_CALL replaceByName(const rtl::OUString& p1, const com::sun::star::uno::Any& p2) throw (com::sun::star::lang::IllegalArgumentException, com::sun::star::container::NoSuchElementException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException)
        { UnoControlDialogModel_Base::ControlModelContainerBase::replaceByName(p1, p2); }
    virtual void SAL_CALL insertByName(const rtl::OUString& p1, const com::sun::star::uno::Any& p2) throw (com::sun::star::lang::IllegalArgumentException, com::sun::star::container::ElementExistException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException)
        { UnoControlDialogModel_Base::ControlModelContainerBase::insertByName(p1, p2); }
    virtual void SAL_CALL removeByName(const rtl::OUString& p1) throw (com::sun::star::container::NoSuchElementException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException)
        { UnoControlDialogModel_Base::ControlModelContainerBase::removeByName(p1); }



};

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
                                ~UnoDialogControl();
    OUString             GetComponentServiceName();

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTopWindow
    void SAL_CALL addTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL toFront(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL toBack(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMenuBar( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar >& xMenu ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XWindowListener
    virtual void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XDialog2
    virtual void SAL_CALL endDialog( ::sal_Int32 Result ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setHelpId( const OUString& Id ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XDialog
    void SAL_CALL setTitle( const OUString& Title ) throw(::com::sun::star::uno::RuntimeException);
    OUString SAL_CALL getTitle() throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL execute() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL endExecute() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model ) throw(::com::sun::star::uno::RuntimeException);

    // XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    // resolve some ambigous methods
    virtual com::sun::star::uno::Reference<com::sun::star::awt::XWindowPeer> SAL_CALL getPeer() throw (com::sun::star::uno::RuntimeException)
        { return UnoDialogControl_Base::ControlContainerBase::getPeer(); }
    virtual void SAL_CALL addWindowListener(const com::sun::star::uno::Reference<com::sun::star::awt::XWindowListener>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::addWindowListener(p1); }
    virtual com::sun::star::uno::Reference<com::sun::star::awt::XControlModel> SAL_CALL getModel() throw (com::sun::star::uno::RuntimeException)
        { return UnoDialogControl_Base::ControlContainerBase::getModel(); }
    virtual void SAL_CALL addEventListener(const com::sun::star::uno::Reference<com::sun::star::lang::XEventListener>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::addEventListener(p1); }
    virtual void SAL_CALL removeEventListener(const com::sun::star::uno::Reference<com::sun::star::lang::XEventListener>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::removeEventListener(p1); }
    virtual void SAL_CALL setContext(const com::sun::star::uno::Reference<com::sun::star::uno::XInterface>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::setContext(p1); }
    virtual com::sun::star::uno::Reference<com::sun::star::uno::XInterface> SAL_CALL getContext() throw (com::sun::star::uno::RuntimeException)
        { return UnoDialogControl_Base::ControlContainerBase::getContext(); }
    virtual com::sun::star::uno::Reference<com::sun::star::awt::XView> SAL_CALL getView() throw (com::sun::star::uno::RuntimeException)
        { return UnoDialogControl_Base::ControlContainerBase::getView(); }
    virtual void SAL_CALL setDesignMode(sal_Bool p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::setDesignMode(p1); }
    virtual sal_Bool SAL_CALL isDesignMode() throw (com::sun::star::uno::RuntimeException)
        { return UnoDialogControl_Base::ControlContainerBase::isDesignMode(); }
    virtual sal_Bool SAL_CALL isTransparent() throw (com::sun::star::uno::RuntimeException)
        { return UnoDialogControl_Base::ControlContainerBase::isTransparent(); }
    virtual void SAL_CALL setPosSize(sal_Int32 p1, sal_Int32 p2, sal_Int32 p3, sal_Int32 p4, sal_Int16 p5) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::setPosSize(p1, p2, p3, p4, p5); }
    virtual com::sun::star::awt::Rectangle SAL_CALL getPosSize() throw (com::sun::star::uno::RuntimeException)
        { return UnoDialogControl_Base::ControlContainerBase::getPosSize(); }
    virtual void SAL_CALL setVisible(sal_Bool p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::setVisible(p1); }
    virtual void SAL_CALL setEnable(sal_Bool p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::setEnable(p1); }
    virtual void SAL_CALL setFocus() throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::setFocus(); }
    virtual void SAL_CALL removeWindowListener(const com::sun::star::uno::Reference<com::sun::star::awt::XWindowListener>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::removeWindowListener(p1); }
    virtual void SAL_CALL addFocusListener(const com::sun::star::uno::Reference<com::sun::star::awt::XFocusListener>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::addFocusListener(p1); }
    virtual void SAL_CALL removeFocusListener(const com::sun::star::uno::Reference<com::sun::star::awt::XFocusListener>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::removeFocusListener(p1); }
    virtual void SAL_CALL addKeyListener(const com::sun::star::uno::Reference<com::sun::star::awt::XKeyListener>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::addKeyListener(p1); }
    virtual void SAL_CALL removeKeyListener(const com::sun::star::uno::Reference<com::sun::star::awt::XKeyListener>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::removeKeyListener(p1); }
    virtual void SAL_CALL addMouseListener(const com::sun::star::uno::Reference<com::sun::star::awt::XMouseListener>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::addMouseListener(p1); }
    virtual void SAL_CALL removeMouseListener(const com::sun::star::uno::Reference<com::sun::star::awt::XMouseListener>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::removeMouseListener(p1); }
    virtual void SAL_CALL addMouseMotionListener(const com::sun::star::uno::Reference<com::sun::star::awt::XMouseMotionListener>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::addMouseMotionListener(p1); }
    virtual void SAL_CALL removeMouseMotionListener(const com::sun::star::uno::Reference<com::sun::star::awt::XMouseMotionListener>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::removeMouseMotionListener(p1); }
    virtual void SAL_CALL addPaintListener(const com::sun::star::uno::Reference<com::sun::star::awt::XPaintListener>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::addPaintListener(p1); }
    virtual void SAL_CALL removePaintListener(const com::sun::star::uno::Reference<com::sun::star::awt::XPaintListener>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::removePaintListener(p1); }
    virtual void SAL_CALL setStatusText(const rtl::OUString& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::setStatusText(p1); }
    virtual com::sun::star::uno::Sequence<com::sun::star::uno::Reference<com::sun::star::awt::XControl> > SAL_CALL getControls() throw (com::sun::star::uno::RuntimeException)
        { return UnoDialogControl_Base::ControlContainerBase::getControls(); }
    virtual com::sun::star::uno::Reference<com::sun::star::awt::XControl> SAL_CALL getControl(const rtl::OUString& p1) throw (com::sun::star::uno::RuntimeException)
        { return UnoDialogControl_Base::ControlContainerBase::getControl(p1); }
    virtual void SAL_CALL addControl(const rtl::OUString& p1, const com::sun::star::uno::Reference<com::sun::star::awt::XControl>& p2) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::addControl(p1, p2); }
    virtual void SAL_CALL removeControl(const com::sun::star::uno::Reference<com::sun::star::awt::XControl>& p1) throw (com::sun::star::uno::RuntimeException)
        { UnoDialogControl_Base::ControlContainerBase::removeControl(p1); }


    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoDialogControl, szServiceName2_UnoControlDialog )

protected:
    virtual void PrepareWindowDescriptor( ::com::sun::star::awt::WindowDescriptor& rDesc );
    virtual void ImplModelPropertiesChanged( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& rEvents ) throw(::com::sun::star::uno::RuntimeException);
protected:
};

class UnoMultiPageModel : public ControlModelContainerBase
{
public:
    UnoMultiPageModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    ~UnoMultiPageModel();
    UnoMultiPageModel( const UnoMultiPageModel& rModel );

    UnoControlModel*    Clone() const;

    DECLIMPL_SERVICEINFO_DERIVED( UnoMultiPageModel, ControlModelContainerBase, szServiceName_UnoMultiPageModel )

    virtual OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw( ::com::sun::star::uno::RuntimeException);
    // XNamedContainer
    void SAL_CALL insertByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // Override the method of parent Class
    virtual sal_Bool SAL_CALL getGroupControl(  ) throw (::com::sun::star::uno::RuntimeException);
protected:
    virtual ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper();

};

class UnoMultiPageControl :  public ControlContainerBase
                            ,public ::com::sun::star::awt::XSimpleTabController
                            ,public ::com::sun::star::awt::XTabListener
{
    TabListenerMultiplexer maTabListeners;
    void bindPage( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl );
public:
    UnoMultiPageControl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    ~UnoMultiPageControl();
    OUString     GetComponentServiceName();

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoMultiPageControl, ControlContainerBase, szServiceName_UnoMultiPageControl )
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return ControlContainerBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }
    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    // com::sun::star::awt::XSimpleTabController
    virtual ::sal_Int32 SAL_CALL insertTab() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTab( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setTabProps( ::sal_Int32 ID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Properties ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > SAL_CALL getTabProps( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL activateTab( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getActiveTabID() throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    // XTabListener
    virtual void SAL_CALL inserted( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removed( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL changed( ::sal_Int32 ID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Properties ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL activated( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deactivated( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& evt ) throw (::com::sun::star::uno::RuntimeException);
    // XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

protected:
    virtual void    impl_createControlPeerIfNecessary(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl
    );

};


class UnoPageModel : public ControlModelContainerBase
{
public:
    UnoPageModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    ~UnoPageModel();
    UnoPageModel( const UnoPageModel& rModel );

    UnoControlModel*    Clone() const;

    DECLIMPL_SERVICEINFO_DERIVED( UnoPageModel, ControlModelContainerBase, szServiceName_UnoPageModel )

    virtual OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw( ::com::sun::star::uno::RuntimeException);

    // Override the method of parent Class
    virtual sal_Bool SAL_CALL getGroupControl(  ) throw (::com::sun::star::uno::RuntimeException);
protected:
    virtual ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper();

};

class UnoPageControl :  public ControlContainerBase
{
public:
    UnoPageControl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    ~UnoPageControl();
    OUString     GetComponentServiceName();


    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoPageControl, ControlContainerBase, szServiceName_UnoPageControl )
};

class UnoFrameModel : public ControlModelContainerBase
{
public:
    UnoFrameModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    ~UnoFrameModel();
    UnoFrameModel( const UnoFrameModel& rModel );

    UnoControlModel*    Clone() const;

    DECLIMPL_SERVICEINFO_DERIVED( UnoFrameModel, ControlModelContainerBase, szServiceName_UnoFrameModel )

    virtual OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw( ::com::sun::star::uno::RuntimeException);

protected:
    virtual ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper();
};

class UnoFrameControl :  public ControlContainerBase
{
protected:
    virtual void        ImplSetPosSize( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& rxCtrl );
public:
    UnoFrameControl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    ~UnoFrameControl();
    OUString     GetComponentServiceName();

// ::com::sun::star::lang::XServiceInfo
DECLIMPL_SERVICEINFO_DERIVED( UnoFrameControl, ControlContainerBase, szServiceName_UnoPageControl )
};

#endif // TOOLKIT_DIALOG_CONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
