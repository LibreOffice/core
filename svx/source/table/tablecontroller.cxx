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

#include <algorithm>

#include <svx/sdr/table/tablecontroller.hxx>
#include <tablemodel.hxx>

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>

#include <sal/config.h>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <svl/whiter.hxx>

#include <sfx2/request.hxx>

#include <editeng/scripttypeitem.hxx>
#include <svx/svdotable.hxx>
#include <svx/sdr/overlay/overlayobjectcell.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/svxids.hrc>
#include <editeng/outlobj.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdetc.hxx>
#include <editeng/editobj.hxx>
#include "editeng/editstat.hxx"
#include "editeng/unolingu.hxx"
#include "svx/sdrpagewindow.hxx"
#include <svx/selectioncontroller.hxx>
#include <svx/svdmodel.hxx>
#include "svx/sdrpaintwindow.hxx"
#include <svx/svxdlg.hxx>
#include <editeng/boxitem.hxx>
#include "cell.hxx"
#include <editeng/borderline.hxx>
#include <editeng/colritem.hxx>
#include "editeng/lineitem.hxx"
#include "svx/svdstr.hrc"
#include "svdglob.hxx"
#include "svx/svdpage.hxx"
#include "tableundo.hxx"
#include "tablelayouter.hxx"
#include <vcl/msgbox.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <memory>
#include <o3tl/enumarray.hxx>
#include <o3tl/enumrange.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/viewsh.hxx>

using ::editeng::SvxBorderLine;
using namespace sdr::table;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;

namespace sdr { namespace table {

class SvxTableControllerModifyListener : public ::cppu::WeakImplHelper< css::util::XModifyListener >
{
public:
    explicit SvxTableControllerModifyListener( SvxTableController* pController )
        : mpController( pController ) {}

    // XModifyListener
    virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) throw (css::uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

    SvxTableController* mpController;
};


// XModifyListener


void SAL_CALL SvxTableControllerModifyListener::modified( const css::lang::EventObject&  ) throw (css::uno::RuntimeException, std::exception)
{
    if( mpController )
        mpController->onTableModified();
}


// XEventListener


void SAL_CALL SvxTableControllerModifyListener::disposing( const css::lang::EventObject&  ) throw (css::uno::RuntimeException, std::exception)
{
    mpController = nullptr;
}


// class SvxTableController


rtl::Reference< sdr::SelectionController > CreateTableController( SdrObjEditView* pView, const SdrObject* pObj, const rtl::Reference< sdr::SelectionController >& xRefController )
{
    return SvxTableController::create( pView, pObj, xRefController );
}


rtl::Reference< sdr::SelectionController > SvxTableController::create( SdrObjEditView* pView, const SdrObject* pObj, const rtl::Reference< sdr::SelectionController >& xRefController )
{
    if( xRefController.is() )
    {
        SvxTableController* pController = dynamic_cast< SvxTableController* >( xRefController.get() );
        if( pController && (pController->mxTableObj.get() == pObj) && (pController->mpView == pView)  )
            return xRefController;
    }
    return new SvxTableController( pView, pObj );
}


SvxTableController::SvxTableController( SdrObjEditView* pView, const SdrObject* pObj )
: mbCellSelectionMode(false)
, mbLeftButtonDown(false)
, mpSelectionOverlay(nullptr)
, mpView( dynamic_cast< SdrView* >( pView ) )
, mxTableObj( dynamic_cast< SdrTableObj* >( const_cast< SdrObject* >( pObj ) ) )
, mpModel( nullptr )
, mnUpdateEvent( nullptr )
{
    if( pObj )
    {
        mpModel = pObj->GetModel();

        if( mxTableObj.is() )
        {
            static_cast< const SdrTableObj* >( pObj )->getActiveCellPos( maCursorFirstPos );
            maCursorLastPos = maCursorFirstPos;

            Reference< XTable > xTable( static_cast< const SdrTableObj* >( pObj )->getTable() );
            if( xTable.is() )
            {
                mxModifyListener = new SvxTableControllerModifyListener( this );
                xTable->addModifyListener( mxModifyListener );

                mxTable.set( dynamic_cast< TableModel* >( xTable.get() ) );
            }
        }
    }
}

SvxTableController::~SvxTableController()
{
    if( mnUpdateEvent )
    {
        Application::RemoveUserEvent( mnUpdateEvent );
    }

    if( mxModifyListener.is() && mxTableObj.get() )
    {
        Reference< XTable > xTable( static_cast< SdrTableObj* >( mxTableObj.get() )->getTable() );
        if( xTable.is() )
        {
            xTable->removeModifyListener( mxModifyListener );
            mxModifyListener.clear();
        }
    }
}


const sal_uInt16 ACTION_NONE = 0;
const sal_uInt16 ACTION_GOTO_FIRST_CELL = 1;
const sal_uInt16 ACTION_GOTO_FIRST_COLUMN = 2;
const sal_uInt16 ACTION_GOTO_FIRST_ROW = 3;
const sal_uInt16 ACTION_GOTO_LEFT_CELL = 4;
const sal_uInt16 ACTION_GOTO_UP_CELL = 5;
const sal_uInt16 ACTION_GOTO_RIGHT_CELL = 6;
const sal_uInt16 ACTION_GOTO_DOWN_CELL = 7;
const sal_uInt16 ACTION_GOTO_LAST_CELL = 8;
const sal_uInt16 ACTION_GOTO_LAST_COLUMN = 9;
const sal_uInt16 ACTION_GOTO_LAST_ROW = 10;
const sal_uInt16 ACTION_EDIT_CELL = 11;
const sal_uInt16 ACTION_STOP_TEXT_EDIT = 12;
const sal_uInt16 ACTION_REMOVE_SELECTION = 13;
const sal_uInt16 ACTION_START_SELECTION = 14;
const sal_uInt16 ACTION_HANDLED_BY_VIEW = 15;
const sal_uInt16 ACTION_TAB = 18;

bool SvxTableController::onKeyInput(const KeyEvent& rKEvt, vcl::Window* pWindow )
{
    if( !checkTableObject() )
        return false;

    // check if we are read only
    if( mpModel && mpModel->IsReadOnly())
    {
        switch( rKEvt.GetKeyCode().GetCode() )
        {
        case awt::Key::DOWN:
        case awt::Key::UP:
        case awt::Key::LEFT:
        case awt::Key::RIGHT:
        case awt::Key::TAB:
        case awt::Key::HOME:
        case awt::Key::END:
        case awt::Key::NUM2:
        case awt::Key::NUM4:
        case awt::Key::NUM6:
        case awt::Key::NUM8:
        case awt::Key::ESCAPE:
        case awt::Key::F2:
            break;
        default:
            // tell the view we eat the event, no further processing needed
            return true;
        }
    }

    sal_uInt16 nAction = getKeyboardAction( rKEvt, pWindow );

    return executeAction( nAction, rKEvt.GetKeyCode().IsShift(), pWindow );
}

namespace {

Point pixelToLogic(const Point& rPoint, vcl::Window* pWindow)
{
    if (!pWindow)
        return rPoint;

    return pWindow->PixelToLogic(rPoint);
}

}

bool SvxTableController::onMouseButtonDown(const MouseEvent& rMEvt, vcl::Window* pWindow )
{
    if (comphelper::LibreOfficeKit::isActive() && !pWindow)
    {
        // Tiled rendering: get the window that has the disabled map mode.
        if (OutputDevice* pOutputDevice = mpView->GetFirstOutputDevice())
        {
            if (pOutputDevice->GetOutDevType() == OUTDEV_WINDOW)
                pWindow = static_cast<vcl::Window*>(pOutputDevice);
        }
    }

    if( !pWindow || !checkTableObject() )
        return false;

    SdrViewEvent aVEvt;
    if( !rMEvt.IsRight() && mpView->PickAnything(rMEvt,SdrMouseEventKind::BUTTONDOWN, aVEvt) == SDRHIT_HANDLE )
        return false;

    TableHitKind eHit = static_cast< SdrTableObj* >(mxTableObj.get())->CheckTableHit(pixelToLogic(rMEvt.GetPosPixel(), pWindow), maMouseDownPos.mnCol, maMouseDownPos.mnRow, 0);

    mbLeftButtonDown = (rMEvt.GetClicks() == 1) && rMEvt.IsLeft();

    if( eHit == SDRTABLEHIT_CELL )
    {
        StartSelection( maMouseDownPos );
        return true;
    }

    if( rMEvt.IsRight() && eHit != SDRTABLEHIT_NONE )
        return true; // right click will become context menu

    // for cell selection with the mouse remember our first hit
    if( mbLeftButtonDown )
    {
        RemoveSelection();

        SdrHdl* pHdl = mpView->PickHandle(pixelToLogic(rMEvt.GetPosPixel(), pWindow));

        if( pHdl )
        {
            mbLeftButtonDown = false;
        }
        else
        {
            sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mxTableObj.get() );

            if( !pWindow || !pTableObj || eHit  == SDRTABLEHIT_NONE)
            {
                mbLeftButtonDown = false;
            }
        }
    }

    if (comphelper::LibreOfficeKit::isActive() && rMEvt.GetClicks() == 2 && rMEvt.IsLeft() && eHit == SDRTABLEHIT_CELLTEXTAREA)
    {
        bool bEmptyOutliner = false;
        if (Outliner* pOutliner = mpView->GetTextEditOutliner())
        {
            if (pOutliner->GetParagraphCount() == 1)
            {
                if (Paragraph* pParagraph = pOutliner->GetParagraph(0))
                    bEmptyOutliner = pOutliner->GetText(pParagraph).isEmpty();
            }
        }
        if (bEmptyOutliner)
        {
            // Tiled rendering: a left double-click in an empty cell: select it.
            StartSelection(maMouseDownPos);
            setSelectedCells(maMouseDownPos, maMouseDownPos);
            // Update graphic selection, should be hidden now.
            mpView->AdjustMarkHdl();
            return true;
        }
    }

    return false;
}


bool SvxTableController::onMouseButtonUp(const MouseEvent& rMEvt, vcl::Window* /*pWin*/)
{
    if( !checkTableObject() )
        return false;

    mbLeftButtonDown = false;

    if( rMEvt.GetClicks() == 2 )
        return true;

    return false;
}


bool SvxTableController::onMouseMove(const MouseEvent& rMEvt, vcl::Window* pWindow )
{
    if( !checkTableObject() )
        return false;

    SdrTableObj* pTableObj = dynamic_cast< SdrTableObj* >( mxTableObj.get() );
    CellPos aPos;
    if (mbLeftButtonDown && pTableObj && pTableObj->CheckTableHit(pixelToLogic(rMEvt.GetPosPixel(), pWindow), aPos.mnCol, aPos.mnRow, 0 ) != SDRTABLEHIT_NONE)
    {
        if(aPos != maMouseDownPos)
        {
            if( mbCellSelectionMode )
            {
                setSelectedCells( maMouseDownPos, aPos );
                return true;
            }
            else
            {
                StartSelection( maMouseDownPos );
            }
        }
        else if( mbCellSelectionMode )
        {
            UpdateSelection( aPos );
            return true;
        }
    }
    return false;
}


void SvxTableController::onSelectionHasChanged()
{
    bool bSelected = false;

    SdrTableObj* pTableObj = dynamic_cast< SdrTableObj* >( mxTableObj.get() );
    if( pTableObj && pTableObj->IsTextEditActive() )
    {
        pTableObj->getActiveCellPos( maCursorFirstPos );
        maCursorLastPos = maCursorFirstPos;
        mbCellSelectionMode = false;
    }
    else
    {
        const SdrMarkList& rMarkList= mpView->GetMarkedObjectList();
        if( rMarkList.GetMarkCount() == 1 )
            bSelected = mxTableObj.get() == rMarkList.GetMark(0)->GetMarkedSdrObj();
        /* fdo#46186 Selecting the table means selecting the entire cells */
        if (!hasSelectedCells() && pTableObj)
        {
            maCursorFirstPos = SdrTableObj::getFirstCell();
            maCursorLastPos = pTableObj->getLastCell();
            mbCellSelectionMode=true;
        }
    }

    if( bSelected )
    {
        updateSelectionOverlay();
    }
    else
    {
        destroySelectionOverlay();
    }
}


void SvxTableController::GetState( SfxItemSet& rSet )
{
    if( !mxTable.is() || !mxTableObj.is() || !mxTableObj->GetModel() )
        return;

    std::unique_ptr<SfxItemSet> xSet;

    bool bVertDone = false;

    // Iterate over all requested items in the set.
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_TABLE_VERT_BOTTOM:
            case SID_TABLE_VERT_CENTER:
            case SID_TABLE_VERT_NONE:
                {
                    if( !mxTable.is() || !mxTableObj->GetModel() )
                    {
                        rSet.DisableItem(nWhich);
                    }
                    else if(!bVertDone)
                    {
                        if (!xSet)
                        {
                            xSet.reset(new SfxItemSet( mxTableObj->GetModel()->GetItemPool() ));
                            MergeAttrFromSelectedCells(*xSet, false);
                        }

                        SdrTextVertAdjust eAdj = SDRTEXTVERTADJUST_BLOCK;

                        if (xSet->GetItemState( SDRATTR_TEXT_VERTADJUST ) != SfxItemState::DONTCARE)
                            eAdj = static_cast<const SdrTextVertAdjustItem&>(xSet->Get(SDRATTR_TEXT_VERTADJUST)).GetValue();

                        rSet.Put(SfxBoolItem(SID_TABLE_VERT_BOTTOM, eAdj == SDRTEXTVERTADJUST_BOTTOM));
                        rSet.Put(SfxBoolItem(SID_TABLE_VERT_CENTER, eAdj == SDRTEXTVERTADJUST_CENTER));
                        rSet.Put(SfxBoolItem(SID_TABLE_VERT_NONE, eAdj == SDRTEXTVERTADJUST_TOP));
                        bVertDone = true;
                    }
                    break;
                }
            case SID_TABLE_DELETE_ROW:
                if( !mxTable.is() || !hasSelectedCells() || (mxTable->getRowCount() <= 1) )
                    rSet.DisableItem(SID_TABLE_DELETE_ROW);
                break;
            case SID_TABLE_DELETE_COL:
                if( !mxTable.is() || !hasSelectedCells() || (mxTable->getColumnCount() <= 1) )
                    rSet.DisableItem(SID_TABLE_DELETE_COL);
                break;
            case SID_TABLE_MERGE_CELLS:
                if( !mxTable.is() || !hasSelectedCells() )
                    rSet.DisableItem(SID_TABLE_MERGE_CELLS);
                break;
            case SID_TABLE_SPLIT_CELLS:
                if( !hasSelectedCells() || !mxTable.is() )
                    rSet.DisableItem(SID_TABLE_SPLIT_CELLS);
                break;

            case SID_OPTIMIZE_TABLE:
            case SID_TABLE_DISTRIBUTE_COLUMNS:
            case SID_TABLE_DISTRIBUTE_ROWS:
            {
                bool bDistributeColumns = false;
                bool bDistributeRows = false;
                if( mxTable.is() )
                {
                    CellPos aStart, aEnd;
                    getSelectedCells( aStart, aEnd );

                    bDistributeColumns = aStart.mnCol != aEnd.mnCol;
                    bDistributeRows = aStart.mnRow != aEnd.mnRow;
                }
                if( !bDistributeColumns && !bDistributeRows )
                    rSet.DisableItem(SID_OPTIMIZE_TABLE);
                if( !bDistributeColumns )
                    rSet.DisableItem(SID_TABLE_DISTRIBUTE_COLUMNS);
                if( !bDistributeRows )
                    rSet.DisableItem(SID_TABLE_DISTRIBUTE_ROWS);
                break;
            }

            case SID_AUTOFORMAT:
            case SID_TABLE_SORT_DIALOG:
            case SID_TABLE_AUTOSUM:
//              if( !mxTable.is() )
//                  rSet.DisableItem( nWhich );
                break;
            default:
                break;
        }
        nWhich = aIter.NextWhich();
    }
}


