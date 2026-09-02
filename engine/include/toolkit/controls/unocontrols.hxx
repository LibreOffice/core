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

#include <config_options.h>
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
#include <comphelper/interfacecontainer4.hxx>
#include <comphelper/uno3.hxx>
#include <tools/gen.hxx>

#include <memory>
#include <vector>

#include <optional>

namespace com::sun::star::graphic { class XGraphic; }
namespace com::sun::star::graphic { class XGraphicObject; }

class ImageHelper
{
public:
    // The routine will always attempt to return a valid XGraphic for the
    // passed _rURL, additionally xOutGraphicObject will contain the
    // associated XGraphicObject ( if url is valid for that ) and is set
    // appropriately ( e.g. NULL if non GraphicObject scheme ) or a valid
    // object if the rURL points to a valid object
    static css::uno::Reference< css::graphic::XGraphic > getGraphicAndGraphicObjectFromURL_nothrow( css::uno::Reference< css::graphic::XGraphicObject >& xOutGraphicObject, const OUString& _rURL );
    static css::uno::Reference< css::graphic::XGraphic > getGraphicFromURL_nothrow( const OUString& _rURL, OUString const & referer );

};



class UnoControlEditModel final : public UnoControlModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                        UnoControlEditModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                        UnoControlEditModel( const UnoControlEditModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlEditModel( *this ); }

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;


    // XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



typedef ::cppu::ImplHelper4  <   css::awt::XTextComponent
                             ,   css::awt::XTextListener
                             ,   css::awt::XLayoutConstrains
                             ,   css::awt::XTextLayoutConstrains
                             >   UnoEditControl_Base;
class UNLESS_MERGELIBS(TOOLKIT_DLLPUBLIC) UnoEditControl    :public UnoControlBase
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
    OUString             GetComponentServiceName() const override;
    TextListenerMultiplexer&    GetTextListeners()  { return maTextListeners; }

    void                        ImplSetPeerProperty( const OUString& rPropName, const cpo::uno::Any& rVal ) override;

    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void disposing( const css::lang::EventObject& Source ) override { UnoControlBase::disposing( Source ); }
    void dispose(  ) override;

    // disambiguate XInterface
    DECLARE_XINTERFACE()

    // XAggregation
    cpo::uno::Any queryAggregation( const cpo::uno::Type & rType ) override;

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XTextListener
    void textChanged( const css::awt::TextEvent& rEvent ) override;

    // XTextComponent
    void addTextListener( const css::uno::Reference< css::awt::XTextListener >& l ) override;
    void removeTextListener( const css::uno::Reference< css::awt::XTextListener >& l ) override;
    void setText( const OUString& aText ) override;
    void insertText( const css::awt::Selection& Sel, const OUString& Text ) override;
    OUString getText(  ) override;
    OUString getSelectedText(  ) override;
    void setSelection( const css::awt::Selection& aSelection ) override;
    css::awt::Selection getSelection(  ) override;
    bool isEditable(  ) override;
    void setEditable( bool bEditable ) override;
    void setMaxTextLen( sal_Int16 nLen ) override;
    sal_Int16 getMaxTextLen(  ) override;

    // XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // XTextLayoutConstrains
    css::awt::Size getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) override;
    void getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) override;

    // XServiceInfo
    OUString getImplementationName(  ) override;
    cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    bool setModel(const css::uno::Reference< css::awt::XControlModel >& Model) override;
};



class UnoControlFileControlModel final : public UnoControlModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                        UnoControlFileControlModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                        UnoControlFileControlModel( const UnoControlFileControlModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlFileControlModel( *this ); }

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoFileControl final : public UnoEditControl
{
public:
                        UnoFileControl();
    OUString     GetComponentServiceName() const override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class GraphicControlModel : public UnoControlModel
{
private:
    bool                                                                                    mbAdjustingImagePosition;
    bool                                                                                    mbAdjustingGraphic;

protected:
    GraphicControlModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext )
        :UnoControlModel( rxContext )
        ,mbAdjustingImagePosition( false )
        ,mbAdjustingGraphic( false )
    {
    }
    GraphicControlModel( const GraphicControlModel& _rSource ) : UnoControlModel( _rSource ), mbAdjustingImagePosition( false ), mbAdjustingGraphic( false ) { }

    // ::comphelper::OPropertySetHelper
    void setFastPropertyValue_NoBroadcast( std::unique_lock<std::mutex>& rGuard, sal_Int32 nHandle, const cpo::uno::Any& rValue ) override;

    // UnoControlModel
    cpo::uno::Any ImplGetDefaultValue( sal_uInt16 nPropId ) const override;

private:
        GraphicControlModel& operator=( const GraphicControlModel& ) = delete;
};



