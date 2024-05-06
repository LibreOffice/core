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

#include <svl/zforlist.hxx>

#include "DataBrowser.hxx"
#include "DataBrowserModel.hxx"
#include <strings.hrc>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <DiagramHelper.hxx>
#include <CommonConverters.hxx>
#include <NumberFormatterWrapper.hxx>
#include <servicenames_charttypes.hxx>
#include <ResId.hxx>
#include <bitmaps.hlst>
#include <helpids.h>
#include <ChartModel.hxx>
#include <ChartType.hxx>

#include <vcl/weld.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <svl/numformat.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/container/XIndexReplace.hpp>

#include <algorithm>
#include <limits>


using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

using namespace ::svt;

namespace
{
/*  BrowserMode::COLUMNSELECTION : single cells may be selected rather than only
                                   entire rows
    BrowserMode::(H|V)LINES : show horizontal or vertical grid-lines
    BrowserMode::AUTO_(H|V)SCROLL : scroll automated horizontally or vertically when
                                    cursor is moved beyond the edge of the dialog
    BrowserMode::HIDESELECT : Do not mark the current row with selection color
                              (usually blue)
  ! BrowserMode::HIDECURSOR would prevent flickering in edit fields, but navigating
        with shift up/down, and entering non-editable cells would be problematic,
        e.g.  the first cell, or when being in read-only mode
*/
const BrowserMode BrowserStdFlags = BrowserMode::COLUMNSELECTION |
                                    BrowserMode::HLINES | BrowserMode::VLINES |
                                    BrowserMode::AUTO_HSCROLL | BrowserMode::AUTO_VSCROLL |
                                    BrowserMode::HIDESELECT;

sal_Int32 lcl_getColumnInData( sal_uInt16 nCol )
{
    return static_cast< sal_Int32 >( nCol ) - 1;
}

} // anonymous namespace

namespace chart
{

namespace impl
{

class SeriesHeaderEdit
{
public:
    explicit SeriesHeaderEdit(std::unique_ptr<weld::Entry> xControl);

    void setStartColumn( sal_Int32 nStartColumn );
    sal_Int32 getStartColumn() const { return m_nStartColumn;}
    void SetShowWarningBox( bool bShowWarning );

    OUString GetText() const { return m_xControl->get_text(); }
    void SetText(const OUString& rText) { m_xControl->set_text(rText); }

    bool HasFocus() const { return m_xControl->has_focus(); }

    void set_size_request(int nWidth, int nHeight) { m_xControl->set_size_request(nWidth, nHeight); }
    void set_margin_start(int nLeft) { m_xControl->set_margin_start(nLeft); }

    void SetModifyHdl(const Link<SeriesHeaderEdit&,void>& rLink) { m_aModifyHdl = rLink; }
    void SetGetFocusHdl(const Link<SeriesHeaderEdit&,void>& rLink) { m_aFocusInHdl = rLink; }

private:
    DECL_LINK(NameEdited, weld::Entry&, void);
    DECL_LINK(NameFocusIn, weld::Widget&, void);
    DECL_LINK(MousePressHdl, const MouseEvent&, bool);

    std::unique_ptr<weld::Entry> m_xControl;
    Link<SeriesHeaderEdit&,void> m_aModifyHdl;
    Link<SeriesHeaderEdit&,void> m_aFocusInHdl;
    sal_Int32 m_nStartColumn;
    bool m_bShowWarningBox;
};

SeriesHeaderEdit::SeriesHeaderEdit(std::unique_ptr<weld::Entry> xControl)
    : m_xControl(std::move(xControl))
    , m_nStartColumn(0)
    , m_bShowWarningBox(false)
{
    m_xControl->set_help_id(HID_SCH_DATA_SERIES_LABEL);
    m_xControl->connect_changed(LINK(this, SeriesHeaderEdit, NameEdited));
    m_xControl->connect_focus_in(LINK(this, SeriesHeaderEdit, NameFocusIn));
    m_xControl->connect_mouse_press(LINK(this, SeriesHeaderEdit, MousePressHdl));
}

IMPL_LINK_NOARG(SeriesHeaderEdit, NameEdited, weld::Entry&, void)
{
    m_aModifyHdl.Call(*this);
}

IMPL_LINK_NOARG(SeriesHeaderEdit, NameFocusIn, weld::Widget&, void)
{
    m_aFocusInHdl.Call(*this);
}

void SeriesHeaderEdit::setStartColumn( sal_Int32 nStartColumn )
{
    m_nStartColumn = nStartColumn;
}

void SeriesHeaderEdit::SetShowWarningBox( bool bShowWarning )
{
    m_bShowWarningBox = bShowWarning;
}

IMPL_LINK_NOARG(SeriesHeaderEdit, MousePressHdl, const MouseEvent&, bool)
{
    if (m_bShowWarningBox)
    {
        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(m_xControl.get(),
                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                   SchResId(STR_INVALID_NUMBER)));
        xWarn->run();
    }

    return false;
}

class SeriesHeader
{
public:
    explicit SeriesHeader(weld::Container* pParent, weld::Container* pColorParent);
            ~SeriesHeader();

    void SetColor( const Color & rCol );
    void SetPos();
    void SetWidth( sal_Int32 nWidth );
    void SetChartType( const rtl::Reference< ::chart::ChartType > & xChartType,
                       bool bSwapXAndYAxis );
    void SetSeriesName( const OUString & rName );
    void SetRange( sal_Int32 nStartCol, sal_Int32 nEndCol );

    void SetPixelWidth( sal_Int32 nWidth );

    sal_Int32 GetStartColumn() const { return m_nStartCol;}
    sal_Int32 GetEndColumn() const { return m_nEndCol;}

    static const sal_Int32 nSymbolHeight = 10;
    static const sal_Int32 nSymbolDistance = 2;

    static sal_Int32 GetRelativeAppFontXPosForNameField() { return nSymbolHeight + nSymbolDistance; }

    void Show();
    void Hide();

