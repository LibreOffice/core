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

#include <com/sun/star/awt/XTextArea.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#include <svtools/svmedit.hxx>
#include <svtools/fmtfield.hxx>


#include <comphelper/uno3.hxx>
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
                    virtual ~VCLXMultiLineEdit();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                           SAL_CALL acquire() throw() override  { VCLXWindow::acquire(); }
    void                           SAL_CALL release() throw() override  { VCLXWindow::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTextComponent
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

    //XTextArea
    OUString SAL_CALL getTextLines(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XWindow
    void SAL_CALL setFocus(  ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};



//  class VCLXFileControl

class VCLXFileControl : css::awt::XTextComponent, public css::awt::XTextLayoutConstrains, public VCLXWindow
{
protected:
    DECL_LINK_TYPED(ModifyHdl, Edit&, void);
    void ModifyHdl();
    TextListenerMultiplexer maTextListeners;

public:
                    VCLXFileControl();
                    virtual ~VCLXFileControl();

    virtual void SetWindow( const VclPtr< vcl::Window > &pWindow ) override;

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                           SAL_CALL acquire() throw() override  { VCLXWindow::acquire(); }
    void                           SAL_CALL release() throw() override  { VCLXWindow::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTextComponent
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

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(css::uno::RuntimeException, std::exception) override;

    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class SVTXFormattedField


class SVTXFormattedField : public VCLXSpinField
{
protected:
    SvNumberFormatsSupplierObj* m_pCurrentSupplier;
    bool                    bIsStandardSupplier;

    sal_Int32                   nKeyToSetDelayed;

public:
    SVTXFormattedField();
    virtual ~SVTXFormattedField();

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

protected:
    css::uno::Reference< css::util::XNumberFormatsSupplier >  getFormatsSupplier() const;
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

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
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
class SVTXRoadmap : public SVTXRoadmap_Base


{
private:
    ItemListenerMultiplexer     maItemListeners;

    RMItemData CurRMItemData;
    static RMItemData GetRMItemData( const css::container::ContainerEvent& _rEvent );

protected:
    virtual void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

    virtual ~SVTXRoadmap();

public:
    SVTXRoadmap();

    void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override { VCLXWindow::disposing( Source ); }

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;

    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    // XContainerListener
    void SAL_CALL elementInserted( const css::container::ContainerEvent& rEvent )throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL elementRemoved( const css::container::ContainerEvent& rEvent )throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL elementReplaced( const css::container::ContainerEvent& rEvent )throw(css::uno::RuntimeException, std::exception) override;

    // XItemEventBroadcaster
    virtual void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw (css::uno::RuntimeException, std::exception) override;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) throw (css::uno::RuntimeException, std::exception) override;

protected:

    // VCLXGraphicControl overridables
    virtual void    ImplSetNewImage() override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};




//  class SVTXNumericField

class SVTXNumericField : public css::awt::XNumericField, public SVTXFormattedField
{
public:
                    SVTXNumericField();
                    virtual ~SVTXNumericField();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                           SAL_CALL acquire() throw() override  { SVTXFormattedField::acquire(); }
    void                           SAL_CALL release() throw() override  { SVTXFormattedField::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

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

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXCurrencyField

class SVTXCurrencyField : public css::awt::XCurrencyField, public SVTXFormattedField
{
public:
                    SVTXCurrencyField();
                    virtual ~SVTXCurrencyField();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                           SAL_CALL acquire() throw() override  { SVTXFormattedField::acquire(); }
    void                           SAL_CALL release() throw() override  { SVTXFormattedField::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

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

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXProgressBar

class VCLXProgressBar : public css::awt::XProgressBar,
                        public VCLXWindow
{
private:
    sal_Int32   m_nValue;
    sal_Int32   m_nValueMin;
    sal_Int32   m_nValueMax;

protected:
    void            ImplUpdateValue();

public:
                    VCLXProgressBar();
                    virtual ~VCLXProgressBar();

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                           SAL_CALL acquire() throw() override  { VCLXWindow::acquire(); }
    void                           SAL_CALL release() throw() override  { VCLXWindow::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XProgressBar
    void SAL_CALL setForegroundColor( sal_Int32 nColor ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setBackgroundColor( sal_Int32 nColor ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setValue( sal_Int32 nValue ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setRange( sal_Int32 nMin, sal_Int32 nMax ) throw(css::uno::RuntimeException, std::exception ) override;
    sal_Int32 SAL_CALL getValue() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class SVTXDateField

class SVTXDateField : public VCLXDateField
{
public:
                    SVTXDateField();
                    virtual ~SVTXDateField();

    // css::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

#endif // INCLUDED_SVTOOLS_SOURCE_INC_UNOIFACE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
