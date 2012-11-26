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



#ifndef SD_VIEW_HXX
#define SD_VIEW_HXX

#include "pres.hxx"
#include <tools/gen.hxx>
#include <svtools/transfer.hxx>
#include <svx/fmview.hxx>
#include <svx/svdpage.hxx>
#include "fupoor.hxx"

#include "smarttag.hxx"

class SdDrawDocument;
class SdrOle2Obj;
class SdrGrafObj;
class SdrMediaObj;
class OutputDevice;
class VirtualDevice;
class ImageMap;
class Point;
class Graphic;
class SdrOutliner;
class TransferableDataHelper;
struct StyleRequestData;
class Outliner;

namespace sd {

class DrawDocShell;
struct SdNavigatorDropEvent;
class ViewShell;
class Window;
class ViewClipboard;

// -------------------
// - SdViewRedrawRec -
// -------------------

struct SdViewRedrawRec
{
    OutputDevice* mpOut;
    Rectangle     aRect;
};


class View : public FmFormView
{
public:
    View (
        SdDrawDocument& rDrawDoc,
        OutputDevice* pOutDev,
        ViewShell* pViewSh=NULL);
    virtual ~View (void);

    void                    CompleteRedraw( OutputDevice* pOutDev, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0L);

    virtual bool            GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr = false ) const;
    virtual bool            SetAttributes(const SfxItemSet& rSet, bool bReplaceAll = false);
    virtual void handleSelectionChange();
    virtual void            LazyReactOnObjectChanges();
    virtual void            SelectAll();
    virtual void            DoCut(::Window* pWindow=NULL);
    virtual void            DoCopy(::Window* pWindow=NULL);
    virtual void            DoPaste(::Window* pWindow=NULL);
    virtual void            DoConnect(SdrOle2Obj* pOleObj);
    virtual bool            SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr = false);
    virtual void            StartDrag( const Point& rStartPos, ::Window* pWindow );
    virtual void            DragFinished( sal_Int8 nDropAction );
    virtual sal_Int8 AcceptDrop (
        const AcceptDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        sal_uInt32 nPage = SDRPAGE_NOTFOUND,
        SdrLayerID aLayer = SDRLAYER_NOTFOUND);
    virtual sal_Int8 ExecuteDrop (
        const ExecuteDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        sal_uInt32 nPage = SDRPAGE_NOTFOUND,
        SdrLayerID aLayer = SDRLAYER_NOTFOUND);

    ::com::sun::star::uno::Reference<
        ::com::sun::star::datatransfer::XTransferable>
        CreateClipboardDataObject (::sd::View*, ::Window& rWindow);
    ::com::sun::star::uno::Reference<
        ::com::sun::star::datatransfer::XTransferable>
        CreateDragDataObject (::sd::View*, ::Window& rWindow,
            const Point& rDragPos);
    ::com::sun::star::uno::Reference<
        ::com::sun::star::datatransfer::XTransferable>
        CreateSelectionDataObject (::sd::View*, ::Window& rWindow);

    void                    UpdateSelectionClipboard( bool bForceDeselect );

    inline DrawDocShell* GetDocSh (void) const;
    inline SdDrawDocument* GetDoc (void) const;
    inline ViewShell* GetViewShell (void) const;

    virtual bool SdrBeginTextEdit(SdrObject* pObj, ::Window* pWin = 0, bool bIsNewObj = false,
        SdrOutliner* pGivenOutliner = 0, OutlinerView* pGivenOutlinerView = 0,
        bool bDontDeleteOutliner = false, bool bOnlyOneView = false, bool bGrabFocus = true);

    virtual SdrEndTextEditKind SdrEndTextEdit(bool bDontDeleteReally = false);

    bool RestoreDefaultText( SdrTextObj* pTextObj );

    bool                    InsertData( const TransferableDataHelper& rDataHelper,
                                        const basegfx::B2DPoint& rPos, sal_Int8& rDnDAction, bool bDrag,
                                        sal_uInt32 nFormat = 0, sal_uInt32 nPage = SDRPAGE_NOTFOUND, sal_uInt32 nLayer = SDRLAYER_NOTFOUND );
    /** gets the metafile from the given transferable helper and insert it as a graphic shape.
        @param bOptimize if set to true, the metafile is analyzed and if only one bitmap action is
                         present, then is is inserted as a single graphic.
    */
    bool InsertMetaFile( TransferableDataHelper& rDataHelper, const basegfx::B2DPoint& rInsertPos, ImageMap* pImageMap, bool bOptimize );

    SdrGrafObj*             InsertGraphic( const Graphic& rGraphic,
                                           sal_Int8& rAction, const basegfx::B2DPoint& rPos,
                                           SdrObject* pSelectedObj, ImageMap* pImageMap );
    SdrMediaObj*            InsertMediaURL( const rtl::OUString& rMediaURL, sal_Int8& rAction,
                                            const basegfx::B2DPoint& rPos, const basegfx::B2DVector& rSize );

    bool PasteRTFTable( SotStorageStreamRef xStm, SdrPage* pPage, sal_uInt32 nPasteOptions );

    bool                    IsPresObjSelected(bool bOnPage=true, bool bOnMasterPage=true, bool bCheckPresObjListOnly=false, bool bCheckLayoutOnly=false) const;

    void                    SetMarkedOriginalSize();

    void                    LockRedraw(bool bLock);

    bool                    IsMorphingAllowed() const;
    bool                    IsVectorizeAllowed() const;

    virtual SfxStyleSheet*  GetStyleSheet() const;

    bool                    GetExchangeList( List*& rpExchangeList, List* pBookmarkList, sal_uInt16 nType );

    virtual void onAccessibilityOptionsChanged();

    virtual SdrModel*   GetMarkedObjModel() const;
    virtual bool Paste(const SdrModel& rMod, const basegfx::B2DPoint& rPos, SdrObjList* pLst = 0, sal_uInt32 nOptions = 0);
    using SdrExchangeView::Paste;

    /** returns true if we have an undo manager and there is an open list undo action */
    bool isRecordingUndo() const;

    virtual void AddCustomHdl();

    SmartTagSet& getSmartTags() { return maSmartTags; }
    void selectSmartTag( const SmartTagReference& xTag );

    virtual SdrViewContext GetContext() const;
    virtual bool HasMarkablePoints() const;
    virtual sal_uInt32 GetMarkablePointCount() const;
    virtual bool HasMarkedPoints() const;
    virtual sal_uInt32 GetMarkedPointCount() const;
    virtual bool IsPointMarkable(const SdrHdl& rHdl) const;
    virtual bool MarkPoint(SdrHdl& rHdl, bool bUnmark = false);
    virtual void CheckPossibilities();
    virtual void MarkPoints(const basegfx::B2DRange* pRange, bool bUnmark);
    using SdrMarkView::MarkPoints;

    void SetPossibilitiesDirty() { if(mbPossibilitiesDirty != true) mbPossibilitiesDirty = true; }
    void SetMoveAllowedOnSelection( bool bSet ) { mbMoveAllowedOnSelection = bSet; }
    void SetMoveProtected( bool bSet ) { if(mbMoveProtect != bSet) mbMoveProtect = bSet; }
    void SetResizeFreeAllowed( bool bSet ) { if(mbResizeFreeAllowed != bSet) mbResizeFreeAllowed = bSet; }
    void SetResizePropAllowed( bool bSet ) { if(mbResizePropAllowed != bSet) mbResizePropAllowed = bSet; }
    void SetResizeProtected( bool bSet ) { if(mbResizeProtect != bSet) mbResizeProtect = bSet; }

    SdrObject* GetEmptyPresentationObject( PresObjKind eKind );
