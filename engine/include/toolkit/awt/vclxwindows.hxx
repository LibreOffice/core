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

#include <com/sun/star/awt/XTextArea.hpp>
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

#include <svl/numuno.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <tools/lineend.hxx>

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
    void setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) override;

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

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

public:
                    VCLXButton();
    virtual ~VCLXButton() override;

    // css::lang::XComponent
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

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

//  class VCLXCheckBox

class UNLESS_MERGELIBS(TOOLKIT_DLLPUBLIC) VCLXCheckBox final : public cppu::ImplInheritanceHelper<
                        VCLXGraphicControl,
                        css::awt::XCheckBox,
                        css::awt::XButton>
{
private:
    ActionListenerMultiplexer   maActionListeners;
    OUString             maActionCommand;
    ItemListenerMultiplexer     maItemListeners;

    void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

public:
                    VCLXCheckBox();


    // css::lang::XComponent
    void dispose(  ) override;

    // css::awt::XCheckBox
    void addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    sal_Int16 getState(  ) override;
    void setState( sal_Int16 n ) override;
    void setLabel( const OUString& Label ) override;
    void enableTriState( bool b ) override;

    // css::awt::XButton:
    void addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void setActionCommand( const OUString& Command ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXRadioButton

class UNLESS_MERGELIBS(TOOLKIT_DLLPUBLIC) VCLXRadioButton final : public cppu::ImplInheritanceHelper<
                        VCLXGraphicControl,
                        css::awt::XRadioButton,
                        css::awt::XButton>
{
private:
    ItemListenerMultiplexer     maItemListeners;
    ActionListenerMultiplexer   maActionListeners;
    OUString             maActionCommand;

    void            ImplClickedOrToggled( bool bToggled );
    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

public:
                    VCLXRadioButton();

    // css::lang::XComponent
    void dispose(  ) override;

    // css::awt::XRadioButton
    void addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    void removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    bool getState(  ) override;
    void setState( bool b ) override;
    void setLabel( const OUString& Label ) override;

    // css::awt::XButton:
    void addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) override;
    void setActionCommand( const OUString& Command ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }

};

//  class VCLXFixedHyperlink
class VCLXFixedHyperlink final :
    public cppu::ImplInheritanceHelper<VCLXWindow, css::awt::XFixedHyperlink>
{
private:
    ActionListenerMultiplexer   maActionListeners;

    void                        ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

public:
    VCLXFixedHyperlink();
    virtual ~VCLXFixedHyperlink() override;

    // css::lang::XComponent
    void dispose(  ) override;

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
    css::awt::Size calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXFixedText

class VCLXFixedText final : public cppu::ImplInheritanceHelper<VCLXWindow, css::awt::XFixedText>
{
public:
                        VCLXFixedText();
                        virtual ~VCLXFixedText() override;

    // css::awt::XFixedText
    void setText( const OUString& Text ) override;
    OUString getText(  ) override;
    void setAlignment( sal_Int16 nAlign ) override;
    sal_Int16 getAlignment(  ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXScrollBar

class UNLESS_MERGELIBS(TOOLKIT_DLLPUBLIC) VCLXScrollBar final :
                        public cppu::ImplInheritanceHelper<VCLXWindow, css::awt::XScrollBar>
{
private:
    AdjustmentListenerMultiplexer maAdjustmentListeners;

    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

public:
                    VCLXScrollBar();


    // css::lang::XComponent
    void dispose(  ) override;

    // css::awt::XScrollbar
    void addAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& l ) override;
    void removeAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& l ) override;
    void setValue( sal_Int32 n ) override;
    void setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) override;
    sal_Int32 getValue(  ) override;
    void setMaximum( sal_Int32 n ) override;
    sal_Int32 getMaximum(  ) override;
    void setLineIncrement( sal_Int32 n ) override;
    sal_Int32 getLineIncrement(  ) override;
    void setBlockIncrement( sal_Int32 n ) override;
    sal_Int32 getBlockIncrement(  ) override;
    void setVisibleSize( sal_Int32 n ) override;
    sal_Int32 getVisibleSize(  ) override;
    void setOrientation( sal_Int32 n ) override;
    sal_Int32 getOrientation(  ) override;

    // why isn't this part of the XScrollbar?
    /// @throws cpo::uno::RuntimeException
    void setMinimum( sal_Int32 n );
    /// @throws cpo::uno::RuntimeException
    sal_Int32 getMinimum(  ) const;

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize() override;
    /// @throws cpo::uno::RuntimeException
    static css::awt::Size implGetMinimumSize( vcl::Window const * p );

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXEdit

class UNLESS_MERGELIBS(SAL_DLLPUBLIC_RTTI) VCLXEdit :  public cppu::ImplInheritanceHelper<
                    VCLXWindow,
                    css::awt::XTextComponent,
                    css::awt::XTextEditField,
                    css::awt::XTextLayoutConstrains>
{
private:
    TextListenerMultiplexer maTextListeners;

protected:
    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

public:
                    VCLXEdit();

    TextListenerMultiplexer&    GetTextListeners() { return maTextListeners; }


    // css::lang::XComponent
    void dispose(  ) override;

    // css::awt::XTextComponent
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

    // css::awt::XTextEditField:
    void setEchoChar( sal_Unicode cEcho ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) override;
    void getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) override;

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

class VCLXMultiLineEdit final : public cppu::ImplInheritanceHelper<
                                    VCLXWindow,
                                    css::awt::XTextComponent,
                                    css::awt::XTextArea,
                                    css::awt::XTextLayoutConstrains>
{
private:
    TextListenerMultiplexer maTextListeners;
    LineEnd                 meLineEndType;

    void                ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

public:
    VCLXMultiLineEdit();
    virtual ~VCLXMultiLineEdit() override;

    // css::awt::XTextComponent
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

    //XTextArea
    OUString getTextLines(  ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) override;
    void getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) override;

    // css::awt::XVclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    // css::awt::XWindow
    void setFocus(  ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

//  class VCLXSpinField
class VCLXSpinField : public cppu::ImplInheritanceHelper<VCLXEdit, css::awt::XSpinField>
{
private:
    SpinListenerMultiplexer maSpinListeners;

protected:
    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

public:
                    VCLXSpinField();


    // css::awt::XSpinField
    void addSpinListener( const css::uno::Reference< css::awt::XSpinListener >& l ) override;
    void removeSpinListener( const css::uno::Reference< css::awt::XSpinListener >& l ) override;
    void up(  ) override;
    void down(  ) override;
    void first(  ) override;
    void last(  ) override;
    void enableRepeat( bool bRepeat ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

class SVTXFormattedField : public VCLXSpinField
{
    rtl::Reference<SvNumberFormatsSupplierObj> m_xCurrentSupplier;
    bool                    bIsStandardSupplier;
    sal_Int32               nKeyToSetDelayed;

public:
    SVTXFormattedField();
    virtual ~SVTXFormattedField() override;

    // css::awt::XVclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

protected:
    void    setFormatsSupplier(const css::uno::Reference< css::util::XNumberFormatsSupplier > & xSupplier);
    sal_Int32   getFormatKey() const;
    void    setFormatKey(sal_Int32 nKey);

    void    SetValue(const cpo::uno::Any& rValue);
    cpo::uno::Any  GetValue() const;

    void    SetTreatAsNumber(bool bSet);
    bool    GetTreatAsNumber() const;

    void    SetDefaultValue(const cpo::uno::Any& rValue);
    cpo::uno::Any  GetDefaultValue() const;

    void    SetMinValue(const cpo::uno::Any& rValue);
    cpo::uno::Any  GetMinValue() const;

    void    SetMaxValue(const cpo::uno::Any& rValue);
    cpo::uno::Any  GetMaxValue() const;

    void    NotifyTextListeners();
    cpo::uno::Any  convertEffectiveValue(const cpo::uno::Any& rValue) const;

    virtual void    SetWindow( const VclPtr< vcl::Window > &_pWindow) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

class UNLESS_MERGELIBS(TOOLKIT_DLLPUBLIC) SVTXNumericField final : public cppu::ImplInheritanceHelper<SVTXFormattedField, css::awt::XNumericField>
{
public:
                    SVTXNumericField();
                    virtual ~SVTXNumericField() override;

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

    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override;
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
    void            ImplCallItemListeners();

public:
                        VCLXListBox();

    // css::lang::XComponent
    void dispose(  ) override;

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

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) override;
    void getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) override;

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    // XItemListListener
    virtual void listItemInserted( const css::awt::ItemListEvent& Event ) override;
    virtual void listItemRemoved( const css::awt::ItemListEvent& Event ) override;
    virtual void listItemModified( const css::awt::ItemListEvent& Event ) override;
    virtual void allItemsRemoved( const css::lang::EventObject& Event ) override;
    virtual void itemListChanged( const css::lang::EventObject& Event ) override;

    // XEventListener
    virtual void disposing( const css::lang::EventObject& i_rEvent ) override;

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

public:
                        VCLXComboBox();
    virtual ~VCLXComboBox() override;

     // css::lang::XComponent
    void dispose(  ) override;

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

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) override;
    void getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) override;

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    // XItemListListener
    virtual void listItemInserted( const css::awt::ItemListEvent& Event ) override;
    virtual void listItemRemoved( const css::awt::ItemListEvent& Event ) override;
    virtual void listItemModified( const css::awt::ItemListEvent& Event ) override;
    virtual void allItemsRemoved( const css::lang::EventObject& Event ) override;
    virtual void itemListChanged( const css::lang::EventObject& Event ) override;
    // XEventListener
    virtual void disposing( const css::lang::EventObject& i_rEvent ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
