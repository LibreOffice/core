/*************************************************************************
 *
 *  $RCSfile: vclxwindows.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mt $ $Date: 2001-03-14 11:56:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLKIT_AWT_VCLXWINDOWS_HXX_
#define _TOOLKIT_AWT_VCLXWINDOWS_HXX_


#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XVETOABLECHANGELISTENER_HPP_
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATECHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyStateChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFILEDIALOG_HPP_
#include <com/sun/star/awt/XFileDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTCOMPONENT_HPP_
#include <com/sun/star/awt/XTextComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XLISTBOX_HPP_
#include <com/sun/star/awt/XListBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPROGRESSMONITOR_HPP_
#include <com/sun/star/awt/XProgressMonitor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_TEXTALIGN_HPP_
#include <com/sun/star/awt/TextAlign.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XSCROLLBAR_HPP_
#include <com/sun/star/awt/XScrollBar.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLCONTAINERPEER_HPP_
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLERMODEL_HPP_
#include <com/sun/star/awt/XTabControllerModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMESSAGEBOX_HPP_
#include <com/sun/star/awt/XMessageBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTEDITFIELD_HPP_
#include <com/sun/star/awt/XTextEditField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_STYLE_HPP_
#include <com/sun/star/awt/Style.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTIMEFIELD_HPP_
#include <com/sun/star/awt/XTimeField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLWINDOWPEER_HPP_
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XSPINFIELD_HPP_
#include <com/sun/star/awt/XSpinField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XUNOCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XUnoControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XNUMERICFIELD_HPP_
#include <com/sun/star/awt/XNumericField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XBUTTON_HPP_
#include <com/sun/star/awt/XButton.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPOINTER_HPP_
#include <com/sun/star/awt/XPointer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTAREA_HPP_
#include <com/sun/star/awt/XTextArea.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XIMAGEBUTTON_HPP_
#include <com/sun/star/awt/XImageButton.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFIXEDTEXT_HPP_
#include <com/sun/star/awt/XFixedText.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDIALOG_HPP_
#include <com/sun/star/awt/XDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SCROLLBARORIENTATION_HPP_
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XRADIOBUTTON_HPP_
#include <com/sun/star/awt/XRadioButton.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCURRENCYFIELD_HPP_
#include <com/sun/star/awt/XCurrencyField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPATTERNFIELD_HPP_
#include <com/sun/star/awt/XPatternField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_VCLWINDOWPEERATTRIBUTE_HPP_
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLER_HPP_
#include <com/sun/star/awt/XTabController.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLCONTAINER_HPP_
#include <com/sun/star/awt/XVclContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDATEFIELD_HPP_
#include <com/sun/star/awt/XDateField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCOMBOBOX_HPP_
#include <com/sun/star/awt/XComboBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCHECKBOX_HPP_
#include <com/sun/star/awt/XCheckBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XIMAGECONSUMER_HPP_
#include <com/sun/star/awt/XImageConsumer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MESSAGEBOXCOMMAND_HPP_
#include <com/sun/star/awt/MessageBoxCommand.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPROGRESSBAR_HPP_
#include <com/sun/star/awt/XProgressBar.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MENUITEMSTYLE_HPP_
#include <com/sun/star/awt/MenuItemStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPOPUPMENU_HPP_
#include <com/sun/star/awt/XPopupMenu.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMENUBAR_HPP_
#include <com/sun/star/awt/XMenuBar.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POPUPMENUDIRECTION_HPP_
#include <com/sun/star/awt/PopupMenuDirection.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MENUEVENT_HPP_
#include <com/sun/star/awt/MenuEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMENULISTENER_HPP_
#include <com/sun/star/awt/XMenuListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMENU_HPP_
#include <com/sun/star/awt/XMenu.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxtopwindow.hxx>

#include <vcl/pointr.hxx>
#include <vcl/imgcons.hxx>
#include <vcl/bitmapex.hxx>

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


static double ImplCalcLongValue( double nValue, sal_uInt16 nDigits )
{
    double n = nValue;
    for ( sal_uInt16 d = 0; d < nDigits; d++ )
        n *= 10;
    return n;
}

static double ImplCalcDoubleValue( double nValue, sal_uInt16 nDigits )
{
    double n = nValue;
    for ( sal_uInt16 d = 0; d < nDigits; d++ )
        n /= 10;
    return n;
}

//  ----------------------------------------------------
//  class VCLXButton
//  ----------------------------------------------------
class VCLXButton :  public ::com::sun::star::awt::XImageConsumer,
                    public ::com::sun::star::awt::XButton,
                    public VCLXWindow
{
private:
    ::rtl::OUString             maActionCommand;
    ImageConsumer               maImageConsumer;
    BitmapEx                    maBitmap;
    ActionListenerMultiplexer   maActionListeners;

protected:
    DECL_LINK(      ClickHdl, Button* );
    void            ImplUpdateImage( sal_Bool bGetNewImage );

public:
                    VCLXButton();
                    ~VCLXButton();

    void            SetWindow( Window* pWindow );

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XButton:
    void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLabel( const ::rtl::OUString& Label ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setActionCommand( const ::rtl::OUString& Command ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XImageConsumer
    void SAL_CALL init( sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setColorModel( sal_Int16 BitCount, const ::com::sun::star::uno::Sequence< sal_Int32 >& RGBAPal, sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask, sal_Int32 AlphaMask ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setPixelsByBytes( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int8 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setPixelsByLongs( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int32 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL complete( sal_Int32 Status, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer >& xProducer ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::VclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXImageControl
//  ----------------------------------------------------
class VCLXImageControl :    public ::com::sun::star::awt::XImageConsumer,
                            public VCLXWindow
{
    ImageConsumer   maImageConsumer;
    BitmapEx        maBitmap;

protected:
    void            ImplUpdateImage( sal_Bool bGetNewImage );

public:
                    VCLXImageControl();
                    ~VCLXImageControl();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XWindow
    void SAL_CALL setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XImageConsumer
    void SAL_CALL init( sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setColorModel( sal_Int16 BitCount, const ::com::sun::star::uno::Sequence< sal_Int32 >& RGBAPal, sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask, sal_Int32 AlphaMask ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setPixelsByBytes( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int8 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setPixelsByLongs( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int32 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL complete( sal_Int32 Status, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer >& xProducer ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXCheckBox
//  ----------------------------------------------------
class VCLXCheckBox :    public ::com::sun::star::awt::XCheckBox,
                        public VCLXWindow
{
private:
    ItemListenerMultiplexer maItemListeners;

protected:
    DECL_LINK(      ClickHdl, CheckBox* );

public:
                    VCLXCheckBox();
                    ~VCLXCheckBox();

    void    SetWindow( Window* pWindow );

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XCheckBox
    void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getState(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setState( sal_Int16 n ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLabel( const ::rtl::OUString& Label ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL enableTriState( sal_Bool b ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::VclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXRadioButton
//  ----------------------------------------------------
class VCLXRadioButton : public ::com::sun::star::awt::XRadioButton,
                        public VCLXWindow
{
private:
    ItemListenerMultiplexer maItemListeners;

protected:
    DECL_LINK(      ClickHdl, RadioButton* );
    DECL_LINK(      ToggleHdl, RadioButton* );
    void            ImplClickedOrToggled( BOOL bToggled );


public:
                    VCLXRadioButton();
                    ~VCLXRadioButton();

    void    SetWindow( Window* pWindow );

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XRadioButton
    void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL getState(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setState( sal_Bool b ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLabel( const ::rtl::OUString& Label ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::VclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXMessageBox
//  ----------------------------------------------------
class VCLXMessageBox :  public ::com::sun::star::awt::XMessageBox,
                        public VCLXWindow
{
public:
                        VCLXMessageBox();
                        ~VCLXMessageBox();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::awt::XMessageBox
    void SAL_CALL setCaptionText( const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getCaptionText(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMessageText( const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getMessageText(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL execute(  ) throw(::com::sun::star::uno::RuntimeException);
};



//  ----------------------------------------------------
//  class VCLXDialog
//  ----------------------------------------------------
class VCLXDialog :  public ::com::sun::star::awt::XDialog,
                    public VCLXTopWindow
{
public:
                        VCLXDialog();
                        ~VCLXDialog();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XDialog
    void SAL_CALL setTitle( const ::rtl::OUString& Title ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getTitle(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL execute(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL endExecute(  ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXFixedText
//  ----------------------------------------------------
class VCLXFixedText :   public ::com::sun::star::awt::XFixedText,
                        public VCLXWindow
{
public:
                        VCLXFixedText();
                        ~VCLXFixedText();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XFixedText
    void SAL_CALL setText( const ::rtl::OUString& Text ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getText(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setAlignment( sal_Int16 nAlign ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getAlignment(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXScrollBar
//  ----------------------------------------------------
class VCLXScrollBar :   public ::com::sun::star::awt::XScrollBar,
                        public VCLXWindow
{
private:
    AdjustmentListenerMultiplexer maAdjustmentListeners;

protected:
    DECL_LINK(      ScrollHdl, ScrollBar* );

public:
                    VCLXScrollBar();
                    ~VCLXScrollBar();

    void    SetWindow( Window* pWindow );

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // XScrollbar
    void SAL_CALL addAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setValue( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getValue(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMaximum( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getMaximum(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLineIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getLineIncrement(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setBlockIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getBlockIncrement(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setVisibleSize( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getVisibleSize(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setOrientation( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getOrientation(  ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXEdit
//  ----------------------------------------------------
class VCLXEdit :    public ::com::sun::star::awt::XTextComponent,
                    public ::com::sun::star::awt::XTextEditField,
                    public ::com::sun::star::awt::XTextLayoutConstrains,
                    public VCLXWindow
{
private:
    TextListenerMultiplexer maTextListeners;

protected:
    DECL_LINK(      ModifyHdl, Edit* );

public:
                    VCLXEdit();
                    ~VCLXEdit();

    void    SetWindow( Window* pWindow );
    TextListenerMultiplexer&    GetTextListeners() { return maTextListeners; }

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextComponent
    void SAL_CALL addTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setText( const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL insertText( const ::com::sun::star::awt::Selection& Sel, const ::rtl::OUString& Text ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getText(  ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getSelectedText(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Selection SAL_CALL getSelection(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isEditable(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setEditable( sal_Bool bEditable ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMaxTextLen( sal_Int16 nLen ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getMaxTextLen(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextEditField:
    void SAL_CALL setEchoChar( sal_Unicode cEcho ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::VclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXListBox
//  ----------------------------------------------------
class VCLXListBox : public ::com::sun::star::awt::XListBox,
                    public ::com::sun::star::awt::XTextLayoutConstrains,
                    public VCLXWindow
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;

protected:
    DECL_LINK(          SelectHdl, ListBox* );
    DECL_LINK(          DoubleClickHdl, ListBox* );

public:
                        VCLXListBox();
                        ~VCLXListBox();

    void        SetWindow( Window* pWindow );

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XListBox
    void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addItem( const ::rtl::OUString& aItem, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addItems( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aItems, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getItemCount(  ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getItem( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getItems(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getSelectedItemPos(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSelectedItemsPos(  ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getSelectedItem(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSelectedItems(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL selectItemPos( sal_Int16 nPos, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL selectItemsPos( const ::com::sun::star::uno::Sequence< sal_Int16 >& aPositions, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL selectItem( const ::rtl::OUString& aItem, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isMutipleMode(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMultipleMode( sal_Bool bMulti ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getDropDownLineCount(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setDropDownLineCount( sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL makeVisible( sal_Int16 nEntry ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::VclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXComboBox
//  ----------------------------------------------------
class VCLXComboBox :    public ::com::sun::star::awt::XComboBox,
                        public VCLXEdit
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;

protected:
    DECL_LINK(          SelectHdl, ComboBox* );
    DECL_LINK(          DoubleClickHdl, ComboBox* );

public:
                        VCLXComboBox();
                        ~VCLXComboBox();

    void        SetWindow( Window* pWindow );

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

     // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XComboBox
    void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addItem( const ::rtl::OUString& aItem, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addItems( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aItems, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getItemCount(  ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getItem( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getItems(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getDropDownLineCount(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setDropDownLineCount( sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::VclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXSpinField
//  ----------------------------------------------------
class VCLXSpinField :   public ::com::sun::star::awt::XSpinField,
                        public VCLXEdit
{
private:
    SpinListenerMultiplexer maSpinListeners;

protected:
    DECL_LINK(      SpinUpHdl, SpinField* );
    DECL_LINK(      SpinDownHdl, SpinField* );
    DECL_LINK(      SpinFirstHdl, SpinField* );
    DECL_LINK(      SpinLastHdl, SpinField* );

public:
                    VCLXSpinField();
                    ~VCLXSpinField();

    void            SetWindow( Window* pWindow );

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::awt::XSpinField
    void SAL_CALL addSpinListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeSpinListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL up(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL down(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL first(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL last(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL enableRepeat( sal_Bool bRepeat ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXFormattedSpinField
//  ----------------------------------------------------
class VCLXFormattedSpinField : public VCLXSpinField
{
private:
    FormatterBase*  mpFormatter;

protected:
    FormatterBase*  GetFormatter() const { return GetWindow() ? mpFormatter : NULL; }

public:
                    VCLXFormattedSpinField();
                    ~VCLXFormattedSpinField();

    void            SetFormatter( FormatterBase* pFormatter ) { mpFormatter = pFormatter; }

    void            setStrictFormat( sal_Bool bStrict );
    sal_Bool        isStrictFormat();

    // ::com::sun::star::awt::VclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXDateField
//  ----------------------------------------------------
class VCLXDateField :   public ::com::sun::star::awt::XDateField,
                        public VCLXFormattedSpinField
{
public:
                    VCLXDateField();
                    ~VCLXDateField();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::awt::XDateField
    void SAL_CALL setDate( sal_Int32 Date ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getDate(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMin( sal_Int32 Date ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getMin(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMax( sal_Int32 Date ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getMax(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setFirst( sal_Int32 Date ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getFirst(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLast( sal_Int32 Date ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getLast(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLongFormat( sal_Bool bLong ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isLongFormat(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setEmpty(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isEmpty(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isStrictFormat(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::VclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXTimeField
//  ----------------------------------------------------
class VCLXTimeField :   public ::com::sun::star::awt::XTimeField,
                        public VCLXFormattedSpinField
{
public:
                    VCLXTimeField();
                    ~VCLXTimeField();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTimeField
    void SAL_CALL setTime( sal_Int32 Time ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getTime(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMin( sal_Int32 Time ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getMin(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMax( sal_Int32 Time ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getMax(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setFirst( sal_Int32 Time ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getFirst(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLast( sal_Int32 Time ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getLast(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setEmpty(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isEmpty(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isStrictFormat(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::VclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXNumericField
//  ----------------------------------------------------
class VCLXNumericField :    public ::com::sun::star::awt::XNumericField,
                            public VCLXFormattedSpinField
{
public:
                    VCLXNumericField();
                    ~VCLXNumericField();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XNumericField
    void SAL_CALL setValue( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getValue(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMin( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getMin(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMax( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getMax(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getFirst(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLast( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getLast(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getSpinSize(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setDecimalDigits( sal_Int16 nDigits ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getDecimalDigits(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isStrictFormat(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::VclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXCurrencyField
//  ----------------------------------------------------
class VCLXCurrencyField :   public ::com::sun::star::awt::XCurrencyField,
                            public VCLXFormattedSpinField
{
public:
                    VCLXCurrencyField();
                    ~VCLXCurrencyField();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::awt::XCurrencyField
    void SAL_CALL setValue( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getValue(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMin( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getMin(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMax( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getMax(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getFirst(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLast( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getLast(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getSpinSize(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setDecimalDigits( sal_Int16 nDigits ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getDecimalDigits(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isStrictFormat(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::VclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXPatternField
//  ----------------------------------------------------
class VCLXPatternField :    public ::com::sun::star::awt::XPatternField,
                            public VCLXFormattedSpinField
{
public:
                    VCLXPatternField();
                    ~VCLXPatternField();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::awt::XPatternField
    void SAL_CALL setMasks( const ::rtl::OUString& EditMask, const ::rtl::OUString& LiteralMask ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL getMasks( ::rtl::OUString& EditMask, ::rtl::OUString& LiteralMask ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setString( const ::rtl::OUString& Str ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getString(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isStrictFormat(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::VclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
};



#endif // _TOOLKIT_AWT_VCLXWINDOWS_HXX_