    /** call this before destroying the class.  This notifies the listeners to
        changes of the edit field for the series name.
     */
    void applyChanges();

    void SetGetFocusHdl(const Link<SeriesHeaderEdit&,void>& rLink);

    void SetEditChangedHdl( const Link<SeriesHeaderEdit&,void> & rLink );

    bool HasFocus() const;

private:
    Timer m_aUpdateDataTimer;

    std::unique_ptr<weld::Builder> m_xBuilder1;
    std::unique_ptr<weld::Builder> m_xBuilder2;

    weld::Container* m_pParent;
    weld::Container* m_pColorParent;

    std::unique_ptr<weld::Container> m_xContainer1;
    std::unique_ptr<weld::Container> m_xContainer2;
    std::unique_ptr<weld::Image> m_spSymbol;
    std::unique_ptr<SeriesHeaderEdit> m_spSeriesName;
    std::unique_ptr<weld::Image> m_spColorBar;
    VclPtr< OutputDevice> m_xDevice;
    Link<SeriesHeaderEdit&,void> m_aChangeLink;
    Color m_aColor;

    void notifyChanges();
    DECL_LINK( ImplUpdateDataHdl, Timer*, void );
    DECL_LINK( SeriesNameEdited, SeriesHeaderEdit&, void );

    static OUString GetChartTypeImage(
        const rtl::Reference< ::chart::ChartType > & xChartType,
        bool bSwapXAndYAxis
        );

    sal_Int32 m_nStartCol, m_nEndCol;
    sal_Int32 m_nWidth;
    bool      m_bSeriesNameChangePending;
};

SeriesHeader::SeriesHeader(weld::Container* pParent, weld::Container* pColorParent)
    : m_aUpdateDataTimer( "SeriesHeader UpdateDataTimer" )
    , m_xBuilder1(Application::CreateBuilder(pParent, u"modules/schart/ui/columnfragment.ui"_ustr))
    , m_xBuilder2(Application::CreateBuilder(pColorParent, u"modules/schart/ui/imagefragment.ui"_ustr))
    , m_pParent(pParent)
    , m_pColorParent(pColorParent)
    , m_xContainer1(m_xBuilder1->weld_container(u"container"_ustr))
    , m_xContainer2(m_xBuilder2->weld_container(u"container"_ustr))
    , m_spSymbol(m_xBuilder1->weld_image(u"image"_ustr))
    , m_spSeriesName(new SeriesHeaderEdit(m_xBuilder1->weld_entry(u"entry"_ustr)))
    , m_spColorBar(m_xBuilder2->weld_image(u"image"_ustr))
    , m_xDevice(Application::GetDefaultDevice())
    , m_nStartCol( 0 )
    , m_nEndCol( 0 )
    , m_nWidth( 42 )
    , m_bSeriesNameChangePending( false )
{
    m_aUpdateDataTimer.SetInvokeHandler(LINK(this, SeriesHeader, ImplUpdateDataHdl));
    m_aUpdateDataTimer.SetTimeout(4 * EDIT_UPDATEDATA_TIMEOUT);

    m_spSeriesName->SetModifyHdl(LINK(this, SeriesHeader, SeriesNameEdited));
    Show();
}

SeriesHeader::~SeriesHeader()
{
    m_aUpdateDataTimer.Stop();
    m_pParent->move(m_xContainer1.get(), nullptr);
    m_pColorParent->move(m_xContainer2.get(), nullptr);
}

void SeriesHeader::notifyChanges()
{
    m_aChangeLink.Call(*m_spSeriesName);
    m_bSeriesNameChangePending = false;
}

void SeriesHeader::applyChanges()
{
    if( m_bSeriesNameChangePending )
    {
        notifyChanges();
    }
}

void SeriesHeader::SetColor( const Color & rCol )
{
    m_aColor = rCol;
}

void SeriesHeader::SetPos()
{
    // chart type symbol
    Size aSize( nSymbolHeight, nSymbolHeight );
    aSize = m_xDevice->LogicToPixel(aSize, MapMode(MapUnit::MapAppFont));
    m_spSymbol->set_size_request(aSize.Width(), aSize.Height());

    // series name edit field
    m_spSeriesName->set_margin_start(2);

    sal_Int32 nHeightPx = m_xDevice->LogicToPixel(Size(0, 12), MapMode(MapUnit::MapAppFont)).Height();
    m_spSeriesName->set_size_request(m_nWidth - aSize.Width() - 2, nHeightPx);

    // color bar
    nHeightPx = m_xDevice->LogicToPixel(Size(0, 3), MapMode(MapUnit::MapAppFont)).Height();
    m_spColorBar->set_size_request(m_nWidth, nHeightPx);

    ScopedVclPtr<VirtualDevice> xVirDev(m_spColorBar->create_virtual_device());
    xVirDev->SetOutputSizePixel(Size(m_nWidth, nHeightPx));
    xVirDev->SetFillColor(m_aColor);
    xVirDev->SetLineColor(m_aColor);
    xVirDev->DrawRect(tools::Rectangle(Point(0, 0), aSize));
    m_spColorBar->set_image(xVirDev.get());
}

void SeriesHeader::SetWidth( sal_Int32 nWidth )
{
    m_nWidth = nWidth;
    SetPos();
}

void SeriesHeader::SetPixelWidth( sal_Int32 nWidth )
{
    SetWidth(nWidth);
}

void SeriesHeader::SetChartType(
    const rtl::Reference< ChartType > & xChartType,
    bool bSwapXAndYAxis
)
{
    m_spSymbol->set_from_icon_name( GetChartTypeImage( xChartType, bSwapXAndYAxis ) );
}

void SeriesHeader::SetSeriesName( const OUString & rName )
{
    m_spSeriesName->SetText(rName);
}

void SeriesHeader::SetRange( sal_Int32 nStartCol, sal_Int32 nEndCol )
{
    m_nStartCol = nStartCol;
    m_nEndCol = std::max(nEndCol, nStartCol);
    m_spSeriesName->setStartColumn( nStartCol );
}

void SeriesHeader::Show()
{
    m_xContainer1->show();
    m_xContainer2->show();
}

void SeriesHeader::Hide()
{
    m_xContainer1->hide();
    m_xContainer2->hide();
}

void SeriesHeader::SetEditChangedHdl( const Link<SeriesHeaderEdit&,void> & rLink )
{
    m_aChangeLink = rLink;
}

IMPL_LINK_NOARG(SeriesHeader, ImplUpdateDataHdl, Timer*, void)
{
    notifyChanges();
}

IMPL_LINK_NOARG(SeriesHeader, SeriesNameEdited, SeriesHeaderEdit&, void)
{
    m_bSeriesNameChangePending = true;
    m_aUpdateDataTimer.Start();
}

void SeriesHeader::SetGetFocusHdl( const Link<SeriesHeaderEdit&,void>& rLink )
{
    m_spSeriesName->SetGetFocusHdl( rLink );
}

bool SeriesHeader::HasFocus() const
{
    return m_spSeriesName->HasFocus();
}

OUString SeriesHeader::GetChartTypeImage(
    const rtl::Reference< ChartType > & xChartType,
    bool bSwapXAndYAxis
)
{
    OUString aResult;
    if( !xChartType.is())
        return aResult;
    OUString aChartTypeName( xChartType->getChartType());

    if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_AREA )
    {
        aResult = BMP_TYPE_AREA;
    }
    else if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_COLUMN )
    {
        if( bSwapXAndYAxis )
            aResult = BMP_TYPE_BAR;
        else
            aResult = BMP_TYPE_COLUMN;
    }
    else if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_LINE )
    {
        aResult = BMP_TYPE_LINE;
    }
    else if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_SCATTER )
    {
        aResult = BMP_TYPE_XY;
    }
    else if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_PIE )
    {
        aResult = BMP_TYPE_PIE;
    }
    else if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_NET
          || aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET )
    {
        aResult = BMP_TYPE_NET;
    }
    else if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK )
    {
        // @todo: correct image for candle-stick type
        aResult = BMP_TYPE_STOCK;
    }
    else if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE )
    {
        aResult = BMP_TYPE_BUBBLE;
    }

    return aResult;
}

} // namespace impl

