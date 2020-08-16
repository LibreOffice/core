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

#include <memory>
#include <pres.hxx>
#include <tools/gen.hxx>
#include <vcl/transfer.hxx>
#include <svx/fmview.hxx>
#include <svx/svdpage.hxx>
#include <vcl/idle.hxx>
#include <VectorGraphicSearchContext.hxx>

#include "smarttag.hxx"

class SdDrawDocument;
class SdPage;
class SdrOle2Obj;
class SdrGrafObj;
class SdrMediaObj;
class OutputDevice;
class ImageMap;
class Graphic;
class SdrOutliner;

namespace sd {

class DrawDocShell;
class ViewShell;
class Window;
class ViewClipboard;

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
        : m_pOutl(nullptr)
        , m_bReadOnly(false)
    {
    }
    void Start(SdrOutliner *pOutl);
    void End();
};

class SAL_DLLPUBLIC_RTTI View : public FmFormView
{
public:

    View (
        SdDrawDocument& rDrawDoc,
        OutputDevice* pOutDev,
        ViewShell* pViewSh=nullptr);
    virtual ~View() override;

    void                    CompleteRedraw( OutputDevice* pOutDev, const vcl::Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = nullptr) override;

    virtual void            GetAttributes( SfxItemSet& rTargetSet, bool bOnlyHardAttr = false ) const;
    virtual bool            SetAttributes(const SfxItemSet& rSet, bool bReplaceAll = false);
    virtual void            MarkListHasChanged() override;
    void                    SelectAll();
    void                    DoCut();
    void                    DoCopy();
    void                    DoPaste(::sd::Window* pWindow=nullptr);
    virtual void            DoConnect(SdrOle2Obj* pOleObj) override;
    virtual bool            SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr = false);
    void                    StartDrag( const Point& rStartPos, vcl::Window* pWindow );
    virtual void            DragFinished( sal_Int8 nDropAction );
    virtual sal_Int8 AcceptDrop (
        const AcceptDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow,
        sal_uInt16 nPage,
        SdrLayerID nLayer);
    virtual sal_Int8 ExecuteDrop (
        const ExecuteDropEvent& rEvt,
        ::sd::Window* pTargetWindow,
        sal_uInt16 nPage,
        SdrLayerID nLayer);

    css::uno::Reference<css::datatransfer::XTransferable>
        CreateClipboardDataObject ();
    css::uno::Reference<css::datatransfer::XTransferable>
        CreateDragDataObject (::sd::View*, vcl::Window& rWindow,
            const Point& rDragPos);
    css::uno::Reference<css::datatransfer::XTransferable>
        CreateSelectionDataObject (::sd::View*, vcl::Window& rWindow);

    void                    UpdateSelectionClipboard( bool bForceDeselect );

    DrawDocShell* GetDocSh() const { return mpDocSh; }
    inline SdDrawDocument& GetDoc() const;
    ViewShell* GetViewShell() const { return mpViewSh; }
    SfxViewShell* GetSfxViewShell() const override;

    virtual bool SdrBeginTextEdit(SdrObject* pObj, SdrPageView* pPV = nullptr, vcl::Window* pWin = nullptr, bool bIsNewObj = false,
        SdrOutliner* pGivenOutliner = nullptr, OutlinerView* pGivenOutlinerView = nullptr,
        bool bDontDeleteOutliner = false, bool bOnlyOneView = false, bool bGrabFocus = true) override;

    virtual SdrEndTextEditKind SdrEndTextEdit(bool bDontDeleteReally = false) override;

    bool RestoreDefaultText( SdrTextObj* pTextObj );

    bool                    InsertData( const TransferableDataHelper& rDataHelper,
                                        const Point& rPos, sal_Int8& rDnDAction, bool bDrag,
                                        SotClipboardFormatId nFormat = SotClipboardFormatId::NONE,
                                        sal_uInt16 nPage = SDRPAGE_NOTFOUND, SdrLayerID nLayer = SDRLAYER_NOTFOUND );
    /** gets the metafile from the given transferable helper and insert it as a graphic shape.
        @param bOptimize if set to true, the metafile is analyzed and if only one bitmap action is
                         present, then is inserted as a single graphic.
    */
    bool                    InsertMetaFile( TransferableDataHelper& rDataHelper,
                                            const Point& rInsertPos,
                                            ImageMap const * pImageMap, bool bOptimize );
    SdrGrafObj*             InsertGraphic( const Graphic& rGraphic,
                                           sal_Int8& rAction, const Point& rPos,
                                           SdrObject* pSelectedObj, ImageMap const * pImageMap );
    void                    InsertMediaURL( const OUString& rMediaURL, sal_Int8& rAction,
                                            const Point& rPos, const Size& rSize,
                                            bool const bLink );
    SdrMediaObj*            InsertMediaObj( const OUString& rURL, const OUString& rMimeType, sal_Int8& rAction,
                                            const Point& rPos, const Size& rSize );

    bool PasteRTFTable( const ::tools::SvRef<SotStorageStream>& xStm, SdrPage* pPage, SdrInsertFlags nPasteOptions );

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

    virtual void onAccessibilityOptionsChanged() override;

    /** returns true if we have an undo manager and there is an open list undo action */
    bool isRecordingUndo() const;

    virtual void AddCustomHdl() override;

    SmartTagSet& getSmartTags() { return maSmartTags; }
    void updateHandles();

    virtual SdrViewContext GetContext() const override;
    virtual bool HasMarkablePoints() const override;
    virtual sal_Int32 GetMarkablePointCount() const override;
    virtual bool HasMarkedPoints() const override;
    virtual bool IsPointMarkable(const SdrHdl& rHdl) const override;
    virtual bool MarkPoint(SdrHdl& rHdl, bool bUnmark=false) override;
    virtual void CheckPossibilities() override;
    virtual bool MarkPoints(const ::tools::Rectangle* pRect, bool bUnmark) override;
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
    */
    void ChangeMarkedObjectsBulletsNumbering(
        const bool bToggle,
        const bool bHandleBullets,
        const SvxNumRule* pNumRule);

    void SetPossibilitiesDirty() { m_bPossibilitiesDirty = true; }
    void SetMoveAllowed( bool bSet ) { m_bMoveAllowed = bSet; }
    void SetMoveProtected( bool bSet ) { m_bMoveProtect = bSet; }
    void SetResizeFreeAllowed( bool bSet ) { m_bResizeFreeAllowed = bSet; }
    void SetResizePropAllowed( bool bSet ) { m_bResizePropAllowed = bSet; }
    void SetResizeProtected( bool bSet ) { m_bResizeProtect = bSet; }

    SdrObject* GetEmptyPresentationObject( PresObjKind eKind );
    SdPage* GetPage();
    SdrObject* GetSelectedSingleObject(SdPage const * pPage);
    void SetAuthor(const OUString& rAuthor) { m_sAuthor = rAuthor; }
    const OUString& GetAuthor() { return m_sAuthor; }

    VectorGraphicSearchContext& getVectorGraphicSearchContext() { return aVectorGraphicSearchContext; }