void SvxTableController::onInsert( sal_uInt16 nSId, const SfxItemSet* pArgs )
{
    sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mxTableObj.get() );
    if( !pTableObj )
        return;

    if( mxTable.is() ) try
    {

        bool bInsertAfter = true;
        sal_uInt16 nCount = 0;
        if( pArgs )
        {
            const SfxPoolItem* pItem = nullptr;
            pArgs->GetItemState(nSId, false, &pItem);
            if (pItem)
            {
                nCount = static_cast<const SfxInt16Item*>(pItem)->GetValue();
                if(SfxItemState::SET == pArgs->GetItemState(SID_TABLE_PARAM_INSERT_AFTER, true, &pItem))
                    bInsertAfter = static_cast<const SfxBoolItem*>(pItem)->GetValue();
            }
        }

        CellPos aStart, aEnd;
        if( hasSelectedCells() )
        {
            getSelectedCells( aStart, aEnd );
        }
        else
        {
            if( bInsertAfter )
            {
                aStart.mnCol = mxTable->getColumnCount() - 1;
                aStart.mnRow = mxTable->getRowCount() - 1;
                aEnd = aStart;
            }
        }

        if( pTableObj->IsTextEditActive() )
            mpView->SdrEndTextEdit(true);

        RemoveSelection();

        const OUString sSize( "Size" );

        const bool bUndo = mpModel && mpModel->IsUndoEnabled();

        switch( nSId )
        {
        case SID_TABLE_INSERT_COL:
        {
            TableModelNotifyGuard aGuard( mxTable.get() );

            if( bUndo )
            {
                mpModel->BegUndo( ImpGetResStr(STR_TABLE_INSCOL) );
                mpModel->AddUndo( mpModel->GetSdrUndoFactory().CreateUndoGeoObject(*pTableObj) );
            }

            Reference< XTableColumns > xCols( mxTable->getColumns() );
            const sal_Int32 nNewColumns = (nCount == 0) ? (aEnd.mnCol - aStart.mnCol + 1) : nCount;
            const sal_Int32 nNewStartColumn = aEnd.mnCol + (bInsertAfter ? 1 : 0);
            xCols->insertByIndex( nNewStartColumn, nNewColumns );

            for( sal_Int32 nOffset = 0; nOffset < nNewColumns; nOffset++ )
            {
                // Resolves fdo#61540
                // On Insert before, the reference column whose size is going to be
                // used for newly created column(s) is wrong. As the new columns are
                // inserted before the reference column, the reference column moved
                // to the new position by no., of new columns i.e (earlier+newcolumns).
                Reference< XPropertySet >(xCols->getByIndex(nNewStartColumn+nOffset), UNO_QUERY_THROW )->
                    setPropertyValue( sSize,
                        Reference< XPropertySet >(xCols->getByIndex( bInsertAfter?nNewStartColumn-1:nNewStartColumn+nNewColumns ), UNO_QUERY_THROW )->
                            getPropertyValue( sSize ) );
            }

            // Copy cell properties
            sal_Int32 nPropSrcCol = (bInsertAfter ? aEnd.mnCol : aStart.mnCol + nNewColumns);
            sal_Int32 nRowSpan = 0;
            bool bNewSpan = false;

            for( sal_Int32 nRow = 0; nRow < mxTable->getRowCount(); ++nRow )
            {
                CellRef xSourceCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nPropSrcCol, nRow ).get() ) );

                // When we insert new COLUMNs, we want to copy ROW spans.
                if (xSourceCell.is() && nRowSpan == 0)
                {
                    // we are not in a span yet. Let's find out if the current cell is in a span.
                    sal_Int32 nColSpan = sal_Int32();
                    sal_Int32 nSpanInfoCol = sal_Int32();

                    if( xSourceCell->getRowSpan() > 1 )
                    {
                        // The current cell is the top-left cell in a span.
                        // Get the span info and propagate it to the target.
                        nRowSpan = xSourceCell->getRowSpan();
                        nColSpan = xSourceCell->getColumnSpan();
                        nSpanInfoCol = nPropSrcCol;
                    }
                    else if( xSourceCell->isMerged() )
                    {
                        // The current cell is a middle cell in a 2D span.
                        // Look for the top-left cell in the span.
                        for( nSpanInfoCol = nPropSrcCol - 1; nSpanInfoCol >= 0; --nSpanInfoCol )
                        {
                            CellRef xMergeInfoCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nSpanInfoCol, nRow ).get() ) );
                            if (xMergeInfoCell.is() && !xMergeInfoCell->isMerged())
                            {
                                nRowSpan = xMergeInfoCell->getRowSpan();
                                nColSpan = xMergeInfoCell->getColumnSpan();
                                break;
                            }
                        }
                        if( nRowSpan == 1 )
                            nRowSpan = 0;
                    }

                    // The target columns are outside the span; Start a new span.
                    if( nRowSpan > 0 && ( nNewStartColumn < nSpanInfoCol || nSpanInfoCol + nColSpan <= nNewStartColumn ) )
                        bNewSpan = true;
                }

                // Now copy the properties from the source to the targets
                for( sal_Int32 nOffset = 0; nOffset < nNewColumns; nOffset++ )
                {
                    CellRef xTargetCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nNewStartColumn + nOffset, nRow ).get() ) );
                    if( xTargetCell.is() )
                    {
                        if( nRowSpan > 0 )
                        {
                            if( bNewSpan )
                                xTargetCell->merge( 1, nRowSpan );
                            else
                                xTargetCell->setMerged();
                        }
                        xTargetCell->copyFormatFrom( xSourceCell );
                    }
                }

                if( nRowSpan > 0 )
                {
                    --nRowSpan;
                    bNewSpan = false;
                }
            }

            if( bUndo )
                mpModel->EndUndo();

            aStart.mnCol = nNewStartColumn;
            aStart.mnRow = 0;
            aEnd.mnCol = aStart.mnCol + nNewColumns - 1;
            aEnd.mnRow = mxTable->getRowCount() - 1;
            break;
        }

        case SID_TABLE_INSERT_ROW:
        {
            TableModelNotifyGuard aGuard( mxTable.get() );

            if( bUndo )
            {
                mpModel->BegUndo( ImpGetResStr(STR_TABLE_INSROW ) );
                mpModel->AddUndo( mpModel->GetSdrUndoFactory().CreateUndoGeoObject(*pTableObj) );
            }

            Reference< XTableRows > xRows( mxTable->getRows() );
            const sal_Int32 nNewRows = (nCount == 0) ? (aEnd.mnRow - aStart.mnRow + 1) : nCount;
            const sal_Int32 nNewRowStart = aEnd.mnRow + (bInsertAfter ? 1 : 0);
            xRows->insertByIndex( nNewRowStart, nNewRows );

            for( sal_Int32 nOffset = 0; nOffset < nNewRows; nOffset++ )
            {
                Reference< XPropertySet >( xRows->getByIndex( aEnd.mnRow + nOffset + 1 ), UNO_QUERY_THROW )->
                    setPropertyValue( sSize,
                        Reference< XPropertySet >( xRows->getByIndex( aStart.mnRow + nOffset ), UNO_QUERY_THROW )->
                            getPropertyValue( sSize ) );
            }

            // Copy the cell properties
            sal_Int32 nPropSrcRow = (bInsertAfter ? aEnd.mnRow : aStart.mnRow + nNewRows);
            sal_Int32 nColSpan = 0;
            bool bNewSpan = false;

            for( sal_Int32 nCol = 0; nCol < mxTable->getColumnCount(); ++nCol )
            {
                CellRef xSourceCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nPropSrcRow ).get() ) );

                if (!xSourceCell.is())
                    continue;

                // When we insert new ROWs, we want to copy COLUMN spans.
                if( nColSpan == 0 )
                {
                    // we are not in a span yet. Let's find out if the current cell is in a span.
                    sal_Int32 nRowSpan = sal_Int32();
                    sal_Int32 nSpanInfoRow = sal_Int32();

                    if( xSourceCell->getColumnSpan() > 1 )
                    {
                        // The current cell is the top-left cell in a span.
                        // Get the span info and propagate it to the target.
                        nColSpan = xSourceCell->getColumnSpan();
                        nRowSpan = xSourceCell->getRowSpan();
                        nSpanInfoRow = nPropSrcRow;
                    }
                    else if( xSourceCell->isMerged() )
                    {
                        // The current cell is a middle cell in a 2D span.
                        // Look for the top-left cell in the span.
                        for( nSpanInfoRow = nPropSrcRow - 1; nSpanInfoRow >= 0; --nSpanInfoRow )
                        {
                            CellRef xMergeInfoCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nSpanInfoRow ).get() ) );
                            if (xMergeInfoCell.is() && !xMergeInfoCell->isMerged())
                            {
                                nColSpan = xMergeInfoCell->getColumnSpan();
                                nRowSpan = xMergeInfoCell->getRowSpan();
                                break;
                            }
                        }
                        if( nColSpan == 1 )
                            nColSpan = 0;
                    }

                    // Inserted rows are outside the span; Start a new span.
                    if( nColSpan > 0 && ( nNewRowStart < nSpanInfoRow || nSpanInfoRow + nRowSpan <= nNewRowStart ) )
                        bNewSpan = true;
                }

                // Now copy the properties from the source to the targets
                for( sal_Int32 nOffset = 0; nOffset < nNewRows; ++nOffset )
                {
                    CellRef xTargetCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nNewRowStart + nOffset ).get() ) );
                    if( xTargetCell.is() )
                    {
                        if( nColSpan > 0 )
                        {
                            if( bNewSpan )
                                xTargetCell->merge( nColSpan, 1 );
                            else
                                xTargetCell->setMerged();
                        }
                        xTargetCell->copyFormatFrom( xSourceCell );
                    }
                }

                if( nColSpan > 0 )
                {
                    --nColSpan;
                    bNewSpan = false;
                }
            }

            if( bUndo )
                mpModel->EndUndo();

            aStart.mnCol = 0;
            aStart.mnRow = nNewRowStart;
            aEnd.mnCol = mxTable->getColumnCount() - 1;
            aEnd.mnRow = aStart.mnRow + nNewRows - 1;
            break;
        }
        }

        StartSelection( aStart );
        UpdateSelection( aEnd );
    }
    catch( Exception& )
    {
        OSL_FAIL("svx::SvxTableController::onInsert(), exception caught!");
    }
}


void SvxTableController::onDelete( sal_uInt16 nSId )
{
    sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mxTableObj.get() );
    if( !pTableObj )
        return;

    if( mxTable.is() && hasSelectedCells() )
    {
        CellPos aStart, aEnd;
        getSelectedCells( aStart, aEnd );

        if( pTableObj->IsTextEditActive() )
            mpView->SdrEndTextEdit(true);

        RemoveSelection();

        bool bDeleteTable = false;
        switch( nSId )
        {
        case SID_TABLE_DELETE_COL:
        {
            const sal_Int32 nRemovedColumns = aEnd.mnCol - aStart.mnCol + 1;
            if( nRemovedColumns == mxTable->getColumnCount() )
            {
                bDeleteTable = true;
            }
            else
            {
                Reference< XTableColumns > xCols( mxTable->getColumns() );
                xCols->removeByIndex( aStart.mnCol, nRemovedColumns );
            }
            break;
        }

        case SID_TABLE_DELETE_ROW:
        {
            const sal_Int32 nRemovedRows = aEnd.mnRow - aStart.mnRow + 1;
            if( nRemovedRows == mxTable->getRowCount() )
            {
                bDeleteTable = true;
            }
            else
            {
                Reference< XTableRows > xRows( mxTable->getRows() );
                xRows->removeByIndex( aStart.mnRow, nRemovedRows );
            }
            break;
        }
        }

        if( bDeleteTable )
            mpView->DeleteMarkedObj();
        else
            UpdateTableShape();
    }
}


void SvxTableController::onSelect( sal_uInt16 nSId )
{
    if( mxTable.is() )
    {
        const sal_Int32 nRowCount = mxTable->getRowCount();
        const sal_Int32 nColCount = mxTable->getColumnCount();
        if( nRowCount && nColCount )
        {
            CellPos aStart, aEnd;
            getSelectedCells( aStart, aEnd );

            switch( nSId )
            {
            case SID_TABLE_SELECT_ALL:
                aEnd.mnCol = 0; aEnd.mnRow = 0;
                aStart.mnCol = nColCount - 1; aStart.mnRow = nRowCount - 1;
                break;
            case SID_TABLE_SELECT_COL:
                aEnd.mnRow = nRowCount - 1;
                aStart.mnRow = 0;
                break;
            case SID_TABLE_SELECT_ROW:
                aEnd.mnCol = nColCount - 1;
                aStart.mnCol = 0;
                break;
            }

            StartSelection( aEnd );
            gotoCell( aStart, true, nullptr );
        }
    }
}

