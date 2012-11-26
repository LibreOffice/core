/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#define ITEMID_BOX SDRATTR_TABLE_BORDER
#define ITEMID_BOXINFO SDRATTR_TABLE_BORDER_INNER

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
#include <svx/svdlegacy.hxx>

// -----------------------------------------------------------------------------

using ::rtl::OUString;
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

    CellRef getCell( const CellPos& rPos ) const;
    void LayoutTable( basegfx::B2DRange& o_aArea, bool bFitWidth, bool bFitHeight );

    bool ApplyCellStyles();
    void UpdateCells( Rectangle& rArea );

    SdrTableObjImpl();
    virtual ~SdrTableObjImpl();

    void init( SdrTableObj* pTable, sal_Int32 nColumns, sal_Int32 nRows );
    void dispose();

    sal_Int32 getColumnCount() const;
    sal_Int32 getRowCount() const;

    void DragEdge( bool mbHorizontal, int nEdge, sal_Int32 nOffset );

    const SfxPoolItem* GetCellItem( const CellPos& rPos, sal_uInt16 nWhich ) const;

    void operator=( const SdrTableObjImpl& rSource );

    // XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    void update();

    void connectTableStyle();
    void disconnectTableStyle();
    virtual bool isInUse();

    bool UpdateWritingMode();
};

// -----------------------------------------------------------------------------

SdrTableObjImpl::SdrTableObjImpl()
: mpTableObj( 0 )
, mpLayouter( 0 )
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
    UpdateWritingMode();

    basegfx::B2DRange aObjectRange(
        mpTableObj->getSdrObjectTranslate(),
        mpTableObj->getSdrObjectTranslate() + basegfx::absolute(mpTableObj->getSdrObjectScale()));

    LayoutTable( aObjectRange, true, true );
}

// -----------------------------------------------------------------------------

