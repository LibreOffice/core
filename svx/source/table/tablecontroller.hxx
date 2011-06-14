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

#ifndef _SVX_TABLECONTROLLER_HXX_
#define _SVX_TABLECONTROLLER_HXX_

#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <rtl/ref.hxx>

#include <svx/sdr/overlay/overlayobjectlist.hxx>
#include <svx/selectioncontroller.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdview.hxx>
#include <tablemodel.hxx>

class SdrObjEditView;
class SdrObject;
class SfxItemSet;

namespace sdr { namespace table {

const sal_Int16 SELTYPE_NONE = 0;
const sal_Int16 SELTYPE_MOUSE = 1;
const sal_Int16 SELTYPE_KEYS = 2;

class SVX_DLLPUBLIC SvxTableController: public sdr::SelectionController
{
public:
    SVX_DLLPRIVATE SvxTableController( SdrObjEditView* pView, const SdrObject* pObj );
    SVX_DLLPRIVATE virtual ~SvxTableController();

    // from sdr::SelectionController
    SVX_DLLPRIVATE virtual bool onKeyInput(const KeyEvent& rKEvt, Window* pWin);
    SVX_DLLPRIVATE virtual bool onMouseButtonDown(const MouseEvent& rMEvt, Window* pWin);
    SVX_DLLPRIVATE virtual bool onMouseButtonUp(const MouseEvent& rMEvt, Window* pWin);
    SVX_DLLPRIVATE virtual bool onMouseMove(const MouseEvent& rMEvt, Window* pWin);

    SVX_DLLPRIVATE virtual bool DeleteMarked();

    SVX_DLLPRIVATE virtual void onSelectionHasChanged();

    SVX_DLLPRIVATE virtual void GetState( SfxItemSet& rSet );
    SVX_DLLPRIVATE virtual void Execute( SfxRequest& rReq );

    SVX_DLLPRIVATE virtual bool GetStyleSheet( SfxStyleSheet* &rpStyleSheet ) const;
    SVX_DLLPRIVATE virtual bool SetStyleSheet( SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr );

    SVX_DLLPRIVATE virtual bool TakeFormatPaintBrush( boost::shared_ptr< SfxItemSet >& rFormatSet  );
    SVX_DLLPRIVATE virtual bool ApplyFormatPaintBrush( SfxItemSet& rFormatSet, bool bNoCharacterFormats, bool bNoParagraphFormats );

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

    SVX_DLLPRIVATE virtual bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const;
    SVX_DLLPRIVATE virtual bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll);

    SVX_DLLPRIVATE virtual bool GetMarkedObjModel( SdrPage* pNewPage );
    SVX_DLLPRIVATE virtual bool PasteObjModel( const SdrModel& rModel );

    SVX_DLLPRIVATE bool hasSelectedCells() const { return mbCellSelectionMode || mpView->IsTextEdit(); }

    void getSelectedCells( CellPos& rFirstPos, CellPos& rLastPos );
    void setSelectedCells( const CellPos& rFirstPos, const CellPos& rLastPos );
    void clearSelection();
    void selectAll();

    SVX_DLLPRIVATE void onTableModified();

private:
    SvxTableController(SvxTableController &); // not defined
    void operator =(SvxTableController &); // not defined

    // internals
    SVX_DLLPRIVATE void ApplyBorderAttr( const SfxItemSet& rAttr );
    SVX_DLLPRIVATE void UpdateTableShape();

    SVX_DLLPRIVATE void SetTableStyle( const SfxItemSet* pArgs );
    SVX_DLLPRIVATE void SetTableStyleSettings( const SfxItemSet* pArgs );

    SVX_DLLPRIVATE bool PasteObject( SdrTableObj* pPasteTableObj );

    SVX_DLLPRIVATE bool checkTableObject();
    SVX_DLLPRIVATE bool updateTableObject();
    SVX_DLLPRIVATE const CellPos& getSelectionStart();
    SVX_DLLPRIVATE void setSelectionStart( const CellPos& rPos );
    SVX_DLLPRIVATE const CellPos& getSelectionEnd();
    SVX_DLLPRIVATE ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellCursor > getSelectionCursor();
    SVX_DLLPRIVATE void checkCell( CellPos& rPos );

    SVX_DLLPRIVATE void MergeRange( sal_Int32 nFirstCol, sal_Int32 nFirstRow, sal_Int32 nLastCol, sal_Int32 nLastRow );

    SVX_DLLPRIVATE void EditCell( const CellPos& rPos, ::Window* pWindow, const ::com::sun::star::awt::MouseEvent* pMouseEvent = 0, sal_uInt16 nAction = 0 );
    SVX_DLLPRIVATE bool StopTextEdit();

    SVX_DLLPRIVATE void DeleteTable();

    SVX_DLLPRIVATE sal_uInt16 getKeyboardAction( const KeyEvent& rKEvt, Window* pWindow );
    SVX_DLLPRIVATE bool executeAction( sal_uInt16 nAction, bool bSelect, Window* pWindow );
    SVX_DLLPRIVATE void gotoCell( const CellPos& rCell, bool bSelect, Window* pWindow, sal_uInt16 nAction = 0 );

    SVX_DLLPRIVATE void StartSelection( const CellPos& rPos );
    SVX_DLLPRIVATE void UpdateSelection( const CellPos& rPos );
    SVX_DLLPRIVATE void RemoveSelection();
    SVX_DLLPRIVATE void updateSelectionOverlay();
    SVX_DLLPRIVATE void destroySelectionOverlay();

    SVX_DLLPRIVATE void findMergeOrigin( CellPos& rPos );

    DECL_LINK( UpdateHdl, void * );

    TableModelRef mxTable;

    CellPos maCursorFirstPos;
    CellPos maCursorLastPos;
    bool mbCellSelectionMode;
    CellPos maMouseDownPos;
    bool mbLeftButtonDown;
    ::sdr::overlay::OverlayObjectList*  mpSelectionOverlay;

    SdrView* mpView;
    SdrObjectWeakRef mxTableObj;
    SdrModel* mpModel;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > mxModifyListener;

    sal_uLong mnUpdateEvent;
};

} }

#endif // _SVX_TABLECONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