class TOOLKIT_DLLPUBLIC UnoControlButtonModel final : public GraphicControlModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                        UnoControlButtonModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                        UnoControlButtonModel( const UnoControlButtonModel& rModel ) : GraphicControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlButtonModel( *this ); }

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



typedef ::cppu::AggImplInheritanceHelper4   <   UnoControlBase
                                            ,   css::awt::XButton
                                            ,   css::awt::XToggleButton
                                            ,   css::awt::XLayoutConstrains
                                            ,   css::awt::XItemListener
                                            >   UnoButtonControl_Base;
class TOOLKIT_DLLPUBLIC UnoButtonControl final : public UnoButtonControl_Base
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;
    OUString             maActionCommand;

public:

                        UnoButtonControl();
    OUString     GetComponentServiceName() const override;

    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void dispose(  ) override;

    // css::awt::XButton
    void addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void setLabel( const OUString& Label ) override;
    void setActionCommand( const OUString& Command ) override;

    // css::awt::XToggleButton
    // css::awt::XItemEventBroadcaster
    void addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;

    // css::lang::XEventListener
    virtual void disposing( const css::lang::EventObject& Source ) override;

    // XItemListener
    virtual void itemStateChanged( const css::awt::ItemEvent& rEvent ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoControlImageControlModel final : public GraphicControlModel
{
private:
    bool    mbAdjustingImageScaleMode;

    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                                    UnoControlImageControlModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                                    UnoControlImageControlModel( const UnoControlImageControlModel& rModel ) : GraphicControlModel( rModel ), mbAdjustingImageScaleMode( false ) { }

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlImageControlModel( *this ); }

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

    // ::cppu::OPropertySetHelper
    void setFastPropertyValue_NoBroadcast( std::unique_lock<std::mutex>& rGuard, sal_Int32 nHandle, const cpo::uno::Any& rValue ) override;
};



typedef ::cppu::AggImplInheritanceHelper1   <   UnoControlBase
                                            ,   css::awt::XLayoutConstrains
                                            >   UnoImageControlControl_Base;
class UnoImageControlControl final : public UnoImageControlControl_Base
{
private:
    ActionListenerMultiplexer   maActionListeners;

public:

                            UnoImageControlControl();
    OUString         GetComponentServiceName() const override;

    void dispose(  ) override;

    // css::awt::XControl
    bool isTransparent(  ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoControlRadioButtonModel final : public GraphicControlModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                        UnoControlRadioButtonModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                        UnoControlRadioButtonModel( const UnoControlRadioButtonModel& rModel ) : GraphicControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlRadioButtonModel( *this ); }

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



typedef ::cppu::AggImplInheritanceHelper4   <   UnoControlBase
                                            ,   css::awt::XButton
                                            ,   css::awt::XRadioButton
                                            ,   css::awt::XItemListener
                                            ,   css::awt::XLayoutConstrains
                                            >   UnoRadioButtonControl_Base;
class UnoRadioButtonControl final : public UnoRadioButtonControl_Base
{
private:
    ItemListenerMultiplexer     maItemListeners;
    ActionListenerMultiplexer   maActionListeners;
    OUString             maActionCommand;

public:

                            UnoRadioButtonControl();
    OUString         GetComponentServiceName() const override;

    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void dispose(  ) override;
    void disposing( const css::lang::EventObject& Source ) override { UnoControlBase::disposing( Source ); }

    // css::awt::XControl
    bool isTransparent(  ) override;

    // css::awt::XButton
    void addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void setActionCommand( const OUString& Command ) override;

    // css::awt::XRadioButton
    void addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    bool getState(  ) override;
    void setState( bool b ) override;
    void setLabel( const OUString& Label ) override;

    // css::awt::XItemListener
    void itemStateChanged( const css::awt::ItemEvent& rEvent ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoControlCheckBoxModel final : public GraphicControlModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                        UnoControlCheckBoxModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                        UnoControlCheckBoxModel( const UnoControlCheckBoxModel& rModel ) : GraphicControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlCheckBoxModel( *this ); }

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



typedef ::cppu::AggImplInheritanceHelper4   <   UnoControlBase
                                            ,   css::awt::XButton
                                            ,   css::awt::XCheckBox
                                            ,   css::awt::XItemListener
                                            ,   css::awt::XLayoutConstrains
                                            >   UnoCheckBoxControl_Base;
class UnoCheckBoxControl final : public UnoCheckBoxControl_Base
{
private:
    ItemListenerMultiplexer     maItemListeners;
    ActionListenerMultiplexer   maActionListeners;
    OUString             maActionCommand;

public:

