/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "sddll.hxx"

#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>

#include <comphelper/processfactory.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/bmpacc.hxx>
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

#include "TableDesignPane.hxx"
#include <svtools/valueset.hxx>

#include "DrawDocShell.hxx"
#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"
#include "DrawController.hxx"
#include "glob.hrc"
#include "sdresid.hxx"
#include "EventMultiplexer.hxx"

using ::rtl::OUString;
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

// --------------------------------------------------------------------

static const OUString* getPropertyNames()
{
    static const OUString gPropNames[ CB_BANDED_COLUMNS-CB_HEADER_ROW+1 ] =
    {
        OUString("UseFirstRowStyle") ,
        OUString("UseLastRowStyle") ,
        OUString("UseBandingRowStyle") ,
        OUString("UseFirstColumnStyle") ,
        OUString("UseLastColumnStyle") ,
        OUString("UseBandingColumnStyle")
    };
    return &gPropNames[0];
}
// --------------------------------------------------------------------

TableDesignPane::TableDesignPane( ::Window* pParent, ViewShellBase& rBase, bool bModal )
: Control( pParent, SdResId(DLG_TABLEDESIGNPANE) )
, mrBase( rBase )
, msTableTemplate( "TableTemplate" )
, mbModal( bModal )
, mbStyleSelected( false )
, mbOptionsChanged( false )
{
    Window* pControlParent = mbModal ? pParent : this;

    mxControls[FL_TABLE_STYLES].reset( new FixedLine( pControlParent, SdResId( FL_TABLE_STYLES + 1 ) ) );

    ValueSet* pValueSet = new ValueSet( pControlParent, SdResId( CT_TABLE_STYLES+1 ) );
    mxControls[CT_TABLE_STYLES].reset( pValueSet );
    if( !mbModal )
    {
        pValueSet->SetStyle( (pValueSet->GetStyle() & ~(WB_ITEMBORDER|WB_BORDER)) | WB_NO_DIRECTSELECT | WB_FLATVALUESET | WB_NOBORDER );
        pValueSet->SetColor();
        pValueSet->SetExtraSpacing(8);
    }
    else
    {
        pValueSet->SetColor( Color( COL_WHITE ) );
        pValueSet->SetBackground( Color( COL_WHITE ) );
    }
    pValueSet->SetSelectHdl (LINK(this, TableDesignPane, implValueSetHdl));

    mxControls[FL_STYLE_OPTIONS].reset( new FixedLine( pControlParent, SdResId( FL_STYLE_OPTIONS + 1 ) ) );
    sal_uInt16 i;
    for( i = CB_HEADER_ROW; i <= CB_BANDED_COLUMNS; ++i )
    {
        CheckBox *pCheckBox = new CheckBox( pControlParent, SdResId( i+1 ) );
        mxControls[i].reset( pCheckBox );
        pCheckBox->SetClickHdl( LINK( this, TableDesignPane, implCheckBoxHdl ) );
    }

    for( i = 0; i < DESIGNPANE_CONTROL_COUNT; i++ )
        mnOrgOffsetY[i] = mxControls[i]->GetPosPixel().Y();

    // get current controller and initialize listeners
    try
    {
        mxView = Reference< XDrawView >::query(mrBase.GetController());
        addListener();

        Reference< XController > xController( mrBase.GetController(), UNO_QUERY_THROW );
        Reference< XStyleFamiliesSupplier > xFamiliesSupp( xController->getModel(), UNO_QUERY_THROW );
        Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
        const OUString sFamilyName( "table" );
        mxTableFamily = Reference< XIndexAccess >( xFamilies->getByName( sFamilyName ), UNO_QUERY_THROW );

    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::CustomAnimationPane::CustomAnimationPane(), Exception caught!" );
    }

    onSelectionChanged();
    updateControls();

    FreeResource();
}

// --------------------------------------------------------------------

TableDesignPane::~TableDesignPane()
{
    removeListener();
}

// --------------------------------------------------------------------

void TableDesignPane::DataChanged( const DataChangedEvent& /*rDCEvt*/ )
{
    updateLayout();
}

// --------------------------------------------------------------------

void TableDesignPane::Resize()
{
    updateLayout();
}

// --------------------------------------------------------------------

static SfxBindings* getBindings( ViewShellBase& rBase )
{
    if( rBase.GetMainViewShell().get() && rBase.GetMainViewShell()->GetViewFrame() )
        return &rBase.GetMainViewShell()->GetViewFrame()->GetBindings();
    else
        return 0;
}

