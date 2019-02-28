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

#include <memory>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <vcl/canvastools.hxx>
#include <vcl/ptrstyle.hxx>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svl/style.hxx>
#include <editeng/editstat.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdview.hxx>
#include <sdr/properties/textproperties.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdhdl.hxx>
#include "viewcontactoftableobj.hxx"
#include <svx/svdoutl.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdpagv.hxx>
#include <tablemodel.hxx>
#include <cell.hxx>
#include <svx/xflclit.hxx>
#include "tablelayouter.hxx"
#include <svx/svdetc.hxx>
#include "tablehandles.hxx"
#include <editeng/boxitem.hxx>
#include <svx/framelink.hxx>
#include <svx/sdr/table/tabledesign.hxx>
#include <svx/svdundo.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <editeng/writingmodeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xfltrit.hxx>
#include <cppuhelper/implbase.hxx>
#include <libxml/xmlwriter.h>


using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
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
using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;

namespace sdr { namespace table {

class TableProperties : public TextProperties
{
protected:
    // create a new itemset
    std::unique_ptr<SfxItemSet> CreateObjectSpecificItemSet(SfxItemPool& rPool) override;

public:
    // basic constructor
    explicit TableProperties(SdrObject& rObj );

    // constructor for copying, but using new object
    TableProperties(const TableProperties& rProps, SdrObject& rObj );

    // Clone() operator, normally just calls the local copy constructor
    std::unique_ptr<BaseProperties> Clone(SdrObject& rObj) const override;

    virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = nullptr) override;
};

TableProperties::TableProperties(SdrObject& rObj)
: TextProperties(rObj)
{
}

TableProperties::TableProperties(const TableProperties& rProps, SdrObject& rObj)
: TextProperties(rProps, rObj)
{
}

std::unique_ptr<BaseProperties> TableProperties::Clone(SdrObject& rObj) const
{
    return std::unique_ptr<BaseProperties>(new TableProperties(*this, rObj));
}

void TableProperties::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
{
    if( nWhich == SDRATTR_TEXTDIRECTION )
        AttributeProperties::ItemChange( nWhich, pNewItem );
    else
        TextProperties::ItemChange( nWhich, pNewItem );
}

// create a new itemset
std::unique_ptr<SfxItemSet> TableProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
{
    return std::make_unique<SfxItemSet>(rPool,

        // range from SdrAttrObj
        svl::Items<SDRATTR_START, SDRATTR_SHADOW_LAST,
        SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
        SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

        // range for SdrTableObj
        SDRATTR_TABLE_FIRST, SDRATTR_TABLE_LAST,

        // range from SdrTextObj
        EE_ITEMS_START, EE_ITEMS_END>{});
}

class TableObjectGeoData : public SdrTextObjGeoData
{
public:
    tools::Rectangle   maLogicRect;
};

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


class SdrTableObjImpl : public TableDesignUser, public ::cppu::WeakImplHelper< css::util::XModifyListener >
{
public:
    CellRef mxActiveCell;
    TableModelRef mxTable;
    SdrTableObj* mpTableObj;
    std::unique_ptr<TableLayouter> mpLayouter;
    CellPos maEditPos;
    TableStyleSettings maTableStyle;
    Reference< XIndexAccess > mxTableStyle;
    std::vector<std::unique_ptr<SdrUndoAction>> maUndos;
    bool mbSkipChangeLayout;

    void CropTableModelToSelection(const CellPos& rStart, const CellPos& rEnd);

    CellRef getCell( const CellPos& rPos ) const;
    void LayoutTable( tools::Rectangle& rArea, bool bFitWidth, bool bFitHeight );

    void ApplyCellStyles();
    void UpdateCells( tools::Rectangle const & rArea );

    SdrTableObjImpl();
    virtual ~SdrTableObjImpl() override;

    void init( SdrTableObj* pTable, sal_Int32 nColumns, sal_Int32 nRows );
    void dispose();

    sal_Int32 getColumnCount() const;
    /// Get widths of the columns in the table.
    std::vector<sal_Int32> getColumnWidths() const;
    sal_Int32 getRowCount() const;

    void DragEdge( bool mbHorizontal, int nEdge, sal_Int32 nOffset );

    SdrTableObjImpl& operator=( const SdrTableObjImpl& rSource );

    // XModifyListener
    virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    void update();

