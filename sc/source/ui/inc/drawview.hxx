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



#ifndef SC_DRAWVIEW_HXX
#define SC_DRAWVIEW_HXX

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
    OutputDevice*           pDev;                   //! noetig ?
    ScDocument*             pDoc;
    SCTAB                   nTab;
    Fraction                aScaleX;                // Faktor fuer Drawing-MapMode
    Fraction                aScaleY;
    SdrDropMarkerOverlay*   pDropMarker;
    SdrObject*              pDropMarkObj;
    sal_Bool                    bInConstruct;
    //HMHBOOL                   bDisableHdl;

    void            Construct();
    void            UpdateBrowser();

protected:
    virtual void    ModelHasChanged();

    // add custom handles (used by other apps, e.g. AnchorPos)
    virtual void AddCustomHdl();

    void ImplClearCalcDropMarker();

    // support enhanced text edit for draw objects
    virtual SdrUndoManager* getSdrUndoManagerForEnhancedTextEdit() const;

public:
                    ScDrawView( OutputDevice* pOut, ScViewData* pData );
    virtual         ~ScDrawView();

    virtual void    MarkListHasChanged();
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual void    DoConnect(SdrOle2Obj* pOleObj);

    virtual void    MakeVisible( const Rectangle& rRect, Window& rWin );

    virtual void    DeleteMarked();

    virtual sal_Bool SdrBeginTextEdit(
        SdrObject* pObj,
        SdrPageView* pPV = 0L,
        ::Window* pWin = 0L,
        sal_Bool bIsNewObj = sal_False,
        SdrOutliner* pGivenOutliner = 0L,
        OutlinerView* pGivenOutlinerView = 0L,
        sal_Bool bDontDeleteOutliner = sal_False,
        sal_Bool bOnlyOneView = sal_False,
        sal_Bool bGrabFocus = sal_True);

    virtual SdrEndTextEditKind SdrEndTextEdit( sal_Bool bDontDeleteReally = sal_False );

    void            DrawMarks( OutputDevice* pOut ) const;

    void            MarkDropObj( SdrObject* pObj );

    //HMHBOOL           IsDisableHdl() const    { return bDisableHdl; }

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

    void            SetAnchor( ScAnchorType );
    ScAnchorType    GetAnchor() const;

    void            VCAddWin( Window* pWin );
    void            VCRemoveWin( Window* pWin );

    void            UpdateIMap( SdrObject* pObj );

    sal_uInt16          GetPopupMenuId();
    void            UpdateUserViewOptions();

    void            SetMarkedOriginalSize();

    sal_Bool            SelectObject( const String& rName );
//UNUSED2008-05  String         GetSelectedChartName() const;
    bool            HasMarkedControl() const;
    bool            HasMarkedInternal() const;

    FASTBOOL        InsertObjectSafe(SdrObject* pObj, SdrPageView& rPV, sal_uLong nOptions=0);

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

    SdrEndTextEditKind  ScEndTextEdit();    // ruft SetDrawTextUndo(0)
//UNUSED2009-05 void                    CaptionTextDirection(sal_uInt16 nSlot);
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > CopyToTransferable();
    SdrObject*  GetObjectByName(const String& rName);
    sal_Bool           GetObjectIsMarked(  SdrObject * pObject );
    sal_Bool           SelectCurrentViewObject( const String& rName );
};




#endif

