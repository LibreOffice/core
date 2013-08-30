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

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include <vcl/canvastools.hxx>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svl/style.hxx>
#include "editeng/editstat.hxx"
#include "editeng/outlobj.hxx"
#include "svx/svdview.hxx"
#include "svx/sdr/properties/textproperties.hxx"
#include "svx/svdotable.hxx"
#include "svx/svdhdl.hxx"
#include "viewcontactoftableobj.hxx"
#include "svx/svdoutl.hxx"
#include "svx/svddrag.hxx"
#include "svx/svdpagv.hxx"
#include "tablemodel.hxx"
#include "cell.hxx"
#include "svx/xflclit.hxx"
#include "tablelayouter.hxx"
#include "svx/svdetc.hxx"
#include "tablehandles.hxx"
#include "editeng/boxitem.hxx"
#include "svx/framelink.hxx"
#include "svx/sdr/table/tabledesign.hxx"
#include "svx/svdundo.hxx"
#include "svx/svdstr.hrc"
#include "svx/svdglob.hxx"
#include "editeng/writingmodeitem.hxx"
#include "editeng/frmdiritem.hxx"
#include "svx/xflhtit.hxx"
#include "svx/xflftrit.hxx"
#include "svx/xfltrit.hxx"

// -----------------------------------------------------------------------------

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::style::XStyle;
using ::com::sun::star::table::XTableRows;
using ::com::sun::star::table::XTableColumns;
using ::com::sun::star::table::XTable;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::util::XModifyBroadcaster;
using sdr::properties::TextProperties;
using sdr::properties::BaseProperties;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;

namespace sdr { namespace table {

class TableProperties : public TextProperties
{
protected:
    // create a new itemset
    SfxItemSet& CreateObjectSpecificItemSet(SfxItemPool& rPool);

public:
    // basic constructor
    TableProperties(SdrObject& rObj );

    // constructor for copying, but using new object
    TableProperties(const TableProperties& rProps, SdrObject& rObj );

    // destructor
    ~TableProperties();

    // Clone() operator, normally just calls the local copy constructor
    BaseProperties& Clone(SdrObject& rObj) const;

    virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem);
};

TableProperties::TableProperties(SdrObject& rObj)
: TextProperties(rObj)
{
}

TableProperties::TableProperties(const TableProperties& rProps, SdrObject& rObj)
: TextProperties(rProps, rObj)
{
}

TableProperties::~TableProperties()
{
}

BaseProperties& TableProperties::Clone(SdrObject& rObj) const
{
    return *(new TableProperties(*this, rObj));
}

void TableProperties::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
{
    if( nWhich == SDRATTR_TEXTDIRECTION )
        AttributeProperties::ItemChange( nWhich, pNewItem );
    else
        TextProperties::ItemChange( nWhich, pNewItem );
}

// create a new itemset
SfxItemSet& TableProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
{
    return *(new SfxItemSet(rPool,

        // range from SdrAttrObj
        SDRATTR_START, SDRATTR_SHADOW_LAST,
        SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
        SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

        // range for SdrTableObj
        SDRATTR_TABLE_FIRST, SDRATTR_TABLE_LAST,

        // range from SdrTextObj
        EE_ITEMS_START, EE_ITEMS_END,

        // end
        0, 0));
}

class TableObjectGeoData : public SdrTextObjGeoData
{
public:
    Rectangle   maLogicRect;
};

//------------------------------------------------------------------------
// TableStyleSettings
//------------------------------------------------------------------------

TableStyleSettings::TableStyleSettings()
: mbUseFirstRow(true)
, mbUseLastRow(false)
, mbUseFirstColumn(false)
, mbUseLastColumn(false)
, mbUseRowBanding(true)
, mbUseColumnBanding(false)
{
}

TableStyleSettings::TableStyleSettings( const TableStyleSettings& rStyle )
{
    (*this) = rStyle;
}

TableStyleSettings& TableStyleSettings::operator=(const TableStyleSettings& rStyle)
{
    mbUseFirstRow = rStyle.mbUseFirstRow;
    mbUseLastRow = rStyle.mbUseLastRow;
    mbUseFirstColumn = rStyle.mbUseFirstColumn;
    mbUseLastColumn = rStyle.mbUseLastColumn;
    mbUseRowBanding = rStyle.mbUseRowBanding;
    mbUseColumnBanding = rStyle.mbUseColumnBanding;
    return *this;
}

bool TableStyleSettings::operator==( const TableStyleSettings& rStyle ) const
{
    return
        (mbUseFirstRow == rStyle.mbUseFirstRow) &&
        (mbUseLastRow == rStyle.mbUseLastRow) &&
        (mbUseFirstColumn == rStyle.mbUseFirstColumn) &&
        (mbUseLastColumn == rStyle.mbUseLastColumn) &&
        (mbUseRowBanding == rStyle.mbUseRowBanding) &&
        (mbUseColumnBanding == rStyle.mbUseColumnBanding);
}

// -----------------------------------------------------------------------------

class SdrTableObjImpl : public TableDesignUser, public ::cppu::WeakImplHelper1< ::com::sun::star::util::XModifyListener >
{
public:
    CellRef mxActiveCell;
    TableModelRef mxTable;
    SdrTableObj* mpTableObj;
    TableLayouter* mpLayouter;
    CellPos maEditPos;
    TableStyleSettings maTableStyle;
    Reference< XIndexAccess > mxTableStyle;
    bool mbModifyPending;

    void SetModel(SdrModel* pOldModel, SdrModel* pNewModel);

    CellRef getCell( const CellPos& rPos ) const;
    void LayoutTable( Rectangle& rArea, bool bFitWidth, bool bFitHeight );

    bool ApplyCellStyles();
    void UpdateCells( Rectangle& rArea );

    SdrTableObjImpl();
    virtual ~SdrTableObjImpl();

    void init( SdrTableObj* pTable, sal_Int32 nColumns, sal_Int32 nRows );
    void dispose();

    sal_Int32 getColumnCount() const;
    sal_Int32 getRowCount() const;

    void DragEdge( bool mbHorizontal, int nEdge, sal_Int32 nOffset );

    SdrTableObjImpl& operator=( const SdrTableObjImpl& rSource );

    // XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    void update();

