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

#ifndef INCLUDED_SVTOOLS_SOURCE_INC_UNOIFACE_HXX
#define INCLUDED_SVTOOLS_SOURCE_INC_UNOIFACE_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>

#include <cppuhelper/typeprovider.hxx>

#include <com/sun/star/awt/XProgressBar.hpp>
#include <com/sun/star/awt/XTextArea.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <svtools/svmedit.hxx>
#include <vcl/fmtfield.hxx>


#include <cppuhelper/implbase.hxx>
#include <com/sun/star/awt/XItemEventBroadcaster.hpp>


namespace com { namespace sun { namespace star { namespace util {
    class XNumberFormatsSupplier;
} } } }

class SvNumberFormatsSupplierObj;


//  class VCLXMultiLineEdit

class VCLXMultiLineEdit :   public css::awt::XTextComponent,
                            public css::awt::XTextArea,
                            public css::awt::XTextLayoutConstrains,
                            public VCLXWindow
{
private:
    TextListenerMultiplexer maTextListeners;
    LineEnd                 meLineEndType;

protected:
    void                ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

public:
                    VCLXMultiLineEdit();
                    virtual ~VCLXMultiLineEdit() override;

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                           SAL_CALL acquire() throw() override  { VCLXWindow::acquire(); }
    void                           SAL_CALL release() throw() override  { VCLXWindow::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

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

    //XTextArea
    OUString SAL_CALL getTextLines(  ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) override;

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

    // css::awt::XWindow
    void SAL_CALL setFocus(  ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXFileControl

class VCLXFileControl final : public css::awt::XTextComponent, public css::awt::XTextLayoutConstrains, public VCLXWindow
{
    DECL_LINK(ModifyHdl, Edit&, void);
    void ModifyHdl();
    TextListenerMultiplexer maTextListeners;

public:
                    VCLXFileControl();
                    virtual ~VCLXFileControl() override;

    virtual void SetWindow( const VclPtr< vcl::Window > &pWindow ) override;

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                           SAL_CALL acquire() throw() override  { VCLXWindow::acquire(); }
    void                           SAL_CALL release() throw() override  { VCLXWindow::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

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

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) override;

    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class SVTXFormattedField


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
    css::uno::Any  GetValue();

    void    SetTreatAsNumber(bool bSet);
    bool    GetTreatAsNumber();

    void    SetDefaultValue(const css::uno::Any& rValue);
    css::uno::Any  GetDefaultValue();

    void    SetMinValue(const css::uno::Any& rValue);
    css::uno::Any  GetMinValue();

    void    SetMaxValue(const css::uno::Any& rValue);
    css::uno::Any  GetMaxValue();

    void    NotifyTextListeners();
    css::uno::Any  convertEffectiveValue(const css::uno::Any& rValue);

    virtual void    SetWindow( const VclPtr< vcl::Window > &_pWindow) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class SVTXRoadmap


namespace svt
{
    class ORoadmap;
}

struct RMItemData
{
    bool            b_Enabled;
    sal_Int32           n_ID;
    OUString     Label;
};

typedef ::cppu::ImplInheritanceHelper  <   VCLXGraphicControl
                                        ,   css::container::XContainerListener
                                        ,   css::beans::XPropertyChangeListener
                                        ,   css::awt::XItemEventBroadcaster
                                        >   SVTXRoadmap_Base;
class SVTXRoadmap final : public SVTXRoadmap_Base
{
public:
    SVTXRoadmap();

    void SAL_CALL disposing( const css::lang::EventObject& Source ) override { VCLXWindow::disposing( Source ); }

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;

    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

    // XContainerListener
    void SAL_CALL elementInserted( const css::container::ContainerEvent& rEvent ) override;
    void SAL_CALL elementRemoved( const css::container::ContainerEvent& rEvent ) override;
    void SAL_CALL elementReplaced( const css::container::ContainerEvent& rEvent ) override;

    // XItemEventBroadcaster
    virtual void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    virtual void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) override;

private:

    // VCLXGraphicControl overridables
    virtual void    ImplSetNewImage() override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }

    static RMItemData GetRMItemData( const css::container::ContainerEvent& _rEvent );

    virtual void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

    virtual ~SVTXRoadmap() override;

    ItemListenerMultiplexer     maItemListeners;
};


//  class SVTXNumericField

class SVTXNumericField : public css::awt::XNumericField, public SVTXFormattedField
{
public:
                    SVTXNumericField();
                    virtual ~SVTXNumericField() override;

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                           SAL_CALL acquire() throw() override  { SVTXFormattedField::acquire(); }
    void                           SAL_CALL release() throw() override  { SVTXFormattedField::release(); }

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
};


//  class VCLXCurrencyField

class SVTXCurrencyField : public css::awt::XCurrencyField, public SVTXFormattedField
{
public:
                    SVTXCurrencyField();
                    virtual ~SVTXCurrencyField() override;

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                           SAL_CALL acquire() throw() override  { SVTXFormattedField::acquire(); }
    void                           SAL_CALL release() throw() override  { SVTXFormattedField::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

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

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXProgressBar

class VCLXProgressBar final : public css::awt::XProgressBar,
                        public VCLXWindow
{
private:
    sal_Int32   m_nValue;
    sal_Int32   m_nValueMin;
    sal_Int32   m_nValueMax;

    void            ImplUpdateValue();

public:
                    VCLXProgressBar();
                    virtual ~VCLXProgressBar() override;

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                           SAL_CALL acquire() throw() override  { VCLXWindow::acquire(); }
    void                           SAL_CALL release() throw() override  { VCLXWindow::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    // css::awt::XProgressBar
    void SAL_CALL setForegroundColor( sal_Int32 nColor ) override;
    void SAL_CALL setBackgroundColor( sal_Int32 nColor ) override;
    void SAL_CALL setValue( sal_Int32 nValue ) override;
    void SAL_CALL setRange( sal_Int32 nMin, sal_Int32 nMax ) override;
    sal_Int32 SAL_CALL getValue() override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class SVTXDateField

class SVTXDateField : public VCLXDateField
{
public:
                    SVTXDateField();
                    virtual ~SVTXDateField() override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

#endif // INCLUDED_SVTOOLS_SOURCE_INC_UNOIFACE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
