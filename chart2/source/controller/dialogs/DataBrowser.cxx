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

#include <svl/zformat.hxx>
#include <svl/zforlist.hxx>

#include "DataBrowser.hxx"
#include "DataBrowserModel.hxx"
#include "Strings.hrc"
#include "DataSeriesHelper.hxx"
#include "DiagramHelper.hxx"
#include "ChartModelHelper.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "NumberFormatterWrapper.hxx"
#include "servicenames_charttypes.hxx"
#include "ResId.hxx"
#include "Bitmaps.hrc"
#include "HelpIds.hrc"

#include <vcl/fixed.hxx>
#include <vcl/image.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartType.hpp>

#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>

#include <algorithm>
#include <functional>


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

sal_Int32 lcl_getRowInData( long nRow )
{
    return static_cast< sal_Int32 >( nRow );
}

sal_Int32 lcl_getColumnInData( sal_uInt16 nCol )
{
    return static_cast< sal_Int32 >( nCol ) - 1;
}

} // anonymous namespace

namespace chart
{

namespace impl
{

class SeriesHeaderEdit : public Edit
{
public:
    explicit SeriesHeaderEdit( vcl::Window * pParent );
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;

    void setStartColumn( sal_Int32 nStartColumn );
    sal_Int32 getStartColumn() const { return m_nStartColumn;}
    void SetShowWarningBox( bool bShowWarning = true );

private:
    sal_Int32 m_nStartColumn;
    bool m_bShowWarningBox;
};

SeriesHeaderEdit::SeriesHeaderEdit( vcl::Window * pParent ) :
        Edit( pParent ),
        m_nStartColumn( 0 ),
        m_bShowWarningBox( false )
{
    SetHelpId(HID_SCH_DATA_SERIES_LABEL);
}

void SeriesHeaderEdit::setStartColumn( sal_Int32 nStartColumn )
{
    m_nStartColumn = nStartColumn;
}

void SeriesHeaderEdit::SetShowWarningBox( bool bShowWarning )
{
    m_bShowWarningBox = bShowWarning;
}

void SeriesHeaderEdit::MouseButtonDown( const MouseEvent& rMEvt )
{
    Edit::MouseButtonDown( rMEvt );

    if( m_bShowWarningBox )
        ScopedVclPtrInstance<WarningBox>(this, WinBits( WB_OK ),
                   SCH_RESSTR(STR_INVALID_NUMBER))->Execute();
}

class SeriesHeader
{
public:
    explicit SeriesHeader(vcl::Window * pParent, vcl::Window *pColorParent);
            ~SeriesHeader();