namespace
{
    SvxBoxItem mergeDrawinglayerTextDistancesAndSvxBoxItem(const SfxItemSet& rAttrSet)
    {
        // merge drawing layer text distance items into SvxBoxItem used by the dialog
        SvxBoxItem aBoxItem( static_cast< const SvxBoxItem& >( rAttrSet.Get( SDRATTR_TABLE_BORDER ) ) );
        aBoxItem.SetDistance( sal::static_int_cast< sal_uInt16 >( static_cast<const SdrMetricItem&>(rAttrSet.Get(SDRATTR_TEXT_LEFTDIST)).GetValue()), SvxBoxItemLine::LEFT );
        aBoxItem.SetDistance( sal::static_int_cast< sal_uInt16 >( static_cast<const SdrMetricItem&>(rAttrSet.Get(SDRATTR_TEXT_RIGHTDIST)).GetValue()), SvxBoxItemLine::RIGHT );
        aBoxItem.SetDistance( sal::static_int_cast< sal_uInt16 >( static_cast<const SdrMetricItem&>(rAttrSet.Get(SDRATTR_TEXT_UPPERDIST)).GetValue()), SvxBoxItemLine::TOP );
        aBoxItem.SetDistance( sal::static_int_cast< sal_uInt16 >( static_cast<const SdrMetricItem&>(rAttrSet.Get(SDRATTR_TEXT_LOWERDIST)).GetValue()), SvxBoxItemLine::BOTTOM );
        return aBoxItem;
    }
}

void SvxTableController::onFormatTable( SfxRequest& rReq )
{
    sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mxTableObj.get() );
    if( !pTableObj )
        return;

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs && pTableObj->GetModel() )
    {
        SfxItemSet aNewAttr( pTableObj->GetModel()->GetItemPool() );

        // merge drawing layer text distance items into SvxBoxItem used by the dialog
        SvxBoxItem aBoxItem(mergeDrawinglayerTextDistancesAndSvxBoxItem(aNewAttr));

        SvxBoxInfoItem aBoxInfoItem( static_cast< const SvxBoxInfoItem& >( aNewAttr.Get( SDRATTR_TABLE_BORDER_INNER ) ) );

        MergeAttrFromSelectedCells(aNewAttr, false);
        FillCommonBorderAttrFromSelectedCells( aBoxItem, aBoxInfoItem );
        aNewAttr.Put( aBoxItem );
        aNewAttr.Put( aBoxInfoItem );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        std::unique_ptr< SfxAbstractTabDialog > xDlg( pFact ? pFact->CreateSvxFormatCellsDialog( nullptr, &aNewAttr, pTableObj->GetModel(), pTableObj) : nullptr );
        // Even Cancel Button is returning positive(101) value,
        if (xDlg.get() && xDlg->Execute() == RET_OK)
        {
            SfxItemSet aNewSet(*(xDlg->GetOutputItemSet()));

            //Only properties that were unchanged by the dialog appear in this
            //itemset.  We had constructed these two properties from other
            //ones, so if they were not changed, then forcible set them back to
            //their originals in the new result set so we can decompose that
            //unchanged state back to their input properties
            if (aNewSet.GetItemState(SDRATTR_TABLE_BORDER, false) != SfxItemState::SET)
            {
                aNewSet.Put(aBoxItem);
            }
            if (aNewSet.GetItemState(SDRATTR_TABLE_BORDER_INNER, false) != SfxItemState::SET)
            {
                aNewSet.Put(aBoxInfoItem);
            }

            SvxBoxItem aNewBoxItem( static_cast< const SvxBoxItem& >( aNewSet.Get( SDRATTR_TABLE_BORDER ) ) );

            if( aNewBoxItem.GetDistance( SvxBoxItemLine::LEFT ) != aBoxItem.GetDistance( SvxBoxItemLine::LEFT ) )
                aNewSet.Put(makeSdrTextLeftDistItem( aNewBoxItem.GetDistance( SvxBoxItemLine::LEFT ) ) );

            if( aNewBoxItem.GetDistance( SvxBoxItemLine::RIGHT ) != aBoxItem.GetDistance( SvxBoxItemLine::RIGHT ) )
                aNewSet.Put(makeSdrTextRightDistItem( aNewBoxItem.GetDistance( SvxBoxItemLine::RIGHT ) ) );

            if( aNewBoxItem.GetDistance( SvxBoxItemLine::TOP ) != aBoxItem.GetDistance( SvxBoxItemLine::TOP ) )
                aNewSet.Put(makeSdrTextUpperDistItem( aNewBoxItem.GetDistance( SvxBoxItemLine::TOP ) ) );

            if( aNewBoxItem.GetDistance( SvxBoxItemLine::BOTTOM ) != aBoxItem.GetDistance( SvxBoxItemLine::BOTTOM ) )
                aNewSet.Put(makeSdrTextLowerDistItem( aNewBoxItem.GetDistance( SvxBoxItemLine::BOTTOM ) ) );

            SetAttrToSelectedCells(aNewSet, false);
        }
    }
}

void SvxTableController::Execute( SfxRequest& rReq )
{
    const sal_uInt16 nSId = rReq.GetSlot();
    switch( nSId )
    {
    case SID_TABLE_INSERT_ROW:
    case SID_TABLE_INSERT_COL:
        onInsert( nSId, rReq.GetArgs() );
        break;
    case SID_TABLE_DELETE_ROW:
    case SID_TABLE_DELETE_COL:
        onDelete( nSId );
        break;
    case SID_TABLE_SELECT_ALL:
    case SID_TABLE_SELECT_COL:
    case SID_TABLE_SELECT_ROW:
        onSelect( nSId );
        break;
    case SID_FORMAT_TABLE_DLG:
        onFormatTable( rReq );
        break;

    case SID_FRAME_LINESTYLE:
    case SID_FRAME_LINECOLOR:
    case SID_ATTR_BORDER:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            if( pArgs )
                ApplyBorderAttr( *pArgs );
        }
        break;

    case SID_ATTR_FILL_STYLE:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            if( pArgs )
                SetAttributes( *pArgs, false );
        }
        break;

    case SID_TABLE_MERGE_CELLS:
        MergeMarkedCells();
        break;

    case SID_TABLE_SPLIT_CELLS:
        SplitMarkedCells();
        break;

    case SID_TABLE_DISTRIBUTE_COLUMNS:
        DistributeColumns();
        break;

    case SID_TABLE_DISTRIBUTE_ROWS:
        DistributeRows();
        break;

    case SID_TABLE_VERT_BOTTOM:
    case SID_TABLE_VERT_CENTER:
    case SID_TABLE_VERT_NONE:
        SetVertical( nSId );
        break;

    case SID_AUTOFORMAT:
    case SID_TABLE_SORT_DIALOG:
    case SID_TABLE_AUTOSUM:
    default:
        break;

    case SID_TABLE_STYLE:
        SetTableStyle( rReq.GetArgs() );
        break;

    case SID_TABLE_STYLE_SETTINGS:
        SetTableStyleSettings( rReq.GetArgs() );
        break;
    }
}

void SvxTableController::SetTableStyle( const SfxItemSet* pArgs )
{
    SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mxTableObj.get() );
    SdrModel* pModel = pTableObj ? pTableObj->GetModel() : nullptr;

    if( !pTableObj || !pModel || !pArgs || (SfxItemState::SET != pArgs->GetItemState(SID_TABLE_STYLE, false)) )
        return;

    const SfxStringItem* pArg = dynamic_cast< const SfxStringItem* >( &pArgs->Get( SID_TABLE_STYLE ) );
    if( pArg && mxTable.is() ) try
    {
        Reference< XStyleFamiliesSupplier > xSFS( pModel->getUnoModel(), UNO_QUERY_THROW );
        Reference< XNameAccess > xFamilyNameAccess( xSFS->getStyleFamilies(), UNO_QUERY_THROW );
        const OUString sFamilyName( "table" );
        Reference< XNameAccess > xTableFamilyAccess( xFamilyNameAccess->getByName( sFamilyName ), UNO_QUERY_THROW );

        if( xTableFamilyAccess->hasByName( pArg->GetValue() ) )
        {
            // found table style with the same name
            Reference< XIndexAccess > xNewTableStyle( xTableFamilyAccess->getByName( pArg->GetValue() ), UNO_QUERY_THROW );

            const bool bUndo = pModel->IsUndoEnabled();

            if( bUndo )
            {
                pModel->BegUndo( ImpGetResStr(STR_TABLE_STYLE) );
                pModel->AddUndo( new TableStyleUndo( *pTableObj ) );
            }

            pTableObj->setTableStyle( xNewTableStyle );

            const sal_Int32 nRowCount = mxTable->getRowCount();
            const sal_Int32 nColCount = mxTable->getColumnCount();
            for( sal_Int32 nRow = 0; nRow < nRowCount; nRow++ )
            {
                for( sal_Int32 nCol = 0; nCol < nColCount; nCol++ ) try
                {
                    CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
                    if( xCell.is() )
                    {
                        SfxItemSet aSet( xCell->GetItemSet() );
                        bool bChanges = false;
                        SfxStyleSheet *pStyleSheet = xCell->GetStyleSheet();
                        SAL_WARN_IF(!pStyleSheet, "svx", "no stylesheet for table cell?");
                        if (pStyleSheet)
                        {
                            const SfxItemSet& rStyleAttribs = pStyleSheet->GetItemSet();

                            for ( sal_uInt16 nWhich = SDRATTR_START; nWhich <= SDRATTR_TABLE_LAST; nWhich++ )
                            {
                                if( (rStyleAttribs.GetItemState( nWhich ) == SfxItemState::SET) && (aSet.GetItemState( nWhich ) == SfxItemState::SET) )
                                {
                                    aSet.ClearItem( nWhich );
                                    bChanges = true;
                                }
                            }
                        }

                        if( bChanges )
                        {
                            if( bUndo )
                                xCell->AddUndo();

                            xCell->SetMergedItemSetAndBroadcast( aSet, true );
                        }
                    }
                }
                catch( Exception& )
                {
                    OSL_FAIL( "svx::SvxTableController::SetTableStyle(), exception caught!" );
                }
            }

            if( bUndo )
                pModel->EndUndo();
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "svx::SvxTableController::SetTableStyle(), exception caught!" );
    }
}

void SvxTableController::SetTableStyleSettings( const SfxItemSet* pArgs )
{
    SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mxTableObj.get() );
    SdrModel* pModel = pTableObj ? pTableObj->GetModel() : nullptr;

    if( !pTableObj || !pModel )
        return;

    TableStyleSettings aSettings( pTableObj->getTableStyleSettings() );

    const SfxPoolItem *pPoolItem=nullptr;

    if( (SfxItemState::SET == pArgs->GetItemState(ID_VAL_USEFIRSTROWSTYLE, false,&pPoolItem)) )
        aSettings.mbUseFirstRow = static_cast< const SfxBoolItem* >(pPoolItem)->GetValue();

    if( (SfxItemState::SET == pArgs->GetItemState(ID_VAL_USELASTROWSTYLE, false,&pPoolItem)) )
        aSettings.mbUseLastRow = static_cast< const SfxBoolItem* >(pPoolItem)->GetValue();

    if( (SfxItemState::SET == pArgs->GetItemState(ID_VAL_USEBANDINGROWSTYLE, false,&pPoolItem)) )
        aSettings.mbUseRowBanding = static_cast< const SfxBoolItem* >(pPoolItem)->GetValue();

    if( (SfxItemState::SET == pArgs->GetItemState(ID_VAL_USEFIRSTCOLUMNSTYLE, false,&pPoolItem)) )
        aSettings.mbUseFirstColumn = static_cast< const SfxBoolItem* >(pPoolItem)->GetValue();

    if( (SfxItemState::SET == pArgs->GetItemState(ID_VAL_USELASTCOLUMNSTYLE, false,&pPoolItem)) )
        aSettings.mbUseLastColumn = static_cast< const SfxBoolItem* >(pPoolItem)->GetValue();

    if( (SfxItemState::SET == pArgs->GetItemState(ID_VAL_USEBANDINGCOLUMNSTYLE, false,&pPoolItem)) )
        aSettings.mbUseColumnBanding = static_cast< const SfxBoolItem* >(pPoolItem)->GetValue();

    if( aSettings == pTableObj->getTableStyleSettings() )
        return;

    const bool bUndo = pModel->IsUndoEnabled();

    if( bUndo )
    {
        pModel->BegUndo( ImpGetResStr(STR_TABLE_STYLE_SETTINGS) );
        pModel->AddUndo( new TableStyleUndo( *pTableObj ) );
    }

    pTableObj->setTableStyleSettings( aSettings );

    if( bUndo )
        pModel->EndUndo();
}

void SvxTableController::SetVertical( sal_uInt16 nSId )
{
    SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mxTableObj.get() );
    if( mxTable.is() && pTableObj )
    {
        TableModelNotifyGuard aGuard( mxTable.get() );

        bool bUndo = mpModel && mpModel->IsUndoEnabled();
        if (bUndo)
        {
            mpModel->BegUndo(ImpGetResStr(STR_TABLE_NUMFORMAT));
            mpModel->AddUndo(mpModel->GetSdrUndoFactory().CreateUndoAttrObject(*pTableObj));
        }

        CellPos aStart, aEnd;
        getSelectedCells( aStart, aEnd );

        SdrTextVertAdjust eAdj = SDRTEXTVERTADJUST_TOP;

        switch( nSId )
        {
            case SID_TABLE_VERT_BOTTOM:
                eAdj = SDRTEXTVERTADJUST_BOTTOM;
                break;
            case SID_TABLE_VERT_CENTER:
                eAdj = SDRTEXTVERTADJUST_CENTER;
                break;
            //case SID_TABLE_VERT_NONE:
            default:
                break;
        }

        SdrTextVertAdjustItem aItem( eAdj );

        for( sal_Int32 nRow = aStart.mnRow; nRow <= aEnd.mnRow; nRow++ )
        {
            for( sal_Int32 nCol = aStart.mnCol; nCol <= aEnd.mnCol; nCol++ )
            {
                CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
                if( xCell.is() )
                {
                    if (bUndo)
                        xCell->AddUndo();
                    SfxItemSet aSet(xCell->GetItemSet());
                    aSet.Put(aItem);
                    xCell->SetMergedItemSetAndBroadcast(aSet, /*bClearAllItems=*/false);
                }
            }
        }

        UpdateTableShape();

        if (bUndo)
            mpModel->EndUndo();
    }
}

void SvxTableController::MergeMarkedCells()
{
    CellPos aStart, aEnd;
    getSelectedCells( aStart, aEnd );
    SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mxTableObj.get() );
    if( pTableObj )
    {
        if( pTableObj->IsTextEditActive() )
            mpView->SdrEndTextEdit(true);

        TableModelNotifyGuard aGuard( mxTable.get() );
        MergeRange( aStart.mnCol, aStart.mnRow, aEnd.mnCol, aEnd.mnRow );
    }
}

