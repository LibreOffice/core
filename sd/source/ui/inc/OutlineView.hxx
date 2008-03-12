/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OutlineView.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:42:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_OUTLINE_VIEW_HXX
#define SD_OUTLINE_VIEW_HXX

#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif


class SdPage;
class SdrPage;
class EditStatus;
class Paragraph;
class SdrTextObj;
class Outliner;
class SfxProgress;

namespace sd { namespace tools {
class EventMultiplexerEvent;
} }

namespace sd {

class DrawDocShell;
class OutlineViewShell;
class OutlineViewModelChangeGuard;
class DrawDocShell;

static const int MAX_OUTLINERVIEWS = 4;

/*************************************************************************
|*
|* Ableitung von ::sd::View fuer den Outline-Modus
|*
\************************************************************************/

class OutlineView
    : public ::sd::View
{
    friend class OutlineViewModelChangeGuard;
public:
    OutlineView (DrawDocShell* pDocSh,
        ::Window* pWindow,
        OutlineViewShell* pOutlineViewSh);
    ~OutlineView (void);

    /** This method is called by the view shell that owns the view to tell
        the view that it can safely connect to the application.
        This method must not be called before the view shell is on the shell
        stack.
    */
    void ConnectToApplication (void);
    void DisconnectFromApplication (void);

    TYPEINFO();

    SdrTextObj*     GetTitleTextObject(SdrPage* pPage);
    SdrTextObj*     GetOutlineTextObject(SdrPage* pPage);

    SdrTextObj*     CreateTitleTextObject(SdPage* pPage);
    SdrTextObj*     CreateOutlineTextObject(SdPage* pPage);

    virtual void AddWindowToPaintView(OutputDevice* pWin);
    virtual void DeleteWindowFromPaintView(OutputDevice* pWin);

    OutlinerView*   GetViewByWindow (::Window* pWin) const;
    SdrOutliner*    GetOutliner() { return(mpOutliner) ; }

    Paragraph*      GetPrevTitle(const Paragraph* pPara);
    Paragraph*      GetNextTitle(const Paragraph* pPara);
    SdPage*         GetActualPage();
    SdPage*         GetPageForParagraph( ::Outliner* pOutl, Paragraph* pPara );
    Paragraph*      GetParagraphForPage( ::Outliner* pOutl, SdPage* pPage );

    /** selects the paragraph for the given page at the outliner view*/
    void            SetActualPage( SdPage* pActual );

    virtual void Paint (const Rectangle& rRect, ::sd::Window* pWin);
    virtual void AdjustPosSizePixel(
        const Point &rPos,
        const Size &rSize,
        ::sd::Window* pWindow);

                    // Callbacks fuer LINKs
    DECL_LINK( ParagraphInsertedHdl, Outliner * );
    DECL_LINK( ParagraphRemovingHdl, Outliner * );
    DECL_LINK( DepthChangedHdl, Outliner * );
    DECL_LINK( StatusEventHdl, EditStatus * );
    DECL_LINK( BeginMovingHdl, Outliner * );
    DECL_LINK( EndMovingHdl, Outliner * );
    DECL_LINK( RemovingPagesHdl, OutlinerView * );
    DECL_LINK( IndentingPagesHdl, OutlinerView * );
    DECL_LINK( BeginDropHdl, void * );
    DECL_LINK( EndDropHdl, void * );

    ULONG         GetPaperWidth() const { return 21000; }  // DIN A4 Breite

    BOOL          PrepareClose(BOOL bUI = TRUE);

    virtual BOOL    GetAttributes( SfxItemSet& rTargetSet, BOOL bOnlyHardAttr=FALSE ) const;
    virtual BOOL    SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll = FALSE);

//  virtual BOOL       HasMarkedObjUnused() const;
    void               FillOutliner();
    void               SetLinks();
    void               ResetLinks() const;

    SfxStyleSheet*     GetStyleSheet() const;

    void               SetSelectedPages();

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


    // #97766# Re-implement GetScriptType for this view to get correct results
    virtual sal_uInt16 GetScriptType() const;

    /** After this method has been called with <TRUE/> following changes of
        the current page are ignored in that the corresponding text is not
        selected.
        This is used to supress unwanted side effects between selection and
        cursor position.
    */
    void IgnoreCurrentPageChanges (bool bIgnore);

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

    /** creates and inserts an empty slide for the given paragraph */
    SdPage* InsertSlideForParagraph( Paragraph* pPara );

    OutlineViewShell*   mpOutlineViewShell;
    SdrOutliner*        mpOutliner;
    OutlinerView*       mpOutlinerView[MAX_OUTLINERVIEWS];

    List*               mpOldParaOrder;
    List*               mpSelectedParas;

    USHORT              mnPagesToProcess;    // fuer die Fortschrittsanzeige
    USHORT              mnPagesProcessed;

    BOOL                mbFirstPaint;

    SfxProgress*        mpProgress;

    /** stores the last used high contrast mode.
        this is changed in onUpdateStyleSettings()
    */
    bool mbHighContrastMode;

    /** stores the last used document color.
        this is changed in onUpdateStyleSettings()
    */
    Color maDocColor;

    /** updates the high contrast settings and document color if they changed.
        @param bForceUpdate forces the method to set all style settings
    */
    void onUpdateStyleSettings( bool bForceUpdate = false );

    /** this link is called from the vcl applicaten when the stylesettings
        change. Its only purpose is to call onUpdateStyleSettings() then.
    */
    DECL_LINK( AppEventListenerHdl, void * );

    DECL_LINK(EventMultiplexerListener, sd::tools::EventMultiplexerEvent*);

    /** holds a model guard during drag and drop between BeginMovingHdl and EndMovingHdl */
    std::auto_ptr< OutlineViewModelChangeGuard > maDragAndDropModelGuard;
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
