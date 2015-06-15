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

#ifndef INCLUDED_TOOLKIT_CONTROLS_UNOCONTROLS_HXX
#define INCLUDED_TOOLKIT_CONTROLS_UNOCONTROLS_HXX

#include <toolkit/dllapi.h>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XTextListener.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XToggleButton.hpp>
#include <com/sun/star/awt/XRadioButton.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XFixedHyperlink.hpp>
#include <com/sun/star/awt/XFixedText.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XComboBox.hpp>
#include <com/sun/star/awt/XDateField.hpp>
#include <com/sun/star/awt/XSpinField.hpp>
#include <com/sun/star/awt/XTimeField.hpp>
#include <com/sun/star/awt/XNumericField.hpp>
#include <com/sun/star/awt/XCurrencyField.hpp>
#include <com/sun/star/awt/XPatternField.hpp>
#include <com/sun/star/awt/XProgressBar.hpp>
#include <com/sun/star/awt/XItemList.hpp>
#include <com/sun/star/graphic/XGraphicObject.hpp>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <vcl/bitmapex.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>

#include <list>
#include <memory>
#include <vector>

#include <boost/optional.hpp>

#define UNO_NAME_GRAPHOBJ_URLPREFIX                             "vnd.sun.star.GraphicObject:"

class ImageHelper
{
public:
    // The routine will always attempt to return a valid XGraphic for the
    // passed _rURL, additionally xOutGraphicObject will contain the
    // associated XGraphicObject ( if url is valid for that ) and is set
    // appropriately ( e.g. NULL if non GraphicObject scheme ) or a valid
    // object if the rURL points to a valid object
    static ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > getGraphicAndGraphicObjectFromURL_nothrow( ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphicObject >& xOutGraphicObject, const OUString& _rURL );
    static ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > getGraphicFromURL_nothrow( const OUString& _rURL );

};


//  class UnoControlEditModel

class UnoControlEditModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                        UnoControlEditModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                        UnoControlEditModel( const UnoControlEditModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlEditModel( *this ); }

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoEditControl

typedef ::cppu::ImplHelper4  <   ::com::sun::star::awt::XTextComponent
                             ,   ::com::sun::star::awt::XTextListener
                             ,   ::com::sun::star::awt::XLayoutConstrains
                             ,   ::com::sun::star::awt::XTextLayoutConstrains
                             >   UnoEditControl_Base;