void SvxTableController::SplitMarkedCells()
{
    if( mxTable.is() )
    {
        CellPos aStart, aEnd;
        getSelectedCells( aStart, aEnd );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        std::unique_ptr< SvxAbstractSplittTableDialog > xDlg( pFact ? pFact->CreateSvxSplittTableDialog( nullptr, false, 99, 99 ) : nullptr );
        if( xDlg.get() && xDlg->Execute() )
        {
            const sal_Int32 nCount = xDlg->GetCount() - 1;
            if( nCount < 1 )
                return;

            getSelectedCells( aStart, aEnd );

            Reference< XMergeableCellRange > xRange( mxTable->createCursorByRange( mxTable->getCellRangeByPosition( aStart.mnCol, aStart.mnRow, aEnd.mnCol, aEnd.mnRow ) ), UNO_QUERY_THROW );

            const sal_Int32 nRowCount = mxTable->getRowCount();
            const sal_Int32 nColCount = mxTable->getColumnCount();


            SdrTableObj* pTableObj = dynamic_cast< SdrTableObj* >( mxTableObj.get() );
            if( pTableObj )
            {
                if( pTableObj->IsTextEditActive() )
                    mpView->SdrEndTextEdit(true);

                TableModelNotifyGuard aGuard( mxTable.get() );

                const bool bUndo = mpModel && mpModel->IsUndoEnabled();
                if( bUndo )
                {
                    mpModel->BegUndo( ImpGetResStr(STR_TABLE_SPLIT) );
                    mpModel->AddUndo( mpModel->GetSdrUndoFactory().CreateUndoGeoObject(*pTableObj) );
                }

                if( xDlg->IsHorizontal() )
                {
                    xRange->split( 0, nCount );
                }
                else
                {
                    xRange->split( nCount, 0 );
                }

                if( bUndo )
                    mpModel->EndUndo();
            }
            aEnd.mnRow += mxTable->getRowCount() - nRowCount;
            aEnd.mnCol += mxTable->getColumnCount() - nColCount;

            setSelectedCells( aStart, aEnd );
        }
    }
}

void SvxTableController::DistributeColumns()
{
    SdrTableObj* pTableObj = dynamic_cast< SdrTableObj* >( mxTableObj.get() );
    if( pTableObj )
    {
        const bool bUndo = mpModel && mpModel->IsUndoEnabled();
        if( bUndo )
        {
            mpModel->BegUndo( ImpGetResStr(STR_TABLE_DISTRIBUTE_COLUMNS) );
            mpModel->AddUndo( mpModel->GetSdrUndoFactory().CreateUndoGeoObject(*pTableObj) );
        }

        CellPos aStart, aEnd;
        getSelectedCells( aStart, aEnd );
        pTableObj->DistributeColumns( aStart.mnCol, aEnd.mnCol );

        if( bUndo )
            mpModel->EndUndo();
    }
}

void SvxTableController::DistributeRows()
{
    SdrTableObj* pTableObj = dynamic_cast< SdrTableObj* >( mxTableObj.get() );
    if( pTableObj )
    {
        const bool bUndo = mpModel && mpModel->IsUndoEnabled();
        if( bUndo )
        {
            mpModel->BegUndo( ImpGetResStr(STR_TABLE_DISTRIBUTE_ROWS) );
            mpModel->AddUndo( mpModel->GetSdrUndoFactory().CreateUndoGeoObject(*pTableObj) );
        }

        CellPos aStart, aEnd;
        getSelectedCells( aStart, aEnd );
        pTableObj->DistributeRows( aStart.mnRow, aEnd.mnRow );

        if( bUndo )
            mpModel->EndUndo();
    }
}

bool SvxTableController::HasMarked()
{
    return mbCellSelectionMode && mxTable.is();
}

bool SvxTableController::DeleteMarked()
{
    if (!HasMarked())
        return false;

    const bool bUndo = mpModel && mpModel->IsUndoEnabled();
    if (bUndo)
        mpModel->BegUndo(ImpGetResStr(STR_TABLE_DELETE_CELL_CONTENTS));

    CellPos aStart, aEnd;
    getSelectedCells( aStart, aEnd );
    for( sal_Int32 nRow = aStart.mnRow; nRow <= aEnd.mnRow; nRow++ )
    {
        for( sal_Int32 nCol = aStart.mnCol; nCol <= aEnd.mnCol; nCol++ )
        {
            CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
            if (xCell.is() && xCell->hasText())
            {
                if (bUndo)
                    xCell->AddUndo();
                xCell->SetOutlinerParaObject(nullptr);
            }
        }
    }

    if (bUndo)
        mpModel->EndUndo();

    UpdateTableShape();
    return true;
}

bool SvxTableController::GetStyleSheet( SfxStyleSheet*& rpStyleSheet ) const
{
    if( hasSelectedCells() )
    {
        rpStyleSheet = nullptr;

        if( mxTable.is() )
        {
            SfxStyleSheet* pRet=nullptr;
            bool b1st=true;

            CellPos aStart, aEnd;
            const_cast<SvxTableController&>(*this).getSelectedCells( aStart, aEnd );

            for( sal_Int32 nRow = aStart.mnRow; nRow <= aEnd.mnRow; nRow++ )
            {
                for( sal_Int32 nCol = aStart.mnCol; nCol <= aEnd.mnCol; nCol++ )
                {
                    CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
                    if( xCell.is() )
                    {
                        SfxStyleSheet* pSS=xCell->GetStyleSheet();
                        if(b1st)
                        {
                            pRet=pSS;
                        }
                        else if(pRet != pSS)
                        {
                            return true;
                        }
                        b1st=false;
                    }
                }
            }
            rpStyleSheet = pRet;
            return true;
        }
    }
    return false;
}

bool SvxTableController::SetStyleSheet( SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr )
{
    if( hasSelectedCells() && (!pStyleSheet || pStyleSheet->GetFamily() == SFX_STYLE_FAMILY_FRAME) )
    {
        if( mxTable.is() )
        {
            CellPos aStart, aEnd;
            getSelectedCells( aStart, aEnd );

            for( sal_Int32 nRow = aStart.mnRow; nRow <= aEnd.mnRow; nRow++ )
            {
                for( sal_Int32 nCol = aStart.mnCol; nCol <= aEnd.mnCol; nCol++ )
                {
                    CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
                    if( xCell.is() )
                        xCell->SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
                }
            }

            UpdateTableShape();
            return true;
        }
    }
    return false;
}


// internals


bool SvxTableController::checkTableObject()
{
    return mxTableObj.is();
}


sal_uInt16 SvxTableController::getKeyboardAction( const KeyEvent& rKEvt, vcl::Window* /*pWindow*/ )
{
    const bool bMod1 = rKEvt.GetKeyCode().IsMod1(); // ctrl
    const bool bMod2 = rKEvt.GetKeyCode().IsMod2(); // Alt

    const bool bTextEdit = mpView->IsTextEdit();

    sal_uInt16 nAction = ACTION_HANDLED_BY_VIEW;

    sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mxTableObj.get() );
    if( !pTableObj )
        return nAction;

    // handle special keys
    const sal_Int16 nCode = rKEvt.GetKeyCode().GetCode();
    switch( nCode )
    {
    case awt::Key::ESCAPE:          // handle escape
    {
        if( bTextEdit )
        {
            // escape during text edit ends text edit
            nAction = ACTION_STOP_TEXT_EDIT;
        }
        if( mbCellSelectionMode )
        {
            // escape with selected cells removes selection
            nAction = ACTION_REMOVE_SELECTION;
        }
        break;
    }
    case awt::Key::RETURN:      // handle return
    {
        if( !bMod1 && !bMod2 && !bTextEdit )
        {
            // when not already editing, return starts text edit
            setSelectionStart( SdrTableObj::getFirstCell() );
            nAction = ACTION_EDIT_CELL;
        }
        break;
    }
    case awt::Key::F2:          // f2 toggles text edit
    {
        if( bMod1 || bMod2 )    // f2 with modifiers is handled by the view
        {
        }
        else if( bTextEdit )
        {
            // f2 during text edit stops text edit
            nAction = ACTION_STOP_TEXT_EDIT;
        }
        else if( mbCellSelectionMode )
        {
            // f2 with selected cells removes selection
            nAction = ACTION_REMOVE_SELECTION;
        }
        else
        {
            // f2 with no selection and no text edit starts text edit
            setSelectionStart( SdrTableObj::getFirstCell() );
            nAction = ACTION_EDIT_CELL;
        }
        break;
    }
    case awt::Key::HOME:
    case awt::Key::NUM7:
    {
        if( (bMod1 ||  bMod2) && (bTextEdit || mbCellSelectionMode) )
        {
            if( bMod1 && !bMod2 )
            {
                // ctrl + home jumps to first cell
                nAction = ACTION_GOTO_FIRST_CELL;
            }
            else if( !bMod1 && bMod2 )
            {
                // alt + home jumps to first column
                nAction = ACTION_GOTO_FIRST_COLUMN;
            }
        }
        break;
    }
    case awt::Key::END:
    case awt::Key::NUM1:
    {
        if( (bMod1 ||  bMod2) && (bTextEdit || mbCellSelectionMode) )
        {
            if( bMod1 && !bMod2 )
            {
                // ctrl + end jumps to last cell
                nAction = ACTION_GOTO_LAST_CELL;
            }
            else if( !bMod1 && bMod2 )
            {
                // alt + home jumps to last column
                nAction = ACTION_GOTO_LAST_COLUMN;
            }
        }
        break;
    }

    case awt::Key::TAB:
    {
        if( bTextEdit || mbCellSelectionMode )
            nAction = ACTION_TAB;
        break;
    }

    case awt::Key::UP:
    case awt::Key::NUM8:
    case awt::Key::DOWN:
    case awt::Key::NUM2:
    case awt::Key::LEFT:
    case awt::Key::NUM4:
    case awt::Key::RIGHT:
    case awt::Key::NUM6:
    {
        bool bTextMove = false;

        if( !bMod1 && bMod2 )
        {
            if( (nCode == awt::Key::UP) || (nCode == awt::Key::NUM8) )
            {
                nAction = ACTION_GOTO_LEFT_CELL;
            }
            else if( (nCode == awt::Key::DOWN) || (nCode == awt::Key::NUM2) )
            {
                nAction = ACTION_GOTO_RIGHT_CELL;
            }
            break;
        }

        if( !bTextMove )
        {
            OutlinerView* pOLV = mpView->GetTextEditOutlinerView();
            if( pOLV )
            {
                RemoveSelection();
                // during text edit, check if we navigate out of the cell
                ESelection aOldSelection = pOLV->GetSelection();
                pOLV->PostKeyEvent(rKEvt);
                bTextMove = pOLV && ( aOldSelection.IsEqual(pOLV->GetSelection()) );
                if( !bTextMove )
                {
                    nAction = ACTION_NONE;
                }
            }
        }

        if( mbCellSelectionMode || bTextMove )
        {
            // no text edit, navigate in cells if selection active
            switch( nCode )
            {
            case awt::Key::LEFT:
            case awt::Key::NUM4:
                nAction = ACTION_GOTO_LEFT_CELL;
                break;
            case awt::Key::RIGHT:
            case awt::Key::NUM6:
                nAction = ACTION_GOTO_RIGHT_CELL;
                break;
            case awt::Key::DOWN:
            case awt::Key::NUM2:
                nAction = ACTION_GOTO_DOWN_CELL;
                break;
            case awt::Key::UP:
            case awt::Key::NUM8:
                nAction = ACTION_GOTO_UP_CELL;
                break;
            }
        }
        break;
    }
    case awt::Key::PAGEUP:
        if( bMod2 )
            nAction = ACTION_GOTO_FIRST_ROW;
        break;

    case awt::Key::PAGEDOWN:
        if( bMod2 )
            nAction = ACTION_GOTO_LAST_ROW;
        break;
    }
    return nAction;
}

bool SvxTableController::executeAction( sal_uInt16 nAction, bool bSelect, vcl::Window* pWindow )
{
    sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mxTableObj.get() );
    if( !pTableObj )
        return false;

    switch( nAction )
    {
    case ACTION_GOTO_FIRST_CELL:
    {
        gotoCell( SdrTableObj::getFirstCell(), bSelect, pWindow, nAction );
        break;
    }

    case ACTION_GOTO_LEFT_CELL:
    {
        gotoCell( pTableObj->getLeftCell( getSelectionEnd(), !bSelect ), bSelect, pWindow, nAction );
        break;
    }

    case ACTION_GOTO_RIGHT_CELL:
    {
        gotoCell( pTableObj->getRightCell( getSelectionEnd(), !bSelect ), bSelect, pWindow, nAction);
        break;
    }

    case ACTION_GOTO_LAST_CELL:
    {
        gotoCell( pTableObj->getLastCell(), bSelect, pWindow, nAction );
        break;
    }

    case ACTION_GOTO_FIRST_COLUMN:
    {
        CellPos aPos( SdrTableObj::getFirstCell().mnCol, getSelectionEnd().mnRow );
        gotoCell( aPos, bSelect, pWindow, nAction );
        break;
    }

    case ACTION_GOTO_LAST_COLUMN:
    {
        CellPos aPos( pTableObj->getLastCell().mnCol, getSelectionEnd().mnRow );
        gotoCell( aPos, bSelect, pWindow, nAction );
        break;
    }

    case ACTION_GOTO_FIRST_ROW:
    {
        CellPos aPos( getSelectionEnd().mnCol, SdrTableObj::getFirstCell().mnRow );
        gotoCell( aPos, bSelect, pWindow, nAction );
        break;
    }

    case ACTION_GOTO_UP_CELL:
    {
        gotoCell( pTableObj->getUpCell(getSelectionEnd(), !bSelect), bSelect, pWindow, nAction );
        break;
    }

    case ACTION_GOTO_DOWN_CELL:
    {
        gotoCell( pTableObj->getDownCell(getSelectionEnd(), !bSelect), bSelect, pWindow, nAction );
        break;
    }

    case ACTION_GOTO_LAST_ROW:
    {
        CellPos aPos( getSelectionEnd().mnCol, pTableObj->getLastCell().mnRow );
        gotoCell( aPos, bSelect, pWindow, nAction );
        break;
    }

    case ACTION_EDIT_CELL:
        EditCell( getSelectionStart(), pWindow, nullptr, nAction );
        break;

    case ACTION_STOP_TEXT_EDIT:
        StopTextEdit();
        break;

    case ACTION_REMOVE_SELECTION:
        RemoveSelection();
        break;

    case ACTION_START_SELECTION:
        StartSelection( getSelectionStart() );
        break;

    case ACTION_TAB:
    {
        if( bSelect )
            gotoCell( pTableObj->getPreviousCell( getSelectionEnd(), true ), false, pWindow, nAction );
        else
        {
            CellPos aSelectionEnd( getSelectionEnd() );
            CellPos aNextCell( pTableObj->getNextCell( aSelectionEnd, true ) );
            if( aSelectionEnd == aNextCell )
            {
                onInsert( SID_TABLE_INSERT_ROW );
                aNextCell = pTableObj->getNextCell( aSelectionEnd, true );
            }
            gotoCell( aNextCell, false, pWindow, nAction );
        }
        break;
    }
    }

    return nAction != ACTION_HANDLED_BY_VIEW;
}