    void connectTableStyle();
    void disconnectTableStyle();
    virtual bool isInUse();
};

// -----------------------------------------------------------------------------

SdrTableObjImpl::SdrTableObjImpl()
: mpTableObj( 0 )
, mpLayouter( 0 )
, mbModifyPending( false )
{
}

// -----------------------------------------------------------------------------

SdrTableObjImpl::~SdrTableObjImpl()
{
}

// -----------------------------------------------------------------------------

void SdrTableObjImpl::init( SdrTableObj* pTable, sal_Int32 nColumns, sal_Int32 nRows )
{
    mpTableObj = pTable;
    mxTable = new TableModel( pTable );
    mxTable->init( nColumns, nRows );
    mpLayouter = new TableLayouter( mxTable );
    Reference< XModifyListener > xListener( static_cast< ::com::sun::star::util::XModifyListener* >(this) );
    mxTable->addModifyListener( xListener );
    LayoutTable( mpTableObj->aRect, true, true );
    mpTableObj->maLogicRect = mpTableObj->aRect;
}

// -----------------------------------------------------------------------------

SdrTableObjImpl& SdrTableObjImpl::operator=( const SdrTableObjImpl& rSource )
{
    if (this != &rSource)
    {
        if( mpLayouter )
        {
            delete mpLayouter;
            mpLayouter = 0;
        }

        if( mxTable.is() )
        {
            Reference< XModifyListener > xListener( static_cast< ::com::sun::star::util::XModifyListener* >(this) );
            mxTable->removeModifyListener( xListener );
            mxTable->dispose();
            mxTable.clear();
        }

        maTableStyle = rSource.maTableStyle;

        mxTable = new TableModel( mpTableObj, rSource.mxTable );
        mpLayouter = new TableLayouter( mxTable );
        Reference< XModifyListener > xListener( static_cast< ::com::sun::star::util::XModifyListener* >(this) );
        mxTable->addModifyListener( xListener );
        mxTableStyle = rSource.mxTableStyle;
        ApplyCellStyles();
        mpTableObj->aRect = mpTableObj->maLogicRect;
        LayoutTable( mpTableObj->aRect, false, false );
    }
    return *this;
}

// -----------------------------------------------------------------------------

void SdrTableObjImpl::SetModel(SdrModel* /*pOldModel*/, SdrModel* pNewModel)
{
    // try to find new table style
    disconnectTableStyle();

    Reference< XIndexAccess > xNewTableStyle;
    if( mxTableStyle.is() ) try
    {
        const OUString sStyleName( Reference< XNamed >( mxTableStyle, UNO_QUERY_THROW )->getName() );

        Reference< XStyleFamiliesSupplier > xSFS( pNewModel->getUnoModel(), UNO_QUERY_THROW );
        Reference< XNameAccess > xFamilyNameAccess( xSFS->getStyleFamilies(), UNO_QUERY_THROW );
        const OUString sFamilyName( "table" );
        Reference< XNameAccess > xTableFamilyAccess( xFamilyNameAccess->getByName( sFamilyName ), UNO_QUERY_THROW );

        if( xTableFamilyAccess->hasByName( sStyleName ) )
        {
            // found table style with the same name
            xTableFamilyAccess->getByName( sStyleName ) >>= xNewTableStyle;
        }
        else
        {
            // copy or?
            Reference< XIndexAccess > xIndexAccess( xTableFamilyAccess, UNO_QUERY_THROW );
            xIndexAccess->getByIndex( 0 ) >>= xNewTableStyle;
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("svx::SdrTableObjImpl::SetModel(), exception caught!");
    }

    mxTableStyle = xNewTableStyle;

    connectTableStyle();
    update();
}

// -----------------------------------------------------------------------------

bool SdrTableObjImpl::ApplyCellStyles()
{
    if( !mxTable.is() || !mxTable.is() || !mxTableStyle.is() )
        return false;

    bool bChanges = false;

    const sal_Int32 nColCount = getColumnCount();
    const sal_Int32 nRowCount = getRowCount();

    const TableStyleSettings& rStyle = maTableStyle;

    CellPos aPos;
    for( aPos.mnRow = 0; aPos.mnRow < nRowCount; ++aPos.mnRow )
    {
        const bool bFirstRow = (aPos.mnRow == 0) && rStyle.mbUseFirstRow;
        const bool bLastRow = (aPos.mnRow == nRowCount-1) && rStyle.mbUseLastRow;

        for( aPos.mnCol = 0; aPos.mnCol < nColCount; ++aPos.mnCol )
        {
            Reference< XStyle > xStyle;

            // first and last row win first, if used and available
            if( bFirstRow )
            {
                mxTableStyle->getByIndex(first_row_style) >>= xStyle;
            }
            else if( bLastRow )
            {
                mxTableStyle->getByIndex(last_row_style) >>= xStyle;
            }

            if( !xStyle.is() )
            {
                // next come first and last column, if used and available
                if( rStyle.mbUseFirstColumn && (aPos.mnCol == 0)  )
                {
                    mxTableStyle->getByIndex(first_column_style) >>= xStyle;
                }
                else if( rStyle.mbUseLastColumn && (aPos.mnCol == nColCount-1) )
                {
                    mxTableStyle->getByIndex(last_column_style) >>= xStyle;
                }
            }

            if( !xStyle.is() && rStyle.mbUseRowBanding )
            {
                if( (aPos.mnRow & 1) == 0 )
                {
                    mxTableStyle->getByIndex(even_rows_style) >>= xStyle;
                }
                else
                {
                    mxTableStyle->getByIndex(odd_rows_style) >>= xStyle;
                }
            }

            if( !xStyle.is() && rStyle.mbUseColumnBanding )
            {
                if( (aPos.mnCol & 1) == 0 )
                {
                    mxTableStyle->getByIndex(even_columns_style) >>= xStyle;
                }
                else
                {
                    mxTableStyle->getByIndex(odd_columns_style) >>= xStyle;
                }
            }

            if( !xStyle.is() )
            {
                // use default cell style if non found yet
                mxTableStyle->getByIndex(body_style) >>= xStyle;
            }


            if( xStyle.is() )
            {
                SfxUnoStyleSheet* pStyle = SfxUnoStyleSheet::getUnoStyleSheet(xStyle);

                if( pStyle )
                {
                    CellRef xCell( getCell( aPos ) );
                    if( xCell.is() && ( xCell->GetStyleSheet() != pStyle ) )
                    {
                        bChanges = true;
                        xCell->SetStyleSheet( pStyle, sal_True );
                    }
                }
            }
        }
    }

    return bChanges;
}

// -----------------------------------------------------------------------------

void SdrTableObjImpl::dispose()
{
    if( mxTable.is() )
        mxTable->dispose();
}

// -----------------------------------------------------------------------------

void SdrTableObjImpl::DragEdge( bool mbHorizontal, int nEdge, sal_Int32 nOffset )
{
    if( (nEdge >= 0) && mxTable.is()) try
    {
        const OUString sSize( "Size" );
        if( mbHorizontal )
        {
            if( (nEdge >= 0) && (nEdge <= getRowCount()) )
            {
                sal_Int32 nHeigth = mpLayouter->getRowHeight( (!nEdge)?nEdge:(nEdge-1) );
                if(nEdge==0)
                    nHeigth -= nOffset;
                else
                    nHeigth += nOffset;
                Reference< XIndexAccess > xRows( mxTable->getRows(), UNO_QUERY_THROW );
                Reference< XPropertySet > xRowSet( xRows->getByIndex( (!nEdge)?nEdge:(nEdge-1) ), UNO_QUERY_THROW );
                xRowSet->setPropertyValue( sSize, Any( nHeigth ) );
            }
        }
        else
        {
            /*
            fixes fdo#59889 and resizing of table in edge dragging
            Total vertical edges in a NxN table is N+1, indexed from 0 to N and total Columns is N, indexed from 0 to N-1
            In LTR table vertical edge responsible for dragging of column x(x=0 to N-1) is, Edge x+1
            But in RTL table vertical edge responisble for dragging of column x(x=0 to N-1, but from right to left)is, Edge x
            In LTR table dragging of edge 0(for RTL table edge N) does nothing.
            */
            //Todo: Implement Dragging functionality for leftmost edge of table.
            if( (nEdge >= 0) && (nEdge <= getColumnCount()) )
            {
                const bool bRTL = !mpTableObj? false: (mpTableObj->GetWritingMode() == WritingMode_RL_TB);
                sal_Int32 nWidth;
                if(bRTL)
                {
                    nWidth = mpLayouter->getColumnWidth( nEdge );
                }
                else
                {
                    nWidth = mpLayouter->getColumnWidth( (!nEdge)?nEdge:(nEdge-1) );
                }
                Reference< XIndexAccess > xCols( mxTable->getColumns(), UNO_QUERY_THROW );
                nWidth += nOffset;
                if(bRTL && nEdge<getColumnCount())
                {
                    Reference< XPropertySet > xColSet( xCols->getByIndex( nEdge ), UNO_QUERY_THROW );
                    xColSet->setPropertyValue( sSize, Any( nWidth ) );
                }
                else if(!bRTL && nEdge>0)
                {
                    Reference< XPropertySet > xColSet( xCols->getByIndex( (nEdge-1) ), UNO_QUERY_THROW );
                    xColSet->setPropertyValue( sSize, Any( nWidth ) );
                }
                /* To prevent the table resizing on edge dragging */
                if( nEdge > 0 && nEdge < mxTable->getColumnCount() )
                {

                    if( bRTL )
                        nEdge--;

                    if( (bRTL && (nEdge >= 0)) || (!bRTL && (nEdge < mxTable->getColumnCount())) )
                    {
                        nWidth = mpLayouter->getColumnWidth( nEdge );
                        nWidth = std::max( (sal_Int32)(nWidth - nOffset), (sal_Int32)0 );

                        Reference< XPropertySet > xColSet( xCols->getByIndex( nEdge ), UNO_QUERY_THROW );
                        xColSet->setPropertyValue( sSize, Any( nWidth ) );
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "svx::SdrTableObjImpl::DragEdge(), exception caught!" );
    }
}

// -----------------------------------------------------------------------------
// XModifyListener
// -----------------------------------------------------------------------------

void SAL_CALL SdrTableObjImpl::modified( const ::com::sun::star::lang::EventObject& /*aEvent*/ ) throw (::com::sun::star::uno::RuntimeException)
{
    update();
}

void SdrTableObjImpl::update()
{
    // source can be the table model itself or the assigned table template
    TableModelNotifyGuard aGuard( mxTable.get() );
    if( mpTableObj )
    {
        if( (maEditPos.mnRow >= getRowCount()) || (maEditPos.mnCol >= getColumnCount()) || (getCell( maEditPos ) != mxActiveCell) )
        {
            if(maEditPos.mnRow >= getRowCount())
                maEditPos.mnRow = getRowCount()-1;

            if(maEditPos.mnCol >= getColumnCount())
                maEditPos.mnCol = getColumnCount()-1;

            mpTableObj->setActiveCell( maEditPos );
        }

        ApplyCellStyles();

        mpTableObj->aRect = mpTableObj->maLogicRect;
        LayoutTable( mpTableObj->aRect, false, false );

        mpTableObj->SetRectsDirty();
        mpTableObj->ActionChanged();
        mpTableObj->BroadcastObjectChange();
    }
}

// -----------------------------------------------------------------------------

void SdrTableObjImpl::connectTableStyle()
{
    if( mxTableStyle.is() )
    {
        Reference< XModifyBroadcaster > xBroadcaster( mxTableStyle, UNO_QUERY );
        if( xBroadcaster.is() )
        {
            Reference< XModifyListener > xListener( static_cast< ::com::sun::star::util::XModifyListener* >(this) );
            xBroadcaster->addModifyListener( xListener );
        }
    }
}

// -----------------------------------------------------------------------------

void SdrTableObjImpl::disconnectTableStyle()
{
    if( mxTableStyle.is() )
    {
        Reference< XModifyBroadcaster > xBroadcaster( mxTableStyle, UNO_QUERY );
        if( xBroadcaster.is() )
        {
            Reference< XModifyListener > xListener( static_cast< ::com::sun::star::util::XModifyListener* >(this) );
            xBroadcaster->removeModifyListener( xListener );
        }
    }
}

// -----------------------------------------------------------------------------

bool SdrTableObjImpl::isInUse()
{
    return mpTableObj && mpTableObj->IsInserted();
}

// -----------------------------------------------------------------------------
// XEventListener
// -----------------------------------------------------------------------------

void SAL_CALL SdrTableObjImpl::disposing( const ::com::sun::star::lang::EventObject& /*Source*/ ) throw (::com::sun::star::uno::RuntimeException)
{
    mxActiveCell.clear();
    mxTable.clear();
    if( mpLayouter )
    {
        delete mpLayouter;
        mpLayouter = 0;
    }
    mpTableObj = 0;
}

// -----------------------------------------------------------------------------

CellRef SdrTableObjImpl::getCell(  const CellPos& rPos  ) const
{
    CellRef xCell;
    if( mxTable.is() ) try
    {
        xCell.set( dynamic_cast< Cell* >( mxTable->getCellByPosition( rPos.mnCol, rPos.mnRow ).get() ) );
    }
    catch( Exception& )
    {
        OSL_FAIL( "svx::SdrTableObjImpl::getCell(), exception caught!" );
    }
    return xCell;
}

// -----------------------------------------------------------------------------

sal_Int32 SdrTableObjImpl::getColumnCount() const
{
    return mxTable.is() ? mxTable->getColumnCount() : 0;
}

// -----------------------------------------------------------------------------

sal_Int32 SdrTableObjImpl::getRowCount() const
{
    return mxTable.is() ? mxTable->getRowCount() : 0;
}

// -----------------------------------------------------------------------------

void SdrTableObjImpl::LayoutTable( Rectangle& rArea, bool bFitWidth, bool bFitHeight )
{
    if( mpLayouter && mpTableObj->GetModel() )
    {
        TableModelNotifyGuard aGuard( mxTable.get() );
        mpLayouter->LayoutTable( rArea, bFitWidth, bFitHeight );
    }
}

// -----------------------------------------------------------------------------

void SdrTableObjImpl::UpdateCells( Rectangle& rArea )
{
    if( mpLayouter && mxTable.is() )
    {
        TableModelNotifyGuard aGuard( mxTable.get() );
        mpLayouter->updateCells( rArea );
        mxTable->setModified(sal_True);
    }
}

// -----------------------------------------------------------------------------
// BaseProperties section
// -----------------------------------------------------------------------------

sdr::properties::BaseProperties* SdrTableObj::CreateObjectSpecificProperties()
{
    return new TableProperties(*this);
}

// -----------------------------------------------------------------------------
// DrawContact section
// -----------------------------------------------------------------------------

sdr::contact::ViewContact* SdrTableObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfTableObj(*this);
}

// --------------------------------------------------------------------

TYPEINIT1(SdrTableObj,SdrTextObj);

// --------------------------------------------------------------------

SdrTableObj::SdrTableObj(SdrModel* _pModel)
{
    pModel = _pModel;
    init( 1, 1 );
}

// --------------------------------------------------------------------

SdrTableObj::SdrTableObj(SdrModel* _pModel, const ::Rectangle& rNewRect, sal_Int32 nColumns, sal_Int32 nRows)
: SdrTextObj( rNewRect )
, maLogicRect( rNewRect )
{
    pModel = _pModel;

    if( nColumns <= 0 )
        nColumns = 1;

    if( nRows <= 0 )
        nRows = 1;

    init( nColumns, nRows );
}

// --------------------------------------------------------------------

void SdrTableObj::init( sal_Int32 nColumns, sal_Int32 nRows )
{
    bClosedObj = sal_True;

    mpImpl = new SdrTableObjImpl;
    mpImpl->acquire();
    mpImpl->init( this, nColumns, nRows );
}

// --------------------------------------------------------------------

SdrTableObj::~SdrTableObj()
{
    mpImpl->dispose();
    mpImpl->release();
}

// --------------------------------------------------------------------
// table stuff
// --------------------------------------------------------------------

Reference< XTable > SdrTableObj::getTable() const
{
    return Reference< XTable >( mpImpl->mxTable.get() );
}

// --------------------------------------------------------------------

bool SdrTableObj::isValid( const CellPos& rPos ) const
{
    return (rPos.mnCol >= 0) && (rPos.mnCol < mpImpl->getColumnCount()) && (rPos.mnRow >= 0) && (rPos.mnRow < mpImpl->getRowCount());
}

// --------------------------------------------------------------------

CellPos SdrTableObj::getFirstCell() const
{
    return CellPos( 0,0 );
}

// --------------------------------------------------------------------

CellPos SdrTableObj::getLastCell() const
{
    CellPos aPos;
    if( mpImpl->mxTable.is() )
    {
        aPos.mnCol = mpImpl->getColumnCount()-1;
        aPos.mnRow = mpImpl->getRowCount()-1;
    }
    return aPos;
}

// --------------------------------------------------------------------

CellPos SdrTableObj::getLeftCell( const CellPos& rPos, bool bEdgeTravel ) const
{
    switch( GetWritingMode() )
    {
    default:
    case WritingMode_LR_TB:
        return getPreviousCell( rPos, bEdgeTravel );
    case WritingMode_RL_TB:
        return getNextCell( rPos, bEdgeTravel );
    case WritingMode_TB_RL:
        return getPreviousRow( rPos, bEdgeTravel );
    }
}

// --------------------------------------------------------------------

CellPos SdrTableObj::getRightCell( const CellPos& rPos, bool bEdgeTravel  ) const
{
    switch( GetWritingMode() )
    {
    default:
    case WritingMode_LR_TB:
        return getNextCell( rPos, bEdgeTravel );
    case WritingMode_RL_TB:
        return getPreviousCell( rPos, bEdgeTravel );
    case WritingMode_TB_RL:
        return getNextRow( rPos, bEdgeTravel );
    }
}

// --------------------------------------------------------------------

CellPos SdrTableObj::getUpCell( const CellPos& rPos, bool bEdgeTravel ) const
{
    switch( GetWritingMode() )
    {
    default:
    case WritingMode_LR_TB:
    case WritingMode_RL_TB:
        return getPreviousRow( rPos, bEdgeTravel );
    case WritingMode_TB_RL:
        return getPreviousCell( rPos, bEdgeTravel );
    }
}

// --------------------------------------------------------------------

CellPos SdrTableObj::getDownCell( const CellPos& rPos, bool bEdgeTravel ) const
{
    switch( GetWritingMode() )
    {
    default:
    case WritingMode_LR_TB:
    case WritingMode_RL_TB:
        return getNextRow( rPos, bEdgeTravel );
    case WritingMode_TB_RL:
        return getNextCell( rPos, bEdgeTravel );
    }
}

// --------------------------------------------------------------------

CellPos SdrTableObj::getPreviousCell( const CellPos& rPos, bool bEdgeTravel ) const
{
    CellPos aPos( rPos );
    if( mpImpl )
    {
        CellRef xCell( mpImpl->getCell( aPos ) );
        if( xCell.is() && xCell->isMerged() )
        {
            sal_Int32 nTemp = 0;
            findMergeOrigin( mpImpl->mxTable.get(), aPos.mnCol, aPos.mnRow, aPos.mnCol, nTemp );
        }

        if( aPos.mnCol > 0 )
        {
            --aPos.mnCol;
        }

        else if( bEdgeTravel && (aPos.mnRow > 0) )
        {
            aPos.mnCol = mpImpl->mxTable->getColumnCount()-1;
            --aPos.mnRow;
        }
    }
    return aPos;
}

// --------------------------------------------------------------------

CellPos SdrTableObj::getNextCell( const CellPos& rPos, bool bEdgeTravel ) const
{
    CellPos aPos( rPos );
    if( mpImpl )
    {
        CellRef xCell( mpImpl->getCell( aPos ) );
        if( xCell.is() )
        {
            if( xCell->isMerged() )
            {
                findMergeOrigin( mpImpl->mxTable, aPos.mnCol, aPos.mnRow, aPos.mnCol, aPos.mnRow );

                xCell = mpImpl->getCell(aPos);

                if( xCell.is() )
                {
                    aPos.mnCol += xCell->getColumnSpan();
                    aPos.mnRow = rPos.mnRow;
                }
            }
            else
            {
                aPos.mnCol += xCell->getColumnSpan();
            }

            if( aPos.mnCol < mpImpl->mxTable->getColumnCount() )
                return aPos;

            if( bEdgeTravel && ((aPos.mnRow + 1) < mpImpl->getRowCount()) )
            {
                aPos.mnCol = 0;
                aPos.mnRow += 1;
                return aPos;
            }
        }
    }

    // last cell reached, no traveling possible
    return rPos;
}

// --------------------------------------------------------------------

CellPos SdrTableObj::getPreviousRow( const CellPos& rPos, bool bEdgeTravel ) const
{
    CellPos aPos( rPos );
    if( mpImpl )
    {
        CellRef xCell( mpImpl->getCell( aPos ) );
        if( xCell.is() )
        {
            if( xCell->isMerged() )
            {
                sal_Int32 nTemp = 0;
                findMergeOrigin( mpImpl->mxTable, aPos.mnCol, aPos.mnRow, nTemp, aPos.mnRow );
            }
        }

        if( aPos.mnRow > 0 )
        {
            --aPos.mnRow;
        }
        else if( bEdgeTravel && (aPos.mnCol > 0) )
        {
            aPos.mnRow = mpImpl->mxTable->getRowCount()-1;
            --aPos.mnCol;
        }
    }
    return aPos;
}

// --------------------------------------------------------------------

CellPos SdrTableObj::getNextRow( const CellPos& rPos, bool bEdgeTravel ) const
{
    CellPos aPos( rPos );

    if( mpImpl )
    {
        CellRef xCell( mpImpl->getCell( rPos ) );
        if( xCell.is() )
        {
            if( xCell->isMerged() )
            {
                findMergeOrigin( mpImpl->mxTable, aPos.mnCol, aPos.mnRow, aPos.mnCol, aPos.mnRow );
                xCell = mpImpl->getCell(aPos);
                aPos.mnCol = rPos.mnCol;
            }

            if( xCell.is() )
                aPos.mnRow += xCell->getRowSpan();

            if( aPos.mnRow < mpImpl->mxTable->getRowCount() )
                return aPos;

            if( bEdgeTravel && (aPos.mnCol + 1) < mpImpl->mxTable->getColumnCount() )
            {
                aPos.mnRow = 0;
                aPos.mnCol += 1;

                while( aPos.mnCol < mpImpl->mxTable->getColumnCount() )
                {
                    xCell = mpImpl->getCell( aPos );
                    if( xCell.is() && !xCell->isMerged() )
                        return aPos;
                    aPos.mnCol += 1;
                }
            }
        }
    }

    // last position reached, no more traveling possible
    return rPos;
}

// --------------------------------------------------------------------

const TableStyleSettings& SdrTableObj::getTableStyleSettings() const
{
    if( mpImpl )
    {
        return mpImpl->maTableStyle;
    }
    else
    {
        static TableStyleSettings aTmp;
        return aTmp;
    }
}

// --------------------------------------------------------------------

void SdrTableObj::setTableStyleSettings( const TableStyleSettings& rStyle )
{
    if( mpImpl )
    {
        mpImpl->maTableStyle = rStyle;
        mpImpl->update();
    }
}

// --------------------------------------------------------------------

TableHitKind SdrTableObj::CheckTableHit( const Point& rPos, sal_Int32& rnX, sal_Int32& rnY, int nTol ) const
{
    if( !mpImpl || !mpImpl->mxTable.is() )
        return SDRTABLEHIT_NONE;

    rnX = 0;
    rnY = 0;

    const sal_Int32 nColCount = mpImpl->getColumnCount();
    const sal_Int32 nRowCount = mpImpl->getRowCount();

    sal_Int32 nX = rPos.X() + nTol - aRect.Left();
    sal_Int32 nY = rPos.Y() + nTol - aRect.Top();

    if( (nX < 0) || (nX > (aRect.GetWidth() + nTol)) || (nY < 0) || (nY > (aRect.GetHeight() + nTol) ) )
        return SDRTABLEHIT_NONE;

    // get vertical edge number and check for a hit
    const bool bRTL = (GetWritingMode() == WritingMode_RL_TB);
    bool bVrtHit = false;
    if( nX >= 0 )
    {
        if( !bRTL )
        {
            while( rnX <= nColCount )
            {
                if( nX <= (2*nTol) )
                {
                    bVrtHit = true;
                    break;
                }

                if( rnX == nColCount )
                    break;

                nX -= mpImpl->mpLayouter->getColumnWidth( rnX );
                if( nX < 0 )
                    break;
                rnX++;
            }
        }
        else
        {
            rnX = nColCount;
            while( rnX >= 0 )
            {
                if( nX <= (2*nTol) )
                {
                    bVrtHit = true;
                    break;
                }

                if( rnX == 0 )
                    break;

                rnX--;
                nX -= mpImpl->mpLayouter->getColumnWidth( rnX );
                if( nX < 0 )
                    break;
            }
        }
    }

    // rnX is now the edge number left to the pointer, if it was hit bHrzHit is also true

    // get vertical edge number and check for a hit
    bool bHrzHit = false;
    if( nY >= 0 )
    {
        while( rnY <= nRowCount )
        {
            if( nY <= (2*nTol) )
            {
                bHrzHit = true;
                break;
            }

            if( rnY == nRowCount )
                break;

            nY -= mpImpl->mpLayouter->getRowHeight(rnY);
            if( nY < 0 )
                break;
            rnY++;
        }
    }

    // rnY is now the edge number above the pointer, if it was hit bVrtHit is also true

    if( bVrtHit && mpImpl->mpLayouter->isEdgeVisible( rnX, rnY, false ) )
        return SDRTABLEHIT_VERTICAL_BORDER;

    if( bHrzHit && mpImpl->mpLayouter->isEdgeVisible( rnX, rnY, true ) )
        return SDRTABLEHIT_HORIZONTAL_BORDER;

    CellRef xCell( mpImpl->getCell( CellPos( rnX, rnY ) ) );
    if( xCell.is() && xCell->isMerged() )
        findMergeOrigin( mpImpl->mxTable.get(), rnX, rnY, rnX, rnY );

    if( xCell.is() )
    {
        nX += mpImpl->mpLayouter->getColumnWidth( rnX );
        //Fix for fdo#62673 : non-editable cell in table on cell merge
        sal_Int32 i=0;
        while(xCell.is() && xCell->isMerged())
        {
            nX += mpImpl->mpLayouter->getColumnWidth( rnX+i );
            i++;
            if(rnX+i < nColCount)
                xCell=mpImpl->getCell( CellPos( rnX+i, rnY) );
            else
                break;
        }

        if( nX < xCell->GetTextLeftDistance() )
            return SDRTABLEHIT_CELL;
    }

    return SDRTABLEHIT_CELLTEXTAREA;
}

const SfxItemSet& SdrTableObj::GetActiveCellItemSet() const
{
    return getActiveCell()->GetItemSet();
}

// --------------------------------------------------------------------

void SdrTableObj::setTableStyle( const Reference< XIndexAccess >& xTableStyle )
{
    if( mpImpl && (mpImpl->mxTableStyle != xTableStyle) )
    {
        mpImpl->disconnectTableStyle();
        mpImpl->mxTableStyle = xTableStyle;
        mpImpl->connectTableStyle();
        mpImpl->update();
    }
}

// --------------------------------------------------------------------

const Reference< XIndexAccess >& SdrTableObj::getTableStyle() const
{
    if( mpImpl )
    {
        return mpImpl->mxTableStyle;
    }
    else
    {
        static Reference< XIndexAccess > aTmp;
        return aTmp;
    }
}

// --------------------------------------------------------------------
// text stuff
// --------------------------------------------------------------------

/** returns the currently active text. */
SdrText* SdrTableObj::getActiveText() const
{
    return dynamic_cast< SdrText* >( getActiveCell().get() );
}

// --------------------------------------------------------------------

/** returns the nth available text. */
SdrText* SdrTableObj::getText( sal_Int32 nIndex ) const
{
    if( mpImpl->mxTable.is() )
    {
        const sal_Int32 nColCount = mpImpl->getColumnCount();
        if( nColCount )
        {
            CellPos aPos( nIndex % nColCount, nIndex / nColCount );

            CellRef xCell( mpImpl->getCell( aPos ) );
            return dynamic_cast< SdrText* >( xCell.get() );
        }
    }
    return 0;
}

// --------------------------------------------------------------------

/** returns the number of texts available for this object. */
sal_Int32 SdrTableObj::getTextCount() const
{
    if( mpImpl->mxTable.is() )
    {
        const sal_Int32 nColCount = mpImpl->getColumnCount();
        const sal_Int32 nRowCount = mpImpl->getRowCount();

        return nColCount * nRowCount;
    }
    else
    {
        return 0;
    }
}

// --------------------------------------------------------------------

/** changes the current active text */
void SdrTableObj::setActiveText( sal_Int32 nIndex )
{
    if( mpImpl && mpImpl->mxTable.is() )
    {
        const sal_Int32 nColCount = mpImpl->mxTable->getColumnCount();
        if( nColCount )
        {
            CellPos aPos( nIndex % nColCount, nIndex / nColCount );
            if( isValid( aPos ) )
                setActiveCell( aPos );
        }
    }
}

// --------------------------------------------------------------------

/** returns the index of the text that contains the given point or -1 */
sal_Int32 SdrTableObj::CheckTextHit(const Point& rPnt) const
{
    if( mpImpl && mpImpl->mxTable.is() )
    {
        CellPos aPos;
        if( CheckTableHit( rPnt, aPos.mnCol, aPos.mnRow, 0 ) == SDRTABLEHIT_CELLTEXTAREA )
            return aPos.mnRow * mpImpl->mxTable->getColumnCount() + aPos.mnCol;
    }

    return 0;
}

// --------------------------------------------------------------------

SdrOutliner* SdrTableObj::GetCellTextEditOutliner( const Cell& rCell ) const
{
    if( mpImpl && (mpImpl->getCell( mpImpl->maEditPos ).get() == &rCell) )
        return pEdtOutl;
    else
        return 0;
}


// --------------------------------------------------------------------

const TableLayouter& SdrTableObj::getTableLayouter() const
{
    OSL_ENSURE(mpImpl && mpImpl->mpLayouter, "getTableLayouter() error: no mpImpl or mpLayouter (!)");
    return *(mpImpl->mpLayouter);
}

// --------------------------------------------------------------------

bool SdrTableObj::IsAutoGrowHeight() const
{
    return true;
}

// --------------------------------------------------------------------

bool SdrTableObj::IsAutoGrowWidth() const
{
    return true;
}

// --------------------------------------------------------------------

bool SdrTableObj::HasText() const
{
    return true;
}

// --------------------------------------------------------------------

bool SdrTableObj::IsTextEditActive( const CellPos& rPos )
{
    return pEdtOutl && mpImpl && (rPos == mpImpl->maEditPos);
}

// --------------------------------------------------------------------

void SdrTableObj::onEditOutlinerStatusEvent( EditStatus* pEditStatus )
{
    if( (pEditStatus->GetStatusWord() & EE_STAT_TEXTHEIGHTCHANGED) && mpImpl && mpImpl->mpLayouter )
    {
        Rectangle aRect0( aRect );
        aRect = maLogicRect;
        mpImpl->LayoutTable( aRect, false, false );
        SetRectsDirty();
        ActionChanged();
        BroadcastObjectChange();
        if( aRect0 != aRect )
            SendUserCall(SDRUSERCALL_RESIZE,aRect0);
    }
}

// --------------------------------------------------------------------

void SdrTableObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bResizeFreeAllowed=sal_True;
    rInfo.bResizePropAllowed=sal_True;
    rInfo.bRotateFreeAllowed=sal_False;
    rInfo.bRotate90Allowed  =sal_False;
    rInfo.bMirrorFreeAllowed=sal_False;
    rInfo.bMirror45Allowed  =sal_False;
    rInfo.bMirror90Allowed  =sal_False;

    // allow transparence
    rInfo.bTransparenceAllowed = sal_True;

    // gradient depends on fillstyle
    XFillStyle eFillStyle = ((XFillStyleItem&)(GetObjectItem(XATTR_FILLSTYLE))).GetValue();
    rInfo.bGradientAllowed = (eFillStyle == XFILL_GRADIENT);
    rInfo.bShearAllowed     =sal_False;
    rInfo.bEdgeRadiusAllowed=sal_False;
    rInfo.bCanConvToPath    =sal_False;
    rInfo.bCanConvToPoly    =sal_False;
    rInfo.bCanConvToPathLineToArea=sal_False;
    rInfo.bCanConvToPolyLineToArea=sal_False;
    rInfo.bCanConvToContour = sal_False;
}

// --------------------------------------------------------------------

sal_uInt16 SdrTableObj::GetObjIdentifier() const
{
    return static_cast<sal_uInt16>(OBJ_TABLE);
}

// --------------------------------------------------------------------

void SdrTableObj::SetPage(SdrPage* pNewPage)
{
    SdrTextObj::SetPage(pNewPage);
}

// --------------------------------------------------------------------

void SdrTableObj::SetModel(SdrModel* pNewModel)
{
    SdrModel* pOldModel = GetModel();
    if( pNewModel != pOldModel )
    {
        SdrTextObj::SetModel(pNewModel);

        if( mpImpl )
        {
            mpImpl->SetModel( pOldModel, pNewModel );

            if( !maLogicRect.IsEmpty() )
            {
                aRect = maLogicRect;
                mpImpl->LayoutTable( aRect, false, false );
            }
        }
    }
}

// --------------------------------------------------------------------

void SdrTableObj::TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, bool bNoEditText, Rectangle* pAnchorRect, bool bLineWidth ) const
{
    if( mpImpl )
        TakeTextRect( mpImpl->maEditPos, rOutliner, rTextRect, bNoEditText, pAnchorRect, bLineWidth );
}

// --------------------------------------------------------------------

void SdrTableObj::TakeTextRect( const CellPos& rPos, SdrOutliner& rOutliner, Rectangle& rTextRect, bool bNoEditText, Rectangle* pAnchorRect, bool /*bLineWidth*/ ) const
{
    if( !mpImpl )
        return;

    CellRef xCell( mpImpl->getCell( rPos ) );
    if( !xCell.is() )
        return;

    Rectangle aAnkRect;
    TakeTextAnchorRect( rPos, aAnkRect );

    SdrTextVertAdjust eVAdj=xCell->GetTextVerticalAdjust();

    sal_uIntPtr nStat0=rOutliner.GetControlWord();
    Size aNullSize;
    nStat0 |= EE_CNTRL_AUTOPAGESIZE;
    rOutliner.SetControlWord(nStat0);
    rOutliner.SetMinAutoPaperSize(aNullSize);
    rOutliner.SetMaxAutoPaperSize(aAnkRect.GetSize());
    rOutliner.SetPaperSize(aAnkRect.GetSize());

    // #103516# New try with _BLOCK for hor and ver after completely
    // supporting full width for vertical text.
//  if( SDRTEXTHORZADJUST_BLOCK == eHAdj && !IsVerticalWriting())
//  {
        rOutliner.SetMinAutoPaperSize(Size(aAnkRect.GetWidth(), 0));
//  }
//  else if(SDRTEXTVERTADJUST_BLOCK == eVAdj && IsVerticalWriting())
//  {
//      rOutliner.SetMinAutoPaperSize(Size(0, aAnkRect.GetHeight()));
//  }

    // ---

    // set text at outliner, maybe from edit outliner
    OutlinerParaObject* pPara= xCell->GetOutlinerParaObject();
    if (pEdtOutl && !bNoEditText && mpImpl->mxActiveCell == xCell )
        pPara=pEdtOutl->CreateParaObject();

    if (pPara)
    {
        const bool bHitTest = pModel && (&pModel->GetHitTestOutliner() == &rOutliner);

        const SdrTextObj* pTestObj = rOutliner.GetTextObj();
        if( !pTestObj || !bHitTest || (pTestObj != this) || (pTestObj->GetOutlinerParaObject() != xCell->GetOutlinerParaObject()) )
        {
            if( bHitTest ) // #i33696# take back fix #i27510#
                rOutliner.SetTextObj( this );

            rOutliner.SetUpdateMode(sal_True);
            rOutliner.SetText(*pPara);
        }
    }
    else
    {
        rOutliner.SetTextObj( NULL );
    }

    if (pEdtOutl && !bNoEditText && pPara && mpImpl->mxActiveCell == xCell )
        delete pPara;

    rOutliner.SetUpdateMode(sal_True);
    rOutliner.SetControlWord(nStat0);

    Point aTextPos(aAnkRect.TopLeft());
    Size aTextSiz(rOutliner.GetPaperSize());
    if (eVAdj==SDRTEXTVERTADJUST_CENTER || eVAdj==SDRTEXTVERTADJUST_BOTTOM)
    {
        long nFreeHgt=aAnkRect.GetHeight()-aTextSiz.Height();
        if (eVAdj==SDRTEXTVERTADJUST_CENTER)
            aTextPos.Y()+=nFreeHgt/2;
        if (eVAdj==SDRTEXTVERTADJUST_BOTTOM)
            aTextPos.Y()+=nFreeHgt;
    }

    if (pAnchorRect)
        *pAnchorRect=aAnkRect;

    rTextRect=Rectangle(aTextPos,aTextSiz);
}

// --------------------------------------------------------------------

const CellRef& SdrTableObj::getActiveCell() const
{
    if( mpImpl )
    {
        if( !mpImpl->mxActiveCell.is() )
        {
            CellPos aPos;
            const_cast< SdrTableObj* >(this)->setActiveCell( aPos );
        }
        return mpImpl->mxActiveCell;
    }
    else
    {
        static CellRef xCell;
        return xCell;
    }
}

// --------------------------------------------------------------------

sal_Int32 SdrTableObj::getRowCount() const
{
    return mpImpl ? mpImpl->getRowCount() : 0;
}

// --------------------------------------------------------------------

sal_Int32 SdrTableObj::getColumnCount() const
{
    return mpImpl ? mpImpl->getColumnCount() : 0;
}

// --------------------------------------------------------------------

void SdrTableObj::setActiveCell( const CellPos& rPos )
{
    if( mpImpl && mpImpl->mxTable.is() ) try
    {
        mpImpl->mxActiveCell.set( dynamic_cast< Cell* >( mpImpl->mxTable->getCellByPosition( rPos.mnCol, rPos.mnRow ).get() ) );
        if( mpImpl->mxActiveCell.is() && mpImpl->mxActiveCell->isMerged() )
        {
            CellPos aOrigin;
            findMergeOrigin( mpImpl->mxTable.get(), rPos.mnCol, rPos.mnRow, aOrigin.mnCol, aOrigin.mnRow );
            mpImpl->mxActiveCell.set( dynamic_cast< Cell* >( mpImpl->mxTable->getCellByPosition( aOrigin.mnCol, aOrigin.mnRow ).get() ) );
            mpImpl->maEditPos = aOrigin;
        }
        else
        {
            mpImpl->maEditPos = rPos;
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("SdrTableObj::setActiveCell(), exception caught!");
    }
}

// --------------------------------------------------------------------

void SdrTableObj::getActiveCellPos( CellPos& rPos ) const
{
    rPos = mpImpl->maEditPos;
}

// --------------------------------------------------------------------

void SdrTableObj::getCellBounds( const CellPos& rPos, ::Rectangle& rCellRect )
{
    if( mpImpl )
    {
        CellRef xCell( mpImpl->getCell( rPos ) );
        if( xCell.is() )
            rCellRect = xCell->getCellRect();
    }
}

// --------------------------------------------------------------------

void SdrTableObj::TakeTextAnchorRect(Rectangle& rAnchorRect) const
{
    if( mpImpl )
        TakeTextAnchorRect( mpImpl->maEditPos, rAnchorRect );
}

// --------------------------------------------------------------------

void SdrTableObj::TakeTextAnchorRect( const CellPos& rPos, Rectangle& rAnchorRect ) const
{
    Rectangle aAnkRect(aRect);

    if( mpImpl )
    {
        CellRef xCell( mpImpl->getCell( rPos ) );
        if( xCell.is() )
            xCell->TakeTextAnchorRect( aAnkRect );
    }

    ImpJustifyRect(aAnkRect);
    rAnchorRect=aAnkRect;
}

// --------------------------------------------------------------------

void SdrTableObj::TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, Rectangle* pViewInit, Rectangle* pViewMin) const
{
    if( mpImpl )
        TakeTextEditArea( mpImpl->maEditPos, pPaperMin, pPaperMax, pViewInit, pViewMin );
}

// --------------------------------------------------------------------

void SdrTableObj::TakeTextEditArea( const CellPos& rPos, Size* pPaperMin, Size* pPaperMax, Rectangle* pViewInit, Rectangle* pViewMin ) const
{
    Size aPaperMin,aPaperMax;
    Rectangle aViewInit;
    TakeTextAnchorRect( rPos, aViewInit );

    Size aAnkSiz(aViewInit.GetSize());
    aAnkSiz.Width()--; aAnkSiz.Height()--; // weil GetSize() ein draufaddiert

    Size aMaxSiz(aAnkSiz.Width(),1000000);
    if (pModel!=NULL)
    {
        Size aTmpSiz(pModel->GetMaxObjSize());
        if (aTmpSiz.Height()!=0)
            aMaxSiz.Height()=aTmpSiz.Height();
    }

    CellRef xCell( mpImpl->getCell( rPos ) );
    SdrTextVertAdjust eVAdj = xCell.is() ? xCell->GetTextVerticalAdjust() : SDRTEXTVERTADJUST_TOP;

    aPaperMax=aMaxSiz;

        aPaperMin.Width() = aAnkSiz.Width();

    if (pViewMin!=NULL)
    {
        *pViewMin=aViewInit;
        long nYFree=aAnkSiz.Height()-aPaperMin.Height();

        if (eVAdj==SDRTEXTVERTADJUST_TOP)
        {
            pViewMin->Bottom()-=nYFree;
        }
        else if (eVAdj==SDRTEXTVERTADJUST_BOTTOM)
        {
            pViewMin->Top()+=nYFree;
        }
        else
        {
            pViewMin->Top()+=nYFree/2;
            pViewMin->Bottom()=pViewMin->Top()+aPaperMin.Height();
        }
    }


    if(IsVerticalWriting())
        aPaperMin.Width() = 0;
    else
        aPaperMin.Height() = 0;

    if (pPaperMin!=NULL) *pPaperMin=aPaperMin;
    if (pPaperMax!=NULL) *pPaperMax=aPaperMax;
    if (pViewInit!=NULL) *pViewInit=aViewInit;
}

// --------------------------------------------------------------------

sal_uInt16 SdrTableObj::GetOutlinerViewAnchorMode() const
{
    EVAnchorMode eRet=ANCHOR_TOP_LEFT;
    CellRef xCell( getActiveCell() );
    if( xCell.is() )
    {
        SdrTextVertAdjust eV=xCell->GetTextVerticalAdjust();

        {
            if (eV==SDRTEXTVERTADJUST_TOP)
            {
                eRet=ANCHOR_TOP_LEFT;
            }
            else if (eV==SDRTEXTVERTADJUST_BOTTOM)
            {
                eRet=ANCHOR_BOTTOM_LEFT;
            }
            else
            {
                eRet=ANCHOR_VCENTER_LEFT;
            }
        }
    }
    return (sal_uInt16)eRet;
}

// --------------------------------------------------------------------

OutlinerParaObject* SdrTableObj::GetEditOutlinerParaObject() const
{
    return SdrTextObj::GetEditOutlinerParaObject();
}

// --------------------------------------------------------------------

struct ImplTableShadowPaintInfo
{
    Color maShadowColor;
    sal_uInt32 mnXDistance;
    sal_uInt32 mnYDistance;
    sal_uInt16 mnShadowTransparence;

    ImplTableShadowPaintInfo( const SfxItemSet& rSet )
    {
        const SdrShadowColorItem& rShadColItem = ((const SdrShadowColorItem&)(rSet.Get(SDRATTR_SHADOWCOLOR)));
        maShadowColor = rShadColItem.GetColorValue();
        mnShadowTransparence = ((const SdrShadowTransparenceItem&)(rSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue();

        mnXDistance = ((SdrShadowXDistItem&)(rSet.Get(SDRATTR_SHADOWXDIST))).GetValue();
        mnYDistance = ((SdrShadowYDistItem&)(rSet.Get(SDRATTR_SHADOWYDIST))).GetValue();
    }
};

// --------------------------------------------------------------------

OUString SdrTableObj::TakeObjNameSingul() const
{
    OUStringBuffer sName(ImpGetResStr(STR_ObjNameSingulTable));

    OUString aName(GetName());
    if (!aName.isEmpty())
    {
        sName.append(' ');
        sName.append('\'');
        sName.append(aName);
        sName.append('\'');
    }

    return sName.makeStringAndClear();
}

// --------------------------------------------------------------------

OUString SdrTableObj::TakeObjNamePlural() const
{
    return ImpGetResStr(STR_ObjNamePluralTable);
}

// --------------------------------------------------------------------

SdrTableObj* SdrTableObj::Clone() const
{
    return CloneHelper< SdrTableObj >();
}

SdrTableObj& SdrTableObj::operator=(const SdrTableObj& rObj)
{
    if( this == &rObj )
        return *this;
    // call parent
    SdrObject::operator=(rObj);

    TableModelNotifyGuard aGuard( mpImpl ? mpImpl->mxTable.get() : 0 );

    maLogicRect = rObj.maLogicRect;
    aRect = rObj.aRect;
    aGeo = rObj.aGeo;
    eTextKind = rObj.eTextKind;
    bTextFrame = rObj.bTextFrame;
    aTextSize = rObj.aTextSize;
    bTextSizeDirty = rObj.bTextSizeDirty;
    bNoShear = rObj.bNoShear;
    bNoRotate = rObj.bNoRotate;
    bNoMirror = rObj.bNoMirror;
    bDisableAutoWidthOnDragging = rObj.bDisableAutoWidthOnDragging;

    if( rObj.mpImpl )
        *mpImpl = *rObj.mpImpl;
    return *this;
}

// --------------------------------------------------------------------

basegfx::B2DPolyPolygon SdrTableObj::TakeXorPoly() const
{
    return SdrTextObj::TakeXorPoly();
}

// --------------------------------------------------------------------

basegfx::B2DPolyPolygon SdrTableObj::TakeContour() const
{
    return SdrTextObj::TakeContour();
}

// --------------------------------------------------------------------

const Rectangle& SdrTableObj::GetSnapRect() const
{
    return aRect;
}

// --------------------------------------------------------------------

void SdrTableObj::NbcSetSnapRect(const Rectangle& rRect)
{
    NbcSetLogicRect( rRect );
}

// --------------------------------------------------------------------

const Rectangle& SdrTableObj::GetLogicRect() const
{
    return maLogicRect;
}

// --------------------------------------------------------------------

void SdrTableObj::RecalcSnapRect()
{
}

// --------------------------------------------------------------------

sal_uInt32 SdrTableObj::GetSnapPointCount() const
{
    return SdrTextObj::GetSnapPointCount();
}

// --------------------------------------------------------------------


Point SdrTableObj::GetSnapPoint(sal_uInt32 i) const
{
    return SdrTextObj::GetSnapPoint(i);
}

// --------------------------------------------------------------------

sal_Bool SdrTableObj::BegTextEdit(SdrOutliner& rOutl)
{
    if( pEdtOutl != NULL )
        return sal_False;

    pEdtOutl=&rOutl;

    mbInEditMode = sal_True;

    rOutl.Init( OUTLINERMODE_TEXTOBJECT );
    rOutl.SetRefDevice( pModel->GetRefDevice() );

        bool bUpdMerk=rOutl.GetUpdateMode();
        if (bUpdMerk) rOutl.SetUpdateMode(sal_False);
        Size aPaperMin;
        Size aPaperMax;
        Rectangle aEditArea;
        TakeTextEditArea(&aPaperMin,&aPaperMax,&aEditArea,NULL);

        rOutl.SetMinAutoPaperSize(aPaperMin);
        rOutl.SetMaxAutoPaperSize(aPaperMax);
        rOutl.SetPaperSize(aPaperMax);

        if (bUpdMerk) rOutl.SetUpdateMode(sal_True);

    sal_uIntPtr nStat=rOutl.GetControlWord();
    nStat   |= EE_CNTRL_AUTOPAGESIZE;
    nStat   &=~EE_CNTRL_STRETCHING;
    rOutl.SetControlWord(nStat);

    OutlinerParaObject* pPara = GetOutlinerParaObject();
    if(pPara)
        rOutl.SetText(*pPara);

    rOutl.UpdateFields();
    rOutl.ClearModifyFlag();

    return sal_True;
}

// --------------------------------------------------------------------

void SdrTableObj::EndTextEdit(SdrOutliner& rOutl)
{
    if(rOutl.IsModified())
    {
        if( GetModel() && GetModel()->IsUndoEnabled() )
            GetModel()->AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*this) );

        OutlinerParaObject* pNewText = 0;
        Paragraph* p1stPara = rOutl.GetParagraph( 0 );
        sal_Int32 nParaAnz = rOutl.GetParagraphCount();

        if(p1stPara)
        {
            if(nParaAnz == 1)
            {
                // if its only one paragraph, check if it is empty
                OUString aStr(rOutl.GetText(p1stPara));
                if (aStr.isEmpty())
                {
                    // gotcha!
                    nParaAnz = 0;
                }
            }

            // to remove the grey field background
            rOutl.UpdateFields();

            if(nParaAnz != 0)
            {
                // create new text object
                pNewText = rOutl.CreateParaObject( 0, nParaAnz );
            }
        }
        SetOutlinerParaObject(pNewText);
    }

    pEdtOutl = 0;
    rOutl.Clear();
    sal_uInt32 nStat = rOutl.GetControlWord();
    nStat &= ~EE_CNTRL_AUTOPAGESIZE;
    rOutl.SetControlWord(nStat);

    mbInEditMode = sal_False;
}

// --------------------------------------------------------------------

OutlinerParaObject* SdrTableObj::GetOutlinerParaObject() const
{
    CellRef xCell( getActiveCell() );
    if( xCell.is() )
        return xCell->GetOutlinerParaObject();
    else
        return 0;
}

// --------------------------------------------------------------------

void SdrTableObj::NbcSetOutlinerParaObject( OutlinerParaObject* pTextObject)
{
    CellRef xCell( getActiveCell() );
    if( xCell.is() )
    {
        if( pModel )
        {
            // Update HitTestOutliner
            const SdrTextObj* pTestObj = pModel->GetHitTestOutliner().GetTextObj();
            if( pTestObj && pTestObj->GetOutlinerParaObject() == xCell->GetOutlinerParaObject() )
                pModel->GetHitTestOutliner().SetTextObj( NULL );
        }

        xCell->SetOutlinerParaObject( pTextObject );

        SetTextSizeDirty();
        NbcAdjustTextFrameWidthAndHeight();
    }
}

// --------------------------------------------------------------------

void SdrTableObj::NbcSetLogicRect(const Rectangle& rRect)
{
    maLogicRect=rRect;
    ImpJustifyRect(maLogicRect);
    const bool bWidth = maLogicRect.getWidth() != aRect.getWidth();
    const bool bHeight = maLogicRect.getHeight() != aRect.getHeight();
    aRect=maLogicRect;
    NbcAdjustTextFrameWidthAndHeight( !bHeight, !bWidth );
    SetRectsDirty();
}


// --------------------------------------------------------------------

void SdrTableObj::AdjustToMaxRect( const Rectangle& rMaxRect, bool /* bShrinkOnly = false */ )
{
    Rectangle aAdjustRect( rMaxRect );
    aAdjustRect.setHeight( GetLogicRect().getHeight() );
    SetLogicRect( aAdjustRect );
}

// --------------------------------------------------------------------

void SdrTableObj::NbcMove(const Size& rSiz)
{
    MoveRect(maLogicRect,rSiz);
    SdrTextObj::NbcMove( rSiz );
    if( mpImpl )
        mpImpl->UpdateCells( aRect );
}

// --------------------------------------------------------------------

void SdrTableObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    Rectangle aOldRect( maLogicRect );
    ResizeRect(maLogicRect,rRef,xFact,yFact);

    aRect = maLogicRect;
    NbcAdjustTextFrameWidthAndHeight( maLogicRect.GetHeight() == aOldRect.GetHeight(), maLogicRect.GetWidth() == aOldRect.GetWidth() );
    SetRectsDirty();
}

// --------------------------------------------------------------------

bool SdrTableObj::AdjustTextFrameWidthAndHeight(bool bHgt, bool bWdt)
{
    Rectangle aNeuRect(maLogicRect);
    bool bRet=AdjustTextFrameWidthAndHeight(aNeuRect,bHgt,bWdt);
    if (bRet)
    {
        Rectangle aBoundRect0;
        if (pUserCall!=NULL)
            aBoundRect0=GetLastBoundRect();
        aRect=aNeuRect;
        SetRectsDirty();
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
    return bRet;
}

// --------------------------------------------------------------------

bool SdrTableObj::AdjustTextFrameWidthAndHeight(Rectangle& rR, bool bHeight, bool bWidth) const
{
    if((pModel == NULL) || rR.IsEmpty() || !mpImpl || !mpImpl->mxTable.is() )
        return sal_False;

    Rectangle aRectangle( rR );
    mpImpl->LayoutTable( aRectangle, !bWidth, !bHeight );

    if( aRectangle != rR )
    {
        rR = aRectangle;
        return sal_True;
    }
    else
    {
        return sal_False;
    }
}

// --------------------------------------------------------------------

void SdrTableObj::NbcReformatText()
{
    NbcAdjustTextFrameWidthAndHeight();
}

// --------------------------------------------------------------------

void SdrTableObj::ReformatText()
{
    Rectangle aBoundRect0;
    if (pUserCall!=NULL)
        aBoundRect0=GetLastBoundRect();
    NbcReformatText();
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

// --------------------------------------------------------------------

sal_Bool SdrTableObj::IsVerticalWriting() const
{
    const SvxWritingModeItem* pModeItem = dynamic_cast< const SvxWritingModeItem* >( &GetObjectItem( SDRATTR_TEXTDIRECTION ) );
    return pModeItem && pModeItem->GetValue() == com::sun::star::text::WritingMode_TB_RL;
}

// --------------------------------------------------------------------

void SdrTableObj::SetVerticalWriting(sal_Bool bVertical )
{
    if( bVertical != IsVerticalWriting() )
    {
        SvxWritingModeItem aModeItem( com::sun::star::text::WritingMode_LR_TB, SDRATTR_TEXTDIRECTION );
        SetObjectItem( aModeItem );
    }
}

// --------------------------------------------------------------------

WritingMode SdrTableObj::GetWritingMode() const
{
    SfxStyleSheet* pStyle = GetStyleSheet();
    if ( !pStyle )
        return WritingMode_LR_TB;

    WritingMode eWritingMode = WritingMode_LR_TB;
    const SfxItemSet &rSet = pStyle->GetItemSet();
    const SfxPoolItem *pItem;

    if ( rSet.GetItemState( SDRATTR_TEXTDIRECTION, sal_False, &pItem ) == SFX_ITEM_SET )
        eWritingMode = static_cast< WritingMode >( static_cast< const SvxWritingModeItem * >( pItem )->GetValue() );

    if ( ( eWritingMode != WritingMode_TB_RL ) &&
         ( rSet.GetItemState( EE_PARA_WRITINGDIR, sal_False, &pItem ) == SFX_ITEM_SET ) )
    {
        if ( static_cast< const SvxFrameDirectionItem * >( pItem )->GetValue() == FRMDIR_HORI_LEFT_TOP )
            eWritingMode = WritingMode_LR_TB;
        else
            eWritingMode = WritingMode_RL_TB;
    }

    return eWritingMode;
}

// --------------------------------------------------------------------

// gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
// with the base geometry and returns TRUE. Otherwise it returns FALSE.
sal_Bool SdrTableObj::TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon ) const
{
    return SdrTextObj::TRGetBaseGeometry( rMatrix, rPolyPolygon );
}

// --------------------------------------------------------------------

// sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
// If it's an SdrPathObj it will use the provided geometry information. The Polygon has
// to use (0,0) as upper left and will be scaled to the given size in the matrix.
void SdrTableObj::TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon )
{
    SdrTextObj::TRSetBaseGeometry( rMatrix, rPolyPolygon );
}

// --------------------------------------------------------------------

bool SdrTableObj::IsRealyEdited() const
{
    return pEdtOutl && pEdtOutl->IsModified();
}

// --------------------------------------------------------------------

bool SdrTableObj::IsFontwork() const
{
    return false;
}

// --------------------------------------------------------------------

sal_uInt32 SdrTableObj::GetHdlCount() const
{
    sal_uInt32 nCount = SdrTextObj::GetHdlCount();
    const sal_Int32 nRowCount = mpImpl->getRowCount();
    const sal_Int32 nColCount = mpImpl->getColumnCount();

    if( nRowCount && nColCount )
        nCount += nRowCount + nColCount + 2 + 1;

    return nCount;
}

// --------------------------------------------------------------------

void SdrTableObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    const sal_Int32 nRowCount = mpImpl->getRowCount();
    const sal_Int32 nColCount = mpImpl->getColumnCount();

