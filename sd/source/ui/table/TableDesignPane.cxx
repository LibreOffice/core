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

#include "sddll.hxx"

#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>

#include <comphelper/processfactory.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/layout.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>

#include <svl/style.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>
#include <svx/svdetc.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/colritem.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdr/table/tabledesign.hxx>
#include <o3tl/enumrange.hxx>

#include "TableDesignPane.hxx"
#include "createtabledesignpanel.hxx"

#include "DrawDocShell.hxx"
#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"
#include "DrawController.hxx"
#include "glob.hrc"
#include "sdresid.hxx"
#include "EventMultiplexer.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui;

namespace sd {

static const sal_Int32 nPreviewColumns = 5;
static const sal_Int32 nPreviewRows = 5;
static const sal_Int32 nCellWidth = 12; // one pixel is shared with the next cell!
static const sal_Int32 nCellHeight = 7; // one pixel is shared with the next cell!
static const sal_Int32 nBitmapWidth = (nCellWidth * nPreviewColumns) - (nPreviewColumns - 1);
static const sal_Int32 nBitmapHeight = (nCellHeight * nPreviewRows) - (nPreviewRows - 1);

static const OUStringLiteral gPropNames[ CB_COUNT ] =
{
    OUStringLiteral("UseFirstRowStyle") ,
    OUStringLiteral("UseLastRowStyle") ,
    OUStringLiteral("UseBandingRowStyle") ,
    OUStringLiteral("UseFirstColumnStyle") ,
    OUStringLiteral("UseLastColumnStyle") ,
    OUStringLiteral("UseBandingColumnStyle")
};

TableDesignWidget::TableDesignWidget( VclBuilderContainer* pParent, ViewShellBase& rBase, bool bModal )
    : mrBase(rBase)
    , mbModal(bModal)
    , mbStyleSelected(false)
    , mbOptionsChanged(false)
{
    pParent->get(m_pValueSet, "previews");
    m_pValueSet->SetStyle(m_pValueSet->GetStyle() | WB_NO_DIRECTSELECT | WB_FLATVALUESET | WB_ITEMBORDER);
    m_pValueSet->SetExtraSpacing(8);
    m_pValueSet->setModal(mbModal);
    if( !mbModal )
    {
        m_pValueSet->SetColor();
    }
    else
    {
        m_pValueSet->SetColor( Color( COL_WHITE ) );
        m_pValueSet->SetBackground( Color( COL_WHITE ) );
    }
    m_pValueSet->SetSelectHdl (LINK(this, TableDesignWidget, implValueSetHdl));

    for (sal_uInt16 i = CB_HEADER_ROW; i <= CB_BANDED_COLUMNS; ++i)
    {
        pParent->get(m_aCheckBoxes[i], OUStringToOString(gPropNames[i], RTL_TEXTENCODING_UTF8));
        m_aCheckBoxes[i]->SetClickHdl( LINK( this, TableDesignWidget, implCheckBoxHdl ) );
    }

    // get current controller and initialize listeners
    try
    {
        mxView.set(mrBase.GetController(), UNO_QUERY);
        addListener();

        Reference< XController > xController( mrBase.GetController(), UNO_QUERY_THROW );
        Reference< XStyleFamiliesSupplier > xFamiliesSupp( xController->getModel(), UNO_QUERY_THROW );
        Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
        const OUString sFamilyName( "table" );
        mxTableFamily.set( xFamilies->getByName( sFamilyName ), UNO_QUERY_THROW );
    }
    catch (const Exception&)
    {
        OSL_FAIL( "sd::CustomAnimationPane::CustomAnimationPane(), Exception caught!" );
    }

    onSelectionChanged();
    updateControls();
}

TableDesignWidget::~TableDesignWidget()
{
    removeListener();
}

static SfxBindings* getBindings( ViewShellBase& rBase )
{
    if( rBase.GetMainViewShell().get() && rBase.GetMainViewShell()->GetViewFrame() )
        return &rBase.GetMainViewShell()->GetViewFrame()->GetBindings();
    else
        return nullptr;
}

static SfxDispatcher* getDispatcher( ViewShellBase& rBase )
{
    if( rBase.GetMainViewShell().get() && rBase.GetMainViewShell()->GetViewFrame() )
        return rBase.GetMainViewShell()->GetViewFrame()->GetDispatcher();
    else
        return nullptr;
}

IMPL_LINK_NOARG_TYPED(TableDesignWidget, implValueSetHdl, ValueSet*, void)
{
    mbStyleSelected = true;
    if( !mbModal )
        ApplyStyle();
}

void TableDesignWidget::ApplyStyle()
{
    try
    {
        OUString sStyleName;
        sal_Int32 nIndex = static_cast< sal_Int32 >( m_pValueSet->GetSelectItemId() ) - 1;

        if( (nIndex >= 0) && (nIndex < mxTableFamily->getCount()) )
        {
            Reference< XNameAccess > xNames( mxTableFamily, UNO_QUERY_THROW );
            sStyleName = xNames->getElementNames()[nIndex];
        }

        if( sStyleName.isEmpty() )
            return;

        SdrView* pView = mrBase.GetDrawView();
        if( mxSelectedTable.is() )
        {
            if( pView )
            {
                SfxRequest aReq( SID_TABLE_STYLE, SfxCallMode::SYNCHRON, SfxGetpApp()->GetPool() );
                aReq.AppendItem( SfxStringItem( SID_TABLE_STYLE, sStyleName ) );

                rtl::Reference< sdr::SelectionController > xController( pView->getSelectionController() );
                if( xController.is() )
                    xController->Execute( aReq );

                SfxBindings* pBindings = getBindings( mrBase );
                if( pBindings )
                {
                    pBindings->Invalidate( SID_UNDO );
                    pBindings->Invalidate( SID_REDO );
                }
            }
        }
        else
        {
            SfxDispatcher* pDispatcher = getDispatcher( mrBase );
            SfxStringItem aArg( SID_TABLE_STYLE, sStyleName );
            pDispatcher->ExecuteList(SID_INSERT_TABLE, SfxCallMode::ASYNCHRON,
                    { &aArg });
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("TableDesignWidget::implValueSetHdl(), exception caught!");
    }
}

IMPL_LINK_NOARG_TYPED(TableDesignWidget, implCheckBoxHdl, Button*, void)
{
    mbOptionsChanged = true;

    if( !mbModal )
        ApplyOptions();

    FillDesignPreviewControl();
}

void TableDesignWidget::ApplyOptions()
{
    static const sal_uInt16 gParamIds[CB_COUNT] =
    {
        ID_VAL_USEFIRSTROWSTYLE, ID_VAL_USELASTROWSTYLE, ID_VAL_USEBANDINGROWSTYLE,
        ID_VAL_USEFIRSTCOLUMNSTYLE, ID_VAL_USELASTCOLUMNSTYLE, ID_VAL_USEBANDINGCOLUMNSTYLE
    };

    if( mxSelectedTable.is() )
    {
        SfxRequest aReq( SID_TABLE_STYLE_SETTINGS, SfxCallMode::SYNCHRON, SfxGetpApp()->GetPool() );

        for( sal_uInt16 i = CB_HEADER_ROW; i <= CB_BANDED_COLUMNS; ++i )
        {
            aReq.AppendItem( SfxBoolItem( gParamIds[i], m_aCheckBoxes[i]->IsChecked() ) );
        }

        SdrView* pView = mrBase.GetDrawView();
        if( pView )
        {
            rtl::Reference< sdr::SelectionController > xController( pView->getSelectionController() );
            if( xController.is() )
            {
                xController->Execute( aReq );

                SfxBindings* pBindings = getBindings( mrBase );
                if( pBindings )
                {
                    pBindings->Invalidate( SID_UNDO );
                    pBindings->Invalidate( SID_REDO );
                }
            }
        }
    }
}

void TableDesignWidget::onSelectionChanged()
{
    Reference< XPropertySet > xNewSelection;

    if( mxView.is() ) try
    {
        Reference< XSelectionSupplier >  xSel( mxView, UNO_QUERY_THROW );
        if (xSel.is())
        {
            Any aSel( xSel->getSelection() );
            Sequence< XShape > xShapeSeq;
            if( aSel >>= xShapeSeq )
            {
                if( xShapeSeq.getLength() == 1 )
                    aSel <<= xShapeSeq[0];
            }
            else
            {
                Reference< XShapes > xShapes( aSel, UNO_QUERY );
                if( xShapes.is() && (xShapes->getCount() == 1) )
                    aSel <<= xShapes->getByIndex(0);
            }

            Reference< XShapeDescriptor > xDesc( aSel, UNO_QUERY );
            if( xDesc.is() && ( xDesc->getShapeType() == "com.sun.star.drawing.TableShape" || xDesc->getShapeType() == "com.sun.star.presentation.TableShape" ) )
            {
                xNewSelection.set( xDesc, UNO_QUERY );
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::TableDesignWidget::onSelectionChanged(), Exception caught!" );
    }

    if( mxSelectedTable != xNewSelection )
    {
        mxSelectedTable = xNewSelection;
        updateControls();
    }
}

void TableValueSet::Resize()
{
    ValueSet::Resize();
    // Calculate the number of rows and columns.
    if( GetItemCount() > 0 )
    {
        Size aValueSetSize = GetSizePixel();

        Image aImage = GetItemImage(GetItemId(0));
        Size aItemSize = aImage.GetSizePixel();

        aItemSize.Height() += 10;
        int nColumnCount = (aValueSetSize.Width() - GetScrollWidth()) / aItemSize.Width();
        if (nColumnCount < 1)
            nColumnCount = 1;

        int nRowCount = (GetItemCount() + nColumnCount - 1) / nColumnCount;
        if (nRowCount < 1)
            nRowCount = 1;

        int nVisibleRowCount = (aValueSetSize.Height()+2) / aItemSize.Height();

        SetColCount ((sal_uInt16)nColumnCount);
        SetLineCount ((sal_uInt16)nRowCount);

        if( !m_bModal )
        {
            WinBits nStyle = GetStyle() & ~(WB_VSCROLL);
            if( nRowCount > nVisibleRowCount )
            {
                nStyle |= WB_VSCROLL;
            }
            SetStyle( nStyle );
        }
    }
}

TableValueSet::TableValueSet(Window *pParent, WinBits nStyle)
    : ValueSet(pParent, nStyle)
    , m_bModal(false)
{
}

void TableValueSet::DataChanged( const DataChangedEvent& /*rDCEvt*/ )
{
    updateSettings();
}

void TableValueSet::updateSettings()
{
    if( !m_bModal )
    {
        SetBackground( GetSettings().GetStyleSettings().GetWindowColor() );
        SetColor( GetSettings().GetStyleSettings().GetWindowColor() );
        SetExtraSpacing(8);
    }
}

VCL_BUILDER_DECL_FACTORY(TableValueSet)
{
    WinBits nWinStyle = WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    rRet = VclPtr<TableValueSet>::Create(pParent, nWinStyle);
}

void TableDesignWidget::updateControls()
{
    static const bool gDefaults[CB_COUNT] = { true, false, true, false, false, false };

    const bool bHasTable = mxSelectedTable.is();

    for (sal_uInt16 i = CB_HEADER_ROW; i <= CB_BANDED_COLUMNS; ++i)
    {
        bool bUse = gDefaults[i];
        if( bHasTable ) try
        {
            mxSelectedTable->getPropertyValue( gPropNames[i] ) >>= bUse;
        }
        catch( Exception& )
        {
            OSL_FAIL("sd::TableDesignWidget::updateControls(), exception caught!");
        }
        m_aCheckBoxes[i]->Check(bUse);
        m_aCheckBoxes[i]->Enable(bHasTable);
    }

    FillDesignPreviewControl();
    m_pValueSet->updateSettings();
    m_pValueSet->Resize();

    sal_uInt16 nSelection = 0;
    if( mxSelectedTable.is() )
    {
        Reference< XNamed > xNamed( mxSelectedTable->getPropertyValue( "TableTemplate" ), UNO_QUERY );
        if( xNamed.is() )
        {
            const OUString sStyleName( xNamed->getName() );

            Reference< XNameAccess > xNames( mxTableFamily, UNO_QUERY );
            if( xNames.is() )
            {
                Sequence< OUString > aNames( xNames->getElementNames() );
                for( sal_Int32 nIndex = 0; nIndex < aNames.getLength(); nIndex++ )
                {
                    if( aNames[nIndex] == sStyleName )
                    {
                        nSelection = (sal_uInt16)nIndex+1;
                        break;
                    }
                }
            }
        }
    }
    m_pValueSet->SelectItem( nSelection );
}

void TableDesignWidget::addListener()
{
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this,TableDesignWidget,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->AddEventListener (
        aLink,
        tools::EventMultiplexerEvent::EID_EDIT_VIEW_SELECTION
        | tools::EventMultiplexerEvent::EID_CURRENT_PAGE
        | tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED
        | tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED
        | tools::EventMultiplexerEvent::EID_DISPOSING);
}

void TableDesignWidget::removeListener()
{
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this,TableDesignWidget,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->RemoveEventListener( aLink );
}

IMPL_LINK_TYPED(TableDesignWidget,EventMultiplexerListener,
    tools::EventMultiplexerEvent&, rEvent, void)
{
    switch (rEvent.meEventId)
    {
        case tools::EventMultiplexerEvent::EID_CURRENT_PAGE:
        case tools::EventMultiplexerEvent::EID_EDIT_VIEW_SELECTION:
            onSelectionChanged();
            break;

        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED:
            mxView.clear();
            onSelectionChanged();
            break;

        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED:
            mxView.set( mrBase.GetController(), UNO_QUERY );
            onSelectionChanged();
            break;
    }
}

struct CellInfo
{
    Color maCellColor;
    Color maTextColor;
    SvxBoxItem maBorder;

    explicit CellInfo( const Reference< XStyle >& xStyle );
};

CellInfo::CellInfo( const Reference< XStyle >& xStyle )
: maBorder(SDRATTR_TABLE_BORDER)
{
    SfxStyleSheet* pStyleSheet = SfxUnoStyleSheet::getUnoStyleSheet( xStyle );
    if( pStyleSheet )
    {
        SfxItemSet& rSet = pStyleSheet->GetItemSet();

        // get style fill color
        if( !GetDraftFillColor(rSet, maCellColor) )
            maCellColor.SetColor( COL_TRANSPARENT );

        // get style text color
        const SvxColorItem* pTextColor = dynamic_cast<const SvxColorItem*>( rSet.GetItem(EE_CHAR_COLOR) );
        if( pTextColor )
            maTextColor = pTextColor->GetValue();
        else
            maTextColor.SetColor( COL_TRANSPARENT );

        // get border
        const SvxBoxItem* pBoxItem = dynamic_cast<const SvxBoxItem*>(rSet.GetItem( SDRATTR_TABLE_BORDER ) );
        if( pBoxItem )
            maBorder = *pBoxItem;
    }
}

typedef std::vector< std::shared_ptr< CellInfo > > CellInfoVector;
typedef std::shared_ptr< CellInfo > CellInfoMatrix[nPreviewColumns][nPreviewRows];

struct TableStyleSettings
{
    bool mbUseFirstRow;
    bool mbUseLastRow;
    bool mbUseFirstColumn;
    bool mbUseLastColumn;
    bool mbUseRowBanding;
    bool mbUseColumnBanding;

    TableStyleSettings()
        : mbUseFirstRow(true)
        , mbUseLastRow(false)
        , mbUseFirstColumn(false)
        , mbUseLastColumn(false)
        , mbUseRowBanding(true)
        , mbUseColumnBanding(false) {}
};

static void FillCellInfoVector( const Reference< XIndexAccess >& xTableStyle, CellInfoVector& rVector )
{
    DBG_ASSERT( xTableStyle.is() && (xTableStyle->getCount() == sdr::table::style_count ), "sd::FillCellInfoVector(), invalid table style!" );
    if( xTableStyle.is() ) try
    {
        rVector.resize( sdr::table::style_count );

        for( sal_Int32 nStyle = 0; nStyle < sdr::table::style_count; ++nStyle )
        {
            Reference< XStyle > xStyle( xTableStyle->getByIndex( nStyle ), UNO_QUERY );
            if( xStyle.is() )
                rVector[nStyle].reset( new CellInfo( xStyle ) );
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("sd::FillCellInfoVector(), exception caught!");
    }
}

static void FillCellInfoMatrix( const CellInfoVector& rStyle, const TableStyleSettings& rSettings, CellInfoMatrix& rMatrix )
{
    for( sal_Int32 nRow = 0; nRow < nPreviewColumns; ++nRow )
    {
        const bool bFirstRow = rSettings.mbUseFirstRow && (nRow == 0);
        const bool bLastRow = rSettings.mbUseLastRow && (nRow == nPreviewColumns - 1);

        for( sal_Int32 nCol = 0; nCol < nPreviewColumns; ++nCol )
        {
            std::shared_ptr< CellInfo > xCellInfo;

            // first and last row win first, if used and available
            if( bFirstRow )
            {
                xCellInfo = rStyle[sdr::table::first_row_style];
            }
            else if( bLastRow )
            {
                xCellInfo = rStyle[sdr::table::last_row_style];
            }

            if( !xCellInfo.get() )
            {
                // next come first and last column, if used and available
                if( rSettings.mbUseFirstColumn && (nCol == 0)  )
                {
                    xCellInfo = rStyle[sdr::table::first_column_style];
                }
                else if( rSettings.mbUseLastColumn && (nCol == nPreviewColumns-1) )
                {
                    xCellInfo = rStyle[sdr::table::last_column_style];
                }
            }

            if( !xCellInfo.get() )
            {
                if( rSettings.mbUseRowBanding )
                {
                    if( (nRow & 1) == 0 )
                    {
                        xCellInfo = rStyle[sdr::table::even_rows_style];
                    }
                    else
                    {
                        xCellInfo = rStyle[sdr::table::odd_rows_style];
                    }
                }
            }

            if( !xCellInfo.get() )
            {
                if( rSettings.mbUseColumnBanding )
                {
                    if( (nCol & 1) == 0 )
                    {
                        xCellInfo = rStyle[sdr::table::even_columns_style];
                    }
                    else
                    {
                        xCellInfo = rStyle[sdr::table::odd_columns_style];
                    }
                }
            }

            if( !xCellInfo.get() )
            {
                // use default cell style if non found yet
                xCellInfo = rStyle[sdr::table::body_style];
            }

            rMatrix[nCol][nRow] = xCellInfo;
        }
    }
}

const Bitmap CreateDesignPreview( const Reference< XIndexAccess >& xTableStyle, const TableStyleSettings& rSettings, bool bIsPageDark )
{
    CellInfoVector aCellInfoVector(sdr::table::style_count);
    FillCellInfoVector( xTableStyle, aCellInfoVector );

    CellInfoMatrix aMatrix;
    FillCellInfoMatrix( aCellInfoVector, rSettings, aMatrix );

// bbbbbbbbbbbb w = 12 pixel
// bccccccccccb h = 7 pixel
// bccccccccccb b = border color
// bcttttttttcb c = cell color
// bccccccccccb t = text color
// bccccccccccb
// bbbbbbbbbbbb

    Bitmap aPreviewBmp( Size( nBitmapWidth, nBitmapHeight), 24, nullptr );
    BitmapWriteAccess* pAccess = aPreviewBmp.AcquireWriteAccess();
    if( pAccess )
    {
        pAccess->Erase( Color( bIsPageDark ? COL_BLACK : COL_WHITE ) );

        // first draw cell background and text line previews
        sal_Int32 nY = 0;
        sal_Int32 nRow;
        for( nRow = 0; nRow < nPreviewRows; ++nRow, nY += nCellHeight-1 )
        {
            sal_Int32 nX = 0;
            for( sal_Int32 nCol = 0; nCol < nPreviewColumns; ++nCol, nX += nCellWidth-1 )
            {
                std::shared_ptr< CellInfo > xCellInfo( aMatrix[nCol][nRow] );

                Color aTextColor( COL_AUTO );
                if( xCellInfo.get() )
                {
                    // fill cell background
                    const Rectangle aRect( nX, nY, nX + nCellWidth - 1, nY + nCellHeight - 1 );

                    if( xCellInfo->maCellColor.GetColor() != COL_TRANSPARENT )
                    {
                        pAccess->SetFillColor( xCellInfo->maCellColor );
                        pAccess->FillRect( aRect );
                    }

                    aTextColor = xCellInfo->maTextColor;
                }

                // draw text preview line
                if( aTextColor.GetColor() == COL_AUTO )
                    aTextColor.SetColor( bIsPageDark ? COL_WHITE : COL_BLACK );
                pAccess->SetLineColor( aTextColor );
                const Point aPnt1( nX + 2, nY + ((nCellHeight - 1 ) >> 1) );
                const Point aPnt2( nX + nCellWidth - 3, aPnt1.Y() );
                pAccess->DrawLine( aPnt1, aPnt2 );
            }
        }

        // second draw border lines
        nY = 0;
        for( nRow = 0; nRow < nPreviewRows; ++nRow, nY += nCellHeight-1 )
        {
            sal_Int32 nX = 0;
            for( sal_Int32 nCol = 0; nCol < nPreviewColumns; ++nCol, nX += nCellWidth-1 )
            {
                std::shared_ptr< CellInfo > xCellInfo( aMatrix[nCol][nRow] );

                if( xCellInfo.get() )
                {
                    const Point aPntTL( nX, nY );
                    const Point aPntTR( nX + nCellWidth - 1, nY );
                    const Point aPntBL( nX, nY + nCellHeight - 1 );
                    const Point aPntBR( nX + nCellWidth - 1, nY + nCellHeight - 1 );

                    sal_Int32 border_diffs[8] = { 0,-1, 0,1, -1,0, 1,0 };
                    sal_Int32* pDiff = &border_diffs[0];

                    // draw top border
                    for( SvxBoxItemLine nLine : o3tl::enumrange<SvxBoxItemLine>() )
                    {
                        const ::editeng::SvxBorderLine* pBorderLine = xCellInfo->maBorder.GetLine(nLine);
                        if( !pBorderLine || ((pBorderLine->GetOutWidth() == 0) && (pBorderLine->GetInWidth()==0)) )
                            continue;

                        sal_Int32 nBorderCol = nCol + *pDiff++;
                        sal_Int32 nBorderRow = nRow + *pDiff++;
                        if( (nBorderCol >= 0) && (nBorderCol < nPreviewColumns) && (nBorderRow >= 0) && (nBorderRow < nPreviewRows) )
                        {
                            // check border
                            std::shared_ptr< CellInfo > xBorderInfo( aMatrix[nBorderCol][nBorderRow] );
                            if( xBorderInfo.get() )
                            {
                                const ::editeng::SvxBorderLine* pBorderLine2 = xBorderInfo->maBorder.GetLine(static_cast<SvxBoxItemLine>(static_cast<int>(nLine)^1));
                                if( pBorderLine2 && pBorderLine2->HasPriority(*pBorderLine) )
                                    continue; // other border line wins
                            }
                        }

                        pAccess->SetLineColor( pBorderLine->GetColor() );
                        switch( nLine )
                        {
                        case SvxBoxItemLine::TOP: pAccess->DrawLine( aPntTL, aPntTR ); break;
                        case SvxBoxItemLine::BOTTOM: pAccess->DrawLine( aPntBL, aPntBR ); break;
                        case SvxBoxItemLine::LEFT: pAccess->DrawLine( aPntTL, aPntBL ); break;
                        case SvxBoxItemLine::RIGHT: pAccess->DrawLine( aPntTR, aPntBR ); break;
                        }
                    }
                }
            }
        }

        Bitmap::ReleaseAccess( pAccess );
    }

    return aPreviewBmp;
}

void TableDesignWidget::FillDesignPreviewControl()
{
    sal_uInt16 nSelectedItem = m_pValueSet->GetSelectItemId();
    m_pValueSet->Clear();
    try
    {
        TableStyleSettings aSettings;
        if( mxSelectedTable.is() )
        {
            aSettings.mbUseFirstRow = m_aCheckBoxes[CB_HEADER_ROW]->IsChecked();
            aSettings.mbUseLastRow = m_aCheckBoxes[CB_TOTAL_ROW]->IsChecked();
            aSettings.mbUseRowBanding = m_aCheckBoxes[CB_BANDED_ROWS]->IsChecked();
            aSettings.mbUseFirstColumn = m_aCheckBoxes[CB_FIRST_COLUMN]->IsChecked();
            aSettings.mbUseLastColumn = m_aCheckBoxes[CB_LAST_COLUMN]->IsChecked();
            aSettings.mbUseColumnBanding = m_aCheckBoxes[CB_BANDED_COLUMNS]->IsChecked();
        }

        bool bIsPageDark = false;
        if( mxView.is() )
        {
            Reference< XPropertySet > xPageSet( mxView->getCurrentPage(), UNO_QUERY );
            if( xPageSet.is() )
            {
                const OUString sIsBackgroundDark( "IsBackgroundDark" );
                xPageSet->getPropertyValue(sIsBackgroundDark) >>= bIsPageDark;
            }
        }

        sal_Int32 nCount = mxTableFamily->getCount();
        for( sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex ) try
        {
            Reference< XIndexAccess > xTableStyle( mxTableFamily->getByIndex( nIndex ), UNO_QUERY );
            if( xTableStyle.is() )
                m_pValueSet->InsertItem( sal::static_int_cast<sal_uInt16>( nIndex + 1 ), Image( CreateDesignPreview( xTableStyle, aSettings, bIsPageDark ) ) );
        }
        catch( Exception& )
        {
            OSL_FAIL("sd::TableDesignWidget::FillDesignPreviewControl(), exception caught!");
        }
        sal_Int32 nCols = 3;
        sal_Int32 nRows = (nCount+2)/3;
        m_pValueSet->SetColCount(nCols);
        m_pValueSet->SetLineCount(nRows);
        WinBits nStyle = m_pValueSet->GetStyle() & ~(WB_VSCROLL);
        m_pValueSet->SetStyle(nStyle);
        Size aSize(m_pValueSet->GetOptimalSize());
        aSize.Width() += (10 * nCols);
        aSize.Height() += (10 * nRows);
        m_pValueSet->set_width_request(aSize.Width());
        m_pValueSet->set_height_request(aSize.Height());
        m_pValueSet->Resize();
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::TableDesignWidget::FillDesignPreviewControl(), exception caught!");
    }
    m_pValueSet->SelectItem(nSelectedItem);
}

short TableDesignDialog::Execute()
{
    if( ModalDialog::Execute() )
    {
        if( aImpl.isStyleChanged() )
            aImpl.ApplyStyle();

        if( aImpl.isOptionsChanged() )
            aImpl.ApplyOptions();
        return RET_OK;
    }
    return RET_CANCEL;
}

VclPtr<vcl::Window> createTableDesignPanel( vcl::Window* pParent, ViewShellBase& rBase )
{
    VclPtr<TableDesignPane> pRet = nullptr;
    try
    {
        pRet = VclPtr<TableDesignPane>::Create( pParent, rBase );
    }
    catch (const uno::Exception&)
    {
    }
    return pRet;
}

void showTableDesignDialog( vcl::Window* pParent, ViewShellBase& rBase )
{
    ScopedVclPtrInstance< TableDesignDialog > xDialog( pParent, rBase );
    xDialog->Execute();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
