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

#ifndef INCLUDED_SVX_SDR_TABLE_TABLECONTROLLER_HXX
#define INCLUDED_SVX_SDR_TABLE_TABLECONTROLLER_HXX

#include <com/sun/star/util/XModifyListener.hpp>
#include <rtl/ref.hxx>

#include <svx/sdr/overlay/overlayobjectlist.hxx>
#include <svx/selectioncontroller.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdview.hxx>
#include <memory>

struct ImplSVEvent;
class SdrView;
class SdrObject;
class SfxItemSet;
class SvxBoxInfoItem;
class SvxBoxItem;

namespace sdr::table {

class TableModel;

class SVXCORE_DLLPUBLIC SvxTableController final : public sdr::SelectionController
{
public:
    SVX_DLLPRIVATE SvxTableController(
        SdrView& rView,
        const SdrTableObj& rObj);
    SVX_DLLPRIVATE virtual ~SvxTableController() override;

    // from sdr::SelectionController
    SVX_DLLPRIVATE virtual bool onKeyInput(const KeyEvent& rKEvt, vcl::Window* pWin) override;
    SVX_DLLPRIVATE virtual bool onMouseButtonDown(const MouseEvent& rMEvt, vcl::Window* pWin) override;
    SVX_DLLPRIVATE virtual bool onMouseButtonUp(const MouseEvent& rMEvt, vcl::Window* pWin) override;
    SVX_DLLPRIVATE virtual bool onMouseMove(const MouseEvent& rMEvt, vcl::Window* pWin) override;

    SVX_DLLPRIVATE bool HasMarked() const;
    SVX_DLLPRIVATE virtual bool DeleteMarked() override;

    SVX_DLLPRIVATE virtual void onSelectionHasChanged() override;

    SVX_DLLPRIVATE virtual void GetState( SfxItemSet& rSet ) override;
    virtual void Execute( SfxRequest& rReq ) override;

    SVX_DLLPRIVATE virtual bool GetStyleSheet( SfxStyleSheet* &rpStyleSheet ) const override;
    SVX_DLLPRIVATE virtual bool SetStyleSheet( SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr ) override;

    SVX_DLLPRIVATE virtual bool ApplyFormatPaintBrush( SfxItemSet& rFormatSet, bool bNoCharacterFormats, bool bNoParagraphFormats ) override;

    // slots
    SVX_DLLPRIVATE void onInsert( sal_uInt16 nSId, const SfxItemSet* pArgs = nullptr );
    SVX_DLLPRIVATE void onDelete( sal_uInt16 nSId );
    SVX_DLLPRIVATE void onSelect( sal_uInt16 nSId );
    SVX_DLLPRIVATE void onFormatTable(const SfxRequest& rReq);
    SVX_DLLPRIVATE void MergeMarkedCells();
    SVX_DLLPRIVATE void SplitMarkedCells(const SfxRequest& rReq);
    SVX_DLLPRIVATE void DistributeColumns( const bool bOptimize, const bool bMinimize );
    SVX_DLLPRIVATE void DistributeRows( const bool bOptimize, const bool bMinimize );
    SVX_DLLPRIVATE void SetVertical( sal_uInt16 nSId );
    SVX_DLLPRIVATE void changeTableEdge(const SfxRequest& rReq);

    SVX_DLLPRIVATE static rtl::Reference< sdr::SelectionController > create(
        SdrView& rView,
        const SdrTableObj& rObj,
        const rtl::Reference< sdr::SelectionController >& xRefController);

    SVX_DLLPRIVATE void MergeAttrFromSelectedCells(SfxItemSet& rAttr, bool bOnlyHardAttr) const;
    SVX_DLLPRIVATE void SetAttrToSelectedCells(const SfxItemSet& rAttr, bool bReplaceAll);
    void SetAttrToSelectedShape(const SfxItemSet& rAttr);
    /** Fill the values that are common for all selected cells.
      *
      * This lets the Borders dialog to display the line arrangement
      * properly.
      */
    SVX_DLLPRIVATE void FillCommonBorderAttrFromSelectedCells(SvxBoxItem& rBox, SvxBoxInfoItem& rBoxInfo) const;

    SVX_DLLPRIVATE virtual bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const override;
    SVX_DLLPRIVATE virtual bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll) override;

    SVX_DLLPRIVATE virtual SdrObject* GetMarkedSdrObjClone( SdrModel& rTargetModel ) override;
    SVX_DLLPRIVATE virtual bool PasteObjModel( const SdrModel& rModel ) override;

