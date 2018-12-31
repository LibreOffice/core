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
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <tools/gen.hxx>

#include <memory>
#include <vector>

namespace boost { template <class T> class optional; }
namespace com { namespace sun { namespace star { namespace graphic { class XGraphic; } } } }
namespace com { namespace sun { namespace star { namespace graphic { class XGraphicObject; } } } }

class ImageHelper
{
public:
    // The routine will always attempt to return a valid XGraphic for the
    // passed _rURL, additionally xOutGraphicObject will contain the
    // associated XGraphicObject ( if url is valid for that ) and is set
    // appropriately ( e.g. NULL if non GraphicObject scheme ) or a valid
    // object if the rURL points to a valid object
    static css::uno::Reference< css::graphic::XGraphic > getGraphicAndGraphicObjectFromURL_nothrow( css::uno::Reference< css::graphic::XGraphicObject >& xOutGraphicObject, const OUString& _rURL );
    static css::uno::Reference< css::graphic::XGraphic > getGraphicFromURL_nothrow( const OUString& _rURL );

};


//  class UnoControlEditModel

class UnoControlEditModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlEditModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlEditModel( const UnoControlEditModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlEditModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;


    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoEditControl

typedef ::cppu::ImplHelper4  <   css::awt::XTextComponent
                             ,   css::awt::XTextListener
                             ,   css::awt::XLayoutConstrains
                             ,   css::awt::XTextLayoutConstrains
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
    OUString             GetComponentServiceName() override;
    TextListenerMultiplexer&    GetTextListeners()  { return maTextListeners; }

    void                        ImplSetPeerProperty( const OUString& rPropName, const css::uno::Any& rVal ) override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void SAL_CALL disposing( const css::lang::EventObject& Source ) override { UnoControlBase::disposing( Source ); }
    void SAL_CALL dispose(  ) override;

    // disambiguate XInterface
    DECLARE_XINTERFACE()

    // XAggregation
    css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) override;

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XTextListener
    void SAL_CALL textChanged( const css::awt::TextEvent& rEvent ) override;

    // XTextComponent
    void SAL_CALL addTextListener( const css::uno::Reference< css::awt::XTextListener >& l ) override;
    void SAL_CALL removeTextListener( const css::uno::Reference< css::awt::XTextListener >& l ) override;
    void SAL_CALL setText( const OUString& aText ) override;
    void SAL_CALL insertText( const css::awt::Selection& Sel, const OUString& Text ) override;
    OUString SAL_CALL getText(  ) override;
    OUString SAL_CALL getSelectedText(  ) override;
    void SAL_CALL setSelection( const css::awt::Selection& aSelection ) override;
    css::awt::Selection SAL_CALL getSelection(  ) override;
    sal_Bool SAL_CALL isEditable(  ) override;
    void SAL_CALL setEditable( sal_Bool bEditable ) override;
    void SAL_CALL setMaxTextLen( sal_Int16 nLen ) override;
    sal_Int16 SAL_CALL getMaxTextLen(  ) override;

    // XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // XTextLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName(  ) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    sal_Bool SAL_CALL setModel(const css::uno::Reference< css::awt::XControlModel >& Model) override;
};


//  class UnoControlFileControlModel

class UnoControlFileControlModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