    // first add row handles
    std::vector< TableEdgeHdl* > aRowEdges( nRowCount + 1 );

    for( sal_Int32 nRow = 0; nRow <= nRowCount; nRow++ )
    {
        sal_Int32 nEdgeMin, nEdgeMax;
        const sal_Int32 nEdge = mpImpl->mpLayouter->getHorizontalEdge( nRow, &nEdgeMin, &nEdgeMax );
        nEdgeMin -= nEdge;
        nEdgeMax -= nEdge;

        Point aPoint( aRect.TopLeft() );
        aPoint.Y() += nEdge;

        TableEdgeHdl* pHdl= new TableEdgeHdl(aPoint,true,nEdgeMin,nEdgeMax,nColCount+1);
        pHdl->SetPointNum( nRow );
        rHdlList.AddHdl( pHdl );
        aRowEdges[nRow] = pHdl;
    }

    // second add column handles
    std::vector< TableEdgeHdl* > aColEdges( nColCount + 1 );

    for( sal_Int32 nCol = 0; nCol <= nColCount; nCol++ )
    {
        sal_Int32 nEdgeMin, nEdgeMax;
        const sal_Int32 nEdge = mpImpl->mpLayouter->getVerticalEdge( nCol, &nEdgeMin, &nEdgeMax );
        nEdgeMin -= nEdge;
        nEdgeMax -= nEdge;

        Point aPoint( aRect.TopLeft() );
        aPoint.X() += nEdge;

        TableEdgeHdl* pHdl = new TableEdgeHdl(aPoint,false,nEdgeMin,nEdgeMax, nRowCount+1);
        pHdl->SetPointNum( nCol );
        rHdlList.AddHdl( pHdl );
        aColEdges[nCol] = pHdl;
    }