// --------------------------------------------------------------------

static SfxDispatcher* getDispatcher( ViewShellBase& rBase )
{
    if( rBase.GetMainViewShell().get() && rBase.GetMainViewShell()->GetViewFrame() )
        return rBase.GetMainViewShell()->GetViewFrame()->GetDispatcher();
    else
        return 0;
}

// --------------------------------------------------------------------

IMPL_LINK_NOARG(TableDesignPane, implValueSetHdl)
{
    mbStyleSelected = true;
    if( !mbModal )
        ApplyStyle();
    return 0;
}

// --------------------------------------------------------------------

void TableDesignPane::ApplyStyle()
{
    try
    {
        OUString sStyleName;
        ValueSet* pValueSet = static_cast< ValueSet* >( mxControls[CT_TABLE_STYLES].get() );
        sal_Int32 nIndex = static_cast< sal_Int32 >( pValueSet->GetSelectItemId() ) - 1;

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
                SfxRequest aReq( SID_TABLE_STYLE, SFX_CALLMODE_SYNCHRON, SFX_APP()->GetPool() );
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
            pDispatcher->Execute(SID_INSERT_TABLE, SFX_CALLMODE_ASYNCHRON, &aArg, 0 );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("TableDesignPane::implValueSetHdl(), exception caught!");
    }
}

// --------------------------------------------------------------------

IMPL_LINK_NOARG(TableDesignPane, implCheckBoxHdl)
{
    mbOptionsChanged = true;

    if( !mbModal )
        ApplyOptions();

    FillDesignPreviewControl();
    return 0;
}

// --------------------------------------------------------------------