    void SetColor( const Color & rCol );
    void SetPos( const Point & rPos );
    void SetWidth( sal_Int32 nWidth );
    void SetChartType( const Reference< chart2::XChartType > & xChartType,
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

    void SetGetFocusHdl( const Link<Control&,void>& rLink );

    void SetEditChangedHdl( const Link<SeriesHeaderEdit*,void> & rLink );

    bool HasFocus() const;

private:
    VclPtr< FixedImage >        m_spSymbol;
    VclPtr< SeriesHeaderEdit >  m_spSeriesName;
    VclPtr< FixedText >         m_spColorBar;
    VclPtr< OutputDevice>       m_pDevice;
    Link<SeriesHeaderEdit*,void> m_aChangeLink;

    void notifyChanges();
    DECL_LINK_TYPED( SeriesNameChanged, Edit&, void );
    DECL_LINK_TYPED( SeriesNameEdited, Edit&, void );

    static Image GetChartTypeImage(
        const Reference< chart2::XChartType > & xChartType,
        bool bSwapXAndYAxis
        );

    sal_Int32 m_nStartCol, m_nEndCol;
    sal_Int32 m_nWidth;
    Point     m_aPos;
    bool      m_bSeriesNameChangePending;
};

SeriesHeader::SeriesHeader( vcl::Window * pParent, vcl::Window *pColorParent ) :
        m_spSymbol( VclPtr<FixedImage>::Create( pParent, WB_NOBORDER )),
        m_spSeriesName( VclPtr<SeriesHeaderEdit>::Create( pParent )),
        m_spColorBar( VclPtr<FixedText>::Create( pColorParent, WB_NOBORDER )),
        m_pDevice( pParent ),
        m_nStartCol( 0 ),
        m_nEndCol( 0 ),
        m_nWidth( 42 ),
        m_aPos( 0, 22 ),
        m_bSeriesNameChangePending( false )
{
    m_spSeriesName->EnableUpdateData( 4 * EDIT_UPDATEDATA_TIMEOUT ); // define is in vcl/edit.hxx
    m_spSeriesName->SetUpdateDataHdl( LINK( this, SeriesHeader, SeriesNameChanged ));
    m_spSeriesName->SetModifyHdl( LINK( this, SeriesHeader, SeriesNameEdited ));
    Show();
}

SeriesHeader::~SeriesHeader()
{
    m_spSymbol.disposeAndClear();
    m_spSeriesName.disposeAndClear();
    m_spColorBar.disposeAndClear();
}

void SeriesHeader::notifyChanges()
{
    m_aChangeLink.Call( m_spSeriesName.get());
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
    m_spColorBar->SetControlBackground( rCol );
}

void SeriesHeader::SetPos( const Point & rPos )
{
    m_aPos = rPos;

    // chart type symbol
    Size aSize( nSymbolHeight, nSymbolHeight );
    aSize = m_pDevice->LogicToPixel( aSize, MAP_APPFONT );
    m_spSymbol->set_width_request(aSize.Width());
    m_spSymbol->set_height_request(aSize.Height());

    // series name edit field
    aSize.setWidth(nSymbolDistance);
    aSize = m_pDevice->LogicToPixel( aSize, MAP_APPFONT );
    m_spSeriesName->set_margin_left(aSize.Width() + 2);
    aSize.setWidth( m_nWidth - nSymbolHeight - nSymbolDistance );
    sal_Int32 nHeight = 12;
    aSize.setHeight( nHeight );
    aSize = m_pDevice->LogicToPixel( aSize, MAP_APPFONT );
    m_spSeriesName->set_width_request(aSize.Width());
    m_spSeriesName->set_height_request(aSize.Height());

    // color bar
    aSize.setWidth(1);
    aSize = m_pDevice->LogicToPixel( aSize, MAP_APPFONT );
    m_spColorBar->set_margin_left(aSize.Width() + 2);
    nHeight = 3;
    aSize.setWidth( m_nWidth - 1 );
    aSize.setHeight( nHeight );
    aSize = m_pDevice->LogicToPixel( aSize, MAP_APPFONT );
    m_spColorBar->set_width_request(aSize.Width());
    m_spColorBar->set_height_request(aSize.Height());
}

void SeriesHeader::SetWidth( sal_Int32 nWidth )
{
    m_nWidth = nWidth;
    SetPos( m_aPos );
}

void SeriesHeader::SetPixelWidth( sal_Int32 nWidth )
{
    SetWidth( m_pDevice->PixelToLogic( Size( nWidth, 0 ), MAP_APPFONT ).getWidth());
}

void SeriesHeader::SetChartType(
    const Reference< chart2::XChartType > & xChartType,
    bool bSwapXAndYAxis
)
{
    m_spSymbol->SetImage( GetChartTypeImage( xChartType, bSwapXAndYAxis ) );
}

void SeriesHeader::SetSeriesName( const OUString & rName )
{
    m_spSeriesName->SetText( rName );
}

void SeriesHeader::SetRange( sal_Int32 nStartCol, sal_Int32 nEndCol )
{
    m_nStartCol = nStartCol;
    m_nEndCol = (nEndCol > nStartCol) ? nEndCol : nStartCol;
    m_spSeriesName->setStartColumn( nStartCol );
}

void SeriesHeader::Show()
{
    m_spSymbol->Show();
    m_spSeriesName->Show();
    m_spColorBar->Show();
}

void SeriesHeader::Hide()
{
    m_spSymbol->Hide();
    m_spSeriesName->Hide();
    m_spColorBar->Hide();
}

void SeriesHeader::SetEditChangedHdl( const Link<SeriesHeaderEdit*,void> & rLink )
{
    m_aChangeLink = rLink;
}

IMPL_LINK_NOARG_TYPED(SeriesHeader, SeriesNameChanged, Edit&, void)
{
    notifyChanges();
}

IMPL_LINK_NOARG_TYPED(SeriesHeader, SeriesNameEdited, Edit&, void)
{
    m_bSeriesNameChangePending = true;
}

void SeriesHeader::SetGetFocusHdl( const Link<Control&,void>& rLink )
{
    m_spSeriesName->SetGetFocusHdl( rLink );
}

bool SeriesHeader::HasFocus() const
{
    return m_spSeriesName->HasFocus();
}

Image SeriesHeader::GetChartTypeImage(
    const Reference< chart2::XChartType > & xChartType,
    bool bSwapXAndYAxis
)
{
    Image aResult;
    if( !xChartType.is())
        return aResult;
    OUString aChartTypeName( xChartType->getChartType());

    if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_AREA )
    {
        aResult = Image( SchResId( IMG_TYPE_AREA ) );
    }
    else if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_COLUMN )
    {
        if( bSwapXAndYAxis )
            aResult = Image( SchResId( IMG_TYPE_BAR ) );
        else
            aResult = Image( SchResId( IMG_TYPE_COLUMN ) );
    }
    else if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_LINE )
    {
        aResult = Image( SchResId( IMG_TYPE_LINE ) );
    }
    else if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_SCATTER )
    {
        aResult = Image( SchResId( IMG_TYPE_XY ) );
    }
    else if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_PIE )
    {
        aResult = Image( SchResId( IMG_TYPE_PIE ) );
    }
    else if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_NET
          || aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET )
    {
        aResult = Image( SchResId( IMG_TYPE_NET ) );
    }
    else if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK )
    {
        // @todo: correct image for candle-stick type
        aResult = Image( SchResId( IMG_TYPE_STOCK ) );
    }
    else if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE )
    {
        aResult = Image( SchResId( IMG_TYPE_BUBBLE ) );
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
    const ::std::vector< std::shared_ptr< ::chart::impl::SeriesHeader > > & rSeriesHeader,
    sal_Int32 * pOutIndex = nullptr )
{
    sal_Int32 nIndex = 0;
    for( auto aIt = rSeriesHeader.begin(); aIt != rSeriesHeader.end(); ++aIt, ++nIndex )
    {
        if( (*aIt)->HasFocus())
        {
            if( pOutIndex )
                *pOutIndex = nIndex;
            return true;
        }
    }
    return false;
}