    // now add visible edges to row and column handles
    if( mpImpl && mpImpl->mpLayouter )
    {
        TableLayouter& rLayouter = *mpImpl->mpLayouter;

        sal_Int32 nY = 0;

        for( sal_Int32 nRow = 0; nRow <= nRowCount; ++nRow )
        {
            const sal_Int32 nRowHeight = (nRow == nRowCount) ? 0 : rLayouter.getRowHeight(nRow);
            sal_Int32 nX = 0;

            for( sal_Int32 nCol = 0; nCol <= nColCount; ++nCol )
            {
                const sal_Int32 nColWidth = (nCol == nColCount) ? 0 : rLayouter.getColumnWidth(nCol);

                if( nRowHeight > 0 )
                {
                    if( rLayouter.isEdgeVisible( nCol, nRow, false ) )
                        aColEdges[nCol]->SetEdge( nRow, nY, nY + nRowHeight, (rLayouter.getBorderLine( nCol, nRow, false ) == 0) ? Visible : Invisible);
                }

                if( nColWidth > 0 )
                {
                    if( rLayouter.isEdgeVisible( nCol, nRow, true ) )
                        aRowEdges[nRow]->SetEdge( nCol, nX, nX + nColWidth, (rLayouter.getBorderLine( nCol, nRow, true ) == 0) ? Visible : Invisible);
                }

                nX += nColWidth;
            }

            nY += nRowHeight;
        }
    }

