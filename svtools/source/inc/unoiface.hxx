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

class VCLXMultiLineEdit :   public ::com::sun::star::awt::XTextComponent,
                            public ::com::sun::star::awt::XTextArea,
                            public ::com::sun::star::awt::XTextLayoutConstrains,
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

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { VCLXWindow::acquire(); }
    void                                        SAL_CALL release() throw() override  { VCLXWindow::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XTextComponent
    void SAL_CALL addTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setText( const OUString& aText ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL insertText( const ::com::sun::star::awt::Selection& Sel, const OUString& Text ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getText(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getSelectedText(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::awt::Selection SAL_CALL getSelection(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isEditable(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setEditable( sal_Bool bEditable ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMaxTextLen( sal_Int16 nLen ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getMaxTextLen(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XTextArea
    OUString SAL_CALL getTextLines(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XTextLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XWindow
    void SAL_CALL setFocus(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};



//  class VCLXFileControl

class VCLXFileControl : ::com::sun::star::awt::XTextComponent, public ::com::sun::star::awt::XTextLayoutConstrains, public VCLXWindow
{
protected:
    DECL_LINK_TYPED(ModifyHdl, Edit&, void);
    void ModifyHdl();
    TextListenerMultiplexer maTextListeners;

public:
                    VCLXFileControl();
                    virtual ~VCLXFileControl();

    virtual void SetWindow( const VclPtr< vcl::Window > &pWindow ) override;

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { VCLXWindow::acquire(); }
    void                                        SAL_CALL release() throw() override  { VCLXWindow::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XTextComponent
    void SAL_CALL addTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setText( const OUString& aText ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL insertText( const ::com::sun::star::awt::Selection& Sel, const OUString& Text ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getText(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getSelectedText(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::awt::Selection SAL_CALL getSelection(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isEditable(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setEditable( sal_Bool bEditable ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMaxTextLen( sal_Int16 nLen ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getMaxTextLen(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XTextLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    void SAL_CALL setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

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

    // ::com::sun::star::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  getFormatsSupplier() const;
    void    setFormatsSupplier(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & xSupplier);
    sal_Int32   getFormatKey() const;
    void    setFormatKey(sal_Int32 nKey);

    void    SetValue(const ::com::sun::star::uno::Any& rValue);
    ::com::sun::star::uno::Any  GetValue();

    void    SetTreatAsNumber(bool bSet);
    bool    GetTreatAsNumber();

    void    SetDefaultValue(const ::com::sun::star::uno::Any& rValue);
    ::com::sun::star::uno::Any  GetDefaultValue();

    void    SetMinValue(const ::com::sun::star::uno::Any& rValue);
    ::com::sun::star::uno::Any  GetMinValue();

    void    SetMaxValue(const ::com::sun::star::uno::Any& rValue);
    ::com::sun::star::uno::Any  GetMaxValue();

    void    NotifyTextListeners();
    ::com::sun::star::uno::Any  convertEffectiveValue(const ::com::sun::star::uno::Any& rValue);

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
                                        ,   ::com::sun::star::container::XContainerListener
                                        ,   ::com::sun::star::beans::XPropertyChangeListener
                                        ,   ::com::sun::star::awt::XItemEventBroadcaster
                                        >   SVTXRoadmap_Base;
class SVTXRoadmap : public SVTXRoadmap_Base


{
private:
    ItemListenerMultiplexer     maItemListeners;

    RMItemData CurRMItemData;
    static RMItemData GetRMItemData( const ::com::sun::star::container::ContainerEvent& _rEvent );

protected:
    virtual void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

    virtual ~SVTXRoadmap();

public:
    SVTXRoadmap();

    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override { VCLXWindow::disposing( Source ); }

    // ::com::sun::star::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    ::com::sun::star::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XContainerListener
    void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& rEvent )throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& rEvent )throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& rEvent )throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XItemEventBroadcaster
    virtual void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

protected:

    // VCLXGraphicControl overridables
    virtual void    ImplSetNewImage() override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};




//  class SVTXNumericField

class SVTXNumericField : public ::com::sun::star::awt::XNumericField, public SVTXFormattedField
{
public:
                    SVTXNumericField();
                    virtual ~SVTXNumericField();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { SVTXFormattedField::acquire(); }
    void                                        SAL_CALL release() throw() override  { SVTXFormattedField::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XNumericField
    void SAL_CALL setValue( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    double SAL_CALL getValue(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMin( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    double SAL_CALL getMin(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMax( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    double SAL_CALL getMax(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    double SAL_CALL getFirst(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setLast( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    double SAL_CALL getLast(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    double SAL_CALL getSpinSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setDecimalDigits( sal_Int16 nDigits ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getDecimalDigits(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isStrictFormat(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXCurrencyField

class SVTXCurrencyField : public ::com::sun::star::awt::XCurrencyField, public SVTXFormattedField
{
public:
                    SVTXCurrencyField();
                    virtual ~SVTXCurrencyField();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { SVTXFormattedField::acquire(); }
    void                                        SAL_CALL release() throw() override  { SVTXFormattedField::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XCurrencyField
    void SAL_CALL setValue( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    double SAL_CALL getValue(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMin( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    double SAL_CALL getMin(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMax( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    double SAL_CALL getMax(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    double SAL_CALL getFirst(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setLast( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    double SAL_CALL getLast(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    double SAL_CALL getSpinSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setDecimalDigits( sal_Int16 nDigits ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getDecimalDigits(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isStrictFormat(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXProgressBar

class VCLXProgressBar : public ::com::sun::star::awt::XProgressBar,
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

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { VCLXWindow::acquire(); }
    void                                        SAL_CALL release() throw() override  { VCLXWindow::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XProgressBar
    void SAL_CALL setForegroundColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setBackgroundColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setValue( sal_Int32 nValue ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setRange( sal_Int32 nMin, sal_Int32 nMax ) throw(::com::sun::star::uno::RuntimeException, std::exception ) override;
    sal_Int32 SAL_CALL getValue() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class SVTXDateField

class SVTXDateField : public VCLXDateField
{
public:
                    SVTXDateField();
                    virtual ~SVTXDateField();

    // ::com::sun::star::awt::VclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

#endif // INCLUDED_SVTOOLS_SOURCE_INC_UNOIFACE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
