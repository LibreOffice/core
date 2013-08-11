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

#ifndef SD_VIEW_HXX
#define SD_VIEW_HXX

#include <boost/ptr_container/ptr_vector.hpp>

#include "pres.hxx"
#include <tools/gen.hxx>
#include <svtools/transfer.hxx>
#include <svx/fmview.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdpage.hxx>
#include "fupoor.hxx"

#include "smarttag.hxx"
#include <editeng/numitem.hxx>

class SdDrawDocument;
class SdPage;
class SdrOle2Obj;
class SdrGrafObj;
class SdrMediaObj;
class OutputDevice;
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
        SdDrawDocument& rDrawDoc,
        OutputDevice* pOutDev,
        ViewShell* pViewSh=NULL);
    virtual ~View (void);

    void                    CompleteRedraw( OutputDevice* pOutDev, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0L);

    virtual sal_Bool            GetAttributes( SfxItemSet& rTargetSet, sal_Bool bOnlyHardAttr=sal_False ) const;
    virtual sal_Bool            SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll = sal_False);
    virtual void            MarkListHasChanged();
    virtual void            ModelHasChanged();
    virtual void            SelectAll();
    virtual void            DoCut(::Window* pWindow=NULL);
    virtual void            DoCopy(::Window* pWindow=NULL);
    virtual void            DoPaste(::Window* pWindow=NULL);
    virtual void            DoConnect(SdrOle2Obj* pOleObj);
    virtual sal_Bool            SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr = sal_False);
    virtual void            StartDrag( const Point& rStartPos, ::Window* pWindow );
    virtual void            DragFinished( sal_Int8 nDropAction );
    virtual sal_Int8 AcceptDrop (
        const AcceptDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        sal_uInt16 nPage = SDRPAGE_NOTFOUND,
        sal_uInt16 nLayer = SDRPAGE_NOTFOUND);
    virtual sal_Int8 ExecuteDrop (
        const ExecuteDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        sal_uInt16 nPage = SDRPAGE_NOTFOUND,
        sal_uInt16 nLayer = SDRPAGE_NOTFOUND);

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

    void                    UpdateSelectionClipboard( sal_Bool bForceDeselect );

    inline DrawDocShell* GetDocSh (void) const;
    inline SdDrawDocument& GetDoc (void) const;
    inline ViewShell* GetViewShell (void) const;

    virtual sal_Bool SdrBeginTextEdit(SdrObject* pObj, SdrPageView* pPV = 0L, ::Window* pWin = 0L, sal_Bool bIsNewObj = sal_False,
        SdrOutliner* pGivenOutliner = 0L, OutlinerView* pGivenOutlinerView = 0L,
        sal_Bool bDontDeleteOutliner = sal_False, sal_Bool bOnlyOneView = sal_False, sal_Bool bGrabFocus = sal_True);

    virtual SdrEndTextEditKind SdrEndTextEdit(sal_Bool bDontDeleteReally = sal_False);

    bool RestoreDefaultText( SdrTextObj* pTextObj );

    sal_Bool                    InsertData( const TransferableDataHelper& rDataHelper,
                                        const Point& rPos, sal_Int8& rDnDAction, sal_Bool bDrag,
                                        sal_uLong nFormat = 0, sal_uInt16 nPage = SDRPAGE_NOTFOUND, sal_uInt16 nLayer = SDRLAYER_NOTFOUND );
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
    SdrMediaObj*            InsertMediaURL( const OUString& rMediaURL, sal_Int8& rAction,
                                            const Point& rPos, const Size& rSize,
                                            bool const bLink );

    bool PasteRTFTable( SotStorageStreamRef xStm, SdrPage* pPage, sal_uLong nPasteOptions );

    sal_Bool                    IsPresObjSelected(sal_Bool bOnPage=sal_True, sal_Bool bOnMasterPage=sal_True, sal_Bool bCheckPresObjListOnly=sal_False, sal_Bool bCheckLayoutOnly=sal_False) const;

    void                    SetMarkedOriginalSize();

    sal_Bool                    IsMorphingAllowed() const;
    sal_Bool                    IsVectorizeAllowed() const;

    virtual SfxStyleSheet*  GetStyleSheet() const;

    /** return parameter:
        pExchangeList == NULL -> all names are unique
        bNameOK == sal_False -> cancel by user
        nType == 0 -> pages
        nType == 1 -> objects
        nType == 2 -> pages and objects    */

    bool GetExchangeList( std::vector<OUString> &rExchangeList,
                          std::vector<OUString> &rBookmarkList,
                          const sal_uInt16 nType );

    virtual void onAccessibilityOptionsChanged();

    virtual SdrModel*   GetMarkedObjModel() const;
    virtual sal_Bool        Paste(const SdrModel& rMod, const Point& rPos, SdrObjList* pLst=NULL, sal_uInt32 nOptions=0);
    using SdrExchangeView::Paste;

    /** returns true if we have an undo manager and there is an open list undo action */
    bool isRecordingUndo() const;

    virtual void AddCustomHdl();

    SmartTagSet& getSmartTags() { return maSmartTags; }
    void selectSmartTag( const SmartTagReference& xTag );
    void updateHandles();

    virtual SdrViewContext GetContext() const;
    virtual sal_Bool HasMarkablePoints() const;
    virtual sal_uLong GetMarkablePointCount() const;
    virtual sal_Bool HasMarkedPoints() const;
    virtual sal_uLong GetMarkedPointCount() const;
    virtual sal_Bool IsPointMarkable(const SdrHdl& rHdl) const;
    virtual sal_Bool MarkPoint(SdrHdl& rHdl, sal_Bool bUnmark=sal_False);
    virtual void CheckPossibilities();
    virtual sal_Bool MarkPoints(const ::Rectangle* pRect, sal_Bool bUnmark);
    using SdrMarkView::MarkPoints;

    bool ShouldToggleOn(
        const bool bBulletOnOffMode,
        const bool bNormalBullet);

    /** change the bullets/numbering of the marked objects

        @param bToggle
        true: just toggle the current bullets/numbering on --> off resp. off --> on

        @param bHandleBullets
        true: handle bullets
        false: handle numbering

        @param pNumRule
        numbering rule which needs to be applied. can be 0.

        @param bSwitchOff
        true: switch off bullets/numbering
    */
    void ChangeMarkedObjectsBulletsNumbering(
        const bool bToggle,
        const bool bHandleBullets,
        const SvxNumRule* pNumRule,
        const bool bSwitchOff);

    void SetPossibilitiesDirty() { bPossibilitiesDirty = true; }
    void SetMoveAllowed( bool bSet ) { bMoveAllowed = bSet; }
    void SetMoveProtected( bool bSet ) { bMoveProtect = bSet; }
    void SetResizeFreeAllowed( bool bSet ) { bResizeFreeAllowed = bSet; }
    void SetResizePropAllowed( bool bSet ) { bResizePropAllowed = bSet; }
    void SetResizeProtected( bool bSet ) { bResizeProtect = bSet; }

    void SetMarkedPointsSmoothPossible( bool bSet ) { bSetMarkedPointsSmoothPossible = bSet; }
    void SetMarkedSegmentsKindPossible( bool bSet ) { bSetMarkedSegmentsKindPossible = bSet; }

    SdrObject* GetEmptyPresentationObject( PresObjKind eKind );
    SdPage* GetPage();
    SdrObject* GetSelectedSingleObject(SdPage* pPage);

