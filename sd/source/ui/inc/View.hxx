/*************************************************************************
 *
 *  $RCSfile: View.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:11:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#ifndef _SVDVMARK_HXX //autogen
#include <svx/svdvmark.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif

class SdDrawDocument;
class SdrOle2Obj;
class SdrGrafObj;
class OutputDevice;
class VirtualDevice;
class ImageMap;
class Point;
class Graphic;
class SdrOutliner;
class TransferableDataHelper;
struct StyleRequestData;

namespace sd {

class DrawDocShell;
struct SdNavigatorDropEvent;
class ViewShell;
class Window;

// -------------------
// - SdViewRedrawRec -
// -------------------

struct SdViewRedrawRec
{
    OutputDevice* pOut;
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

    void                    CompleteRedraw( OutputDevice* pOutDev, const Region& rReg, ::sdr::contact::ViewObjectContactRedirector* pRedirector = 0L);

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
    virtual void            StartDrag( const Point& rStartPos, Window* pWindow );
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

    DrawDocShell*         GetDocSh() const { return pDocSh; }
    SdDrawDocument*         GetDoc() const { return pDoc; }

    BOOL                    BegTextEdit( SdrObject* pObj, SdrPageView* pPV=NULL, Window* pWin=NULL, BOOL bIsNewObj=FALSE,
                                         SdrOutliner* pGivenOutliner=NULL, OutlinerView* pGivenOutlinerView=NULL,
                                         BOOL bDontDeleteOutliner=FALSE, BOOL bOnlyOneView=FALSE );
    SdrEndTextEditKind      EndTextEdit(BOOL bDontDeleteReally=FALSE);

    BOOL                    InsertData( const TransferableDataHelper& rDataHelper,
                                        const Point& rPos, sal_Int8& rDnDAction, BOOL bDrag,
                                        ULONG nFormat = 0, USHORT nPage = SDRPAGE_NOTFOUND, USHORT nLayer = SDRLAYER_NOTFOUND );
    SdrGrafObj*             InsertGraphic( const Graphic& rGraphic,
                                           sal_Int8& rAction, const Point& rPos,
                                           SdrObject* pSelectedObj, ImageMap* pImageMap );

    BOOL                    IsPresObjSelected(BOOL bOnPage=TRUE, BOOL bOnMasterPage=TRUE, BOOL bCheckPresObjListOnly=FALSE, BOOL bCheckLayoutOnly=FALSE) const;

    void                    SetMarkedOriginalSize();

    VirtualDevice*          CreatePageVDev(USHORT nSdPage, PageKind ePageKind, ULONG nWidthPixel);

    void                    LockRedraw(BOOL bLock);

    BOOL                    IsMorphingAllowed() const;
    BOOL                    IsVectorizeAllowed() const;

    virtual SfxStyleSheet*  GetStyleSheet() const;

    BOOL                    GetExchangeList( List*& rpExchangeList, List* pBookmarkList, USHORT nType );

    virtual void onAccessibilityOptionsChanged();

    virtual SdrModel*   GetMarkedObjModel() const;
    virtual BOOL        Paste(const SdrModel& rMod, const Point& rPos, SdrObjList* pLst=NULL, UINT32 nOptions=0);

protected:
    SdDrawDocument*         pDoc;
    DrawDocShell*       pDocSh;
    ViewShell* pViewSh;
    SdrMarkList*            pDragSrcMarkList;
    SdrObject*              pDropMarkerObj;
    SdrViewUserMarker*      pDropMarker;
    USHORT                  nDragSrcPgNum;
    Point                   aDropPos;
    ::std::vector< String > aDropFileVector;
    sal_Int8                nAction;
    Timer                   aDropErrorTimer;
    Timer                   aDropInsertFileTimer;
    USHORT                  nLockRedrawSmph;
    List*                   pLockedRedraws;
    bool bIsDropAllowed;

                            DECL_LINK( DropErrorHdl, Timer* );
                            DECL_LINK( DropInsertFileHdl, Timer* );
                            DECL_LINK( ExecuteNavigatorDrop, SdNavigatorDropEvent* pSdNavigatorDropEvent );
};

} // end of namespace sd

#endif