void SvxTableController::gotoCell( const CellPos& rPos, bool bSelect, vcl::Window* pWindow, sal_uInt16 nAction )
{
    if( mxTableObj.is() && static_cast<SdrTableObj*>(mxTableObj.get())->IsTextEditActive() )
        mpView->SdrEndTextEdit(true);

    if( bSelect )
    {
        maCursorLastPos = rPos;
        if( mxTableObj.is() )
            static_cast< SdrTableObj* >( mxTableObj.get() )->setActiveCell( rPos );

        if( !mbCellSelectionMode )
        {
            setSelectedCells( maCursorFirstPos, rPos );
        }
        else
        {
            UpdateSelection( rPos );
        }
    }
    else
    {
        RemoveSelection();
        EditCell( rPos, pWindow, nullptr, nAction );
    }
}


const CellPos& SvxTableController::getSelectionStart()
{
    checkCell( maCursorFirstPos );
    return maCursorFirstPos;
}


void SvxTableController::setSelectionStart( const CellPos& rPos )
{
    maCursorFirstPos = rPos;
}


const CellPos& SvxTableController::getSelectionEnd()
{
    checkCell( maCursorLastPos );
    return maCursorLastPos;
}


void SvxTableController::MergeRange( sal_Int32 nFirstCol, sal_Int32 nFirstRow, sal_Int32 nLastCol, sal_Int32 nLastRow )
{
    if( mxTable.is() ) try
    {
        Reference< XMergeableCellRange > xRange( mxTable->createCursorByRange( mxTable->getCellRangeByPosition( nFirstCol, nFirstRow,nLastCol, nLastRow ) ), UNO_QUERY_THROW );
        if( xRange->isMergeable() )
        {
            const bool bUndo = mpModel && mpModel->IsUndoEnabled();
            if( bUndo )
            {
                mpModel->BegUndo( ImpGetResStr(STR_TABLE_MERGE) );
                mpModel->AddUndo( mpModel->GetSdrUndoFactory().CreateUndoGeoObject(*mxTableObj.get()) );
            }

            xRange->merge();

            if( bUndo )
                mpModel->EndUndo();
        }
    }
    catch( Exception& )
    {
        DBG_ASSERT( false, "sdr::table::SvxTableController::MergeRange(), exception caught!" );
    }
}


void SvxTableController::checkCell( CellPos& rPos )
{
    if( mxTable.is() ) try
    {
        if( rPos.mnCol >= mxTable->getColumnCount() )
            rPos.mnCol = mxTable->getColumnCount()-1;

        if( rPos.mnRow >= mxTable->getRowCount() )
            rPos.mnRow = mxTable->getRowCount()-1;
    }
    catch( Exception& )
    {
        OSL_FAIL("sdr::table::SvxTableController::checkCell(), exception caught!" );
    }
}


void SvxTableController::findMergeOrigin( CellPos& rPos )
{
    if( mxTable.is() ) try
    {
        Reference< XMergeableCell > xCell( mxTable->getCellByPosition( rPos.mnCol, rPos.mnRow ), UNO_QUERY_THROW );
        if( xCell.is() && xCell->isMerged() )
        {
            ::findMergeOrigin( mxTable, rPos.mnCol, rPos.mnRow, rPos.mnCol, rPos.mnRow );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sdr::table::SvxTableController::findMergeOrigin(), exception caught!" );
    }
}


void SvxTableController::EditCell( const CellPos& rPos, vcl::Window* pWindow, const awt::MouseEvent* pMouseEvent /*= 0*/, sal_uInt16 nAction /*= ACTION_NONE */ )
{
    SdrPageView* pPV = mpView->GetSdrPageView();

    sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mxTableObj.get() );
    if( pTableObj && pTableObj->GetPage() == pPV->GetPage() )
    {
        bool bEmptyOutliner = false;

        if(!pTableObj->GetOutlinerParaObject() && mpView->GetTextEditOutliner())
        {
            ::Outliner* pOutl = mpView->GetTextEditOutliner();
            sal_Int32 nParaAnz = pOutl->GetParagraphCount();
            Paragraph* p1stPara = pOutl->GetParagraph( 0 );

            if(nParaAnz==1 && p1stPara)
            {
                // Bei nur einem Paragraph
                if (pOutl->GetText(p1stPara).isEmpty())
                {
                    bEmptyOutliner = true;
                }
            }
        }

        CellPos aPos( rPos );
        findMergeOrigin( aPos );

        if( pTableObj != mpView->GetTextEditObject() || bEmptyOutliner || !pTableObj->IsTextEditActive( aPos ) )
        {
            if( pTableObj->IsTextEditActive() )
                mpView->SdrEndTextEdit(true);

            pTableObj->setActiveCell( aPos );

            // create new outliner, owner will be the SdrObjEditView
            SdrOutliner* pOutl = mpModel ? SdrMakeOutliner(OUTLINERMODE_OUTLINEOBJECT, *mpModel) : nullptr;
            if (pOutl && pTableObj->IsVerticalWriting())
                pOutl->SetVertical( true );

            if (mpView->SdrBeginTextEdit(pTableObj, pPV, pWindow, true, pOutl))
            {
                maCursorLastPos = maCursorFirstPos = rPos;

                OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

                bool bNoSel = true;

                if( pMouseEvent )
                {
                    ::MouseEvent aMEvt( *pMouseEvent );

                    SdrViewEvent aVEvt;
                    SdrHitKind eHit = mpView->PickAnything(aMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);

                    if (eHit == SDRHIT_TEXTEDIT)
                    {
                        // Text getroffen
                        pOLV->MouseButtonDown(aMEvt);
                        pOLV->MouseMove(aMEvt);
                        pOLV->MouseButtonUp(aMEvt);
//                      pOLV->MouseButtonDown(aMEvt);
                        bNoSel = false;
                    }
                    else
                    {
                        nAction = ACTION_GOTO_LEFT_CELL;
                    }
                }

                if( bNoSel )
                {
                    // Move cursor to end of text
                    ESelection aNewSelection;

                    const WritingMode eMode = pTableObj->GetWritingMode();
                    if( ((nAction == ACTION_GOTO_LEFT_CELL) || (nAction == ACTION_GOTO_RIGHT_CELL)) && (eMode != WritingMode_TB_RL) )
                    {
                        const bool bLast = ((nAction == ACTION_GOTO_LEFT_CELL) && (eMode == WritingMode_LR_TB)) ||
                                             ((nAction == ACTION_GOTO_RIGHT_CELL) && (eMode == WritingMode_RL_TB));

                        if( bLast )
                            aNewSelection = ESelection(EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND, EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND);
                    }
                    pOLV->SetSelection(aNewSelection);
                }
            }
        }
    }
}



bool SvxTableController::StopTextEdit()
{
    if(mpView->IsTextEdit())
    {
        mpView->SdrEndTextEdit();
        mpView->SetCurrentObj(OBJ_TABLE);
        mpView->SetEditMode(SDREDITMODE_EDIT);
        return true;
    }
    else
    {
        return false;
    }
}


void SvxTableController::getSelectedCells( CellPos& rFirst, CellPos& rLast )
{
    if( mbCellSelectionMode )
    {
        checkCell( maCursorFirstPos );
        checkCell( maCursorLastPos );

        rFirst.mnCol = std::min( maCursorFirstPos.mnCol, maCursorLastPos.mnCol );
        rFirst.mnRow = std::min( maCursorFirstPos.mnRow, maCursorLastPos.mnRow );
        rLast.mnCol = std::max( maCursorFirstPos.mnCol, maCursorLastPos.mnCol );
        rLast.mnRow = std::max( maCursorFirstPos.mnRow, maCursorLastPos.mnRow );

        bool bExt = false;
        if( mxTable.is() ) do
        {
            bExt = false;
            for( sal_Int32 nRow = rFirst.mnRow; nRow <= rLast.mnRow && !bExt; nRow++ )
            {
                for( sal_Int32 nCol = rFirst.mnCol; nCol <= rLast.mnCol && !bExt; nCol++ )
                {
                    Reference< XMergeableCell > xCell( mxTable->getCellByPosition( nCol, nRow ), UNO_QUERY );
                    if( !xCell.is() )
                        continue;

                    if( xCell->isMerged() )
                    {
                        CellPos aPos( nCol, nRow );
                        findMergeOrigin( aPos );
                        if( (aPos.mnCol < rFirst.mnCol) || (aPos.mnRow < rFirst.mnRow) )
                        {
                            rFirst.mnCol = std::min( rFirst.mnCol, aPos.mnCol );
                            rFirst.mnRow = std::min( rFirst.mnRow, aPos.mnRow );
                            bExt = true;
                        }
                    }
                    else
                    {
                        if( ((nCol + xCell->getColumnSpan() - 1) > rLast.mnCol) || (nRow + xCell->getRowSpan() - 1 ) > rLast.mnRow )
                        {
                            rLast.mnCol = std::max( rLast.mnCol, nCol + xCell->getColumnSpan() - 1 );
                            rLast.mnRow = std::max( rLast.mnRow, nRow + xCell->getRowSpan() - 1 );
                            bExt = true;
                        }
                    }
                }
            }
        }
        while(bExt);
    }
    else if( mpView && mpView->IsTextEdit() )
    {
        rFirst = getSelectionStart();
        findMergeOrigin( rFirst );
        rLast = rFirst;

        if( mxTable.is() )
        {
            Reference< XMergeableCell > xCell( mxTable->getCellByPosition( rLast.mnCol, rLast.mnRow ), UNO_QUERY );
            if( xCell.is() )
            {
                rLast.mnCol += xCell->getColumnSpan() - 1;
                rLast.mnRow += xCell->getRowSpan() - 1;
            }
        }
    }
    else
    {
        rFirst.mnCol = 0;
        rFirst.mnRow = 0;
        if( mxTable.is() )
        {
            rLast.mnRow = mxTable->getRowCount()-1;
            rLast.mnCol = mxTable->getColumnCount()-1;
        }
        else
        {
            rLast.mnRow = 0;
            rLast.mnCol = 0;
        }
    }
}


void SvxTableController::StartSelection( const CellPos& rPos )
{
    StopTextEdit();
    mbCellSelectionMode = true;
    maCursorLastPos = maCursorFirstPos = rPos;
    mpView->MarkListHasChanged();
}


void SvxTableController::setSelectedCells( const CellPos& rStart, const CellPos& rEnd )
{
    StopTextEdit();
    mbCellSelectionMode = true;
    maCursorFirstPos = rStart;
    UpdateSelection( rEnd );
}


void SvxTableController::UpdateSelection( const CellPos& rPos )
{
    maCursorLastPos = rPos;
    mpView->MarkListHasChanged();
}


void SvxTableController::clearSelection()
{
    RemoveSelection();
}


void SvxTableController::selectAll()
{
    if( mxTable.is() )
    {
        CellPos aPos1, aPos2( mxTable->getColumnCount()-1, mxTable->getRowCount()-1 );
        if( (aPos2.mnCol >= 0) && (aPos2.mnRow >= 0) )
        {
            setSelectedCells( aPos1, aPos2 );
        }
    }
}


void SvxTableController::RemoveSelection()
{
    if( mbCellSelectionMode )
    {
        mbCellSelectionMode = false;
        mpView->MarkListHasChanged();
    }
}


void SvxTableController::onTableModified()
{
    if( mnUpdateEvent == nullptr )
        mnUpdateEvent = Application::PostUserEvent( LINK( this, SvxTableController, UpdateHdl ) );
}


void SvxTableController::updateSelectionOverlay()
{
    destroySelectionOverlay();
    if( mbCellSelectionMode )
    {
        sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mxTableObj.get() );
        if( pTableObj )
        {
            sdr::overlay::OverlayObjectCell::RangeVector aRanges;

            Rectangle aStartRect, aEndRect;
            CellPos aStart,aEnd;
            getSelectedCells( aStart, aEnd );
            pTableObj->getCellBounds( aStart, aStartRect );

            basegfx::B2DRange a2DRange( basegfx::B2DPoint(aStartRect.Left(), aStartRect.Top()) );
            a2DRange.expand( basegfx::B2DPoint(aStartRect.Right(), aStartRect.Bottom()) );

            findMergeOrigin( aEnd );
            pTableObj->getCellBounds( aEnd, aEndRect );
            a2DRange.expand( basegfx::B2DPoint(aEndRect.Left(), aEndRect.Top()) );
            a2DRange.expand( basegfx::B2DPoint(aEndRect.Right(), aEndRect.Bottom()) );
            aRanges.push_back( a2DRange );

            ::Color aHighlight( COL_BLUE );
            OutputDevice* pOutDev = mpView->GetFirstOutputDevice();
            if( pOutDev )
                aHighlight = pOutDev->GetSettings().GetStyleSettings().GetHighlightColor();

            const sal_uInt32 nCount = mpView->PaintWindowCount();
            for( sal_uInt32 nIndex = 0; nIndex < nCount; nIndex++ )
            {
                SdrPaintWindow* pPaintWindow = mpView->GetPaintWindow(nIndex);
                if( pPaintWindow )
                {
                    rtl::Reference < sdr::overlay::OverlayManager > xOverlayManager = pPaintWindow->GetOverlayManager();
                    if( xOverlayManager.is() )
                    {
                        // sdr::overlay::CellOverlayType eType = sdr::overlay::CELL_OVERLAY_INVERT;
                        sdr::overlay::CellOverlayType eType = sdr::overlay::CELL_OVERLAY_TRANSPARENT;

                        sdr::overlay::OverlayObjectCell* pOverlay = new sdr::overlay::OverlayObjectCell( eType, aHighlight, aRanges );

                        xOverlayManager->add(*pOverlay);
                        mpSelectionOverlay = new sdr::overlay::OverlayObjectList;
                        mpSelectionOverlay->append(*pOverlay);
                    }
                }
            }

            // If tiled rendering, emit callbacks for sdr table selection.
            if (pOutDev && comphelper::LibreOfficeKit::isActive())
            {
                // Left edge of aStartRect.
                Rectangle aSelectionStart(aStartRect.Left(), aStartRect.Top(), aStartRect.Left(), aStartRect.Bottom());
                // Right edge of aEndRect.
                Rectangle aSelectionEnd(aEndRect.Right(), aEndRect.Top(), aEndRect.Right(), aEndRect.Bottom());
                Rectangle aSelection(a2DRange.getMinX(), a2DRange.getMinY(), a2DRange.getMaxX(), a2DRange.getMaxY());

                if (pOutDev->GetMapMode().GetMapUnit() == MAP_100TH_MM)
                {
                    aSelectionStart = OutputDevice::LogicToLogic(aSelectionStart, MAP_100TH_MM, MAP_TWIP);
                    aSelectionEnd = OutputDevice::LogicToLogic(aSelectionEnd, MAP_100TH_MM, MAP_TWIP);
                    aSelection = OutputDevice::LogicToLogic(aSelection, MAP_100TH_MM, MAP_TWIP);
                }

                if (comphelper::LibreOfficeKit::isViewCallback())
                {
                    if(SfxViewShell* pViewShell = SfxViewShell::Current())
                    {
                        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION_START, aSelectionStart.toString().getStr());
                        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION_END, aSelectionEnd.toString().getStr());
                        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, aSelection.toString().getStr());
                    }
                }
                else
                {
                    pTableObj->GetModel()->libreOfficeKitCallback(LOK_CALLBACK_TEXT_SELECTION_START, aSelectionStart.toString().getStr());
                    pTableObj->GetModel()->libreOfficeKitCallback(LOK_CALLBACK_TEXT_SELECTION_END, aSelectionEnd.toString().getStr());
                    pTableObj->GetModel()->libreOfficeKitCallback(LOK_CALLBACK_TEXT_SELECTION, aSelection.toString().getStr());
                }
            }
        }
    }
}


