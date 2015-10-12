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
#include <com/sun/star/table/XTable.hpp>
#include <rtl/ref.hxx>

#include <svx/sdr/overlay/overlayobjectlist.hxx>
#include <svx/selectioncontroller.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdview.hxx>

class SdrObjEditView;
class SdrObject;
class SfxItemSet;
class SvxBoxInfoItem;
class SvxBoxItem;

namespace sdr { namespace table {

class TableModel;

class SVX_DLLPUBLIC SvxTableController: public sdr::SelectionController
{
public:
    SVX_DLLPRIVATE SvxTableController( SdrObjEditView* pView, const SdrObject* pObj );
    SVX_DLLPRIVATE virtual ~SvxTableController();

    // from sdr::SelectionController
    SVX_DLLPRIVATE virtual bool onKeyInput(const KeyEvent& rKEvt, vcl::Window* pWin) override;
    SVX_DLLPRIVATE virtual bool onMouseButtonDown(const MouseEvent& rMEvt, vcl::Window* pWin) override;
    SVX_DLLPRIVATE virtual bool onMouseButtonUp(const MouseEvent& rMEvt, vcl::Window* pWin) override;
    SVX_DLLPRIVATE virtual bool onMouseMove(const MouseEvent& rMEvt, vcl::Window* pWin) override;

    SVX_DLLPRIVATE virtual bool HasMarked() override;
    SVX_DLLPRIVATE virtual bool DeleteMarked() override;

    SVX_DLLPRIVATE virtual void onSelectionHasChanged() override;

    SVX_DLLPRIVATE virtual void GetState( SfxItemSet& rSet ) override;
    SVX_DLLPRIVATE virtual void Execute( SfxRequest& rReq ) override;

    SVX_DLLPRIVATE virtual bool GetStyleSheet( SfxStyleSheet* &rpStyleSheet ) const override;
    SVX_DLLPRIVATE virtual bool SetStyleSheet( SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr ) override;

    SVX_DLLPRIVATE virtual bool TakeFormatPaintBrush( std::shared_ptr< SfxItemSet >& rFormatSet  ) override;
    SVX_DLLPRIVATE virtual bool ApplyFormatPaintBrush( SfxItemSet& rFormatSet, bool bNoCharacterFormats, bool bNoParagraphFormats ) override;

    // slots
    SVX_DLLPRIVATE void onInsert( sal_uInt16 nSId, const SfxItemSet* pArgs = 0 );
    SVX_DLLPRIVATE void onDelete( sal_uInt16 nSId );
    SVX_DLLPRIVATE void onSelect( sal_uInt16 nSId );
    SVX_DLLPRIVATE void onFormatTable( SfxRequest& rReq );
    SVX_DLLPRIVATE void MergeMarkedCells();
    SVX_DLLPRIVATE void SplitMarkedCells();
    SVX_DLLPRIVATE void DistributeColumns();
    SVX_DLLPRIVATE void DistributeRows();
    SVX_DLLPRIVATE void SetVertical( sal_uInt16 nSId );

    SVX_DLLPRIVATE static rtl::Reference< sdr::SelectionController > create( SdrObjEditView* pView, const SdrObject* pObj, const rtl::Reference< sdr::SelectionController >& xRefController );

    SVX_DLLPRIVATE void MergeAttrFromSelectedCells(SfxItemSet& rAttr, bool bOnlyHardAttr) const;
    SVX_DLLPRIVATE void SetAttrToSelectedCells(const SfxItemSet& rAttr, bool bReplaceAll);
    /** Fill the values that are common for all selected cells.
      *
      * This lets the Borders dialog to display the line arrangement
      * properly.
      */
    SVX_DLLPRIVATE void FillCommonBorderAttrFromSelectedCells(SvxBoxItem& rBox, SvxBoxInfoItem& rBoxInfo) const;

    SVX_DLLPRIVATE virtual bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const override;
    SVX_DLLPRIVATE virtual bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll) override;