                            UnoCheckBoxControl();
    OUString         GetComponentServiceName() const override;

    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void dispose(  ) override;
    void disposing( const css::lang::EventObject& Source ) override { UnoControlBase::disposing( Source ); }

    // css::awt::XControl
    bool isTransparent(  ) override;

    // css::awt::XButton
    void addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void setActionCommand( const OUString& Command ) override;

    virtual void addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    virtual void removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;


    sal_Int16 getState(  ) override;
    void setState( sal_Int16 n ) override;
    void setLabel( const OUString& Label ) override;
    void enableTriState( bool b ) override;

    // css::awt::XItemListener
    void itemStateChanged( const css::awt::ItemEvent& rEvent ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoControlFixedHyperlinkModel final : public UnoControlModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
    UnoControlFixedHyperlinkModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
    UnoControlFixedHyperlinkModel( const UnoControlFixedHyperlinkModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlFixedHyperlinkModel( *this ); }

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlFixedHyperlinkModel, UnoControlModel, u"com.sun.star.awt.UnoControlFixedHyperlinkModel"_ustr )
};



class UnoFixedHyperlinkControl final : public UnoControlBase,
                                 public css::awt::XFixedHyperlink,
                                 public css::awt::XLayoutConstrains
{
private:
    ActionListenerMultiplexer   maActionListeners;

public:
    UnoFixedHyperlinkControl();

    OUString     GetComponentServiceName() const override;

    cpo::uno::Any  queryInterface( const cpo::uno::Type & rType ) override { return UnoControlBase::queryInterface(rType); }
    cpo::uno::Any  queryAggregation( const cpo::uno::Type & rType ) override;
    void                        acquire() noexcept override  { OWeakAggObject::acquire(); }
    void                        release() noexcept override  { OWeakAggObject::release(); }

    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void dispose(  ) override;

    // css::lang::XTypeProvider
    cpo::uno::Sequence< cpo::uno::Type >  getTypes() override;
    cpo::uno::Sequence< sal_Int8 >                     getImplementationId() override;

    // css::awt::XControl
    bool isTransparent(  ) override;

    // css::awt::XFixedHyperlink
    void setText( const OUString& Text ) override;
    OUString getText(  ) override;
    void setURL( const OUString& URL ) override;
    OUString getURL(  ) override;
    void setAlignment( sal_Int16 nAlign ) override;
    sal_Int16 getAlignment(  ) override;
    void addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoFixedHyperlinkControl, UnoControlBase, u"com.sun.star.awt.UnoControlFixedHyperlink"_ustr )
};



class TOOLKIT_DLLPUBLIC UnoControlFixedTextModel final : public UnoControlModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                        UnoControlFixedTextModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                        UnoControlFixedTextModel( const UnoControlFixedTextModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlFixedTextModel( *this ); }

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class TOOLKIT_DLLPUBLIC UnoFixedTextControl final : public UnoControlBase,
                            public css::awt::XFixedText,
                            public css::awt::XLayoutConstrains
{
public:
                        UnoFixedTextControl();
    OUString     GetComponentServiceName() const override;

    cpo::uno::Any  queryInterface( const cpo::uno::Type & rType ) override { return UnoControlBase::queryInterface(rType); }
    cpo::uno::Any  queryAggregation( const cpo::uno::Type & rType ) override;
    void                        acquire() noexcept override  { OWeakAggObject::acquire(); }
    void                        release() noexcept override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    cpo::uno::Sequence< cpo::uno::Type >  getTypes() override;
    cpo::uno::Sequence< sal_Int8 >                     getImplementationId() override;

    // css::awt::XControl
    bool isTransparent(  ) override;

    // css::awt::XFixedText
    void setText( const OUString& Text ) override;
    OUString getText(  ) override;
    void setAlignment( sal_Int16 nAlign ) override;
    sal_Int16 getAlignment(  ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoControlGroupBoxModel final : public UnoControlModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                        UnoControlGroupBoxModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                        UnoControlGroupBoxModel( const UnoControlGroupBoxModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlGroupBoxModel( *this ); }

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoGroupBoxControl final : public UnoControlBase
{
public:
                        UnoGroupBoxControl();
    OUString     GetComponentServiceName() const override;

    bool isTransparent(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



struct UnoControlListBoxModel_Data;
typedef ::cppu::AggImplInheritanceHelper1   <   UnoControlModel
                                            ,   css::awt::XItemList
                                            >   UnoControlListBoxModel_Base;
class UnoControlListBoxModel : public UnoControlListBoxModel_Base
{
protected:
    enum ConstructorMode
    {
        ConstructDefault,
        ConstructWithoutProperties
    };

public:
                        UnoControlListBoxModel(
                            const css::uno::Reference< cpo::uno::XComponentContext >& rxContext,
                            ConstructorMode const i_mode = ConstructDefault
                        );
                        UnoControlListBoxModel( const UnoControlListBoxModel& i_rSource );
                        virtual ~UnoControlListBoxModel() override;

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlListBoxModel( *this ); }

    virtual void        ImplNormalizePropertySequence(
                            const sal_Int32                 _nCount,        /// the number of entries in the arrays
                            sal_Int32*                      _pHandles,      /// the handles of the properties to set
                            cpo::uno::Any*     _pValues,       /// the values of the properties to set
                            sal_Int32*                      _pValidHandles  /// pointer to the valid handles, allowed to be adjusted
                        )   const override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::lang::XServiceInfo
    OUString getImplementationName(  ) override;
    cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // css::awt::XItemList
    virtual ::sal_Int32 getItemCount() override;
    virtual void insertItem( ::sal_Int32 Position, const OUString& ItemText, const OUString& ItemImageURL ) override;
    virtual void insertItemText( ::sal_Int32 Position, const OUString& ItemText ) override;
    virtual void insertItemImage( ::sal_Int32 Position, const OUString& ItemImageURL ) override;
    virtual void removeItem( ::sal_Int32 Position ) override;
    virtual void removeAllItems(  ) override;
    virtual void setItemText( ::sal_Int32 Position, const OUString& ItemText ) override;
    virtual void setItemImage( ::sal_Int32 Position, const OUString& ItemImageURL ) override;
    virtual void setItemTextAndImage( ::sal_Int32 Position, const OUString& ItemText, const OUString& ItemImageURL ) override;
    virtual void setItemData( ::sal_Int32 Position, const cpo::uno::Any& DataValue ) override;
    virtual OUString getItemText( ::sal_Int32 Position ) override;
    virtual OUString getItemImage( ::sal_Int32 Position ) override;
    virtual css::beans::Pair< OUString, OUString > getItemTextAndImage( ::sal_Int32 Position ) override;
    virtual cpo::uno::Any getItemData( ::sal_Int32 Position ) override;
    virtual cpo::uno::Sequence< css::beans::Pair< OUString, OUString > > getAllItems(  ) override;
    virtual void addItemListListener( const css::uno::Reference< css::awt::XItemListListener >& Listener ) override;
    virtual void removeItemListListener( const css::uno::Reference< css::awt::XItemListListener >& Listener ) override;

    // OPropertySetHelper
    void setFastPropertyValue_NoBroadcast( std::unique_lock<std::mutex>& rGuard, sal_Int32 nHandle, const cpo::uno::Any& rValue ) override;

protected:
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

private:
    void    impl_notifyItemListEvent(
                std::unique_lock<std::mutex>& rGuard,
                const sal_Int32 i_nItemPosition,
                const ::std::optional< OUString >& i_rItemText,
                const ::std::optional< OUString >& i_rItemImageURL,
                void ( css::awt::XItemListListener::*NotificationMethod )( const css::awt::ItemListEvent& )
            );

    void    impl_handleInsert(
                std::unique_lock<std::mutex>& rGuard,
                const sal_Int32 i_nItemPosition,
                const ::std::optional< OUString >& i_rItemText,
                const ::std::optional< OUString >& i_rItemImageURL
            );

    void    impl_handleRemove(
                const sal_Int32 i_nItemPosition,
                std::unique_lock<std::mutex>& i_rClearBeforeNotify
            );

    void    impl_handleModify(
                const sal_Int32 i_nItemPosition,
                const ::std::optional< OUString >& i_rItemText,
                const ::std::optional< OUString >& i_rItemImageURL,
                std::unique_lock<std::mutex>& i_rClearBeforeNotify
            );

    void    impl_getStringItemList( std::unique_lock<std::mutex>& rGuard, ::std::vector< OUString >& o_rStringItems ) const;
    void    impl_setStringItemList( std::unique_lock<std::mutex>& rGuard, const ::std::vector< OUString >& i_rStringItems );

protected:
    std::unique_ptr<UnoControlListBoxModel_Data>  m_xData;
    ::comphelper::OInterfaceContainerHelper4<css::awt::XItemListListener> m_aItemListListeners;
};



typedef ::cppu::AggImplInheritanceHelper5   <   UnoControlBase
                                            ,   css::awt::XListBox
                                            ,   css::awt::XItemListener
                                            ,   css::awt::XLayoutConstrains
                                            ,   css::awt::XTextLayoutConstrains
                                            ,   css::awt::XItemListListener
                                            >   UnoListBoxControl_Base;
class UnoListBoxControl final : public UnoListBoxControl_Base
{
public:
                        UnoListBoxControl();
    OUString     GetComponentServiceName() const override;

    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void dispose(  ) override;
    void disposing( const css::lang::EventObject& Source ) override { UnoControlBase::disposing( Source ); }

    // css::awt::XListBox
    void addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void addItem( const OUString& aItem, sal_Int16 nPos ) override;
    void addItems( const cpo::uno::Sequence< OUString >& aItems, sal_Int16 nPos ) override;
    void removeItems( sal_Int16 nPos, sal_Int16 nCount ) override;
    sal_Int16 getItemCount(  ) override;
    OUString getItem( sal_Int16 nPos ) override;
    cpo::uno::Sequence< OUString > getItems(  ) override;
    sal_Int16 getSelectedItemPos(  ) override;
    cpo::uno::Sequence< sal_Int16 > getSelectedItemsPos(  ) override;
    OUString getSelectedItem(  ) override;
    cpo::uno::Sequence< OUString > getSelectedItems(  ) override;
    void selectItemPos( sal_Int16 nPos, bool bSelect ) override;
    void selectItemsPos( const cpo::uno::Sequence< sal_Int16 >& aPositions, bool bSelect ) override;
    void selectItem( const OUString& aItem, bool bSelect ) override;
    bool isMutipleMode(  ) override;
    void setMultipleMode( bool bMulti ) override;
    sal_Int16 getDropDownLineCount(  ) override;
    void setDropDownLineCount( sal_Int16 nLines ) override;
    void makeVisible( sal_Int16 nEntry ) override;

    // css::awt::XItemListener
    void itemStateChanged( const css::awt::ItemEvent& rEvent ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) override;
    void getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) override;

    // XUnoControl
    bool setModel(const css::uno::Reference< css::awt::XControlModel >& Model) override;

    // XItemListListener
    virtual void listItemInserted( const css::awt::ItemListEvent& Event ) override;
    virtual void listItemRemoved( const css::awt::ItemListEvent& Event ) override;
    virtual void listItemModified( const css::awt::ItemListEvent& Event ) override;
    virtual void allItemsRemoved( const css::lang::EventObject& Event ) override;
    virtual void itemListChanged( const css::lang::EventObject& Event ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName(  ) override;
    cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

private:
    void                ImplUpdateSelectedItemsProperty();
    virtual void        ImplSetPeerProperty( const OUString& rPropName, const cpo::uno::Any& rVal ) override;
    virtual void        updateFromModel() override;

    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;
};



class UnoControlComboBoxModel final : public UnoControlListBoxModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                        UnoControlComboBoxModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                        UnoControlComboBoxModel( const UnoControlComboBoxModel& rModel ) : UnoControlListBoxModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlComboBoxModel( *this ); }

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;
    // OPropertySetHelper
    void setFastPropertyValue_NoBroadcast( std::unique_lock<std::mutex>& rGuard, sal_Int32 nHandle, const cpo::uno::Any& rValue ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName(  ) override;
    cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

};



class UnoComboBoxControl final :  public UnoEditControl
                        ,   public css::awt::XComboBox
                        ,   public css::awt::XItemListener
                        ,   public css::awt::XItemListListener
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;

public:

                        UnoComboBoxControl();
    OUString     GetComponentServiceName() const override;

    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    void disposing( const css::lang::EventObject& Source ) override { UnoEditControl::disposing( Source ); }
    void dispose(  ) override;

    cpo::uno::Any  queryInterface( const cpo::uno::Type & rType ) override { return UnoEditControl::queryInterface(rType); }
    cpo::uno::Any  queryAggregation( const cpo::uno::Type & rType ) override;
    void                        acquire() noexcept override  { OWeakAggObject::acquire(); }
    void                        release() noexcept override  { OWeakAggObject::release(); }


    // css::lang::XTypeProvider
    cpo::uno::Sequence< cpo::uno::Type >  getTypes() override;
    cpo::uno::Sequence< sal_Int8 >                     getImplementationId() override;

    // css::awt::XComboBox
    void addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void addItem( const OUString& aItem, sal_Int16 nPos ) override;
    void addItems( const cpo::uno::Sequence< OUString >& aItems, sal_Int16 nPos ) override;
    void removeItems( sal_Int16 nPos, sal_Int16 nCount ) override;
    sal_Int16 getItemCount(  ) override;
    OUString getItem( sal_Int16 nPos ) override;
    cpo::uno::Sequence< OUString > getItems(  ) override;
    sal_Int16 getDropDownLineCount(  ) override;
    void setDropDownLineCount( sal_Int16 nLines ) override;

    // XUnoControl
    virtual bool setModel(const css::uno::Reference< css::awt::XControlModel >& Model) override;

    // XItemListListener
    virtual void listItemInserted( const css::awt::ItemListEvent& Event ) override;
    virtual void listItemRemoved( const css::awt::ItemListEvent& Event ) override;
    virtual void listItemModified( const css::awt::ItemListEvent& Event ) override;
    virtual void allItemsRemoved( const css::lang::EventObject& Event ) override;
    virtual void itemListChanged( const css::lang::EventObject& Event ) override;

    // XItemListener
    virtual void itemStateChanged( const css::awt::ItemEvent& rEvent ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName(  ) override;
    cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
private:
    virtual void        ImplSetPeerProperty( const OUString& rPropName, const cpo::uno::Any& rVal ) override;
    virtual void        updateFromModel() override;

};



class UnoSpinFieldControl : public UnoEditControl,
                            public css::awt::XSpinField
{
private:
    SpinListenerMultiplexer     maSpinListeners;
    bool                        mbRepeat;

public:
                                UnoSpinFieldControl();

    cpo::uno::Any  queryInterface( const cpo::uno::Type & rType ) override { return UnoEditControl::queryInterface(rType); }
    cpo::uno::Any  queryAggregation( const cpo::uno::Type & rType ) override;
    void                        acquire() noexcept override  { OWeakAggObject::acquire(); }
    void                        release() noexcept override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    cpo::uno::Sequence< cpo::uno::Type >  getTypes() override;
    cpo::uno::Sequence< sal_Int8 >                     getImplementationId() override;

    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::awt::XSpinField
    void addSpinListener( const css::uno::Reference< css::awt::XSpinListener >& l ) override;
    void removeSpinListener( const css::uno::Reference< css::awt::XSpinListener >& l ) override;
    void up() override;
    void down() override;
    void first() override;
    void last() override;
    void enableRepeat( bool bRepeat ) override;


    // css::lang::XServiceInfo
    // No service info, only base class for other fields.
};



class UnoControlDateFieldModel final : public UnoControlModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                UnoControlDateFieldModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                UnoControlDateFieldModel( const UnoControlDateFieldModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlDateFieldModel( *this ); }

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoDateFieldControl final : public UnoSpinFieldControl,
                            public css::awt::XDateField
{
private:
    css::util::Date       mnFirst;
    css::util::Date       mnLast;
    TriState         mbLongFormat;
public:
                            UnoDateFieldControl();
    OUString         GetComponentServiceName() const override;

    cpo::uno::Any  queryInterface( const cpo::uno::Type & rType ) override { return UnoSpinFieldControl::queryInterface(rType); }
    cpo::uno::Any  queryAggregation( const cpo::uno::Type & rType ) override;
    void                        acquire() noexcept override  { OWeakAggObject::acquire(); }
    void                        release() noexcept override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    cpo::uno::Sequence< cpo::uno::Type >  getTypes() override;
    cpo::uno::Sequence< sal_Int8 >                     getImplementationId() override;

    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::awt::XTextListener
    void textChanged( const css::awt::TextEvent& rEvent ) override;

    //XDateField
    void setDate( const css::util::Date& Date ) override;
    css::util::Date getDate(  ) override;
    void setMin( const css::util::Date& Date ) override;
    css::util::Date getMin(  ) override;
    void setMax( const css::util::Date& Date ) override;
    css::util::Date getMax(  ) override;
    void setFirst( const css::util::Date& Date ) override;
    css::util::Date getFirst(  ) override;
    void setLast( const css::util::Date& Date ) override;
    css::util::Date getLast(  ) override;
    void setLongFormat( bool bLong ) override;
    bool isLongFormat(  ) override;
    void setEmpty(  ) override;
    bool isEmpty(  ) override;
    void setStrictFormat( bool bStrict ) override;
    bool isStrictFormat(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoControlTimeFieldModel final : public UnoControlModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                        UnoControlTimeFieldModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                        UnoControlTimeFieldModel( const UnoControlTimeFieldModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlTimeFieldModel( *this ); }

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoTimeFieldControl final : public UnoSpinFieldControl,
                            public css::awt::XTimeField
{
private:
    css::util::Time       mnFirst;
    css::util::Time       mnLast;

public:
                        UnoTimeFieldControl();
    OUString     GetComponentServiceName() const override;

    cpo::uno::Any  queryInterface( const cpo::uno::Type & rType ) override { return UnoSpinFieldControl::queryInterface(rType); }
    cpo::uno::Any  queryAggregation( const cpo::uno::Type & rType ) override;
    void                        acquire() noexcept override  { OWeakAggObject::acquire(); }
    void                        release() noexcept override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    cpo::uno::Sequence< cpo::uno::Type >  getTypes() override;
    cpo::uno::Sequence< sal_Int8 >                     getImplementationId() override;

    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::awt::XTextListener
    void textChanged( const css::awt::TextEvent& rEvent ) override;

    //XTimeField
    void setTime( const css::util::Time& Time ) override;
    css::util::Time getTime(  ) override;
    void setMin( const css::util::Time& Time ) override;
    css::util::Time getMin(  ) override;
    void setMax( const css::util::Time& Time ) override;
    css::util::Time getMax(  ) override;
    void setFirst( const css::util::Time& Time ) override;
    css::util::Time getFirst(  ) override;
    void setLast( const css::util::Time& Time ) override;
    css::util::Time getLast(  ) override;
    void setEmpty(  ) override;
    bool isEmpty(  ) override;
    void setStrictFormat( bool bStrict ) override;
    bool isStrictFormat(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoControlNumericFieldModel final : public UnoControlModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                UnoControlNumericFieldModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                UnoControlNumericFieldModel( const UnoControlNumericFieldModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlNumericFieldModel( *this ); }

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoNumericFieldControl final : public UnoSpinFieldControl,
                                public css::awt::XNumericField
{
private:
    double mnFirst;
    double mnLast;

public:
                        UnoNumericFieldControl();
    OUString     GetComponentServiceName() const override;

    cpo::uno::Any  queryInterface( const cpo::uno::Type & rType ) override { return UnoSpinFieldControl::queryInterface(rType); }
    cpo::uno::Any  queryAggregation( const cpo::uno::Type & rType ) override;
    void                        acquire() noexcept override  { OWeakAggObject::acquire(); }
    void                        release() noexcept override  { OWeakAggObject::release(); }

    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::lang::XTypeProvider
    cpo::uno::Sequence< cpo::uno::Type >  getTypes() override;
    cpo::uno::Sequence< sal_Int8 >                     getImplementationId() override;

    // css::awt::XTextListener
    void textChanged( const css::awt::TextEvent& rEvent ) override;

    // css::awt::XNumericField
    void setValue( double Value ) override;
    double getValue(  ) override;
    void setMin( double Value ) override;
    double getMin(  ) override;
    void setMax( double Value ) override;
    double getMax(  ) override;
    void setFirst( double Value ) override;
    double getFirst(  ) override;
    void setLast( double Value ) override;
    double getLast(  ) override;
    void setSpinSize( double Value ) override;
    double getSpinSize(  ) override;
    void setDecimalDigits( sal_Int16 nDigits ) override;
    sal_Int16 getDecimalDigits(  ) override;
    void setStrictFormat( bool bStrict ) override;
    bool isStrictFormat(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoControlCurrencyFieldModel final : public UnoControlModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                        UnoControlCurrencyFieldModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                        UnoControlCurrencyFieldModel( const UnoControlCurrencyFieldModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlCurrencyFieldModel( *this ); }

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoCurrencyFieldControl final : public UnoSpinFieldControl,
                                public css::awt::XCurrencyField
{
private:
    double mnFirst;
    double mnLast;

public:
                        UnoCurrencyFieldControl();
    OUString     GetComponentServiceName() const override;

    cpo::uno::Any  queryInterface( const cpo::uno::Type & rType ) override { return UnoSpinFieldControl::queryInterface(rType); }
    cpo::uno::Any  queryAggregation( const cpo::uno::Type & rType ) override;
    void                        acquire() noexcept override  { OWeakAggObject::acquire(); }
    void                        release() noexcept override  { OWeakAggObject::release(); }

    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::lang::XTypeProvider
    cpo::uno::Sequence< cpo::uno::Type >  getTypes() override;
    cpo::uno::Sequence< sal_Int8 >                     getImplementationId() override;

    // css::awt::XTextListener
    void textChanged( const css::awt::TextEvent& rEvent ) override;

    // css::awt::XCurrencyField
    void setValue( double Value ) override;
    double getValue(  ) override;
    void setMin( double Value ) override;
    double getMin(  ) override;
    void setMax( double Value ) override;
    double getMax(  ) override;
    void setFirst( double Value ) override;
    double getFirst(  ) override;
    void setLast( double Value ) override;
    double getLast(  ) override;
    void setSpinSize( double Value ) override;
    double getSpinSize(  ) override;
    void setDecimalDigits( sal_Int16 nDigits ) override;
    sal_Int16 getDecimalDigits(  ) override;
    void setStrictFormat( bool bStrict ) override;
    bool isStrictFormat(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoControlPatternFieldModel final : public UnoControlModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                        UnoControlPatternFieldModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                        UnoControlPatternFieldModel( const UnoControlPatternFieldModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlPatternFieldModel( *this ); }

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoPatternFieldControl final : public UnoSpinFieldControl,
                                public css::awt::XPatternField
{
    void            ImplSetPeerProperty( const OUString& rPropName, const cpo::uno::Any& rVal ) override;

public:
                        UnoPatternFieldControl();
    OUString     GetComponentServiceName() const override;

    cpo::uno::Any  queryInterface( const cpo::uno::Type & rType ) override { return UnoSpinFieldControl::queryInterface(rType); }
    cpo::uno::Any  queryAggregation( const cpo::uno::Type & rType ) override;
    void                        acquire() noexcept override  { OWeakAggObject::acquire(); }
    void                        release() noexcept override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    cpo::uno::Sequence< cpo::uno::Type >  getTypes() override;
    cpo::uno::Sequence< sal_Int8 >                     getImplementationId() override;

    // css::awt::XPatternField
    void setMasks( const OUString& EditMask, const OUString& LiteralMask ) override;
    void getMasks( OUString& EditMask, OUString& LiteralMask ) override;
    void setString( const OUString& Str ) override;
    OUString getString(  ) override;
    void setStrictFormat( bool bStrict ) override;
    bool isStrictFormat(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoControlProgressBarModel final : public UnoControlModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                        UnoControlProgressBarModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                        UnoControlProgressBarModel( const UnoControlProgressBarModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlProgressBarModel( *this ); }

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::io::XPersistObject
    OUString getServiceName() override;

    // XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoProgressBarControl final : public UnoControlBase,
                                public css::awt::XProgressBar
{
public:
                                UnoProgressBarControl();
    OUString             GetComponentServiceName() const override;

    cpo::uno::Any  queryInterface( const cpo::uno::Type & rType ) override { return UnoControlBase::queryInterface(rType); }
    cpo::uno::Any  queryAggregation( const cpo::uno::Type & rType ) override;
    void                        acquire() noexcept override  { OWeakAggObject::acquire(); }
    void                        release() noexcept override  { OWeakAggObject::release(); }

    // css::lang::XTypeProvider
    cpo::uno::Sequence< cpo::uno::Type >  getTypes() override;
    cpo::uno::Sequence< sal_Int8 >                     getImplementationId() override;

    // css::awt::XProgressBar
    void setForegroundColor( sal_Int32 nColor ) override;
    void setBackgroundColor( sal_Int32 nColor ) override;
    void setValue( sal_Int32 nValue ) override;
    void setRange( sal_Int32 nMin, sal_Int32 nMax ) override;
    sal_Int32 getValue() override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoControlFixedLineModel final : public UnoControlModel
{
    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
                        UnoControlFixedLineModel( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext );
                        UnoControlFixedLineModel( const UnoControlFixedLineModel& rModel ) : UnoControlModel( rModel ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlFixedLineModel( *this ); }

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};



class UnoFixedLineControl final : public UnoControlBase
{
public:
                        UnoFixedLineControl();
    OUString     GetComponentServiceName() const override;

    bool isTransparent(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
