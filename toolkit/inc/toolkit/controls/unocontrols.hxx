/*************************************************************************
 *
 *  $RCSfile: unocontrols.hxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:02:56 $
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
#ifndef _COM_SUN_STAR_AWT_XSPINFIELD_HPP_
#include <com/sun/star/awt/XSpinField.hpp>
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
#ifndef _COM_SUN_STAR_AWT_XPROGRESSBAR_HPP_
#include <com/sun/star/awt/XProgressBar.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XADJUSTMENTLISTENER_HPP_
#include <com/sun/star/awt/XAdjustmentListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_ADJUSTMENTTYPE_HPP_
#include <com/sun/star/awt/AdjustmentType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XSCROLLBAR_HPP_
#include <com/sun/star/awt/XScrollBar.hpp>
#endif

#ifndef _TOOLKIT_AWT_UNOCONTROLMODEL_HXX_
#include <toolkit/controls/unocontrolmodel.hxx>
#endif
#ifndef _TOOLKIT_AWT_UNOCONTROLBASE_HXX_
#include <toolkit/controls/unocontrolbase.hxx>
#endif
#ifndef _TOOLKIT_HELPER_MACROS_HXX_
#include <toolkit/helper/macros.hxx>
#endif
#ifndef _TOOLKIT_HELPER_SERVICENAMES_HXX_
#include <toolkit/helper/servicenames.hxx>
#endif

#ifndef _IMGCONS_HXX
#include <vcl/imgcons.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif

#include <list>


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

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);


    // XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlEditModel, UnoControlModel, szServiceName2_UnoControlEditModel )
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

    // Not all fields derived from UnoEditCOntrol have the property "Text"
    // They only support ::com::sun::star::awt::XTextComponent, so keep the text
    // here, maybe there is no Peer when calling setText()...
    ::rtl::OUString     maText;
    BOOL                mbSetTextInPeer;

    USHORT              mnMaxTextLen;
    BOOL                mbSetMaxTextLenInPeer;

public:

                                UnoEditControl();
    ::rtl::OUString             GetComponentServiceName();
    TextListenerMultiplexer&    GetTextListeners()  { return maTextListeners; }

    void                        ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal );

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }
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
    DECLIMPL_SERVICEINFO_DERIVED( UnoEditControl, UnoControlBase, szServiceName2_UnoControlEdit )
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

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlFileControlModel, UnoControlModel, szServiceName2_UnoControlFileControlModel )
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
    DECLIMPL_SERVICEINFO_DERIVED( UnoFileControl, UnoEditControl, szServiceName2_UnoControlFileControl )
};

//  ----------------------------------------------------
//  class UnoControlButtonModel
//  ----------------------------------------------------
class UnoControlButtonModel :   public ::com::sun::star::awt::XImageProducer,
                                public UnoControlModel
{
private:
    std::list< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer > > maListeners;

protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

public:
                        UnoControlButtonModel();
                        UnoControlButtonModel( const UnoControlButtonModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlButtonModel( *this ); }

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlModel::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

    // ::com::sun::star::awt::XImageProducer
    void SAL_CALL addConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL startProduction(  ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlButtonModel, UnoControlModel, szServiceName2_UnoControlButtonModel )
};

//  ----------------------------------------------------
//  class UnoButtonControl
//  ----------------------------------------------------
class UnoButtonControl :    public UnoControlBase,
                            public ::com::sun::star::awt::XButton,
                            public ::com::sun::star::awt::XLayoutConstrains
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ::rtl::OUString             maActionCommand;

public:

                        UnoButtonControl();
    ::rtl::OUString     GetComponentServiceName();
    void                        ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal );

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }
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

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoButtonControl, UnoControlBase, szServiceName2_UnoControlButton )

};

//  ----------------------------------------------------
//  class UnoControlImageControlModel
//  ----------------------------------------------------
class UnoControlImageControlModel : public ::com::sun::star::awt::XImageProducer,
                                    public UnoControlModel
{
private:
    std::list< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer > > maListeners;

protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

public:
                                    UnoControlImageControlModel();
                                    UnoControlImageControlModel( const UnoControlImageControlModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlImageControlModel( *this ); }

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlModel::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

    // ::com::sun::star::awt::XImageProducer
    void SAL_CALL addConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL startProduction(  ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlImageControlModel, UnoControlModel, szServiceName2_UnoControlImageControlModel )
};

//  ----------------------------------------------------
//  class UnoImageControlControl
//  ----------------------------------------------------
class UnoImageControlControl :  public UnoControlBase,
                                public ::com::sun::star::awt::XLayoutConstrains
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ::rtl::OUString             maActionCommand;

public:

                            UnoImageControlControl();
    ::rtl::OUString         GetComponentServiceName();

    void                    ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal );

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }
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

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoImageControlControl, UnoControlBase, szServiceName2_UnoControlImageControl )

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

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlRadioButtonModel, UnoControlModel, szServiceName2_UnoControlRadioButtonModel )

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
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }
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
    DECLIMPL_SERVICEINFO_DERIVED( UnoRadioButtonControl, UnoControlBase, szServiceName2_UnoControlRadioButton )

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

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlCheckBoxModel, UnoControlModel, szServiceName2_UnoControlCheckBoxModel )
};

//  ----------------------------------------------------
//  class UnoCheckBoxControl
//  ----------------------------------------------------
class UnoCheckBoxControl :  public UnoControlBase,
                            public ::com::sun::star::awt::XButton,
                            public ::com::sun::star::awt::XCheckBox,
                            public ::com::sun::star::awt::XItemListener,
                            public ::com::sun::star::awt::XLayoutConstrains
{
private:
    ItemListenerMultiplexer     maItemListeners;
    ActionListenerMultiplexer   maActionListeners;
    ::rtl::OUString             maActionCommand;

public:

                            UnoCheckBoxControl();
                            ~UnoCheckBoxControl(){;}
    ::rtl::OUString         GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }
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
    DECLIMPL_SERVICEINFO_DERIVED( UnoCheckBoxControl, UnoControlBase, szServiceName2_UnoControlCheckBox )

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

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlFixedTextModel, UnoControlModel, szServiceName2_UnoControlFixedTextModel )

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
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

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
    DECLIMPL_SERVICEINFO_DERIVED( UnoFixedTextControl, UnoControlBase, szServiceName2_UnoControlFixedText )

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

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlGroupBoxModel, UnoControlModel, szServiceName2_UnoControlGroupBoxModel )

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
    DECLIMPL_SERVICEINFO_DERIVED( UnoGroupBoxControl, UnoControlBase, szServiceName2_UnoControlGroupBox )

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

    void                ImplPropertyChanged( sal_uInt16 nPropId );
    virtual void        ImplNormalizePropertySequence(
                            const sal_Int32                 _nCount,        /// the number of entries in the arrays
                            sal_Int32*                      _pHandles,      /// the handles of the properties to set
                            ::com::sun::star::uno::Any*     _pValues,       /// the values of the properties to set
                            sal_Int32*                      _pValidHandles  /// pointer to the valid handles, allowed to be adjusted
                        )   const SAL_THROW(());

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlListBoxModel, UnoControlModel, szServiceName2_UnoControlListBoxModel )

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

    void                ImplUpdateSelectedItemsProperty();
    void                ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal );

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }
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
    DECLIMPL_SERVICEINFO_DERIVED( UnoListBoxControl, UnoControlBase, szServiceName2_UnoControlListBox )

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

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlComboBoxModel, UnoControlModel, szServiceName2_UnoControlComboBoxModel )

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
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }
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
    DECLIMPL_SERVICEINFO_DERIVED( UnoComboBoxControl, UnoEditControl, szServiceName2_UnoControlComboBox )

};

//  ----------------------------------------------------
//  class UnoSpinFieldControl
//  ----------------------------------------------------
class UnoSpinFieldControl : public UnoEditControl,
                            public ::com::sun::star::awt::XSpinField
{
private:
    SpinListenerMultiplexer     maSpinListeners;
    BOOL                        mbRepeat;

public:
                                UnoSpinFieldControl();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoEditControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XSpinField
    void SAL_CALL addSpinListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeSpinListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL up() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL down() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL first() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL last() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL enableRepeat( sal_Bool bRepeat ) throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::lang::XServiceInfo
    // No service info, only base class for other fields.
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

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlDateFieldModel, UnoControlModel, szServiceName2_UnoControlDateFieldModel )

};

//  ----------------------------------------------------
//  class UnoDateFieldControl
//  ----------------------------------------------------
class UnoDateFieldControl : public UnoSpinFieldControl,
                            public ::com::sun::star::awt::XDateField
{
private:
    sal_Int32       mnFirst;
    sal_Int32       mnLast;
    sal_Bool        mbLongFormat;
public:
                    UnoDateFieldControl();
    ::rtl::OUString         GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoSpinFieldControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);

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
    DECLIMPL_SERVICEINFO_DERIVED( UnoDateFieldControl, UnoSpinFieldControl, szServiceName2_UnoControlDateField )
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

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlTimeFieldModel, UnoControlModel, szServiceName2_UnoControlTimeFieldModel )

};

//  ----------------------------------------------------
//  class UnoTimeFieldControl
//  ----------------------------------------------------
class UnoTimeFieldControl : public UnoSpinFieldControl,
                            public ::com::sun::star::awt::XTimeField
{
private:
    sal_Int32       mnFirst;
    sal_Int32       mnLast;

public:
                        UnoTimeFieldControl();
    ::rtl::OUString     GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoSpinFieldControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);

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
    DECLIMPL_SERVICEINFO_DERIVED( UnoTimeFieldControl, UnoSpinFieldControl, szServiceName2_UnoControlTimeField )

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

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlNumericFieldModel, UnoControlModel, szServiceName2_UnoControlNumericFieldModel )

};

//  ----------------------------------------------------
//  class UnoNumericFieldControl
//  ----------------------------------------------------
class UnoNumericFieldControl :  public UnoSpinFieldControl,
                                public ::com::sun::star::awt::XNumericField
{
private:
    double mnFirst;
    double mnLast;

public:
                        UnoNumericFieldControl();
    ::rtl::OUString     GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoSpinFieldControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);

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
    DECLIMPL_SERVICEINFO_DERIVED( UnoNumericFieldControl, UnoSpinFieldControl, szServiceName2_UnoControlNumericField )

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

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlCurrencyFieldModel, UnoControlModel, szServiceName2_UnoControlCurrencyFieldModel )

};

//  ----------------------------------------------------
//  class UnoCurrencyFieldControl
//  ----------------------------------------------------
class UnoCurrencyFieldControl : public UnoSpinFieldControl,
                                public ::com::sun::star::awt::XCurrencyField
{
private:
    double mnFirst;
    double mnLast;

public:
                        UnoCurrencyFieldControl();
    ::rtl::OUString     GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoSpinFieldControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);

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
    DECLIMPL_SERVICEINFO_DERIVED( UnoCurrencyFieldControl, UnoSpinFieldControl, szServiceName2_UnoControlCurrencyField )
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

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlPatternFieldModel, UnoControlModel, szServiceName2_UnoControlPatternFieldModel )

};

//  ----------------------------------------------------
//  class UnoPatternFieldControl
//  ----------------------------------------------------
class UnoPatternFieldControl :  public UnoSpinFieldControl,
                                public ::com::sun::star::awt::XPatternField
{
protected:
    void            ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal );

public:
                        UnoPatternFieldControl();
    ::rtl::OUString     GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoSpinFieldControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

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
    DECLIMPL_SERVICEINFO_DERIVED( UnoPatternFieldControl, UnoSpinFieldControl, szServiceName2_UnoControlPatternField )

};

//  ----------------------------------------------------
//  class UnoControlProgressBarModel
//  ----------------------------------------------------
class UnoControlProgressBarModel :  public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
                        UnoControlProgressBarModel();
                        UnoControlProgressBarModel( const UnoControlProgressBarModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlProgressBarModel( *this ); }

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlProgressBarModel, UnoControlModel, szServiceName2_UnoControlProgressBarModel )
};

//  ----------------------------------------------------
//  class UnoProgressBarControl
//  ----------------------------------------------------
class UnoProgressBarControl :   public UnoControlBase,
                                public ::com::sun::star::awt::XProgressBar
{
public:
                                UnoProgressBarControl();
    ::rtl::OUString             GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XProgressBar
    void SAL_CALL setForegroundColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setBackgroundColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setValue( sal_Int32 nValue ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setRange( sal_Int32 nMin, sal_Int32 nMax ) throw(::com::sun::star::uno::RuntimeException );
    sal_Int32 SAL_CALL getValue() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoProgressBarControl, UnoControlBase, szServiceName2_UnoControlProgressBar )
};


//  ----------------------------------------------------
//  class UnoControlScrollBarModel
//  ----------------------------------------------------
class UnoControlScrollBarModel :    public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
                        UnoControlScrollBarModel();
                        UnoControlScrollBarModel( const UnoControlScrollBarModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlScrollBarModel( *this ); }

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlScrollBarModel, UnoControlModel, szServiceName2_UnoControlScrollBarModel )
};

//  ----------------------------------------------------
//  class UnoScrollBarControl
//  ----------------------------------------------------
class UnoScrollBarControl : public UnoControlBase,
                            public ::com::sun::star::awt::XAdjustmentListener,
                            public ::com::sun::star::awt::XScrollBar
{
private:
    AdjustmentListenerMultiplexer maAdjustmentListeners;

public:
                                UnoScrollBarControl();
    ::rtl::OUString             GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException) { UnoControlBase::disposing( Source ); }
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XAdjustmentListener
    void SAL_CALL adjustmentValueChanged( const ::com::sun::star::awt::AdjustmentEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XScrollBar
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

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoScrollBarControl, UnoControlBase, szServiceName2_UnoControlScrollBar )
};


//  ----------------------------------------------------
//  class UnoControlFixedLineModel
//  ----------------------------------------------------
class UnoControlFixedLineModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
                        UnoControlFixedLineModel();
                        UnoControlFixedLineModel( const UnoControlFixedLineModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlFixedLineModel( *this ); }

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlFixedLineModel, UnoControlModel, szServiceName2_UnoControlFixedLineModel )

};

//  ----------------------------------------------------
//  class UnoFixedLineControl
//  ----------------------------------------------------
class UnoFixedLineControl : public UnoControlBase
{
public:
                        UnoFixedLineControl();
    ::rtl::OUString     GetComponentServiceName();

    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoFixedLineControl, UnoControlBase, szServiceName2_UnoControlFixedLine )

};


#endif // _TOOLKIT_HELPER_UNOCONTROLS_HXX_