sal_Int32 lcl_getColumnInDataOrHeader(
    sal_uInt16 nCol, const ::std::vector< std::shared_ptr< ::chart::impl::SeriesHeader > > & rSeriesHeader )
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

DataBrowser::DataBrowser( vcl::Window* pParent, WinBits nStyle, bool bLiveUpdate ) :
    ::svt::EditBrowseBox( pParent, EditBrowseBoxFlags::SMART_TAB_TRAVEL | EditBrowseBoxFlags::HANDLE_COLUMN_TEXT, nStyle, BrowserStdFlags ),
    m_nSeekRow( 0 ),
    m_bIsReadOnly( false ),
    m_bIsDirty( false ),
    m_bLiveUpdate( bLiveUpdate ),
    m_bDataValid( true ),
    m_aNumberEditField( VclPtr<FormattedField>::Create( & EditBrowseBox::GetDataWindow(), WB_NOBORDER ) ),
    m_aTextEditField( VclPtr<Edit>::Create( & EditBrowseBox::GetDataWindow(), WB_NOBORDER ) ),
    m_rNumberEditController( new ::svt::FormattedFieldCellController( m_aNumberEditField.get() )),
    m_rTextEditController( new ::svt::EditCellController( m_aTextEditField.get() ))
{
    double fNan;
    ::rtl::math::setNan( & fNan );
    m_aNumberEditField->SetDefaultValue( fNan );
    m_aNumberEditField->TreatAsNumber( true );
    RenewTable();
    SetClean();
}

DataBrowser::~DataBrowser()
{
    disposeOnce();
}