    // add remaining handles
    SdrHdl* pH=0;
    rHdlList.AddHdl( pH = new TableBorderHdl( aRect, !IsTextEditActive() ) ); pH->SetMoveOutside( true );
    rHdlList.AddHdl( pH = new SdrHdl(aRect.TopLeft(),HDL_UPLFT) ); pH->SetMoveOutside( true );
    rHdlList.AddHdl( pH = new SdrHdl(aRect.TopCenter(),HDL_UPPER) ); pH->SetMoveOutside( true );
    rHdlList.AddHdl( pH = new SdrHdl(aRect.TopRight(),HDL_UPRGT) ); pH->SetMoveOutside( true );
    rHdlList.AddHdl( pH = new SdrHdl(aRect.LeftCenter(),HDL_LEFT) ); pH->SetMoveOutside( true );
    rHdlList.AddHdl( pH = new SdrHdl(aRect.RightCenter(),HDL_RIGHT) ); pH->SetMoveOutside( true );
    rHdlList.AddHdl( pH = new SdrHdl(aRect.BottomLeft(),HDL_LWLFT) ); pH->SetMoveOutside( true );
    rHdlList.AddHdl( pH = new SdrHdl(aRect.BottomCenter(),HDL_LOWER) ); pH->SetMoveOutside( true );
    rHdlList.AddHdl( pH = new SdrHdl(aRect.BottomRight(),HDL_LWRGT) ); pH->SetMoveOutside( true );

