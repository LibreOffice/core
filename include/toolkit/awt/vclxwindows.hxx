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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXWINDOWS_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXWINDOWS_HXX

#include <toolkit/dllapi.h>

#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyStateChangeEvent.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/awt/XFileDialog.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XProgressMonitor.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/XScrollBar.hpp>
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XMessageBox.hpp>
#include <com/sun/star/awt/XTextEditField.hpp>
#include <com/sun/star/awt/Style.hpp>
#include <com/sun/star/awt/XTimeField.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XSpinField.hpp>
#include <com/sun/star/awt/XUnoControlContainer.hpp>
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#include <com/sun/star/awt/XNumericField.hpp>
#include <com/sun/star/awt/XMetricField.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XToggleButton.hpp>
#include <com/sun/star/awt/XPointer.hpp>
#include <com/sun/star/awt/XTextArea.hpp>
#include <com/sun/star/awt/XImageButton.hpp>
#include <com/sun/star/awt/XFixedHyperlink.hpp>
#include <com/sun/star/awt/XFixedText.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XDialog2.hpp>
#include <com/sun/star/awt/XRadioButton.hpp>
#include <com/sun/star/awt/XCurrencyField.hpp>
#include <com/sun/star/awt/XPatternField.hpp>
#include <com/sun/star/awt/XDateField.hpp>
#include <com/sun/star/awt/XComboBox.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XItemListListener.hpp>
#include <com/sun/star/awt/XImageConsumer.hpp>
#include <com/sun/star/awt/XSimpleTabController.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/Date.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase2.hxx>

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxtopwindow.hxx>
#include <cppuhelper/implbase1.hxx>

#include <vcl/pointr.hxx>
#include <vcl/image.hxx>
#include <vcl/tabctrl.hxx>

class Button;
class CheckBox;
class RadioButton;
class ListBox;
class ScrollBar;
class Edit;
class Menu;
class ComboBox;
class FormatterBase;
class SpinField;
class ToolBox;
class VclSimpleEvent;
class VclMenuEvent;


//  class VCLXGraphicControl
//    deriving from VCLXWindow, drawing the graphic which exists as "Graphic" at the model



class TOOLKIT_DLLPUBLIC VCLXGraphicControl : public VCLXWindow
{
private:
    /// the image we currently display
    Image                       maImage;

protected:
    const Image&    GetImage() const { return maImage; }

protected:
    // css::awt::XWindow
    void SAL_CALL setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

protected:
    /** forward our bitmap to our window
        @precond
            our mutex is locked
        @precond
            GetWindow is not <NULL/>
        @see GetBitmap
    */
    virtual void    ImplSetNewImage();

public:
    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }

};


//  class VCLXButton

typedef ::cppu::ImplInheritanceHelper2  <   VCLXGraphicControl
                                        ,   css::awt::XButton
                                        ,   css::awt::XToggleButton
                                        >   VCLXButton_Base;
class VCLXButton :public VCLXButton_Base
{
private:
    OUString             maActionCommand;
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;

protected:
    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;

public:
                    VCLXButton();
    virtual ~VCLXButton();