namespace
{

/** returns false, if no header as the focus.

    If a header has the focus, true is returned and the index of the header
    with focus is set at pIndex if pOutIndex is not 0.
*/
bool lcl_SeriesHeaderHasFocus(
    const std::vector< std::shared_ptr< ::chart::impl::SeriesHeader > > & rSeriesHeader,
    sal_Int32 * pOutIndex = nullptr )
{
    sal_Int32 nIndex = 0;
    for (auto const& elem : rSeriesHeader)
    {
        if(elem->HasFocus())
        {
            if( pOutIndex )
                *pOutIndex = nIndex;
            return true;
        }
        ++nIndex;
    }
    return false;
}

sal_Int32 lcl_getColumnInDataOrHeader(
    sal_uInt16 nCol, const std::vector< std::shared_ptr< ::chart::impl::SeriesHeader > > & rSeriesHeader )
{
    sal_Int32 nColIdx = 0;
    bool bHeaderHasFocus( lcl_SeriesHeaderHasFocus( rSeriesHeader, &nColIdx ));

    if( bHeaderHasFocus )
        nColIdx = lcl_getColumnInData( static_cast< sal_uInt16 >( rSeriesHeader[nColIdx]->GetStartColumn()));
    else
        nColIdx = lcl_getColumnInData( nCol );

    return nColIdx;
}

} // anonymous namespace

DataBrowser::DataBrowser(const css::uno::Reference<css::awt::XWindow> &rParent,
                         weld::Container* pColumns, weld::Container* pColors) :
    ::svt::EditBrowseBox(VCLUnoHelper::GetWindow(rParent),
            EditBrowseBoxFlags::SMART_TAB_TRAVEL | EditBrowseBoxFlags::HANDLE_COLUMN_TEXT,
            WB_BORDER | WB_TABSTOP, BrowserStdFlags ),
    m_nSeekRow( 0 ),
    m_bIsReadOnly( false ),
    m_bDataValid( true ),
    m_aNumberEditField(VclPtr<FormattedControl>::Create(&EditBrowseBox::GetDataWindow(), false)),
    m_aTextEditField(VclPtr<EditControl>::Create(&EditBrowseBox::GetDataWindow())),
    m_pColumnsWin(pColumns),
    m_pColorsWin(pColors),
    m_rNumberEditController( new ::svt::FormattedFieldCellController( m_aNumberEditField.get() )),
    m_rTextEditController( new ::svt::EditCellController( m_aTextEditField.get() ))
{
    Formatter& rFormatter = m_aNumberEditField->get_formatter();
    rFormatter.SetDefaultValue( std::numeric_limits<double>::quiet_NaN() );
    rFormatter.TreatAsNumber( true );
    RenewTable();
}

DataBrowser::~DataBrowser()
{
    disposeOnce();
}

void DataBrowser::dispose()
{
    m_aSeriesHeaders.clear();
    m_aNumberEditField.disposeAndClear();
    m_aTextEditField.disposeAndClear();
    ::svt::EditBrowseBox::dispose();
}

bool DataBrowser::MayInsertRow() const
{
    return ! IsReadOnly()
        && ( !lcl_SeriesHeaderHasFocus( m_aSeriesHeaders ));
}

bool DataBrowser::MayInsertColumn() const
{
    return ! IsReadOnly();
}

bool DataBrowser::MayDeleteRow() const
{
    return ! IsReadOnly()
        && ( !lcl_SeriesHeaderHasFocus( m_aSeriesHeaders ))
        && ( GetCurRow() >= 0 )
        && ( GetRowCount() > 1 );
}

bool DataBrowser::MayDeleteColumn() const
{
    // if a series header has the focus
    if( lcl_SeriesHeaderHasFocus( m_aSeriesHeaders ))
        return true;

    return ! IsReadOnly()
        && ( GetCurColumnId() > 1 )
        && ( ColCount() > 2 );
}