void SvxTableController::destroySelectionOverlay()
{
    if( mpSelectionOverlay )
    {
        delete mpSelectionOverlay;
        mpSelectionOverlay = nullptr;

        if (comphelper::LibreOfficeKit::isActive())
        {
            // Clear the LOK text selection so far provided by this table.
            if (comphelper::LibreOfficeKit::isViewCallback())
            {
                if(SfxViewShell* pViewShell = SfxViewShell::Current())
                {
                    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION_START, "EMPTY");
                    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION_END, "EMPTY");
                    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, "EMPTY");
                }
            }
            else
            {
                mxTableObj->GetModel()->libreOfficeKitCallback(LOK_CALLBACK_TEXT_SELECTION_START, "EMPTY");
                mxTableObj->GetModel()->libreOfficeKitCallback(LOK_CALLBACK_TEXT_SELECTION_END, "EMPTY");
                mxTableObj->GetModel()->libreOfficeKitCallback(LOK_CALLBACK_TEXT_SELECTION, "EMPTY");
            }
        }
    }
}


void SvxTableController::MergeAttrFromSelectedCells(SfxItemSet& rAttr, bool bOnlyHardAttr) const
{
    if( mxTable.is() )
    {
        CellPos aStart, aEnd;
        const_cast<SvxTableController&>(*this).getSelectedCells( aStart, aEnd );

        for( sal_Int32 nRow = aStart.mnRow; nRow <= aEnd.mnRow; nRow++ )
        {
            for( sal_Int32 nCol = aStart.mnCol; nCol <= aEnd.mnCol; nCol++ )
            {
                CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
                if( xCell.is() && !xCell->isMerged() )
                {
                    const SfxItemSet& rSet = xCell->GetItemSet();
                    SfxWhichIter aIter(rSet);
                    sal_uInt16 nWhich(aIter.FirstWhich());
                    while(nWhich)
                    {
                        if(!bOnlyHardAttr)
                        {
                            if(SfxItemState::DONTCARE == rSet.GetItemState(nWhich, false))
                                rAttr.InvalidateItem(nWhich);
                            else
                                rAttr.MergeValue(rSet.Get(nWhich), true);
                        }
                        else if(SfxItemState::SET == rSet.GetItemState(nWhich, false))
                        {
                            const SfxPoolItem& rItem = rSet.Get(nWhich);
                            rAttr.MergeValue(rItem, true);
                        }

                        nWhich = aIter.NextWhich();
                    }
                }
            }
        }
    }
}


const sal_uInt16 CELL_BEFORE = 0x0001;
const sal_uInt16 CELL_LEFT   = 0x0002;
const sal_uInt16 CELL_RIGHT  = 0x0004;
const sal_uInt16 CELL_AFTER  = 0x0008;

const sal_uInt16 CELL_UPPER  = 0x0010;
const sal_uInt16 CELL_TOP    = 0x0020;
const sal_uInt16 CELL_BOTTOM = 0x0040;
const sal_uInt16 CELL_LOWER  = 0x0080;


static void ImplSetLinePreserveColor( SvxBoxItem& rNewFrame, const SvxBorderLine* pNew, SvxBoxItemLine nLine )
{
    if( pNew )
    {
        const SvxBorderLine* pOld = rNewFrame.GetLine(nLine);
        if( pOld )
        {
            SvxBorderLine aNewLine( *pNew );
            aNewLine.SetColor( pOld->GetColor() );
            rNewFrame.SetLine( &aNewLine, nLine );
            return;
        }
    }
    rNewFrame.SetLine( pNew, nLine );
}


static void ImplApplyBoxItem( sal_uInt16 nCellFlags, const SvxBoxItem* pBoxItem, const SvxBoxInfoItem* pBoxInfoItem, SvxBoxItem& rNewFrame )
{
    if( (nCellFlags & (CELL_BEFORE|CELL_AFTER|CELL_UPPER|CELL_LOWER)) != 0 )
    {
        // current cell is outside the selection

        if( (nCellFlags & ( CELL_BEFORE|CELL_AFTER)) == 0 ) // check if its not nw or ne corner
        {
            if( nCellFlags & CELL_UPPER )
            {
                if( pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::TOP) )
                    rNewFrame.SetLine(nullptr, SvxBoxItemLine::BOTTOM );
            }
            else if( nCellFlags & CELL_LOWER )
            {
                if( pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::BOTTOM) )
                    rNewFrame.SetLine( nullptr, SvxBoxItemLine::TOP );
            }
        }
        else if( (nCellFlags & ( CELL_UPPER|CELL_LOWER)) == 0 ) // check if its not sw or se corner
        {
            if( nCellFlags & CELL_BEFORE )
            {
                if( pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::LEFT) )
                    rNewFrame.SetLine( nullptr, SvxBoxItemLine::RIGHT );
            }
            else if( nCellFlags & CELL_AFTER )
            {
                if( pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::RIGHT) )
                    rNewFrame.SetLine( nullptr, SvxBoxItemLine::LEFT );
            }
        }
    }
    else
    {
        // current cell is inside the selection

        if( (nCellFlags & CELL_LEFT) ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::LEFT) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::VERT) )
            rNewFrame.SetLine( (nCellFlags & CELL_LEFT) ? pBoxItem->GetLeft() : pBoxInfoItem->GetVert(), SvxBoxItemLine::LEFT );

        if( (nCellFlags & CELL_RIGHT) ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::RIGHT) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::VERT) )
            rNewFrame.SetLine( (nCellFlags & CELL_RIGHT) ? pBoxItem->GetRight() : pBoxInfoItem->GetVert(), SvxBoxItemLine::RIGHT );

        if( (nCellFlags & CELL_TOP) ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::TOP) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::HORI) )
            rNewFrame.SetLine( (nCellFlags & CELL_TOP) ? pBoxItem->GetTop() : pBoxInfoItem->GetHori(), SvxBoxItemLine::TOP );

        if( (nCellFlags & CELL_BOTTOM) ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::BOTTOM) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::HORI) )
            rNewFrame.SetLine( (nCellFlags & CELL_BOTTOM) ? pBoxItem->GetBottom() : pBoxInfoItem->GetHori(), SvxBoxItemLine::BOTTOM );

        // apply distance to borders
        if( pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::DISTANCE ) )
            for( SvxBoxItemLine nLine : o3tl::enumrange<SvxBoxItemLine>() )
                rNewFrame.SetDistance( pBoxItem->GetDistance( nLine ), nLine );
    }
}


static void ImplSetLineColor( SvxBoxItem& rNewFrame, SvxBoxItemLine nLine, const Color& rColor )
{
    const SvxBorderLine* pSourceLine = rNewFrame.GetLine( nLine );
    if( pSourceLine )
    {
        SvxBorderLine aLine( *pSourceLine );
        aLine.SetColor( rColor );
        rNewFrame.SetLine( &aLine, nLine );
    }
}


static void ImplApplyLineColorItem( sal_uInt16 nCellFlags, const SvxColorItem* pLineColorItem, SvxBoxItem& rNewFrame )
{
    const Color aColor( pLineColorItem->GetValue() );

    if( (nCellFlags & (CELL_LOWER|CELL_BEFORE|CELL_AFTER)) == 0 )
        ImplSetLineColor( rNewFrame, SvxBoxItemLine::BOTTOM, aColor );

    if( (nCellFlags & (CELL_UPPER|CELL_BEFORE|CELL_AFTER)) == 0 )
        ImplSetLineColor( rNewFrame, SvxBoxItemLine::TOP, aColor );

    if( (nCellFlags & (CELL_UPPER|CELL_LOWER|CELL_AFTER)) == 0 )
        ImplSetLineColor( rNewFrame, SvxBoxItemLine::RIGHT, aColor );

    if( (nCellFlags & (CELL_UPPER|CELL_LOWER|CELL_BEFORE)) == 0 )
        ImplSetLineColor( rNewFrame, SvxBoxItemLine::LEFT, aColor );
}


static void ImplApplyBorderLineItem( sal_uInt16 nCellFlags, const SvxBorderLine* pBorderLineItem, SvxBoxItem& rNewFrame )
{
    if( (nCellFlags & ( CELL_BEFORE|CELL_AFTER|CELL_UPPER|CELL_LOWER)) != 0 )
    {
        if( (nCellFlags & ( CELL_BEFORE|CELL_AFTER)) == 0 ) // check if its not nw or ne corner
        {
            if( nCellFlags & CELL_UPPER )
            {
                if( rNewFrame.GetBottom() )
                    ImplSetLinePreserveColor( rNewFrame, pBorderLineItem, SvxBoxItemLine::BOTTOM );
            }
            else if( nCellFlags & CELL_LOWER )
            {
                if( rNewFrame.GetTop() )
                    ImplSetLinePreserveColor( rNewFrame, pBorderLineItem, SvxBoxItemLine::TOP );
            }
        }
        else if( (nCellFlags & ( CELL_UPPER|CELL_LOWER)) == 0 ) // check if its not sw or se corner
        {
            if( nCellFlags & CELL_BEFORE )
            {
                if( rNewFrame.GetRight() )
                    ImplSetLinePreserveColor( rNewFrame, pBorderLineItem, SvxBoxItemLine::RIGHT );
            }
            else if( nCellFlags & CELL_AFTER )
            {
                if( rNewFrame.GetLeft() )
                    ImplSetLinePreserveColor( rNewFrame, pBorderLineItem, SvxBoxItemLine::LEFT );
            }
        }
    }
    else
    {
        if( rNewFrame.GetBottom() )
            ImplSetLinePreserveColor( rNewFrame, pBorderLineItem, SvxBoxItemLine::BOTTOM );
        if( rNewFrame.GetTop() )
            ImplSetLinePreserveColor( rNewFrame, pBorderLineItem, SvxBoxItemLine::TOP );
        if( rNewFrame.GetRight() )
            ImplSetLinePreserveColor( rNewFrame, pBorderLineItem, SvxBoxItemLine::RIGHT );
        if( rNewFrame.GetLeft() )
            ImplSetLinePreserveColor( rNewFrame, pBorderLineItem, SvxBoxItemLine::LEFT );
    }
}


void SvxTableController::ApplyBorderAttr( const SfxItemSet& rAttr )
{
    if( mxTable.is() )
    {
        const sal_Int32 nRowCount = mxTable->getRowCount();
        const sal_Int32 nColCount = mxTable->getColumnCount();
        if( nRowCount && nColCount )
        {
            const SvxBoxItem* pBoxItem = nullptr;
            if(SfxItemState::SET == rAttr.GetItemState(SDRATTR_TABLE_BORDER, false) )
                pBoxItem = dynamic_cast< const SvxBoxItem* >( &rAttr.Get( SDRATTR_TABLE_BORDER ) );

            const SvxBoxInfoItem* pBoxInfoItem = nullptr;
            if(SfxItemState::SET == rAttr.GetItemState(SDRATTR_TABLE_BORDER_INNER, false) )
                pBoxInfoItem = dynamic_cast< const SvxBoxInfoItem* >( &rAttr.Get( SDRATTR_TABLE_BORDER_INNER ) );

            const SvxColorItem* pLineColorItem = nullptr;
            if(SfxItemState::SET == rAttr.GetItemState(SID_FRAME_LINECOLOR, false) )
                pLineColorItem = dynamic_cast< const SvxColorItem* >( &rAttr.Get( SID_FRAME_LINECOLOR ) );

            const SvxBorderLine* pBorderLineItem = nullptr;
            if(SfxItemState::SET == rAttr.GetItemState(SID_FRAME_LINESTYLE, false) )
                pBorderLineItem = static_cast<const SvxLineItem&>(rAttr.Get( SID_FRAME_LINESTYLE )).GetLine();

            if( pBoxInfoItem && !pBoxItem )
            {
                const static SvxBoxItem gaEmptyBoxItem( SDRATTR_TABLE_BORDER );
                pBoxItem = &gaEmptyBoxItem;
            }
            else if( pBoxItem && !pBoxInfoItem )
            {
                const static SvxBoxInfoItem gaEmptyBoxInfoItem( SDRATTR_TABLE_BORDER_INNER );
                pBoxInfoItem = &gaEmptyBoxInfoItem;
            }

            CellPos aStart, aEnd;
            getSelectedCells( aStart, aEnd );

            const sal_Int32 nLastRow = std::min( aEnd.mnRow + 2, nRowCount );
            const sal_Int32 nLastCol = std::min( aEnd.mnCol + 2, nColCount );

            for( sal_Int32 nRow = std::max( aStart.mnRow - 1, (sal_Int32)0 ); nRow < nLastRow; nRow++ )
            {
                sal_uInt16 nRowFlags = 0;
                nRowFlags |= (nRow == aStart.mnRow) ? CELL_TOP : 0;
                nRowFlags |= (nRow == aEnd.mnRow)   ? CELL_BOTTOM : 0;
                nRowFlags |= (nRow < aStart.mnRow)  ? CELL_UPPER : 0;
                nRowFlags |= (nRow > aEnd.mnRow)    ? CELL_LOWER : 0;

                for( sal_Int32 nCol = std::max( aStart.mnCol - 1, (sal_Int32)0 ); nCol < nLastCol; nCol++ )
                {
                    CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
                    if( !xCell.is() )
                        continue;

                    const SfxItemSet& rSet = xCell->GetItemSet();
                    const SvxBoxItem* pOldOuter = static_cast<const SvxBoxItem*>(&rSet.Get( SDRATTR_TABLE_BORDER ));

                    SvxBoxItem aNewFrame( *pOldOuter );

                    sal_uInt16 nCellFlags = nRowFlags;
                    nCellFlags |= (nCol == aStart.mnCol) ? CELL_LEFT : 0;
                    nCellFlags |= (nCol == aEnd.mnCol)   ? CELL_RIGHT : 0;
                    nCellFlags |= (nCol < aStart.mnCol)  ? CELL_BEFORE : 0;
                    nCellFlags |= (nCol > aEnd.mnCol)    ? CELL_AFTER : 0;

                    if( pBoxItem && pBoxInfoItem )
                        ImplApplyBoxItem( nCellFlags, pBoxItem, pBoxInfoItem, aNewFrame );

                    if( pLineColorItem )
                        ImplApplyLineColorItem( nCellFlags, pLineColorItem, aNewFrame );

                    if( pBorderLineItem )
                        ImplApplyBorderLineItem( nCellFlags, pBorderLineItem, aNewFrame );

                    if (aNewFrame != *pOldOuter)
                    {
                        SfxItemSet aAttr(*rSet.GetPool(), rSet.GetRanges());
                        aAttr.Put(aNewFrame);
                        xCell->SetMergedItemSetAndBroadcast( aAttr, false );
                    }
                }
            }
        }
    }
}


