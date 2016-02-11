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

#ifndef INCLUDED_SD_SOURCE_UI_INC_OUTLINEVIEW_HXX
#define INCLUDED_SD_SOURCE_UI_INC_OUTLINEVIEW_HXX

#include <vcl/image.hxx>
#include <editeng/lrspitem.hxx>
#include "View.hxx"

class SdPage;
class SdrPage;
class Paragraph;
class SdrTextObj;
class Outliner;
class SfxProgress;
struct PaintFirstLineInfo;
struct PasteOrDropInfos;
class EditView;

namespace sd { namespace tools {
class EventMultiplexerEvent;
} }

namespace sd {

class DrawDocShell;
class OutlineViewShell;
class OutlineViewModelChangeGuard;
class DrawDocShell;

static const int MAX_OUTLINERVIEWS = 4;

/**
 * Derivative of ::sd::View for the outline mode
|*
\************************************************************************/

class OutlineView
    : public ::sd::View
{
    friend class OutlineViewModelChangeGuard;
public:
    OutlineView (DrawDocShell& rDocSh,
        vcl::Window* pWindow,
        OutlineViewShell& rOutlineViewSh);
    virtual ~OutlineView();

    /** This method is called by the view shell that owns the view to tell
        the view that it can safely connect to the application.
        This method must not be called before the view shell is on the shell
        stack.
    */
    void ConnectToApplication();
    void DisconnectFromApplication();


    static SdrTextObj*     GetTitleTextObject(SdrPage* pPage);
    static SdrTextObj*     GetOutlineTextObject(SdrPage* pPage);

    static SdrTextObj*     CreateTitleTextObject(SdPage* pPage);
    static SdrTextObj*     CreateOutlineTextObject(SdPage* pPage);

    virtual void AddWindowToPaintView(OutputDevice* pWin, vcl::Window* pWindow) override;
    virtual void DeleteWindowFromPaintView(OutputDevice* pWin) override;

    OutlinerView*   GetViewByWindow(vcl::Window* pWin) const;
    SdrOutliner&    GetOutliner() { return mrOutliner; }

    Paragraph*      GetPrevTitle(const Paragraph* pPara);
    Paragraph*      GetNextTitle(const Paragraph* pPara);
    SdPage*         GetActualPage();
    SdPage*         GetPageForParagraph( Paragraph* pPara );
    Paragraph*      GetParagraphForPage( ::Outliner& rOutl, SdPage* pPage );

    /** selects the paragraph for the given page at the outliner view*/
    void            SetActualPage( SdPage* pActual );

    void Paint (const Rectangle& rRect, ::sd::Window* pWin);

                    // Callbacks fuer LINKs
    DECL_LINK_TYPED( ParagraphInsertedHdl, ::Outliner *, void );
    DECL_LINK_TYPED( ParagraphRemovingHdl, ::Outliner *, void );
    DECL_LINK_TYPED( DepthChangedHdl, ::Outliner *, void );
    DECL_LINK_TYPED( StatusEventHdl, EditStatus&, void );
    DECL_LINK_TYPED( BeginMovingHdl, ::Outliner *, void );
    DECL_LINK_TYPED( EndMovingHdl, ::Outliner *, void );
    DECL_LINK_TYPED( RemovingPagesHdl, OutlinerView *, bool );
    DECL_LINK_TYPED( IndentingPagesHdl, OutlinerView *, bool );
    DECL_LINK_TYPED( BeginDropHdl, EditView*, void );
    DECL_LINK_TYPED( EndDropHdl, EditView*, void );
    DECL_LINK_TYPED( PaintingFirstLineHdl, PaintFirstLineInfo*, void );

    sal_uLong         GetPaperWidth() { return mnPaperWidth;}

    bool          PrepareClose(bool bUI = true);

    virtual bool    GetAttributes( SfxItemSet& rTargetSet, bool bOnlyHardAttr = false ) const override;
    virtual bool    SetAttributes(const SfxItemSet& rSet, bool bReplaceAll = false) override;

    void               FillOutliner();
    void               SetLinks();
    void               ResetLinks() const;

    SfxStyleSheet*     GetStyleSheet() const override;

    void               SetSelectedPages();

    virtual sal_Int8 AcceptDrop (
        const AcceptDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = nullptr,
        sal_uInt16 nPage = SDRPAGE_NOTFOUND,
        sal_uInt16 nLayer = SDRPAGE_NOTFOUND) override;
    virtual sal_Int8 ExecuteDrop (
        const ExecuteDropEvent& rEvt,
        ::sd::Window* pTargetWindow = nullptr,
        sal_uInt16 nPage = SDRPAGE_NOTFOUND,
        sal_uInt16 nLayer = SDRPAGE_NOTFOUND) override;

    // Re-implement GetScriptType for this view to get correct results
    virtual SvtScriptType GetScriptType() const override;

    /** After this method has been called with <TRUE/> following changes of
        the current page are ignored in that the corresponding text is not
        selected.
        This is used to suppress unwanted side effects between selection and
        cursor position.
    */
    void IgnoreCurrentPageChanges (bool bIgnore);

    /** creates and inserts an empty slide for the given paragraph. */
    SdPage* InsertSlideForParagraph( Paragraph* pPara );

    void UpdateParagraph( sal_Int32 nPara );

protected:
    virtual void OnBeginPasteOrDrop( PasteOrDropInfos* pInfo ) override;
    virtual void OnEndPasteOrDrop( PasteOrDropInfos* pInfo ) override;

private:
    /** call this method before you do anything that can modify the outliner
        and or the drawing document model. It will create needed undo actions */
    void BeginModelChange();

    /** call this method after BeginModelChange(), when all possible model
        changes are done. */
    void EndModelChange();

    /** merge edit engine undo actions if possible */
    void TryToMergeUndoActions();

    /** updates all changes in the outliner model to the draw model */
    void UpdateDocument();

    OutlineViewShell&   mrOutlineViewShell;
    SdrOutliner&        mrOutliner;
    OutlinerView*       mpOutlinerView[MAX_OUTLINERVIEWS];

    std::vector<Paragraph*> maOldParaOrder;
    std::vector<Paragraph*> maSelectedParas;

    sal_Int32               mnPagesToProcess;    // for the progress bar
    sal_Int32               mnPagesProcessed;

    bool                mbFirstPaint;

    sal_uLong               mnPaperWidth;

    SfxProgress*        mpProgress;

    /** stores the last used document color.
        this is changed in onUpdateStyleSettings()
    */
    Color maDocColor;

    /** updates the high contrast settings and document color if they changed.
        @param bForceUpdate forces the method to set all style settings
    */
    void onUpdateStyleSettings( bool bForceUpdate = false );

    /** this link is called from the vcl application when the stylesettings
        change. Its only purpose is to call onUpdateStyleSettings() then.
    */
    DECL_LINK_TYPED( AppEventListenerHdl, VclSimpleEvent&, void );

    DECL_LINK_TYPED(EventMultiplexerListener, sd::tools::EventMultiplexerEvent&, void);

    /** holds a model guard during drag and drop between BeginMovingHdl and EndMovingHdl */
    std::unique_ptr< OutlineViewModelChangeGuard > maDragAndDropModelGuard;

    vcl::Font maPageNumberFont;
    vcl::Font maBulletFont;

    SvxLRSpaceItem maLRSpaceItem;
    Image maSlideImage;
};

// calls IgnoreCurrentPageChangesLevel with true in ctor and with false in dtor
class OutlineViewPageChangesGuard
{
public:
    OutlineViewPageChangesGuard( OutlineView* pView );
    ~OutlineViewPageChangesGuard();
private:
    OutlineView* mpView;
};

// calls BeginModelChange() on c'tor and EndModelChange() on d'tor
class OutlineViewModelChangeGuard
{
public:
    OutlineViewModelChangeGuard( OutlineView& rView );
    ~OutlineViewModelChangeGuard();
private:
    OutlineView& mrView;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
