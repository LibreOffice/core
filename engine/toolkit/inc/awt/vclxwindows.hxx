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

#include <com/sun/star/awt/XCurrencyField.hpp>
#include <com/sun/star/awt/XDateField.hpp>
#include <com/sun/star/awt/XDialog2.hpp>
#include <com/sun/star/awt/XMessageBox.hpp>
#include <com/sun/star/awt/XMetricField.hpp>
#include <com/sun/star/awt/XNumericField.hpp>
#include <com/sun/star/awt/XPatternField.hpp>
#include <com/sun/star/awt/XProgressBar.hpp>
#include <com/sun/star/awt/XSimpleTabController.hpp>
#include <com/sun/star/awt/XTimeField.hpp>
#include <com/sun/star/awt/grid/XGridControl.hpp>
#include <com/sun/star/awt/grid/XGridRowSelection.hpp>
#include <com/sun/star/awt/grid/XGridDataListener.hpp>
#include <com/sun/star/awt/grid/GridDataEvent.hpp>
#include <com/sun/star/awt/grid/XGridSelectionListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/Date.hpp>

#include <cppuhelper/implbase.hxx>

#include <awt/vclxtopwindow.hxx>
#include <toolkit/awt/vclxwindows.hxx>

class FormatterBase;
class TabControl;
class TabPage;
class Edit;

//  class VCLXImageControl
class VCLXImageControl final : public VCLXGraphicControl
{
public:
                    VCLXImageControl();
                    virtual ~VCLXImageControl() override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& rNewSize ) override;

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }

private:
    virtual void    ImplSetNewImage() override;
};

