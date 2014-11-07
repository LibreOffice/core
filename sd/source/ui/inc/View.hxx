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

#ifndef INCLUDED_SD_SOURCE_UI_INC_VIEW_HXX
#define INCLUDED_SD_SOURCE_UI_INC_VIEW_HXX

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

// SdViewRedrawRec
struct SdViewRedrawRec
{
    OutputDevice* mpOut;
    Rectangle     aRect;
};

//For master view we want to force that master
//textboxes have readonly text, because the
//text is the auto-generated click-here-to-edit
//and it doesn't help to change it
class OutlinerMasterViewFilter
{
private:
    SdrOutliner *m_pOutl;
    bool m_bReadOnly;
public:
    OutlinerMasterViewFilter()
        : m_pOutl(0)
        , m_bReadOnly(false)
    {
    }
    void Start(SdrOutliner *pOutl);
    void End();
};

class View : public FmFormView
{
public:
    TYPEINFO_OVERRIDE();

    View (
        SdDrawDocument& rDrawDoc,
        OutputDevice* pOutDev,
        ViewShell* pViewSh=NULL);
    virtual ~View (void);

    void                    CompleteRedraw( OutputDevice* pOutDev, const ::vcl::Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0L) SAL_OVERRIDE;

    virtual bool            GetAttributes( SfxItemSet& rTargetSet, bool bOnlyHardAttr = false ) const;
    virtual bool            SetAttributes(const SfxItemSet& rSet, bool bReplaceAll = false);
    virtual void            MarkListHasChanged() SAL_OVERRIDE;
    virtual void            ModelHasChanged() SAL_OVERRIDE;
    virtual void            SelectAll();
    virtual void            DoCut(vcl::Window* pWindow=NULL);
    virtual void            DoCopy(vcl::Window* pWindow=NULL);
    virtual void            DoPaste(vcl::Window* pWindow=NULL);
    virtual void            DoConnect(SdrOle2Obj* pOleObj) SAL_OVERRIDE;
    virtual bool            SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr = false);
    virtual void            StartDrag( const Point& rStartPos, vcl::Window* pWindow );
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
        CreateClipboardDataObject (::sd::View*, vcl::Window& rWindow);
    ::com::sun::star::uno::Reference<
        ::com::sun::star::datatransfer::XTransferable>
        CreateDragDataObject (::sd::View*, vcl::Window& rWindow,
            const Point& rDragPos);
    ::com::sun::star::uno::Reference<
        ::com::sun::star::datatransfer::XTransferable>
        CreateSelectionDataObject (::sd::View*, vcl::Window& rWindow);

    void                    UpdateSelectionClipboard( bool bForceDeselect );

    inline DrawDocShell* GetDocSh(void) const { return mpDocSh; }
    inline SdDrawDocument& GetDoc(void) const;
    inline ViewShell* GetViewShell(void) const { return mpViewSh; }

    virtual bool SdrBeginTextEdit(SdrObject* pObj, SdrPageView* pPV = 0L, ::vcl::Window* pWin = 0L, bool bIsNewObj = false,
        SdrOutliner* pGivenOutliner = 0L, OutlinerView* pGivenOutlinerView = 0L,
        bool bDontDeleteOutliner = false, bool bOnlyOneView = false, bool bGrabFocus = true) SAL_OVERRIDE;

    virtual SdrEndTextEditKind SdrEndTextEdit(bool bDontDeleteReally = false) SAL_OVERRIDE;

    bool RestoreDefaultText( SdrTextObj* pTextObj );

    bool                    InsertData( const TransferableDataHelper& rDataHelper,
                                        const Point& rPos, sal_Int8& rDnDAction, bool bDrag,
                                        sal_uLong nFormat = 0, sal_uInt16 nPage = SDRPAGE_NOTFOUND, sal_uInt16 nLayer = SDRLAYER_NOTFOUND );
    /** gets the metafile from the given transferable helper and insert it as a graphic shape.
        @param bOptimize if set to true, the metafile is analyzed and if only one bitmap action is
                         present, then is inserted as a single graphic.
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

    SdrMediaObj*            Insert3DModelURL( const OUString& rModelURL, sal_Int8& rAction,
                                              const Point& rPos, const Size& rSize,
                                              bool const bLink );

    SdrMediaObj*            InsertMediaObj( const OUString& rURL, const OUString& rMimeType, sal_Int8& rAction,
                                            const Point& rPos, const Size& rSize );

    bool PasteRTFTable( SotStorageStreamRef xStm, SdrPage* pPage, sal_uLong nPasteOptions );

    bool                    IsPresObjSelected(bool bOnPage = true, bool bOnMasterPage = true, bool bCheckPresObjListOnly = false, bool bCheckLayoutOnly = false) const;

    void                    SetMarkedOriginalSize();

    bool                    IsMorphingAllowed() const;
    bool                    IsVectorizeAllowed() const;

    virtual SfxStyleSheet*  GetStyleSheet() const;

    /** return parameter:
        pExchangeList == NULL -> all names are unique
        bNameOK == false -> cancel by user
        nType == 0 -> pages
        nType == 1 -> objects
        nType == 2 -> pages and objects    */

    bool GetExchangeList( std::vector<OUString> &rExchangeList,
                          std::vector<OUString> &rBookmarkList,
                          const sal_uInt16 nType );

    virtual void onAccessibilityOptionsChanged() SAL_OVERRIDE;

    virtual SdrModel*   GetMarkedObjModel() const SAL_OVERRIDE;
    virtual bool Paste(
        const SdrModel& rMod, const Point& rPos, SdrObjList* pLst, sal_uInt32 nOptions,
        const OUString& rSrcShellID, const OUString& rDestShellID ) SAL_OVERRIDE;

    using SdrExchangeView::Paste;

    /** returns true if we have an undo manager and there is an open list undo action */
    bool isRecordingUndo() const;

    virtual void AddCustomHdl() SAL_OVERRIDE;

    SmartTagSet& getSmartTags() { return maSmartTags; }
    void selectSmartTag( const SmartTagReference& xTag );
    void updateHandles();

    virtual SdrViewContext GetContext() const SAL_OVERRIDE;
    virtual bool HasMarkablePoints() const SAL_OVERRIDE;
    virtual sal_uLong GetMarkablePointCount() const SAL_OVERRIDE;
    virtual bool HasMarkedPoints() const SAL_OVERRIDE;
    virtual sal_uLong GetMarkedPointCount() const SAL_OVERRIDE;
    virtual bool IsPointMarkable(const SdrHdl& rHdl) const SAL_OVERRIDE;
    virtual bool MarkPoint(SdrHdl& rHdl, bool bUnmark=false) SAL_OVERRIDE;
    virtual void CheckPossibilities() SAL_OVERRIDE;
    virtual bool MarkPoints(const ::Rectangle* pRect, bool bUnmark) SAL_OVERRIDE;
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

    virtual void OnBeginPasteOrDrop( PasteOrDropInfos* pInfos ) SAL_OVERRIDE;
    virtual void OnEndPasteOrDrop( PasteOrDropInfos* pInfos ) SAL_OVERRIDE;

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
    ::std::unique_ptr<ViewClipboard> mpClipboard;
    OutlinerMasterViewFilter maMasterViewFilter;
};

SdDrawDocument& View::GetDoc (void) const
{
    return mrDoc;
}

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