class TOOLKIT_DLLPUBLIC UnoEditControl    :public UnoControlBase
                                            ,public UnoEditControl_Base
{
private:
    TextListenerMultiplexer maTextListeners;

    // Not all fields derived from UnoEditCOntrol have the property "Text"
    // They only support XTextComponent, so keep the text
    // here, maybe there is no Peer when calling setText()...
    OUString     maText;
    sal_uInt16              mnMaxTextLen;

    bool            mbSetTextInPeer;
    bool            mbSetMaxTextLenInPeer;
    bool            mbHasTextProperty;

public:

                                UnoEditControl();
    OUString             GetComponentServiceName() SAL_OVERRIDE;
    TextListenerMultiplexer&    GetTextListeners()  { return maTextListeners; }

    void                        ImplSetPeerProperty( const OUString& rPropName, const ::com::sun::star::uno::Any& rVal ) SAL_OVERRIDE;

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { UnoControlBase::disposing( Source ); }
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // disambiguate XInterface
    DECLARE_XINTERFACE()

    // XAggregation
    ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XTextListener
    void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XTextComponent
    void SAL_CALL addTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL removeTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setText( const OUString& aText ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL insertText( const ::com::sun::star::awt::Selection& Sel, const OUString& Text ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    OUString SAL_CALL getText(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    OUString SAL_CALL getSelectedText(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Selection SAL_CALL getSelection(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Bool SAL_CALL isEditable(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setEditable( sal_Bool bEditable ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setMaxTextLen( sal_Int16 nLen ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Int16 SAL_CALL getMaxTextLen(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XTextLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    sal_Bool SAL_CALL setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
};


//  class UnoControlFileControlModel

class UnoControlFileControlModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                        UnoControlFileControlModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                        UnoControlFileControlModel( const UnoControlFileControlModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlFileControlModel( *this ); }

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoFileControl

class UnoFileControl : public UnoEditControl
{
public:
                        UnoFileControl();
    OUString     GetComponentServiceName() SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class GraphicControlModel

class GraphicControlModel : public UnoControlModel
{
private:
    bool                                                                                    mbAdjustingImagePosition;
    bool                                                                                    mbAdjustingGraphic;

    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphicObject > mxGrfObj;

protected:
    GraphicControlModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext )
        :UnoControlModel( rxContext )
        ,mbAdjustingImagePosition( false )
        ,mbAdjustingGraphic( false )
    {
    }
    GraphicControlModel( const GraphicControlModel& _rSource ) : UnoControlModel( _rSource ), mbAdjustingImagePosition( false ), mbAdjustingGraphic( false ) { }

    // ::cppu::OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

    // UnoControlModel
    ::com::sun::star::uno::Any ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;

private:
        GraphicControlModel& operator=( const GraphicControlModel& ) SAL_DELETED_FUNCTION;
};


//  class UnoControlButtonModel

class UnoControlButtonModel : public GraphicControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                        UnoControlButtonModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                        UnoControlButtonModel( const UnoControlButtonModel& rModel ) : GraphicControlModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlButtonModel( *this ); }

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoButtonControl

typedef ::cppu::AggImplInheritanceHelper4   <   UnoControlBase
                                            ,   ::com::sun::star::awt::XButton
                                            ,   ::com::sun::star::awt::XToggleButton
                                            ,   ::com::sun::star::awt::XLayoutConstrains
                                            ,   ::com::sun::star::awt::XItemListener
                                            >   UnoButtonControl_Base;
class UnoButtonControl :    public UnoButtonControl_Base
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;
    OUString             maActionCommand;

public:

                        UnoButtonControl();
    OUString     GetComponentServiceName() SAL_OVERRIDE;

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XButton
    void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setLabel( const OUString& Label ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setActionCommand( const OUString& Command ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XToggleButton
    // ::com::sun::star::awt::XItemEventBroadcaster
    void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XItemListener
    virtual void SAL_CALL itemStateChanged( const ::com::sun::star::awt::ItemEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoControlImageControlModel

class UnoControlImageControlModel : public GraphicControlModel
{
private:
    bool    mbAdjustingImageScaleMode;

protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                                    UnoControlImageControlModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                                    UnoControlImageControlModel( const UnoControlImageControlModel& rModel ) : GraphicControlModel( rModel ), mbAdjustingImageScaleMode( false ) { }

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlImageControlModel( *this ); }

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::cppu::OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;
};


//  class UnoImageControlControl

typedef ::cppu::AggImplInheritanceHelper1   <   UnoControlBase
                                            ,   ::com::sun::star::awt::XLayoutConstrains
                                            >   UnoImageControlControl_Base;
class UnoImageControlControl : public UnoImageControlControl_Base
{
private:
    ActionListenerMultiplexer   maActionListeners;

public:

                            UnoImageControlControl();
    OUString         GetComponentServiceName() SAL_OVERRIDE;

    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoControlRadioButtonModel

class UnoControlRadioButtonModel :  public GraphicControlModel

{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                        UnoControlRadioButtonModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                        UnoControlRadioButtonModel( const UnoControlRadioButtonModel& rModel ) : GraphicControlModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlRadioButtonModel( *this ); }

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoRadioButtonControl

typedef ::cppu::AggImplInheritanceHelper4   <   UnoControlBase
                                            ,   ::com::sun::star::awt::XButton
                                            ,   ::com::sun::star::awt::XRadioButton
                                            ,   ::com::sun::star::awt::XItemListener
                                            ,   ::com::sun::star::awt::XLayoutConstrains
                                            >   UnoRadioButtonControl_Base;
class UnoRadioButtonControl : public UnoRadioButtonControl_Base
{
private:
    ItemListenerMultiplexer     maItemListeners;
    ActionListenerMultiplexer   maActionListeners;
    OUString             maActionCommand;

public:

                            UnoRadioButtonControl();
    OUString         GetComponentServiceName() SAL_OVERRIDE;

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { UnoControlBase::disposing( Source ); }

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XButton
    void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setActionCommand( const OUString& Command ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XRadioButton
    void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Bool SAL_CALL getState(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setState( sal_Bool b ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setLabel( const OUString& Label ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XItemListener
    void SAL_CALL itemStateChanged( const ::com::sun::star::awt::ItemEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoControlCheckBoxModel

class UnoControlCheckBoxModel : public GraphicControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                        UnoControlCheckBoxModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                        UnoControlCheckBoxModel( const UnoControlCheckBoxModel& rModel ) : GraphicControlModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlCheckBoxModel( *this ); }

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoCheckBoxControl

typedef ::cppu::AggImplInheritanceHelper4   <   UnoControlBase
                                            ,   ::com::sun::star::awt::XButton
                                            ,   ::com::sun::star::awt::XCheckBox
                                            ,   ::com::sun::star::awt::XItemListener
                                            ,   ::com::sun::star::awt::XLayoutConstrains
                                            >   UnoCheckBoxControl_Base;
class UnoCheckBoxControl : public UnoCheckBoxControl_Base
{
private:
    ItemListenerMultiplexer     maItemListeners;
    ActionListenerMultiplexer   maActionListeners;
    OUString             maActionCommand;

public:

                            UnoCheckBoxControl();
                            virtual ~UnoCheckBoxControl(){;}
    OUString         GetComponentServiceName() SAL_OVERRIDE;

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { UnoControlBase::disposing( Source ); }

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XButton
    void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setActionCommand( const OUString& Command ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


    sal_Int16 SAL_CALL getState(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setState( sal_Int16 n ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setLabel( const OUString& Label ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL enableTriState( sal_Bool b ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XItemListener
    void SAL_CALL itemStateChanged( const ::com::sun::star::awt::ItemEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoControlFixedTextModel

class UnoControlFixedHyperlinkModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
    UnoControlFixedHyperlinkModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    UnoControlFixedHyperlinkModel( const UnoControlFixedHyperlinkModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlFixedHyperlinkModel( *this ); }

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlFixedHyperlinkModel, UnoControlModel, "com.sun.star.awt.UnoControlFixedHyperlinkModel" )
};


//  class UnoFixedHyperlinkControl

class UnoFixedHyperlinkControl : public UnoControlBase,
                                 public ::com::sun::star::awt::XFixedHyperlink,
                                 public ::com::sun::star::awt::XLayoutConstrains
{
private:
    ActionListenerMultiplexer   maActionListeners;

public:
    UnoFixedHyperlinkControl();

    OUString     GetComponentServiceName() SAL_OVERRIDE;

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void                        SAL_CALL acquire() throw() SAL_OVERRIDE  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() SAL_OVERRIDE  { OWeakAggObject::release(); }

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XFixedHyperlink
    void SAL_CALL setText( const OUString& Text ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    OUString SAL_CALL getText(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setURL( const OUString& URL ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    OUString SAL_CALL getURL(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setAlignment( sal_Int16 nAlign ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Int16 SAL_CALL getAlignment(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoFixedHyperlinkControl, UnoControlBase, "com.sun.star.awt.UnoControlFixedHyperlink" )
};


//  class UnoControlFixedTextModel

class UnoControlFixedTextModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                        UnoControlFixedTextModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                        UnoControlFixedTextModel( const UnoControlFixedTextModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlFixedTextModel( *this ); }

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoFixedTextControl

class UnoFixedTextControl : public UnoControlBase,
                            public ::com::sun::star::awt::XFixedText,
                            public ::com::sun::star::awt::XLayoutConstrains
{
public:
                        UnoFixedTextControl();
    OUString     GetComponentServiceName() SAL_OVERRIDE;

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void                        SAL_CALL acquire() throw() SAL_OVERRIDE  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() SAL_OVERRIDE  { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XFixedText
    void SAL_CALL setText( const OUString& Text ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    OUString SAL_CALL getText(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setAlignment( sal_Int16 nAlign ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Int16 SAL_CALL getAlignment(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoControlGroupBoxModel

class UnoControlGroupBoxModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                        UnoControlGroupBoxModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                        UnoControlGroupBoxModel( const UnoControlGroupBoxModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlGroupBoxModel( *this ); }

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoGroupBoxControl

class UnoGroupBoxControl :  public UnoControlBase
{
public:
                        UnoGroupBoxControl();
    OUString     GetComponentServiceName() SAL_OVERRIDE;

    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoControlListBoxModel

struct UnoControlListBoxModel_Data;
typedef ::cppu::AggImplInheritanceHelper1   <   UnoControlModel
                                            ,   ::com::sun::star::awt::XItemList
                                            >   UnoControlListBoxModel_Base;
class TOOLKIT_DLLPUBLIC UnoControlListBoxModel : public UnoControlListBoxModel_Base
{
protected:
    enum ConstructorMode
    {
        ConstructDefault,
        ConstructWithoutProperties
    };

public:
                        UnoControlListBoxModel(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            ConstructorMode const i_mode = ConstructDefault
                        );
                        UnoControlListBoxModel( const UnoControlListBoxModel& i_rSource );
                        virtual ~UnoControlListBoxModel();

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlListBoxModel( *this ); }

    virtual void        ImplNormalizePropertySequence(
                            const sal_Int32                 _nCount,        /// the number of entries in the arrays
                            sal_Int32*                      _pHandles,      /// the handles of the properties to set
                            ::com::sun::star::uno::Any*     _pValues,       /// the values of the properties to set
                            sal_Int32*                      _pValidHandles  /// pointer to the valid handles, allowed to be adjusted
                        )   const SAL_OVERRIDE;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XItemList
    virtual ::sal_Int32 SAL_CALL getItemCount() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL insertItem( ::sal_Int32 Position, const OUString& ItemText, const OUString& ItemImageURL ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL insertItemText( ::sal_Int32 Position, const OUString& ItemText ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL insertItemImage( ::sal_Int32 Position, const OUString& ItemImageURL ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeItem( ::sal_Int32 Position ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeAllItems(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setItemText( ::sal_Int32 Position, const OUString& ItemText ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setItemImage( ::sal_Int32 Position, const OUString& ItemImageURL ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setItemTextAndImage( ::sal_Int32 Position, const OUString& ItemText, const OUString& ItemImageURL ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setItemData( ::sal_Int32 Position, const ::com::sun::star::uno::Any& DataValue ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getItemText( ::sal_Int32 Position ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getItemImage( ::sal_Int32 Position ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::beans::Pair< OUString, OUString > SAL_CALL getItemTextAndImage( ::sal_Int32 Position ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getItemData( ::sal_Int32 Position ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Pair< OUString, OUString > > SAL_CALL getAllItems(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addItemListListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListListener >& Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeItemListListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListListener >& Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

private:
    void    impl_notifyItemListEvent_nolck(
                const sal_Int32 i_nItemPosition,
                const ::boost::optional< OUString >& i_rItemText,
                const ::boost::optional< OUString >& i_rItemImageURL,
                void ( SAL_CALL ::com::sun::star::awt::XItemListListener::*NotificationMethod )( const ::com::sun::star::awt::ItemListEvent& )
            );

    void    impl_handleInsert(
                const sal_Int32 i_nItemPosition,
                const ::boost::optional< OUString >& i_rItemText,
                const ::boost::optional< OUString >& i_rItemImageURL,
                ::osl::ClearableMutexGuard& i_rClearBeforeNotify
            );

    void    impl_handleRemove(
                const sal_Int32 i_nItemPosition,
                ::osl::ClearableMutexGuard& i_rClearBeforeNotify
            );

    void    impl_handleModify(
                const sal_Int32 i_nItemPosition,
                const ::boost::optional< OUString >& i_rItemText,
                const ::boost::optional< OUString >& i_rItemImageURL,
                ::osl::ClearableMutexGuard& i_rClearBeforeNotify
            );

    void    impl_getStringItemList( ::std::vector< OUString >& o_rStringItems ) const;
    void    impl_setStringItemList_nolck( const ::std::vector< OUString >& i_rStringItems );

protected:
    std::unique_ptr<UnoControlListBoxModel_Data>  m_xData;
    ::cppu::OInterfaceContainerHelper             m_aItemListListeners;
};


//  class UnoListBoxControl

typedef ::cppu::AggImplInheritanceHelper5   <   UnoControlBase
                                            ,   ::com::sun::star::awt::XListBox
                                            ,   ::com::sun::star::awt::XItemListener
                                            ,   ::com::sun::star::awt::XLayoutConstrains
                                            ,   ::com::sun::star::awt::XTextLayoutConstrains
                                            ,   ::com::sun::star::awt::XItemListListener
                                            >   UnoListBoxControl_Base;
class TOOLKIT_DLLPUBLIC UnoListBoxControl : public UnoListBoxControl_Base
{
public:
                        UnoListBoxControl();
    OUString     GetComponentServiceName() SAL_OVERRIDE;

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { UnoControlBase::disposing( Source ); }

    // ::com::sun::star::awt::XListBox
    void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL addItem( const OUString& aItem, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL addItems( const ::com::sun::star::uno::Sequence< OUString >& aItems, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Int16 SAL_CALL getItemCount(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    OUString SAL_CALL getItem( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getItems(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Int16 SAL_CALL getSelectedItemPos(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSelectedItemsPos(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    OUString SAL_CALL getSelectedItem(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSelectedItems(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL selectItemPos( sal_Int16 nPos, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL selectItemsPos( const ::com::sun::star::uno::Sequence< sal_Int16 >& aPositions, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL selectItem( const OUString& aItem, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Bool SAL_CALL isMutipleMode(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setMultipleMode( sal_Bool bMulti ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Int16 SAL_CALL getDropDownLineCount(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setDropDownLineCount( sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL makeVisible( sal_Int16 nEntry ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XItemListener
    void SAL_CALL itemStateChanged( const ::com::sun::star::awt::ItemEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XTextLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XUnoControl
    sal_Bool SAL_CALL setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XItemListListener
    virtual void SAL_CALL listItemInserted( const ::com::sun::star::awt::ItemListEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL listItemRemoved( const ::com::sun::star::awt::ItemListEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL listItemModified( const ::com::sun::star::awt::ItemListEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL allItemsRemoved( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL itemListChanged( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    void                ImplUpdateSelectedItemsProperty();
    virtual void        ImplSetPeerProperty( const OUString& rPropName, const ::com::sun::star::uno::Any& rVal ) SAL_OVERRIDE;
    virtual void        updateFromModel() SAL_OVERRIDE;

private:
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;
};


//  class UnoControlComboBoxModel

class UnoControlComboBoxModel : public UnoControlListBoxModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                        UnoControlComboBoxModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                        UnoControlComboBoxModel( const UnoControlComboBoxModel& rModel ) : UnoControlListBoxModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlComboBoxModel( *this ); }

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

};


//  class UnoComboBoxControl

class UnoComboBoxControl :  public UnoEditControl
                        ,   public ::com::sun::star::awt::XComboBox
                        ,   public ::com::sun::star::awt::XItemListener
                        ,   public ::com::sun::star::awt::XItemListListener
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;

public:

                        UnoComboBoxControl();
    OUString     GetComponentServiceName() SAL_OVERRIDE;

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { UnoEditControl::disposing( Source ); }
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { return UnoEditControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void                        SAL_CALL acquire() throw() SAL_OVERRIDE  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() SAL_OVERRIDE  { OWeakAggObject::release(); }


    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XComboBox
    void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL addItem( const OUString& aItem, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL addItems( const ::com::sun::star::uno::Sequence< OUString >& aItems, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Int16 SAL_CALL getItemCount(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    OUString SAL_CALL getItem( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getItems(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Int16 SAL_CALL getDropDownLineCount(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setDropDownLineCount( sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XUnoControl
    virtual sal_Bool SAL_CALL setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XItemListListener
    virtual void SAL_CALL listItemInserted( const ::com::sun::star::awt::ItemListEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL listItemRemoved( const ::com::sun::star::awt::ItemListEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL listItemModified( const ::com::sun::star::awt::ItemListEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL allItemsRemoved( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL itemListChanged( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XItemListener
    virtual void SAL_CALL itemStateChanged( const ::com::sun::star::awt::ItemEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
protected:
    virtual void        ImplSetPeerProperty( const OUString& rPropName, const ::com::sun::star::uno::Any& rVal ) SAL_OVERRIDE;
    virtual void        updateFromModel() SAL_OVERRIDE;
    ActionListenerMultiplexer&  getActionListeners();
    ItemListenerMultiplexer&    getItemListeners();

};


//  class UnoSpinFieldControl

class UnoSpinFieldControl : public UnoEditControl,
                            public ::com::sun::star::awt::XSpinField
{
private:
    SpinListenerMultiplexer     maSpinListeners;
    bool                        mbRepeat;

public:
                                UnoSpinFieldControl();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { return UnoEditControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void                        SAL_CALL acquire() throw() SAL_OVERRIDE  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() SAL_OVERRIDE  { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XSpinField
    void SAL_CALL addSpinListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL removeSpinListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL up() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL down() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL first() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL last() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL enableRepeat( sal_Bool bRepeat ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


    // ::com::sun::star::lang::XServiceInfo
    // No service info, only base class for other fields.
};



//  class UnoControlDateFieldModel

class UnoControlDateFieldModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                UnoControlDateFieldModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                UnoControlDateFieldModel( const UnoControlDateFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlDateFieldModel( *this ); }

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoDateFieldControl

class UnoDateFieldControl : public UnoSpinFieldControl,
                            public ::com::sun::star::awt::XDateField
{
private:
    ::com::sun::star::util::Date       mnFirst;
    ::com::sun::star::util::Date       mnLast;
    TriState         mbLongFormat;
public:
                            UnoDateFieldControl();
    OUString         GetComponentServiceName() SAL_OVERRIDE;

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { return UnoSpinFieldControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void                        SAL_CALL acquire() throw() SAL_OVERRIDE  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() SAL_OVERRIDE  { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XTextListener
    void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XDateField
    void SAL_CALL setDate( const ::com::sun::star::util::Date& Date ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::util::Date SAL_CALL getDate(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setMin( const ::com::sun::star::util::Date& Date ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::util::Date SAL_CALL getMin(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setMax( const ::com::sun::star::util::Date& Date ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::util::Date SAL_CALL getMax(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setFirst( const ::com::sun::star::util::Date& Date ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::util::Date SAL_CALL getFirst(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setLast( const ::com::sun::star::util::Date& Date ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::util::Date SAL_CALL getLast(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setLongFormat( sal_Bool bLong ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Bool SAL_CALL isLongFormat(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setEmpty(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Bool SAL_CALL isEmpty(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Bool SAL_CALL isStrictFormat(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoControlTimeFieldModel

class UnoControlTimeFieldModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                        UnoControlTimeFieldModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                        UnoControlTimeFieldModel( const UnoControlTimeFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlTimeFieldModel( *this ); }

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoTimeFieldControl

class UnoTimeFieldControl : public UnoSpinFieldControl,
                            public ::com::sun::star::awt::XTimeField
{
private:
    ::com::sun::star::util::Time       mnFirst;
    ::com::sun::star::util::Time       mnLast;

public:
                        UnoTimeFieldControl();
    OUString     GetComponentServiceName() SAL_OVERRIDE;

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { return UnoSpinFieldControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void                        SAL_CALL acquire() throw() SAL_OVERRIDE  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() SAL_OVERRIDE  { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XTextListener
    void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XTimeField
    void SAL_CALL setTime( const ::com::sun::star::util::Time& Time ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::util::Time SAL_CALL getTime(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setMin( const ::com::sun::star::util::Time& Time ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::util::Time SAL_CALL getMin(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setMax( const ::com::sun::star::util::Time& Time ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::util::Time SAL_CALL getMax(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setFirst( const ::com::sun::star::util::Time& Time ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::util::Time SAL_CALL getFirst(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setLast( const ::com::sun::star::util::Time& Time ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::util::Time SAL_CALL getLast(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setEmpty(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Bool SAL_CALL isEmpty(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Bool SAL_CALL isStrictFormat(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoControlNumericFieldModel

class UnoControlNumericFieldModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                UnoControlNumericFieldModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                UnoControlNumericFieldModel( const UnoControlNumericFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlNumericFieldModel( *this ); }

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoNumericFieldControl

class UnoNumericFieldControl :  public UnoSpinFieldControl,
                                public ::com::sun::star::awt::XNumericField
{
private:
    double mnFirst;
    double mnLast;

public:
                        UnoNumericFieldControl();
    OUString     GetComponentServiceName() SAL_OVERRIDE;

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { return UnoSpinFieldControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void                        SAL_CALL acquire() throw() SAL_OVERRIDE  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() SAL_OVERRIDE  { OWeakAggObject::release(); }

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XTextListener
    void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XNumericField
    void SAL_CALL setValue( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    double SAL_CALL getValue(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setMin( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    double SAL_CALL getMin(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setMax( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    double SAL_CALL getMax(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    double SAL_CALL getFirst(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setLast( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    double SAL_CALL getLast(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    double SAL_CALL getSpinSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setDecimalDigits( sal_Int16 nDigits ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Int16 SAL_CALL getDecimalDigits(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Bool SAL_CALL isStrictFormat(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoControlCurrencyFieldModel

class UnoControlCurrencyFieldModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                        UnoControlCurrencyFieldModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                        UnoControlCurrencyFieldModel( const UnoControlCurrencyFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlCurrencyFieldModel( *this ); }

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoCurrencyFieldControl

class UnoCurrencyFieldControl : public UnoSpinFieldControl,
                                public ::com::sun::star::awt::XCurrencyField
{
private:
    double mnFirst;
    double mnLast;

public:
                        UnoCurrencyFieldControl();
    OUString     GetComponentServiceName() SAL_OVERRIDE;

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { return UnoSpinFieldControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void                        SAL_CALL acquire() throw() SAL_OVERRIDE  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() SAL_OVERRIDE  { OWeakAggObject::release(); }

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XTextListener
    void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XCurrencyField
    void SAL_CALL setValue( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    double SAL_CALL getValue(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setMin( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    double SAL_CALL getMin(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setMax( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    double SAL_CALL getMax(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    double SAL_CALL getFirst(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setLast( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    double SAL_CALL getLast(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    double SAL_CALL getSpinSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setDecimalDigits( sal_Int16 nDigits ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Int16 SAL_CALL getDecimalDigits(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Bool SAL_CALL isStrictFormat(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoControlPatternFieldModel

class UnoControlPatternFieldModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                        UnoControlPatternFieldModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                        UnoControlPatternFieldModel( const UnoControlPatternFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlPatternFieldModel( *this ); }

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoPatternFieldControl

class UnoPatternFieldControl :  public UnoSpinFieldControl,
                                public ::com::sun::star::awt::XPatternField
{
protected:
    void            ImplSetPeerProperty( const OUString& rPropName, const ::com::sun::star::uno::Any& rVal ) SAL_OVERRIDE;

public:
                        UnoPatternFieldControl();
    OUString     GetComponentServiceName() SAL_OVERRIDE;

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { return UnoSpinFieldControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void                        SAL_CALL acquire() throw() SAL_OVERRIDE  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() SAL_OVERRIDE  { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XPatternField
    void SAL_CALL setMasks( const OUString& EditMask, const OUString& LiteralMask ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL getMasks( OUString& EditMask, OUString& LiteralMask ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setString( const OUString& Str ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    OUString SAL_CALL getString(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Bool SAL_CALL isStrictFormat(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoControlProgressBarModel

class UnoControlProgressBarModel :  public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                        UnoControlProgressBarModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                        UnoControlProgressBarModel( const UnoControlProgressBarModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlProgressBarModel( *this ); }

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoProgressBarControl

class UnoProgressBarControl :   public UnoControlBase,
                                public ::com::sun::star::awt::XProgressBar
{
public:
                                UnoProgressBarControl();
    OUString             GetComponentServiceName() SAL_OVERRIDE;

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { return UnoControlBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void                        SAL_CALL acquire() throw() SAL_OVERRIDE  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() SAL_OVERRIDE  { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XProgressBar
    void SAL_CALL setForegroundColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setBackgroundColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setValue( sal_Int32 nValue ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setRange( sal_Int32 nMin, sal_Int32 nMax ) throw(::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    sal_Int32 SAL_CALL getValue() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};



//  class UnoControlFixedLineModel

class UnoControlFixedLineModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
                        UnoControlFixedLineModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                        UnoControlFixedLineModel( const UnoControlFixedLineModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const SAL_OVERRIDE { return new UnoControlFixedLineModel( *this ); }

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


//  class UnoFixedLineControl

class UnoFixedLineControl : public UnoControlBase
{
public:
                        UnoFixedLineControl();
    OUString     GetComponentServiceName() SAL_OVERRIDE;

    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};



#endif // INCLUDED_TOOLKIT_CONTROLS_UNOCONTROLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
