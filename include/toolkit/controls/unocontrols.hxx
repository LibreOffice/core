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
                        UnoControlEditModel( const UnoControlEditModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlEditModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;


    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
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

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override { UnoControlBase::disposing( Source ); }
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

    // disambiguate XInterface
    DECLARE_XINTERFACE()

    // XAggregation
    css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XTextListener
    void SAL_CALL textChanged( const css::awt::TextEvent& rEvent ) throw(css::uno::RuntimeException, std::exception) override;

    // XTextComponent
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

    // XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // XTextLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    sal_Bool SAL_CALL setModel(const css::uno::Reference< css::awt::XControlModel >& Model) throw ( css::uno::RuntimeException, std::exception ) override;
};


//  class UnoControlFileControlModel

class UnoControlFileControlModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

public:
                        UnoControlFileControlModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlFileControlModel( const UnoControlFileControlModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlFileControlModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};


//  class UnoFileControl

class UnoFileControl : public UnoEditControl
{
public:
                        UnoFileControl();
    OUString     GetComponentServiceName() override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};


//  class GraphicControlModel

class GraphicControlModel : public UnoControlModel
{
private:
    bool                                                                                    mbAdjustingImagePosition;
    bool                                                                                    mbAdjustingGraphic;

    css::uno::Reference< css::graphic::XGraphicObject > mxGrfObj;

protected:
    GraphicControlModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
        :UnoControlModel( rxContext )
        ,mbAdjustingImagePosition( false )
        ,mbAdjustingGraphic( false )
    {
    }
    GraphicControlModel( const GraphicControlModel& _rSource ) : UnoControlModel( _rSource ), mbAdjustingImagePosition( false ), mbAdjustingGraphic( false ) { }

    // ::cppu::OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) throw (css::uno::Exception, std::exception) override;

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
                        UnoControlButtonModel( const UnoControlButtonModel& rModel ) : GraphicControlModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlButtonModel( *this ); }

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
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

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;
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

    // css::lang::XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

    // XItemListener
    virtual void SAL_CALL itemStateChanged( const css::awt::ItemEvent& rEvent ) throw (css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
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

    UnoControlModel*    Clone() const override { return new UnoControlImageControlModel( *this ); }

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    // ::cppu::OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) throw (css::uno::Exception, std::exception) override;
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

    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};


//  class UnoControlRadioButtonModel

class UnoControlRadioButtonModel :  public GraphicControlModel

{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlRadioButtonModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlRadioButtonModel( const UnoControlRadioButtonModel& rModel ) : GraphicControlModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlRadioButtonModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
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

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override { UnoControlBase::disposing( Source ); }

