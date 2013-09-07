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

// header for class SvNumberformat
#include <svl/zformat.hxx>
// header for SvNumberFormatter
#include <svl/zforlist.hxx>

#include "DataBrowser.hxx"
#include "DataBrowserModel.hxx"
#include "Strings.hrc"
#include "ContainerHelper.hxx"
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
#include <vcl/msgbox.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartType.hpp>

#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>

#include <algorithm>
#include <functional>

/*  BROWSER_COLUMNSELECTION :  single cells may be selected rather than only
                               entire rows
    BROWSER_(H|V)LINES :       show horizontal or vertical grid-lines

    BROWSER_AUTO_(H|V)SCROLL : scroll automated horizontally or vertically when
                               cursor is moved beyond the edge of the dialog
    BROWSER_HIGHLIGHT_NONE :   Do not mark the current row with selection color
                               (usually blue)

 */
#define BROWSER_STANDARD_FLAGS  \
    BROWSER_COLUMNSELECTION | \
    BROWSER_HLINES | BROWSER_VLINES | \
    BROWSER_AUTO_HSCROLL | BROWSER_AUTO_VSCROLL | \
    BROWSER_HIGHLIGHT_NONE

// BROWSER_HIDECURSOR would prevent flickering in edit fields, but navigating
// with shift up/down, and entering non-editable cells would be problematic,
// e.g.  the first cell, or when being in read-only mode

using namespace ::com::sun::star;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;

using namespace ::svt;

namespace
{
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
    SeriesHeaderEdit( Window * pParent );
    virtual ~SeriesHeaderEdit();
    virtual void MouseButtonDown( const MouseEvent& rMEvt );

    void setStartColumn( sal_Int32 nStartColumn );
    sal_Int32 getStartColumn() const;
    void SetShowWarningBox( bool bShowWarning = true );

private:
    sal_Int32 m_nStartColumn;
    bool m_bShowWarningBox;
};

SeriesHeaderEdit::SeriesHeaderEdit( Window * pParent ) :
        Edit( pParent ),
        m_nStartColumn( 0 ),
        m_bShowWarningBox( false )
{}

SeriesHeaderEdit::~SeriesHeaderEdit()
{}

void SeriesHeaderEdit::setStartColumn( sal_Int32 nStartColumn )
{
    m_nStartColumn = nStartColumn;
}

sal_Int32 SeriesHeaderEdit::getStartColumn() const
{
    return m_nStartColumn;
}

void SeriesHeaderEdit::SetShowWarningBox( bool bShowWarning )
{
    m_bShowWarningBox = bShowWarning;
}

void SeriesHeaderEdit::MouseButtonDown( const MouseEvent& rMEvt )
{
    Edit::MouseButtonDown( rMEvt );

    if( m_bShowWarningBox )
        WarningBox(this, WinBits( WB_OK ),
                   SCH_RESSTR(STR_INVALID_NUMBER)).Execute();
}

class SeriesHeader
{
public:
    explicit SeriesHeader( Window * pParent );

    void SetColor( const Color & rCol );
    void SetPos( const Point & rPos );
    void SetWidth( sal_Int32 nWidth );
    void SetChartType( const Reference< chart2::XChartType > & xChartType,
                       bool bSwapXAndYAxis );
    void SetSeriesName( const OUString & rName );
    void SetRange( sal_Int32 nStartCol, sal_Int32 nEndCol );

    void SetPixelPosX( sal_Int32 nPos );
    void SetPixelWidth( sal_Int32 nWidth );

    sal_Int32 GetStartColumn() const;
    sal_Int32 GetEndColumn() const;

    static sal_Int32 GetRelativeAppFontXPosForNameField();

    void Show();

    /** call this before destroying the class.  This notifies the listeners to
        changes of the edit field for the series name.
     */
    void applyChanges();

    void SetGetFocusHdl( const Link& rLink );

    void SetEditChangedHdl( const Link & rLink );

    bool HasFocus() const;

private:
    ::boost::shared_ptr< FixedImage >        m_spSymbol;
    ::boost::shared_ptr< SeriesHeaderEdit >  m_spSeriesName;
    ::boost::shared_ptr< FixedText >         m_spColorBar;
    OutputDevice *                           m_pDevice;
    Link                                     m_aChangeLink;