    void connectTableStyle();
    void disconnectTableStyle();
    virtual bool isInUse() override;
    void dumpAsXml(xmlTextWriterPtr pWriter) const;
private:
    static SdrTableObjImpl* lastLayoutTable;
    static tools::Rectangle lastLayoutInputRectangle;
    static tools::Rectangle lastLayoutResultRectangle;
    static bool lastLayoutFitWidth;
    static bool lastLayoutFitHeight;
    static WritingMode lastLayoutMode;
    static sal_Int32 lastRowCount;
    static sal_Int32 lastColCount;
    static std::vector<sal_Int32> lastColWidths;
};

SdrTableObjImpl* SdrTableObjImpl::lastLayoutTable = nullptr;
tools::Rectangle SdrTableObjImpl::lastLayoutInputRectangle;
tools::Rectangle SdrTableObjImpl::lastLayoutResultRectangle;
bool SdrTableObjImpl::lastLayoutFitWidth;
bool SdrTableObjImpl::lastLayoutFitHeight;
WritingMode SdrTableObjImpl::lastLayoutMode;
sal_Int32 SdrTableObjImpl::lastRowCount;
sal_Int32 SdrTableObjImpl::lastColCount;
std::vector<sal_Int32> SdrTableObjImpl::lastColWidths;

SdrTableObjImpl::SdrTableObjImpl()
: mpTableObj( nullptr )
, mbSkipChangeLayout(false)
{
}


SdrTableObjImpl::~SdrTableObjImpl()
{
    if( lastLayoutTable == this )
        lastLayoutTable = nullptr;
}


void SdrTableObjImpl::CropTableModelToSelection(const CellPos& rStart, const CellPos& rEnd)
{
    if(!mxTable.is())
    {
        return;
    }

    const sal_Int32 nColumns(rEnd.mnCol - rStart.mnCol + 1);
    const sal_Int32 nRows(rEnd.mnRow - rStart.mnRow + 1);

    if(nColumns < 1 || nRows < 1 || nColumns > getColumnCount() || nRows > getRowCount())
    {
        return;
    }

    // tdf#116977 First thought was to create the new TableModel, copy data to it and then exchange
    // mxTable and dispose old one. This does *not* work, even when all stuff looks nicely referenced
    // and safe *because* Cell::create gets handed over the current SdrTableObj, hands it to
    // ::Cell and there the local mxTable is initialized using rTableObj.getTable() (!). Due to This,
    // the new created Cells in a new created TableModel based on given mpTableObj *will be disposed*
    // when the old mxTable gets disposed - ARGH!
    // To avoid, change strategy: Remember old TableModel, reset mxTable immediately - this is the
    // SdrTableObjImpl of the current SdrTableObj anyways. Luckily, this works as intended...

    // remember old TableModel
    TableModelRef xOldTable(mxTable);

    // immediately create new one and initialize. This creates ::Cell's which then will use
    // the correct TableModel (accessed through SdrTableObj, but using local mxTable)
    mxTable = new TableModel(mpTableObj);
    mxTable->init(nColumns, nRows);

    // copy cells
    for( sal_Int32 nRow = 0; nRow < nRows; ++nRow )
    {
        for( sal_Int32 nCol = 0; nCol < nColumns; ++nCol ) try
        {
            CellRef xTargetCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
            if( xTargetCell.is() )
                xTargetCell->cloneFrom( dynamic_cast< Cell* >( xOldTable->getCellByPosition( rStart.mnCol + nCol, rStart.mnRow + nRow ).get() ) );
        }
        catch( Exception& )
        {
            OSL_FAIL( "SdrTableObj::CropTableModelToSelection(), exception caught!" );
        }
    }

    // copy row heights
    Reference< XTableRows > xNewRows(mxTable->getRows(), UNO_QUERY_THROW );
    const OUString sHeight( "Height" );
    for( sal_Int32 nRow = 0; nRow < nRows; ++nRow )
    {
        Reference< XPropertySet > xNewSet( xNewRows->getByIndex( nRow ), UNO_QUERY_THROW );
        xNewSet->setPropertyValue( sHeight, Any( mpLayouter->getRowHeight( rStart.mnRow + nRow ) ) );
    }

    // copy column widths
    Reference< XTableColumns > xNewColumns( mxTable->getColumns(), UNO_QUERY_THROW );
    const OUString sWidth( "Width" );
    for( sal_Int32 nCol = 0; nCol < nColumns; ++nCol )
    {
        Reference< XPropertySet > xNewSet( xNewColumns->getByIndex( nCol ), UNO_QUERY_THROW );
        xNewSet->setPropertyValue( sWidth, Any( mpLayouter->getColumnWidth( rStart.mnCol + nCol ) ) );
    }

    // reset layouter which still holds a copy to old TableModel
    mpLayouter.reset();

    // cleanup old TableModel
    {
        Reference< XModifyListener > xListener( static_cast< css::util::XModifyListener* >(this) );
        xOldTable->removeModifyListener( xListener );
        xOldTable->dispose();
        xOldTable.clear();
    }

    // create and hand over to new TableLayouter
    mpLayouter.reset(new TableLayouter( mxTable ));

    // add needed listener to react on changes
    Reference< XModifyListener > xListener( static_cast< css::util::XModifyListener* >(this) );
    mxTable->addModifyListener( xListener );

    // Apply Style to Cells
    ApplyCellStyles();

    // layout cropped table
    LayoutTable( mpTableObj->maRect, false, false );
}

void SdrTableObjImpl::init( SdrTableObj* pTable, sal_Int32 nColumns, sal_Int32 nRows )
{
    mpTableObj = pTable;
    mxTable = new TableModel( pTable );
    mxTable->init( nColumns, nRows );
    Reference< XModifyListener > xListener( static_cast< css::util::XModifyListener* >(this) );
    mxTable->addModifyListener( xListener );
    mpLayouter.reset(new TableLayouter( mxTable ));
    LayoutTable( mpTableObj->maRect, true, true );
    mpTableObj->maLogicRect = mpTableObj->maRect;
}


SdrTableObjImpl& SdrTableObjImpl::operator=( const SdrTableObjImpl& rSource )
{
    if(this == &rSource)
    {
        return *this;
    }

    if(nullptr == mpTableObj || nullptr == rSource.mpTableObj)
    {
        // error: need both SdrObjects to successfully copy data
        return *this;
    }

    // remove evtl. listeners from local
    disconnectTableStyle();

    // reset layouter which holds a copy
    mpLayouter.reset();

    // cleanup local mxTable if used
    if( mxTable.is() )
    {
        Reference< XModifyListener > xListener( static_cast< css::util::XModifyListener* >(this) );
        mxTable->removeModifyListener( xListener );
        mxTable->dispose();
        mxTable.clear();
    }

    // copy TableStyle (short internal data)
    maTableStyle = rSource.maTableStyle;

    // create/copy new mxTable. This will copy all needed cells, too
    mxTable = new TableModel( mpTableObj, rSource.mxTable );

    // create and hand over to new TableLayouter
    mpLayouter.reset(new TableLayouter( mxTable ));

    // add needed listener to react on changes
    Reference< XModifyListener > xListener( static_cast< css::util::XModifyListener* >(this) );
    mxTable->addModifyListener( xListener );

    // handle TableStyle
    Reference< XIndexAccess > xNewTableStyle;
    SdrModel& rSourceSdrModel(rSource.mpTableObj->getSdrModelFromSdrObject());
    SdrModel& rTargetSdrModel(mpTableObj->getSdrModelFromSdrObject());

    if(rSource.mxTableStyle.is() && &rSourceSdrModel == &rTargetSdrModel)
    {
        // source and target model the same -> keep current TableStyle
        xNewTableStyle = rSource.mxTableStyle;
    }

    if(!xNewTableStyle.is() && rSource.mxTableStyle.is()) try
    {
        // search in traget SdrModel for that TableStyle
        const OUString sStyleName( Reference< XNamed >( rSource.mxTableStyle, UNO_QUERY_THROW )->getName() );
        Reference< XStyleFamiliesSupplier > xSFS(rTargetSdrModel.getUnoModel(), UNO_QUERY_THROW );
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
            // copy or? Not found, use 1st existing TableStyle (or none)
            Reference< XIndexAccess > xIndexAccess( xTableFamilyAccess, UNO_QUERY_THROW );
            xIndexAccess->getByIndex( 0 ) >>= xNewTableStyle;
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("svx::SdrTableObjImpl::operator=(), exception caught!");
    }

    // set that TableStyle
    mxTableStyle = xNewTableStyle;

    // Apply Style to Cells
    ApplyCellStyles();

    // copy geometry
    mpTableObj->maRect = mpTableObj->maLogicRect;

    // layout cloned table
    LayoutTable( mpTableObj->maRect, false, false );

    // re-connect to styles (evtl. in new SdrModel)
    connectTableStyle();

    return *this;
}

void SdrTableObjImpl::ApplyCellStyles()
{
    if( !mxTable.is() || !mxTableStyle.is() )
        return;

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
                        xCell->SetStyleSheet( pStyle, true );
                    }
                }
            }
        }
    }
}


void SdrTableObjImpl::dispose()
{
    disconnectTableStyle();
    mxTableStyle.clear();

    mpLayouter.reset();

    if( mxTable.is() )
    {
        Reference< XModifyListener > xListener( static_cast< css::util::XModifyListener* >(this) );
        mxTable->removeModifyListener( xListener );
        mxTable->dispose();
        mxTable.clear();
    }
}


