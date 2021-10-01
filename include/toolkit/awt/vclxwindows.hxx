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

#pragma once

#include <toolkit/dllapi.h>

#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XNumericField.hpp>
#include <com/sun/star/awt/XScrollBar.hpp>
#include <com/sun/star/awt/XTextEditField.hpp>
#include <com/sun/star/awt/XSpinField.hpp>
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XToggleButton.hpp>
#include <com/sun/star/awt/XFixedHyperlink.hpp>
#include <com/sun/star/awt/XFixedText.hpp>
#include <com/sun/star/awt/XRadioButton.hpp>
#include <com/sun/star/awt/XComboBox.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XItemListListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weak.hxx>

#include <svl/numuno.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>

#include <vcl/image.hxx>

//  class VCLXGraphicControl
//    deriving from VCLXWindow, drawing the graphic which exists as "Graphic" at the model

class VCLXGraphicControl : public VCLXWindow
{
private:
    /// the image we currently display
    Image                       maImage;

protected:
    const Image&    GetImage() const { return maImage; }

protected:
    // css::awt::XWindow
    void SAL_CALL setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

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
    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }

};


//  class VCLXButton

typedef cppu::ImplInheritanceHelper< VCLXGraphicControl,
                                     css::awt::XButton,
                                     css::awt::XToggleButton
                                   > VCLXButton_Base;
class VCLXButton final : public VCLXButton_Base
{
private:
    OUString             maActionCommand;
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;

    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;

public:
                    VCLXButton();
    virtual ~VCLXButton() override;