void DataBrowser::dispose()
{
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

bool DataBrowser::MaySwapRows() const
{
    return ! IsReadOnly()
        && ( !lcl_SeriesHeaderHasFocus( m_aSeriesHeaders ))
        && ( GetCurRow() >= 0 )
        && ( GetCurRow() < GetRowCount() - 1 );
}

bool DataBrowser::MaySwapColumns() const
{
    // if a series header (except the last one) has the focus
    {
        sal_Int32 nColIndex(0);
        if( lcl_SeriesHeaderHasFocus( m_aSeriesHeaders, &nColIndex ))
            return (static_cast< sal_uInt32 >( nColIndex ) < (m_aSeriesHeaders.size() - 1));
    }

    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( GetCurColumnId(), m_aSeriesHeaders );
    return ! IsReadOnly()
        && ( nColIdx > 0 )
        && ( nColIdx < ColCount()-2 )
        && m_apDataBrowserModel.get()
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
    if( ! m_apDataBrowserModel.get())
        return;

    long   nOldRow     = GetCurRow();
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

    OUString aDefaultSeriesName(SCH_RESSTR(STR_COLUMN_LABEL));
    replaceParamterInString( aDefaultSeriesName, "%COLUMNNUMBER", OUString::number( 24 ) );
    sal_Int32 nColumnWidth = GetDataWindow().GetTextWidth( aDefaultSeriesName )
        + GetDataWindow().LogicToPixel( Point( 4 + impl::SeriesHeader::GetRelativeAppFontXPosForNameField(), 0 ), MAP_APPFONT ).X();
    sal_Int32 nColumnCount = m_apDataBrowserModel->getColumnCount();
    // nRowCount is a member of a base class
    sal_Int32 nRowCountLocal = m_apDataBrowserModel->getMaxRowCount();
    for( sal_Int32 nColIdx=1; nColIdx<=nColumnCount; ++nColIdx )
    {
        InsertDataColumn( static_cast< sal_uInt16 >( nColIdx ), GetColString( nColIdx ), nColumnWidth );
    }

    RowInserted( 1, nRowCountLocal );
    GoToRow( ::std::min( nOldRow, GetRowCount() - 1 ));
    GoToColumnId( ::std::min( nOldColId, static_cast< sal_uInt16 >( ColCount() - 1 )));

    Dialog* pDialog = GetParentDialog();
    vcl::Window* pWin = pDialog->get<VclContainer>("columns");
    vcl::Window* pColorWin = pDialog->get<VclContainer>("colorcolumns");

    // fill series headers
    clearHeaders();
    const DataBrowserModel::tDataHeaderVector& aHeaders( m_apDataBrowserModel->getDataHeaders());
    Link<Control&,void> aFocusLink( LINK( this, DataBrowser, SeriesHeaderGotFocus ));
    Link<impl::SeriesHeaderEdit*,void> aSeriesHeaderChangedLink( LINK( this, DataBrowser, SeriesHeaderChanged ));

    for( DataBrowserModel::tDataHeaderVector::const_iterator aIt( aHeaders.begin());
         aIt != aHeaders.end(); ++aIt )
    {
        std::shared_ptr< impl::SeriesHeader > spHeader( new impl::SeriesHeader( pWin, pColorWin ));
        Reference< beans::XPropertySet > xSeriesProp( aIt->m_xDataSeries, uno::UNO_QUERY );
        sal_Int32 nColor = 0;
        // @todo: Set "DraftColor", i.e. interpolated colors for gradients, bitmaps, etc.
        if( xSeriesProp.is() &&
            ( xSeriesProp->getPropertyValue( "Color" ) >>= nColor ))
            spHeader->SetColor( Color( nColor ));
        spHeader->SetChartType( aIt->m_xChartType, aIt->m_bSwapXAndYAxis );
        spHeader->SetSeriesName(
            OUString( DataSeriesHelper::getDataSeriesLabel(
                        aIt->m_xDataSeries,
                        (aIt->m_xChartType.is() ?
                         aIt->m_xChartType->getRoleOfSequenceForSeriesLabel() :
                         OUString("values-y")))));
        // index is 1-based, as 0 is for the column that contains the row-numbers
        spHeader->SetRange( aIt->m_nStartColumn + 1, aIt->m_nEndColumn + 1 );
        spHeader->SetGetFocusHdl( aFocusLink );
        spHeader->SetEditChangedHdl( aSeriesHeaderChangedLink );
        m_aSeriesHeaders.push_back( spHeader );
    }

    ImplAdjustHeaderControls();
    SetDirty();
    SetUpdateMode( bLastUpdateMode );
    ActivateCell();
    Invalidate();
}

OUString DataBrowser::GetColString( sal_Int32 nColumnId ) const
{
    OSL_ASSERT( m_apDataBrowserModel.get());
    if( nColumnId > 0 )
        return OUString( m_apDataBrowserModel->getRoleOfColumn( static_cast< sal_Int32 >( nColumnId ) - 1 ));
    return OUString();
}

OUString DataBrowser::GetRowString( sal_Int32 nRow )
{
    return OUString::number(nRow + 1);
}

OUString DataBrowser::GetCellText( long nRow, sal_uInt16 nColumnId ) const
{
    OUString aResult;

    if( nColumnId == 0 )
    {
        aResult = GetRowString( static_cast< sal_Int32 >( nRow ));
    }
    else if( nRow >= 0 && m_apDataBrowserModel.get())
    {
        sal_Int32 nColIndex = static_cast< sal_Int32 >( nColumnId ) - 1;

        if( m_apDataBrowserModel->getCellType( nColIndex, nRow ) == DataBrowserModel::NUMBER )
        {
            double fData( m_apDataBrowserModel->getCellNumber( nColIndex, nRow ));
            sal_Int32 nLabelColor;

            if( ! ::rtl::math::isNan( fData ) &&
                m_spNumberFormatterWrapper.get() )
            {
                bool bColorChanged = false;
                aResult = m_spNumberFormatterWrapper->getFormattedString(
                                      GetNumberFormatKey( nRow, nColumnId ),
                                      fData, nLabelColor, bColorChanged );
            }
        }
        else if( m_apDataBrowserModel->getCellType( nColIndex, nRow ) == DataBrowserModel::TEXTORDATE )
        {
            uno::Any aAny = m_apDataBrowserModel->getCellAny( nColIndex, nRow );
            OUString aText;
            double fDouble=0.0;
            if( aAny>>=aText )
                aResult = aText;
            else if( aAny>>=fDouble )
            {
                if( ! ::rtl::math::isNan( fDouble ) && m_spNumberFormatterWrapper.get() )
                {
                    // If a numberformat was available here we could directly
                    // obtain the corresponding edit format in
                    // getDateTimeInputNumberFormat() instead of doing the
                    // guess work.
                    sal_Int32 nNumberFormat = DiagramHelper::getDateTimeInputNumberFormat(
                            Reference< util::XNumberFormatsSupplier >( m_xChartDoc, uno::UNO_QUERY), fDouble );
                    sal_Int32 nLabelColor;
                    bool bColorChanged = false;
                    aResult = m_spNumberFormatterWrapper->getFormattedString(
                        nNumberFormat, fDouble, nLabelColor, bColorChanged );
                }
            }
        }
        else
        {
            OSL_ASSERT( m_apDataBrowserModel->getCellType( nColIndex, nRow ) == DataBrowserModel::TEXT );
            aResult = m_apDataBrowserModel->getCellText( nColIndex, nRow );
        }
    }

    return aResult;
}

double DataBrowser::GetCellNumber( long nRow, sal_uInt16 nColumnId ) const
{
    double fResult;
    ::rtl::math::setNan( & fResult );

    if(( nColumnId >= 1 ) && ( nRow >= 0 ) &&
        m_apDataBrowserModel.get())
    {
        fResult = m_apDataBrowserModel->getCellNumber(
            static_cast< sal_Int32 >( nColumnId ) - 1, nRow );
    }

    return fResult;
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

void DataBrowser::SetClean()
{
    m_bIsDirty = false;
}

void DataBrowser::SetDirty()
{
    if( !m_bLiveUpdate )
        m_bIsDirty = true;
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
    ScopedVclPtrInstance<WarningBox>(this, WinBits( WB_OK ),
                                     SCH_RESSTR(STR_INVALID_NUMBER))->Execute();
}

bool DataBrowser::ShowQueryBox()
{
    ScopedVclPtrInstance<QueryBox> pQueryBox(this, WB_YES_NO, SCH_RESSTR(STR_DATA_EDITOR_INCORRECT_INPUT));

    return pQueryBox->Execute() == RET_YES;
}

bool DataBrowser::IsDataValid()
{
    bool bValid = true;
    const sal_Int32 nRow = lcl_getRowInData( GetCurRow());
    const sal_Int32 nCol = lcl_getColumnInData( GetCurColumnId());

    if( m_apDataBrowserModel->getCellType( nCol, nRow ) == DataBrowserModel::NUMBER )
    {
        sal_uInt32 nDummy = 0;
        double fDummy = 0.0;
        OUString aText( m_aNumberEditField->GetText());

        if( !aText.isEmpty() &&
            m_spNumberFormatterWrapper.get() &&
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
    SetDirty();
    m_aCursorMovedHdlLink.Call( this );
}

void DataBrowser::SetDataFromModel(
    const Reference< chart2::XChartDocument > & xChartDoc,
    const Reference< uno::XComponentContext > & xContext )
{
    if( m_bLiveUpdate )
    {
        m_xChartDoc.set( xChartDoc );
    }
    else
    {
        Reference< util::XCloneable > xCloneable( xChartDoc, uno::UNO_QUERY );
        if( xCloneable.is())
            m_xChartDoc.set( xCloneable->createClone(), uno::UNO_QUERY );
    }

    m_apDataBrowserModel.reset( new DataBrowserModel( m_xChartDoc, xContext ));
    m_spNumberFormatterWrapper.reset(
        new NumberFormatterWrapper(
            Reference< util::XNumberFormatsSupplier >( m_xChartDoc, uno::UNO_QUERY )));

    if( m_spNumberFormatterWrapper.get() )
        m_aNumberEditField->SetFormatter( m_spNumberFormatterWrapper->getSvNumberFormatter() );

    RenewTable();

    const sal_Int32 nColCnt  = m_apDataBrowserModel->getColumnCount();
    const sal_Int32 nRowCnt =  m_apDataBrowserModel->getMaxRowCount();
    if( nRowCnt && nColCnt )
    {
        GoToRow( 0 );
        GoToColumnId( 1 );
    }
    SetClean();
}

void DataBrowser::InsertColumn()
{
    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( GetCurColumnId(), m_aSeriesHeaders );

    if( nColIdx >= 0 &&
        m_apDataBrowserModel.get())
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

    if( nColIdx >= 0 &&
        m_apDataBrowserModel.get())
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

    if( nColIdx >= 0 &&
        m_apDataBrowserModel.get())
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
     sal_Int32 nRowIdx = lcl_getRowInData( GetCurRow());

     if( nRowIdx >= 0 &&
        m_apDataBrowserModel.get())
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
     sal_Int32 nRowIdx = lcl_getRowInData( GetCurRow());

     if( nRowIdx >= 0 &&
        m_apDataBrowserModel.get())
    {
        // save changes made to edit-field
        if( IsModified() )
            SaveModified();

        m_bDataValid = true;
        m_apDataBrowserModel->removeDataPointForAllSeries( nRowIdx );
        RenewTable();
    }
}

void DataBrowser::SwapColumn()
{
    sal_Int32 nColIdx = lcl_getColumnInDataOrHeader( GetCurColumnId(), m_aSeriesHeaders );

    if( nColIdx >= 0 &&
        m_apDataBrowserModel.get())
    {
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
}

void DataBrowser::SwapRow()
{
     sal_Int32 nRowIdx = lcl_getRowInData( GetCurRow());

     if( nRowIdx >= 0 &&
        m_apDataBrowserModel.get())
    {
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
}

void DataBrowser::SetCursorMovedHdl( const Link<DataBrowser*,void>& rLink )
{
    m_aCursorMovedHdlLink = rLink;
}

// implementations for ::svt::EditBrowseBox (pure virtual methods)
void DataBrowser::PaintCell(
    OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColumnId ) const
{
    Point aPos( rRect.TopLeft());
    aPos.X() += 1;

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

bool DataBrowser::SeekRow( long nRow )
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
    long nRow = GetCurRow();
    long nCol = GetCurColumnId();

    // column 0 is header-column
    long nBadCol = bForward
        ? GetColumnCount() - 1
        : 1;
    long nBadRow = bForward
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

::svt::CellController* DataBrowser::GetController( long nRow, sal_uInt16 nCol )
{
    if( m_bIsReadOnly )
        return nullptr;

    if( CellContainsNumbers( nRow, nCol ))
    {
        m_aNumberEditField->UseInputStringForFormatting();
        m_aNumberEditField->SetFormatKey( GetNumberFormatKey( nRow, nCol ));
        return m_rNumberEditController;
    }

    return m_rTextEditController;
}

void DataBrowser::InitController(
    ::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol )
{
    if( rController == m_rTextEditController )
    {
        OUString aText( GetCellText( nRow, nCol ) );
        m_aTextEditField->SetText( aText );
        m_aTextEditField->SetSelection( Selection( 0, aText.getLength() ));
    }
    else if( rController == m_rNumberEditController )
    {
        // treat invalid and empty text as Nan
        m_aNumberEditField->EnableNotANumber( true );
        if( ::rtl::math::isNan( GetCellNumber( nRow, nCol )))
            m_aNumberEditField->SetTextValue( OUString());
        else
            m_aNumberEditField->SetValue( GetCellNumber( nRow, nCol ) );
        OUString aText( m_aNumberEditField->GetText());
        m_aNumberEditField->SetSelection( Selection( 0, aText.getLength()));
    }
    else
    {
        OSL_FAIL( "Invalid Controller" );
    }
}

bool DataBrowser::CellContainsNumbers( sal_Int32 nRow, sal_uInt16 nCol ) const
{
    if( ! m_apDataBrowserModel.get())
        return false;
    return (m_apDataBrowserModel->getCellType( lcl_getColumnInData( nCol ), lcl_getRowInData( nRow )) ==
            DataBrowserModel::NUMBER);
}

sal_uInt32 DataBrowser::GetNumberFormatKey( sal_Int32 nRow, sal_uInt16 nCol ) const
{
    if( ! m_apDataBrowserModel.get())
        return 0;
    return m_apDataBrowserModel->getNumberFormatKey( lcl_getColumnInData( nCol ), lcl_getRowInData( nRow ));
}

bool DataBrowser::isDateTimeString( const OUString& aInputString, double& fOutDateTimeValue )
{
    sal_uInt32 nNumberFormat=0;
    SvNumberFormatter* pSvNumberFormatter = m_spNumberFormatterWrapper.get() ? m_spNumberFormatterWrapper->getSvNumberFormatter() : nullptr;
    if( !aInputString.isEmpty() &&  pSvNumberFormatter && pSvNumberFormatter->IsNumberFormat( aInputString, nNumberFormat, fOutDateTimeValue ) )
    {
        short nType = pSvNumberFormatter->GetType( nNumberFormat);
        return (nType & util::NumberFormat::DATE) || (nType & util::NumberFormat::TIME);
    }
    return false;
}

bool DataBrowser::SaveModified()
{
    if( ! IsModified() )
        return true;

    bool bChangeValid = true;

    const sal_Int32 nRow = lcl_getRowInData( GetCurRow());
    const sal_Int32 nCol = lcl_getColumnInData( GetCurColumnId());

    OSL_ENSURE( nRow >= 0 || nCol >= 0, "This cell should not be modified!" );

    SvNumberFormatter* pSvNumberFormatter = m_spNumberFormatterWrapper.get() ? m_spNumberFormatterWrapper->getSvNumberFormatter() : nullptr;
    switch( m_apDataBrowserModel->getCellType( nCol, nRow ))
    {
        case DataBrowserModel::NUMBER:
        {
            sal_uInt32 nDummy = 0;
            double fDummy = 0.0;
            OUString aText( m_aNumberEditField->GetText());
            // an empty string is valid, if no numberformatter exists, all
            // values are treated as valid
            if( !aText.isEmpty() && pSvNumberFormatter &&
                ! pSvNumberFormatter->IsNumberFormat( aText, nDummy, fDummy ) )
            {
                bChangeValid = false;
            }
            else
            {
                double fData = m_aNumberEditField->GetValue();
                bChangeValid = m_apDataBrowserModel->setCellNumber( nCol, nRow, fData );
            }
        }
        break;
        case DataBrowserModel::TEXTORDATE:
        {
            OUString aText( m_aTextEditField->GetText() );
            double fValue = 0.0;
            bChangeValid = false;
            if( isDateTimeString( aText, fValue ) )
                bChangeValid = m_apDataBrowserModel->setCellAny( nCol, nRow, uno::makeAny( fValue ) );
            if(!bChangeValid)
                bChangeValid = m_apDataBrowserModel->setCellAny( nCol, nRow, uno::makeAny( aText ) );
        }
        break;
        case DataBrowserModel::TEXT:
        {
            OUString aText( m_aTextEditField->GetText());
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
            pCtrl->ClearModified();
        SetDirty();
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

sal_Int16 DataBrowser::GetFirstVisibleColumNumber() const
{
    return GetFirstVisibleColNumber();
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
    Dialog* pDialog = GetParentDialog();
    vcl::Window* pWin = pDialog->get<VclContainer>("columns");
    vcl::Window* pColorWin = pDialog->get<VclContainer>("colorcolumns");

    clearHeaders();
    DataBrowserModel::tDataHeaderVector aHeaders( m_apDataBrowserModel->getDataHeaders());
    Link<Control&,void> aFocusLink( LINK( this, DataBrowser, SeriesHeaderGotFocus ));
    Link<impl::SeriesHeaderEdit*,void> aSeriesHeaderChangedLink( LINK( this, DataBrowser, SeriesHeaderChanged ));

    for( DataBrowserModel::tDataHeaderVector::const_iterator aIt( aHeaders.begin());
         aIt != aHeaders.end(); ++aIt )
    {
        std::shared_ptr< impl::SeriesHeader > spHeader( new impl::SeriesHeader( pWin, pColorWin ));
        Reference< beans::XPropertySet > xSeriesProp( aIt->m_xDataSeries, uno::UNO_QUERY );
        sal_Int32 nColor = 0;
        if( xSeriesProp.is() &&
            ( xSeriesProp->getPropertyValue( "Color" ) >>= nColor ))
            spHeader->SetColor( Color( nColor ));
        spHeader->SetChartType( aIt->m_xChartType, aIt->m_bSwapXAndYAxis );
        spHeader->SetSeriesName(
            DataSeriesHelper::getDataSeriesLabel(
                        aIt->m_xDataSeries,
                        (aIt->m_xChartType.is() ?
                         aIt->m_xChartType->getRoleOfSequenceForSeriesLabel() :
                         OUString( "values-y"))));
        spHeader->SetRange( aIt->m_nStartColumn + 1, aIt->m_nEndColumn + 1 );
        spHeader->SetGetFocusHdl( aFocusLink );
        spHeader->SetEditChangedHdl( aSeriesHeaderChangedLink );
        m_aSeriesHeaders.push_back( spHeader );
    }

    ImplAdjustHeaderControls();
}

void DataBrowser::ImplAdjustHeaderControls()
{
    sal_uInt16 nColCount = this->GetColumnCount();
    sal_uInt32 nCurrentPos = this->GetPosPixel().getX();
    sal_uInt32 nMaxPos = nCurrentPos + this->GetOutputSizePixel().getWidth();
    sal_uInt32 nStartPos = nCurrentPos;

    // width of header column
    nCurrentPos +=  this->GetColumnWidth( 0 );

    Dialog* pDialog = GetParentDialog();
    vcl::Window* pWin = pDialog->get<VclContainer>("columns");
    vcl::Window* pColorWin = pDialog->get<VclContainer>("colorcolumns");
    pWin->set_margin_left(nCurrentPos);
    pColorWin->set_margin_left(nCurrentPos);

    tSeriesHeaderContainer::iterator aIt( m_aSeriesHeaders.begin());
    sal_uInt16 i = this->GetFirstVisibleColumNumber();
    while( (aIt != m_aSeriesHeaders.end()) && ((*aIt)->GetStartColumn() < i) )
    {
        (*aIt)->Hide();
        ++aIt;
    }
    for( ; i < nColCount && aIt != m_aSeriesHeaders.end(); ++i )
    {
        if( (*aIt)->GetStartColumn() == i )
            nStartPos = nCurrentPos;

        nCurrentPos += (this->GetColumnWidth( i ));

        if( (*aIt)->GetEndColumn() == i )
        {
            if( nStartPos < nMaxPos )
            {
                (*aIt)->SetPixelWidth( nCurrentPos - nStartPos - 3 );
                (*aIt)->Show();

                if (pWin)
                {
                    pWin->set_margin_left(nStartPos);
                    pColorWin->set_margin_left(nStartPos);
                    pWin = pColorWin = nullptr;
                }

            }
            else
                (*aIt)->Hide();
            ++aIt;
        }
    }
}

IMPL_LINK_TYPED( DataBrowser, SeriesHeaderGotFocus, Control&, rControl, void )
{
    impl::SeriesHeaderEdit* pEdit = static_cast<impl::SeriesHeaderEdit*>(&rControl);
    pEdit->SetShowWarningBox( !m_bDataValid );

    if( !m_bDataValid )
        GoToCell( 0, 0 );
    else
    {
        MakeFieldVisible( GetCurRow(), static_cast< sal_uInt16 >( pEdit->getStartColumn()) );
        ActivateCell();
        m_aCursorMovedHdlLink.Call( this );
    }
}

IMPL_LINK_TYPED( DataBrowser, SeriesHeaderChanged, impl::SeriesHeaderEdit*, pEdit, void )
{
    if( pEdit )
    {
        Reference< chart2::XDataSeries > xSeries(
            m_apDataBrowserModel->getDataSeriesByColumn( pEdit->getStartColumn() - 1 ));
        Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
        if( xSource.is())
        {
            Reference< chart2::XChartType > xChartType(
                m_apDataBrowserModel->getHeaderForSeries( xSeries ).m_xChartType );
            if( xChartType.is())
            {
                Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
                    DataSeriesHelper::getDataSequenceByRole( xSource, xChartType->getRoleOfSequenceForSeriesLabel()));
                if( xLabeledSeq.is())
                {
                    Reference< container::XIndexReplace > xIndexReplace( xLabeledSeq->getLabel(), uno::UNO_QUERY );
                    if( xIndexReplace.is())
                        xIndexReplace->replaceByIndex(
                            0, uno::makeAny( OUString( pEdit->GetText())));
                }
            }
        }
    }
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