bool DataBrowser::MayMoveUpRows() const
{
    return ! IsReadOnly()
        && ( !lcl_SeriesHeaderHasFocus( m_aSeriesHeaders ))
        && ( GetCurRow() > 0 )
        && ( GetCurRow() <= GetRowCount() - 1 );
}

bool DataBrowser::MayMoveDownRows() const
{
    return ! IsReadOnly()
        && ( !lcl_SeriesHeaderHasFocus( m_aSeriesHeaders ))
        && ( GetCurRow() >= 0 )
        && ( GetCurRow() < GetRowCount() - 1 );
}

bool DataBrowser::MayMoveLeftColumns() const
{
    // if a series header (except the last one) has the focus
    {
        sal_Int32 nColIndex(0);
        if( lcl_SeriesHeaderHasFocus( m_aSeriesHeaders, &nColIndex ))
            return (o3tl::make_unsigned( nColIndex ) <= (m_aSeriesHeaders.size() - 1)) && (static_cast< sal_uInt32 >( nColIndex ) != 0);
    }

    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( GetCurColumnId(), m_aSeriesHeaders );
    return ! IsReadOnly()
        && ( nColIdx > 1 )
        && ( nColIdx <= ColCount() - 2 )
        && m_apDataBrowserModel
        && !m_apDataBrowserModel->isCategoriesColumn( nColIdx );
}

bool DataBrowser::MayMoveRightColumns() const
{
    // if a series header (except the last one) has the focus
    {
        sal_Int32 nColIndex(0);
        if( lcl_SeriesHeaderHasFocus( m_aSeriesHeaders, &nColIndex ))
            return (o3tl::make_unsigned( nColIndex ) < (m_aSeriesHeaders.size() - 1));
    }

    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( GetCurColumnId(), m_aSeriesHeaders );
    return ! IsReadOnly()
        && ( nColIdx > 0 )
        && ( nColIdx < ColCount()-2 )
        && m_apDataBrowserModel
        && !m_apDataBrowserModel->isCategoriesColumn( nColIdx );
}

void DataBrowser::clearHeaders()
{
    for( const auto& spHeader : m_aSeriesHeaders )
        spHeader->applyChanges();
    m_aSeriesHeaders.clear();
}

void DataBrowser::RenewTable()
{
    if (!m_apDataBrowserModel)
        return;

    sal_Int32  nOldRow     = GetCurRow();
    sal_uInt16 nOldColId   = GetCurColumnId();

    bool bLastUpdateMode = GetUpdateMode();
    SetUpdateMode( false );

    if( IsModified() )
        SaveModified();

    DeactivateCell();

    RemoveColumns();
    RowRemoved( 1, GetRowCount() );

    // for row numbers
    InsertHandleColumn( static_cast< sal_uInt16 >(
                            GetDataWindow().LogicToPixel( Size( 42, 0 )).getWidth() ));

    OUString aDefaultSeriesName(SchResId(STR_COLUMN_LABEL));
    replaceParamterInString( aDefaultSeriesName, u"%COLUMNNUMBER", OUString::number( 24 ) );
    sal_Int32 nColumnWidth = GetDataWindow().GetTextWidth( aDefaultSeriesName )
        + GetDataWindow().LogicToPixel(Point(8 + impl::SeriesHeader::GetRelativeAppFontXPosForNameField(), 0), MapMode(MapUnit::MapAppFont)).X();
    sal_Int32 nColumnCount = m_apDataBrowserModel->getColumnCount();
    // nRowCount is a member of a base class
    sal_Int32 nRowCountLocal = m_apDataBrowserModel->getMaxRowCount();
    for( sal_Int32 nColIdx=1; nColIdx<=nColumnCount; ++nColIdx )
    {
        InsertDataColumn( static_cast< sal_uInt16 >( nColIdx ), GetColString( nColIdx ), nColumnWidth );
    }

    RowInserted( 1, nRowCountLocal );
    GoToRow( std::min( nOldRow, GetRowCount() - 1 ));
    GoToColumnId( std::min( nOldColId, static_cast< sal_uInt16 >( ColCount() - 1 )));

    // fill series headers
    clearHeaders();
    const DataBrowserModel::tDataHeaderVector& aHeaders( m_apDataBrowserModel->getDataHeaders());
    Link<impl::SeriesHeaderEdit&,void> aFocusLink( LINK( this, DataBrowser, SeriesHeaderGotFocus ));
    Link<impl::SeriesHeaderEdit&,void> aSeriesHeaderChangedLink( LINK( this, DataBrowser, SeriesHeaderChanged ));

    for (auto const& elemHeader : aHeaders)
    {
        auto spHeader = std::make_shared<impl::SeriesHeader>( m_pColumnsWin, m_pColorsWin );
        Color nColor;
        // @todo: Set "DraftColor", i.e. interpolated colors for gradients, bitmaps, etc.
        if( elemHeader.m_xDataSeries.is() &&
            ( elemHeader.m_xDataSeries->getPropertyValue( u"Color"_ustr ) >>= nColor ))
            spHeader->SetColor( nColor );
        spHeader->SetChartType( elemHeader.m_xChartType, elemHeader.m_bSwapXAndYAxis );
        spHeader->SetSeriesName(
            elemHeader.m_xDataSeries->getLabelForRole(
                        elemHeader.m_xChartType.is() ?
                         elemHeader.m_xChartType->getRoleOfSequenceForSeriesLabel() :
                         u"values-y"_ustr));
        // index is 1-based, as 0 is for the column that contains the row-numbers
        spHeader->SetRange( elemHeader.m_nStartColumn + 1, elemHeader.m_nEndColumn + 1 );
        spHeader->SetGetFocusHdl( aFocusLink );
        spHeader->SetEditChangedHdl( aSeriesHeaderChangedLink );
        m_aSeriesHeaders.push_back( spHeader );
    }

    ImplAdjustHeaderControls();
    SetUpdateMode( bLastUpdateMode );
    ActivateCell();
    Invalidate();
}