void TableDesignPane::ApplyOptions()
{
    static sal_uInt16 gParamIds[CB_BANDED_COLUMNS-CB_HEADER_ROW+1] =
    {
        ID_VAL_USEFIRSTROWSTYLE, ID_VAL_USELASTROWSTYLE, ID_VAL_USEBANDINGROWSTYLE,
        ID_VAL_USEFIRSTCOLUMNSTYLE, ID_VAL_USELASTCOLUMNSTYLE, ID_VAL_USEBANDINGCOLUMNSTYLE
    };

    if( mxSelectedTable.is() )
    {
        SfxRequest aReq( SID_TABLE_STYLE_SETTINGS, SFX_CALLMODE_SYNCHRON, SFX_APP()->GetPool() );

        for( sal_uInt16 i = 0; i < (CB_BANDED_COLUMNS-CB_HEADER_ROW+1); ++i )
        {
            aReq.AppendItem( SfxBoolItem( gParamIds[i], static_cast< CheckBox* >( mxControls[CB_HEADER_ROW+i].get() )->IsChecked() ) );
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

// --------------------------------------------------------------------

void TableDesignPane::onSelectionChanged()
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
                xNewSelection = Reference< XPropertySet >::query( xDesc );
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::TableDesignPane::onSelectionChanged(), Exception caught!" );
    }

    if( mxSelectedTable != xNewSelection )
    {
        mxSelectedTable = xNewSelection;
        updateControls();
    }
}

// --------------------------------------------------------------------

void TableDesignPane::updateLayout()
{
    ::Size aPaneSize( GetSizePixel() );
    if(IsVisible() && aPaneSize.Width() > 0)
    {
        Point aOffset( LogicToPixel( Point(3,3), MAP_APPFONT ) );

        ValueSet* pValueSet = static_cast< ValueSet* >( mxControls[CT_TABLE_STYLES].get() );

        Size aValueSetSize;

        if( !mbModal )
        {
            const long nOptionsHeight = mnOrgOffsetY[CB_BANDED_COLUMNS] + mxControls[CB_BANDED_COLUMNS]->GetSizePixel().Height() + aOffset.Y();

            const long nStylesHeight = aPaneSize.Height() - nOptionsHeight;

            // set with of controls to size of pane
            for( sal_Int32 nId = 0; nId < DESIGNPANE_CONTROL_COUNT; ++nId )
            {
                Size aSize( mxControls[nId]->GetSizePixel() );
                aSize.Width() = aPaneSize.Width() - aOffset.X() - mxControls[nId]->GetPosPixel().X();
                mxControls[nId]->SetSizePixel( aSize );
                mxControls[nId]->SetPaintTransparent(sal_True);
                mxControls[nId]->SetBackground();
            }
            aValueSetSize = Size( pValueSet->GetSizePixel().Width(), nStylesHeight - mxControls[FL_TABLE_STYLES]->GetSizePixel().Height() - mnOrgOffsetY[FL_TABLE_STYLES]  );
        }
        else
        {
            aValueSetSize = pValueSet->GetSizePixel();
        }


        // Calculate the number of rows and columns.
        if( pValueSet->GetItemCount() > 0 )
        {
            Image aImage = pValueSet->GetItemImage(pValueSet->GetItemId(0));
            Size aItemSize = pValueSet->CalcItemSizePixel(aImage.GetSizePixel());
            pValueSet->SetItemWidth( aItemSize.Width() );
            pValueSet->SetItemHeight( aItemSize.Height() );

            aItemSize.Width() += 10;
            aItemSize.Height() += 10;
            int nColumnCount = (aValueSetSize.Width() - pValueSet->GetScrollWidth()) / aItemSize.Width();
            if (nColumnCount < 1)
                nColumnCount = 1;

            int nRowCount = (pValueSet->GetItemCount() + nColumnCount - 1) / nColumnCount;
            if (nRowCount < 1)
                nRowCount = 1;

            int nVisibleRowCount = (aValueSetSize.Height()+2) / aItemSize.Height();

            pValueSet->SetLineCount ( (nRowCount < nVisibleRowCount) ? (sal_uInt16)nRowCount : 0 );

            pValueSet->SetColCount ((sal_uInt16)nColumnCount);
            pValueSet->SetLineCount ((sal_uInt16)nRowCount);

            if( !mbModal )
            {
                WinBits nStyle = pValueSet->GetStyle() & ~(WB_VSCROLL);
                if( nRowCount < nVisibleRowCount )
                {
                    aValueSetSize.Height() = nRowCount * aItemSize.Height();
                }
                else if( nRowCount > nVisibleRowCount )
                {
                    nStyle |= WB_VSCROLL;
                }
                pValueSet->SetStyle( nStyle );
            }
       }

        if( !mbModal )
        {
            pValueSet->SetSizePixel( aValueSetSize );
            pValueSet->SetBackground( GetSettings().GetStyleSettings().GetWindowColor() );
            pValueSet->SetColor( GetSettings().GetStyleSettings().GetWindowColor() );

            Point aPos( pValueSet->GetPosPixel() );

            // The following line may look like a no-op but without it the
            // control is placed off-screen when RTL is active.
            pValueSet->SetPosPixel(pValueSet->GetPosPixel());

            // shift show options section down
            const long nOptionsPos = aPos.Y() + aValueSetSize.Height();
            for( sal_Int32 nId = FL_STYLE_OPTIONS; nId <= CB_BANDED_COLUMNS; ++nId )
            {
                Point aCPos( mxControls[nId]->GetPosPixel() );
                aCPos.X() = ( nId == FL_STYLE_OPTIONS ?  1 : 2 ) * aOffset.X();
                aCPos.Y() = mnOrgOffsetY[nId] + nOptionsPos;
                mxControls[nId]->SetPosPixel( aCPos );
            }
        }
    }

    if( !mbModal )
        SetBackground( GetSettings().GetStyleSettings().GetWindowColor() );
}

// --------------------------------------------------------------------

void TableDesignPane::updateControls()
{
    static sal_Bool gDefaults[CB_BANDED_COLUMNS-CB_HEADER_ROW+1] = { sal_True, sal_False, sal_True, sal_False, sal_False, sal_False };

    const bool bHasTable = mxSelectedTable.is();
    const OUString* pPropNames = getPropertyNames();

    for( sal_uInt16 i = CB_HEADER_ROW; i <= CB_BANDED_COLUMNS; ++i )
    {
        sal_Bool bUse = gDefaults[i-CB_HEADER_ROW];
        if( bHasTable ) try
        {
            mxSelectedTable->getPropertyValue( *pPropNames++ ) >>= bUse;
        }
        catch( Exception& )
        {
            OSL_FAIL("sd::TableDesignPane::updateControls(), exception caught!");
        }
        static_cast< CheckBox* >( mxControls[i].get() )->Check( bUse ? sal_True : sal_False );
        mxControls[i]->Enable(bHasTable ? sal_True : sal_False );
    }

    FillDesignPreviewControl();
    updateLayout();


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
    ValueSet* pValueSet = static_cast< ValueSet* >( mxControls[CT_TABLE_STYLES].get() );
    pValueSet->SelectItem( nSelection );
}

// --------------------------------------------------------------------

void TableDesignPane::addListener()
{
    Link aLink( LINK(this,TableDesignPane,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->AddEventListener (
        aLink,
        tools::EventMultiplexerEvent::EID_EDIT_VIEW_SELECTION
        | tools::EventMultiplexerEvent::EID_CURRENT_PAGE
        | tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED
        | tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED
        | tools::EventMultiplexerEvent::EID_DISPOSING);
}

// --------------------------------------------------------------------

void TableDesignPane::removeListener()
{
    Link aLink( LINK(this,TableDesignPane,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->RemoveEventListener( aLink );
}

// --------------------------------------------------------------------

IMPL_LINK(TableDesignPane,EventMultiplexerListener,
    tools::EventMultiplexerEvent*,pEvent)
{
    switch (pEvent->meEventId)
    {
        case tools::EventMultiplexerEvent::EID_CURRENT_PAGE:
        case tools::EventMultiplexerEvent::EID_EDIT_VIEW_SELECTION:
            onSelectionChanged();
            break;

        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED:
            mxView = Reference<XDrawView>();
            onSelectionChanged();
            break;

        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED:
            mxView = Reference<XDrawView>::query( mrBase.GetController() );
            onSelectionChanged();
            break;
    }
    return 0;
}

// --------------------------------------------------------------------

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

// --------------------------------------------------------------------

typedef std::vector< boost::shared_ptr< CellInfo > > CellInfoVector;
typedef boost::shared_ptr< CellInfo > CellInfoMatrix[nPreviewColumns][nPreviewRows];

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

// --------------------------------------------------------------------

static void FillCellInfoVector( const Reference< XIndexAccess >& xTableStyle, CellInfoVector& rVector )
{
    DBG_ASSERT( xTableStyle.is() && (xTableStyle->getCount() == sdr::table::style_count ), "sd::FillCellInfoVector(), inavlid table style!" );
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
            boost::shared_ptr< CellInfo > xCellInfo;

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

// --------------------------------------------------------------------

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


    const sal_Int32 nCellWidth = 12; // one pixel is shared with the next cell!
    const sal_Int32 nCellHeight = 7; // one pixel is shared with the next cell!

    Bitmap aPreviewBmp( Size( (nCellWidth * nPreviewColumns) - (nPreviewColumns - 1), (nCellHeight * nPreviewRows) - (nPreviewRows - 1)), 24, NULL );
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
                boost::shared_ptr< CellInfo > xCellInfo( aMatrix[nCol][nRow] );

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
                boost::shared_ptr< CellInfo > xCellInfo( aMatrix[nCol][nRow] );

                if( xCellInfo.get() )
                {
                    const Point aPntTL( nX, nY );
                    const Point aPntTR( nX + nCellWidth - 1, nY );
                    const Point aPntBL( nX, nY + nCellHeight - 1 );
                    const Point aPntBR( nX + nCellWidth - 1, nY + nCellHeight - 1 );

                    sal_Int32 border_diffs[8] = { 0,-1, 0,1, -1,0, 1,0 };
                    sal_Int32* pDiff = &border_diffs[0];

                    // draw top border
                    for( sal_uInt16 nLine = 0; nLine < 4; ++nLine )
                    {
                        const ::editeng::SvxBorderLine* pBorderLine = xCellInfo->maBorder.GetLine(nLine);
                        if( !pBorderLine || ((pBorderLine->GetOutWidth() == 0) && (pBorderLine->GetInWidth()==0)) )
                            continue;

                        sal_Int32 nBorderCol = nCol + *pDiff++;
                        sal_Int32 nBorderRow = nRow + *pDiff++;
                        if( (nBorderCol >= 0) && (nBorderCol < nPreviewColumns) && (nBorderRow >= 0) && (nBorderRow < nPreviewRows) )
                        {
                            // check border
                            boost::shared_ptr< CellInfo > xBorderInfo( aMatrix[nBorderCol][nBorderRow] );
                            if( xBorderInfo.get() )
                            {
                                const sal_uInt16 nOtherLine = nLine ^ 1;
                                const ::editeng::SvxBorderLine* pBorderLine2 = xBorderInfo->maBorder.GetLine(nOtherLine^1);
                                if( pBorderLine2 && pBorderLine2->HasPriority(*pBorderLine) )
                                    continue; // other border line wins
                            }
                        }

                        pAccess->SetLineColor( pBorderLine->GetColor() );
                        switch( nLine )
                        {
                        case 0: pAccess->DrawLine( aPntTL, aPntTR ); break;
                        case 1: pAccess->DrawLine( aPntBL, aPntBR ); break;
                        case 2: pAccess->DrawLine( aPntTL, aPntBL ); break;
                        case 3: pAccess->DrawLine( aPntTR, aPntBR ); break;
                        }
                    }
                }
            }
        }

        aPreviewBmp.ReleaseAccess( pAccess );
    }

    return aPreviewBmp;
}

void TableDesignPane::FillDesignPreviewControl()
{
    ValueSet* pValueSet = static_cast< ValueSet* >( mxControls[CT_TABLE_STYLES].get() );

    sal_uInt16 nSelectedItem = pValueSet->GetSelectItemId();
    pValueSet->Clear();
    try
    {
        TableStyleSettings aSettings;
        if( mxSelectedTable.is() )
        {
            aSettings.mbUseFirstRow = static_cast< CheckBox* >(mxControls[CB_HEADER_ROW].get())->IsChecked();
            aSettings.mbUseLastRow = static_cast< CheckBox* >(mxControls[CB_TOTAL_ROW].get())->IsChecked();
            aSettings.mbUseRowBanding = static_cast< CheckBox* >(mxControls[CB_BANDED_ROWS].get())->IsChecked();
            aSettings.mbUseFirstColumn = static_cast< CheckBox* >(mxControls[CB_FIRST_COLUMN].get())->IsChecked();
            aSettings.mbUseLastColumn = static_cast< CheckBox* >(mxControls[CB_LAST_COLUMN].get())->IsChecked();
            aSettings.mbUseColumnBanding = static_cast< CheckBox* >(mxControls[CB_BANDED_COLUMNS].get())->IsChecked();
        }

        sal_Bool bIsPageDark = sal_False;
        if( mxView.is() )
        {
            Reference< XPropertySet > xPageSet( mxView->getCurrentPage(), UNO_QUERY );
            if( xPageSet.is() )
            {
                const OUString sIsBackgroundDark( "IsBackgroundDark" );
                xPageSet->getPropertyValue(sIsBackgroundDark) >>= bIsPageDark;
            }
        }

        for( sal_Int32 nIndex = 0; nIndex < mxTableFamily->getCount(); nIndex++ ) try
        {
            Reference< XIndexAccess > xTableStyle( mxTableFamily->getByIndex( nIndex ), UNO_QUERY );
            if( xTableStyle.is() )
                pValueSet->InsertItem( sal::static_int_cast<sal_uInt16>( nIndex + 1 ), Image( CreateDesignPreview( xTableStyle, aSettings, bIsPageDark ) ) );
        }
        catch( Exception& )
        {
            OSL_FAIL("sd::TableDesignPane::FillDesignPreviewControl(), exception caught!");
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::TableDesignPane::FillDesignPreviewControl(), exception caught!");
    }
    pValueSet->SelectItem(nSelectedItem);
}

// ====================================================================

TableDesignDialog::TableDesignDialog(::Window* pParent, ViewShellBase& rBase )
: ModalDialog( pParent, SdResId( DLG_TABLEDESIGNPANE ))
{
    mxFlSep1.reset( new FixedLine( this, SdResId( FL_SEP1 ) ) );
    mxFlSep2.reset( new FixedLine( this, SdResId( FL_SEP2 ) ) );
    mxHelpButton.reset( new HelpButton( this, SdResId( BTN_HELP ) ) );
    mxOkButton.reset( new OKButton( this, SdResId( BTN_OK ) ) );
    mxCancelButton.reset( new CancelButton( this, SdResId( BTN_CANCEL ) ) );
    FreeResource();

    mpDesignPane.reset( new TableDesignPane( this, rBase, true ) );
    mpDesignPane->Hide();
}

// --------------------------------------------------------------------

short TableDesignDialog::Execute()
{
    if( ModalDialog::Execute() )
    {
        if( mpDesignPane->isStyleChanged() )
            mpDesignPane->ApplyStyle();

        if( mpDesignPane->isOptionsChanged() )
            mpDesignPane->ApplyOptions();
        return sal_True;
    }
    return sal_False;
}

// ====================================================================

::Window * createTableDesignPanel( ::Window* pParent, ViewShellBase& rBase )
{
    return new TableDesignPane( pParent, rBase, false );
}

// ====================================================================

void showTableDesignDialog( ::Window* pParent, ViewShellBase& rBase )
{
    boost::scoped_ptr< TableDesignDialog > xDialog( new TableDesignDialog( pParent, rBase ) );
    xDialog->Execute();
}


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