void SvxTableController::UpdateTableShape()
{
    SdrObject* pTableObj = mxTableObj.get();
    if( pTableObj )
    {
        pTableObj->ActionChanged();
        pTableObj->BroadcastObjectChange();
    }
    updateSelectionOverlay();
}


void SvxTableController::SetAttrToSelectedCells(const SfxItemSet& rAttr, bool bReplaceAll)
{
    if( mxTable.is() )
    {
        const bool bUndo = mpModel && mpModel->IsUndoEnabled();

        if( bUndo )
            mpModel->BegUndo( ImpGetResStr(STR_TABLE_NUMFORMAT) );

        CellPos aStart, aEnd;
        getSelectedCells( aStart, aEnd );

        SfxItemSet aAttr(*rAttr.GetPool(), rAttr.GetRanges());
        aAttr.Put(rAttr);

        const bool bFrame = (rAttr.GetItemState( SDRATTR_TABLE_BORDER ) == SfxItemState::SET) || (rAttr.GetItemState( SDRATTR_TABLE_BORDER_INNER ) == SfxItemState::SET);

        if( bFrame )
        {
            aAttr.ClearItem( SDRATTR_TABLE_BORDER );
            aAttr.ClearItem( SDRATTR_TABLE_BORDER_INNER );
        }

        for( sal_Int32 nRow = aStart.mnRow; nRow <= aEnd.mnRow; nRow++ )
        {
            for( sal_Int32 nCol = aStart.mnCol; nCol <= aEnd.mnCol; nCol++ )
            {
                CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
                if( xCell.is() )
                {
                    if( bUndo )
                        xCell->AddUndo();
                    xCell->SetMergedItemSetAndBroadcast(aAttr, bReplaceAll);
                }
            }
        }

        if( bFrame )
        {
            ApplyBorderAttr( rAttr );
        }

        UpdateTableShape();

        if( bUndo )
            mpModel->EndUndo();

    }
}


bool SvxTableController::GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const
{
    if( mxTableObj.is() && hasSelectedCells() )
    {
        MergeAttrFromSelectedCells( rTargetSet, bOnlyHardAttr );

        if( mpView->IsTextEdit() )
        {
            if( mxTableObj->GetOutlinerParaObject() )
                rTargetSet.Put( SvxScriptTypeItem( mxTableObj->GetOutlinerParaObject()->GetTextObject().GetScriptType() ) );

            OutlinerView* pTextEditOutlinerView = mpView->GetTextEditOutlinerView();
            if(pTextEditOutlinerView)
            {
                // FALSE= InvalidItems nicht al Default, sondern als "Loecher" betrachten
                rTargetSet.Put(pTextEditOutlinerView->GetAttribs(), false);
                rTargetSet.Put( SvxScriptTypeItem( pTextEditOutlinerView->GetSelectedScriptType() ) );
            }
        }

        return true;
    }
    else
    {
        return false;
    }
}


bool SvxTableController::SetAttributes(const SfxItemSet& rSet, bool bReplaceAll)
{
    if( mbCellSelectionMode || mpView->IsTextEdit()  )
    {
        SetAttrToSelectedCells( rSet, bReplaceAll );
        return true;
    }
    return false;
}


bool SvxTableController::GetMarkedObjModel( SdrPage* pNewPage )
{
    if( mxTableObj.is() && mbCellSelectionMode && pNewPage ) try
    {
        sdr::table::SdrTableObj& rTableObj = *static_cast< sdr::table::SdrTableObj* >( mxTableObj.get() );

        CellPos aStart, aEnd;
        getSelectedCells( aStart, aEnd );

        SdrTableObj* pNewTableObj = rTableObj.CloneRange( aStart, aEnd );

        pNewTableObj->SetPage( pNewPage );
        pNewTableObj->SetModel( pNewPage->GetModel() );

        SdrInsertReason aReason(SDRREASON_VIEWCALL);
        pNewPage->InsertObject(pNewTableObj, SAL_MAX_SIZE, &aReason);

        return true;
    }
    catch( Exception& )
    {
        OSL_FAIL( "svx::SvxTableController::GetMarkedObjModel(), exception caught!" );
    }
    return false;
}


bool SvxTableController::PasteObjModel( const SdrModel& rModel )
{
    if( mxTableObj.is() && mpView && (rModel.GetPageCount() >= 1) )
    {
        const SdrPage* pPastePage = rModel.GetPage(0);
        if( pPastePage && pPastePage->GetObjCount() == 1 )
        {
            SdrTableObj* pPasteTableObj = dynamic_cast< SdrTableObj* >( pPastePage->GetObj(0) );
            if( pPasteTableObj )
            {
                return PasteObject( pPasteTableObj );
            }
        }
    }

    return false;
}


bool SvxTableController::PasteObject( SdrTableObj* pPasteTableObj )
{
    if( !pPasteTableObj )
        return false;

    Reference< XTable > xPasteTable( pPasteTableObj->getTable() );
    if( !xPasteTable.is() )
        return false;

    if( !mxTable.is() )
        return false;

    sal_Int32 nPasteColumns = xPasteTable->getColumnCount();
    sal_Int32 nPasteRows = xPasteTable->getRowCount();

    CellPos aStart, aEnd;
    getSelectedCells( aStart, aEnd );

    if( mpView->IsTextEdit() )
        mpView->SdrEndTextEdit(true);

    sal_Int32 nColumns = mxTable->getColumnCount();
    sal_Int32 nRows = mxTable->getRowCount();

    const sal_Int32 nMissing = nPasteRows - ( nRows - aStart.mnRow );
    if( nMissing > 0 )
    {
        Reference< XTableRows > xRows( mxTable->getRows() );
        xRows->insertByIndex( nRows, nMissing );
        nRows = mxTable->getRowCount();
    }

    nPasteRows = std::min( nPasteRows, nRows - aStart.mnRow );
    nPasteColumns = std::min( nPasteColumns, nColumns - aStart.mnCol );

    // copy cell contents
    for( sal_Int32 nRow = 0; nRow < nPasteRows; ++nRow )
    {
        for( sal_Int32 nCol = 0; nCol < nPasteColumns; ++nCol )
        {
            CellRef xTargetCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( aStart.mnCol + nCol, aStart.mnRow + nRow ).get() ) );
            if( xTargetCell.is() && !xTargetCell->isMerged() )
            {
                xTargetCell->AddUndo();
                xTargetCell->cloneFrom( dynamic_cast< Cell* >( xPasteTable->getCellByPosition( nCol, nRow ).get() ) );
                nCol += xTargetCell->getColumnSpan() - 1;
            }
        }
    }

    UpdateTableShape();

    return true;
}

bool SvxTableController::TakeFormatPaintBrush( std::shared_ptr< SfxItemSet >& /*rFormatSet*/  )
{
    // SdrView::TakeFormatPaintBrush() is enough
    return false;
}

bool SvxTableController::ApplyFormatPaintBrush( SfxItemSet& rFormatSet, bool bNoCharacterFormats, bool bNoParagraphFormats )
{
    if( mbCellSelectionMode )
    {
        SdrTextObj* pTableObj = dynamic_cast<SdrTextObj*>( mxTableObj.get() );
        if( !pTableObj )
            return false;

        const bool bUndo = mpModel && mpModel->IsUndoEnabled();

        if( bUndo )
            mpModel->BegUndo( ImpGetResStr(STR_TABLE_NUMFORMAT) );

        CellPos aStart, aEnd;
        getSelectedCells( aStart, aEnd );

        SfxItemSet aAttr(*rFormatSet.GetPool(), rFormatSet.GetRanges());
        aAttr.Put(rFormatSet);

        const bool bFrame = (rFormatSet.GetItemState( SDRATTR_TABLE_BORDER ) == SfxItemState::SET) || (rFormatSet.GetItemState( SDRATTR_TABLE_BORDER_INNER ) == SfxItemState::SET);

        if( bFrame )
        {
            aAttr.ClearItem( SDRATTR_TABLE_BORDER );
            aAttr.ClearItem( SDRATTR_TABLE_BORDER_INNER );
        }

        const sal_uInt16* pRanges = rFormatSet.GetRanges();
        bool bTextOnly = true;

        while( *pRanges )
        {
            if( (*pRanges != EE_PARA_START) && (*pRanges != EE_CHAR_START) )
            {
                bTextOnly = false;
                break;
            }
            pRanges += 2;
        }

        const bool bReplaceAll = false;
        for( sal_Int32 nRow = aStart.mnRow; nRow <= aEnd.mnRow; nRow++ )
        {
            for( sal_Int32 nCol = aStart.mnCol; nCol <= aEnd.mnCol; nCol++ )
            {
                CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
                if( xCell.is() )
                {
                    if( bUndo )
                        xCell->AddUndo();
                    if( !bTextOnly )
                        xCell->SetMergedItemSetAndBroadcast(aAttr, bReplaceAll);

                    SdrText* pText = static_cast< SdrText* >( xCell.get() );
                    SdrObjEditView::ApplyFormatPaintBrushToText( rFormatSet, *pTableObj, pText, bNoCharacterFormats, bNoParagraphFormats );
                }
            }
        }

        if( bFrame )
        {
            ApplyBorderAttr( rFormatSet );
        }

        UpdateTableShape();

        if( bUndo )
            mpModel->EndUndo();

        return true;

    }
    return false;
}


IMPL_LINK_NOARG_TYPED(SvxTableController, UpdateHdl, void*, void)
{
    mnUpdateEvent = nullptr;

    if( mbCellSelectionMode )
    {
        CellPos aStart( maCursorFirstPos );
        CellPos aEnd( maCursorLastPos );
        checkCell(aStart);
        checkCell(aEnd);
        if( aStart != maCursorFirstPos  || aEnd != maCursorLastPos )
        {
            setSelectedCells( aStart, aEnd );
        }
    }
    updateSelectionOverlay();
}

namespace
{

struct LinesState
{
    LinesState(SvxBoxItem& rBoxItem_, SvxBoxInfoItem& rBoxInfoItem_)
        : rBoxItem(rBoxItem_)
        , rBoxInfoItem(rBoxInfoItem_)
        , bDistanceIndeterminate(false)
    {
        aBorderSet.fill(false);
        aInnerLineSet.fill(false);
        aBorderIndeterminate.fill(false);
        aInnerLineIndeterminate.fill(false);
        aDistanceSet.fill(false);
        aDistance.fill(0);
    }

    SvxBoxItem& rBoxItem;
    SvxBoxInfoItem& rBoxInfoItem;
    o3tl::enumarray<SvxBoxItemLine, bool>       aBorderSet;
    o3tl::enumarray<SvxBoxInfoItemLine, bool>   aInnerLineSet;
    o3tl::enumarray<SvxBoxItemLine, bool>       aBorderIndeterminate;
    o3tl::enumarray<SvxBoxInfoItemLine, bool>   aInnerLineIndeterminate;
    o3tl::enumarray<SvxBoxItemLine, bool>       aDistanceSet;
    o3tl::enumarray<SvxBoxItemLine, sal_uInt16> aDistance;
    bool bDistanceIndeterminate;
};

class BoxItemWrapper
{
public:
    BoxItemWrapper(SvxBoxItem& rBoxItem, SvxBoxInfoItem& rBoxInfoItem, SvxBoxItemLine nBorderLine, SvxBoxInfoItemLine nInnerLine, bool bBorder);