    void notifyChanges();
    DECL_LINK( SeriesNameChanged, void * );
    DECL_LINK( SeriesNameEdited, void * );

    static Image GetChartTypeImage(
        const Reference< chart2::XChartType > & xChartType,
        bool bSwapXAndYAxis
        );

    sal_Int32 m_nStartCol, m_nEndCol;
    sal_Int32 m_nWidth;
    Point     m_aPos;
    bool      m_bSeriesNameChangePending;
};

SeriesHeader::SeriesHeader( Window * pParent ) :
        m_spSymbol( new FixedImage( pParent, WB_NOBORDER )),
        m_spSeriesName( new SeriesHeaderEdit( pParent )),
        m_spColorBar( new FixedText( pParent, WB_NOBORDER )),
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
    m_spSeriesName->SetHelpId( HID_SCH_DATA_SERIES_LABEL );
    Show();
}

void SeriesHeader::notifyChanges()
{
    if( m_aChangeLink.IsSet())
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

const sal_Int32 nSymbolHeight = 10;
const sal_Int32 nSymbolDistance = 2;

sal_Int32 SeriesHeader::GetRelativeAppFontXPosForNameField()
{
    return nSymbolHeight + nSymbolDistance;
}

void SeriesHeader::SetPos( const Point & rPos )
{
    m_aPos = rPos;

    // chart type symbol
    Point aPos( rPos );
    aPos.setY( aPos.getY() + nSymbolDistance );
    Size aSize( nSymbolHeight, nSymbolHeight );
    m_spSymbol->SetPosPixel( m_pDevice->LogicToPixel( aPos, MAP_APPFONT ));
    m_spSymbol->SetSizePixel( m_pDevice->LogicToPixel( aSize, MAP_APPFONT ));
    aPos.setY( aPos.getY() - nSymbolDistance );

    // series name edit field
    aPos.setX( aPos.getX() + nSymbolHeight + nSymbolDistance );
    aSize.setWidth( m_nWidth - nSymbolHeight - nSymbolDistance );
    sal_Int32 nHeight = 12;
    aSize.setHeight( nHeight );
    m_spSeriesName->SetPosPixel( m_pDevice->LogicToPixel( aPos, MAP_APPFONT ));
    m_spSeriesName->SetSizePixel( m_pDevice->LogicToPixel( aSize, MAP_APPFONT ));

    // color bar
    aPos.setX( rPos.getX() + 1 );
    aPos.setY( aPos.getY() + nHeight + 2 );
    nHeight = 3;
    aSize.setWidth( m_nWidth - 1 );
    aSize.setHeight( nHeight );
    m_spColorBar->SetPosPixel( m_pDevice->LogicToPixel( aPos, MAP_APPFONT ));
    m_spColorBar->SetSizePixel( m_pDevice->LogicToPixel( aSize, MAP_APPFONT ));
}

void SeriesHeader::SetWidth( sal_Int32 nWidth )
{
    m_nWidth = nWidth;
    SetPos( m_aPos );
}

void SeriesHeader::SetPixelPosX( sal_Int32 nPos )
{
    Point aPos( m_pDevice->LogicToPixel( m_aPos, MAP_APPFONT ));
    aPos.setX( nPos );
    SetPos( m_pDevice->PixelToLogic( aPos, MAP_APPFONT ));
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

sal_Int32 SeriesHeader::GetStartColumn() const
{
    return m_nStartCol;
}

sal_Int32 SeriesHeader::GetEndColumn() const
{
    return m_nEndCol;
}

void SeriesHeader::Show()
{
    m_spSymbol->Show();
    m_spSeriesName->Show();
    m_spColorBar->Show();
}

void SeriesHeader::SetEditChangedHdl( const Link & rLink )
{
    m_aChangeLink = rLink;
}

IMPL_LINK_NOARG(SeriesHeader, SeriesNameChanged)
{
    notifyChanges();
    return 0;
}

IMPL_LINK_NOARG(SeriesHeader, SeriesNameEdited)
{
    m_bSeriesNameChangePending = true;
    return 0;
}

void SeriesHeader::SetGetFocusHdl( const Link& rLink )
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

    if( aChartTypeName.equals( CHART2_SERVICE_NAME_CHARTTYPE_AREA ))
    {
        aResult = Image( SchResId( IMG_TYPE_AREA ) );
    }
    else if( aChartTypeName.equals( CHART2_SERVICE_NAME_CHARTTYPE_COLUMN ))
    {
        if( bSwapXAndYAxis )
            aResult = Image( SchResId( IMG_TYPE_BAR ) );
        else
            aResult = Image( SchResId( IMG_TYPE_COLUMN ) );
    }
    else if( aChartTypeName.equals( CHART2_SERVICE_NAME_CHARTTYPE_LINE ))
    {
        aResult = Image( SchResId( IMG_TYPE_LINE ) );
    }
    else if( aChartTypeName.equals( CHART2_SERVICE_NAME_CHARTTYPE_SCATTER ))
    {
        aResult = Image( SchResId( IMG_TYPE_XY ) );
    }
    else if( aChartTypeName.equals( CHART2_SERVICE_NAME_CHARTTYPE_PIE ))
    {
        aResult = Image( SchResId( IMG_TYPE_PIE ) );
    }
    else if( aChartTypeName.equals( CHART2_SERVICE_NAME_CHARTTYPE_NET )
          || aChartTypeName.equals( CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET ) )
    {
        aResult = Image( SchResId( IMG_TYPE_NET ) );
    }
    else if( aChartTypeName.equals( CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK ))
    {
        // @todo: correct image for candle-stick type
        aResult = Image( SchResId( IMG_TYPE_STOCK ) );
    }
    else if( aChartTypeName.equals( CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE ))
    {
        aResult = Image( SchResId( IMG_TYPE_BUBBLE ) );
    }

    return aResult;
}

struct applyChangesFunctor : public ::std::unary_function< ::boost::shared_ptr< SeriesHeader >, void >
{
    void operator() ( ::boost::shared_ptr< SeriesHeader > spHeader )
    {
        spHeader->applyChanges();
    }
};

} // namespace impl