public:
                        UnoControlFileControlModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlFileControlModel( const UnoControlFileControlModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlFileControlModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoFileControl

class UnoFileControl : public UnoEditControl
{
public:
                        UnoFileControl();
    OUString     GetComponentServiceName() override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class GraphicControlModel

class GraphicControlModel : public UnoControlModel
{
private:
    bool                                                                                    mbAdjustingImagePosition;
    bool                                                                                    mbAdjustingGraphic;

protected:
    GraphicControlModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
        :UnoControlModel( rxContext )
        ,mbAdjustingImagePosition( false )
        ,mbAdjustingGraphic( false )
    {
    }
    GraphicControlModel( const GraphicControlModel& _rSource ) : UnoControlModel( _rSource ), mbAdjustingImagePosition( false ), mbAdjustingGraphic( false ) { }

    // ::cppu::OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) override;

    // UnoControlModel
    css::uno::Any ImplGetDefaultValue( sal_uInt16 nPropId ) const override;

private:
        GraphicControlModel& operator=( const GraphicControlModel& ) = delete;
};


//  class UnoControlButtonModel

class UnoControlButtonModel : public GraphicControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

public:
                        UnoControlButtonModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlButtonModel( const UnoControlButtonModel& rModel ) : GraphicControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlButtonModel( *this ); }

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoButtonControl

typedef ::cppu::AggImplInheritanceHelper4   <   UnoControlBase
                                            ,   css::awt::XButton
                                            ,   css::awt::XToggleButton
                                            ,   css::awt::XLayoutConstrains
                                            ,   css::awt::XItemListener
                                            >   UnoButtonControl_Base;
class UnoButtonControl :    public UnoButtonControl_Base
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;
    OUString             maActionCommand;

public:

                        UnoButtonControl();
    OUString     GetComponentServiceName() override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void SAL_CALL dispose(  ) override;

    // css::awt::XButton
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void SAL_CALL setLabel( const OUString& Label ) override;
    void SAL_CALL setActionCommand( const OUString& Command ) override;

    // css::awt::XToggleButton
    // css::awt::XItemEventBroadcaster
    void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;

    // css::lang::XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    // XItemListener
    virtual void SAL_CALL itemStateChanged( const css::awt::ItemEvent& rEvent ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoControlImageControlModel

class UnoControlImageControlModel : public GraphicControlModel
{
private:
    bool    mbAdjustingImageScaleMode;

protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

public:
                                    UnoControlImageControlModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                                    UnoControlImageControlModel( const UnoControlImageControlModel& rModel ) : GraphicControlModel( rModel ), mbAdjustingImageScaleMode( false ) { }

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlImageControlModel( *this ); }

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // ::cppu::OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) override;
};


//  class UnoImageControlControl

typedef ::cppu::AggImplInheritanceHelper1   <   UnoControlBase
                                            ,   css::awt::XLayoutConstrains
                                            >   UnoImageControlControl_Base;
class UnoImageControlControl : public UnoImageControlControl_Base
{
private:
    ActionListenerMultiplexer   maActionListeners;

public:

                            UnoImageControlControl();
    OUString         GetComponentServiceName() override;

    void SAL_CALL dispose(  ) override;

    // css::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoControlRadioButtonModel

class UnoControlRadioButtonModel :  public GraphicControlModel

{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlRadioButtonModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlRadioButtonModel( const UnoControlRadioButtonModel& rModel ) : GraphicControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlRadioButtonModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoRadioButtonControl

typedef ::cppu::AggImplInheritanceHelper4   <   UnoControlBase
                                            ,   css::awt::XButton
                                            ,   css::awt::XRadioButton
                                            ,   css::awt::XItemListener
                                            ,   css::awt::XLayoutConstrains
                                            >   UnoRadioButtonControl_Base;
class UnoRadioButtonControl : public UnoRadioButtonControl_Base
{
private:
    ItemListenerMultiplexer     maItemListeners;
    ActionListenerMultiplexer   maActionListeners;
    OUString             maActionCommand;

public:

                            UnoRadioButtonControl();
    OUString         GetComponentServiceName() override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void SAL_CALL dispose(  ) override;
    void SAL_CALL disposing( const css::lang::EventObject& Source ) override { UnoControlBase::disposing( Source ); }

    // css::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) override;

    // css::awt::XButton
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void SAL_CALL setActionCommand( const OUString& Command ) override;

    // css::awt::XRadioButton
    void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    sal_Bool SAL_CALL getState(  ) override;
    void SAL_CALL setState( sal_Bool b ) override;
    void SAL_CALL setLabel( const OUString& Label ) override;

    // css::awt::XItemListener
    void SAL_CALL itemStateChanged( const css::awt::ItemEvent& rEvent ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoControlCheckBoxModel

class UnoControlCheckBoxModel : public GraphicControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlCheckBoxModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlCheckBoxModel( const UnoControlCheckBoxModel& rModel ) : GraphicControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlCheckBoxModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoCheckBoxControl

typedef ::cppu::AggImplInheritanceHelper4   <   UnoControlBase
                                            ,   css::awt::XButton
                                            ,   css::awt::XCheckBox
                                            ,   css::awt::XItemListener
                                            ,   css::awt::XLayoutConstrains
                                            >   UnoCheckBoxControl_Base;
class UnoCheckBoxControl : public UnoCheckBoxControl_Base
{
private:
    ItemListenerMultiplexer     maItemListeners;
    ActionListenerMultiplexer   maActionListeners;
    OUString             maActionCommand;

public:

                            UnoCheckBoxControl();
    OUString         GetComponentServiceName() override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void SAL_CALL dispose(  ) override;
    void SAL_CALL disposing( const css::lang::EventObject& Source ) override { UnoControlBase::disposing( Source ); }

    // css::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) override;

    // css::awt::XButton
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void SAL_CALL setActionCommand( const OUString& Command ) override;

    virtual void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    virtual void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;


    sal_Int16 SAL_CALL getState(  ) override;
    void SAL_CALL setState( sal_Int16 n ) override;
    void SAL_CALL setLabel( const OUString& Label ) override;
    void SAL_CALL enableTriState( sal_Bool b ) override;

    // css::awt::XItemListener
    void SAL_CALL itemStateChanged( const css::awt::ItemEvent& rEvent ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoControlFixedTextModel

class UnoControlFixedHyperlinkModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
    UnoControlFixedHyperlinkModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    UnoControlFixedHyperlinkModel( const UnoControlFixedHyperlinkModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlFixedHyperlinkModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlFixedHyperlinkModel, UnoControlModel, "com.sun.star.awt.UnoControlFixedHyperlinkModel" )
};


//  class UnoFixedHyperlinkControl

class UnoFixedHyperlinkControl : public UnoControlBase,
                                 public css::awt::XFixedHyperlink,
                                 public css::awt::XLayoutConstrains
{
private:
    ActionListenerMultiplexer   maActionListeners;

public:
    UnoFixedHyperlinkControl();

    OUString     GetComponentServiceName() override;

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override { return UnoControlBase::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void SAL_CALL dispose(  ) override;

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    // css::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) override;

    // css::awt::XFixedHyperlink
    void SAL_CALL setText( const OUString& Text ) override;
    OUString SAL_CALL getText(  ) override;
    void SAL_CALL setURL( const OUString& URL ) override;
    OUString SAL_CALL getURL(  ) override;
    void SAL_CALL setAlignment( sal_Int16 nAlign ) override;
    sal_Int16 SAL_CALL getAlignment(  ) override;
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoFixedHyperlinkControl, UnoControlBase, "com.sun.star.awt.UnoControlFixedHyperlink" )
};


//  class UnoControlFixedTextModel

class UnoControlFixedTextModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlFixedTextModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlFixedTextModel( const UnoControlFixedTextModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlFixedTextModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoFixedTextControl

class UnoFixedTextControl : public UnoControlBase,
                            public css::awt::XFixedText,
                            public css::awt::XLayoutConstrains
{
public:
                        UnoFixedTextControl();
    OUString     GetComponentServiceName() override;

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override { return UnoControlBase::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    // css::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) override;

    // css::awt::XFixedText
    void SAL_CALL setText( const OUString& Text ) override;
    OUString SAL_CALL getText(  ) override;
    void SAL_CALL setAlignment( sal_Int16 nAlign ) override;
    sal_Int16 SAL_CALL getAlignment(  ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoControlGroupBoxModel

class UnoControlGroupBoxModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlGroupBoxModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlGroupBoxModel( const UnoControlGroupBoxModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlGroupBoxModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoGroupBoxControl

class UnoGroupBoxControl :  public UnoControlBase
{
public:
                        UnoGroupBoxControl();
    OUString     GetComponentServiceName() override;

    sal_Bool SAL_CALL isTransparent(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoControlListBoxModel

struct UnoControlListBoxModel_Data;
typedef ::cppu::AggImplInheritanceHelper1   <   UnoControlModel
                                            ,   css::awt::XItemList
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
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            ConstructorMode const i_mode = ConstructDefault
                        );
                        UnoControlListBoxModel( const UnoControlListBoxModel& i_rSource );
                        virtual ~UnoControlListBoxModel() override;

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlListBoxModel( *this ); }

    virtual void        ImplNormalizePropertySequence(
                            const sal_Int32                 _nCount,        /// the number of entries in the arrays
                            sal_Int32*                      _pHandles,      /// the handles of the properties to set
                            css::uno::Any*     _pValues,       /// the values of the properties to set
                            sal_Int32*                      _pValidHandles  /// pointer to the valid handles, allowed to be adjusted
                        )   const override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // css::awt::XItemList
    virtual ::sal_Int32 SAL_CALL getItemCount() override;
    virtual void SAL_CALL insertItem( ::sal_Int32 Position, const OUString& ItemText, const OUString& ItemImageURL ) override;
    virtual void SAL_CALL insertItemText( ::sal_Int32 Position, const OUString& ItemText ) override;
    virtual void SAL_CALL insertItemImage( ::sal_Int32 Position, const OUString& ItemImageURL ) override;
    virtual void SAL_CALL removeItem( ::sal_Int32 Position ) override;
    virtual void SAL_CALL removeAllItems(  ) override;
    virtual void SAL_CALL setItemText( ::sal_Int32 Position, const OUString& ItemText ) override;
    virtual void SAL_CALL setItemImage( ::sal_Int32 Position, const OUString& ItemImageURL ) override;
    virtual void SAL_CALL setItemTextAndImage( ::sal_Int32 Position, const OUString& ItemText, const OUString& ItemImageURL ) override;
    virtual void SAL_CALL setItemData( ::sal_Int32 Position, const css::uno::Any& DataValue ) override;
    virtual OUString SAL_CALL getItemText( ::sal_Int32 Position ) override;
    virtual OUString SAL_CALL getItemImage( ::sal_Int32 Position ) override;
    virtual css::beans::Pair< OUString, OUString > SAL_CALL getItemTextAndImage( ::sal_Int32 Position ) override;
    virtual css::uno::Any SAL_CALL getItemData( ::sal_Int32 Position ) override;
    virtual css::uno::Sequence< css::beans::Pair< OUString, OUString > > SAL_CALL getAllItems(  ) override;
    virtual void SAL_CALL addItemListListener( const css::uno::Reference< css::awt::XItemListListener >& Listener ) override;
    virtual void SAL_CALL removeItemListListener( const css::uno::Reference< css::awt::XItemListListener >& Listener ) override;

    // OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) override;

protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

private:
    void    impl_notifyItemListEvent_nolck(
                const sal_Int32 i_nItemPosition,
                const ::boost::optional< OUString >& i_rItemText,
                const ::boost::optional< OUString >& i_rItemImageURL,
                void ( SAL_CALL css::awt::XItemListListener::*NotificationMethod )( const css::awt::ItemListEvent& )
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
    ::comphelper::OInterfaceContainerHelper2             m_aItemListListeners;
};


//  class UnoListBoxControl

typedef ::cppu::AggImplInheritanceHelper5   <   UnoControlBase
                                            ,   css::awt::XListBox
                                            ,   css::awt::XItemListener
                                            ,   css::awt::XLayoutConstrains
                                            ,   css::awt::XTextLayoutConstrains
                                            ,   css::awt::XItemListListener
                                            >   UnoListBoxControl_Base;
class TOOLKIT_DLLPUBLIC UnoListBoxControl final : public UnoListBoxControl_Base
{
public:
                        UnoListBoxControl();
    OUString     GetComponentServiceName() override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void SAL_CALL dispose(  ) override;
    void SAL_CALL disposing( const css::lang::EventObject& Source ) override { UnoControlBase::disposing( Source ); }

    // css::awt::XListBox
    void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void SAL_CALL addItem( const OUString& aItem, sal_Int16 nPos ) override;
    void SAL_CALL addItems( const css::uno::Sequence< OUString >& aItems, sal_Int16 nPos ) override;
    void SAL_CALL removeItems( sal_Int16 nPos, sal_Int16 nCount ) override;
    sal_Int16 SAL_CALL getItemCount(  ) override;
    OUString SAL_CALL getItem( sal_Int16 nPos ) override;
    css::uno::Sequence< OUString > SAL_CALL getItems(  ) override;
    sal_Int16 SAL_CALL getSelectedItemPos(  ) override;
    css::uno::Sequence< sal_Int16 > SAL_CALL getSelectedItemsPos(  ) override;
    OUString SAL_CALL getSelectedItem(  ) override;
    css::uno::Sequence< OUString > SAL_CALL getSelectedItems(  ) override;
    void SAL_CALL selectItemPos( sal_Int16 nPos, sal_Bool bSelect ) override;
    void SAL_CALL selectItemsPos( const css::uno::Sequence< sal_Int16 >& aPositions, sal_Bool bSelect ) override;
    void SAL_CALL selectItem( const OUString& aItem, sal_Bool bSelect ) override;
    sal_Bool SAL_CALL isMutipleMode(  ) override;
    void SAL_CALL setMultipleMode( sal_Bool bMulti ) override;
    sal_Int16 SAL_CALL getDropDownLineCount(  ) override;
    void SAL_CALL setDropDownLineCount( sal_Int16 nLines ) override;
    void SAL_CALL makeVisible( sal_Int16 nEntry ) override;

    // css::awt::XItemListener
    void SAL_CALL itemStateChanged( const css::awt::ItemEvent& rEvent ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) override;

    // XUnoControl
    sal_Bool SAL_CALL setModel(const css::uno::Reference< css::awt::XControlModel >& Model) override;

    // XItemListListener
    virtual void SAL_CALL listItemInserted( const css::awt::ItemListEvent& Event ) override;
    virtual void SAL_CALL listItemRemoved( const css::awt::ItemListEvent& Event ) override;
    virtual void SAL_CALL listItemModified( const css::awt::ItemListEvent& Event ) override;
    virtual void SAL_CALL allItemsRemoved( const css::lang::EventObject& Event ) override;
    virtual void SAL_CALL itemListChanged( const css::lang::EventObject& Event ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

private:
    void                ImplUpdateSelectedItemsProperty();
    virtual void        ImplSetPeerProperty( const OUString& rPropName, const css::uno::Any& rVal ) override;
    virtual void        updateFromModel() override;

    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;
};


//  class UnoControlComboBoxModel

class UnoControlComboBoxModel : public UnoControlListBoxModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlComboBoxModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlComboBoxModel( const UnoControlComboBoxModel& rModel ) : UnoControlListBoxModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlComboBoxModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    // OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

};


//  class UnoComboBoxControl

class UnoComboBoxControl :  public UnoEditControl
                        ,   public css::awt::XComboBox
                        ,   public css::awt::XItemListener
                        ,   public css::awt::XItemListListener
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;

public:

                        UnoComboBoxControl();
    OUString     GetComponentServiceName() override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void SAL_CALL disposing( const css::lang::EventObject& Source ) override { UnoEditControl::disposing( Source ); }
    void SAL_CALL dispose(  ) override;

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override { return UnoEditControl::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }


    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    // css::awt::XComboBox
    void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void SAL_CALL addItem( const OUString& aItem, sal_Int16 nPos ) override;
    void SAL_CALL addItems( const css::uno::Sequence< OUString >& aItems, sal_Int16 nPos ) override;
    void SAL_CALL removeItems( sal_Int16 nPos, sal_Int16 nCount ) override;
    sal_Int16 SAL_CALL getItemCount(  ) override;
    OUString SAL_CALL getItem( sal_Int16 nPos ) override;
    css::uno::Sequence< OUString > SAL_CALL getItems(  ) override;
    sal_Int16 SAL_CALL getDropDownLineCount(  ) override;
    void SAL_CALL setDropDownLineCount( sal_Int16 nLines ) override;

    // XUnoControl
    virtual sal_Bool SAL_CALL setModel(const css::uno::Reference< css::awt::XControlModel >& Model) override;

    // XItemListListener
    virtual void SAL_CALL listItemInserted( const css::awt::ItemListEvent& Event ) override;
    virtual void SAL_CALL listItemRemoved( const css::awt::ItemListEvent& Event ) override;
    virtual void SAL_CALL listItemModified( const css::awt::ItemListEvent& Event ) override;
    virtual void SAL_CALL allItemsRemoved( const css::lang::EventObject& Event ) override;
    virtual void SAL_CALL itemListChanged( const css::lang::EventObject& Event ) override;

    // XItemListener
    virtual void SAL_CALL itemStateChanged( const css::awt::ItemEvent& rEvent ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
protected:
    virtual void        ImplSetPeerProperty( const OUString& rPropName, const css::uno::Any& rVal ) override;
    virtual void        updateFromModel() override;

};


//  class UnoSpinFieldControl

class UnoSpinFieldControl : public UnoEditControl,
                            public css::awt::XSpinField
{
private:
    SpinListenerMultiplexer     maSpinListeners;
    bool                        mbRepeat;

public:
                                UnoSpinFieldControl();

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override { return UnoEditControl::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::awt::XSpinField
    void SAL_CALL addSpinListener( const css::uno::Reference< css::awt::XSpinListener >& l ) override;
    void SAL_CALL removeSpinListener( const css::uno::Reference< css::awt::XSpinListener >& l ) override;
    void SAL_CALL up() override;
    void SAL_CALL down() override;
    void SAL_CALL first() override;
    void SAL_CALL last() override;
    void SAL_CALL enableRepeat( sal_Bool bRepeat ) override;


    // css::lang::XServiceInfo
    // No service info, only base class for other fields.
};


//  class UnoControlDateFieldModel

class UnoControlDateFieldModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                UnoControlDateFieldModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                UnoControlDateFieldModel( const UnoControlDateFieldModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlDateFieldModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoDateFieldControl

class UnoDateFieldControl : public UnoSpinFieldControl,
                            public css::awt::XDateField
{
private:
    css::util::Date       mnFirst;
    css::util::Date       mnLast;
    TriState         mbLongFormat;
public:
                            UnoDateFieldControl();
    OUString         GetComponentServiceName() override;

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override { return UnoSpinFieldControl::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::awt::XTextListener
    void SAL_CALL textChanged( const css::awt::TextEvent& rEvent ) override;

    //XDateField
    void SAL_CALL setDate( const css::util::Date& Date ) override;
    css::util::Date SAL_CALL getDate(  ) override;
    void SAL_CALL setMin( const css::util::Date& Date ) override;
    css::util::Date SAL_CALL getMin(  ) override;
    void SAL_CALL setMax( const css::util::Date& Date ) override;
    css::util::Date SAL_CALL getMax(  ) override;
    void SAL_CALL setFirst( const css::util::Date& Date ) override;
    css::util::Date SAL_CALL getFirst(  ) override;
    void SAL_CALL setLast( const css::util::Date& Date ) override;
    css::util::Date SAL_CALL getLast(  ) override;
    void SAL_CALL setLongFormat( sal_Bool bLong ) override;
    sal_Bool SAL_CALL isLongFormat(  ) override;
    void SAL_CALL setEmpty(  ) override;
    sal_Bool SAL_CALL isEmpty(  ) override;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) override;
    sal_Bool SAL_CALL isStrictFormat(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoControlTimeFieldModel

class UnoControlTimeFieldModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlTimeFieldModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlTimeFieldModel( const UnoControlTimeFieldModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlTimeFieldModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoTimeFieldControl

class UnoTimeFieldControl : public UnoSpinFieldControl,
                            public css::awt::XTimeField
{
private:
    css::util::Time       mnFirst;
    css::util::Time       mnLast;

public:
                        UnoTimeFieldControl();
    OUString     GetComponentServiceName() override;

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override { return UnoSpinFieldControl::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::awt::XTextListener
    void SAL_CALL textChanged( const css::awt::TextEvent& rEvent ) override;

    //XTimeField
    void SAL_CALL setTime( const css::util::Time& Time ) override;
    css::util::Time SAL_CALL getTime(  ) override;
    void SAL_CALL setMin( const css::util::Time& Time ) override;
    css::util::Time SAL_CALL getMin(  ) override;
    void SAL_CALL setMax( const css::util::Time& Time ) override;
    css::util::Time SAL_CALL getMax(  ) override;
    void SAL_CALL setFirst( const css::util::Time& Time ) override;
    css::util::Time SAL_CALL getFirst(  ) override;
    void SAL_CALL setLast( const css::util::Time& Time ) override;
    css::util::Time SAL_CALL getLast(  ) override;
    void SAL_CALL setEmpty(  ) override;
    sal_Bool SAL_CALL isEmpty(  ) override;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) override;
    sal_Bool SAL_CALL isStrictFormat(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoControlNumericFieldModel

class UnoControlNumericFieldModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                UnoControlNumericFieldModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                UnoControlNumericFieldModel( const UnoControlNumericFieldModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlNumericFieldModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoNumericFieldControl

class UnoNumericFieldControl :  public UnoSpinFieldControl,
                                public css::awt::XNumericField
{
private:
    double mnFirst;
    double mnLast;

public:
                        UnoNumericFieldControl();
    OUString     GetComponentServiceName() override;

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override { return UnoSpinFieldControl::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    // css::awt::XTextListener
    void SAL_CALL textChanged( const css::awt::TextEvent& rEvent ) override;

    // css::awt::XNumericField
    void SAL_CALL setValue( double Value ) override;
    double SAL_CALL getValue(  ) override;
    void SAL_CALL setMin( double Value ) override;
    double SAL_CALL getMin(  ) override;
    void SAL_CALL setMax( double Value ) override;
    double SAL_CALL getMax(  ) override;
    void SAL_CALL setFirst( double Value ) override;
    double SAL_CALL getFirst(  ) override;
    void SAL_CALL setLast( double Value ) override;
    double SAL_CALL getLast(  ) override;
    void SAL_CALL setSpinSize( double Value ) override;
    double SAL_CALL getSpinSize(  ) override;
    void SAL_CALL setDecimalDigits( sal_Int16 nDigits ) override;
    sal_Int16 SAL_CALL getDecimalDigits(  ) override;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) override;
    sal_Bool SAL_CALL isStrictFormat(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoControlCurrencyFieldModel

class UnoControlCurrencyFieldModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlCurrencyFieldModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlCurrencyFieldModel( const UnoControlCurrencyFieldModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlCurrencyFieldModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoCurrencyFieldControl

class UnoCurrencyFieldControl : public UnoSpinFieldControl,
                                public css::awt::XCurrencyField
{
private:
    double mnFirst;
    double mnLast;

public:
                        UnoCurrencyFieldControl();
    OUString     GetComponentServiceName() override;

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override { return UnoSpinFieldControl::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    // css::awt::XTextListener
    void SAL_CALL textChanged( const css::awt::TextEvent& rEvent ) override;

    // css::awt::XCurrencyField
    void SAL_CALL setValue( double Value ) override;
    double SAL_CALL getValue(  ) override;
    void SAL_CALL setMin( double Value ) override;
    double SAL_CALL getMin(  ) override;
    void SAL_CALL setMax( double Value ) override;
    double SAL_CALL getMax(  ) override;
    void SAL_CALL setFirst( double Value ) override;
    double SAL_CALL getFirst(  ) override;
    void SAL_CALL setLast( double Value ) override;
    double SAL_CALL getLast(  ) override;
    void SAL_CALL setSpinSize( double Value ) override;
    double SAL_CALL getSpinSize(  ) override;
    void SAL_CALL setDecimalDigits( sal_Int16 nDigits ) override;
    sal_Int16 SAL_CALL getDecimalDigits(  ) override;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) override;
    sal_Bool SAL_CALL isStrictFormat(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoControlPatternFieldModel

class UnoControlPatternFieldModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlPatternFieldModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlPatternFieldModel( const UnoControlPatternFieldModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlPatternFieldModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoPatternFieldControl

class UnoPatternFieldControl :  public UnoSpinFieldControl,
                                public css::awt::XPatternField
{
protected:
    void            ImplSetPeerProperty( const OUString& rPropName, const css::uno::Any& rVal ) override;

public:
                        UnoPatternFieldControl();
    OUString     GetComponentServiceName() override;

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override { return UnoSpinFieldControl::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    // css::awt::XPatternField
    void SAL_CALL setMasks( const OUString& EditMask, const OUString& LiteralMask ) override;
    void SAL_CALL getMasks( OUString& EditMask, OUString& LiteralMask ) override;
    void SAL_CALL setString( const OUString& Str ) override;
    OUString SAL_CALL getString(  ) override;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) override;
    sal_Bool SAL_CALL isStrictFormat(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoControlProgressBarModel

class UnoControlProgressBarModel :  public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlProgressBarModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlProgressBarModel( const UnoControlProgressBarModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlProgressBarModel( *this ); }

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoProgressBarControl

class UnoProgressBarControl :   public UnoControlBase,
                                public css::awt::XProgressBar
{
public:
                                UnoProgressBarControl();
    OUString             GetComponentServiceName() override;

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override { return UnoControlBase::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    // css::awt::XProgressBar
    void SAL_CALL setForegroundColor( sal_Int32 nColor ) override;
    void SAL_CALL setBackgroundColor( sal_Int32 nColor ) override;
    void SAL_CALL setValue( sal_Int32 nValue ) override;
    void SAL_CALL setRange( sal_Int32 nMin, sal_Int32 nMax ) override;
    sal_Int32 SAL_CALL getValue() override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoControlFixedLineModel

class UnoControlFixedLineModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlFixedLineModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlFixedLineModel( const UnoControlFixedLineModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlFixedLineModel( *this ); }

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


//  class UnoFixedLineControl

class UnoFixedLineControl : public UnoControlBase
{
public:
                        UnoFixedLineControl();
    OUString     GetComponentServiceName() override;

    sal_Bool SAL_CALL isTransparent(  ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


#endif // INCLUDED_TOOLKIT_CONTROLS_UNOCONTROLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
