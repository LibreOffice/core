/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: View.hxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:43:17 $
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

#ifndef SD_VIEW_HXX
#define SD_VIEW_HXX

#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif
#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _TRANSFER_HXX //autogen
#include <svtools/transfer.hxx>
#endif
#ifndef _SVX_FMVIEW_HXX
#include <svx/fmview.hxx>
#endif
#ifndef _SVDMARK_HXX //autogen
#include <svx/svdmark.hxx>
#endif
//#ifndef _SVDVMARK_HXX //autogen
//#include <svx/svdvmark.hxx>
//#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif

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

    void                    CompleteRedraw( OutputDevice* pOutDev, const Region& rReg, USHORT nPaintMode = 0, ::sdr::contact::ViewObjectContactRedirector* pRedirector = 0L);

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

    VirtualDevice*          CreatePageVDev(USHORT nSdPage, PageKind ePageKind, ULONG nWidthPixel);

    void                    LockRedraw(BOOL bLock);
    bool IsRedrawLocked (void) const;

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

protected:
    DECL_LINK( OnParagraphInsertedHdl, ::Outliner * );
    DECL_LINK( OnParagraphRemovingHdl, ::Outliner * );

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