void SdrTableObjImpl::operator=( const SdrTableObjImpl& rSource )
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
    UpdateWritingMode();
    ApplyCellStyles();

    basegfx::B2DRange aObjectRange(
        mpTableObj->getSdrObjectTranslate(),
        mpTableObj->getSdrObjectTranslate() + basegfx::absolute(mpTableObj->getSdrObjectScale()));

    LayoutTable( aObjectRange, false, false );
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
                        xCell->SetStyleSheet( pStyle, true);
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
    if( (nEdge > 0) && mxTable.is()) try
    {
        const OUString sSize( RTL_CONSTASCII_USTRINGPARAM( "Size" ) );
        nEdge--;
        if( mbHorizontal )
        {
            if( (nEdge >= 0) && (nEdge < getRowCount()) )
            {
                sal_Int32 nHeigth = mpLayouter->getRowHeight( nEdge );
                nHeigth += nOffset;
                Reference< XIndexAccess > xRows( mxTable->getRows(), UNO_QUERY_THROW );
                Reference< XPropertySet > xRowSet( xRows->getByIndex( nEdge ), UNO_QUERY_THROW );
                xRowSet->setPropertyValue( sSize, Any( nHeigth ) );
            }
        }
        else
        {
            if( (nEdge >= 0) && (nEdge < getColumnCount()) )
            {
                sal_Int32 nWidth = mpLayouter->getColumnWidth( nEdge );
                nWidth += nOffset;

                Reference< XIndexAccess > xCols( mxTable->getColumns(), UNO_QUERY_THROW );
                Reference< XPropertySet > xColSet( xCols->getByIndex( nEdge ), UNO_QUERY_THROW );
                xColSet->setPropertyValue( sSize, Any( nWidth ) );

                if( nEdge > 0 && nEdge < mxTable->getColumnCount() )
                {
                    const bool bRTL = mpLayouter->GetWritingMode() == WritingMode_RL_TB;

                    if( bRTL )
                        nEdge--;
                    else
                        nEdge++;

                    if( (bRTL && (nEdge >= 0)) || (!bRTL && (nEdge < mxTable->getColumnCount())) )
                    {
                        nWidth = mpLayouter->getColumnWidth( nEdge );
                        nWidth = std::max( (sal_Int32)(nWidth - nOffset), (sal_Int32)0 );

                        xColSet = Reference< XPropertySet >( xCols->getByIndex( nEdge ), UNO_QUERY_THROW );
                        xColSet->setPropertyValue( sSize, Any( nWidth ) );
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        DBG_ERROR( "svx::SdrTableObjImpl::DragEdge(), exception caught!" );
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
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*mpTableObj);

        if( (maEditPos.mnRow >= getRowCount()) || (maEditPos.mnCol >= getColumnCount()) || (getCell( maEditPos ) != mxActiveCell) )
        {
            if(maEditPos.mnRow >= getRowCount())
                maEditPos.mnRow = getRowCount()-1;

            if(maEditPos.mnCol >= getColumnCount())
                maEditPos.mnCol = getColumnCount()-1;

            mpTableObj->setActiveCell( maEditPos );
        }

        ApplyCellStyles();

        basegfx::B2DRange aObjectRange(
            mpTableObj->getSdrObjectTranslate(),
            mpTableObj->getSdrObjectTranslate() + basegfx::absolute(mpTableObj->getSdrObjectScale()));

        LayoutTable( aObjectRange, false, false );
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
    return mpTableObj && mpTableObj->IsObjectInserted();
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
        DBG_ERROR( "svx::SdrTableObjImpl::getCell(), exception caught!" );
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

void SdrTableObjImpl::LayoutTable( basegfx::B2DRange& o_aArea, bool bFitWidth, bool bFitHeight )
{
    if( mpTableObj && mpLayouter )
    {
        TableModelNotifyGuard aGuard( mxTable.get() );

        mpLayouter->LayoutTable( o_aArea, bFitWidth, bFitHeight );
    }
}

// -----------------------------------------------------------------------------

bool SdrTableObjImpl::UpdateWritingMode()
{
    if( mpTableObj && mpLayouter )
    {
        WritingMode eWritingMode = (WritingMode)static_cast< const SvxWritingModeItem& >( mpTableObj->GetObjectItem( SDRATTR_TEXTDIRECTION ) ).GetValue();

        if( eWritingMode != WritingMode_TB_RL )
        {
            if( static_cast< const SvxFrameDirectionItem& >( mpTableObj->GetObjectItem( EE_PARA_WRITINGDIR ) ).GetValue() == FRMDIR_HORI_LEFT_TOP )
                eWritingMode = WritingMode_LR_TB;
            else
                eWritingMode = WritingMode_RL_TB;
        }

        if( eWritingMode != mpLayouter->GetWritingMode() )
        {
            mpLayouter->SetWritingMode( eWritingMode );
            return true;
        }
    }
    return false;
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

const SfxPoolItem* SdrTableObjImpl::GetCellItem( const CellPos& rPos, sal_uInt16 nWhich ) const
{
    CellRef xCell( getCell( rPos  ) );
    if( xCell.is() )
        return xCell->GetItemSet().GetItem( nWhich );
    else
        return 0;
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

SdrTableObj::SdrTableObj(
    SdrModel& rSdrModel,
    const basegfx::B2DHomMatrix& rTransform,
    sal_Int32 nColumns,
    sal_Int32 nRows)
:   SdrTextObj(rSdrModel, rTransform)
{
    if( nColumns <= 0 )
        nColumns = 1;

    if( nRows <= 0 )
        nRows = 1;

    init( nColumns, nRows );
}

// --------------------------------------------------------------------

void SdrTableObj::init( sal_Int32 nColumns, sal_Int32 nRows )
{
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

void SdrTableObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const SdrTableObj* pSource = dynamic_cast< const SdrTableObj* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrTextObj::copyDataFromSdrObject(rSource);

            // copy local data
            TableModelNotifyGuard aGuard( mpImpl ? mpImpl->mxTable.get() : 0 );

            eTextKind = pSource->eTextKind;
            bTextFrame = pSource->bTextFrame;
            aTextSize = pSource->aTextSize;
            bTextSizeDirty = pSource->bTextSizeDirty;
            bNoShear = pSource->bNoShear;
            bNoRotate = pSource->bNoRotate;
            bNoMirror = pSource->bNoMirror;
            bDisableAutoWidthOnDragging = pSource->bDisableAutoWidthOnDragging;

            if( pSource->mpImpl )
            {
                *mpImpl = *pSource->mpImpl;
            }
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* SdrTableObj::CloneSdrObject(SdrModel* pTargetModel) const
{
    SdrTableObj* pClone = new SdrTableObj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

bool SdrTableObj::IsClosedObj() const
{
    return true;
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

TableHitKind SdrTableObj::CheckTableHit( const basegfx::B2DPoint& rPos, sal_Int32& rnX, sal_Int32& rnY, int nTol ) const
{
    if( !mpImpl || !mpImpl->mxTable.is() )
        return SDRTABLEHIT_NONE;

    rnX = 0;
    rnY = 0;

    const sal_Int32 nColCount = mpImpl->getColumnCount();
    const sal_Int32 nRowCount = mpImpl->getRowCount();

    const Rectangle aOldObjLogicRect(sdr::legacy::GetLogicRect(*this));
    sal_Int32 nX = basegfx::fround(rPos.getX()) + nTol - aOldObjLogicRect.nLeft;
    sal_Int32 nY = basegfx::fround(rPos.getY()) + nTol - aOldObjLogicRect.nTop;

    if( (nX < 0) || (nX > (aOldObjLogicRect.GetWidth() + nTol)) || (nY < 0) || (nY > (aOldObjLogicRect.GetHeight() + nTol) ) )
        return SDRTABLEHIT_NONE;

    // get vertical edge number and check for a hit
    const bool bRTL = GetWritingMode() == WritingMode_RL_TB;
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

void SdrTableObj::InsertRows( sal_Int32 nIndex, sal_Int32 nCount /*= 1*/ )
{
    if( mpImpl->mxTable.is() ) try
    {
        Reference< XTableRows > xRows( mpImpl->mxTable->getRows(), UNO_QUERY_THROW );
        xRows->insertByIndex( nIndex, nCount );
    }
    catch( Exception& )
    {
        DBG_ERROR("SdrTableObj::InsertRows(), exception caught!");
    }
}

// --------------------------------------------------------------------

void SdrTableObj::InsertColumns( sal_Int32 nIndex, sal_Int32 nCount /*= 1*/ )
{
    if( mpImpl->mxTable.is() ) try
    {
        Reference< XTableColumns > xColumns( mpImpl->mxTable->getColumns(), UNO_QUERY_THROW );
        xColumns->insertByIndex( nIndex, nCount );
    }
    catch( Exception& )
    {
        DBG_ERROR("SdrTableObj::InsertColumns(), exception caught!");
    }
}

// --------------------------------------------------------------------

void SdrTableObj::DeleteRows( sal_Int32 nIndex, sal_Int32 nCount /*= 1*/ )
{
    if( mpImpl->mxTable.is() ) try
    {
        Reference< XTableRows > xRows( mpImpl->mxTable->getRows(), UNO_QUERY_THROW );
        xRows->removeByIndex( nIndex, nCount );
    }
    catch( Exception& )
    {
        DBG_ERROR("SdrTableObj::DeleteRows(), exception caught!");
    }
}

// --------------------------------------------------------------------

void SdrTableObj::DeleteColumns( sal_Int32 nIndex, sal_Int32 nCount /*= 1*/ )
{
    if( mpImpl->mxTable.is() ) try
    {
        Reference< XTableColumns > xColumns( mpImpl->mxTable->getColumns(), UNO_QUERY_THROW );
        xColumns->removeByIndex( nIndex, nCount );
    }
    catch( Exception& )
    {
        DBG_ERROR("SdrTableObj::DeleteColumns(), exception caught!");
    }
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
sal_Int32 SdrTableObj::CheckTextHit(const basegfx::B2DPoint& rPnt) const
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

void SdrTableObj::FitFrameToTextSize()
{
    // todo
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
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
        basegfx::B2DRange aObjectRange(
            getSdrObjectTranslate(),
            getSdrObjectTranslate() + basegfx::absolute(getSdrObjectScale()));

        mpImpl->LayoutTable( aObjectRange, false, false );
    }
}

// --------------------------------------------------------------------

void SdrTableObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.mbResizeFreeAllowed = true;
    rInfo.mbResizePropAllowed = true;
    rInfo.mbRotateFreeAllowed = false;
    rInfo.mbRotate90Allowed = false;
    rInfo.mbMirrorFreeAllowed = false;
    rInfo.mbMirror45Allowed = false;
    rInfo.mbMirror90Allowed = false;

    // allow transparence
    rInfo.mbTransparenceAllowed = true;

    // gradient depends on fillstyle
    XFillStyle eFillStyle = ((XFillStyleItem&)(GetObjectItem(XATTR_FILLSTYLE))).GetValue();
    rInfo.mbGradientAllowed = (XFILL_GRADIENT == eFillStyle);
    rInfo.mbShearAllowed = false;
    rInfo.mbEdgeRadiusAllowed = false;
    rInfo.mbCanConvToPath = false;
    rInfo.mbCanConvToPoly = false;
    rInfo.mbCanConvToPathLineToArea = false;
    rInfo.mbCanConvToPolyLineToArea = false;
    rInfo.mbCanConvToContour = false;
}

// --------------------------------------------------------------------

sal_uInt16 SdrTableObj::GetObjIdentifier() const
{
    return static_cast<sal_uInt16>(OBJ_TABLE);
}

void SdrTableObj::TakeTextRange(SdrOutliner& rOutliner, basegfx::B2DRange& rTextRange, basegfx::B2DRange& rAnchorRange) const
{
    if( mpImpl )
    {
        TakeTextRange(mpImpl->maEditPos, rOutliner, rTextRange, rAnchorRange);
    }
}

// --------------------------------------------------------------------

void SdrTableObj::TakeTextRange(const CellPos& rPos, SdrOutliner& rOutliner, basegfx::B2DRange& rTextRange, basegfx::B2DRange& rAnchorRange) const
{
    if( !mpImpl )
        return;

    CellRef xCell( mpImpl->getCell( rPos ) );
    if( !xCell.is() )
        return;

    TakeTextAnchorRangeFromCell( rPos, rAnchorRange );

    SdrTextVertAdjust eVAdj=xCell->GetTextVerticalAdjust();

    sal_uIntPtr nStat0=rOutliner.GetControlWord();
    Size aNullSize;
    const Size aIntAnchorSize(basegfx::fround(rAnchorRange.getWidth()), basegfx::fround(rAnchorRange.getHeight()));
    nStat0 |= EE_CNTRL_AUTOPAGESIZE;
    rOutliner.SetControlWord(nStat0);
    rOutliner.SetMinAutoPaperSize(aNullSize);
    rOutliner.SetMaxAutoPaperSize(aIntAnchorSize);
    rOutliner.SetPaperSize(aIntAnchorSize);

    // #103516# New try with _BLOCK for hor and ver after completely
    // supporting full width for vertical text.
    rOutliner.SetMinAutoPaperSize(Size(aIntAnchorSize.getWidth(), 0));

    // set text at outliner, maybe from edit outliner
    OutlinerParaObject* pPara= xCell->GetOutlinerParaObject();

    if (pEdtOutl && mpImpl->mxActiveCell == xCell )
    {
        pPara = pEdtOutl->CreateParaObject();
    }

    if (pPara)
    {
        const SdrTextObj* pTestObj = rOutliner.GetTextObj();

        if( !pTestObj || (pTestObj != this) || (pTestObj->GetOutlinerParaObject() != xCell->GetOutlinerParaObject()) )
        {
            rOutliner.SetTextObj( this );
            rOutliner.SetUpdateMode(true);
            rOutliner.SetText(*pPara);
        }
    }
    else
    {
        rOutliner.SetTextObj( NULL );
    }

    if (pEdtOutl && pPara && mpImpl->mxActiveCell == xCell )
    {
        delete pPara;
    }

    rOutliner.SetUpdateMode(true);
    rOutliner.SetControlWord(nStat0);

    basegfx::B2DPoint aTextPos(rAnchorRange.getMinimum());
    const basegfx::B2DVector aTextSiz(rOutliner.GetPaperSize().getWidth(), rOutliner.GetPaperSize().getHeight());

    if(SDRTEXTVERTADJUST_CENTER == eVAdj || SDRTEXTVERTADJUST_BOTTOM == eVAdj)
    {
        const double fFreeHgt(rAnchorRange.getHeight() - aTextSiz.getY());

        if(SDRTEXTVERTADJUST_CENTER == eVAdj)
        {
            aTextPos.setY(aTextPos.getY() + (fFreeHgt * 0.5));
        }
        else if(SDRTEXTVERTADJUST_BOTTOM == eVAdj)
        {
            aTextPos.setY(aTextPos.getY() + fFreeHgt);
        }
    }

    rTextRange = basegfx::B2DRange(aTextPos, aTextPos + aTextSiz);
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
        DBG_ERROR("SdrTableObj::setActiveCell(), exception caught!");
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

basegfx::B2DRange SdrTableObj::getUnifiedTextRange() const
{
    // per default the text range is the whole object range
    basegfx::B2DRange aRetval(basegfx::B2DRange::getUnitB2DRange());

    if(mpImpl)
    {
        basegfx::B2DRange aRange;

        // get the absolute range
        TakeTextAnchorRangeFromCell(mpImpl->maEditPos, aRange);

        // scale back to unit range, could also be done by using an inverse of object
        // transformation's simplified scale and translate
        const double fAbsInvScaleX(basegfx::fTools::equalZero(getSdrObjectScale().getX()) ? 1.0 : 1.0 / fabs(getSdrObjectScale().getX()));
        const double fAbsInvScaleY(basegfx::fTools::equalZero(getSdrObjectScale().getY()) ? 1.0 : 1.0 / fabs(getSdrObjectScale().getY()));
        const basegfx::B2DPoint aTopLeft(aRange.getMinimum() - getSdrObjectTranslate());
        const basegfx::B2DPoint aUnitTopLeft(aTopLeft.getX() * fAbsInvScaleX, aTopLeft.getY() * fAbsInvScaleY);

        aRetval = basegfx::B2DRange(
            aUnitTopLeft,
            aUnitTopLeft + basegfx::B2DVector(
                aRange.getWidth() * fAbsInvScaleX,
                aRange.getHeight() * fAbsInvScaleY));
    }

    return aRetval;
}

// --------------------------------------------------------------------

void SdrTableObj::TakeTextAnchorRangeFromCell( const CellPos& rPos, basegfx::B2DRange& rAnchorRange ) const
{
    basegfx::B2DRange aAnkRange(sdr::legacy::GetLogicRange(*this));

    if( mpImpl )
    {
        CellRef xCell( mpImpl->getCell( rPos ) );
        if( xCell.is() )
        {
            Rectangle aTemp;
            xCell->TakeCellsTextAnchorRect(aTemp);
            aAnkRange = basegfx::B2DRange(aTemp.Left(), aTemp.Top(), aTemp.Right(), aTemp.Bottom());
        }
    }

    rAnchorRange = aAnkRange;
}

// --------------------------------------------------------------------

void SdrTableObj::TakeTextEditArea(basegfx::B2DVector* pPaperMin, basegfx::B2DVector* pPaperMax, basegfx::B2DRange* pViewInit, basegfx::B2DRange* pViewMin) const
{
    if(mpImpl)
    {
        TakeTextEditArea(mpImpl->maEditPos, pPaperMin, pPaperMax, pViewInit, pViewMin);
    }
}

// --------------------------------------------------------------------

void SdrTableObj::TakeTextEditArea(const CellPos& rPos, basegfx::B2DVector* pPaperMin, basegfx::B2DVector* pPaperMax, basegfx::B2DRange* pViewInit, basegfx::B2DRange* pViewMin) const
{
    basegfx::B2DVector aPaperMin;
    basegfx::B2DVector aPaperMax;
    basegfx::B2DRange aViewInit;

    TakeTextAnchorRangeFromCell(rPos, aViewInit);

    basegfx::B2DVector aAnkSiz(aViewInit.getRange());
    basegfx::B2DVector aMaxSiz(aAnkSiz.getX(), 1000000.0);

    if(!basegfx::fTools::equalZero(getSdrModelFromSdrObject().GetMaxObjectScale().getY()))
    {
        aMaxSiz.setY(getSdrModelFromSdrObject().GetMaxObjectScale().getY());
    }

    CellRef xCell( mpImpl->getCell( rPos ) );
    SdrTextVertAdjust eVAdj = xCell.is() ? xCell->GetTextVerticalAdjust() : SDRTEXTVERTADJUST_TOP;

    aPaperMax = aMaxSiz;
    aPaperMin.setX(aAnkSiz.getX());

    if(pViewMin)
    {
        *pViewMin = aViewInit;
        const double fYFree(aAnkSiz.getY() - aPaperMin.getY());

        if(SDRTEXTVERTADJUST_TOP == eVAdj)
        {
            *pViewMin = basegfx::B2DRange(
                pViewMin->getMinX(),
                pViewMin->getMinY(),
                pViewMin->getMaxX(),
                pViewMin->getMaxY() - fYFree);
        }
        else if(SDRTEXTVERTADJUST_BOTTOM == eVAdj)
        {
            *pViewMin = basegfx::B2DRange(
                pViewMin->getMinX(),
                pViewMin->getMinY() + fYFree,
                pViewMin->getMaxX(),
                pViewMin->getMaxY());
        }
        else
        {
            const double fNewMinY(pViewMin->getMinY() + (fYFree * 0.5));
            *pViewMin = basegfx::B2DRange(
                pViewMin->getMinX(),
                fNewMinY,
                pViewMin->getMaxX(),
                fNewMinY + aPaperMin.getY());
        }
    }


    if(IsVerticalWriting())
    {
        aPaperMin.setX(0.0);
    }
    else
    {
        aPaperMin.setY(0.0);
    }

    if(pPaperMin)
    {
        *pPaperMin = aPaperMin;
    }

    if(pPaperMax)
    {
        *pPaperMax = aPaperMax;
    }

    if(pViewInit)
    {
        *pViewInit = aViewInit;
    }
}

// --------------------------------------------------------------------

sal_uInt16 SdrTableObj::GetOutlinerViewAnchorMode() const
{
    EVAnchorMode eRet=ANCHOR_TOP_LEFT;
    CellRef xCell( getActiveCell() );
    if( xCell.is() )
    {
        SdrTextVertAdjust eV=xCell->GetTextVerticalAdjust();
//      SdrTextHorzAdjust eH=xCell->GetTextHorizontalAdjust();

//      if (eH==SDRTEXTHORZADJUST_LEFT)
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
/*
        else if (eH==SDRTEXTHORZADJUST_RIGHT)
        {
            if (eV==SDRTEXTVERTADJUST_TOP)
            {
                eRet=ANCHOR_TOP_RIGHT;
            }
            else if (eV==SDRTEXTVERTADJUST_BOTTOM)
            {
                eRet=ANCHOR_BOTTOM_RIGHT;
            }
            else
            {
                eRet=ANCHOR_VCENTER_RIGHT;
            }
        }
        else
        {
            if (eV==SDRTEXTVERTADJUST_TOP)
            {
                eRet=ANCHOR_TOP_HCENTER;
            }
            else if (eV==SDRTEXTVERTADJUST_BOTTOM)
            {
                eRet=ANCHOR_BOTTOM_HCENTER;
            }
            else
            {
                eRet=ANCHOR_VCENTER_HCENTER;
            }
        }
*/
    }
    return (sal_uInt16)eRet;
}

// --------------------------------------------------------------------

OutlinerParaObject* SdrTableObj::GetEditOutlinerParaObject() const
{
    return SdrTextObj::GetEditOutlinerParaObject();
}

// --------------------------------------------------------------------

SdrOutliner* SdrTableObj::GetCellTextEditOutliner( const CellPos& rPos ) const
{
    if( pEdtOutl && mpImpl && (mpImpl->maEditPos == rPos) )
        return pEdtOutl;
    else
        return 0;
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
        const XColorItem& rShadColItem = ((const XColorItem&)(rSet.Get(SDRATTR_SHADOWCOLOR)));
        maShadowColor = rShadColItem.GetColorValue();
        mnShadowTransparence = ((const SdrPercentItem&)(rSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue();

        mnXDistance = ((SdrMetricItem&)(rSet.Get(SDRATTR_SHADOWXDIST))).GetValue();
        mnYDistance = ((SdrMetricItem&)(rSet.Get(SDRATTR_SHADOWYDIST))).GetValue();
    }
};

// --------------------------------------------------------------------

void lcl_VertLineEnds( OutputDevice& rDev, const Point& rTop, const Point& rBottom,
        const Color& rColor, long nXOffs, long nWidth,
        const svx::frame::Style& rTopLine, const svx::frame::Style& rBottomLine )
{
    rDev.SetLineColor(rColor);              // PEN_NULL ???
    rDev.SetFillColor(rColor);

    //  Position oben/unten muss unabhaengig von der Liniendicke sein,
    //  damit der Winkel stimmt (oder X-Position auch anpassen)
    long nTopPos = rTop.Y();
    long nBotPos = rBottom.Y();

    long nTopLeft = rTop.X() + nXOffs;
    long nTopRight = nTopLeft + nWidth - 1;

    long nBotLeft = rBottom.X() + nXOffs;
    long nBotRight = nBotLeft + nWidth - 1;

    //  oben abschliessen

    if ( rTopLine.Prim() )
    {
        long nLineW = rTopLine.GetWidth();
        if (nLineW >= 2)
        {
            Point aTriangle[3];
            aTriangle[0] = Point( nTopLeft, nTopPos );      // wie aPoints[0]
            aTriangle[1] = Point( nTopRight, nTopPos );     // wie aPoints[1]
            aTriangle[2] = Point( rTop.X(), nTopPos - (nLineW - 1) / 2 );
            Polygon aTriPoly( 3, aTriangle );
            rDev.DrawPolygon( aTriPoly );
        }
    }

    //  unten abschliessen

    if ( rBottomLine.Prim() )
    {
        long nLineW = rBottomLine.GetWidth();
        if (nLineW >= 2)
        {
            Point aTriangle[3];
            aTriangle[0] = Point( nBotLeft, nBotPos );      // wie aPoints[3]
            aTriangle[1] = Point( nBotRight, nBotPos );     // wie aPoints[2]
            aTriangle[2] = Point( rBottom.X(), nBotPos - (nLineW - 1) / 2 + nLineW - 1 );
            Polygon aTriPoly( 3, aTriangle );
            rDev.DrawPolygon( aTriPoly );
        }
    }
}

void lcl_VertLine( OutputDevice& rDev, const Point& rTop, const Point& rBottom,
                    const svx::frame::Style& rLine,
                    const svx::frame::Style& rTopLine, const svx::frame::Style& rBottomLine,
                    const Color* pForceColor )
{
    if( rLine.Prim() )
    {
        svx::frame::DrawVerFrameBorderSlanted( rDev, rTop, rBottom, rLine, pForceColor );

        svx::frame::Style aScaled( rLine );
        aScaled.ScaleSelf( 1.0 / cos( svx::frame::GetVerDiagAngle( rTop, rBottom ) ) );
        if( pForceColor )
            aScaled.SetColor( *pForceColor );

        long nXOffs = (aScaled.GetWidth() - 1) / -2L;

        lcl_VertLineEnds( rDev, rTop, rBottom, aScaled.GetColor(),
            nXOffs, aScaled.Prim(), rTopLine, rBottomLine );

        if( aScaled.Secn() )
            lcl_VertLineEnds( rDev, rTop, rBottom, aScaled.GetColor(),
                nXOffs + aScaled.Prim() + aScaled.Dist(), aScaled.Secn(), rTopLine, rBottomLine );
    }
}

// --------------------------------------------------------------------

void SdrTableObj::TakeObjNameSingul(XubString& rName) const
{
    rName = ImpGetResStr(STR_ObjNameSingulTable);

    String aName( GetName() );
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

// --------------------------------------------------------------------

void SdrTableObj::TakeObjNamePlural(XubString& rName) const
{
    rName = ImpGetResStr(STR_ObjNamePluralTable);
}

// --------------------------------------------------------------------

basegfx::B2DPolyPolygon SdrTableObj::TakeXorPoly() const
{
    return SdrTextObj::TakeXorPoly();
}

// --------------------------------------------------------------------

sal_uInt32 SdrTableObj::GetSnapPointCount() const
{
    return SdrTextObj::GetSnapPointCount();
}

// --------------------------------------------------------------------


basegfx::B2DPoint SdrTableObj::GetSnapPoint(sal_uInt32 i) const
{
    return SdrTextObj::GetSnapPoint(i);
}

// --------------------------------------------------------------------

bool SdrTableObj::BegTextEdit(SdrOutliner& rOutl)
{
    if( pEdtOutl != NULL )
        return sal_False;

    pEdtOutl = &rOutl;
    mbInEditMode = true;

    rOutl.Init( OUTLINERMODE_TEXTOBJECT );
    rOutl.SetRefDevice( getSdrModelFromSdrObject().GetReferenceDevice() );

    {
        const bool bUpdMerk(rOutl.GetUpdateMode());

        if(bUpdMerk)
        {
            rOutl.SetUpdateMode(false);
        }

        basegfx::B2DVector aPaperMin;
        basegfx::B2DVector aPaperMax;
//      basegfx::B2DRange aEditArea; // TTTT: unused, check if calculation changes when removing

//      TakeTextEditArea(&aPaperMin, &aPaperMax, &aEditArea, 0);
        TakeTextEditArea(&aPaperMin, &aPaperMax, 0, 0);

        const Size aPaperMinSize(basegfx::fround(aPaperMin.getX()), basegfx::fround(aPaperMin.getY()));
        const Size aPaperMaxSize(basegfx::fround(aPaperMax.getX()), basegfx::fround(aPaperMax.getY()));

        rOutl.SetMinAutoPaperSize(aPaperMinSize);
        rOutl.SetMaxAutoPaperSize(aPaperMaxSize);
        rOutl.SetPaperSize(aPaperMaxSize);

        if(bUpdMerk)
        {
            rOutl.SetUpdateMode(true);
        }
    }

    sal_uIntPtr nStat(rOutl.GetControlWord());
//  nStat &= ~EE_CNTRL_AUTOPAGESIZE;
    nStat |= EE_CNTRL_AUTOPAGESIZE;
    nStat &= ~EE_CNTRL_STRETCHING;

    rOutl.SetControlWord(nStat);

    OutlinerParaObject* pPara = GetOutlinerParaObject();

    if(pPara)
    {
        rOutl.SetText(*pPara);
    }

    rOutl.UpdateFields();
    rOutl.ClearModifyFlag();

    return sal_True;
}

// --------------------------------------------------------------------

void SdrTableObj::EndTextEdit(SdrOutliner& rOutl)
{
    if(rOutl.IsModified())
    {
        if( getSdrModelFromSdrObject().IsUndoEnabled() )
        {
            getSdrModelFromSdrObject().AddUndo( getSdrModelFromSdrObject().GetSdrUndoFactory().CreateUndoGeoObject(*this) );
        }

        OutlinerParaObject* pNewText = 0;
        Paragraph* p1stPara = rOutl.GetParagraph( 0 );
        sal_uInt32 nParaAnz = rOutl.GetParagraphCount();

        if(p1stPara)
        {
            if(nParaAnz == 1)
            {
                // if its only one paragraph, check if it is empty
                XubString aStr(rOutl.GetText(p1stPara));

                if(!aStr.Len())
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
                pNewText = rOutl.CreateParaObject( 0, (sal_uInt16)nParaAnz );
            }
        }
        SetOutlinerParaObject(pNewText);
    }

    pEdtOutl = 0;
    rOutl.Clear();
    sal_uInt32 nStat = rOutl.GetControlWord();
    nStat &= ~EE_CNTRL_AUTOPAGESIZE;
    rOutl.SetControlWord(nStat);

    mbInEditMode = false;
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

void SdrTableObj::SetOutlinerParaObject( OutlinerParaObject* pTextObject)
{
    CellRef xCell( getActiveCell() );
    if( xCell.is() )
    {
        xCell->SetOutlinerParaObject( pTextObject );
        SetTextSizeDirty();
        AdjustTextFrameWidthAndHeight();
    }
}

// --------------------------------------------------------------------

void SdrTableObj::AdjustToMaxRange( const basegfx::B2DRange& rMaxRange, bool /* bShrinkOnly = false */ )
{
    const double fMyHeight(fabs(getSdrObjectScale().getY()));
    const basegfx::B2DRange aAdjustRange(
        rMaxRange.getMinX(),
        rMaxRange.getMinY(),
        rMaxRange.getMaxX(),
        rMaxRange.getMinY() + fMyHeight);

    sdr::legacy::SetLogicRange(*this, aAdjustRange);
}

// --------------------------------------------------------------------

void SdrTableObj::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
    // remember original scaling
    const basegfx::B2DVector aScaleBefore(basegfx::absolute(getSdrObjectScale()));

    // call parent
    SdrTextObj::setSdrObjectTransformation(rTransformation);

    // get new scaling
    const basegfx::B2DVector aNewSize(basegfx::absolute(getSdrObjectScale()));

    if(!aScaleBefore.equal(aNewSize))
    {
        // react on scale change
        AdjustTextFrameWidthAndHeight(
            basegfx::fTools::equal(aScaleBefore.getY(), aNewSize.getY()),
            basegfx::fTools::equal(aScaleBefore.getX(), aNewSize.getX()));
    }

    if( mpImpl )
    {
        Rectangle aRectangle(sdr::legacy::GetLogicRect(*this));
        mpImpl->UpdateCells( aRectangle );
    }
}

// --------------------------------------------------------------------

bool SdrTableObj::AdjustTextFrameWidthAndHeight(bool bHgt, bool bWdt)
{
    const basegfx::B2DRange aOldRange(getSdrObjectTranslate(), getSdrObjectTranslate() + basegfx::absolute(getSdrObjectScale()));
    const basegfx::B2DRange aNewRange(AdjustTextFrameWidthAndHeight(aOldRange, bHgt, bWdt));

    if(!aOldRange.equal(aNewRange))
    {
        sdr::legacy::SetLogicRange(*this, aNewRange);

        return true;
    }

    return false;
}

// --------------------------------------------------------------------

basegfx::B2DRange SdrTableObj::AdjustTextFrameWidthAndHeight(const basegfx::B2DRange& rRange, bool bHeight, bool bWidth) const
{
    if(rRange.isEmpty() || !mpImpl || !mpImpl->mxTable.is())
    {
        // no change
        return rRange;
    }
    else
    {
        basegfx::B2DRange aNewRange(rRange);

        mpImpl->LayoutTable(aNewRange, !bWidth, !bHeight);
        return aNewRange;
    }
}

// --------------------------------------------------------------------

void SdrTableObj::ReformatText()
{
    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
    AdjustTextFrameWidthAndHeight();
    SetChanged();
}

// --------------------------------------------------------------------

bool SdrTableObj::IsVerticalWriting() const
{
    const SvxWritingModeItem* pModeItem = dynamic_cast< const SvxWritingModeItem* >( &GetObjectItem( SDRATTR_TEXTDIRECTION ) );
    return pModeItem && pModeItem->GetValue() == com::sun::star::text::WritingMode_TB_RL;
}

// --------------------------------------------------------------------

void SdrTableObj::SetVerticalWriting(bool bVertical )
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
    WritingMode eMode = WritingMode_LR_TB;
    if( mpImpl && mpImpl->mpLayouter )
        eMode = mpImpl->mpLayouter->GetWritingMode();
    return eMode;
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

        basegfx::B2DPoint aObjectTopLeft(getSdrObjectTranslate());
        aObjectTopLeft.setY(aObjectTopLeft.getY() + nEdge);

        TableEdgeHdl* pHdl = new TableEdgeHdl(rHdlList, *this, aObjectTopLeft, true, nEdgeMin, nEdgeMax, nColCount + 1);
        pHdl->SetPointNum( nRow );
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

        basegfx::B2DPoint aObjectTopLeft(getSdrObjectTranslate());
        aObjectTopLeft.setX(aObjectTopLeft.getX() + nEdge);

        TableEdgeHdl* pHdl = new TableEdgeHdl(rHdlList, *this, aObjectTopLeft, false, nEdgeMin, nEdgeMax, nRowCount + 1);
        pHdl->SetPointNum( nCol );
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

    // add standard object handles from SdrTextObj
    // add handles from parent and remember their range
    const sal_uInt32 nStartMoveOutside(rHdlList.GetHdlCount());
    SdrTextObj::AddToHdlList(rHdlList);
    const sal_uInt32 nHdlCount(rHdlList.GetHdlCount());

    for(sal_uInt32 a(nStartMoveOutside); a < nHdlCount; a++)
    {
        // extend information at handles
        SdrHdl* pCandidate = rHdlList.GetHdlByIndex(a);
        pCandidate->SetMoveOutside( true );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Draging

bool SdrTableObj::hasSpecialDrag() const
{
    return true;
}

bool SdrTableObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl = rDrag.GetActiveHdl();
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
    const SdrHdl* pHdl = rDrag.GetActiveHdl();
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
            bRet = SdrTextObj::applySpecialDrag(rDrag);
            break;
        }

        case HDL_MOVE:
        {
               sdr::legacy::MoveSdrObject(*this, Size( rDrag.GetDX(), rDrag.GetDY() ) );
            break;
        }

        case HDL_USER:
        {
            rDrag.SetEndDragChangesAttributes(false);
            rDrag.SetNoSnap(true);
            const TableEdgeHdl* pEdgeHdl = dynamic_cast< const TableEdgeHdl* >( pHdl );

            if( pEdgeHdl )
            {
                if( IsObjectInserted() )
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

String SdrTableObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    return SdrTextObj::getSpecialDragComment( rDrag );
}

basegfx::B2DPolyPolygon SdrTableObj::getSpecialDragPoly(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aRetval;
    const SdrHdl* pHdl = rDrag.GetActiveHdl();

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

void SdrTableObj::BrkCreate(SdrDragStat& /*rStat*/)
{
}

// --------------------------------------------------------------------

bool SdrTableObj::BckCreate(SdrDragStat& /*rStat*/)
{
    return true;
}

// --------------------------------------------------------------------

Pointer SdrTableObj::GetCreatePointer(const SdrView& /*rSdrView*/) const
{
    return Pointer(POINTER_CROSS);
}

// --------------------------------------------------------------------

void SdrTableObj::createCell( CellRef& xNewCell )
{
    xNewCell = Cell::create( *this, 0 );
}

// --------------------------------------------------------------------

SdrTableObj* SdrTableObj::CloneRange( const CellPos& rStart, const CellPos& rEnd )
{
    const sal_Int32 nColumns = rEnd.mnCol - rStart.mnCol + 1;
    const sal_Int32 nRows = rEnd.mnRow - rStart.mnRow + 1;

    SdrTableObj* pNewTableObj = new SdrTableObj(
        getSdrModelFromSdrObject(),
        getSdrObjectTransformation(),
        nColumns,
        nRows);
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
            DBG_ERROR( "svx::SvxTableController::GetMarkedObjModel(), exception caught!" );
        }
    }

    // copy row heights
    Reference< XTableRows > xNewRows( xNewTable->getRows(), UNO_QUERY_THROW );
    const OUString sHeight( RTL_CONSTASCII_USTRINGPARAM( "Height" ) );
    for( sal_Int32 nRow = 0; nRow < nRows; ++nRow )
    {
        Reference< XPropertySet > xNewSet( xNewRows->getByIndex( nRow ), UNO_QUERY_THROW );
        xNewSet->setPropertyValue( sHeight, Any( mpImpl->mpLayouter->getRowHeight( rStart.mnRow + nRow ) ) );
    }

    // copy column widths
    Reference< XTableColumns > xNewColumns( xNewTable->getColumns(), UNO_QUERY_THROW );
    const OUString sWidth( RTL_CONSTASCII_USTRINGPARAM( "Width" ) );
    for( sal_Int32 nCol = 0; nCol < nColumns; ++nCol )
    {
        Reference< XPropertySet > xNewSet( xNewColumns->getByIndex( nCol ), UNO_QUERY_THROW );
        xNewSet->setPropertyValue( sWidth, Any( mpImpl->mpLayouter->getColumnWidth( rStart.mnCol + nCol ) ) );
    }

    pNewTableObj->ReformatText();

    // TTTT: Needed? Check...
    sdr::legacy::SetLogicRange(*pNewTableObj, pNewTableObj->getObjectRange(0) );

    return pNewTableObj;
}

// --------------------------------------------------------------------

void SdrTableObj::DistributeColumns( sal_Int32 nFirstColumn, sal_Int32 nLastColumn )
{
    if( mpImpl && mpImpl->mpLayouter )
    {
        TableModelNotifyGuard aGuard( mpImpl->mxTable.get() );
        basegfx::B2DRange aObjectRange(
            getSdrObjectTranslate(),
            getSdrObjectTranslate() + basegfx::absolute(getSdrObjectScale()));

        mpImpl->mpLayouter->DistributeColumns( aObjectRange, nFirstColumn, nLastColumn );
    }
}

// --------------------------------------------------------------------

void SdrTableObj::DistributeRows( sal_Int32 nFirstRow, sal_Int32 nLastRow )
{
    if( mpImpl && mpImpl->mpLayouter )
    {
        TableModelNotifyGuard aGuard( mpImpl->mxTable.get() );
        basegfx::B2DRange aObjectRange(
            getSdrObjectTranslate(),
            getSdrObjectTranslate() + basegfx::absolute(getSdrObjectScale()));

        mpImpl->mpLayouter->DistributeRows( aObjectRange, nFirstRow, nLastRow );
    }
}

// --------------------------------------------------------------------

void SdrTableObj::SetChanged()
{
    if( mpImpl )
    {
        if( mpImpl->UpdateWritingMode() )
        {
            basegfx::B2DRange aObjectRange(
                getSdrObjectTranslate(),
                getSdrObjectTranslate() + basegfx::absolute(getSdrObjectScale()));

            mpImpl->LayoutTable( aObjectRange, false, false );
        }
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