//  class VCLXMessageBox
class VCLXMessageBox final :
                        public cppu::ImplInheritanceHelper<VCLXTopWindow, css::awt::XMessageBox>
{
public:
                        VCLXMessageBox();
                        virtual ~VCLXMessageBox() override;


    // css::awt::XMessageBox
    void setCaptionText( const OUString& aText ) override;
    OUString getCaptionText(  ) override;
    void setMessageText( const OUString& aText ) override;
    OUString getMessageText(  ) override;
    sal_Int16 execute(  ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize() override;

    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override;
};

//  class VCLXFrame
class VCLXFrame final : public VCLXContainer
{
    void                        ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

public:
    VCLXFrame();
    virtual ~VCLXFrame() override;

    // css::awt::XView
    void draw( sal_Int32 nX, sal_Int32 nY ) override;

    // css::awt::XVclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

//  class VCLXDialog
class VCLXDialog final : public cppu::ImplInheritanceHelper<VCLXTopWindow, css::awt::XDialog2>
{
public:
                        VCLXDialog();
                        virtual ~VCLXDialog() override;

    // css::awt::XDialog2
    virtual void endDialog( ::sal_Int32 Result ) override;
    virtual void setHelpId( const OUString& Id ) override;

    // css::awt::XDialog
    void setTitle( const OUString& Title ) override;
    OUString getTitle(  ) override;
    sal_Int16 execute(  ) override;
    void endExecute(  ) override;

    // css::awt::XView
    void draw( sal_Int32 nX, sal_Int32 nY ) override;

    // css::awt::XDevice,
    css::awt::DeviceInfo getInfo() override;

    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override;

    // css::awt::XVclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;

};

//  class VCLXTabPage
class VCLXTabPage final : public VCLXContainer
{
public:
                        VCLXTabPage();
                        virtual ~VCLXTabPage() override;

    // css::awt::XView
    void draw( sal_Int32 nX, sal_Int32 nY ) override;

    // css::awt::XVclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;

    /// @throws cpo::uno::RuntimeException
    TabPage*  getTabPage() const;
    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

class VCLXMultiPage final :
    public cppu::ImplInheritanceHelper<VCLXContainer, css::awt::XSimpleTabController>
{
    TabListenerMultiplexer maTabListeners;
    sal_Int32 mTabId;

    void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
public:
    VCLXMultiPage();
    virtual ~VCLXMultiPage() override;

    // css::lang::XComponent
    void dispose(  ) override;

    // css::awt::XView
    void draw( sal_Int32 nX, sal_Int32 nY ) override;

    // css::awt::XVclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;
    // XSimpleTabController
    virtual ::sal_Int32 insertTab() override;
    virtual void removeTab( ::sal_Int32 ID ) override;

    virtual void setTabProps( ::sal_Int32 ID, const cpo::uno::Sequence< css::beans::NamedValue >& Properties ) override;
    virtual cpo::uno::Sequence< css::beans::NamedValue > getTabProps( ::sal_Int32 ID ) override;

    virtual void activateTab( ::sal_Int32 ID ) override;
    virtual ::sal_Int32 getActiveTabID() override;

    virtual void addTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) override;
    virtual void removeTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) override;
    // C++
    /// @throws cpo::uno::RuntimeException
    TabControl*  getTabControl() const;
    sal_uInt16 insertTab( TabPage*, OUString const & sTitle );
    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

//  class VCLXProgressBar
class VCLXProgressBar final : public cppu::ImplInheritanceHelper<VCLXWindow, css::awt::XProgressBar>
{
private:
    sal_Int32   m_nValue;
    sal_Int32   m_nValueMin;
    sal_Int32   m_nValueMax;

    void            ImplUpdateValue();

public:
                    VCLXProgressBar();
                    virtual ~VCLXProgressBar() override;

    // css::awt::XProgressBar
    void setForegroundColor( sal_Int32 nColor ) override;
    void setBackgroundColor( sal_Int32 nColor ) override;
    void setValue( sal_Int32 nValue ) override;
    void setRange( sal_Int32 nMin, sal_Int32 nMax ) override;
    sal_Int32 getValue() override;

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXFormattedSpinField
class VCLXFormattedSpinField : public VCLXSpinField
{
private:
    FormatterBase*  mpFormatter;

protected:
    FormatterBase*  GetFormatter() const { return GetWindow() ? mpFormatter : nullptr; }

public:
                    VCLXFormattedSpinField();
                    virtual ~VCLXFormattedSpinField() override;

    void            SetFormatter( FormatterBase* pFormatter ) { mpFormatter = pFormatter; }

    void            setStrictFormat( bool bStrict );
    bool            isStrictFormat() const;

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXDateField

class VCLXDateField :
    public cppu::ImplInheritanceHelper<VCLXFormattedSpinField, css::awt::XDateField>
{
public:
                    VCLXDateField();
                    virtual ~VCLXDateField() override;


    // css::awt::XDateField
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

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXTimeField

class VCLXTimeField final :
    public cppu::ImplInheritanceHelper<VCLXFormattedSpinField, css::awt::XTimeField>
{
public:
                    VCLXTimeField();
                    virtual ~VCLXTimeField() override;

    // css::awt::XTimeField
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

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXNumericField

class VCLXNumericField final :
    public cppu::ImplInheritanceHelper<VCLXFormattedSpinField, css::awt::XNumericField>
{
public:
                    VCLXNumericField();
                    virtual ~VCLXNumericField() override;

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

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


//  class VCLXMetricField

class MetricFormatter;
class MetricField;
class VCLXMetricField final :
    public cppu::ImplInheritanceHelper<VCLXFormattedSpinField, css::awt::XMetricField>
{
    /// @throws cpo::uno::RuntimeException
    MetricFormatter *GetMetricFormatter();
    /// @throws cpo::uno::RuntimeException
    MetricField     *GetMetricField();
    void CallListeners();
public:
    VCLXMetricField();
    virtual ~VCLXMetricField() override;

    // css::awt::XMetricField
    virtual void setValue( ::sal_Int64 Value, ::sal_Int16 Unit ) override;
    virtual void setUserValue( ::sal_Int64 Value, ::sal_Int16 Unit ) override;
    virtual ::sal_Int64 getValue( ::sal_Int16 Unit ) override;
    virtual ::sal_Int64 getCorrectedValue( ::sal_Int16 Unit ) override;
    virtual void setMin( ::sal_Int64 Value, ::sal_Int16 Unit ) override;
    virtual ::sal_Int64 getMin( ::sal_Int16 Unit ) override;
    virtual void setMax( ::sal_Int64 Value, ::sal_Int16 Unit ) override;
    virtual ::sal_Int64 getMax( ::sal_Int16 Unit ) override;
    virtual void setFirst( ::sal_Int64 Value, ::sal_Int16 Unit ) override;
    virtual ::sal_Int64 getFirst( ::sal_Int16 Unit ) override;
    virtual void setLast( ::sal_Int64 Value, ::sal_Int16 Unit ) override;
    virtual ::sal_Int64 getLast( ::sal_Int16 Unit ) override;
    virtual void setSpinSize( ::sal_Int64 Value ) override;
    virtual ::sal_Int64 getSpinSize(  ) override;
    virtual void setDecimalDigits( ::sal_Int16 nDigits ) override;
    virtual ::sal_Int16 getDecimalDigits(  ) override;
    virtual void setStrictFormat( bool bStrict ) override;
    virtual bool isStrictFormat(  ) override;

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

//  class VCLXPatternField
class VCLXPatternField final :
    public cppu::ImplInheritanceHelper<VCLXFormattedSpinField, css::awt::XPatternField>
{
public:
                    VCLXPatternField();
                    virtual ~VCLXPatternField() override;


    // css::awt::XPatternField
    void setMasks( const OUString& EditMask, const OUString& LiteralMask ) override;
    void getMasks( OUString& EditMask, OUString& LiteralMask ) override;
    void setString( const OUString& Str ) override;
    OUString getString(  ) override;
    void setStrictFormat( bool bStrict ) override;
    bool isStrictFormat(  ) override;

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


class VCLXFileControl final : public cppu::ImplInheritanceHelper<VCLXWindow, css::awt::XTextComponent, css::awt::XTextLayoutConstrains>
{
    DECL_LINK(ModifyHdl, Edit&, void);
    void ModifyHdl();
    TextListenerMultiplexer maTextListeners;

public:
                    VCLXFileControl();
                    virtual ~VCLXFileControl() override;

    virtual void SetWindow( const VclPtr< vcl::Window > &pWindow ) override;

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

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::awt::XTextLayoutConstrains
    css::awt::Size getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) override;
    void getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) override;

    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

class SVTXCurrencyField final :
    public cppu::ImplInheritanceHelper<SVTXFormattedField, css::awt::XCurrencyField>
{
public:
                    SVTXCurrencyField();
                    virtual ~SVTXCurrencyField() override;

    // css::awt::XVclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

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

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


class SVTXDateField final : public VCLXDateField
{
public:
                    SVTXDateField();
                    virtual ~SVTXDateField() override;

    // css::awt::VclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

namespace svt::table {
    class TableControl;
    class UnoControlTableModel;
}

typedef ::cppu::ImplInheritanceHelper  <   VCLXWindow
                                        ,   css::awt::grid::XGridControl
                                        ,   css::awt::grid::XGridRowSelection
                                        ,   css::awt::grid::XGridDataListener
                                        ,   css::container::XContainerListener
                                        >   SVTXGridControl_Base;
class SVTXGridControl final : public SVTXGridControl_Base
{
public:
    SVTXGridControl();
    virtual ~SVTXGridControl() override;

    // XGridDataListener
    virtual void rowsInserted( const css::awt::grid::GridDataEvent& Event ) override;
    virtual void rowsRemoved( const css::awt::grid::GridDataEvent& Event ) override;
    virtual void dataChanged( const css::awt::grid::GridDataEvent& Event ) override;
    virtual void rowHeadingChanged( const css::awt::grid::GridDataEvent& Event ) override;

    // XContainerListener
    virtual void elementInserted( const css::container::ContainerEvent& Event ) override;
    virtual void elementRemoved( const css::container::ContainerEvent& Event ) override;
    virtual void elementReplaced( const css::container::ContainerEvent& Event ) override;

    // XEventListener
    virtual void disposing( const css::lang::EventObject& Source ) override;

    // XGridControl
    virtual ::sal_Int32 getRowAtPoint(::sal_Int32 x, ::sal_Int32 y) override;
    virtual ::sal_Int32 getColumnAtPoint(::sal_Int32 x, ::sal_Int32 y) override;
    virtual ::sal_Int32 getCurrentColumn(  ) override;
    virtual ::sal_Int32 getCurrentRow(  ) override;
    virtual void goToCell( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex ) override;

    // XGridRowSelection
    virtual void selectRow( ::sal_Int32 i_rowIndex ) override;
    virtual void selectAllRows() override;
    virtual void deselectRow( ::sal_Int32 i_rowIndex ) override;
    virtual void deselectAllRows() override;
    virtual cpo::uno::Sequence< ::sal_Int32 > getSelectedRows() override;
    virtual bool hasSelectedRows() override;
    virtual bool isRowSelected(::sal_Int32 index) override;
    virtual void addSelectionListener(const css::uno::Reference< css::awt::grid::XGridSelectionListener > & listener) override;
    virtual void removeSelectionListener(const css::uno::Reference< css::awt::grid::XGridSelectionListener > & listener) override;

    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    // css::lang::XComponent
    void dispose(  ) override;

    // XWindow
    void setEnable( bool bEnable ) override;

private:
    // VCLXWindow
    virtual void    SetWindow( const VclPtr< vcl::Window > &pWindow ) override;

    void    impl_updateColumnsFromModel_nothrow();
    void    impl_checkTableModelInit();

    void    impl_checkColumnIndex_throw( ::svt::table::TableControl const & i_table, sal_Int32 const i_columnIndex ) const;
    void    impl_checkRowIndex_throw( ::svt::table::TableControl const & i_table, sal_Int32 const i_rowIndex ) const;

    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    void            ImplCallItemListeners();

    std::shared_ptr< ::svt::table::UnoControlTableModel >   m_xTableModel;
    bool                                                    m_bTableModelInitCompleted;
    SelectionListenerMultiplexer                            m_aSelectionListeners;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