protected:
    DECL_LINK( OnParagraphInsertedHdl, ::Outliner::ParagraphHdlParam, void );
    DECL_LINK( OnParagraphRemovingHdl, ::Outliner::ParagraphHdlParam, void );

    virtual void OnBeginPasteOrDrop( PasteOrDropInfos* pInfo ) override;
    virtual void OnEndPasteOrDrop( PasteOrDropInfos* pInfo ) override;

    SdDrawDocument&         mrDoc;
    DrawDocShell*           mpDocSh;
    ViewShell*              mpViewSh;
    std::unique_ptr<SdrMarkList> mpDragSrcMarkList;
    SdrObject*              mpDropMarkerObj;
    std::unique_ptr<SdrDropMarkerOverlay> mpDropMarker;
    sal_uInt16              mnDragSrcPgNum;
    Point                   maDropPos;
    ::std::vector<OUString> maDropFileVector;
    sal_Int8                mnAction;
    Idle                    maDropErrorIdle;
    Idle                    maDropInsertFileIdle;
    sal_uInt16              mnLockRedrawSmph;
    bool                    mbIsDropAllowed;

                            DECL_LINK( DropErrorHdl, Timer*, void );
                            DECL_LINK( DropInsertFileHdl, Timer*, void );
                            DECL_LINK( ExecuteNavigatorDrop, void*, void );

    void ImplClearDrawDropMarker();

    SmartTagSet             maSmartTags;

private:
    ::std::unique_ptr<ViewClipboard> mpClipboard;
    OutlinerMasterViewFilter maMasterViewFilter;

    VectorGraphicSearchContext aVectorGraphicSearchContext;

    OUString m_sAuthor;
};

SdDrawDocument& View::GetDoc() const
{
    return mrDoc;
}

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