    sal_uIntPtr nHdlCount = rHdlList.GetHdlCount();
    for( sal_uIntPtr nHdl = 0; nHdl < nHdlCount; nHdl++ )
        rHdlList.GetHdl(nHdl)->SetObj((SdrObject*)this);
}

// --------------------------------------------------------------------

SdrHdl* SdrTableObj::GetHdl(sal_uInt32 nHdlNum) const
{
    // #i73248#
    // Warn the user that this is ineffective and show alternatives. Should not be used at all.
    OSL_FAIL("SdrTableObj::GetHdl(): ineffective, use AddToHdlList instead (!)");

    // to have an alternative, get single handle using the ineffective way
    SdrHdl* pRetval = 0;
    SdrHdlList aLocalList(0);
    AddToHdlList(aLocalList);
    const sal_uInt32 nHdlCount(aLocalList.GetHdlCount());

    if(nHdlCount && nHdlNum < nHdlCount)
    {
        // remove and remember. The other created handles will be deleted again with the
        // destruction of the local list
        pRetval = aLocalList.RemoveHdl(nHdlNum);
    }

    return pRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Draging

bool SdrTableObj::hasSpecialDrag() const
{
    return true;
}

bool SdrTableObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl = rDrag.GetHdl();
    const SdrHdlKind eHdl((pHdl == NULL) ? HDL_MOVE : pHdl->GetKind());

    switch( eHdl )
    {
        case HDL_UPLFT:
        case HDL_UPPER:
        case HDL_UPRGT:
        case HDL_LEFT:
        case HDL_RIGHT:
        case HDL_LWLFT:
        case HDL_LOWER:
        case HDL_LWRGT:
        case HDL_MOVE:
        {
            break;
        }

        case HDL_USER:
        {
            rDrag.SetEndDragChangesAttributes(false);
            rDrag.SetNoSnap(true);
            break;
        }

        default:
        {
            return false;
        }
    }

    return true;
}

