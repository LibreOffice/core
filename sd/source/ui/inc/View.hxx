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

#ifndef SD_VIEW_HXX
#define SD_VIEW_HXX

#include "pres.hxx"
#include <tools/gen.hxx>
#include <svtools/transfer.hxx>
#include <svx/fmview.hxx>
#include <svx/svdmark.hxx>
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
    TYPEINFO();

    View (
        SdDrawDocument* pDrawDoc,
        OutputDevice* pOutDev,
        ViewShell* pViewSh=NULL);
    virtual ~View (void);

    void                    CompleteRedraw( OutputDevice* pOutDev, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0L);

    virtual BOOL            GetAttributes( SfxItemSet& rTargetSet, BOOL bOnlyHardAttr=FALSE ) const;
    virtual BOOL            SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll = FALSE);
    virtual void            MarkListHasChanged();
    virtual void            ModelHasChanged();
    virtual void            SelectAll();
    virtual void            DoCut(::Window* pWindow=NULL);
    virtual void            DoCopy(::Window* pWindow=NULL);
    virtual void            DoPaste(::Window* pWindow=NULL);
    virtual void            DoConnect(SdrOle2Obj* pOleObj);
    virtual BOOL            SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr = FALSE);
    virtual void            StartDrag( const Point& rStartPos, ::Window* pWindow );
    virtual void            DragFinished( sal_Int8 nDropAction );
    virtual sal_Int8 AcceptDrop (
        const AcceptDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        USHORT nPage = SDRPAGE_NOTFOUND,
        USHORT nLayer = SDRPAGE_NOTFOUND);
    virtual sal_Int8 ExecuteDrop (
        const ExecuteDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        USHORT nPage = SDRPAGE_NOTFOUND,
        USHORT nLayer = SDRPAGE_NOTFOUND);

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

    void                    UpdateSelectionClipboard( BOOL bForceDeselect );

    inline DrawDocShell* GetDocSh (void) const;
    inline SdDrawDocument* GetDoc (void) const;
    inline ViewShell* GetViewShell (void) const;

    virtual sal_Bool SdrBeginTextEdit(SdrObject* pObj, SdrPageView* pPV = 0L, ::Window* pWin = 0L, sal_Bool bIsNewObj = sal_False,
        SdrOutliner* pGivenOutliner = 0L, OutlinerView* pGivenOutlinerView = 0L,
        sal_Bool bDontDeleteOutliner = sal_False, sal_Bool bOnlyOneView = sal_False, sal_Bool bGrabFocus = sal_True);

    virtual SdrEndTextEditKind SdrEndTextEdit(sal_Bool bDontDeleteReally = sal_False);

    bool RestoreDefaultText( SdrTextObj* pTextObj );

    BOOL                    InsertData( const TransferableDataHelper& rDataHelper,
                                        const Point& rPos, sal_Int8& rDnDAction, BOOL bDrag,
                                        ULONG nFormat = 0, USHORT nPage = SDRPAGE_NOTFOUND, USHORT nLayer = SDRLAYER_NOTFOUND );
    /** gets the metafile from the given transferable helper and insert it as a graphic shape.
        @param bOptimize if set to true, the metafile is analyzed and if only one bitmap action is
                         present, then is is inserted as a single graphic.
    */
    bool                    InsertMetaFile( TransferableDataHelper& rDataHelper,
                                            const Point& rInsertPos,
                                            ImageMap* pImageMap, bool bOptimize );

    SdrGrafObj*             InsertGraphic( const Graphic& rGraphic,
                                           sal_Int8& rAction, const Point& rPos,
                                           SdrObject* pSelectedObj, ImageMap* pImageMap );
    SdrMediaObj*            InsertMediaURL( const rtl::OUString& rMediaURL, sal_Int8& rAction,
                                            const Point& rPos, const Size& rSize );

    bool PasteRTFTable( SotStorageStreamRef xStm, SdrPage* pPage, ULONG nPasteOptions );

    BOOL                    IsPresObjSelected(BOOL bOnPage=TRUE, BOOL bOnMasterPage=TRUE, BOOL bCheckPresObjListOnly=FALSE, BOOL bCheckLayoutOnly=FALSE) const;

    void                    SetMarkedOriginalSize();

    void                    LockRedraw(BOOL bLock);

    BOOL                    IsMorphingAllowed() const;
    BOOL                    IsVectorizeAllowed() const;

    virtual SfxStyleSheet*  GetStyleSheet() const;

    BOOL                    GetExchangeList( List*& rpExchangeList, List* pBookmarkList, USHORT nType );

    virtual void onAccessibilityOptionsChanged();

    virtual SdrModel*   GetMarkedObjModel() const;
    virtual BOOL        Paste(const SdrModel& rMod, const Point& rPos, SdrObjList* pLst=NULL, UINT32 nOptions=0);
    using SdrExchangeView::Paste;

    /** returns true if we have an undo manager and there is an open list undo action */
    bool isRecordingUndo() const;

    virtual void AddCustomHdl();

    SmartTagSet& getSmartTags() { return maSmartTags; }
    void selectSmartTag( const SmartTagReference& xTag );
    void updateHandles();

    virtual SdrViewContext GetContext() const;
    virtual BOOL HasMarkablePoints() const;
    virtual ULONG GetMarkablePointCount() const;
    virtual BOOL HasMarkedPoints() const;
    virtual ULONG GetMarkedPointCount() const;
    virtual BOOL IsPointMarkable(const SdrHdl& rHdl) const;
    virtual BOOL MarkPoint(SdrHdl& rHdl, BOOL bUnmark=FALSE);
    virtual void CheckPossibilities();
    virtual BOOL MarkPoints(const ::Rectangle* pRect, BOOL bUnmark);
    using SdrMarkView::MarkPoints;

    void SetPossibilitiesDirty() { bPossibilitiesDirty = true; }
    void SetMoveAllowed( bool bSet ) { bMoveAllowed = bSet; }
    void SetMoveProtected( bool bSet ) { bMoveProtect = bSet; }
    void SetResizeFreeAllowed( bool bSet ) { bResizeFreeAllowed = bSet; }
    void SetResizePropAllowed( bool bSet ) { bResizePropAllowed = bSet; }
    void SetResizeProtected( bool bSet ) { bResizeProtect = bSet; }

    void SetMarkedPointsSmoothPossible( bool bSet ) { bSetMarkedPointsSmoothPossible = bSet; }
    void SetMarkedSegmentsKindPossible( bool bSet ) { bSetMarkedSegmentsKindPossible = bSet; }

    SdrObject* GetEmptyPresentationObject( PresObjKind eKind );
protected:
    DECL_LINK( OnParagraphInsertedHdl, ::Outliner * );
    DECL_LINK( OnParagraphRemovingHdl, ::Outliner * );

    virtual void OnBeginPasteOrDrop( PasteOrDropInfos* pInfos );
    virtual void OnEndPasteOrDrop( PasteOrDropInfos* pInfos );

    SdDrawDocument*         mpDoc;
    DrawDocShell*           mpDocSh;
    ViewShell*              mpViewSh;
    SdrMarkList*            mpDragSrcMarkList;
    SdrObject*              mpDropMarkerObj;
    SdrDropMarkerOverlay*   mpDropMarker;
    USHORT                  mnDragSrcPgNum;
    Point                   maDropPos;
    ::std::vector< String > maDropFileVector;
    sal_Int8                mnAction;
    Timer                   maDropErrorTimer;
    Timer                   maDropInsertFileTimer;
    USHORT                  mnLockRedrawSmph;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
