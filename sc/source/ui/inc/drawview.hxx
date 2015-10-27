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

#ifndef INCLUDED_SC_SOURCE_UI_INC_DRAWVIEW_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DRAWVIEW_HXX

#include <svx/fmview.hxx>

#include "global.hxx"

namespace com { namespace sun { namespace star { namespace datatransfer { class XTransferable; } } } }

class ScDocument;
class ScViewData;
class ScDrawObjData;
class SdrUndoManager;

class ScDrawView: public FmFormView
{
    ScViewData*             pViewData;
    VclPtr<OutputDevice>    pDev;                   //! needed ?
    ScDocument*             pDoc;
    SCTAB                   nTab;
    Fraction                aScaleX;                // Factor for Drawing-MapMode
    Fraction                aScaleY;
    SdrDropMarkerOverlay*   pDropMarker;
    SdrObject*              pDropMarkObj;
    bool                    bInConstruct;

    void            Construct();

protected:
    virtual void    ModelHasChanged() override;

    // add custom handles (used by other apps, e.g. AnchorPos)
    virtual void AddCustomHdl() override;

    void ImplClearCalcDropMarker();

    // support enhanced text edit for draw objects
    virtual SdrUndoManager* getSdrUndoManagerForEnhancedTextEdit() const override;

public:
                    ScDrawView( OutputDevice* pOut, ScViewData* pData );
    virtual         ~ScDrawView();

    virtual void    MarkListHasChanged() override;
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual void    DoConnect(SdrOle2Obj* pOleObj) override;

    virtual void    MakeVisible( const Rectangle& rRect, vcl::Window& rWin ) override;

    virtual void    DeleteMarked() override;

    virtual bool    SdrBeginTextEdit(
                      SdrObject* pObj,
                      SdrPageView* pPV = 0L,
                      vcl::Window* pWin = 0L,
                      bool bIsNewObj = false,
                      SdrOutliner* pGivenOutliner = 0L,
                      OutlinerView* pGivenOutlinerView = 0L,
                      bool bDontDeleteOutliner = false,
                      bool bOnlyOneView = false,
                      bool bGrabFocus = true) override;

    virtual SdrEndTextEditKind SdrEndTextEdit( bool bDontDeleteReally = false ) override;

    void            MarkDropObj( SdrObject* pObj );

    void            SetMarkedToLayer( sal_uInt8 nLayerNo );

    void            InvalidateAttribs();
    void            InvalidateDrawTextAttrs();

    bool            BeginDrag( vcl::Window* pWindow, const Point& rStartPos );
    void            DoCut();
    void            DoCopy();

    void            GetScale( Fraction& rFractX, Fraction& rFractY ) const;
    void            RecalcScale();
    void            UpdateWorkArea();
    SCTAB           GetTab() const      { return nTab; }

    void            CalcNormScale( Fraction& rFractX, Fraction& rFractY ) const;

    void            SetPageAnchored();
    void            SetCellAnchored();
    ScAnchorType    GetAnchorType() const;

    void            UpdateIMap( SdrObject* pObj );

    void            UpdateUserViewOptions();

    void            SetMarkedOriginalSize();

    bool            SelectObject( const OUString& rName );
    bool            HasMarkedControl() const;
    bool            HasMarkedInternal() const;

    bool            InsertObjectSafe(SdrObject* pObj, SdrPageView& rPV, SdrInsertFlags nOptions=SdrInsertFlags::NONE);

    /** Returns the selected object, if it is the caption object of a cell note.
        @param ppCaptData  (out-param) If not null, returns the pointer to the caption object data. */
    SdrObject*      GetMarkedNoteCaption( ScDrawObjData** ppCaptData = 0 );

    /** Locks/unlocks the specified layer in the draw page.
        Unlocked layer is required to be able to edit the contained objects. */
    void            LockCalcLayer( SdrLayerID nLayer, bool bLock = true );

    /** Locks/unlocks the background layer that contains background objects.
        Unlocked layer is required to be able to edit the objects. */
    inline void     LockBackgroundLayer( bool bLock = true ) { LockCalcLayer( SC_LAYER_BACK, bLock ); }
    /** Unlocks the background layer that contains background objects. */
    inline void     UnlockBackgroundLayer() { LockBackgroundLayer( false ); }

    /** Locks/unlocks the internal layer that contains caption objects of cell notes.
        Unlocked layer is required to be able to edit the contained objects. */
    inline void     LockInternalLayer( bool bLock = true ) { LockCalcLayer( SC_LAYER_INTERN, bLock ); }
    /** Unlocks the internal layer that contains caption objects of cell notes. */
    inline void     UnlockInternalLayer() { LockInternalLayer( false ); }

    SdrEndTextEditKind  ScEndTextEdit();    // calls SetDrawTextUndo(0)
    css::uno::Reference< css::datatransfer::XTransferable > CopyToTransferable();

    SdrObject*  GetObjectByName(const OUString& rName);
    bool        GetObjectIsMarked(  SdrObject * pObject );
    bool        SelectCurrentViewObject( const OUString& rName );

    // #i123922# helper which checks if a Graphic may be applied to an existing
    // SdrObject; if it's a SdrGrafObj the fill will be replaced. If it's a
    // fillable, non-OLE SdrObject, the FillStyle will be adapted
    SdrObject* ApplyGraphicToObject(
        SdrObject& rHitObject,
        const Graphic& rGraphic,
        const OUString& rBeginUndoText,
        const OUString& rFile,
        const OUString& rFilter);

    static void CheckOle( const SdrMarkList& rMarkList, bool& rAnyOle, bool& rOneOle );
    void SyncForGrid( SdrObject* pObj );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