protected:
    DECL_LINK( OnParagraphInsertedHdl, ::Outliner * );
    DECL_LINK( OnParagraphRemovingHdl, ::Outliner * );

    virtual void OnBeginPasteOrDrop( PasteOrDropInfos* pInfos );
    virtual void OnEndPasteOrDrop( PasteOrDropInfos* pInfos );

    SdDrawDocument&         mrDoc;
    DrawDocShell*           mpDocSh;
    ViewShell*              mpViewSh;
    SdrMarkList*            mpDragSrcMarkList;
    SdrObject*              mpDropMarkerObj;
    SdrDropMarkerOverlay*   mpDropMarker;
    sal_uInt16                  mnDragSrcPgNum;
    Point                   maDropPos;
    ::std::vector<OUString> maDropFileVector;
    sal_Int8                mnAction;
    Timer                   maDropErrorTimer;
    Timer                   maDropInsertFileTimer;
    sal_uInt16                  mnLockRedrawSmph;
    boost::ptr_vector<SdViewRedrawRec> maLockedRedraws;
    bool                    mbIsDropAllowed;

                            DECL_LINK( DropErrorHdl, void* );
                            DECL_LINK( DropInsertFileHdl, void* );
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
SdDrawDocument& View::GetDoc (void) const
{
    return mrDoc;
}

ViewShell* View::GetViewShell (void) const
{
    return mpViewSh;
}

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