bool SdrTableObj::applySpecialDrag(SdrDragStat& rDrag)
{
    bool bRet(true);
    const SdrHdl* pHdl = rDrag.GetHdl();
    const SdrHdlKind eHdl((pHdl == NULL) ? HDL_MOVE : pHdl->GetKind());

    switch( eHdl )
    {
        case HDL_UPLFT:
        case HDL_UPPER:
        case HDL_UPRGT:
        case HDL_LEFT:
        case HDL_RIGHT:
        case HDL_LWLFT:
        case HDL_LOWER:
        case HDL_LWRGT:
        {
            const Rectangle aNewRectangle(ImpDragCalcRect(rDrag));

            if(aNewRectangle != aRect)
            {
                   NbcSetLogicRect(aNewRectangle);
            }

            break;
        }

        case HDL_MOVE:
        {
               NbcMove( Size( rDrag.GetDX(), rDrag.GetDY() ) );
            break;
        }

        case HDL_USER:
        {
            rDrag.SetEndDragChangesAttributes(false);
            rDrag.SetNoSnap(true);
            const TableEdgeHdl* pEdgeHdl = dynamic_cast< const TableEdgeHdl* >( pHdl );

            if( pEdgeHdl )
            {
                if( GetModel() && IsInserted() )
                {
                    rDrag.SetEndDragChangesAttributes(true);
                }

                mpImpl->DragEdge( pEdgeHdl->IsHorizontalEdge(), pEdgeHdl->GetPointNum(), pEdgeHdl->GetValidDragOffset( rDrag ) );
            }
            break;
        }

        default:
        {
            bRet = false;
        }
    }

    return bRet;
}