OUString DataBrowser::GetColString( sal_Int32 nColumnId ) const
{
    OSL_ASSERT(m_apDataBrowserModel);
    if( nColumnId > 0 )
        return m_apDataBrowserModel->getRoleOfColumn( nColumnId - 1 );
    return OUString();
}

OUString DataBrowser::GetCellText( sal_Int32 nRow, sal_uInt16 nColumnId ) const
{
    OUString aResult;

    if( nColumnId == 0 )
    {
        aResult = OUString::number(nRow + 1);
    }
    else if( nRow >= 0 && m_apDataBrowserModel)
    {
        sal_Int32 nColIndex = static_cast< sal_Int32 >( nColumnId ) - 1;

        if( m_apDataBrowserModel->getCellType( nColIndex ) == DataBrowserModel::NUMBER )
        {
            double fData( m_apDataBrowserModel->getCellNumber( nColIndex, nRow ));

            if( ! std::isnan( fData ) &&
                m_spNumberFormatterWrapper )
            {
                bool bColorChanged = false;
                Color nLabelColor;
                aResult = m_spNumberFormatterWrapper->getFormattedString(
                                      GetNumberFormatKey( nColumnId ),
                                      fData, nLabelColor, bColorChanged );
            }
        }
        else if( m_apDataBrowserModel->getCellType( nColIndex ) == DataBrowserModel::TEXTORDATE )
        {
            uno::Any aAny = m_apDataBrowserModel->getCellAny( nColIndex, nRow );
            OUString aText;
            double fDouble=0.0;
            if( aAny>>=aText )
                aResult = aText;
            else if( aAny>>=fDouble )
            {
                if( ! std::isnan( fDouble ) && m_spNumberFormatterWrapper )
                {
                    // If a numberformat was available here we could directly
                    // obtain the corresponding edit format in
                    // getDateTimeInputNumberFormat() instead of doing the
                    // guess work.
                    sal_Int32 nNumberFormat = DiagramHelper::getDateTimeInputNumberFormat(
                            m_xChartDoc, fDouble );
                    Color nLabelColor;
                    bool bColorChanged = false;
                    aResult = m_spNumberFormatterWrapper->getFormattedString(
                        nNumberFormat, fDouble, nLabelColor, bColorChanged );
                }
            }
        }
        else
        {
            OSL_ASSERT( m_apDataBrowserModel->getCellType( nColIndex ) == DataBrowserModel::TEXT );
            aResult = m_apDataBrowserModel->getCellText( nColIndex, nRow );
        }
    }

    return aResult;
}

double DataBrowser::GetCellNumber( sal_Int32 nRow, sal_uInt16 nColumnId ) const
{
    if(( nColumnId >= 1 ) && ( nRow >= 0 ) && m_apDataBrowserModel)
    {
        return m_apDataBrowserModel->getCellNumber(
            static_cast< sal_Int32 >( nColumnId ) - 1, nRow );
    }

    return std::numeric_limits<double>::quiet_NaN();
}

void DataBrowser::Resize()
{
    bool bLastUpdateMode = GetUpdateMode();
    SetUpdateMode( false );

    ::svt::EditBrowseBox::Resize();
    ImplAdjustHeaderControls();
    SetUpdateMode( bLastUpdateMode );
}

void DataBrowser::SetReadOnly( bool bNewState )
{
    if( m_bIsReadOnly != bNewState )
    {
        m_bIsReadOnly = bNewState;
        Invalidate();
        DeactivateCell();
    }
}

void DataBrowser::CursorMoved()
{
    EditBrowseBox::CursorMoved();

    if( GetUpdateMode() )
        m_aCursorMovedHdlLink.Call( this );
}

void DataBrowser::MouseButtonDown( const BrowserMouseEvent& rEvt )
{
    if( !m_bDataValid )
        ShowWarningBox();
    else
        EditBrowseBox::MouseButtonDown( rEvt );
}

void DataBrowser::ShowWarningBox()
{
    std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetFrameWeld(),
                                               VclMessageType::Warning, VclButtonsType::Ok,
                                               SchResId(STR_INVALID_NUMBER)));
    xWarn->run();
}

bool DataBrowser::ShowQueryBox()
{
    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                   VclMessageType::Question, VclButtonsType::YesNo,
                                                   SchResId(STR_DATA_EDITOR_INCORRECT_INPUT)));
    return xQueryBox->run() == RET_YES;
}

bool DataBrowser::IsDataValid() const
{
    bool bValid = true;
    const sal_Int32 nCol = lcl_getColumnInData( GetCurColumnId());

    if( m_apDataBrowserModel->getCellType( nCol ) == DataBrowserModel::NUMBER )
    {
        sal_uInt32 nDummy = 0;
        double fDummy = 0.0;
        OUString aText(m_aNumberEditField->get_widget().get_text());

        if( !aText.isEmpty() &&
            m_spNumberFormatterWrapper &&
            m_spNumberFormatterWrapper->getSvNumberFormatter() &&
            ! m_spNumberFormatterWrapper->getSvNumberFormatter()->IsNumberFormat(
              aText, nDummy, fDummy ))
        {
            bValid = false;
        }
    }

    return bValid;
}

void DataBrowser::CellModified()
{
    m_bDataValid = IsDataValid();
    m_aCursorMovedHdlLink.Call( this );
}

void DataBrowser::SetDataFromModel(
    const rtl::Reference<::chart::ChartModel> & xChartDoc )
{
    m_xChartDoc = xChartDoc;

    m_apDataBrowserModel.reset( new DataBrowserModel( m_xChartDoc ));
    m_spNumberFormatterWrapper =
        std::make_shared<NumberFormatterWrapper>(m_xChartDoc);

    Formatter& rFormatter = m_aNumberEditField->get_formatter();
    rFormatter.SetFormatter( m_spNumberFormatterWrapper->getSvNumberFormatter() );

    RenewTable();

    const sal_Int32 nColCnt  = m_apDataBrowserModel->getColumnCount();
    const sal_Int32 nRowCnt =  m_apDataBrowserModel->getMaxRowCount();
    if( nRowCnt && nColCnt )
    {
        GoToRow( 0 );
        GoToColumnId( 1 );
    }
}