    const SvxBorderLine* getLine() const;
    void setLine(const SvxBorderLine* pLine);

private:
    SvxBoxItem& m_rBoxItem;
    SvxBoxInfoItem& m_rBoxInfoItem;
    const SvxBoxItemLine m_nBorderLine;
    const SvxBoxInfoItemLine m_nInnerLine;
    const bool m_bBorder;
};

BoxItemWrapper::BoxItemWrapper(
        SvxBoxItem& rBoxItem, SvxBoxInfoItem& rBoxInfoItem,
        const SvxBoxItemLine nBorderLine, const SvxBoxInfoItemLine nInnerLine, const bool bBorder)
    : m_rBoxItem(rBoxItem)
    , m_rBoxInfoItem(rBoxInfoItem)
    , m_nBorderLine(nBorderLine)
    , m_nInnerLine(nInnerLine)
    , m_bBorder(bBorder)
{
}

const SvxBorderLine* BoxItemWrapper::getLine() const
{
    if (m_bBorder)
        return m_rBoxItem.GetLine(m_nBorderLine);
    else
        return (m_nInnerLine == SvxBoxInfoItemLine::HORI) ? m_rBoxInfoItem.GetHori() : m_rBoxInfoItem.GetVert();
}

void BoxItemWrapper::setLine(const SvxBorderLine* pLine)
{
    if (m_bBorder)
        m_rBoxItem.SetLine(pLine, m_nBorderLine);
    else
        m_rBoxInfoItem.SetLine(pLine, m_nInnerLine);
}

void lcl_MergeBorderLine(
        LinesState& rLinesState, const SvxBorderLine* const pLine, const SvxBoxItemLine nLine,
        SvxBoxInfoItemValidFlags nValidFlag, const bool bBorder = true)
{
    const SvxBoxInfoItemLine nInnerLine(bBorder ? SvxBoxInfoItemLine::HORI : ((nValidFlag & SvxBoxInfoItemValidFlags::HORI) ? SvxBoxInfoItemLine::HORI : SvxBoxInfoItemLine::VERT));
    BoxItemWrapper aBoxItem(rLinesState.rBoxItem, rLinesState.rBoxInfoItem, nLine, nInnerLine, bBorder);
    bool& rbSet(bBorder ? rLinesState.aBorderSet[nLine] : rLinesState.aInnerLineSet[nInnerLine]);

    if (rbSet)
    {
        bool& rbIndeterminate(bBorder ? rLinesState.aBorderIndeterminate[nLine] : rLinesState.aInnerLineIndeterminate[nInnerLine]);
        if (!rbIndeterminate)
        {
            const SvxBorderLine* const pMergedLine(aBoxItem.getLine());
            if ((pLine && !pMergedLine) || (!pLine && pMergedLine) || (pLine && (*pLine != *pMergedLine)))
            {
                aBoxItem.setLine(nullptr);
                rbIndeterminate = true;
            }
        }
    }
    else
    {
        aBoxItem.setLine(pLine);
        rbSet = true;
    }
}

void lcl_MergeBorderOrInnerLine(
        LinesState& rLinesState, const SvxBorderLine* const pLine, const SvxBoxItemLine nLine,
        SvxBoxInfoItemValidFlags nValidFlag, const bool bBorder)
{
    if (bBorder)
        lcl_MergeBorderLine(rLinesState, pLine, nLine, nValidFlag);
    else
    {
        const bool bVertical = (nLine == SvxBoxItemLine::LEFT) || (nLine == SvxBoxItemLine::RIGHT);
        lcl_MergeBorderLine(rLinesState, pLine, nLine, bVertical ? SvxBoxInfoItemValidFlags::VERT : SvxBoxInfoItemValidFlags::HORI, false);
    }
}

void lcl_MergeDistance(
        LinesState& rLinesState, const SvxBoxItemLine nIndex, const sal_uInt16 nDistance)
{
    if (rLinesState.aDistanceSet[nIndex])
    {
        if (!rLinesState.bDistanceIndeterminate)
            rLinesState.bDistanceIndeterminate = nDistance != rLinesState.aDistance[nIndex];
    }
    else
    {
        rLinesState.aDistance[nIndex] = nDistance;
        rLinesState.aDistanceSet[nIndex] = true;
    }
}

void lcl_MergeCommonBorderAttr(LinesState& rLinesState, const SvxBoxItem& rCellBoxItem, const sal_Int32 nCellFlags)
{
    if( (nCellFlags & (CELL_BEFORE|CELL_AFTER|CELL_UPPER|CELL_LOWER)) != 0 )
    {
        // current cell is outside the selection

        if( (nCellFlags & ( CELL_BEFORE|CELL_AFTER)) == 0 ) // check if its not nw or ne corner
        {
            if( nCellFlags & CELL_UPPER )
                lcl_MergeBorderLine(rLinesState, rCellBoxItem.GetBottom(), SvxBoxItemLine::TOP, SvxBoxInfoItemValidFlags::TOP);
            else if( nCellFlags & CELL_LOWER )
                lcl_MergeBorderLine(rLinesState, rCellBoxItem.GetTop(), SvxBoxItemLine::BOTTOM, SvxBoxInfoItemValidFlags::BOTTOM);
        }
        else if( (nCellFlags & ( CELL_UPPER|CELL_LOWER)) == 0 ) // check if its not sw or se corner
        {
            if( nCellFlags & CELL_BEFORE )
                lcl_MergeBorderLine(rLinesState, rCellBoxItem.GetRight(), SvxBoxItemLine::LEFT, SvxBoxInfoItemValidFlags::LEFT);
            else if( nCellFlags & CELL_AFTER )
                lcl_MergeBorderLine(rLinesState, rCellBoxItem.GetLeft(), SvxBoxItemLine::RIGHT, SvxBoxInfoItemValidFlags::RIGHT);
        }

        // NOTE: inner distances for cells outside the selected range
        // are not relevant -> we ignore them.
    }
    else
    {
        // current cell is inside the selection

        lcl_MergeBorderOrInnerLine(rLinesState, rCellBoxItem.GetTop(), SvxBoxItemLine::TOP, SvxBoxInfoItemValidFlags::TOP, (nCellFlags & CELL_TOP) != 0);
        lcl_MergeBorderOrInnerLine(rLinesState, rCellBoxItem.GetBottom(), SvxBoxItemLine::BOTTOM, SvxBoxInfoItemValidFlags::BOTTOM, (nCellFlags & CELL_BOTTOM) != 0);
        lcl_MergeBorderOrInnerLine(rLinesState, rCellBoxItem.GetLeft(), SvxBoxItemLine::LEFT, SvxBoxInfoItemValidFlags::LEFT, (nCellFlags & CELL_LEFT) != 0);
        lcl_MergeBorderOrInnerLine(rLinesState, rCellBoxItem.GetRight(), SvxBoxItemLine::RIGHT, SvxBoxInfoItemValidFlags::RIGHT, (nCellFlags & CELL_RIGHT) != 0);

        lcl_MergeDistance(rLinesState, SvxBoxItemLine::TOP, rCellBoxItem.GetDistance(SvxBoxItemLine::TOP));
        lcl_MergeDistance(rLinesState, SvxBoxItemLine::BOTTOM, rCellBoxItem.GetDistance(SvxBoxItemLine::BOTTOM));
        lcl_MergeDistance(rLinesState, SvxBoxItemLine::LEFT, rCellBoxItem.GetDistance(SvxBoxItemLine::LEFT));
        lcl_MergeDistance(rLinesState, SvxBoxItemLine::RIGHT, rCellBoxItem.GetDistance(SvxBoxItemLine::RIGHT));
    }
}

}

void SvxTableController::FillCommonBorderAttrFromSelectedCells( SvxBoxItem& rBoxItem, SvxBoxInfoItem& rBoxInfoItem ) const
{
    if( mxTable.is() )
    {
        const sal_Int32 nRowCount = mxTable->getRowCount();
        const sal_Int32 nColCount = mxTable->getColumnCount();
        if( nRowCount && nColCount )
        {
            CellPos aStart, aEnd;
            const_cast< SvxTableController* >( this )->getSelectedCells( aStart, aEnd );

            // We are adding one more row/column around the block of selected cells.
            // We will be checking the adjoining border of these too.
            const sal_Int32 nLastRow = std::min( aEnd.mnRow + 2, nRowCount );
            const sal_Int32 nLastCol = std::min( aEnd.mnCol + 2, nColCount );

            rBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::ALL, false );
            LinesState aLinesState( rBoxItem, rBoxInfoItem );

            /* Here we go through all the selected cells (enhanced by
             * the adjoining row/column on each side) and determine the
             * lines for presentation. The algorithm is simple:
             * 1. if a border or inner line is set (or unset) in all
             *    cells to the same value, it will be used.
             * 2. if a border or inner line is set only in some cells,
             *    it will be set to indeterminate state (SetValid() on
             *    rBoxInfoItem).
             */
            for( sal_Int32 nRow = std::max( aStart.mnRow - 1, (sal_Int32)0 ); nRow < nLastRow; nRow++ )
            {
                sal_uInt16 nRowFlags = 0;
                nRowFlags |= (nRow == aStart.mnRow) ? CELL_TOP : 0;
                nRowFlags |= (nRow == aEnd.mnRow)   ? CELL_BOTTOM : 0;
                nRowFlags |= (nRow < aStart.mnRow)  ? CELL_UPPER : 0;
                nRowFlags |= (nRow > aEnd.mnRow)    ? CELL_LOWER : 0;

                for( sal_Int32 nCol = std::max( aStart.mnCol - 1, (sal_Int32)0 ); nCol < nLastCol; nCol++ )
                {
                    CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
                    if( !xCell.is() )
                        continue;

                    sal_uInt16 nCellFlags = nRowFlags;
                    nCellFlags |= (nCol == aStart.mnCol) ? CELL_LEFT : 0;
                    nCellFlags |= (nCol == aEnd.mnCol)   ? CELL_RIGHT : 0;
                    nCellFlags |= (nCol < aStart.mnCol)  ? CELL_BEFORE : 0;
                    nCellFlags |= (nCol > aEnd.mnCol)    ? CELL_AFTER : 0;

                    const SfxItemSet& rSet = xCell->GetItemSet();
                    SvxBoxItem aCellBoxItem(mergeDrawinglayerTextDistancesAndSvxBoxItem(rSet));
                    lcl_MergeCommonBorderAttr( aLinesState, aCellBoxItem, nCellFlags );
                }
            }

            if (!aLinesState.aBorderIndeterminate[SvxBoxItemLine::TOP])
                aLinesState.rBoxInfoItem.SetValid(SvxBoxInfoItemValidFlags::TOP);
            if (!aLinesState.aBorderIndeterminate[SvxBoxItemLine::BOTTOM])
                aLinesState.rBoxInfoItem.SetValid(SvxBoxInfoItemValidFlags::BOTTOM);
            if (!aLinesState.aBorderIndeterminate[SvxBoxItemLine::LEFT])
                aLinesState.rBoxInfoItem.SetValid(SvxBoxInfoItemValidFlags::LEFT);
            if (!aLinesState.aBorderIndeterminate[SvxBoxItemLine::RIGHT])
                aLinesState.rBoxInfoItem.SetValid(SvxBoxInfoItemValidFlags::RIGHT);
            if (!aLinesState.aInnerLineIndeterminate[SvxBoxInfoItemLine::HORI])
                aLinesState.rBoxInfoItem.SetValid(SvxBoxInfoItemValidFlags::HORI);
            if (!aLinesState.aInnerLineIndeterminate[SvxBoxInfoItemLine::VERT])
                aLinesState.rBoxInfoItem.SetValid(SvxBoxInfoItemValidFlags::VERT);

            if (!aLinesState.bDistanceIndeterminate)
            {
                if (aLinesState.aDistanceSet[SvxBoxItemLine::TOP])
                    aLinesState.rBoxItem.SetDistance(aLinesState.aDistance[SvxBoxItemLine::TOP], SvxBoxItemLine::TOP);
                if (aLinesState.aDistanceSet[SvxBoxItemLine::BOTTOM])
                    aLinesState.rBoxItem.SetDistance(aLinesState.aDistance[SvxBoxItemLine::BOTTOM], SvxBoxItemLine::BOTTOM);
                if (aLinesState.aDistanceSet[SvxBoxItemLine::LEFT])
                    aLinesState.rBoxItem.SetDistance(aLinesState.aDistance[SvxBoxItemLine::LEFT], SvxBoxItemLine::LEFT);
                if (aLinesState.aDistanceSet[SvxBoxItemLine::RIGHT])
                    aLinesState.rBoxItem.SetDistance(aLinesState.aDistance[SvxBoxItemLine::RIGHT], SvxBoxItemLine::RIGHT);
                aLinesState.rBoxInfoItem.SetValid(SvxBoxInfoItemValidFlags::DISTANCE);
            }
        }
    }
}

bool SvxTableController::selectRow( sal_Int32 row )
{
    if( !mxTable.is() )
        return false;
    CellPos aStart( 0, row ), aEnd( mxTable->getColumnCount() - 1, row );
    StartSelection( aEnd );
    gotoCell( aStart, true, nullptr );
    return true;
}

bool SvxTableController::selectColumn( sal_Int32 column )
{
    if( !mxTable.is() )
        return false;
    CellPos aStart( column, 0 ), aEnd( column, mxTable->getRowCount() - 1 );
    StartSelection( aEnd );
    gotoCell( aStart, true, nullptr );
    return true;
}

bool SvxTableController::deselectRow( sal_Int32 row )
{
    if( !mxTable.is() )
        return false;
    CellPos aStart( 0, row ), aEnd( mxTable->getColumnCount() - 1, row );
    StartSelection( aEnd );
    gotoCell( aStart, false, nullptr );
    return true;
}

bool SvxTableController::deselectColumn( sal_Int32 column )
{
    if( !mxTable.is() )
        return false;
    CellPos aStart( column, 0 ), aEnd( column, mxTable->getRowCount() - 1 );
    StartSelection( aEnd );
    gotoCell( aStart, false, nullptr );
    return true;
}

bool SvxTableController::isRowSelected( sal_Int32 nRow )
{
    if( hasSelectedCells() )
    {
        CellPos aFirstPos, aLastPos;
        getSelectedCells( aFirstPos, aLastPos );
        if( (aFirstPos.mnCol == 0) && (nRow >= aFirstPos.mnRow && nRow <= aLastPos.mnRow) && (mxTable->getColumnCount() - 1 == aLastPos.mnCol) )
            return true;
    }
    return false;
}

bool SvxTableController::isColumnSelected( sal_Int32 nColumn )
{
    if( hasSelectedCells() )
    {
        CellPos aFirstPos, aLastPos;
        getSelectedCells( aFirstPos, aLastPos );
        if( (aFirstPos.mnRow == 0) && (nColumn >= aFirstPos.mnCol && nColumn <= aLastPos.mnCol) && (mxTable->getRowCount() - 1 == aLastPos.mnRow) )
            return true;
    }
    return false;
}

bool SvxTableController::isRowHeader()
{
    SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mxTableObj.get() );
    SdrModel* pModel = pTableObj ? pTableObj->GetModel() : nullptr;

    if( !pTableObj || !pModel )
        return false;

    TableStyleSettings aSettings( pTableObj->getTableStyleSettings() );

    return aSettings.mbUseFirstRow;
}

bool SvxTableController::isColumnHeader()
{
    SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mxTableObj.get() );
    SdrModel* pModel = pTableObj ? pTableObj->GetModel() : nullptr;

    if( !pTableObj || !pModel )
        return false;

    TableStyleSettings aSettings( pTableObj->getTableStyleSettings() );

    return aSettings.mbUseFirstColumn;
}

bool SvxTableController::setCursorLogicPosition(const Point& rPosition, bool bPoint)
{
    if (mxTableObj->GetObjIdentifier() != OBJ_TABLE)
        return false;

    SdrTableObj* pTableObj = static_cast<SdrTableObj*>(mxTableObj.get());
    CellPos aCellPos;
    if (pTableObj->CheckTableHit(rPosition, aCellPos.mnCol, aCellPos.mnRow, 0) != SDRTABLEHIT_NONE)
    {
        // Position is a table cell.
        if (mbCellSelectionMode)
        {
            // We have a table selection already: adjust the point or the mark.
            if (bPoint)
                setSelectedCells(maCursorFirstPos, aCellPos);
            else
                setSelectedCells(aCellPos, maCursorLastPos);
            return true;
        }
        else if (aCellPos != maMouseDownPos)
        {
            // No selection, but rPosition is at an other cell: start table selection.
            StartSelection(maMouseDownPos);
            // Update graphic selection, should be hidden now.
            mpView->AdjustMarkHdl();
        }
    }

    return false;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