void SdrTableObjImpl::DragEdge( bool mbHorizontal, int nEdge, sal_Int32 nOffset )
{
    if( (nEdge >= 0) && mxTable.is()) try
    {
        const OUString sSize( "Size" );
        if( mbHorizontal )
        {
            if (nEdge <= getRowCount())
            {
                sal_Int32 nHeight = mpLayouter->getRowHeight( (!nEdge)?nEdge:(nEdge-1) );
                if(nEdge==0)
                    nHeight -= nOffset;
                else
                    nHeight += nOffset;
                Reference< XIndexAccess > xRows( mxTable->getRows(), UNO_QUERY_THROW );
                Reference< XPropertySet > xRowSet( xRows->getByIndex( (!nEdge)?nEdge:(nEdge-1) ), UNO_QUERY_THROW );
                xRowSet->setPropertyValue( sSize, Any( nHeight ) );
            }
        }
        else
        {
            /*
            fixes fdo#59889 and resizing of table in edge dragging
            Total vertical edges in a NxN table is N+1, indexed from 0 to N and total Columns is N, indexed from 0 to N-1
            In LTR table vertical edge responsible for dragging of column x(x=0 to N-1) is, Edge x+1
            But in RTL table vertical edge responsible for dragging of column x(x=0 to N-1, but from right to left)is, Edge x
            In LTR table dragging of edge 0(for RTL table edge N) does nothing.
            */
            //Todo: Implement Dragging functionality for leftmost edge of table.
            if (nEdge <= getColumnCount())
            {
                const bool bRTL = mpTableObj != nullptr && (mpTableObj->GetWritingMode() == WritingMode_RL_TB);
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
                    Reference< XPropertySet > xColSet( xCols->getByIndex( nEdge-1 ), UNO_QUERY_THROW );
                    xColSet->setPropertyValue( sSize, Any( nWidth ) );
                }
                /* To prevent the table resizing on edge dragging */
                if( nEdge > 0 && nEdge < mxTable->getColumnCount() )
                {
                    if( bRTL )
                        nEdge--;

                    nWidth = mpLayouter->getColumnWidth(nEdge);
                    nWidth = std::max(static_cast<sal_Int32>(nWidth - nOffset), sal_Int32(0));

                    Reference<XPropertySet> xColSet(xCols->getByIndex(nEdge), UNO_QUERY_THROW);
                    xColSet->setPropertyValue(sSize, Any(nWidth));
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "svx::SdrTableObjImpl::DragEdge(), exception caught!" );
    }
}


// XModifyListener


void SAL_CALL SdrTableObjImpl::modified( const css::lang::EventObject& /*aEvent*/ )
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

        mpTableObj->maRect = mpTableObj->maLogicRect;
        LayoutTable( mpTableObj->maRect, false, false );

        mpTableObj->SetRectsDirty();
        mpTableObj->ActionChanged();
        mpTableObj->BroadcastObjectChange();
    }
}


void SdrTableObjImpl::connectTableStyle()
{
    if( mxTableStyle.is() )
    {
        Reference< XModifyBroadcaster > xBroadcaster( mxTableStyle, UNO_QUERY );
        if( xBroadcaster.is() )
        {
            Reference< XModifyListener > xListener( static_cast< css::util::XModifyListener* >(this) );
            xBroadcaster->addModifyListener( xListener );
        }
    }
}


void SdrTableObjImpl::disconnectTableStyle()
{
    if( mxTableStyle.is() )
    {
        Reference< XModifyBroadcaster > xBroadcaster( mxTableStyle, UNO_QUERY );
        if( xBroadcaster.is() )
        {
            Reference< XModifyListener > xListener( static_cast< css::util::XModifyListener* >(this) );
            xBroadcaster->removeModifyListener( xListener );
        }
    }
}


bool SdrTableObjImpl::isInUse()
{
    return mpTableObj && mpTableObj->IsInserted();
}

void SdrTableObjImpl::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SdrTableObjImpl"));
    if (mpLayouter)
        mpLayouter->dumpAsXml(pWriter);
    mxTable->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}


// XEventListener


void SAL_CALL SdrTableObjImpl::disposing( const css::lang::EventObject& /*Source*/ )
{
    mxActiveCell.clear();
    mxTable.clear();
    mpLayouter.reset();
    mpTableObj = nullptr;
}


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


sal_Int32 SdrTableObjImpl::getColumnCount() const
{
    return mxTable.is() ? mxTable->getColumnCount() : 0;
}

std::vector<sal_Int32> SdrTableObjImpl::getColumnWidths() const
{
    std::vector<sal_Int32> aRet;

    if (mxTable.is())
        aRet = mxTable->getColumnWidths();

    return aRet;
}

sal_Int32 SdrTableObjImpl::getRowCount() const
{
    return mxTable.is() ? mxTable->getRowCount() : 0;
}

void SdrTableObjImpl::LayoutTable( tools::Rectangle& rArea, bool bFitWidth, bool bFitHeight )
{
    if(mpLayouter)
    {
        // Optimization: SdrTableObj::SetChanged() can call this very often, repeatedly
        // with the same settings, noticeably increasing load time. Skip if already done.
        bool bInteractiveMightGrowBecauseTextChanged =
            mpTableObj->IsReallyEdited() && (mpTableObj->IsAutoGrowHeight() || mpTableObj->IsAutoGrowWidth());
        WritingMode writingMode = mpTableObj->GetWritingMode();
        if( bInteractiveMightGrowBecauseTextChanged
            || lastLayoutTable != this || lastLayoutInputRectangle != rArea
            || lastLayoutFitWidth != bFitWidth || lastLayoutFitHeight != bFitHeight
            || lastLayoutMode != writingMode
            || lastRowCount != getRowCount()
            || lastColCount != getColumnCount()
            || lastColWidths != getColumnWidths() )
        {
            lastLayoutTable = this;
            lastLayoutInputRectangle = rArea;
            lastLayoutFitWidth = bFitWidth;
            lastLayoutFitHeight = bFitHeight;
            lastLayoutMode = writingMode;
            lastRowCount = getRowCount();
            lastColCount = getColumnCount();
            // Column resize, when the total width and column count of the
            // table is unchanged, but re-layout is still needed.
            lastColWidths = getColumnWidths();
            TableModelNotifyGuard aGuard( mxTable.get() );
            mpLayouter->LayoutTable( rArea, bFitWidth, bFitHeight );
            lastLayoutResultRectangle = rArea;
        }
        else
        {
            rArea = lastLayoutResultRectangle;
            mpLayouter->UpdateBorderLayout();
        }
    }
}

void SdrTableObjImpl::UpdateCells( tools::Rectangle const & rArea )
{
    if( mpLayouter && mxTable.is() )
    {
        TableModelNotifyGuard aGuard( mxTable.get() );
        mpLayouter->updateCells( rArea );
        mxTable->setModified(true);
    }
}


// BaseProperties section


std::unique_ptr<sdr::properties::BaseProperties> SdrTableObj::CreateObjectSpecificProperties()
{
    return std::make_unique<TableProperties>(*this);
}


// DrawContact section


std::unique_ptr<sdr::contact::ViewContact> SdrTableObj::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfTableObj>(*this);
}

SdrTableObj::SdrTableObj(SdrModel& rSdrModel)
:   SdrTextObj(rSdrModel)
{
    init( 1, 1 );
}

SdrTableObj::SdrTableObj(
    SdrModel& rSdrModel,
    const ::tools::Rectangle& rNewRect,
    sal_Int32 nColumns,
    sal_Int32 nRows)
:   SdrTextObj(rSdrModel, rNewRect)
    ,maLogicRect(rNewRect)
{
    if( nColumns <= 0 )
        nColumns = 1;

    if( nRows <= 0 )
        nRows = 1;

    init( nColumns, nRows );
}


void SdrTableObj::init( sal_Int32 nColumns, sal_Int32 nRows )
{
    bClosedObj = true;

    mpImpl = new SdrTableObjImpl;
    mpImpl->init( this, nColumns, nRows );

    // Stuff done from old SetModel:
    if( !maLogicRect.IsEmpty() )
    {
        maRect = maLogicRect;
        mpImpl->LayoutTable( maRect, false, false );
    }
}


SdrTableObj::~SdrTableObj()
{
    mpImpl->dispose();
}


// table stuff


Reference< XTable > SdrTableObj::getTable() const
{
    return Reference< XTable >( mpImpl->mxTable.get() );
}


bool SdrTableObj::isValid( const CellPos& rPos ) const
{
    return (rPos.mnCol >= 0) && (rPos.mnCol < mpImpl->getColumnCount()) && (rPos.mnRow >= 0) && (rPos.mnRow < mpImpl->getRowCount());
}