void DataBrowser::InsertColumn()
{
    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( GetCurColumnId(), m_aSeriesHeaders );

    if( nColIdx >= 0 && m_apDataBrowserModel)
    {
        // save changes made to edit-field
        if( IsModified() )
            SaveModified();

        m_apDataBrowserModel->insertDataSeries( nColIdx );
        RenewTable();
    }
}

void DataBrowser::InsertTextColumn()
{
    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( GetCurColumnId(), m_aSeriesHeaders );

    if( nColIdx >= 0 && m_apDataBrowserModel)
    {
        // save changes made to edit-field
        if( IsModified() )
            SaveModified();

        m_apDataBrowserModel->insertComplexCategoryLevel( nColIdx );
        RenewTable();
    }
}

void DataBrowser::RemoveColumn()
{
    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( GetCurColumnId(), m_aSeriesHeaders );

    if( nColIdx >= 0 && m_apDataBrowserModel)
    {
        // save changes made to edit-field
        if( IsModified() )
            SaveModified();

        m_bDataValid = true;
        m_apDataBrowserModel->removeDataSeriesOrComplexCategoryLevel( nColIdx );
        RenewTable();
    }
}

void DataBrowser::InsertRow()
{
    sal_Int32 nRowIdx = GetCurRow();

    if( nRowIdx >= 0 && m_apDataBrowserModel)
    {
        // save changes made to edit-field
        if( IsModified() )
            SaveModified();

        m_apDataBrowserModel->insertDataPointForAllSeries( nRowIdx );
        RenewTable();
    }
}

void DataBrowser::RemoveRow()
{
    sal_Int32 nRowIdx = GetCurRow();

    if( nRowIdx >= 0 && m_apDataBrowserModel)
    {
        // save changes made to edit-field
        if( IsModified() )
            SaveModified();

        m_bDataValid = true;
        m_apDataBrowserModel->removeDataPointForAllSeries( nRowIdx );
        RenewTable();
    }
}

void DataBrowser::MoveLeftColumn()
{
    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( GetCurColumnId(), m_aSeriesHeaders );

    if( !(nColIdx > 0 && m_apDataBrowserModel))
        return;

    // save changes made to edit-field
    if( IsModified() )
        SaveModified();

    m_apDataBrowserModel->swapDataSeries( nColIdx - 1 );

    // keep cursor in swapped column
    if(( 0 < GetCurColumnId() ) && ( GetCurColumnId() <= ColCount() - 1 ))
    {
        Dispatch( BROWSER_CURSORLEFT );
    }
    RenewTable();
}

void DataBrowser::MoveRightColumn()
{
    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( GetCurColumnId(), m_aSeriesHeaders );

    if( !(nColIdx >= 0 && m_apDataBrowserModel))
        return;

    // save changes made to edit-field
    if( IsModified() )
        SaveModified();

    m_apDataBrowserModel->swapDataSeries( nColIdx );

    // keep cursor in swapped column
    if( GetCurColumnId() < ColCount() - 1 )
    {
        Dispatch( BROWSER_CURSORRIGHT );
    }
    RenewTable();
}

void DataBrowser::MoveUpRow()
{
    sal_Int32 nRowIdx = GetCurRow();

    if( !(nRowIdx > 0 && m_apDataBrowserModel))
        return;

    // save changes made to edit-field
    if( IsModified() )
        SaveModified();

    m_apDataBrowserModel->swapDataPointForAllSeries( nRowIdx - 1 );

    // keep cursor in swapped row
    if(( 0 < GetCurRow() ) && ( GetCurRow() <= GetRowCount() - 1 ))
    {
        Dispatch( BROWSER_CURSORUP );
    }
    RenewTable();
}

void DataBrowser::MoveDownRow()
{
    sal_Int32 nRowIdx = GetCurRow();

    if( !(nRowIdx >= 0 && m_apDataBrowserModel))
        return;

    // save changes made to edit-field
    if( IsModified() )
        SaveModified();

    m_apDataBrowserModel->swapDataPointForAllSeries( nRowIdx );

    // keep cursor in swapped row
    if( GetCurRow() < GetRowCount() - 1 )
    {
        Dispatch( BROWSER_CURSORDOWN );
    }
    RenewTable();
}

void DataBrowser::SetCursorMovedHdl( const Link<DataBrowser*,void>& rLink )
{
    m_aCursorMovedHdlLink = rLink;
}

// implementations for ::svt::EditBrowseBox (pure virtual methods)
void DataBrowser::PaintCell(
    OutputDevice& rDev, const tools::Rectangle& rRect, sal_uInt16 nColumnId ) const
{
    Point aPos( rRect.TopLeft());
    aPos.AdjustX(1 );

    OUString aText = GetCellText( m_nSeekRow, nColumnId );
    Size TxtSize( GetDataWindow().GetTextWidth( aText ), GetDataWindow().GetTextHeight());

    // clipping
    if( aPos.X() < rRect.Right() || aPos.X() + TxtSize.Width() > rRect.Right() ||
        aPos.Y() < rRect.Top() || aPos.Y() + TxtSize.Height() > rRect.Bottom())
        rDev.SetClipRegion(vcl::Region(rRect));

    // allow for a disabled control ...
    bool bEnabled = IsEnabled();
    Color aOriginalColor = rDev.GetTextColor();
    if( ! bEnabled )
        rDev.SetTextColor( GetSettings().GetStyleSettings().GetDisableColor() );

    // draw the text
    rDev.DrawText( aPos, aText );

    // reset the color (if necessary)
    if( ! bEnabled )
        rDev.SetTextColor( aOriginalColor );

    if( rDev.IsClipRegion())
        rDev.SetClipRegion();
}