    // css::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XButton
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setActionCommand( const OUString& Command ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XRadioButton
    void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL getState(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setState( sal_Bool b ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setLabel( const OUString& Label ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XItemListener
    void SAL_CALL itemStateChanged( const css::awt::ItemEvent& rEvent ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};


//  class UnoControlCheckBoxModel

class UnoControlCheckBoxModel : public GraphicControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlCheckBoxModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlCheckBoxModel( const UnoControlCheckBoxModel& rModel ) : GraphicControlModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlCheckBoxModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
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
                            virtual ~UnoCheckBoxControl(){;}
    OUString         GetComponentServiceName() override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override { UnoControlBase::disposing( Source ); }

    // css::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XButton
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setActionCommand( const OUString& Command ) throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw (css::uno::RuntimeException, std::exception) override;


    sal_Int16 SAL_CALL getState(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setState( sal_Int16 n ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setLabel( const OUString& Label ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL enableTriState( sal_Bool b ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XItemListener
    void SAL_CALL itemStateChanged( const css::awt::ItemEvent& rEvent ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};


//  class UnoControlFixedTextModel

class UnoControlFixedHyperlinkModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
    UnoControlFixedHyperlinkModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    UnoControlFixedHyperlinkModel( const UnoControlFixedHyperlinkModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlFixedHyperlinkModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

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

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override { return UnoControlBase::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) throw(css::uno::RuntimeException, std::exception) override;

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
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) throw(css::uno::RuntimeException, std::exception) override;

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
                        UnoControlFixedTextModel( const UnoControlFixedTextModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlFixedTextModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};


//  class UnoFixedTextControl

class UnoFixedTextControl : public UnoControlBase,
                            public css::awt::XFixedText,
                            public css::awt::XLayoutConstrains
{
public:
                        UnoFixedTextControl();
    OUString     GetComponentServiceName() override;

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override { return UnoControlBase::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XControl
    sal_Bool SAL_CALL isTransparent(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XFixedText
    void SAL_CALL setText( const OUString& Text ) throw(css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getText(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setAlignment( sal_Int16 nAlign ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getAlignment(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};


//  class UnoControlGroupBoxModel

class UnoControlGroupBoxModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlGroupBoxModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlGroupBoxModel( const UnoControlGroupBoxModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlGroupBoxModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};


//  class UnoGroupBoxControl

class UnoGroupBoxControl :  public UnoControlBase
{
public:
                        UnoGroupBoxControl();
    OUString     GetComponentServiceName() override;

    sal_Bool SAL_CALL isTransparent(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
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
                        virtual ~UnoControlListBoxModel();

    UnoControlModel*    Clone() const override { return new UnoControlListBoxModel( *this ); }

    virtual void        ImplNormalizePropertySequence(
                            const sal_Int32                 _nCount,        /// the number of entries in the arrays
                            sal_Int32*                      _pHandles,      /// the handles of the properties to set
                            css::uno::Any*     _pValues,       /// the values of the properties to set
                            sal_Int32*                      _pValidHandles  /// pointer to the valid handles, allowed to be adjusted
                        )   const override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XItemList
    virtual ::sal_Int32 SAL_CALL getItemCount() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertItem( ::sal_Int32 Position, const OUString& ItemText, const OUString& ItemImageURL ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertItemText( ::sal_Int32 Position, const OUString& ItemText ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertItemImage( ::sal_Int32 Position, const OUString& ItemImageURL ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeItem( ::sal_Int32 Position ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeAllItems(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setItemText( ::sal_Int32 Position, const OUString& ItemText ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setItemImage( ::sal_Int32 Position, const OUString& ItemImageURL ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setItemTextAndImage( ::sal_Int32 Position, const OUString& ItemText, const OUString& ItemImageURL ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setItemData( ::sal_Int32 Position, const css::uno::Any& DataValue ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getItemText( ::sal_Int32 Position ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getItemImage( ::sal_Int32 Position ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::beans::Pair< OUString, OUString > SAL_CALL getItemTextAndImage( ::sal_Int32 Position ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getItemData( ::sal_Int32 Position ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::Pair< OUString, OUString > > SAL_CALL getAllItems(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addItemListListener( const css::uno::Reference< css::awt::XItemListListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeItemListListener( const css::uno::Reference< css::awt::XItemListListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;

    // OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) throw (css::uno::Exception, std::exception) override;

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
    ::cppu::OInterfaceContainerHelper             m_aItemListListeners;
};


//  class UnoListBoxControl

typedef ::cppu::AggImplInheritanceHelper5   <   UnoControlBase
                                            ,   css::awt::XListBox
                                            ,   css::awt::XItemListener
                                            ,   css::awt::XLayoutConstrains
                                            ,   css::awt::XTextLayoutConstrains
                                            ,   css::awt::XItemListListener
                                            >   UnoListBoxControl_Base;
class TOOLKIT_DLLPUBLIC UnoListBoxControl : public UnoListBoxControl_Base
{
public:
                        UnoListBoxControl();
    OUString     GetComponentServiceName() override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override { UnoControlBase::disposing( Source ); }

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

    // css::awt::XItemListener
    void SAL_CALL itemStateChanged( const css::awt::ItemEvent& rEvent ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(css::uno::RuntimeException, std::exception) override;

    // XUnoControl
    sal_Bool SAL_CALL setModel(const css::uno::Reference< css::awt::XControlModel >& Model) throw ( css::uno::RuntimeException, std::exception ) override;

    // XItemListListener
    virtual void SAL_CALL listItemInserted( const css::awt::ItemListEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL listItemRemoved( const css::awt::ItemListEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL listItemModified( const css::awt::ItemListEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL allItemsRemoved( const css::lang::EventObject& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL itemListChanged( const css::lang::EventObject& Event ) throw (css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

protected:
    void                ImplUpdateSelectedItemsProperty();
    virtual void        ImplSetPeerProperty( const OUString& rPropName, const css::uno::Any& rVal ) override;
    virtual void        updateFromModel() override;

private:
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
                        UnoControlComboBoxModel( const UnoControlComboBoxModel& rModel ) : UnoControlListBoxModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlComboBoxModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
    // OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) throw (css::uno::Exception, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

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

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override { UnoEditControl::disposing( Source ); }
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override { return UnoEditControl::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }


    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

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

    // XUnoControl
    virtual sal_Bool SAL_CALL setModel(const css::uno::Reference< css::awt::XControlModel >& Model) throw ( css::uno::RuntimeException, std::exception ) override;

    // XItemListListener
    virtual void SAL_CALL listItemInserted( const css::awt::ItemListEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL listItemRemoved( const css::awt::ItemListEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL listItemModified( const css::awt::ItemListEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL allItemsRemoved( const css::lang::EventObject& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL itemListChanged( const css::lang::EventObject& Event ) throw (css::uno::RuntimeException, std::exception) override;

    // XItemListener
    virtual void SAL_CALL itemStateChanged( const css::awt::ItemEvent& rEvent ) throw (css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
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

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override { return UnoEditControl::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XSpinField
    void SAL_CALL addSpinListener( const css::uno::Reference< css::awt::XSpinListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeSpinListener( const css::uno::Reference< css::awt::XSpinListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL up() throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL down() throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL first() throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL last() throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL enableRepeat( sal_Bool bRepeat ) throw(css::uno::RuntimeException, std::exception) override;


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
                UnoControlDateFieldModel( const UnoControlDateFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlDateFieldModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
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

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override { return UnoSpinFieldControl::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTextListener
    void SAL_CALL textChanged( const css::awt::TextEvent& rEvent ) throw(css::uno::RuntimeException, std::exception) override;

    //XDateField
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

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};


//  class UnoControlTimeFieldModel

class UnoControlTimeFieldModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlTimeFieldModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlTimeFieldModel( const UnoControlTimeFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlTimeFieldModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
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

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override { return UnoSpinFieldControl::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTextListener
    void SAL_CALL textChanged( const css::awt::TextEvent& rEvent ) throw(css::uno::RuntimeException, std::exception) override;

    //XTimeField
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

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};


//  class UnoControlNumericFieldModel

class UnoControlNumericFieldModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                UnoControlNumericFieldModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                UnoControlNumericFieldModel( const UnoControlNumericFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlNumericFieldModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
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

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override { return UnoSpinFieldControl::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTextListener
    void SAL_CALL textChanged( const css::awt::TextEvent& rEvent ) throw(css::uno::RuntimeException, std::exception) override;

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

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};


//  class UnoControlCurrencyFieldModel

class UnoControlCurrencyFieldModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlCurrencyFieldModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlCurrencyFieldModel( const UnoControlCurrencyFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlCurrencyFieldModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
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

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override { return UnoSpinFieldControl::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTextListener
    void SAL_CALL textChanged( const css::awt::TextEvent& rEvent ) throw(css::uno::RuntimeException, std::exception) override;

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

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};


//  class UnoControlPatternFieldModel

class UnoControlPatternFieldModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlPatternFieldModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlPatternFieldModel( const UnoControlPatternFieldModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlPatternFieldModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
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

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override { return UnoSpinFieldControl::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

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

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};


//  class UnoControlProgressBarModel

class UnoControlProgressBarModel :  public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlProgressBarModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlProgressBarModel( const UnoControlProgressBarModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlProgressBarModel( *this ); }

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};


//  class UnoProgressBarControl

class UnoProgressBarControl :   public UnoControlBase,
                                public css::awt::XProgressBar
{
public:
                                UnoProgressBarControl();
    OUString             GetComponentServiceName() override;

    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override { return UnoControlBase::queryInterface(rType); }
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XProgressBar
    void SAL_CALL setForegroundColor( sal_Int32 nColor ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setBackgroundColor( sal_Int32 nColor ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setValue( sal_Int32 nValue ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setRange( sal_Int32 nMin, sal_Int32 nMax ) throw(css::uno::RuntimeException, std::exception ) override;
    sal_Int32 SAL_CALL getValue() throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};



//  class UnoControlFixedLineModel

class UnoControlFixedLineModel : public UnoControlModel
{
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
                        UnoControlFixedLineModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        UnoControlFixedLineModel( const UnoControlFixedLineModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlFixedLineModel( *this ); }

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};


//  class UnoFixedLineControl

class UnoFixedLineControl : public UnoControlBase
{
public:
                        UnoFixedLineControl();
    OUString     GetComponentServiceName() override;

    sal_Bool SAL_CALL isTransparent(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};



#endif // INCLUDED_TOOLKIT_CONTROLS_UNOCONTROLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