protected:
    DECL_LINK( OnParagraphInsertedHdl, ::Outliner * );
    DECL_LINK( OnParagraphRemovingHdl, ::Outliner * );

    virtual void OnBeginPasteOrDrop( PasteOrDropInfos* pInfos );
    virtual void OnEndPasteOrDrop( PasteOrDropInfos* pInfos );

    SdDrawDocument*         mpDoc;
    DrawDocShell*           mpDocSh;
    ViewShell*              mpViewSh;
    SdrObjectVector maDragSrcMarkList;
    SdrObject*              mpDropMarkerObj;
    SdrDropMarkerOverlay*   mpDropMarker;
    sal_uInt32              mnDragSrcPgNum;
    basegfx::B2DPoint       maDropPos;
    ::std::vector< String > maDropFileVector;
    sal_Int8                mnAction;
    Timer                   maDropErrorTimer;
    Timer                   maDropInsertFileTimer;
    sal_uInt16                  mnLockRedrawSmph;
    List*                   mpLockedRedraws;
    bool                    mbIsDropAllowed;

                            DECL_LINK( DropErrorHdl, Timer* );
                            DECL_LINK( DropInsertFileHdl, Timer* );
                            DECL_LINK( ExecuteNavigatorDrop, SdNavigatorDropEvent* pSdNavigatorDropEvent );

    void ImplClearDrawDropMarker();

    SmartTagSet             maSmartTags;

private:
    ::std::auto_ptr<ViewClipboard> mpClipboard;
};


DrawDocShell* View::GetDocSh (void) const
{
    return mpDocSh;
}
SdDrawDocument* View::GetDoc (void) const
{
    return mpDoc;
}

ViewShell* View::GetViewShell (void) const
{
    return mpViewSh;
}

} // end of namespace sd

#endif