    SVX_DLLPRIVATE virtual bool hasSelectedCells() const override { return mbCellSelectionMode || mrView.IsTextEdit(); }
    /// @see sdr::SelectionController::setCursorLogicPosition().
    SVX_DLLPRIVATE virtual bool setCursorLogicPosition(const Point& rPosition, bool bPoint) override;

    /// @see sdr::SelectionController::getSelectedCells().
    void getSelectedCells( CellPos& rFirstPos, CellPos& rLastPos ) override;
    void setSelectedCells( const CellPos& rFirstPos, const CellPos& rLastPos );

    virtual bool ChangeFontSize(bool bGrow, const FontList* pFontList) override;

    void clearSelection();
    void selectAll();

    SVX_DLLPRIVATE void onTableModified();

    bool selectRow( sal_Int32 row );
    bool selectColumn( sal_Int32 column );
    bool deselectRow( sal_Int32 row );
    bool deselectColumn( sal_Int32 column );
    bool isRowSelected( sal_Int32 nRow );
    bool isColumnSelected( sal_Int32 nColumn );
    bool isRowHeader();
    bool isColumnHeader();
    sdr::table::SdrTableObj* GetTableObj() { return mxTableObj.get(); }
private:
    SvxTableController(SvxTableController const &) = delete;
    SvxTableController& operator =(SvxTableController const &) = delete;

    // internals
    enum class TblAction
    {
        NONE,
        GotoFirstCell, GotoFirstColumn, GotoFirstRow,
        GotoLeftCell, GotoUpCell, GotoRightCell, GotoDownCell,
        GotoLastCell, GotoLastColumn, GotoLastRow,
        EditCell, StopTextEdit,
        RemoveSelection,
        HandledByView, Tab
    };
    SVX_DLLPRIVATE void ApplyBorderAttr( const SfxItemSet& rAttr );
    SVX_DLLPRIVATE void UpdateTableShape();

    SVX_DLLPRIVATE void SetTableStyle( const SfxItemSet* pArgs );
    SVX_DLLPRIVATE void SetTableStyleSettings( const SfxItemSet* pArgs );

    SVX_DLLPRIVATE bool PasteObject( SdrTableObj const * pPasteTableObj );

    SVX_DLLPRIVATE bool checkTableObject();
    SVX_DLLPRIVATE const CellPos& getSelectionStart();
    SVX_DLLPRIVATE void setSelectionStart( const CellPos& rPos );
    SVX_DLLPRIVATE const CellPos& getSelectionEnd();
    SVX_DLLPRIVATE void checkCell( CellPos& rPos );

    SVX_DLLPRIVATE void MergeRange( sal_Int32 nFirstCol, sal_Int32 nFirstRow, sal_Int32 nLastCol, sal_Int32 nLastRow );

    SVX_DLLPRIVATE void EditCell(const CellPos& rPos, vcl::Window* pWindow, TblAction nAction);
    SVX_DLLPRIVATE void StopTextEdit();

    SVX_DLLPRIVATE TblAction getKeyboardAction(const KeyEvent& rKEvt);
    SVX_DLLPRIVATE bool executeAction(TblAction nAction, bool bSelect, vcl::Window* pWindow);
    SVX_DLLPRIVATE void gotoCell(const CellPos& rCell, bool bSelect, vcl::Window* pWindow, TblAction nAction = TblAction::NONE);

    SVX_DLLPRIVATE void StartSelection( const CellPos& rPos );
    SVX_DLLPRIVATE void UpdateSelection( const CellPos& rPos );
    SVX_DLLPRIVATE void RemoveSelection();
    SVX_DLLPRIVATE void updateSelectionOverlay();
    SVX_DLLPRIVATE void destroySelectionOverlay();

    SVX_DLLPRIVATE void findMergeOrigin( CellPos& rPos );

    DECL_LINK( UpdateHdl, void *, void );

    //TableModelRef mxTable;
    rtl::Reference< TableModel > mxTable;

    CellPos maCursorFirstPos;
    CellPos maCursorLastPos;
    bool mbCellSelectionMode;
    bool mbHasJustMerged;
    CellPos maMouseDownPos;
    bool mbLeftButtonDown;
    std::unique_ptr<sdr::overlay::OverlayObjectList>  mpSelectionOverlay;
    SdrView& mrView;
    tools::WeakReference<SdrTableObj> mxTableObj;
    css::uno::Reference< css::util::XModifyListener > mxModifyListener;
    ImplSVEvent * mnUpdateEvent;
};

rtl::Reference< sdr::SelectionController > CreateTableController(
     SdrView& rView,
     const SdrTableObj& rObj,
     const rtl::Reference< sdr::SelectionController >& xRefController );

}

#endif // INCLUDED_SVX_SDR_TABLE_TABLECONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
