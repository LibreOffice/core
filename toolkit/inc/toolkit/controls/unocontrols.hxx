/*************************************************************************
 *
 *  $RCSfile: unocontrols.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: mt $ $Date: 2001-04-04 09:26:17 $
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

#ifndef _TOOLKIT_HELPER_UNOCONTROLS_HXX_
#define _TOOLKIT_HELPER_UNOCONTROLS_HXX_

#ifndef _COM_SUN_STAR_AWT_XDIALOG_HPP_
#include <com/sun/star/awt/XDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTCOMPONENT_HPP_
#include <com/sun/star/awt/XTextComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTLISTENER_HPP_
#include <com/sun/star/awt/XTextListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XBUTTON_HPP_
#include <com/sun/star/awt/XButton.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XIMAGECONSUMER_HPP_
#include <com/sun/star/awt/XImageConsumer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XIMAGEPRODUCER_HPP_
#include <com/sun/star/awt/XImageProducer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XRADIOBUTTON_HPP_
#include <com/sun/star/awt/XRadioButton.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XITEMLISTENER_HPP_
#include <com/sun/star/awt/XItemListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCHECKBOX_HPP_
#include <com/sun/star/awt/XCheckBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFIXEDTEXT_HPP_
#include <com/sun/star/awt/XFixedText.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XLISTBOX_HPP_
#include <com/sun/star/awt/XListBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCOMBOBOX_HPP_
#include <com/sun/star/awt/XComboBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDATEFIELD_HPP_
#include <com/sun/star/awt/XDateField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTIMEFIELD_HPP_
#include <com/sun/star/awt/XTimeField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XNUMERICFIELD_HPP_
#include <com/sun/star/awt/XNumericField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCURRENCYFIELD_HPP_
#include <com/sun/star/awt/XCurrencyField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPATTERNFIELD_HPP_
#include <com/sun/star/awt/XPatternField.hpp>
#endif
// #ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
// #include <com/sun/star/container/XIndexContainer.hpp>
// #endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif

#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/controls/unocontrolcontainer.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/servicenames.hxx>

#include <vcl/imgcons.hxx>
#include <vcl/bitmapex.hxx>

struct UnoControlModelHolder;
class UnoControlModelHolderList;

//  ----------------------------------------------------
//  class UnoControlDialogModel
//  ----------------------------------------------------
class UnoControlDialogModel : public UnoControlModel,
                              public ::com::sun::star::lang::XMultiServiceFactory,
                              public ::com::sun::star::container::XContainer,
                              public ::com::sun::star::container::XNameContainer
{
private:
    ContainerListenerMultiplexer    maContainerListeners;
    UnoControlModelHolderList*      mpModels;

protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();
    UnoControlModelHolder*          ImplFindElement( const ::rtl::OUString& rName ) const;


public:
                        UnoControlDialogModel();
                        UnoControlDialogModel( const UnoControlDialogModel& rModel );
                        ~UnoControlDialogModel();

    UnoControlModel*    Clone() const;

    ::rtl::OUString     getServiceName() const;

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlModel::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XContainer
    void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XElementAcces
    ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XIndexContainer, XIndexReplace, XIndexAcces
    // void SAL_CALL replaceByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) = 0;
    // sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException) = 0;
    // ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) = 0;
    // void SAL_CALL insertByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) = 0;
    // void SAL_CALL removeByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) = 0;

    // ::com::sun::star::container::XNameContainer, XNameReplace, XNameAccess
    void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeByName( const ::rtl::OUString& Name ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XMultiServiceFactory
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const ::rtl::OUString& aServiceSpecifier ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);


    // XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlDialogModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlDialogModel ) )
};

//  ----------------------------------------------------
//  class UnoDialogControl
//  ----------------------------------------------------
class UnoDialogControl : public UnoControlContainer,
                         public ::com::sun::star::container::XContainerListener,
                         public ::com::sun::star::awt::XDialog
{
protected:

    void        ImplInsertControl( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel, const ::rtl::OUString& rName );
    void        ImplRemoveControl( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel );
    void        ImplSetPosSize( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& rxCtrl );

public:

                                UnoDialogControl();
    ::rtl::OUString             GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlContainer::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException) { UnoControlContainer::disposing( Source ); }
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertiesChangeListener
    void SAL_CALL propertiesChange( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& evt ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XContainerListener
    void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);


    void SAL_CALL setTitle( const ::rtl::OUString& Title ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getTitle() throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL execute() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL endExecute() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setDesignMode( sal_Bool bOn ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoDialogControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlDialog ) )
};


//  ----------------------------------------------------
//  class UnoControlEditModel
//  ----------------------------------------------------
class UnoControlEditModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
                        UnoControlEditModel();
                        UnoControlEditModel( const UnoControlEditModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlEditModel( *this ); }

    ::rtl::OUString     getServiceName() const;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);


    // XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlEditModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlEditModel ) )
};

//  ----------------------------------------------------
//  class UnoEditControl
//  ----------------------------------------------------
class UnoEditControl :  public UnoControlBase,
                        public ::com::sun::star::awt::XTextComponent,
                        public ::com::sun::star::awt::XTextListener,
                        public ::com::sun::star::awt::XLayoutConstrains,
                        public ::com::sun::star::awt::XTextLayoutConstrains
{
private:
    TextListenerMultiplexer maTextListeners;

public:

                                UnoEditControl();
    ::rtl::OUString             GetComponentServiceName();
    TextListenerMultiplexer&    GetTextListeners()  { return maTextListeners; }

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException) { UnoControlBase::disposing( Source ); }
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextListener
    void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);

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

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoEditControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlEdit ) )
};

//  ----------------------------------------------------
//  class UnoControlFormattedFieldModel
//  ----------------------------------------------------
class UnoControlFormattedFieldModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

public:
                        UnoControlFormattedFieldModel();
                        UnoControlFormattedFieldModel( const UnoControlFormattedFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlFormattedFieldModel( *this ); }

    ::rtl::OUString     getServiceName() const;


    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlFormattedFieldModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFormattedFieldModel ) )
};

//  ----------------------------------------------------
//  class UnoFormattedFieldControl
//  ----------------------------------------------------
class UnoFormattedFieldControl : public UnoEditControl
{
public:
                        UnoFormattedFieldControl();
    ::rtl::OUString     GetComponentServiceName();

    // ::com::sun::star::awt::XTextListener
    void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoFormattedFieldControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFormattedField ) )
};

//  ----------------------------------------------------
//  class UnoControlFileControlModel
//  ----------------------------------------------------
class UnoControlFileControlModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

public:
                        UnoControlFileControlModel();
                        UnoControlFileControlModel( const UnoControlFileControlModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlFileControlModel( *this ); }

    ::rtl::OUString     getServiceName() const;


    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlFileControlModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFileControlModel ) )
};

//  ----------------------------------------------------
//  class UnoFileControl
//  ----------------------------------------------------
class UnoFileControl : public UnoEditControl
{
public:
                        UnoFileControl();
    ::rtl::OUString     GetComponentServiceName();

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoFileControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFileControl ) )
};

//  ----------------------------------------------------
//  class UnoControlButtonModel
//  ----------------------------------------------------
class UnoControlButtonModel :   public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

public:
                        UnoControlButtonModel();
                        UnoControlButtonModel( const UnoControlButtonModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlButtonModel( *this ); }

    ::rtl::OUString     getServiceName() const;


    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlButtonModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlButtonModel ) )
};

//  ----------------------------------------------------
//  class UnoButtonControl
//  ----------------------------------------------------
class UnoButtonControl :    public UnoControlBase,
                            public ::com::sun::star::awt::XButton,
                            public ::com::sun::star::awt::XImageConsumer,
                            public ::com::sun::star::awt::XImageProducer,
                            public ::com::sun::star::awt::XLayoutConstrains
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ::rtl::OUString             maActionCommand;

    ImageConsumer               maImageConsumer;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer > mxImageProducer;
    BitmapEx                    maBitmap;

protected:
    void                        ImplUpdateImage( sal_Bool bGetNewImage );

public:

                        UnoButtonControl();
    ::rtl::OUString     GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XButton
    void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLabel( const ::rtl::OUString& Label ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setActionCommand( const ::rtl::OUString& Command ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XImageConsumer
    void SAL_CALL init( sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setColorModel( sal_Int16 BitCount, const ::com::sun::star::uno::Sequence< sal_Int32 >& RGBAPal, sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask, sal_Int32 AlphaMask ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setPixelsByBytes( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int8 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setPixelsByLongs( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int32 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL complete( sal_Int32 Status, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer >& xProducer ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XImageProducer
    void SAL_CALL addConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL startProduction(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoButtonControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlButton ) )

};

//  ----------------------------------------------------
//  class UnoControlImageControlModel
//  ----------------------------------------------------
class UnoControlImageControlModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

public:
                                    UnoControlImageControlModel();
                                    UnoControlImageControlModel( const UnoControlImageControlModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlImageControlModel( *this ); }

    ::rtl::OUString                 getServiceName() const;


    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlImageControlModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlImageControlModel ) )
};

//  ----------------------------------------------------
//  class UnoImageControlControl
//  ----------------------------------------------------
class UnoImageControlControl :  public UnoControlBase,
                                public ::com::sun::star::awt::XImageConsumer,
                                public ::com::sun::star::awt::XImageProducer,
                                public ::com::sun::star::awt::XLayoutConstrains
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ::rtl::OUString             maActionCommand;

    ImageConsumer               maImageConsumer;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer > mxImageProducer;
    BitmapEx                    maBitmap;

protected:
    void            ImplUpdateImage( sal_Bool bGetNewImage );

public:

                            UnoImageControlControl();
    ::rtl::OUString         GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XImageConsumer
    void SAL_CALL init( sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setColorModel( sal_Int16 BitCount, const ::com::sun::star::uno::Sequence< sal_Int32 >& RGBAPal, sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask, sal_Int32 AlphaMask ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setPixelsByBytes( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int8 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setPixelsByLongs( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int32 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL complete( sal_Int32 Status, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer >& xProducer ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XImageProducer
    void SAL_CALL addConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL startProduction(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoImageControlControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlImageControl ) )

};

//  ----------------------------------------------------
//  class UnoControlRadioButtonModel
//  ----------------------------------------------------
class UnoControlRadioButtonModel :  public UnoControlModel

{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
                        UnoControlRadioButtonModel();
                        UnoControlRadioButtonModel( const UnoControlRadioButtonModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlRadioButtonModel( *this ); }

    ::rtl::OUString     getServiceName() const;


    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlRadioButtonModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlRadioButtonModel ) )

};

//  ----------------------------------------------------
//  class UnoRadioButtonControl
//  ----------------------------------------------------
class UnoRadioButtonControl :   public UnoControlBase,
                                public ::com::sun::star::awt::XButton,
                                public ::com::sun::star::awt::XRadioButton,
                                public ::com::sun::star::awt::XItemListener,
                                public ::com::sun::star::awt::XLayoutConstrains
{
private:
    ItemListenerMultiplexer     maItemListeners;
    ActionListenerMultiplexer   maActionListeners;
    ::rtl::OUString             maActionCommand;

public:

                            UnoRadioButtonControl();
    ::rtl::OUString         GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException) { UnoControlBase::disposing( Source ); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XButton
    void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setActionCommand( const ::rtl::OUString& Command ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XRadioButton
    void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL getState(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setState( sal_Bool b ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLabel( const ::rtl::OUString& Label ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XItemListener
    void SAL_CALL itemStateChanged( const ::com::sun::star::awt::ItemEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoRadioButtonControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlRadioButton ) )

};

//  ----------------------------------------------------
//  class UnoControlCheckBoxModel
//  ----------------------------------------------------
class UnoControlCheckBoxModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
                        UnoControlCheckBoxModel();
                        UnoControlCheckBoxModel( const UnoControlCheckBoxModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlCheckBoxModel( *this ); }

    ::rtl::OUString     getServiceName() const;


    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlCheckBoxModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlCheckBoxModel ) )
};

//  ----------------------------------------------------
//  class UnoCheckBoxControl
//  ----------------------------------------------------
class UnoCheckBoxControl :  public UnoControlBase,
                            public ::com::sun::star::awt::XCheckBox,
                            public ::com::sun::star::awt::XItemListener,
                            public ::com::sun::star::awt::XLayoutConstrains
{
private:
    ItemListenerMultiplexer maItemListeners;
    ::rtl::OUString         maLabel;

public:

                            UnoCheckBoxControl();
                            ~UnoCheckBoxControl(){;}
    ::rtl::OUString         GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException) { UnoControlBase::disposing( Source ); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XCheckBox
    void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getState(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setState( sal_Int16 n ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLabel( const ::rtl::OUString& Label ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL enableTriState( sal_Bool b ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XItemListener
    void SAL_CALL itemStateChanged( const ::com::sun::star::awt::ItemEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoCheckBoxControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlCheckBox ) )

};

//  ----------------------------------------------------
//  class UnoControlFixedTextModel
//  ----------------------------------------------------
class UnoControlFixedTextModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
                        UnoControlFixedTextModel();
                        UnoControlFixedTextModel( const UnoControlFixedTextModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlFixedTextModel( *this ); }

    ::rtl::OUString     getServiceName() const;


    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlFixedTextModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFixedTextModel ) )

};

//  ----------------------------------------------------
//  class UnoFixedTextControl
//  ----------------------------------------------------
class UnoFixedTextControl : public UnoControlBase,
                            public ::com::sun::star::awt::XFixedText,
                            public ::com::sun::star::awt::XLayoutConstrains
{
public:
                        UnoFixedTextControl();
    ::rtl::OUString     GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XFixedText
    void SAL_CALL setText( const ::rtl::OUString& Text ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getText(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setAlignment( sal_Int16 nAlign ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getAlignment(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoFixedTextControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFixedText ) )

};

//  ----------------------------------------------------
//  class UnoControlGroupBoxModel
//  ----------------------------------------------------
class UnoControlGroupBoxModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
                        UnoControlGroupBoxModel();
                        UnoControlGroupBoxModel( const UnoControlGroupBoxModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlGroupBoxModel( *this ); }

    ::rtl::OUString     getServiceName() const;


    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlGroupBoxModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlGroupBoxModel ) )

};

//  ----------------------------------------------------
//  class UnoGroupBoxControl
//  ----------------------------------------------------
class UnoGroupBoxControl :  public UnoControlBase
{
public:
                        UnoGroupBoxControl();
    ::rtl::OUString     GetComponentServiceName();

    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoGroupBoxControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlGroupBox ) )

};

//  ----------------------------------------------------
//  class UnoControlListBoxModel
//  ----------------------------------------------------
class UnoControlListBoxModel :  public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
                        UnoControlListBoxModel();
                        UnoControlListBoxModel( const UnoControlListBoxModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlListBoxModel( *this ); }

    ::rtl::OUString     getServiceName() const;

    void                ImplPropertyChanged( sal_uInt16 nPropId );


    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlListBoxModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlListBoxModel ) )

};

//  ----------------------------------------------------
//  class UnoListBoxControl
//  ----------------------------------------------------
class UnoListBoxControl :   public UnoControlBase,
                            public ::com::sun::star::awt::XListBox,
                            public ::com::sun::star::awt::XItemListener,
                            public ::com::sun::star::awt::XLayoutConstrains,
                            public ::com::sun::star::awt::XTextLayoutConstrains
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;

public:

                        UnoListBoxControl();
    ::rtl::OUString     GetComponentServiceName();

    void                ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal );

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException) { UnoControlBase::disposing( Source ); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

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

    // ::com::sun::star::awt::XItemListener
    void SAL_CALL itemStateChanged( const ::com::sun::star::awt::ItemEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoListBoxControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlListBox ) )

};

//  ----------------------------------------------------
//  class UnoControlComboBoxModel
//  ----------------------------------------------------
class UnoControlComboBoxModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
                        UnoControlComboBoxModel();
                        UnoControlComboBoxModel( const UnoControlComboBoxModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlComboBoxModel( *this ); }

    ::rtl::OUString     getServiceName() const;


    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlComboBoxModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlComboBoxModel ) )

};

//  ----------------------------------------------------
//  class UnoComboBoxControl
//  ----------------------------------------------------
class UnoComboBoxControl :  public UnoEditControl,
                            public ::com::sun::star::awt::XComboBox
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;

public:

                UnoComboBoxControl();
    ::rtl::OUString     GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoEditControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

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

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoComboBoxControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlComboBox ) )

};

//  ----------------------------------------------------
//  class UnoControlDateFieldModel
//  ----------------------------------------------------
class UnoControlDateFieldModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
                UnoControlDateFieldModel();
                UnoControlDateFieldModel( const UnoControlDateFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlDateFieldModel( *this ); }

    ::rtl::OUString     getServiceName() const;


    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlDateFieldModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlDateFieldModel ) )

};

//  ----------------------------------------------------
//  class UnoDateFieldControl
//  ----------------------------------------------------
class UnoDateFieldControl : public UnoEditControl,
                            public ::com::sun::star::awt::XDateField
{
public:
                    UnoDateFieldControl();
    ::rtl::OUString         GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoEditControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextListener
    void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);

    //XDateField
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

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoDateFieldControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlDateField ) )
};

//  ----------------------------------------------------
//  class UnoControlTimeFieldModel
//  ----------------------------------------------------
class UnoControlTimeFieldModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
                        UnoControlTimeFieldModel();
                        UnoControlTimeFieldModel( const UnoControlTimeFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlTimeFieldModel( *this ); }

    ::rtl::OUString     getServiceName() const;


    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlTimeFieldModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlTimeFieldModel ) )

};

//  ----------------------------------------------------
//  class UnoTimeFieldControl
//  ----------------------------------------------------
class UnoTimeFieldControl : public UnoEditControl,
                            public ::com::sun::star::awt::XTimeField
{
public:
                        UnoTimeFieldControl();
    ::rtl::OUString     GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoEditControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextListener
    void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);

    //XTimeField
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

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoTimeFieldControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlTimeField ) )

};

//  ----------------------------------------------------
//  class UnoControlNumericFieldModel
//  ----------------------------------------------------
class UnoControlNumericFieldModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
                UnoControlNumericFieldModel();
                UnoControlNumericFieldModel( const UnoControlNumericFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlNumericFieldModel( *this ); }

    ::rtl::OUString     getServiceName() const;


    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlNumericFieldModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlNumericFieldModel ) )

};

//  ----------------------------------------------------
//  class UnoNumericFieldControl
//  ----------------------------------------------------
class UnoNumericFieldControl :  public UnoEditControl,
                                public ::com::sun::star::awt::XNumericField
{
public:
                        UnoNumericFieldControl();
    ::rtl::OUString     GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoEditControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextListener
    void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);

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

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoNumericFieldControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlNumericField ) )

};

//  ----------------------------------------------------
//  class UnoControlCurrencyFieldModel
//  ----------------------------------------------------
class UnoControlCurrencyFieldModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
                        UnoControlCurrencyFieldModel();
                        UnoControlCurrencyFieldModel( const UnoControlCurrencyFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlCurrencyFieldModel( *this ); }

    ::rtl::OUString     getServiceName() const;


    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlCurrencyFieldModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlCurrencyFieldModel ) )

};

//  ----------------------------------------------------
//  class UnoCurrencyFieldControl
//  ----------------------------------------------------
class UnoCurrencyFieldControl : public UnoEditControl,
                                public ::com::sun::star::awt::XCurrencyField
{
public:
                        UnoCurrencyFieldControl();
    ::rtl::OUString     GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoEditControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextListener
    void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);

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

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoCurrencyFieldControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlCurrencyField ) )
};

//  ----------------------------------------------------
//  class UnoControlPatternFieldModel
//  ----------------------------------------------------
class UnoControlPatternFieldModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
                        UnoControlPatternFieldModel();
                        UnoControlPatternFieldModel( const UnoControlPatternFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlPatternFieldModel( *this ); }

    ::rtl::OUString     getServiceName() const;


    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlPatternFieldModel, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlPatternFieldModel ) )

};

//  ----------------------------------------------------
//  class UnoPatternFieldControl
//  ----------------------------------------------------
class UnoPatternFieldControl :  public UnoEditControl,
                                public ::com::sun::star::awt::XPatternField
{
protected:
    void            ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal );

public:
                        UnoPatternFieldControl();
    ::rtl::OUString     GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoEditControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }

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

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoPatternFieldControl, ::rtl::OUString::createFromAscii( szServiceName2_UnoControlPatternField ) )

};



#endif // _TOOLKIT_HELPER_UNOCONTROLS_HXX_