bool DataBrowser::SeekRow( sal_Int32 nRow )
{
    if( ! EditBrowseBox::SeekRow( nRow ))
        return false;

    if( nRow < 0 )
        m_nSeekRow = - 1;
    else
        m_nSeekRow = nRow;

    return true;
}

bool DataBrowser::IsTabAllowed( bool bForward ) const
{
    sal_Int32 nRow = GetCurRow();
    sal_Int32 nCol = GetCurColumnId();

    // column 0 is header-column
    sal_Int32 nBadCol = bForward
        ? GetColumnCount() - 1
        : 1;
    sal_Int32 nBadRow = bForward
        ? GetRowCount() - 1
        : 0;

    if( !m_bDataValid )
    {
        const_cast< DataBrowser* >( this )->ShowWarningBox();
        return false;
    }

    return ( nRow != nBadRow ||
             nCol != nBadCol );
}

::svt::CellController* DataBrowser::GetController( sal_Int32 /*nRow*/, sal_uInt16 nCol )
{
    if( m_bIsReadOnly )
        return nullptr;

    if( CellContainsNumbers( nCol ))
    {
        Formatter& rFormatter = m_aNumberEditField->get_formatter();
        rFormatter.UseInputStringForFormatting();
        rFormatter.SetFormatKey( GetNumberFormatKey( nCol ));
        return m_rNumberEditController.get();
    }

    return m_rTextEditController.get();
}

void DataBrowser::InitController(
    ::svt::CellControllerRef& rController, sal_Int32 nRow, sal_uInt16 nCol )
{
    if( rController == m_rTextEditController )
    {
        OUString aText( GetCellText( nRow, nCol ) );
        weld::Entry& rEntry = m_aTextEditField->get_widget();
        rEntry.set_text(aText);
        rEntry.select_region(0, -1);
    }
    else if( rController == m_rNumberEditController )
    {
        // treat invalid and empty text as Nan
        Formatter& rFormatter = m_aNumberEditField->get_formatter();
        rFormatter.EnableNotANumber( true );
        if( std::isnan( GetCellNumber( nRow, nCol )))
            rFormatter.SetTextValue( OUString());
        else
            rFormatter.SetValue( GetCellNumber( nRow, nCol ) );
        weld::Entry& rEntry = m_aNumberEditField->get_widget();
        rEntry.select_region(0, -1);
    }
    else
    {
        OSL_FAIL( "Invalid Controller" );
    }
}

bool DataBrowser::CellContainsNumbers( sal_uInt16 nCol ) const
{
    if (!m_apDataBrowserModel)
        return false;
    return m_apDataBrowserModel->getCellType( lcl_getColumnInData( nCol )) == DataBrowserModel::NUMBER;
}

sal_uInt32 DataBrowser::GetNumberFormatKey( sal_uInt16 nCol ) const
{
    if (!m_apDataBrowserModel)
        return 0;
    return m_apDataBrowserModel->getNumberFormatKey( lcl_getColumnInData( nCol ) );
}

bool DataBrowser::isDateTimeString( const OUString& aInputString, double& fOutDateTimeValue )
{
    sal_uInt32 nNumberFormat=0;
    SvNumberFormatter* pSvNumberFormatter = m_spNumberFormatterWrapper ? m_spNumberFormatterWrapper->getSvNumberFormatter() : nullptr;
    if( !aInputString.isEmpty() &&  pSvNumberFormatter && pSvNumberFormatter->IsNumberFormat( aInputString, nNumberFormat, fOutDateTimeValue ) )
    {
        SvNumFormatType nType = pSvNumberFormatter->GetType( nNumberFormat);
        return (nType & SvNumFormatType::DATE) || (nType & SvNumFormatType::TIME);
    }
    return false;
}

bool DataBrowser::SaveModified()
{
    if( ! IsModified() )
        return true;

    bool bChangeValid = true;

    const sal_Int32 nRow = GetCurRow();
    const sal_Int32 nCol = lcl_getColumnInData( GetCurColumnId());

    OSL_ENSURE( nRow >= 0 || nCol >= 0, "This cell should not be modified!" );

    SvNumberFormatter* pSvNumberFormatter = m_spNumberFormatterWrapper ? m_spNumberFormatterWrapper->getSvNumberFormatter() : nullptr;
    switch( m_apDataBrowserModel->getCellType( nCol ))
    {
        case DataBrowserModel::NUMBER:
        {
            sal_uInt32 nDummy = 0;
            double fDummy = 0.0;
            OUString aText(m_aNumberEditField->get_widget().get_text());
            // an empty string is valid, if no numberformatter exists, all
            // values are treated as valid
            if( !aText.isEmpty() && pSvNumberFormatter &&
                ! pSvNumberFormatter->IsNumberFormat( aText, nDummy, fDummy ) )
            {
                bChangeValid = false;
            }
            else
            {
                Formatter& rFormatter = m_aNumberEditField->get_formatter();
                double fData = rFormatter.GetValue();
                bChangeValid = m_apDataBrowserModel->setCellNumber( nCol, nRow, fData );
            }
        }
        break;
        case DataBrowserModel::TEXTORDATE:
        {
            weld::Entry& rEntry = m_aTextEditField->get_widget();
            OUString aText(rEntry.get_text());
            double fValue = 0.0;
            bChangeValid = false;
            if( isDateTimeString( aText, fValue ) )
                bChangeValid = m_apDataBrowserModel->setCellAny( nCol, nRow, uno::Any( fValue ) );
            if(!bChangeValid)
                bChangeValid = m_apDataBrowserModel->setCellAny( nCol, nRow, uno::Any( aText ) );
        }
        break;
        case DataBrowserModel::TEXT:
        {
            weld::Entry& rEntry = m_aTextEditField->get_widget();
            OUString aText(rEntry.get_text());
            bChangeValid = m_apDataBrowserModel->setCellText( nCol, nRow, aText );
        }
        break;
    }

    // the first valid change changes this to true
    if( bChangeValid )
    {
        RowModified( GetCurRow(), GetCurColumnId());
        ::svt::CellController* pCtrl = GetController( GetCurRow(), GetCurColumnId());
        if( pCtrl )
            pCtrl->SaveValue();
    }

    return bChangeValid;
}