    // css::lang::XComponent
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

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

//  class VCLXCheckBox

class TOOLKIT_DLLPUBLIC VCLXCheckBox final : public css::awt::XCheckBox,
                        public css::awt::XButton,
                        public VCLXGraphicControl
{
private:
    ActionListenerMultiplexer   maActionListeners;
    OUString             maActionCommand;
    ItemListenerMultiplexer     maItemListeners;

    void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;

public:
                    VCLXCheckBox();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                                        SAL_CALL acquire() noexcept override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() noexcept override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;


    // css::lang::XComponent
    void SAL_CALL dispose(  ) override;

    // css::awt::XCheckBox
    void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    sal_Int16 SAL_CALL getState(  ) override;
    void SAL_CALL setState( sal_Int16 n ) override;
    void SAL_CALL setLabel( const OUString& Label ) override;
    void SAL_CALL enableTriState( sal_Bool b ) override;

    // css::awt::XButton:
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void SAL_CALL setActionCommand( const OUString& Command ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXRadioButton

class TOOLKIT_DLLPUBLIC VCLXRadioButton final : public css::awt::XRadioButton,
                        public css::awt::XButton,
                        public VCLXGraphicControl
{
private:
    ItemListenerMultiplexer     maItemListeners;
    ActionListenerMultiplexer   maActionListeners;
    OUString             maActionCommand;

    void            ImplClickedOrToggled( bool bToggled );
    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;

public:
                    VCLXRadioButton();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                                        SAL_CALL acquire() noexcept override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() noexcept override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    // css::lang::XComponent
    void SAL_CALL dispose(  ) override;

    // css::awt::XRadioButton
    void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    sal_Bool SAL_CALL getState(  ) override;
    void SAL_CALL setState( sal_Bool b ) override;
    void SAL_CALL setLabel( const OUString& Label ) override;

    // css::awt::XButton:
    void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void SAL_CALL setActionCommand( const OUString& Command ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }

};

//  class VCLXFixedHyperlink
class VCLXFixedHyperlink final :
    public css::awt::XFixedHyperlink,
    public VCLXWindow
{
private:
    ActionListenerMultiplexer   maActionListeners;

    void                        ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
                                CreateAccessibleContext() override;

public:
    VCLXFixedHyperlink();
    virtual ~VCLXFixedHyperlink() override;

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                                        SAL_CALL acquire() noexcept override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() noexcept override  { OWeakObject::release(); }

    // css::lang::XComponent
    void SAL_CALL dispose(  ) override;

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

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
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXFixedText

class VCLXFixedText final : public css::awt::XFixedText,
                        public VCLXWindow
{
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;

public:
                        VCLXFixedText();
                        virtual ~VCLXFixedText() override;

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                                        SAL_CALL acquire() noexcept override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() noexcept override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    // css::awt::XFixedText
    void SAL_CALL setText( const OUString& Text ) override;
    OUString SAL_CALL getText(  ) override;
    void SAL_CALL setAlignment( sal_Int16 nAlign ) override;
    sal_Int16 SAL_CALL getAlignment(  ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXScrollBar

class TOOLKIT_DLLPUBLIC VCLXScrollBar final : public css::awt::XScrollBar,
                        public VCLXWindow
{
private:
    AdjustmentListenerMultiplexer maAdjustmentListeners;

    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;

public:
                    VCLXScrollBar();
    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                                        SAL_CALL acquire() noexcept override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() noexcept override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;


    // css::lang::XComponent
    void SAL_CALL dispose(  ) override;

    // css::awt::XScrollbar
    void SAL_CALL addAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& l ) override;
    void SAL_CALL removeAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& l ) override;
    void SAL_CALL setValue( sal_Int32 n ) override;
    void SAL_CALL setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) override;
    sal_Int32 SAL_CALL getValue(  ) override;
    void SAL_CALL setMaximum( sal_Int32 n ) override;
    sal_Int32 SAL_CALL getMaximum(  ) override;
    void SAL_CALL setLineIncrement( sal_Int32 n ) override;
    sal_Int32 SAL_CALL getLineIncrement(  ) override;
    void SAL_CALL setBlockIncrement( sal_Int32 n ) override;
    sal_Int32 SAL_CALL getBlockIncrement(  ) override;
    void SAL_CALL setVisibleSize( sal_Int32 n ) override;
    sal_Int32 SAL_CALL getVisibleSize(  ) override;
    void SAL_CALL setOrientation( sal_Int32 n ) override;
    sal_Int32 SAL_CALL getOrientation(  ) override;

    // why isn't this part of the XScrollbar?
    /// @throws css::uno::RuntimeException
    void setMinimum( sal_Int32 n );
    /// @throws css::uno::RuntimeException
    sal_Int32 getMinimum(  ) const;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize() override;
    /// @throws css::uno::RuntimeException
    static css::awt::Size implGetMinimumSize( vcl::Window const * p );

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXEdit

class SAL_DLLPUBLIC_RTTI VCLXEdit :  public css::awt::XTextComponent,
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
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                                        SAL_CALL acquire() noexcept override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() noexcept override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;


    // css::lang::XComponent
    void SAL_CALL dispose(  ) override;

    // css::awt::XTextComponent
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

    // css::awt::XTextEditField:
    void SAL_CALL setEchoChar( sal_Unicode cEcho ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXSpinField
class VCLXSpinField : public css::awt::XSpinField, public VCLXEdit
{
private:
    SpinListenerMultiplexer maSpinListeners;

protected:
    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

public:
                    VCLXSpinField();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                                        SAL_CALL acquire() noexcept override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() noexcept override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;


    // css::awt::XSpinField
    void SAL_CALL addSpinListener( const css::uno::Reference< css::awt::XSpinListener >& l ) override;
    void SAL_CALL removeSpinListener( const css::uno::Reference< css::awt::XSpinListener >& l ) override;
    void SAL_CALL up(  ) override;
    void SAL_CALL down(  ) override;
    void SAL_CALL first(  ) override;
    void SAL_CALL last(  ) override;
    void SAL_CALL enableRepeat( sal_Bool bRepeat ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

class SVTXFormattedField : public VCLXSpinField
{
protected:
    rtl::Reference<SvNumberFormatsSupplierObj> m_xCurrentSupplier;
    bool                    bIsStandardSupplier;

    sal_Int32                   nKeyToSetDelayed;

public:
    SVTXFormattedField();
    virtual ~SVTXFormattedField() override;

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

protected:
    void    setFormatsSupplier(const css::uno::Reference< css::util::XNumberFormatsSupplier > & xSupplier);
    sal_Int32   getFormatKey() const;
    void    setFormatKey(sal_Int32 nKey);

    void    SetValue(const css::uno::Any& rValue);
    css::uno::Any  GetValue() const;

    void    SetTreatAsNumber(bool bSet);
    bool    GetTreatAsNumber() const;

    void    SetDefaultValue(const css::uno::Any& rValue);
    css::uno::Any  GetDefaultValue() const;

    void    SetMinValue(const css::uno::Any& rValue);
    css::uno::Any  GetMinValue() const;

    void    SetMaxValue(const css::uno::Any& rValue);
    css::uno::Any  GetMaxValue() const;

    void    NotifyTextListeners();
    css::uno::Any  convertEffectiveValue(const css::uno::Any& rValue) const;

    virtual void    SetWindow( const VclPtr< vcl::Window > &_pWindow) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

class TOOLKIT_DLLPUBLIC SVTXNumericField final : public css::awt::XNumericField, public SVTXFormattedField
{
public:
                    SVTXNumericField();
                    virtual ~SVTXNumericField() override;

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                           SAL_CALL acquire() noexcept override  { SVTXFormattedField::acquire(); }
    void                           SAL_CALL release() noexcept override  { SVTXFormattedField::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

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

    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;
};

//  class VCLXListBox

typedef cppu::ImplInheritanceHelper< VCLXWindow,
                                     css::awt::XListBox,
                                     css::awt::XTextLayoutConstrains,
                                     css::awt::XItemListListener
                                   > VCLXListBox_Base;
class VCLXListBox final : public VCLXListBox_Base
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;

    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
                    CreateAccessibleContext() override;
    void            ImplCallItemListeners();

public:
                        VCLXListBox();

    // css::lang::XComponent
    void SAL_CALL dispose(  ) override;

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

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

    // XItemListListener
    virtual void SAL_CALL listItemInserted( const css::awt::ItemListEvent& Event ) override;
    virtual void SAL_CALL listItemRemoved( const css::awt::ItemListEvent& Event ) override;
    virtual void SAL_CALL listItemModified( const css::awt::ItemListEvent& Event ) override;
    virtual void SAL_CALL allItemsRemoved( const css::lang::EventObject& Event ) override;
    virtual void SAL_CALL itemListChanged( const css::lang::EventObject& Event ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& i_rEvent ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXComboBox

typedef cppu::ImplInheritanceHelper< VCLXEdit,
                                     css::awt::XComboBox,
                                     css::awt::XItemListListener
                                   > VCLXComboBox_Base;
class VCLXComboBox final : public VCLXComboBox_Base
{
private:
    ActionListenerMultiplexer   maActionListeners;
    ItemListenerMultiplexer     maItemListeners;

    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;


public:
                        VCLXComboBox();
    virtual ~VCLXComboBox() override;

     // css::lang::XComponent
    void SAL_CALL dispose(  ) override;

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

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

    // XItemListListener
    virtual void SAL_CALL listItemInserted( const css::awt::ItemListEvent& Event ) override;
    virtual void SAL_CALL listItemRemoved( const css::awt::ItemListEvent& Event ) override;
    virtual void SAL_CALL listItemModified( const css::awt::ItemListEvent& Event ) override;
    virtual void SAL_CALL allItemsRemoved( const css::lang::EventObject& Event ) override;
    virtual void SAL_CALL itemListChanged( const css::lang::EventObject& Event ) override;
    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& i_rEvent ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

//  class VCLXToolBox
class VCLXToolBox final : public VCLXWindow
{
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
                        CreateAccessibleContext() override;

public:
                        VCLXToolBox();
                        virtual ~VCLXToolBox() override;
};

class VCLXHeaderBar final : public VCLXWindow
{
public:
    VCLXHeaderBar();
    virtual ~VCLXHeaderBar() override;

    virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