namespace
{

/** returns false, if no header as the focus.

    If a header has the focus, true is returned and the index of the header
    with focus is set at pIndex if pOutIndex is not 0.
*/
bool lcl_SeriesHeaderHasFocus(
    const ::std::vector< ::boost::shared_ptr< ::chart::impl::SeriesHeader > > & rSeriesHeader,
    sal_Int32 * pOutIndex = 0 )
{
    sal_Int32 nIndex = 0;
    for( ::std::vector< ::boost::shared_ptr< ::chart::impl::SeriesHeader > >::const_iterator aIt( rSeriesHeader.begin());
         aIt != rSeriesHeader.end(); ++aIt, ++nIndex )
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
    sal_uInt16 nCol, const ::std::vector< ::boost::shared_ptr< ::chart::impl::SeriesHeader > > & rSeriesHeader )
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

DataBrowser::DataBrowser( Window* pParent, const ResId& rId, bool bLiveUpdate ) :
    ::svt::EditBrowseBox( pParent, rId, EBBF_SMART_TAB_TRAVEL | EBBF_HANDLE_COLUMN_TEXT, BROWSER_STANDARD_FLAGS ),
    m_nSeekRow( 0 ),
    m_bIsReadOnly( false ),
    m_bIsDirty( false ),
    m_bLiveUpdate( bLiveUpdate ),
    m_bDataValid( true ),
    m_aNumberEditField( & EditBrowseBox::GetDataWindow(), WB_NOBORDER ),
    m_aTextEditField( & EditBrowseBox::GetDataWindow(), WB_NOBORDER ),
    m_rNumberEditController( new ::svt::FormattedFieldCellController( & m_aNumberEditField )),
    m_rTextEditController( new ::svt::EditCellController( & m_aTextEditField ))
{
    double fNan;
    ::rtl::math::setNan( & fNan );
    m_aNumberEditField.SetDefaultValue( fNan );
    m_aNumberEditField.TreatAsNumber( sal_True );
    RenewTable();
    SetClean();
}

DataBrowser::~DataBrowser()
{
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
    ::std::for_each( m_aSeriesHeaders.begin(), m_aSeriesHeaders.end(), impl::applyChangesFunctor());
    m_aSeriesHeaders.clear();
}

void DataBrowser::RenewTable()
{
    if( ! m_apDataBrowserModel.get())
        return;

    long   nOldRow     = GetCurRow();
    sal_uInt16 nOldColId   = GetCurColumnId();

    sal_Bool bLastUpdateMode = GetUpdateMode();
    SetUpdateMode( sal_False );

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

    Window * pWin = this->GetParent();
    if( !pWin )
        pWin = this;

    // fill series headers
    clearHeaders();
    const DataBrowserModel::tDataHeaderVector& aHeaders( m_apDataBrowserModel->getDataHeaders());
    Link aFocusLink( LINK( this, DataBrowser, SeriesHeaderGotFocus ));
    Link aSeriesHeaderChangedLink( LINK( this, DataBrowser, SeriesHeaderChanged ));

    for( DataBrowserModel::tDataHeaderVector::const_iterator aIt( aHeaders.begin());
         aIt != aHeaders.end(); ++aIt )
    {
        ::boost::shared_ptr< impl::SeriesHeader > spHeader( new impl::SeriesHeader( pWin ));
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

OUString DataBrowser::GetRowString( sal_Int32 nRow ) const
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
    else if( nRow >= 0 &&
             m_apDataBrowserModel.get())
    {
        sal_Int32 nColIndex = static_cast< sal_Int32 >( nColumnId ) - 1;

        if( m_apDataBrowserModel->getCellType( nColIndex, nRow ) == DataBrowserModel::NUMBER )
        {
            double fData( m_apDataBrowserModel->getCellNumber( nColIndex, nRow ));
            sal_Int32 nLabelColor;
            bool bColorChanged = false;

            if( ! ::rtl::math::isNan( fData ) &&
                m_spNumberFormatterWrapper.get() )
                aResult = m_spNumberFormatterWrapper->getFormattedString(
                                      GetNumberFormatKey( nRow, nColumnId ),
                                      fData, nLabelColor, bColorChanged );
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
                sal_Int32 nLabelColor;
                bool bColorChanged = false;
                sal_Int32 nDateNumberFormat = DiagramHelper::getDateNumberFormat( Reference< util::XNumberFormatsSupplier >( m_xChartDoc, uno::UNO_QUERY) );
                if( ! ::rtl::math::isNan( fDouble ) && m_spNumberFormatterWrapper.get() )
                    aResult = m_spNumberFormatterWrapper->getFormattedString(
                        nDateNumberFormat, fDouble, nLabelColor, bColorChanged );
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
    sal_Bool bLastUpdateMode = GetUpdateMode();
    SetUpdateMode( sal_False );

    ::svt::EditBrowseBox::Resize();
    ImplAdjustHeaderControls();
    SetUpdateMode( bLastUpdateMode );
}

bool DataBrowser::SetReadOnly( bool bNewState )
{
    bool bResult = m_bIsReadOnly;

    if( m_bIsReadOnly != bNewState )
    {
        m_bIsReadOnly = bNewState;
        Invalidate();
        DeactivateCell();
    }

    return bResult;
}

bool DataBrowser::IsReadOnly() const
{
    return m_bIsReadOnly;
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

    if( GetUpdateMode() && m_aCursorMovedHdlLink.IsSet())
        m_aCursorMovedHdlLink.Call( this );
}

void DataBrowser::SetCellModifiedHdl( const Link& rLink )
{
    m_aCellModifiedLink = rLink;
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
    WarningBox(this, WinBits( WB_OK ),
               SCH_RESSTR(STR_INVALID_NUMBER)).Execute();
}

bool DataBrowser::ShowQueryBox()
{
    QueryBox* pQueryBox = new QueryBox(this, WB_YES_NO, SCH_RESSTR(STR_DATA_EDITOR_INCORRECT_INPUT));

    return ( pQueryBox->Execute() == RET_YES );
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
        OUString aText( m_aNumberEditField.GetText());

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

bool DataBrowser::IsEnableItem()
{
    return m_bDataValid;
}

void DataBrowser::CellModified()
{
    m_bDataValid = IsDataValid();
    SetDirty();
    if( m_aCellModifiedLink.IsSet())
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
        m_aNumberEditField.SetFormatter( m_spNumberFormatterWrapper->getSvNumberFormatter() );

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

void DataBrowser::SetCursorMovedHdl( const Link& rLink )
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
        rDev.SetClipRegion(Region(rRect));

    // allow for a disabled control ...
    sal_Bool bEnabled = IsEnabled();
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

sal_Bool DataBrowser::SeekRow( long nRow )
{
    if( ! EditBrowseBox::SeekRow( nRow ))
        return sal_False;

    if( nRow < 0 )
        m_nSeekRow = - 1;
    else
        m_nSeekRow = nRow;

    return sal_True;
}

sal_Bool DataBrowser::IsTabAllowed( sal_Bool bForward ) const
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
        return sal_False;
    }

    return ( nRow != nBadRow ||
             nCol != nBadCol );
}

::svt::CellController* DataBrowser::GetController( long nRow, sal_uInt16 nCol )
{
    if( m_bIsReadOnly )
        return 0;

    if( CellContainsNumbers( nRow, nCol ))
    {
        m_aNumberEditField.UseInputStringForFormatting();
        m_aNumberEditField.SetFormatKey( GetNumberFormatKey( nRow, nCol ));
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
        m_aTextEditField.SetText( aText );
        m_aTextEditField.SetSelection( Selection( 0, aText.getLength() ));
    }
    else if( rController == m_rNumberEditController )
    {
        // treat invalid and empty text as Nan
        m_aNumberEditField.EnableNotANumber( true );
        if( ::rtl::math::isNan( GetCellNumber( nRow, nCol )))
            m_aNumberEditField.SetTextValue( OUString());
        else
            m_aNumberEditField.SetValue( GetCellNumber( nRow, nCol ) );
        OUString aText( m_aNumberEditField.GetText());
        m_aNumberEditField.SetSelection( Selection( 0, aText.getLength()));
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

bool DataBrowser::isDateString( OUString aInputString, double& fOutDateValue )
{
    sal_uInt32 nNumberFormat=0;
    SvNumberFormatter* pSvNumberFormatter = m_spNumberFormatterWrapper.get() ? m_spNumberFormatterWrapper->getSvNumberFormatter() : 0;
    if( !aInputString.isEmpty() &&  pSvNumberFormatter && pSvNumberFormatter->IsNumberFormat( aInputString, nNumberFormat, fOutDateValue ) )
    {
        Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( m_xChartDoc, uno::UNO_QUERY );
        Reference< util::XNumberFormats > xNumberFormats;
        if( xNumberFormatsSupplier.is() )
             xNumberFormats = Reference< util::XNumberFormats >( xNumberFormatsSupplier->getNumberFormats() );
        if( DiagramHelper::isDateNumberFormat( nNumberFormat, xNumberFormats ) )
            return true;
    }
    return false;
}

sal_Bool DataBrowser::SaveModified()
{
    if( ! IsModified() )
        return sal_True;

    sal_Bool bChangeValid = sal_True;

    const sal_Int32 nRow = lcl_getRowInData( GetCurRow());
    const sal_Int32 nCol = lcl_getColumnInData( GetCurColumnId());

    OSL_ENSURE( nRow >= 0 || nCol >= 0, "This cell should not be modified!" );

    SvNumberFormatter* pSvNumberFormatter = m_spNumberFormatterWrapper.get() ? m_spNumberFormatterWrapper->getSvNumberFormatter() : 0;
    switch( m_apDataBrowserModel->getCellType( nCol, nRow ))
    {
        case DataBrowserModel::NUMBER:
        {
            sal_uInt32 nDummy = 0;
            double fDummy = 0.0;
            OUString aText( m_aNumberEditField.GetText());
            // an empty string is valid, if no numberformatter exists, all
            // values are treated as valid
            if( !aText.isEmpty() && pSvNumberFormatter &&
                ! pSvNumberFormatter->IsNumberFormat( aText, nDummy, fDummy ) )
            {
                bChangeValid = sal_False;
            }
            else
            {
                double fData = m_aNumberEditField.GetValue();
                bChangeValid = m_apDataBrowserModel->setCellNumber( nCol, nRow, fData );
            }
        }
        break;
        case DataBrowserModel::TEXTORDATE:
        {
            OUString aText( m_aTextEditField.GetText() );
            double fDateValue=0.0;
            bChangeValid = sal_False;
            if( isDateString( aText, fDateValue ) )
                bChangeValid = m_apDataBrowserModel->setCellAny( nCol, nRow, uno::makeAny( fDateValue ) );
            if(!bChangeValid)
                bChangeValid = m_apDataBrowserModel->setCellAny( nCol, nRow, uno::makeAny( aText ) );
        }
        break;
        case DataBrowserModel::TEXT:
        {
            OUString aText( m_aTextEditField.GetText());
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
    if( IsModified())
        SaveModified();

    // apply changes made to series headers
    ::std::for_each( m_aSeriesHeaders.begin(), m_aSeriesHeaders.end(), impl::applyChangesFunctor());

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
    sal_Bool bLastUpdateMode = GetUpdateMode();
    SetUpdateMode( sal_False );

    EditBrowseBox::ColumnResized( nColId );
    ImplAdjustHeaderControls();
    SetUpdateMode( bLastUpdateMode );
}

void DataBrowser::EndScroll()
{
    sal_Bool bLastUpdateMode = GetUpdateMode();
    SetUpdateMode( sal_False );

    EditBrowseBox::EndScroll();
    RenewSeriesHeaders();

    SetUpdateMode( bLastUpdateMode );
}

void DataBrowser::RenewSeriesHeaders()
{
    Window * pWin = this->GetParent();
    if( !pWin )
        pWin = this;

    clearHeaders();
    DataBrowserModel::tDataHeaderVector aHeaders( m_apDataBrowserModel->getDataHeaders());
    Link aFocusLink( LINK( this, DataBrowser, SeriesHeaderGotFocus ));
    Link aSeriesHeaderChangedLink( LINK( this, DataBrowser, SeriesHeaderChanged ));

    for( DataBrowserModel::tDataHeaderVector::const_iterator aIt( aHeaders.begin());
         aIt != aHeaders.end(); ++aIt )
    {
        ::boost::shared_ptr< impl::SeriesHeader > spHeader( new impl::SeriesHeader( pWin ));
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
    tSeriesHeaderContainer::iterator aIt( m_aSeriesHeaders.begin());
    sal_uInt16 i = this->GetFirstVisibleColumNumber();
    while( (aIt != m_aSeriesHeaders.end()) && ((*aIt)->GetStartColumn() < i) )
        ++aIt;
    for( ; i < nColCount && aIt != m_aSeriesHeaders.end(); ++i )
    {
        if( (*aIt)->GetStartColumn() == i )
            nStartPos = nCurrentPos;

        nCurrentPos += (this->GetColumnWidth( i ));

        if( (*aIt)->GetEndColumn() == i )
        {
            if( nStartPos < nMaxPos )
            {
                (*aIt)->SetPixelPosX( nStartPos + 2 );
                (*aIt)->SetPixelWidth( nCurrentPos - nStartPos - 3 );
            }
            else
                // do not hide, to get focus events. Move outside the dialog for "hiding"
                (*aIt)->SetPixelPosX( nMaxPos + 42 );
            ++aIt;
        }
    }
}

IMPL_LINK( DataBrowser, SeriesHeaderGotFocus, impl::SeriesHeaderEdit*, pEdit )
{
    if( pEdit )
    {
        pEdit->SetShowWarningBox( !m_bDataValid );

        if( !m_bDataValid )
            GoToCell( 0, 0 );
        else
        {
            MakeFieldVisible( GetCurRow(), static_cast< sal_uInt16 >( pEdit->getStartColumn()), true /* bComplete */ );
            ActivateCell();
            m_aCursorMovedHdlLink.Call( this );
        }
    }
    return 0;
}

IMPL_LINK( DataBrowser, SeriesHeaderChanged, impl::SeriesHeaderEdit*, pEdit )
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
    return 0;
}

sal_Int32 DataBrowser::GetTotalWidth() const
{
    sal_uLong nWidth = 0;
    for ( sal_uInt16 nCol = 0; nCol < ColCount(); ++nCol )
        nWidth += GetColumnWidth( nCol );
    return static_cast< sal_Int32 >( nWidth );
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
