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

#include <svl/zforlist.hxx>

#include "DataBrowser.hxx"
#include <DataBrowserModel.hxx>
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

#include <com/sun/star/container/XIndexReplace.hpp>

#include <algorithm>
#include <limits>


using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

namespace chart
{

namespace impl
{

class SeriesHeaderEdit
{
public:
    explicit SeriesHeaderEdit(std::unique_ptr<weld::Entry> xControl, sal_Int32 nHeader);

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

SeriesHeaderEdit::SeriesHeaderEdit(std::unique_ptr<weld::Entry> xControl, sal_Int32 nHeader)
    : m_xControl(std::move(xControl))
    , m_nStartColumn(0)
    , m_bShowWarningBox(false)
{
    m_xControl->set_help_id(HID_SCH_DATA_SERIES_LABEL);
    m_xControl->connect_changed(LINK(this, SeriesHeaderEdit, NameEdited));
    m_xControl->connect_focus_in(LINK(this, SeriesHeaderEdit, NameFocusIn));
    m_xControl->connect_mouse_press(LINK(this, SeriesHeaderEdit, MousePressHdl));

    m_xControl->set_buildable_name(m_xControl->get_buildable_name() + OUString::number(nHeader));
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
    explicit SeriesHeader(weld::Box* pParent, weld::Box* pColorParent, sal_Int32 nHeader);
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

    weld::Box* m_pParent;
    weld::Box* m_pColorParent;

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

SeriesHeader::SeriesHeader(weld::Box* pParent, weld::Box* pColorParent, sal_Int32 nHeader)
    : m_aUpdateDataTimer( "SeriesHeader UpdateDataTimer" )
    , m_xBuilder1(Application::CreateBuilder(pParent, u"modules/schart/ui/columnfragment.ui"_ustr))
    , m_xBuilder2(Application::CreateBuilder(pColorParent, u"modules/schart/ui/imagefragment.ui"_ustr))
    , m_pParent(pParent)
    , m_pColorParent(pColorParent)
    , m_xContainer1(m_xBuilder1->weld_container(u"container"_ustr))
    , m_xContainer2(m_xBuilder2->weld_container(u"container"_ustr))
    , m_spSymbol(m_xBuilder1->weld_image(u"image"_ustr))
    , m_spSeriesName(new SeriesHeaderEdit(m_xBuilder1->weld_entry(u"entry"_ustr), nHeader))
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
    sal_Int32 nCol, const std::vector< std::shared_ptr< ::chart::impl::SeriesHeader > > & rSeriesHeader )
{
    sal_Int32 nColIdx = 0;
    bool bHeaderHasFocus( lcl_SeriesHeaderHasFocus( rSeriesHeader, &nColIdx ));

    if( bHeaderHasFocus )
        nColIdx = rSeriesHeader[nColIdx]->GetStartColumn() - 1;
    else
        nColIdx = nCol;

    return nColIdx;
}

} // anonymous namespace

DataBrowser::DataBrowser(weld::TreeView& rTreeView, weld::Box* pColumns, weld::Box* pColors)
    : m_rTreeView(rTreeView)
    , m_pColumnsWin(pColumns)
    , m_pColorsWin(pColors)
    , m_nCurRow(0)
    , m_nCurCol(0)
    , m_bIsReadOnly(false)
    , m_bDataValid(true)
{
    m_rTreeView.connect_editing(
        LINK(this, DataBrowser, EditingStartedHdl),
        LINK(this, DataBrowser, EditingDoneHdl));
    m_rTreeView.connect_selection_changed(LINK(this, DataBrowser, SelectionChangedHdl));
    m_rTreeView.connect_header_name_changed(LINK(this, DataBrowser, HeaderNameChangedHdl));
    RenewTable();
}

DataBrowser::~DataBrowser()
{
    m_aSeriesHeaders.clear();
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
        && ( m_nCurRow >= 0 )
        && ( GetRowCount() > 1 );
}

bool DataBrowser::MayDeleteColumn() const
{
    // if a series header has the focus
    if( lcl_SeriesHeaderHasFocus( m_aSeriesHeaders ))
        return true;

    return ! IsReadOnly()
        && ( m_nCurCol > 0 )
        && ( GetColumnCount() > 1 );
}

bool DataBrowser::MayMoveUpRows() const
{
    return ! IsReadOnly()
        && ( !lcl_SeriesHeaderHasFocus( m_aSeriesHeaders ))
        && ( m_nCurRow > 0 )
        && ( m_nCurRow <= GetRowCount() - 1 );
}

bool DataBrowser::MayMoveDownRows() const
{
    return ! IsReadOnly()
        && ( !lcl_SeriesHeaderHasFocus( m_aSeriesHeaders ))
        && ( m_nCurRow >= 0 )
        && ( m_nCurRow < GetRowCount() - 1 );
}

bool DataBrowser::MayMoveLeftColumns() const
{
    // if a series header (except the last one) has the focus
    {
        sal_Int32 nColIndex(0);
        if( lcl_SeriesHeaderHasFocus( m_aSeriesHeaders, &nColIndex ))
            return (o3tl::make_unsigned( nColIndex ) <= (m_aSeriesHeaders.size() - 1)) && (static_cast< sal_uInt32 >( nColIndex ) != 0);
    }

    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( m_nCurCol, m_aSeriesHeaders );
    return ! IsReadOnly()
        && ( nColIdx > 1 )
        && ( nColIdx <= GetColumnCount() - 1 )
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

    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( m_nCurCol, m_aSeriesHeaders );
    return ! IsReadOnly()
        && ( nColIdx > 0 )
        && ( nColIdx < GetColumnCount() - 1 )
        && m_apDataBrowserModel
        && !m_apDataBrowserModel->isCategoriesColumn( nColIdx );
}

void DataBrowser::clearHeaders()
{
    for( const auto& spHeader : m_aSeriesHeaders )
        spHeader->applyChanges();
    m_aSeriesHeaders.clear();
}

sal_Int32 DataBrowser::GetColumnCount() const
{
    if (!m_apDataBrowserModel)
        return 0;
    return m_apDataBrowserModel->getColumnCount();
}

sal_Int32 DataBrowser::GetRowCount() const
{
    if (!m_apDataBrowserModel)
        return 0;
    return m_apDataBrowserModel->getMaxRowCount();
}

void DataBrowser::RenewTable()
{
    if (!m_apDataBrowserModel)
        return;

    sal_Int32 nOldRow = m_nCurRow;
    sal_Int32 nOldCol = m_nCurCol;

    sal_Int32 nColumnCount = m_apDataBrowserModel->getColumnCount();
    sal_Int32 nRowCount = m_apDataBrowserModel->getMaxRowCount();

    if (nColumnCount <= 0)
    {
        m_rTreeView.freeze();
        m_rTreeView.clear();
        m_rTreeView.thaw();
        clearHeaders();
        return;
    }

    // set up column widths and editables
    // column 0 in TreeView = row number (not editable)
    // columns 1..nColumnCount = data columns (editable)
    OUString aDefaultSeriesName(SchResId(STR_COLUMN_LABEL));
    replaceParamterInString( aDefaultSeriesName, u"%COLUMNNUMBER", OUString::number( 24 ) );

    // estimate column width based on text
    int nColWidth = m_rTreeView.get_approximate_digit_width() * 12;

    std::vector<int> aWidths;
    // first column for row numbers - narrower
    aWidths.push_back(m_rTreeView.get_approximate_digit_width() * 6);
    for (sal_Int32 i = 0; i < nColumnCount; ++i)
        aWidths.push_back(nColWidth);

    // all data columns are editable, row number column is not
    std::vector<bool> aEditables;
    aEditables.push_back(false); // row number column
    for (sal_Int32 i = 0; i < nColumnCount; ++i)
        aEditables.push_back(!m_bIsReadOnly);

    // Set widths first (creates mvTabList), then editables (sets flags on mvTabList),
    // then widths again to force SetTabs() which propagates editable flags to aTabs
    // for correct JSON serialization in JSDialog.
    m_rTreeView.set_column_fixed_widths(aWidths);
    m_rTreeView.set_column_editables(aEditables);
    m_rTreeView.set_column_fixed_widths(aWidths);

    // set column titles
    m_rTreeView.set_column_title(0, OUString());
    for (sal_Int32 nColIdx = 1; nColIdx <= nColumnCount; ++nColIdx)
    {
        const OUString& sRole = m_apDataBrowserModel->getRoleOfColumn(nColIdx - 1);
        m_rTreeView.set_column_title(nColIdx, sRole);
    }

    // populate rows inside freeze/thaw for performance
    m_rTreeView.freeze();
    m_rTreeView.clear();

    for (sal_Int32 nRow = 0; nRow < nRowCount; ++nRow)
    {
        m_rTreeView.append();
        // row number in column 0
        m_rTreeView.set_text(nRow, OUString::number(nRow + 1), 0);
        // data in columns 1..nColumnCount
        for (sal_Int32 nCol = 0; nCol < nColumnCount; ++nCol)
        {
            OUString sText = GetCellText(nRow, nCol);
            m_rTreeView.set_text(nRow, sText, nCol + 1);
        }
    }

    // Set header colors and names on the TreeView inside the freeze block
    // so they are included in the single thaw update sent to JSDialog/COOL.
    const DataBrowserModel::tDataHeaderVector& aHeaders( m_apDataBrowserModel->getDataHeaders());
    for (const auto& elemHeader : aHeaders)
    {
        Color nColor;
        if( elemHeader.m_xDataSeries.is() &&
            ( elemHeader.m_xDataSeries->getPropertyValue( u"Color"_ustr ) >>= nColor ))
        {
            for (sal_Int32 nCol = elemHeader.m_nStartColumn; nCol <= elemHeader.m_nEndColumn; ++nCol)
                m_rTreeView.set_column_header_color(nCol + 1, nColor);
        }
        OUString aSeriesName =
            elemHeader.m_xDataSeries->getLabelForRole(
                        elemHeader.m_xChartType.is() ?
                         elemHeader.m_xChartType->getRoleOfSequenceForSeriesLabel() :
                         u"values-y"_ustr);
        for (sal_Int32 nCol = elemHeader.m_nStartColumn; nCol <= elemHeader.m_nEndColumn; ++nCol)
            m_rTreeView.set_column_header_name(nCol + 1, aSeriesName);
    }

    m_rTreeView.thaw();

    // restore cursor position
    sal_Int32 nNewRow = std::min(nOldRow, nRowCount - 1);
    sal_Int32 nNewCol = std::min(nOldCol, nColumnCount - 1);
    if (nNewRow >= 0 && nRowCount > 0)
        m_rTreeView.select(nNewRow);
    m_nCurRow = nNewRow;
    m_nCurCol = nNewCol;

    // fill series headers (VCL controls for desktop rendering)
    clearHeaders();
    Link<impl::SeriesHeaderEdit&,void> aFocusLink( LINK( this, DataBrowser, SeriesHeaderGotFocus ));
    Link<impl::SeriesHeaderEdit&,void> aSeriesHeaderChangedLink( LINK( this, DataBrowser, SeriesHeaderChanged ));

    for (size_t i = 0; i < aHeaders.size(); ++i)
    {
        auto const& elemHeader = aHeaders[i];
        auto spHeader = std::make_shared<impl::SeriesHeader>( m_pColumnsWin, m_pColorsWin, i);
        Color nColor;
        if( elemHeader.m_xDataSeries.is() &&
            ( elemHeader.m_xDataSeries->getPropertyValue( u"Color"_ustr ) >>= nColor ))
            spHeader->SetColor( nColor );
        spHeader->SetChartType( elemHeader.m_xChartType, elemHeader.m_bSwapXAndYAxis );
        OUString aSeriesName =
            elemHeader.m_xDataSeries->getLabelForRole(
                        elemHeader.m_xChartType.is() ?
                         elemHeader.m_xChartType->getRoleOfSequenceForSeriesLabel() :
                         u"values-y"_ustr);
        spHeader->SetSeriesName(aSeriesName);
        // index is 1-based, as 0 is for the column that contains the row-numbers
        spHeader->SetRange( elemHeader.m_nStartColumn + 1, elemHeader.m_nEndColumn + 1 );
        spHeader->SetGetFocusHdl( aFocusLink );
        spHeader->SetEditChangedHdl( aSeriesHeaderChangedLink );
        m_aSeriesHeaders.push_back(std::move(spHeader));
    }

    ImplAdjustHeaderControls();
}

OUString DataBrowser::GetCellText( sal_Int32 nRow, sal_Int32 nCol ) const
{
    OUString aResult;

    if( nRow >= 0 && m_apDataBrowserModel)
    {
        if( m_apDataBrowserModel->getCellType( nCol ) == DataBrowserModel::NUMBER )
        {
            double fData( m_apDataBrowserModel->getCellNumber( nCol, nRow ));

            if( ! std::isnan( fData ) &&
                m_spNumberFormatterWrapper )
            {
                bool bColorChanged = false;
                Color nLabelColor;
                sal_uInt32 nFormatKey = m_apDataBrowserModel->getNumberFormatKey( nCol );
                aResult = m_spNumberFormatterWrapper->getFormattedString(
                                      nFormatKey,
                                      fData, nLabelColor, bColorChanged );
            }
        }
        else if( m_apDataBrowserModel->getCellType( nCol ) == DataBrowserModel::TEXTORDATE )
        {
            uno::Any aAny = m_apDataBrowserModel->getCellAny( nCol, nRow );
            OUString aText;
            double fDouble=0.0;
            if( aAny>>=aText )
                aResult = aText;
            else if( aAny>>=fDouble )
            {
                if( ! std::isnan( fDouble ) && m_spNumberFormatterWrapper )
                {
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
            OSL_ASSERT( m_apDataBrowserModel->getCellType( nCol ) == DataBrowserModel::TEXT );
            aResult = m_apDataBrowserModel->getCellText( nCol, nRow );
        }
    }

    return aResult;
}

void DataBrowser::SetReadOnly( bool bNewState )
{
    if( m_bIsReadOnly != bNewState )
    {
        m_bIsReadOnly = bNewState;
        // update editability of columns
        if (m_apDataBrowserModel)
        {
            sal_Int32 nColumnCount = m_apDataBrowserModel->getColumnCount();
            std::vector<bool> aEditables;
            aEditables.push_back(false); // row number column
            for (sal_Int32 i = 0; i < nColumnCount; ++i)
                aEditables.push_back(!m_bIsReadOnly);
            m_rTreeView.set_column_editables(aEditables);

            // re-trigger set_column_fixed_widths to propagate editable flags to aTabs
            std::vector<int> aWidths;
            aWidths.push_back(m_rTreeView.get_column_width(0));
            for (sal_Int32 i = 0; i < nColumnCount; ++i)
                aWidths.push_back(m_rTreeView.get_column_width(i + 1));
            m_rTreeView.set_column_fixed_widths(aWidths);
        }
    }
}

void DataBrowser::SetDataFromModel(
    const rtl::Reference<::chart::ChartModel> & xChartDoc )
{
    m_xChartDoc = xChartDoc;

    m_apDataBrowserModel.reset( new DataBrowserModel( m_xChartDoc ));
    m_spNumberFormatterWrapper =
        std::make_shared<NumberFormatterWrapper>(m_xChartDoc);

    RenewTable();

    const sal_Int32 nColCnt  = m_apDataBrowserModel->getColumnCount();
    const sal_Int32 nRowCnt =  m_apDataBrowserModel->getMaxRowCount();
    if( nRowCnt && nColCnt )
    {
        m_nCurRow = 0;
        m_nCurCol = 0;
        m_rTreeView.select(0);
    }
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

bool DataBrowser::SaveCellEdit( sal_Int32 nRow, sal_Int32 nCol, const OUString& rText )
{
    if (!m_apDataBrowserModel || nCol < 0 || nRow < 0)
        return false;

    bool bChangeValid = true;

    SvNumberFormatter* pSvNumberFormatter = m_spNumberFormatterWrapper ? m_spNumberFormatterWrapper->getSvNumberFormatter() : nullptr;
    switch( m_apDataBrowserModel->getCellType( nCol ))
    {
        case DataBrowserModel::NUMBER:
        {
            sal_uInt32 nDummy = 0;
            double fDummy = 0.0;
            // an empty string is valid, if no numberformatter exists, all
            // values are treated as valid
            if( !rText.isEmpty() && pSvNumberFormatter &&
                ! pSvNumberFormatter->IsNumberFormat( rText, nDummy, fDummy ) )
            {
                bChangeValid = false;
            }
            else
            {
                double fData;
                if (rText.isEmpty())
                    fData = std::numeric_limits<double>::quiet_NaN();
                else
                {
                    sal_uInt32 nFormat = 0;
                    if (pSvNumberFormatter)
                        pSvNumberFormatter->IsNumberFormat(rText, nFormat, fData);
                    else
                        fData = rText.toDouble();
                }
                bChangeValid = m_apDataBrowserModel->setCellNumber( nCol, nRow, fData );
            }
        }
        break;
        case DataBrowserModel::TEXTORDATE:
        {
            double fValue = 0.0;
            bChangeValid = false;
            if( isDateTimeString( rText, fValue ) )
                bChangeValid = m_apDataBrowserModel->setCellAny( nCol, nRow, uno::Any( fValue ) );
            if(!bChangeValid)
                bChangeValid = m_apDataBrowserModel->setCellAny( nCol, nRow, uno::Any( rText ) );
        }
        break;
        case DataBrowserModel::TEXT:
        {
            bChangeValid = m_apDataBrowserModel->setCellText( nCol, nRow, rText );
        }
        break;
    }

    return bChangeValid;
}

void DataBrowser::InsertColumn()
{
    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( m_nCurCol, m_aSeriesHeaders );

    if( nColIdx >= 0 && m_apDataBrowserModel)
    {
        m_apDataBrowserModel->insertDataSeries( nColIdx );
        RenewTable();
    }
}

void DataBrowser::InsertTextColumn()
{
    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( m_nCurCol, m_aSeriesHeaders );

    if( nColIdx >= 0 && m_apDataBrowserModel)
    {
        m_apDataBrowserModel->insertComplexCategoryLevel( nColIdx );
        RenewTable();
    }
}

void DataBrowser::RemoveColumn()
{
    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( m_nCurCol, m_aSeriesHeaders );

    if( nColIdx >= 0 && m_apDataBrowserModel)
    {
        m_bDataValid = true;
        m_apDataBrowserModel->removeDataSeriesOrComplexCategoryLevel( nColIdx );
        RenewTable();
    }
}

void DataBrowser::InsertRow()
{
    sal_Int32 nRowIdx = m_nCurRow;

    if( nRowIdx >= 0 && m_apDataBrowserModel)
    {
        m_apDataBrowserModel->insertDataPointForAllSeries( nRowIdx );
        RenewTable();
    }
}

void DataBrowser::RemoveRow()
{
    sal_Int32 nRowIdx = m_nCurRow;

    if( nRowIdx >= 0 && m_apDataBrowserModel)
    {
        m_bDataValid = true;
        m_apDataBrowserModel->removeDataPointForAllSeries( nRowIdx );
        RenewTable();
    }
}

void DataBrowser::MoveLeftColumn()
{
    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( m_nCurCol, m_aSeriesHeaders );

    if( !(nColIdx > 0 && m_apDataBrowserModel))
        return;

    m_apDataBrowserModel->swapDataSeries( nColIdx - 1 );

    // keep cursor in swapped column
    if( m_nCurCol > 0 )
        m_nCurCol--;
    RenewTable();
}

void DataBrowser::MoveRightColumn()
{
    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( m_nCurCol, m_aSeriesHeaders );

    if( !(nColIdx >= 0 && m_apDataBrowserModel))
        return;

    m_apDataBrowserModel->swapDataSeries( nColIdx );

    // keep cursor in swapped column
    if( m_nCurCol < GetColumnCount() - 1 )
        m_nCurCol++;
    RenewTable();
}

void DataBrowser::MoveUpRow()
{
    sal_Int32 nRowIdx = m_nCurRow;

    if( !(nRowIdx > 0 && m_apDataBrowserModel))
        return;

    m_apDataBrowserModel->swapDataPointForAllSeries( nRowIdx - 1 );

    // keep cursor in swapped row
    if( m_nCurRow > 0 )
        m_nCurRow--;
    RenewTable();
}

void DataBrowser::MoveDownRow()
{
    sal_Int32 nRowIdx = m_nCurRow;

    if( !(nRowIdx >= 0 && m_apDataBrowserModel))
        return;

    m_apDataBrowserModel->swapDataPointForAllSeries( nRowIdx );

    // keep cursor in swapped row
    if( m_nCurRow < GetRowCount() - 1 )
        m_nCurRow++;
    RenewTable();
}

void DataBrowser::SetCursorMovedHdl( const Link<DataBrowser*,void>& rLink )
{
    m_aCursorMovedHdlLink = rLink;
}

bool DataBrowser::EndEditing()
{
    // apply changes made to series headers
    for( const auto& spHeader : m_aSeriesHeaders )
        spHeader->applyChanges();

    if( m_bDataValid )
        return true;

    // ask user if invalid data should be accepted
    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(
        &m_rTreeView,
        VclMessageType::Question, VclButtonsType::YesNo,
        SchResId(STR_DATA_EDITOR_INCORRECT_INPUT)));
    return xQueryBox->run() == RET_YES;
}

void DataBrowser::GrabFocus()
{
    m_rTreeView.grab_focus();
}

void DataBrowser::ImplAdjustHeaderControls()
{
    sal_Int32 nColCount = GetColumnCount();
    if (nColCount <= 0)
        return;

    // use TreeView column widths to align headers
    // column 0 is the row-number column, data starts at column 1
    sal_Int32 nCurrentPos = m_rTreeView.get_column_width(0);

    weld::Container* pWin = m_pColumnsWin;
    weld::Container* pColorWin = m_pColorsWin;
    if (pWin)
        pWin->set_margin_start(nCurrentPos);
    if (pColorWin)
        pColorWin->set_margin_start(nCurrentPos);

    tSeriesHeaderContainer::iterator aIt( m_aSeriesHeaders.begin());
    for( sal_Int32 i = 0; i < nColCount && aIt != m_aSeriesHeaders.end(); ++i )
    {
        sal_Int32 nTreeCol = i + 1; // +1 for row number column
        sal_Int32 nColWidth = m_rTreeView.get_column_width(nTreeCol);

        if( (*aIt)->GetStartColumn() == i + 1 )
        {
            // start of a new header
            if (pWin)
            {
                pWin->set_margin_start(nCurrentPos);
                pColorWin->set_margin_start(nCurrentPos);
                pWin = pColorWin = nullptr;
            }
        }

        nCurrentPos += nColWidth;

        if( (*aIt)->GetEndColumn() == i + 1 )
        {
            sal_Int32 nStartPos = 0;
            // calculate start pos from column widths
            nStartPos = m_rTreeView.get_column_width(0);
            for (sal_Int32 c = 0; c < i; ++c)
            {
                if ((*aIt)->GetStartColumn() <= c + 1)
                    break;
                nStartPos += m_rTreeView.get_column_width(c + 1);
            }

            (*aIt)->SetPixelWidth( nCurrentPos - nStartPos );
            (*aIt)->Show();
            ++aIt;
        }
    }

    // hide remaining headers
    while (aIt != m_aSeriesHeaders.end())
    {
        (*aIt)->Hide();
        ++aIt;
    }
}

IMPL_LINK( DataBrowser, SeriesHeaderGotFocus, impl::SeriesHeaderEdit&, rEdit, void )
{
    rEdit.SetShowWarningBox( !m_bDataValid );

    if( !m_bDataValid )
    {
        m_nCurRow = 0;
        m_nCurCol = 0;
        if (m_rTreeView.n_children() > 0)
            m_rTreeView.select(0);
    }
    else
    {
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

IMPL_LINK( DataBrowser, HeaderNameChangedHdl, const ColumnNamePair&, rData, void )
{
    if (m_bIsReadOnly || !m_apDataBrowserModel)
        return;

    // rData.first is the TreeView column index (0 = row numbers, 1.. = data columns)
    // getDataSeriesByColumn expects 0-based data column index
    sal_Int32 nDataCol = rData.first - 1;
    if (nDataCol < 0)
        return;

    rtl::Reference< DataSeries > xSeries =
        m_apDataBrowserModel->getDataSeriesByColumn( nDataCol );
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
                    0, uno::Any( rData.second ));
        }
    }
}

IMPL_LINK_NOARG( DataBrowser, EditingStartedHdl, const weld::TreeIter&, bool )
{
    // allow editing unless read-only
    return !m_bIsReadOnly;
}

IMPL_LINK( DataBrowser, EditingDoneHdl, const weld::TreeView::iter_string&, rIterText, bool )
{
    if (m_bIsReadOnly || !m_apDataBrowserModel)
        return false;

    // determine which row
    sal_Int32 nRow = m_rTreeView.get_iter_index_in_parent(rIterText.first);
    if (nRow < 0)
        return false;

    // determine which column was edited
    // In JSDialog path, the executor sets this before calling trigger_editing_done
    sal_Int32 nTreeCol = m_rTreeView.get_editing_column();
    if (nTreeCol < 0)
        nTreeCol = m_nCurCol + 1; // fallback: use current cursor column (+1 for row number col)
    if (nTreeCol <= 0)
        return false;

    // nTreeCol is 1-based (0 = row number column), convert to data column (0-based)
    sal_Int32 nDataCol = nTreeCol - 1;
    if (nDataCol >= m_apDataBrowserModel->getColumnCount())
        return false;

    const OUString& sNewValue = rIterText.second;

    bool bValid = SaveCellEdit(nRow, nDataCol, sNewValue);
    if (bValid)
    {
        m_bDataValid = true;
        m_nCurRow = nRow;
        m_nCurCol = nDataCol;

        // update the displayed text (the model may have formatted it differently)
        OUString sFormatted = GetCellText(nRow, nDataCol);
        m_rTreeView.set_text(nRow, sFormatted, nTreeCol);

        m_aCursorMovedHdlLink.Call(this);
    }
    else
    {
        m_bDataValid = false;
        m_aCursorMovedHdlLink.Call(this);
    }

    return bValid;
}

IMPL_LINK_NOARG( DataBrowser, SelectionChangedHdl, weld::TreeView&, void )
{
    int nRow = m_rTreeView.get_selected_index();
    if (nRow >= 0)
        m_nCurRow = nRow;

    // Update current column from cell click info (sent by COOL)
    int nCol = m_rTreeView.get_cursor_column();
    if (nCol >= 0)
    {
        // nCol is 0-based TreeView column; column 0 is the row-number column,
        // so data columns start at 1. Convert to data column index (0-based).
        m_nCurCol = nCol > 0 ? nCol - 1 : 0;
    }

    m_aCursorMovedHdlLink.Call(this);
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
