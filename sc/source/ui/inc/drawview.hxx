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

#ifndef SC_DRAWVIEW_HXX
#define SC_DRAWVIEW_HXX

#include <svx/fmview.hxx>

#include "global.hxx"

namespace com { namespace sun { namespace star { namespace datatransfer { class XTransferable; } } } }

class ScDocument;
class ScViewData;
class ScDrawObjData;

class ScDrawView: public FmFormView
{
    ScViewData*             pViewData;
    OutputDevice*           pDev;                   //! needed ?
    ScDocument*             pDoc;
    SCTAB                   nTab;
    Fraction                aScaleX;                // Factor for Drawing-MapMode
    Fraction                aScaleY;
    SdrDropMarkerOverlay*   pDropMarker;
    SdrObject*              pDropMarkObj;
    sal_Bool                    bInConstruct;

    void            Construct();
    void            UpdateBrowser();

protected:
    virtual void    ModelHasChanged();

    // add custom handles (used by other apps, e.g. AnchorPos)
    virtual void AddCustomHdl();

    void ImplClearCalcDropMarker();

public:
                    ScDrawView( OutputDevice* pOut, ScViewData* pData );
    virtual         ~ScDrawView();

    virtual void    MarkListHasChanged();
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual void    DoConnect(SdrOle2Obj* pOleObj);

    virtual void    MakeVisible( const Rectangle& rRect, Window& rWin );

    virtual void    DeleteMarked();

    void            MarkDropObj( SdrObject* pObj );

    void            SetMarkedToLayer( sal_uInt8 nLayerNo );

    void            InvalidateAttribs();
    void            InvalidateDrawTextAttrs();

    sal_Bool            BeginDrag( Window* pWindow, const Point& rStartPos );
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

    void            VCAddWin( Window* pWin );
    void            VCRemoveWin( Window* pWin );

    void            UpdateIMap( SdrObject* pObj );

    sal_uInt16          GetPopupMenuId();
    void            UpdateUserViewOptions();

    void            SetMarkedOriginalSize();

    sal_Bool            SelectObject( const String& rName );
    bool            HasMarkedControl() const;
    bool            HasMarkedInternal() const;

    bool            InsertObjectSafe(SdrObject* pObj, SdrPageView& rPV, sal_uLong nOptions=0);

    /** Returns the selected object, if it is the caption object of a cell note.
        @param ppCaptData  (out-param) If not null, returns the pointer to the caption object data. */
    SdrObject*      GetMarkedNoteCaption( ScDrawObjData** ppCaptData = 0 );

    /** Locks/unlocks the specified layer in the draw page.
        Unlocked layer is required to be able to edit the contained objects. */
    void            LockCalcLayer( SdrLayerID nLayer, bool bLock = true );
    /** Unlocks the specified layer in the draw page. */
    inline void     UnlockCalcLayer( SdrLayerID nLayer ) { LockCalcLayer( nLayer, false ); }

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
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > CopyToTransferable();

    static void CheckOle( const SdrMarkList& rMarkList, sal_Bool& rAnyOle, sal_Bool& rOneOle );
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