OUString SdrTableObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    return SdrTextObj::getSpecialDragComment( rDrag );
}

basegfx::B2DPolyPolygon SdrTableObj::getSpecialDragPoly(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aRetval;
    const SdrHdl* pHdl = rDrag.GetHdl();

    if( pHdl && (HDL_USER == pHdl->GetKind()) )
    {
        const TableEdgeHdl* pEdgeHdl = dynamic_cast< const TableEdgeHdl* >( pHdl );

        if( pEdgeHdl )
        {
            aRetval = pEdgeHdl->getSpecialDragPoly( rDrag );
        }
    }

    return aRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Create
// --------------------------------------------------------------------

bool SdrTableObj::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho4Possible();
    Rectangle aRect1(rStat.GetStart(), rStat.GetNow());
    aRect1.Justify();
    rStat.SetActionRect(aRect1);
    aRect = aRect1;
    return true;
}

// --------------------------------------------------------------------

bool SdrTableObj::MovCreate(SdrDragStat& rStat)
{
    Rectangle aRect1;
    rStat.TakeCreateRect(aRect1);
    ImpJustifyRect(aRect1);
    rStat.SetActionRect(aRect1);
    aRect=aRect1; // fuer ObjName
    SetBoundRectDirty();
    bSnapRectDirty=sal_True;
    return true;
}

// --------------------------------------------------------------------

bool SdrTableObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    rStat.TakeCreateRect(aRect);
    ImpJustifyRect(aRect);
    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
}

void SdrTableObj::BrkCreate(SdrDragStat& /*rStat*/)
{
}

// --------------------------------------------------------------------

bool SdrTableObj::BckCreate(SdrDragStat& /*rStat*/)
{
    return true;
}

// --------------------------------------------------------------------

basegfx::B2DPolyPolygon SdrTableObj::TakeCreatePoly(const SdrDragStat& rDrag) const
{
    Rectangle aRect1;
    rDrag.TakeCreateRect(aRect1);
    aRect1.Justify();

    basegfx::B2DPolyPolygon aRetval;
    const basegfx::B2DRange aRange(aRect1.Left(), aRect1.Top(), aRect1.Right(), aRect1.Bottom());
    aRetval.append(basegfx::tools::createPolygonFromRect(aRange));
    return aRetval;
}

// --------------------------------------------------------------------

Pointer SdrTableObj::GetCreatePointer() const
{
    return Pointer(POINTER_CROSS);
}

// --------------------------------------------------------------------

void SdrTableObj::createCell( CellRef& xNewCell )
{
    xNewCell = Cell::create( *this, 0 );
}

// --------------------------------------------------------------------

SdrObjGeoData *SdrTableObj::NewGeoData() const
{
    return new TableObjectGeoData;
}

// --------------------------------------------------------------------

void SdrTableObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    DBG_ASSERT( dynamic_cast< TableObjectGeoData* >( &rGeo ), "svx::SdrTableObj::SaveGeoData(), illegal geo data!" );
    SdrTextObj::SaveGeoData (rGeo);

    ((TableObjectGeoData &) rGeo).maLogicRect = maLogicRect;
}

// --------------------------------------------------------------------

void SdrTableObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    DBG_ASSERT( dynamic_cast< const TableObjectGeoData* >( &rGeo ), "svx::SdrTableObj::SaveGeoData(), illegal geo data!" );

    maLogicRect = ((TableObjectGeoData &) rGeo).maLogicRect;

    SdrTextObj::RestGeoData (rGeo);

    if( mpImpl )
        mpImpl->LayoutTable( aRect, false, false );
    ActionChanged();
}

// --------------------------------------------------------------------

SdrTableObj* SdrTableObj::CloneRange( const CellPos& rStart, const CellPos& rEnd )
{
    const sal_Int32 nColumns = rEnd.mnCol - rStart.mnCol + 1;
    const sal_Int32 nRows = rEnd.mnRow - rStart.mnRow + 1;

    SdrTableObj* pNewTableObj = new SdrTableObj( GetModel(), GetCurrentBoundRect(), nColumns, nRows);
    pNewTableObj->setTableStyleSettings( getTableStyleSettings() );
    pNewTableObj->setTableStyle( getTableStyle() );

    Reference< XTable > xTable( getTable() );
    Reference< XTable > xNewTable( pNewTableObj->getTable() );

    if( !xTable.is() || !xNewTable.is() )
    {
        delete pNewTableObj;
        return 0;
    }

    // copy cells
    for( sal_Int32 nRow = 0; nRow < nRows; ++nRow )
    {
        for( sal_Int32 nCol = 0; nCol < nColumns; ++nCol ) try
        {
            CellRef xTargetCell( dynamic_cast< Cell* >( xNewTable->getCellByPosition( nCol, nRow ).get() ) );
            if( xTargetCell.is() )
                xTargetCell->cloneFrom( dynamic_cast< Cell* >( xTable->getCellByPosition( rStart.mnCol + nCol, rStart.mnRow + nRow ).get() ) );
        }
        catch( Exception& )
        {
            OSL_FAIL( "svx::SvxTableController::GetMarkedObjModel(), exception caught!" );
        }
    }

    // copy row heights
    Reference< XTableRows > xNewRows( xNewTable->getRows(), UNO_QUERY_THROW );
    const OUString sHeight( "Height" );
    for( sal_Int32 nRow = 0; nRow < nRows; ++nRow )
    {
        Reference< XPropertySet > xNewSet( xNewRows->getByIndex( nRow ), UNO_QUERY_THROW );
        xNewSet->setPropertyValue( sHeight, Any( mpImpl->mpLayouter->getRowHeight( rStart.mnRow + nRow ) ) );
    }

    // copy column widths
    Reference< XTableColumns > xNewColumns( xNewTable->getColumns(), UNO_QUERY_THROW );
    const OUString sWidth( "Width" );
    for( sal_Int32 nCol = 0; nCol < nColumns; ++nCol )
    {
        Reference< XPropertySet > xNewSet( xNewColumns->getByIndex( nCol ), UNO_QUERY_THROW );
        xNewSet->setPropertyValue( sWidth, Any( mpImpl->mpLayouter->getColumnWidth( rStart.mnCol + nCol ) ) );
    }

    pNewTableObj->NbcReformatText();
    pNewTableObj->SetLogicRect( pNewTableObj->GetCurrentBoundRect() );

    return pNewTableObj;
}

// --------------------------------------------------------------------

void SdrTableObj::DistributeColumns( sal_Int32 nFirstColumn, sal_Int32 nLastColumn )
{
    if( mpImpl && mpImpl->mpLayouter )
    {
        TableModelNotifyGuard aGuard( mpImpl->mxTable.get() );
        mpImpl->mpLayouter->DistributeColumns( aRect, nFirstColumn, nLastColumn );
    }
}

// --------------------------------------------------------------------

void SdrTableObj::DistributeRows( sal_Int32 nFirstRow, sal_Int32 nLastRow )
{
    if( mpImpl && mpImpl->mpLayouter )
    {
        TableModelNotifyGuard aGuard( mpImpl->mxTable.get() );
        mpImpl->mpLayouter->DistributeRows( aRect, nFirstRow, nLastRow );
    }
}

// --------------------------------------------------------------------

void SdrTableObj::SetChanged()
{
    if( mpImpl )
    {
        mpImpl->LayoutTable( aRect, false, false );
    }

    ::SdrTextObj::SetChanged();
}

// --------------------------------------------------------------------

void SdrTableObj::uno_lock()
{
    if( mpImpl && mpImpl->mxTable.is() )
        mpImpl->mxTable->lockBroadcasts();
}

// --------------------------------------------------------------------

void SdrTableObj::uno_unlock()
{
    if( mpImpl && mpImpl->mxTable.is() )
        mpImpl->mxTable->unlockBroadcasts();
}

// --------------------------------------------------------------------



} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