    SVX_DLLPRIVATE virtual bool GetMarkedObjModel( SdrPage* pNewPage ) override;
    SVX_DLLPRIVATE virtual bool PasteObjModel( const SdrModel& rModel ) override;

    SVX_DLLPRIVATE virtual bool hasSelectedCells() const override { return mbCellSelectionMode || mpView->IsTextEdit(); }
    /// @see sdr::SelectionController::setCursorLogicPosition().
    SVX_DLLPRIVATE virtual bool setCursorLogicPosition(const Point& rPosition, bool bPoint) override;

    void getSelectedCells( CellPos& rFirstPos, CellPos& rLastPos );
    void setSelectedCells( const CellPos& rFirstPos, const CellPos& rLastPos );
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
    sdr::table::SdrTableObj* GetTableObj() { return dynamic_cast< sdr::table::SdrTableObj* >( mxTableObj.get() ); }
private:
    SvxTableController(SvxTableController &) = delete;
    void operator =(SvxTableController &) = delete;

    // internals
    SVX_DLLPRIVATE void ApplyBorderAttr( const SfxItemSet& rAttr );
    SVX_DLLPRIVATE void UpdateTableShape();

    SVX_DLLPRIVATE void SetTableStyle( const SfxItemSet* pArgs );
    SVX_DLLPRIVATE void SetTableStyleSettings( const SfxItemSet* pArgs );

    SVX_DLLPRIVATE bool PasteObject( SdrTableObj* pPasteTableObj );

    SVX_DLLPRIVATE bool checkTableObject();
    SVX_DLLPRIVATE const CellPos& getSelectionStart();
    SVX_DLLPRIVATE void setSelectionStart( const CellPos& rPos );
    SVX_DLLPRIVATE const CellPos& getSelectionEnd();
    SVX_DLLPRIVATE void checkCell( CellPos& rPos );

    SVX_DLLPRIVATE void MergeRange( sal_Int32 nFirstCol, sal_Int32 nFirstRow, sal_Int32 nLastCol, sal_Int32 nLastRow );

    SVX_DLLPRIVATE void EditCell( const CellPos& rPos, vcl::Window* pWindow, const ::com::sun::star::awt::MouseEvent* pMouseEvent = 0, sal_uInt16 nAction = 0 );
    SVX_DLLPRIVATE bool StopTextEdit();

    SVX_DLLPRIVATE sal_uInt16 getKeyboardAction( const KeyEvent& rKEvt, vcl::Window* pWindow );
    SVX_DLLPRIVATE bool executeAction( sal_uInt16 nAction, bool bSelect, vcl::Window* pWindow );
    SVX_DLLPRIVATE void gotoCell( const CellPos& rCell, bool bSelect, vcl::Window* pWindow, sal_uInt16 nAction = 0 );

    SVX_DLLPRIVATE void StartSelection( const CellPos& rPos );
    SVX_DLLPRIVATE void UpdateSelection( const CellPos& rPos );
    SVX_DLLPRIVATE void RemoveSelection();
    SVX_DLLPRIVATE void updateSelectionOverlay();
    SVX_DLLPRIVATE void destroySelectionOverlay();

    SVX_DLLPRIVATE void findMergeOrigin( CellPos& rPos );

    DECL_LINK_TYPED( UpdateHdl, void *, void );

    //TableModelRef mxTable;
    rtl::Reference< TableModel > mxTable;

    CellPos maCursorFirstPos;
    CellPos maCursorLastPos;
    bool mbCellSelectionMode;
    CellPos maMouseDownPos;
    bool mbLeftButtonDown;
    sdr::overlay::OverlayObjectList*  mpSelectionOverlay;

    SdrView* mpView;
    SdrObjectWeakRef mxTableObj;
    SdrModel* mpModel;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > mxModifyListener;

    ImplSVEvent * mnUpdateEvent;
};

rtl::Reference< sdr::SelectionController > CreateTableController( SdrObjEditView* pView, const SdrObject* pObj, const rtl::Reference< sdr::SelectionController >& xRefController );

} }

#endif // INCLUDED_SVX_SDR_TABLE_TABLECONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