bool DataBrowser::EndEditing()
{
    SaveModified();

    // apply changes made to series headers
    for( const auto& spHeader : m_aSeriesHeaders )
        spHeader->applyChanges();

    if( m_bDataValid )
        return true;
    else
        return ShowQueryBox();
}

void DataBrowser::ColumnResized( sal_uInt16 nColId )
{
    bool bLastUpdateMode = GetUpdateMode();
    SetUpdateMode( false );

    EditBrowseBox::ColumnResized( nColId );
    ImplAdjustHeaderControls();
    SetUpdateMode( bLastUpdateMode );
}

void DataBrowser::EndScroll()
{
    bool bLastUpdateMode = GetUpdateMode();
    SetUpdateMode( false );

    EditBrowseBox::EndScroll();
    RenewSeriesHeaders();

    SetUpdateMode( bLastUpdateMode );
}

void DataBrowser::RenewSeriesHeaders()
{
    clearHeaders();
    DataBrowserModel::tDataHeaderVector aHeaders( m_apDataBrowserModel->getDataHeaders());
    Link<impl::SeriesHeaderEdit&,void> aFocusLink( LINK( this, DataBrowser, SeriesHeaderGotFocus ));
    Link<impl::SeriesHeaderEdit&,void> aSeriesHeaderChangedLink( LINK( this, DataBrowser, SeriesHeaderChanged ));

    for (auto const& elemHeader : aHeaders)
    {
        auto spHeader = std::make_shared<impl::SeriesHeader>( m_pColumnsWin, m_pColorsWin );
        Color nColor;
        if( elemHeader.m_xDataSeries.is() &&
            ( elemHeader.m_xDataSeries->getPropertyValue( u"Color"_ustr ) >>= nColor ))
            spHeader->SetColor( nColor );
        spHeader->SetChartType( elemHeader.m_xChartType, elemHeader.m_bSwapXAndYAxis );
        spHeader->SetSeriesName(
            elemHeader.m_xDataSeries->getLabelForRole(
                        elemHeader.m_xChartType.is() ?
                         elemHeader.m_xChartType->getRoleOfSequenceForSeriesLabel() :
                         u"values-y"_ustr));
        spHeader->SetRange( elemHeader.m_nStartColumn + 1, elemHeader.m_nEndColumn + 1 );
        spHeader->SetGetFocusHdl( aFocusLink );
        spHeader->SetEditChangedHdl( aSeriesHeaderChangedLink );
        m_aSeriesHeaders.push_back( spHeader );
    }

    ImplAdjustHeaderControls();
}

void DataBrowser::ImplAdjustHeaderControls()
{
    sal_uInt16 nColCount = GetColumnCount();
    sal_uInt32 nCurrentPos = GetPosPixel().getX();
    sal_uInt32 nMaxPos = nCurrentPos + GetOutputSizePixel().getWidth();
    sal_uInt32 nStartPos = nCurrentPos;

    // width of header column
    nCurrentPos +=  GetColumnWidth( 0 );

    weld::Container* pWin = m_pColumnsWin;
    weld::Container* pColorWin = m_pColorsWin;
    pWin->set_margin_start(nCurrentPos);
    pColorWin->set_margin_start(nCurrentPos);

    tSeriesHeaderContainer::iterator aIt( m_aSeriesHeaders.begin());
    sal_uInt16 i = GetFirstVisibleColNumber();
    while( (aIt != m_aSeriesHeaders.end()) && ((*aIt)->GetStartColumn() < i) )
    {
        (*aIt)->Hide();
        ++aIt;
    }
    for( ; i < nColCount && aIt != m_aSeriesHeaders.end(); ++i )
    {
        if( (*aIt)->GetStartColumn() == i )
            nStartPos = nCurrentPos;

        nCurrentPos += (GetColumnWidth( i ));

        if( (*aIt)->GetEndColumn() == i )
        {
            if( nStartPos < nMaxPos )
            {
                (*aIt)->SetPixelWidth( nCurrentPos - nStartPos );
                (*aIt)->Show();

                if (pWin)
                {
                    pWin->set_margin_start(nStartPos);
                    pColorWin->set_margin_start(nStartPos);
                    pWin = pColorWin = nullptr;
                }

            }
            else
                (*aIt)->Hide();
            ++aIt;
        }
    }
}

IMPL_LINK( DataBrowser, SeriesHeaderGotFocus, impl::SeriesHeaderEdit&, rEdit, void )
{
    rEdit.SetShowWarningBox( !m_bDataValid );

    if( !m_bDataValid )
        GoToCell( 0, 0 );
    else
    {
        MakeFieldVisible( GetCurRow(), static_cast< sal_uInt16 >( rEdit.getStartColumn()) );
        ActivateCell();
        m_aCursorMovedHdlLink.Call( this );
    }
}

IMPL_LINK( DataBrowser, SeriesHeaderChanged, impl::SeriesHeaderEdit&, rEdit, void )
{
    rtl::Reference< DataSeries > xSeries =
        m_apDataBrowserModel->getDataSeriesByColumn( rEdit.getStartColumn() - 1 );
    if( !xSeries.is())
        return;

    rtl::Reference< ChartType > xChartType(
        m_apDataBrowserModel->getHeaderForSeries( xSeries ).m_xChartType );
    if( xChartType.is())
    {
        uno::Reference< chart2::data::XLabeledDataSequence > xLabeledSeq =
            DataSeriesHelper::getDataSequenceByRole( xSeries, xChartType->getRoleOfSequenceForSeriesLabel());
        if( xLabeledSeq.is())
        {
            Reference< container::XIndexReplace > xIndexReplace( xLabeledSeq->getLabel(), uno::UNO_QUERY );
            if( xIndexReplace.is())
                xIndexReplace->replaceByIndex(
                    0, uno::Any( rEdit.GetText()));
        }
    }
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