CellPos SdrTableObj::getFirstCell()
{
    return CellPos( 0,0 );
}


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


CellPos SdrTableObj::getPreviousCell( const CellPos& rPos, bool bEdgeTravel ) const
{
    CellPos aPos( rPos );
    if( mpImpl.is() )
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


CellPos SdrTableObj::getNextCell( const CellPos& rPos, bool bEdgeTravel ) const
{
    CellPos aPos( rPos );
    if( mpImpl.is() )
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


CellPos SdrTableObj::getPreviousRow( const CellPos& rPos, bool bEdgeTravel ) const
{
    CellPos aPos( rPos );
    if( mpImpl.is() )
    {
        CellRef xCell( mpImpl->getCell( aPos ) );
        if( xCell.is() && xCell->isMerged() )
        {
            sal_Int32 nTemp = 0;
            findMergeOrigin( mpImpl->mxTable, aPos.mnCol, aPos.mnRow, nTemp, aPos.mnRow );
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


CellPos SdrTableObj::getNextRow( const CellPos& rPos, bool bEdgeTravel ) const
{
    CellPos aPos( rPos );

    if( mpImpl.is() )
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


const TableStyleSettings& SdrTableObj::getTableStyleSettings() const
{
    if( mpImpl.is())
    {
        return mpImpl->maTableStyle;
    }
    else
    {
        static TableStyleSettings aTmp;
        return aTmp;
    }
}


void SdrTableObj::setTableStyleSettings( const TableStyleSettings& rStyle )
{
    if( mpImpl.is() )
    {
        mpImpl->maTableStyle = rStyle;
        mpImpl->update();
    }
}


TableHitKind SdrTableObj::CheckTableHit( const Point& rPos, sal_Int32& rnX, sal_Int32& rnY, const sal_uInt16 aTol ) const
{
    if( !mpImpl.is() || !mpImpl->mxTable.is() )
        return TableHitKind::NONE;

    rnX = 0;
    rnY = 0;

    const sal_Int32 nColCount = mpImpl->getColumnCount();
    const sal_Int32 nRowCount = mpImpl->getRowCount();

    sal_Int32 nX = rPos.X() - maRect.Left();
    sal_Int32 nY = rPos.Y() - maRect.Top();

    if( (nX < 0) || (nX > maRect.GetWidth()) || (nY < 0) || (nY > maRect.GetHeight() ) )
        return TableHitKind::NONE;

    // get vertical edge number and check for a hit
    const bool bRTL = (GetWritingMode() == WritingMode_RL_TB);
    bool bVrtHit = false;
    if( !bRTL )
    {
        while( rnX <= nColCount )
        {
            if( nX - aTol <= 0 )
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
            if( nX - aTol <= 0 )
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

    // rnX is now the edge number left to the pointer, if it was hit bHrzHit is also true

    // get vertical edge number and check for a hit
    bool bHrzHit = false;
    while( rnY <= nRowCount )
    {
        if( nY - aTol <= 0 )
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

    // rnY is now the edge number above the pointer, if it was hit bVrtHit is also true

    if( bVrtHit && mpImpl->mpLayouter->isEdgeVisible( rnX, rnY, false ) )
        return TableHitKind::VerticallBorder;

    if( bHrzHit && mpImpl->mpLayouter->isEdgeVisible( rnX, rnY, true ) )
        return TableHitKind::HorizontalBorder;

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
            return TableHitKind::Cell;
    }

    return TableHitKind::CellTextArea;
}

const SfxItemSet& SdrTableObj::GetActiveCellItemSet() const
{
    return getActiveCell()->GetItemSet();
}

void SdrTableObj::setTableStyle( const Reference< XIndexAccess >& xTableStyle )
{
    if( mpImpl.is() && (mpImpl->mxTableStyle != xTableStyle) )
    {
        mpImpl->disconnectTableStyle();
        mpImpl->mxTableStyle = xTableStyle;
        mpImpl->connectTableStyle();
        mpImpl->update();
    }
}


const Reference< XIndexAccess >& SdrTableObj::getTableStyle() const
{
    if( mpImpl.is() )
    {
        return mpImpl->mxTableStyle;
    }
    else
    {
        static Reference< XIndexAccess > aTmp;
        return aTmp;
    }
}


// text stuff


/** returns the currently active text. */
SdrText* SdrTableObj::getActiveText() const
{
    return dynamic_cast< SdrText* >( getActiveCell().get() );
}


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
    return nullptr;
}


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


/** changes the current active text */
void SdrTableObj::setActiveText( sal_Int32 nIndex )
{
    if( mpImpl.is() && mpImpl->mxTable.is() )
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


/** returns the index of the text that contains the given point or -1 */
sal_Int32 SdrTableObj::CheckTextHit(const Point& rPnt) const
{
    if( mpImpl.is() && mpImpl->mxTable.is() )
    {
        CellPos aPos;
        if( CheckTableHit( rPnt, aPos.mnCol, aPos.mnRow ) == TableHitKind::CellTextArea )
            return aPos.mnRow * mpImpl->mxTable->getColumnCount() + aPos.mnCol;
    }

    return 0;
}

SdrOutliner* SdrTableObj::GetCellTextEditOutliner( const Cell& rCell ) const
{
    if( mpImpl.is() && (mpImpl->getCell( mpImpl->maEditPos ).get() == &rCell) )
        return pEdtOutl;
    else
        return nullptr;
}

const TableLayouter& SdrTableObj::getTableLayouter() const
{
    assert(mpImpl.is() && mpImpl->mpLayouter && "getTableLayouter() error: no mpImpl or mpLayouter (!)");
    return *(mpImpl->mpLayouter);
}

bool SdrTableObj::IsAutoGrowHeight() const
{
    return true;
}

bool SdrTableObj::IsAutoGrowWidth() const
{
    return true;
}

bool SdrTableObj::HasText() const
{
    return true;
}

bool SdrTableObj::IsTextEditActive( const CellPos& rPos )
{
    return pEdtOutl && mpImpl.is() && (rPos == mpImpl->maEditPos);
}


void SdrTableObj::onEditOutlinerStatusEvent( EditStatus* pEditStatus )
{
    if( (pEditStatus->GetStatusWord() & EditStatusFlags::TextHeightChanged) && mpImpl.is() && mpImpl->mpLayouter )
    {
        tools::Rectangle aRect0( maRect );
        maRect = maLogicRect;
        mpImpl->LayoutTable( maRect, false, false );
        SetRectsDirty();
        ActionChanged();
        BroadcastObjectChange();
        if (aRect0 != maRect)
            SendUserCall(SdrUserCallType::Resize,aRect0);
    }
}


void SdrTableObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bResizeFreeAllowed=true;
    rInfo.bResizePropAllowed=true;
    rInfo.bRotateFreeAllowed=false;
    rInfo.bRotate90Allowed  =false;
    rInfo.bMirrorFreeAllowed=false;
    rInfo.bMirror45Allowed  =false;
    rInfo.bMirror90Allowed  =false;

    // allow transparence
    rInfo.bTransparenceAllowed = true;

    rInfo.bShearAllowed     =false;
    rInfo.bEdgeRadiusAllowed=false;
    rInfo.bCanConvToPath    =false;
    rInfo.bCanConvToPoly    =false;
    rInfo.bCanConvToPathLineToArea=false;
    rInfo.bCanConvToPolyLineToArea=false;
    rInfo.bCanConvToContour = false;
}


sal_uInt16 SdrTableObj::GetObjIdentifier() const
{
    return static_cast<sal_uInt16>(OBJ_TABLE);
}

void SdrTableObj::TakeTextRect( SdrOutliner& rOutliner, tools::Rectangle& rTextRect, bool bNoEditText, tools::Rectangle* pAnchorRect, bool /*bLineWidth*/ ) const
{
    if( mpImpl.is() )
        TakeTextRect( mpImpl->maEditPos, rOutliner, rTextRect, bNoEditText, pAnchorRect );
}


void SdrTableObj::TakeTextRect( const CellPos& rPos, SdrOutliner& rOutliner, tools::Rectangle& rTextRect, bool bNoEditText, tools::Rectangle* pAnchorRect ) const
{
    if( !mpImpl.is())
        return;

    CellRef xCell( mpImpl->getCell( rPos ) );
    if( !xCell.is() )
        return;

    tools::Rectangle aAnkRect;
    TakeTextAnchorRect( rPos, aAnkRect );

    SdrTextVertAdjust eVAdj=xCell->GetTextVerticalAdjust();

    EEControlBits nStat0=rOutliner.GetControlWord();
    nStat0 |= EEControlBits::AUTOPAGESIZE;
    rOutliner.SetControlWord(nStat0);
    rOutliner.SetMinAutoPaperSize(Size());
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


    // set text at outliner, maybe from edit outliner
    OutlinerParaObject* pPara= xCell->GetOutlinerParaObject();
    if (pEdtOutl && !bNoEditText && mpImpl->mxActiveCell == xCell )
        pPara=pEdtOutl->CreateParaObject().release();

    if (pPara)
    {
        const bool bHitTest(&getSdrModelFromSdrObject().GetHitTestOutliner() == &rOutliner);
        const SdrTextObj* pTestObj(rOutliner.GetTextObj());

        if( !pTestObj || !bHitTest || (pTestObj != this) || (pTestObj->GetOutlinerParaObject() != xCell->GetOutlinerParaObject()) )
        {
            if( bHitTest ) // #i33696# take back fix #i27510#
                rOutliner.SetTextObj( this );

            rOutliner.SetUpdateMode(true);
            rOutliner.SetText(*pPara);
        }
    }
    else
    {
        rOutliner.SetTextObj( nullptr );
    }

    if (pEdtOutl && !bNoEditText && pPara && mpImpl->mxActiveCell == xCell )
        delete pPara;

    rOutliner.SetUpdateMode(true);
    rOutliner.SetControlWord(nStat0);

    Point aTextPos(aAnkRect.TopLeft());
    Size aTextSiz(rOutliner.GetPaperSize());
    if (eVAdj==SDRTEXTVERTADJUST_CENTER || eVAdj==SDRTEXTVERTADJUST_BOTTOM)
    {
        long nFreeHgt=aAnkRect.GetHeight()-aTextSiz.Height();
        if (eVAdj==SDRTEXTVERTADJUST_CENTER)
            aTextPos.AdjustY(nFreeHgt/2 );
        if (eVAdj==SDRTEXTVERTADJUST_BOTTOM)
            aTextPos.AdjustY(nFreeHgt );
    }

    if (pAnchorRect)
        *pAnchorRect=aAnkRect;

    rTextRect=tools::Rectangle(aTextPos,aTextSiz);
}


const CellRef& SdrTableObj::getActiveCell() const
{
    if( mpImpl.is() )
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


sal_Int32 SdrTableObj::getColumnCount() const
{
    return mpImpl.is() ? mpImpl->getColumnCount() : 0;
}


void SdrTableObj::setActiveCell( const CellPos& rPos )
{
    if( mpImpl.is() && mpImpl->mxTable.is() ) try
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


void SdrTableObj::getActiveCellPos( CellPos& rPos ) const
{
    rPos = mpImpl->maEditPos;
}


void SdrTableObj::getCellBounds( const CellPos& rPos, ::tools::Rectangle& rCellRect )
{
    if( mpImpl.is() )
    {
        CellRef xCell( mpImpl->getCell( rPos ) );
        if( xCell.is() )
            rCellRect = xCell->getCellRect();
    }
}


void SdrTableObj::TakeTextAnchorRect(tools::Rectangle& rAnchorRect) const
{
    if( mpImpl.is() )
        TakeTextAnchorRect( mpImpl->maEditPos, rAnchorRect );
}


void SdrTableObj::TakeTextAnchorRect( const CellPos& rPos, tools::Rectangle& rAnchorRect ) const
{
    tools::Rectangle aAnkRect(maRect);

    if( mpImpl.is() )
    {
        CellRef xCell( mpImpl->getCell( rPos ) );
        if( xCell.is() )
            xCell->TakeTextAnchorRect( aAnkRect );
    }

    ImpJustifyRect(aAnkRect);
    rAnchorRect=aAnkRect;
}


void SdrTableObj::TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, tools::Rectangle* pViewInit, tools::Rectangle* pViewMin) const
{
    if( mpImpl.is() )
        TakeTextEditArea( mpImpl->maEditPos, pPaperMin, pPaperMax, pViewInit, pViewMin );
}


void SdrTableObj::TakeTextEditArea( const CellPos& rPos, Size* pPaperMin, Size* pPaperMax, tools::Rectangle* pViewInit, tools::Rectangle* pViewMin ) const
{
    Size aPaperMin,aPaperMax;
    tools::Rectangle aViewInit;
    TakeTextAnchorRect( rPos, aViewInit );

    Size aAnkSiz(aViewInit.GetSize());
    aAnkSiz.AdjustWidth( -1 ); aAnkSiz.AdjustHeight( -1 ); // because GetSize() increments by one

    Size aMaxSiz(aAnkSiz.Width(),1000000);
    Size aTmpSiz(getSdrModelFromSdrObject().GetMaxObjSize());
    if (aTmpSiz.Height()!=0)
        aMaxSiz.setHeight(aTmpSiz.Height() );

    CellRef xCell( mpImpl->getCell( rPos ) );
    SdrTextVertAdjust eVAdj = xCell.is() ? xCell->GetTextVerticalAdjust() : SDRTEXTVERTADJUST_TOP;

    aPaperMax=aMaxSiz;

    aPaperMin.setWidth( aAnkSiz.Width() );

    if (pViewMin!=nullptr)
    {
        *pViewMin=aViewInit;
        long nYFree=aAnkSiz.Height()-aPaperMin.Height();

        if (eVAdj==SDRTEXTVERTADJUST_TOP)
        {
            pViewMin->AdjustBottom( -nYFree );
        }
        else if (eVAdj==SDRTEXTVERTADJUST_BOTTOM)
        {
            pViewMin->AdjustTop(nYFree );
        }
        else
        {
            pViewMin->AdjustTop(nYFree/2 );
            pViewMin->SetBottom(pViewMin->Top()+aPaperMin.Height() );
        }
    }


    if(IsVerticalWriting())
        aPaperMin.setWidth( 0 );
    else
        aPaperMin.setHeight( 0 );

    if (pPaperMin!=nullptr) *pPaperMin=aPaperMin;
    if (pPaperMax!=nullptr) *pPaperMax=aPaperMax;
    if (pViewInit!=nullptr) *pViewInit=aViewInit;
}


EEAnchorMode SdrTableObj::GetOutlinerViewAnchorMode() const
{
    EEAnchorMode eRet=EEAnchorMode::TopLeft;
    CellRef xCell( getActiveCell() );
    if( xCell.is() )
    {
        SdrTextVertAdjust eV=xCell->GetTextVerticalAdjust();

        {
            if (eV==SDRTEXTVERTADJUST_TOP)
            {
                eRet=EEAnchorMode::TopLeft;
            }
            else if (eV==SDRTEXTVERTADJUST_BOTTOM)
            {
                eRet=EEAnchorMode::BottomLeft;
            }
            else
            {
                eRet=EEAnchorMode::VCenterLeft;
            }
        }
    }
    return eRet;
}


OUString SdrTableObj::TakeObjNameSingul() const
{
    OUStringBuffer sName(SvxResId(STR_ObjNameSingulTable));

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


OUString SdrTableObj::TakeObjNamePlural() const
{
    return SvxResId(STR_ObjNamePluralTable);
}


SdrTableObj* SdrTableObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< SdrTableObj >(rTargetModel);
}

SdrTableObj& SdrTableObj::operator=(const SdrTableObj& rObj)
{
    if( this == &rObj )
    {
        return *this;
    }

    // call parent
    // before SdrObject::operator= was called which is wrong from
    // the derivation hierarchy and may leave quite some entries
    // uninitialized. Changed to SdrTextObj::operator=, but had to adapt
    // usage of pNewOutlinerParaObject/mpText there due to nullptr access
    SdrTextObj::operator=(rObj);

    TableModelNotifyGuard aGuard( mpImpl.is() ? mpImpl->mxTable.get() : nullptr );

    maLogicRect = rObj.maLogicRect;
    maRect = rObj.maRect;
    aGeo = rObj.aGeo;
    eTextKind = rObj.eTextKind;
    bTextFrame = rObj.bTextFrame;
    aTextSize = rObj.aTextSize;
    bTextSizeDirty = rObj.bTextSizeDirty;
    bNoShear = rObj.bNoShear;
    bDisableAutoWidthOnDragging = rObj.bDisableAutoWidthOnDragging;

    // use SdrTableObjImpl::operator= now to
    // copy model data and other stuff (see there)
    *mpImpl = *rObj.mpImpl;

    return *this;
}


const tools::Rectangle& SdrTableObj::GetSnapRect() const
{
    return maRect;
}


void SdrTableObj::NbcSetSnapRect(const tools::Rectangle& rRect)
{
    NbcSetLogicRect( rRect );
}


const tools::Rectangle& SdrTableObj::GetLogicRect() const
{
    return maLogicRect;
}


void SdrTableObj::RecalcSnapRect()
{
}


bool SdrTableObj::BegTextEdit(SdrOutliner& rOutl)
{
    if( pEdtOutl != nullptr )
        return false;

    pEdtOutl=&rOutl;

    mbInEditMode = true;

    rOutl.Init( OutlinerMode::TextObject );
    rOutl.SetRefDevice(getSdrModelFromSdrObject().GetRefDevice());

    bool bUpdMerk=rOutl.GetUpdateMode();
    if (bUpdMerk) rOutl.SetUpdateMode(false);
    Size aPaperMin;
    Size aPaperMax;
    tools::Rectangle aEditArea;
    TakeTextEditArea(&aPaperMin,&aPaperMax,&aEditArea,nullptr);

    rOutl.SetMinAutoPaperSize(aPaperMin);
    rOutl.SetMaxAutoPaperSize(aPaperMax);
    rOutl.SetPaperSize(aPaperMax);

    if (bUpdMerk) rOutl.SetUpdateMode(true);

    EEControlBits nStat=rOutl.GetControlWord();
    nStat   |= EEControlBits::AUTOPAGESIZE;
    nStat   &=~EEControlBits::STRETCHING;
    rOutl.SetControlWord(nStat);

    OutlinerParaObject* pPara = GetOutlinerParaObject();
    if(pPara)
        rOutl.SetText(*pPara);

    rOutl.UpdateFields();
    rOutl.ClearModifyFlag();

    return true;
}


void SdrTableObj::EndTextEdit(SdrOutliner& rOutl)
{

    if (getSdrModelFromSdrObject().IsUndoEnabled() && !mpImpl->maUndos.empty())
    {
        // These actions should be on the undo stack after text edit.
        for (std::unique_ptr<SdrUndoAction>& pAction : mpImpl->maUndos)
            getSdrModelFromSdrObject().AddUndo( std::move(pAction));
        mpImpl->maUndos.clear();

        getSdrModelFromSdrObject().AddUndo(getSdrModelFromSdrObject().GetSdrUndoFactory().CreateUndoGeoObject(*this));
    }

    if(rOutl.IsModified())
    {
        std::unique_ptr<OutlinerParaObject> pNewText;
        Paragraph* p1stPara = rOutl.GetParagraph( 0 );
        sal_Int32 nParaCnt = rOutl.GetParagraphCount();

        if(p1stPara)
        {
            // to remove the grey field background
            rOutl.UpdateFields();

            // create new text object
            pNewText = rOutl.CreateParaObject( 0, nParaCnt );
        }
        SetOutlinerParaObject(std::move(pNewText));
    }

    pEdtOutl = nullptr;
    rOutl.Clear();
    EEControlBits nStat = rOutl.GetControlWord();
    nStat &= ~EEControlBits::AUTOPAGESIZE;
    rOutl.SetControlWord(nStat);

    mbInEditMode = false;
}


OutlinerParaObject* SdrTableObj::GetOutlinerParaObject() const
{
    CellRef xCell( getActiveCell() );
    if( xCell.is() )
        return xCell->GetOutlinerParaObject();
    else
        return nullptr;
}


void SdrTableObj::NbcSetOutlinerParaObject( std::unique_ptr<OutlinerParaObject> pTextObject)
{
    CellRef xCell( getActiveCell() );
    if( xCell.is() )
    {
        // Update HitTestOutliner
        const SdrTextObj* pTestObj(getSdrModelFromSdrObject().GetHitTestOutliner().GetTextObj());

        if(pTestObj && pTestObj->GetOutlinerParaObject() == xCell->GetOutlinerParaObject())
        {
            getSdrModelFromSdrObject().GetHitTestOutliner().SetTextObj(nullptr);
        }

        xCell->SetOutlinerParaObject( std::move(pTextObject) );
        SetTextSizeDirty();
        NbcAdjustTextFrameWidthAndHeight();
    }
}


void SdrTableObj::NbcSetLogicRect(const tools::Rectangle& rRect)
{
    maLogicRect=rRect;
    ImpJustifyRect(maLogicRect);
    const bool bWidth = maLogicRect.getWidth() != maRect.getWidth();
    const bool bHeight = maLogicRect.getHeight() != maRect.getHeight();
    maRect = maLogicRect;
    if (mpImpl->mbSkipChangeLayout)
        // Avoid distributing newly available space between existing cells.
        NbcAdjustTextFrameWidthAndHeight();
    else
        NbcAdjustTextFrameWidthAndHeight(!bHeight, !bWidth);
    SetRectsDirty();
}


void SdrTableObj::AdjustToMaxRect( const tools::Rectangle& rMaxRect, bool /* bShrinkOnly = false */ )
{
    tools::Rectangle aAdjustRect( rMaxRect );
    aAdjustRect.setHeight( GetLogicRect().getHeight() );
    SetLogicRect( aAdjustRect );
}


void SdrTableObj::NbcMove(const Size& rSiz)
{
    maLogicRect.Move(rSiz);
    SdrTextObj::NbcMove( rSiz );
    if( mpImpl.is() )
        mpImpl->UpdateCells( maRect );
}


void SdrTableObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    tools::Rectangle aOldRect( maLogicRect );
    ResizeRect(maLogicRect,rRef,xFact,yFact);

    maRect = maLogicRect;
    NbcAdjustTextFrameWidthAndHeight( maLogicRect.GetHeight() == aOldRect.GetHeight(), maLogicRect.GetWidth() == aOldRect.GetWidth() );
    SetRectsDirty();
}


bool SdrTableObj::AdjustTextFrameWidthAndHeight()
{
    tools::Rectangle aNewRect(maLogicRect);
    bool bRet=AdjustTextFrameWidthAndHeight(aNewRect);
    if (bRet)
    {
        tools::Rectangle aBoundRect0;
        if (pUserCall!=nullptr)
            aBoundRect0=GetLastBoundRect();
        maRect = aNewRect;
        SetRectsDirty();
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
    return bRet;
}


bool SdrTableObj::AdjustTextFrameWidthAndHeight(tools::Rectangle& rR, bool bHeight, bool bWidth) const
{
    if(rR.IsEmpty() || !mpImpl.is() || !mpImpl->mxTable.is())
        return false;

    tools::Rectangle aRectangle( rR );
    mpImpl->LayoutTable( aRectangle, !bWidth, !bHeight );

    if( aRectangle != rR )
    {
        rR = aRectangle;
        return true;
    }
    else
    {
        return false;
    }
}


void SdrTableObj::NbcReformatText()
{
    NbcAdjustTextFrameWidthAndHeight();
}


bool SdrTableObj::IsVerticalWriting() const
{
    const SvxWritingModeItem& rModeItem = GetObjectItem( SDRATTR_TEXTDIRECTION );
    return rModeItem.GetValue() == css::text::WritingMode_TB_RL;
}


void SdrTableObj::SetVerticalWriting(bool bVertical)
{
    if(bVertical != IsVerticalWriting() )
    {
        SvxWritingModeItem aModeItem( css::text::WritingMode_LR_TB, SDRATTR_TEXTDIRECTION );
        SetObjectItem( aModeItem );
    }
}


WritingMode SdrTableObj::GetWritingMode() const
{
    SfxStyleSheet* pStyle = GetStyleSheet();
    if ( !pStyle )
        return WritingMode_LR_TB;

    WritingMode eWritingMode = WritingMode_LR_TB;
    const SfxItemSet &rSet = pStyle->GetItemSet();
    const SfxPoolItem *pItem;

    if ( rSet.GetItemState( SDRATTR_TEXTDIRECTION, false, &pItem ) == SfxItemState::SET )
        eWritingMode = static_cast< const SvxWritingModeItem * >( pItem )->GetValue();

    if ( ( eWritingMode != WritingMode_TB_RL ) &&
         ( rSet.GetItemState( EE_PARA_WRITINGDIR, false, &pItem ) == SfxItemState::SET ) )
    {
        if ( static_cast< const SvxFrameDirectionItem * >( pItem )->GetValue() == SvxFrameDirection::Horizontal_LR_TB )
            eWritingMode = WritingMode_LR_TB;
        else
            eWritingMode = WritingMode_RL_TB;
    }

    return eWritingMode;
}

void SdrTableObj::AddUndo(SdrUndoAction* pUndo)
{
    mpImpl->maUndos.push_back(std::unique_ptr<SdrUndoAction>(pUndo));
}

void SdrTableObj::SetSkipChangeLayout(bool bSkipChangeLayout)
{
    mpImpl->mbSkipChangeLayout = bSkipChangeLayout;
}

bool SdrTableObj::IsReallyEdited() const
{
    return pEdtOutl && pEdtOutl->IsModified();
}

bool SdrTableObj::IsFontwork() const
{
    return false;
}

sal_uInt32 SdrTableObj::GetHdlCount() const
{
    sal_uInt32 nCount = SdrTextObj::GetHdlCount();
    const sal_Int32 nRowCount = mpImpl->getRowCount();
    const sal_Int32 nColCount = mpImpl->getColumnCount();

    if( nRowCount && nColCount )
        nCount += nRowCount + nColCount + 2 + 1;

    return nCount;
}

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

        Point aPoint( maRect.TopLeft() );
        aPoint.AdjustY(nEdge );

        std::unique_ptr<TableEdgeHdl> pHdl(new TableEdgeHdl(aPoint,true,nEdgeMin,nEdgeMax,nColCount+1));
        pHdl->SetPointNum( nRow );
        aRowEdges[nRow] = pHdl.get();
        rHdlList.AddHdl( std::move(pHdl) );
    }

    // second add column handles
    std::vector< TableEdgeHdl* > aColEdges( nColCount + 1 );

    for( sal_Int32 nCol = 0; nCol <= nColCount; nCol++ )
    {
        sal_Int32 nEdgeMin, nEdgeMax;
        const sal_Int32 nEdge = mpImpl->mpLayouter->getVerticalEdge( nCol, &nEdgeMin, &nEdgeMax );
        nEdgeMin -= nEdge;
        nEdgeMax -= nEdge;

        Point aPoint( maRect.TopLeft() );
        aPoint.AdjustX(nEdge );

        std::unique_ptr<TableEdgeHdl> pHdl(new TableEdgeHdl(aPoint,false,nEdgeMin,nEdgeMax, nRowCount+1));
        pHdl->SetPointNum( nCol );
        aColEdges[nCol] = pHdl.get();
        rHdlList.AddHdl( std::move(pHdl) );
    }

    // now add visible edges to row and column handles
    if( mpImpl->mpLayouter )
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
                        aColEdges[nCol]->SetEdge( nRow, nY, nY + nRowHeight, (rLayouter.getBorderLine( nCol, nRow, false ) == nullptr) ? Visible : Invisible);
                }

                if( nColWidth > 0 )
                {
                    if( rLayouter.isEdgeVisible( nCol, nRow, true ) )
                        aRowEdges[nRow]->SetEdge( nCol, nX, nX + nColWidth, (rLayouter.getBorderLine( nCol, nRow, true ) == nullptr) ? Visible : Invisible);
                }

                nX += nColWidth;
            }

            nY += nRowHeight;
        }
    }

    // add remaining handles
    SdrHdlList tempList(nullptr);
    tempList.AddHdl( std::make_unique<TableBorderHdl>( maRect, !IsTextEditActive() ) );
    tempList.AddHdl( std::make_unique<SdrHdl>(maRect.TopLeft(),SdrHdlKind::UpperLeft) );
    tempList.AddHdl( std::make_unique<SdrHdl>(maRect.TopCenter(),SdrHdlKind::Upper) );
    tempList.AddHdl( std::make_unique<SdrHdl>(maRect.TopRight(),SdrHdlKind::UpperRight) );
    tempList.AddHdl( std::make_unique<SdrHdl>(maRect.LeftCenter(),SdrHdlKind::Left) );
    tempList.AddHdl( std::make_unique<SdrHdl>(maRect.RightCenter(),SdrHdlKind::Right) );
    tempList.AddHdl( std::make_unique<SdrHdl>(maRect.BottomLeft(),SdrHdlKind::LowerLeft) );
    tempList.AddHdl( std::make_unique<SdrHdl>(maRect.BottomCenter(),SdrHdlKind::Lower) );
    tempList.AddHdl( std::make_unique<SdrHdl>(maRect.BottomRight(),SdrHdlKind::LowerRight) );
    for( size_t nHdl = 0; nHdl < tempList.GetHdlCount(); ++nHdl )
        tempList.GetHdl(nHdl)->SetMoveOutside(true);
    tempList.MoveTo(rHdlList);

    const size_t nHdlCount = rHdlList.GetHdlCount();
    for( size_t nHdl = 0; nHdl < nHdlCount; ++nHdl )
        rHdlList.GetHdl(nHdl)->SetObj(const_cast<SdrTableObj*>(this));
}

// Dragging

bool SdrTableObj::hasSpecialDrag() const
{
    return true;
}

bool SdrTableObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl = rDrag.GetHdl();
    const SdrHdlKind eHdl((pHdl == nullptr) ? SdrHdlKind::Move : pHdl->GetKind());

    switch( eHdl )
    {
        case SdrHdlKind::UpperLeft:
        case SdrHdlKind::Upper:
        case SdrHdlKind::UpperRight:
        case SdrHdlKind::Left:
        case SdrHdlKind::Right:
        case SdrHdlKind::LowerLeft:
        case SdrHdlKind::Lower:
        case SdrHdlKind::LowerRight:
        case SdrHdlKind::Move:
        {
            break;
        }

        case SdrHdlKind::User:
        {
            rDrag.SetEndDragChangesAttributes(false);
            rDrag.SetNoSnap();
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
    const SdrHdlKind eHdl((pHdl == nullptr) ? SdrHdlKind::Move : pHdl->GetKind());

    switch( eHdl )
    {
        case SdrHdlKind::UpperLeft:
        case SdrHdlKind::Upper:
        case SdrHdlKind::UpperRight:
        case SdrHdlKind::Left:
        case SdrHdlKind::Right:
        case SdrHdlKind::LowerLeft:
        case SdrHdlKind::Lower:
        case SdrHdlKind::LowerRight:
        {
            const tools::Rectangle aNewRectangle(ImpDragCalcRect(rDrag));

            if (aNewRectangle != maRect)
            {
                   NbcSetLogicRect(aNewRectangle);
            }

            break;
        }

        case SdrHdlKind::Move:
        {
            NbcMove( Size( rDrag.GetDX(), rDrag.GetDY() ) );
            break;
        }

        case SdrHdlKind::User:
        {
            rDrag.SetEndDragChangesAttributes(false);
            rDrag.SetNoSnap();
            const TableEdgeHdl* pEdgeHdl = dynamic_cast< const TableEdgeHdl* >( pHdl );

            if( pEdgeHdl )
            {
                if( IsInserted() )
                {
                    rDrag.SetEndDragChangesAttributes(true);
                    rDrag.SetEndDragChangesLayout(true);
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

basegfx::B2DPolyPolygon SdrTableObj::getSpecialDragPoly(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aRetval;
    const SdrHdl* pHdl = rDrag.GetHdl();

    if( pHdl && (SdrHdlKind::User == pHdl->GetKind()) )
    {
        const TableEdgeHdl* pEdgeHdl = dynamic_cast< const TableEdgeHdl* >( pHdl );

        if( pEdgeHdl )
        {
            aRetval = pEdgeHdl->getSpecialDragPoly( rDrag );
        }
    }

    return aRetval;
}


// Create


bool SdrTableObj::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho4Possible();
    tools::Rectangle aRect1(rStat.GetStart(), rStat.GetNow());
    aRect1.Justify();
    rStat.SetActionRect(aRect1);
    maRect = aRect1;
    return true;
}


bool SdrTableObj::MovCreate(SdrDragStat& rStat)
{
    tools::Rectangle aRect1;
    rStat.TakeCreateRect(aRect1);
    ImpJustifyRect(aRect1);
    rStat.SetActionRect(aRect1);
    maRect = aRect1; // for ObjName
    SetBoundRectDirty();
    bSnapRectDirty=true;
    return true;
}


bool SdrTableObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    rStat.TakeCreateRect(maRect);
    ImpJustifyRect(maRect);
    return (eCmd==SdrCreateCmd::ForceEnd || rStat.GetPointCount()>=2);
}

void SdrTableObj::BrkCreate(SdrDragStat& /*rStat*/)
{
}


bool SdrTableObj::BckCreate(SdrDragStat& /*rStat*/)
{
    return true;
}


basegfx::B2DPolyPolygon SdrTableObj::TakeCreatePoly(const SdrDragStat& rDrag) const
{
    tools::Rectangle aRect1;
    rDrag.TakeCreateRect(aRect1);
    aRect1.Justify();

    basegfx::B2DPolyPolygon aRetval;
    const basegfx::B2DRange aRange(aRect1.Left(), aRect1.Top(), aRect1.Right(), aRect1.Bottom());
    aRetval.append(basegfx::utils::createPolygonFromRect(aRange));
    return aRetval;
}


PointerStyle SdrTableObj::GetCreatePointer() const
{
    return PointerStyle::Cross;
}


void SdrTableObj::createCell( CellRef& xNewCell )
{
    xNewCell = Cell::create( *this );
}


SdrObjGeoData *SdrTableObj::NewGeoData() const
{
    return new TableObjectGeoData;
}


void SdrTableObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    DBG_ASSERT( dynamic_cast< TableObjectGeoData* >( &rGeo ), "svx::SdrTableObj::SaveGeoData(), illegal geo data!" );
    SdrTextObj::SaveGeoData (rGeo);

    static_cast<TableObjectGeoData &>(rGeo).maLogicRect = maLogicRect;
}


void SdrTableObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    DBG_ASSERT( dynamic_cast< const TableObjectGeoData* >( &rGeo ), "svx::SdrTableObj::SaveGeoData(), illegal geo data!" );

    maLogicRect = static_cast<const TableObjectGeoData &>(rGeo).maLogicRect;

    SdrTextObj::RestGeoData (rGeo);

    if( mpImpl.is() )
        mpImpl->LayoutTable(maRect, false, false);
    ActionChanged();
}

void SdrTableObj::CropTableModelToSelection(const CellPos& rStart, const CellPos& rEnd)
{
    if(!mpImpl.is())
    {
        return;
    }

    mpImpl->CropTableModelToSelection(rStart, rEnd);
}

void SdrTableObj::DistributeColumns( sal_Int32 nFirstColumn, sal_Int32 nLastColumn, const bool bOptimize, const bool bMinimize )
{
    if( mpImpl.is() && mpImpl->mpLayouter )
    {
        TableModelNotifyGuard aGuard( mpImpl->mxTable.get() );
        mpImpl->mpLayouter->DistributeColumns( maRect, nFirstColumn, nLastColumn, bOptimize, bMinimize );
    }
}


void SdrTableObj::DistributeRows( sal_Int32 nFirstRow, sal_Int32 nLastRow, const bool bOptimize, const bool bMinimize )
{
    if( mpImpl.is() && mpImpl->mpLayouter )
    {
        TableModelNotifyGuard aGuard( mpImpl->mxTable.get() );
        mpImpl->mpLayouter->DistributeRows( maRect, nFirstRow, nLastRow, bOptimize, bMinimize );
    }
}


void SdrTableObj::SetChanged()
{
    if( mpImpl.is() )
    {
        mpImpl->LayoutTable( maRect, false, false );
    }

    ::SdrTextObj::SetChanged();
}


void SdrTableObj::uno_lock()
{
    if( mpImpl.is() && mpImpl->mxTable.is() )
        mpImpl->mxTable->lockBroadcasts();
}


void SdrTableObj::uno_unlock()
{
    if( mpImpl.is() && mpImpl->mxTable.is() )
        mpImpl->mxTable->unlockBroadcasts();
}

void SdrTableObj::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SdrTableObj"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    SdrObject::dumpAsXml(pWriter);

    mpImpl->dumpAsXml(pWriter);

    xmlTextWriterEndElement(pWriter);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