    // css::lang::XComponent
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XButton
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setLabel( const OUString& Label ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setActionCommand( const OUString& Command ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XToggleButton
    // css::awt::XItemEventBroadcaster
    void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXImageControl

class VCLXImageControl : public VCLXGraphicControl
{
public:
                    VCLXImageControl();
                    virtual ~VCLXImageControl();

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }

protected:
    virtual void    ImplSetNewImage() override;
};


//  class VCLXCheckBox

class VCLXCheckBox :    public css::awt::XCheckBox,
                        public css::awt::XButton,
                        public VCLXGraphicControl
{
private:
    ActionListenerMultiplexer   maActionListeners;
    OUString             maActionCommand;
    ItemListenerMultiplexer     maItemListeners;

protected:
    void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;

public:
                    VCLXCheckBox();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;


    // css::lang::XComponent
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XCheckBox
    void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getState(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setState( sal_Int16 n ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setLabel( const OUString& Label ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL enableTriState( sal_Bool b ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XButton:
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setActionCommand( const OUString& Command ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXRadioButton

class VCLXRadioButton : public css::awt::XRadioButton,
                        public css::awt::XButton,
                        public VCLXGraphicControl
{
private:
    ItemListenerMultiplexer     maItemListeners;
    ActionListenerMultiplexer   maActionListeners;
    OUString             maActionCommand;

protected:
    void            ImplClickedOrToggled( bool bToggled );
    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;

public:
                    VCLXRadioButton();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XComponent
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XRadioButton
    void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL getState(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setState( sal_Bool b ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setLabel( const OUString& Label ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XButton:
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setActionCommand( const OUString& Command ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }

};


//  class VCLXMessageBox

class VCLXMessageBox :  public css::awt::XMessageBox,
                        public VCLXTopWindow
{
public:
                        VCLXMessageBox();
                        virtual ~VCLXMessageBox();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;


    // css::awt::XMessageBox
    void SAL_CALL setCaptionText( const OUString& aText ) throw(css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getCaptionText(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMessageText( const OUString& aText ) throw(css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getMessageText(  ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL execute(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize() throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXFrame

class VCLXFrame :   public VCLXContainer
{
protected:
    void                        ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

public:
    VCLXFrame();
    virtual ~VCLXFrame();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XView
    void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XDevice,
    css::awt::DeviceInfo SAL_CALL getInfo() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};




//  class VCLXDialog

class TOOLKIT_DLLPUBLIC VCLXDialog :    public css::awt::XDialog2,
                    public VCLXTopWindow
{
public:
                        VCLXDialog();
                        virtual ~VCLXDialog();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XDialog2
    virtual void SAL_CALL endDialog( ::sal_Int32 Result ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHelpId( const OUString& Id ) throw (css::uno::RuntimeException, std::exception) override;

    // css::awt::XDialog
    void SAL_CALL setTitle( const OUString& Title ) throw(css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getTitle(  ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL execute(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL endExecute(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XView
    void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XDevice,
    css::awt::DeviceInfo SAL_CALL getInfo() throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;

};


//  class VCLXTabPage

class VCLXTabPage : public VCLXContainer
{
public:
                        VCLXTabPage();
                        virtual ~VCLXTabPage();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XView
    void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XDevice,
    css::awt::DeviceInfo SAL_CALL getInfo() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;

    TabPage*  getTabPage() const throw ( css::uno::RuntimeException);
    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

class VCLXMultiPage : public css::awt::XSimpleTabController, public VCLXContainer
{
    TabListenerMultiplexer maTabListeners;
    sal_Int32 mTabId;
protected:
    void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
public:
    VCLXMultiPage();
    virtual ~VCLXMultiPage();

    // css::uno::XInterface
    css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL acquire() throw() override { OWeakObject::acquire(); }
    void SAL_CALL release() throw() override { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XComponent
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XView
    void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XDevice,
    css::awt::DeviceInfo SAL_CALL getInfo() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;
    // XSimpleTabController
    virtual ::sal_Int32 SAL_CALL insertTab() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTab( ::sal_Int32 ID ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setTabProps( ::sal_Int32 ID, const css::uno::Sequence< css::beans::NamedValue >& Properties ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::NamedValue > SAL_CALL getTabProps( ::sal_Int32 ID ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL activateTab( ::sal_Int32 ID ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getActiveTabID() throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;
    // C++
    TabControl*  getTabControl() const throw ( css::uno::RuntimeException);
    sal_uInt16 insertTab( TabPage*, OUString& sTitle );
    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXFixedHyperlink

class TOOLKIT_DLLPUBLIC VCLXFixedHyperlink :
    public css::awt::XFixedHyperlink,
    public VCLXWindow
{
private:
    ActionListenerMultiplexer   maActionListeners;

protected:
    void                        ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
                                CreateAccessibleContext() override;

public:
    VCLXFixedHyperlink();
    virtual ~VCLXFixedHyperlink();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XComponent
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XFixedHyperlink
    void SAL_CALL setText( const OUString& Text ) throw(css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getText(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setURL( const OUString& URL ) throw(css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getURL(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setAlignment( sal_Int16 nAlign ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getAlignment(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXFixedText

class VCLXFixedText :   public css::awt::XFixedText,
                        public VCLXWindow
{
protected:
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;

public:
                        VCLXFixedText();
                        virtual ~VCLXFixedText();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XFixedText
    void SAL_CALL setText( const OUString& Text ) throw(css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getText(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setAlignment( sal_Int16 nAlign ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getAlignment(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXScrollBar

class VCLXScrollBar :   public css::awt::XScrollBar,
                        public VCLXWindow
{
private:
    AdjustmentListenerMultiplexer maAdjustmentListeners;

protected:
    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;

public:
                    VCLXScrollBar();
    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;


    // css::lang::XComponent
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XScrollbar
    void SAL_CALL addAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setValue( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int32 SAL_CALL getValue(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMaximum( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int32 SAL_CALL getMaximum(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setLineIncrement( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int32 SAL_CALL getLineIncrement(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setBlockIncrement( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int32 SAL_CALL getBlockIncrement(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setVisibleSize( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int32 SAL_CALL getVisibleSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setOrientation( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int32 SAL_CALL getOrientation(  ) throw(css::uno::RuntimeException, std::exception) override;

    // why isn't this part of the XScrollbar?
    void SAL_CALL setMinimum( sal_Int32 n ) throw(css::uno::RuntimeException);
    sal_Int32 SAL_CALL getMinimum(  ) throw(css::uno::RuntimeException);

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize() throw(css::uno::RuntimeException, std::exception) override;
    static css::awt::Size SAL_CALL implGetMinimumSize( vcl::Window* p ) throw(css::uno::RuntimeException);

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXEdit

class TOOLKIT_DLLPUBLIC VCLXEdit :  public css::awt::XTextComponent,
                    public css::awt::XTextEditField,
                    public css::awt::XTextLayoutConstrains,
                    public VCLXWindow
{
private:
    TextListenerMultiplexer maTextListeners;

protected:
    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;


public:
                    VCLXEdit();

    TextListenerMultiplexer&    GetTextListeners() { return maTextListeners; }

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;


    // css::lang::XComponent
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTextComponent
    void SAL_CALL addTextListener( const css::uno::Reference< css::awt::XTextListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeTextListener( const css::uno::Reference< css::awt::XTextListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setText( const OUString& aText ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL insertText( const css::awt::Selection& Sel, const OUString& Text ) throw(css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getText(  ) throw(css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getSelectedText(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setSelection( const css::awt::Selection& aSelection ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Selection SAL_CALL getSelection(  ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isEditable(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setEditable( sal_Bool bEditable ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMaxTextLen( sal_Int16 nLen ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getMaxTextLen(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTextEditField:
    void SAL_CALL setEchoChar( sal_Unicode cEcho ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXListBox

typedef ::cppu::ImplInheritanceHelper3  <   VCLXWindow
                                        ,   css::awt::XListBox
                                        ,   css::awt::XTextLayoutConstrains
                                        ,   css::awt::XItemListListener
                                        >   VCLXListBox_Base;
class VCLXListBox  : public VCLXListBox_Base
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;

protected:
    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
                    CreateAccessibleContext() override;
    void            ImplCallItemListeners();

public:
                        VCLXListBox();

    // css::lang::XComponent
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XListBox
    void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addItem( const OUString& aItem, sal_Int16 nPos ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addItems( const css::uno::Sequence< OUString >& aItems, sal_Int16 nPos ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getItemCount(  ) throw(css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getItem( sal_Int16 nPos ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< OUString > SAL_CALL getItems(  ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getSelectedItemPos(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int16 > SAL_CALL getSelectedItemsPos(  ) throw(css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getSelectedItem(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< OUString > SAL_CALL getSelectedItems(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL selectItemPos( sal_Int16 nPos, sal_Bool bSelect ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL selectItemsPos( const css::uno::Sequence< sal_Int16 >& aPositions, sal_Bool bSelect ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL selectItem( const OUString& aItem, sal_Bool bSelect ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isMutipleMode(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMultipleMode( sal_Bool bMulti ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getDropDownLineCount(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setDropDownLineCount( sal_Int16 nLines ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL makeVisible( sal_Int16 nEntry ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    // XItemListListener
    virtual void SAL_CALL listItemInserted( const css::awt::ItemListEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL listItemRemoved( const css::awt::ItemListEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL listItemModified( const css::awt::ItemListEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL allItemsRemoved( const css::lang::EventObject& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL itemListChanged( const css::lang::EventObject& Event ) throw (css::uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& i_rEvent ) throw (css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXComboBox

typedef ::cppu::ImplInheritanceHelper2  <   VCLXEdit
                                        ,   css::awt::XComboBox
                                        ,   css::awt::XItemListListener > VCLXComboBox_Base;
class VCLXComboBox :    public VCLXComboBox_Base
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;

protected:
    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;


public:
                        VCLXComboBox();
    virtual ~VCLXComboBox();

     // css::lang::XComponent
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XComboBox
    void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addItem( const OUString& aItem, sal_Int16 nPos ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addItems( const css::uno::Sequence< OUString >& aItems, sal_Int16 nPos ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getItemCount(  ) throw(css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getItem( sal_Int16 nPos ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< OUString > SAL_CALL getItems(  ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getDropDownLineCount(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setDropDownLineCount( sal_Int16 nLines ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    // XItemListListener
    virtual void SAL_CALL listItemInserted( const css::awt::ItemListEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL listItemRemoved( const css::awt::ItemListEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL listItemModified( const css::awt::ItemListEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL allItemsRemoved( const css::lang::EventObject& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL itemListChanged( const css::lang::EventObject& Event ) throw (css::uno::RuntimeException, std::exception) override;
    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& i_rEvent ) throw (css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXSpinField

class TOOLKIT_DLLPUBLIC VCLXSpinField : public css::awt::XSpinField,
                        public VCLXEdit
{
private:
    SpinListenerMultiplexer maSpinListeners;

protected:
    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

public:
                    VCLXSpinField();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;


    // css::awt::XSpinField
    void SAL_CALL addSpinListener( const css::uno::Reference< css::awt::XSpinListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeSpinListener( const css::uno::Reference< css::awt::XSpinListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL up(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL down(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL first(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL last(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL enableRepeat( sal_Bool bRepeat ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXFormattedSpinField

class VCLXFormattedSpinField : public VCLXSpinField
{
private:
    FormatterBase*  mpFormatter;

protected:
    FormatterBase*  GetFormatter() const { return GetWindow() ? mpFormatter : nullptr; }

public:
                    VCLXFormattedSpinField();
                    virtual ~VCLXFormattedSpinField();

    void            SetFormatter( FormatterBase* pFormatter ) { mpFormatter = pFormatter; }

    void            setStrictFormat( bool bStrict );
    bool        isStrictFormat();

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXDateField

class TOOLKIT_DLLPUBLIC VCLXDateField : public css::awt::XDateField,
                        public VCLXFormattedSpinField
{
protected:
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;
public:
                    VCLXDateField();
                    virtual ~VCLXDateField();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;


    // css::awt::XDateField
    void SAL_CALL setDate( const css::util::Date& Date ) throw(css::uno::RuntimeException, std::exception) override;
    css::util::Date SAL_CALL getDate(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMin( const css::util::Date& Date ) throw(css::uno::RuntimeException, std::exception) override;
    css::util::Date SAL_CALL getMin(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMax( const css::util::Date& Date ) throw(css::uno::RuntimeException, std::exception) override;
    css::util::Date SAL_CALL getMax(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setFirst( const css::util::Date& Date ) throw(css::uno::RuntimeException, std::exception) override;
    css::util::Date SAL_CALL getFirst(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setLast( const css::util::Date& Date ) throw(css::uno::RuntimeException, std::exception) override;
    css::util::Date SAL_CALL getLast(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setLongFormat( sal_Bool bLong ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isLongFormat(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setEmpty(  ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isEmpty(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isStrictFormat(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXTimeField

class VCLXTimeField :   public css::awt::XTimeField,
                        public VCLXFormattedSpinField
{
protected:
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;
public:
                    VCLXTimeField();
                    virtual ~VCLXTimeField();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTimeField
    void SAL_CALL setTime( const css::util::Time& Time ) throw(css::uno::RuntimeException, std::exception) override;
    css::util::Time SAL_CALL getTime(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMin( const css::util::Time& Time ) throw(css::uno::RuntimeException, std::exception) override;
    css::util::Time SAL_CALL getMin(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMax( const css::util::Time& Time ) throw(css::uno::RuntimeException, std::exception) override;
    css::util::Time SAL_CALL getMax(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setFirst( const css::util::Time& Time ) throw(css::uno::RuntimeException, std::exception) override;
    css::util::Time SAL_CALL getFirst(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setLast( const css::util::Time& Time ) throw(css::uno::RuntimeException, std::exception) override;
    css::util::Time SAL_CALL getLast(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setEmpty(  ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isEmpty(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isStrictFormat(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXNumericField

class VCLXNumericField :    public css::awt::XNumericField,
                            public VCLXFormattedSpinField
{
public:
                    VCLXNumericField();
                    virtual ~VCLXNumericField();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XNumericField
    void SAL_CALL setValue( double Value ) throw(css::uno::RuntimeException, std::exception) override;
    double SAL_CALL getValue(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMin( double Value ) throw(css::uno::RuntimeException, std::exception) override;
    double SAL_CALL getMin(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMax( double Value ) throw(css::uno::RuntimeException, std::exception) override;
    double SAL_CALL getMax(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setFirst( double Value ) throw(css::uno::RuntimeException, std::exception) override;
    double SAL_CALL getFirst(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setLast( double Value ) throw(css::uno::RuntimeException, std::exception) override;
    double SAL_CALL getLast(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setSpinSize( double Value ) throw(css::uno::RuntimeException, std::exception) override;
    double SAL_CALL getSpinSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setDecimalDigits( sal_Int16 nDigits ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getDecimalDigits(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isStrictFormat(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXMetricField

class MetricFormatter;
class MetricField;
class VCLXMetricField : public css::awt::XMetricField,
                        public VCLXFormattedSpinField
{
    MetricFormatter *GetMetricFormatter() throw(css::uno::RuntimeException);
    MetricField     *GetMetricField() throw(css::uno::RuntimeException);
    void CallListeners();
public:
    VCLXMetricField();
    virtual ~VCLXMetricField();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XMetricField
    virtual void SAL_CALL setValue( ::sal_Int64 Value, ::sal_Int16 Unit ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setUserValue( ::sal_Int64 Value, ::sal_Int16 Unit ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int64 SAL_CALL getValue( ::sal_Int16 Unit ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int64 SAL_CALL getCorrectedValue( ::sal_Int16 Unit ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMin( ::sal_Int64 Value, ::sal_Int16 Unit ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int64 SAL_CALL getMin( ::sal_Int16 Unit ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMax( ::sal_Int64 Value, ::sal_Int16 Unit ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int64 SAL_CALL getMax( ::sal_Int16 Unit ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFirst( ::sal_Int64 Value, ::sal_Int16 Unit ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int64 SAL_CALL getFirst( ::sal_Int16 Unit ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLast( ::sal_Int64 Value, ::sal_Int16 Unit ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int64 SAL_CALL getLast( ::sal_Int16 Unit ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSpinSize( ::sal_Int64 Value ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int64 SAL_CALL getSpinSize(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDecimalDigits( ::sal_Int16 nDigits ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int16 SAL_CALL getDecimalDigits(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isStrictFormat(  ) throw (css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXCurrencyField

class VCLXCurrencyField :   public css::awt::XCurrencyField,
                            public VCLXFormattedSpinField
{
public:
                    VCLXCurrencyField();
                    virtual ~VCLXCurrencyField();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;


    // css::awt::XCurrencyField
    void SAL_CALL setValue( double Value ) throw(css::uno::RuntimeException, std::exception) override;
    double SAL_CALL getValue(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMin( double Value ) throw(css::uno::RuntimeException, std::exception) override;
    double SAL_CALL getMin(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMax( double Value ) throw(css::uno::RuntimeException, std::exception) override;
    double SAL_CALL getMax(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setFirst( double Value ) throw(css::uno::RuntimeException, std::exception) override;
    double SAL_CALL getFirst(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setLast( double Value ) throw(css::uno::RuntimeException, std::exception) override;
    double SAL_CALL getLast(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setSpinSize( double Value ) throw(css::uno::RuntimeException, std::exception) override;
    double SAL_CALL getSpinSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setDecimalDigits( sal_Int16 nDigits ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getDecimalDigits(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isStrictFormat(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXPatternField

class VCLXPatternField :    public css::awt::XPatternField,
                            public VCLXFormattedSpinField
{
public:
                    VCLXPatternField();
                    virtual ~VCLXPatternField();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;


    // css::awt::XPatternField
    void SAL_CALL setMasks( const OUString& EditMask, const OUString& LiteralMask ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL getMasks( OUString& EditMask, OUString& LiteralMask ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setString( const OUString& Str ) throw(css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getString(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isStrictFormat(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXToolBox

class VCLXToolBox : public VCLXWindow
{
private:

protected:
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
                        CreateAccessibleContext() override;

public:
                        VCLXToolBox();
                        virtual ~VCLXToolBox();
};

#endif // INCLUDED_TOOLKIT_AWT_VCLXWINDOWS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
