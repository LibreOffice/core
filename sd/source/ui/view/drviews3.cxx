/*************************************************************************
 *
 *  $RCSfile: drviews3.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tbe $ $Date: 2000-10-23 10:43:22 $
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

#ifndef _EEITEMID_HXX
#include <svx/eeitemid.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif

#ifndef _SVX_RULER_HXX
#include <svx/ruler.hxx>
#endif
#ifndef _SVX_RULERITEM_HXX
#include <svx/rulritem.hxx>
#endif
#ifndef _SVX_ZOOMITEM_HXX
#include <svx/zoomitem.hxx>
#endif
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
//#ifndef _IDETEMP_HXX //autogen
//#include <basctl/idetemp.hxx>
//#endif
#ifndef _SFXRECTITEM_HXX //autogen
#include <svtools/rectitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _SB_SBERRORS_HXX //autogen
#include <basic/sberrors.hxx>
#endif
#ifndef _SVX_FMSHELL_HXX //autogen
#include <svx/fmshell.hxx>
#endif

#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"

#include "sdundogr.hxx"
#include "undopage.hxx"
#include "glob.hxx"
#include "app.hxx"
#include "fupoor.hxx"
#include "fuslshow.hxx"
#include "frmview.hxx"
#include "sdpage.hxx"
#include "sdwindow.hxx"
#include "sdresid.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "drviewsh.hxx"
#include "sdruler.hxx"
#include "docshell.hxx"
#include "preview.hxx"

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif



/*************************************************************************
|*
|* SfxRequests fuer Controller bearbeiten
|*
\************************************************************************/

void __EXPORT SdDrawViewShell::ExecCtrl(SfxRequest& rReq)
{
    // waehrend einer Diashow wird nichts ausser dem Seitenwechsel und dem
    // Sprung zur Bookmark ausgefuehrt!
    if (pFuActual &&
        pFuActual->GetSlotID() == SID_PRESENTATION &&
        rReq.GetSlot() != SID_SWITCHPAGE &&
        rReq.GetSlot() != SID_JUMPTOMARK)
        return;

    CheckLineTo (rReq);

    if ( pDrView->IsTextEdit() )
    {
       pDrView->EndTextEdit();
    }

    USHORT nSlot = rReq.GetSlot();
    switch (nSlot)
    {
        case SID_SWITCHPAGE:  // BASIC
        {
            BOOL bWasBasic = FALSE;

            // Seitenwechsel in der Diashow abspielen
            if (pFuSlideShow && rReq.GetArgs())
            {
                SFX_REQUEST_ARG(rReq, pWhatPage, SfxUInt32Item, ID_VAL_WHATPAGE, FALSE);
                pFuSlideShow->JumpToPage((USHORT)pWhatPage->GetValue());
            }
            else
            {
                const SfxItemSet *pArgs = rReq.GetArgs ();
                USHORT nSelectedPage;

                if (! pArgs)
                {
                    nSelectedPage = aTabControl.GetCurPageId() - 1;
                }
                else if (pArgs->Count () == 2)
                {
                    SFX_REQUEST_ARG (rReq, pWhatPage, SfxUInt32Item, ID_VAL_WHATPAGE, FALSE);
                    SFX_REQUEST_ARG (rReq, pWhatKind, SfxUInt32Item, ID_VAL_WHATKIND, FALSE);

                    if (! CHECK_RANGE (PK_STANDARD, pWhatKind->GetValue (), PK_HANDOUT))
                    {
                        StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
                        rReq.Ignore ();
                        break;
                    }
                    else if (eEditMode != EM_MASTERPAGE)
                    {
                        if (! CHECK_RANGE (0, pWhatPage->GetValue (),
                            pDoc->GetSdPageCount ((PageKind) pWhatKind->GetValue ())))
                        {
                            StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
                            rReq.Ignore ();
                            break;
                        }

                        nSelectedPage = (short) pWhatPage->GetValue ();
                        ePageKind     = (PageKind) pWhatKind->GetValue ();
                        bWasBasic     = TRUE;
                    }
                }
                else
                {
                    StarBASIC::FatalError (SbERR_WRONG_ARGS);
                    rReq.Ignore ();
                    break;
                }

                SwitchPage(nSelectedPage);

                if (pFuActual && pFuActual->GetSlotID() == SID_BEZIER_EDIT)
                    GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

                Invalidate();
                InvalidateWindows();
                rReq.Done ();
            }
            break;
        }

        case SID_SWITCHLAYER:  // BASIC
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();
            USHORT nCurPage = aLayerTab.GetCurPageId ();

            if( pArgs && pArgs->Count () == 1)
            {
                SFX_REQUEST_ARG (rReq, pWhatLayer, SfxUInt32Item, ID_VAL_WHATLAYER, FALSE);
                if( pWhatLayer )
                    nCurPage = (short) pWhatLayer->GetValue ();
            }

            pDrView->SetActiveLayer( aLayerTab.GetPageText(nCurPage) );
            Invalidate();
            rReq.Done ();

            break;
        }

        case SID_PAGEMODE:  // BASIC
        {

            const SfxItemSet *pArgs = rReq.GetArgs ();

            if ( pArgs && pArgs->Count () == 2)
            {
                SFX_REQUEST_ARG (rReq, pIsActive, SfxBoolItem, ID_VAL_ISACTIVE, FALSE);
                SFX_REQUEST_ARG (rReq, pWhatKind, SfxUInt32Item, ID_VAL_WHATKIND, FALSE);

                if (CHECK_RANGE (PK_STANDARD, pWhatKind->GetValue (), PK_HANDOUT))
                {
                    bLayerMode = pIsActive->GetValue ();
                    ePageKind = (PageKind) pWhatKind->GetValue ();
                }
            }

            // Default-Layer der Page einschalten
            pDrView->SetActiveLayer( String( SdResId(STR_LAYER_LAYOUT) ) );

            ChangeEditMode(EM_PAGE, bLayerMode);

            Invalidate();
            rReq.Done ();

            break;
        }

        case SID_LAYERMODE:  // BASIC
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();

            if ( pArgs && pArgs->Count () == 2)
            {
                SFX_REQUEST_ARG (rReq, pWhatLayerMode, SfxBoolItem, ID_VAL_ISACTIVE, FALSE);
                SFX_REQUEST_ARG (rReq, pWhatLayer, SfxUInt32Item, ID_VAL_WHATLAYER, FALSE);

                if (CHECK_RANGE (EM_PAGE, pWhatLayer->GetValue (), EM_MASTERPAGE))
                {
                    bLayerMode = pWhatLayerMode->GetValue ();
                    eEditMode = (EditMode) pWhatLayer->GetValue ();
                }
            }

            ChangeEditMode(eEditMode, !bLayerMode);

            Invalidate();
            rReq.Done ();

            break;
        }

        case SID_OBJECTRESIZE:
        {
            /******************************************************************
            * Der Server moechte die Clientgrosse verandern
            ******************************************************************/
            SfxInPlaceClient* pIPClient = GetIPClient();

            if ( pIPClient && pIPClient->IsInPlaceActive() )
            {
                const SfxRectangleItem& rRect =
                    (SfxRectangleItem&)rReq.GetArgs()->Get(SID_OBJECTRESIZE);
                Rectangle aRect( pWindow->PixelToLogic( rRect.GetValue() ) );

                if ( pDrView->HasMarkedObj() )
                {
                    const SdrMarkList& rMarkList = pDrView->GetMarkList();

                    if (rMarkList.GetMarkCount() == 1)
                    {
                        SdrMark* pMark = rMarkList.GetMark(0);
                        SdrObject* pObj = pMark->GetObj();

                        UINT16 nSdrObjKind = pObj->GetObjIdentifier();

                        if (pObj->GetObjInventor() == SdrInventor &&
                            pObj->GetObjIdentifier() == OBJ_OLE2)
                        {
                            SvInPlaceObjectRef aIPObj =
                            ( (SdrOle2Obj*) pObj)->GetObjRef();

                            if ( aIPObj.Is() )
                            {
                                pObj->SetLogicRect(aRect);
                            }
                        }
                    }
                }
            }
            rReq.Ignore ();
            break;
        }

        case SID_RELOAD:
        {
            // DER SLOT KANN IN DEN SFX VERSCHOBEN WERDEN
            SFX_REQUEST_ARG(rReq, pForceReloadItem, SfxBoolItem, SID_NOCACHE, FALSE);
            // Normale Weiterleitung an ViewFrame zur Ausfuehrung
            GetViewFrame()->ExecuteSlot(rReq);

            // Muss sofort beendet werden
            return;
        }
        break;

        case SID_JUMPTOMARK:
        {
            if( rReq.GetArgs() )
            {
                SFX_REQUEST_ARG(rReq, pBookmark, SfxStringItem, SID_JUMPTOMARK, FALSE);

                UniString sBookmark( pBookmark->GetValue() );
                if( sBookmark.Search( sal_Unicode('#') ) == 0 )
                    sBookmark = sBookmark.Copy( 1 );

                if (pBookmark)
                {
                    if (pFuSlideShow)
                    {
                        pFuSlideShow->JumpToBookmark(sBookmark);
                    }
                    else
                    {
                        GotoBookmark( sBookmark );
                    }
                }
            }
            rReq.Done();
            break;
        }

        case SID_OUTPUT_QUALITY_COLOR:
        case SID_OUTPUT_QUALITY_GRAYSCALE:
        case SID_OUTPUT_QUALITY_BLACKWHITE:
        {
            ULONG nMode = DRAWMODE_DEFAULT;
            if( nSlot == SID_OUTPUT_QUALITY_GRAYSCALE )
                nMode = DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_BLACKTEXT |
                        DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT;
            else if( nSlot == SID_OUTPUT_QUALITY_BLACKWHITE )
                nMode = DRAWMODE_BLACKLINE | DRAWMODE_BLACKTEXT | DRAWMODE_WHITEFILL |
                        DRAWMODE_GRAYBITMAP | DRAWMODE_WHITEGRADIENT;

            pWindow->SetDrawMode( nMode );
            pDrView->ReleaseMasterPagePaintCache();
            pWindow->Invalidate();

            Invalidate();
            rReq.Done();
            break;
        }

        case SID_PREVIEW_QUALITY_COLOR:
        case SID_PREVIEW_QUALITY_GRAYSCALE:
        case SID_PREVIEW_QUALITY_BLACKWHITE:
        {
            ULONG nMode;
            switch(nSlot)
            {
            case SID_PREVIEW_QUALITY_COLOR:
                nMode = PREVIEW_DRAWMODE_COLOR;
                break;
            case SID_PREVIEW_QUALITY_GRAYSCALE:
                nMode = PREVIEW_DRAWMODE_GRAYSCALE;
                break;
            default: //case SID_PREVIEW_QUALITY_BLACKWHITE:
                nMode = PREVIEW_DRAWMODE_BLACKWHITE;
                break;
            }

            pFrameView->SetPreviewDrawMode( nMode );
            SdOptions* pOptions = SD_MOD()->GetSdOptions( pDoc->GetDocumentType() );
            pOptions->SetPreviewQuality( nMode );
            Invalidate();
            rReq.Done();
            break;
        }

        case SID_MAIL_SCROLLBODY_PAGEDOWN:
        {
            if (pFuActual)
            {
                pFuActual->ScrollStart();
                ScrollLines(0, -1);
                pFuActual->ScrollEnd();
            }
            rReq.Done();
        }
        break;

        case SID_ATTR_YEAR2000:
        {
            FmFormShell* pShell = (FmFormShell*) aShellTable.Get(RID_FORMLAYER_TOOLBOX);
            if( pShell )
            {
                const SfxPoolItem* pItem;
                if (rReq.GetArgs()->GetItemState(SID_ATTR_YEAR2000, TRUE, &pItem) == SFX_ITEM_SET)
                    pShell->SetY2KState( ( (const SfxUInt16Item*) pItem )->GetValue() );
            }

            rReq.Done();
        }
        break;

        default:
        break;
    }
}

/*************************************************************************
|*
|* SfxRequests fuer Lineale bearbeiten
|*
\************************************************************************/

void __EXPORT SdDrawViewShell::ExecRuler(SfxRequest& rReq)
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if (pFuActual &&
        pFuActual->GetSlotID() == SID_PRESENTATION)
        return;

    CheckLineTo (rReq);

    const SfxItemSet* pArgs = rReq.GetArgs();
    Point aPagePos = pWindow->GetViewOrigin();
    Size aPageSize = pActualPage->GetSize();
    Size aViewSize = pWindow->GetViewSize();
    SdUndoGroup* pUndoGroup = NULL;

    if ( rReq.GetSlot() == SID_ATTR_LONG_LRSPACE ||
         rReq.GetSlot() == SID_ATTR_LONG_ULSPACE )
    {
        pUndoGroup = new SdUndoGroup(pDoc);
        String aString(SdResId(STR_UNDO_CHANGE_PAGEBORDER));
        pUndoGroup->SetComment(aString);
    }

    switch ( rReq.GetSlot() )
    {
        case SID_ATTR_LONG_LRSPACE:
        {
            const SvxLongLRSpaceItem& rLRSpace = (const SvxLongLRSpaceItem&)
                    pArgs->Get(GetPool().GetWhich(SID_ATTR_LONG_LRSPACE));
            long nLeft = Max(0L, rLRSpace.GetLeft() - aPagePos.X());
            long nRight = Max(0L, rLRSpace.GetRight() + aPagePos.X() +
                                  aPageSize.Width() - aViewSize.Width());

            USHORT nPageCnt = pDoc->GetSdPageCount(ePageKind);

            for (USHORT i = 0; i < nPageCnt; i++)
            {
                SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
                SdUndoAction* pUndo = new SdPageLRUndoAction(pDoc,
                                        pPage,
                                        pPage->GetLftBorder(),
                                        pPage->GetRgtBorder(),
                                        nLeft, nRight);
                pUndoGroup->AddAction(pUndo);
                pPage->SetLftBorder(nLeft);
                pPage->SetRgtBorder(nRight);
            }
            nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);

            for (i = 0; i < nPageCnt; i++)
            {
                SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
                SdUndoAction* pUndo = new SdPageLRUndoAction(pDoc,
                                        pPage,
                                        pPage->GetLftBorder(),
                                        pPage->GetRgtBorder(),
                                        nLeft, nRight);
                pUndoGroup->AddAction(pUndo);
                pPage->SetLftBorder(nLeft);
                pPage->SetRgtBorder(nRight);
            }
            InvalidateWindows();
            break;
        }
        case SID_ATTR_LONG_ULSPACE:
        {
            const SvxLongULSpaceItem& rULSpace = (const SvxLongULSpaceItem&)
                    pArgs->Get(GetPool().GetWhich(SID_ATTR_LONG_ULSPACE));
            long nUpper = Max(0L, rULSpace.GetUpper() - aPagePos.Y());
            long nLower = Max(0L, rULSpace.GetLower() + aPagePos.Y() +
                                  aPageSize.Height() - aViewSize.Height());

            USHORT nPageCnt = pDoc->GetSdPageCount(ePageKind);

            for (USHORT i = 0; i < nPageCnt; i++)
            {
                SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
                SdUndoAction* pUndo = new SdPageULUndoAction(pDoc,
                                        pPage,
                                        pPage->GetUppBorder(),
                                        pPage->GetLwrBorder(),
                                        nUpper, nLower);
                pUndoGroup->AddAction(pUndo);
                pPage->SetUppBorder(nUpper);
                pPage->SetLwrBorder(nLower);
            }
            nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);

            for (i = 0; i < nPageCnt; i++)
            {
                SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
                SdUndoAction* pUndo = new SdPageULUndoAction(pDoc,
                                        pPage,
                                        pPage->GetUppBorder(),
                                        pPage->GetLwrBorder(),
                                        nUpper, nLower);
                pUndoGroup->AddAction(pUndo);
                pPage->SetUppBorder(nUpper);
                pPage->SetLwrBorder(nLower);
            }
            InvalidateWindows();
            break;
        }
/*      case SID_RULER_OBJECT:
        {
            Point aPagePos = pWindow->GetViewOrigin();
            Rectangle aRect = aMarkRect;
            aRect.SetPos(aRect.TopLeft() + aPagePos);

            const SvxObjectItem& rOI = (const SvxObjectItem&)
                    pArgs->Get(GetPool().GetWhich(SID_RULER_OBJECT));

            if ( rOI.GetStartX() != rOI.GetEndX() )
            {
                aRect.Left()  = rOI.GetStartX();
                aRect.Right() = rOI.GetEndX();
            }
            if ( rOI.GetStartY() != rOI.GetEndY() )
            {
                aRect.Top()    = rOI.GetStartY();
                aRect.Bottom() = rOI.GetEndY();
            }
            aRect.SetPos(aRect.TopLeft() - aPagePos);
            if ( aRect != aMarkRect)
            {
                pDrView->SetAllMarkedRect(aRect);
                aMarkRect = pDrView->GetAllMarkedRect();
                Invalidate( SID_RULER_OBJECT );
            }
            break;
        }
*/

/*      case SID_ATTR_TABSTOP:
        {
            if( pDrView->IsTextEdit() )
            {
                const SvxTabStopItem& rItem = (const SvxTabStopItem&)
                            pArgs->Get( ITEMID_TABSTOP );

                SfxItemSet aEditAttr( GetPool(), ITEMID_TABSTOP, ITEMID_TABSTOP );

                aEditAttr.Put( rItem );
                pDrView->SetAttributes( aEditAttr );
            }
            break;
        }
*/
        case SID_ATTR_PARA_LRSPACE:
        {
            if( pDrView->IsTextEdit() )
            {
                USHORT nId = SID_ATTR_PARA_LRSPACE;
                const SvxLRSpaceItem& rItem = (const SvxLRSpaceItem&)
                            pArgs->Get( nId );

                SfxItemSet aEditAttr( GetPool(), ITEMID_LRSPACE, ITEMID_LRSPACE );

                nId = ITEMID_LRSPACE;
                SvxLRSpaceItem aLRSpaceItem( rItem.GetLeft(),
                        rItem.GetRight(), rItem.GetTxtLeft(),
                        rItem.GetTxtFirstLineOfst(), nId );
                aEditAttr.Put( aLRSpaceItem );
                pDrView->SetAttributes( aEditAttr );
            }
            break;
        }
    }
    if ( pUndoGroup )
        // Undo Gruppe dem Undo Manager uebergeben
        GetViewFrame()->GetObjectShell()->GetUndoManager()->
                                            AddUndoAction(pUndoGroup);
}

/*************************************************************************
|*
|* Statuswerte der Lineale bestimmen
|*
\************************************************************************/
void __EXPORT SdDrawViewShell::GetRulerState(SfxItemSet& rSet)
{
    Point aOrigin;

    if (pDrView->GetPageViewPvNum(0))
    {
        aOrigin = pDrView->GetPageViewPvNum(0)->GetPageOrigin();
    }

    Size aViewSize = pWindow->GetViewSize();

    Point aPagePos = pWindow->GetViewOrigin();
    Size aPageSize = pActualPage->GetSize();

    Rectangle aRect(aPagePos, Point(
                    aViewSize.Width() - (aPagePos.X() + aPageSize.Width()),
                    aViewSize.Height() - (aPagePos.Y() + aPageSize.Height())));
    SfxRectangleItem aMinMax(SID_RULER_LR_MIN_MAX, aRect);
    rSet.Put(aMinMax);

    SvxLongLRSpaceItem aLRSpace(aPagePos.X() + pActualPage->GetLftBorder(),
                                aRect.Right() + pActualPage->GetRgtBorder(),
                                GetPool().GetWhich(SID_ATTR_LONG_LRSPACE));
    SvxLongULSpaceItem aULSpace(aPagePos.Y() + pActualPage->GetUppBorder(),
                                aRect.Bottom() + pActualPage->GetLwrBorder(),
                                GetPool().GetWhich(SID_ATTR_LONG_ULSPACE));
    rSet.Put(SvxPagePosSizeItem(Point(0,0) - aPagePos, aViewSize.Width(),
                                                       aViewSize.Height()));
    SfxPointItem aPointItem( SID_RULER_NULL_OFFSET, aPagePos + aOrigin );

    SvxProtectItem aProtect( SID_RULER_PROTECT );

    aMarkRect = pDrView->GetAllMarkedRect();

    if( pDrView->HasMarkedObj() )
    {
        if( pDrView->IsTextEdit() )
        {
            SdrObject* pObj = pDrView->GetMarkList().GetMark( 0 )->GetObj();
            if( pObj->GetObjInventor() == SdrInventor)
            {
                SfxItemSet aEditAttr( pDoc->GetPool() );
                pDrView->GetAttributes( aEditAttr );
                if( aEditAttr.GetItemState( ITEMID_TABSTOP ) >= SFX_ITEM_AVAILABLE )
                {
                    const SvxTabStopItem& rItem = (const SvxTabStopItem&) aEditAttr.Get( ITEMID_TABSTOP );
                    rSet.Put( rItem );

                    //Rectangle aRect = aMarkRect;

                    const SvxLRSpaceItem& rLRSpaceItem = (const SvxLRSpaceItem&) aEditAttr.Get( ITEMID_LRSPACE );
                    USHORT nId = SID_ATTR_PARA_LRSPACE;
                    SvxLRSpaceItem aLRSpaceItem( rLRSpaceItem.GetLeft(),
                            rLRSpaceItem.GetRight(), rLRSpaceItem.GetTxtLeft(),
                            rLRSpaceItem.GetTxtFirstLineOfst(), nId );
                    rSet.Put( aLRSpaceItem );

                    Point aPos( aPagePos + aMarkRect.TopLeft() );

                    if ( aEditAttr.GetItemState( SDRATTR_TEXT_LEFTDIST ) == SFX_ITEM_ON )
                    {
                        const SdrTextLeftDistItem& rTLDItem = (const SdrTextLeftDistItem&)
                                                              aEditAttr.Get( SDRATTR_TEXT_LEFTDIST );
                        long nLD = rTLDItem.GetValue();
                        aPos.X() += nLD;
                    }

                    aPointItem.SetValue( aPos );

                    aLRSpace.SetLeft( aPagePos.X() + aMarkRect.Left() );

                    if ( aEditAttr.GetItemState( SDRATTR_TEXT_LEFTDIST ) == SFX_ITEM_ON )
                    {
                        const SdrTextLeftDistItem& rTLDItem = (const SdrTextLeftDistItem&)
                                                              aEditAttr.Get( SDRATTR_TEXT_LEFTDIST );
                        long nLD = rTLDItem.GetValue();
                        aLRSpace.SetLeft( aLRSpace.GetLeft() + nLD );
                    }

                    aLRSpace.SetRight( aRect.Right() + aPageSize.Width() - aMarkRect.Right() );
                    aULSpace.SetUpper( aPagePos.Y() + aMarkRect.Top() );
                    aULSpace.SetLower( aRect.Bottom() + aPageSize.Height() - aMarkRect.Bottom() );

//                  rSet.DisableItem( SID_RULER_OBJECT );

                    // Seitenraender werden gelocked
                    aProtect.SetSizeProtect( TRUE );
                    aProtect.SetPosProtect( TRUE );
                }
            }
        }
        else if( pDrView->IsResizeAllowed(TRUE) )
        {
            Rectangle aRect = aMarkRect;

            aRect.SetPos(aRect.TopLeft() + aPagePos);
            SvxObjectItem aObjItem(aRect.Left(), aRect.Right(),
                                   aRect.Top(), aRect.Bottom());
            rSet.Put(aObjItem);
            rSet.DisableItem( ITEMID_TABSTOP );
        }
        else
        {
//          rSet.DisableItem( SID_RULER_OBJECT );
            rSet.DisableItem( ITEMID_TABSTOP );
        }
    }
    else
    {
//      rSet.DisableItem( SID_RULER_OBJECT );
        rSet.DisableItem( ITEMID_TABSTOP );
    }

    rSet.Put( aLRSpace );
    rSet.Put( aULSpace );

    rSet.Put( aPointItem );
    rSet.Put( aProtect );
}

/*************************************************************************
|*
|* SfxRequests fuer StatusBar bearbeiten
|*
\************************************************************************/

void __EXPORT SdDrawViewShell::ExecStatusBar(SfxRequest& rReq)
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if (pFuActual &&
        pFuActual->GetSlotID() == SID_PRESENTATION)
        return;

    CheckLineTo (rReq);

    const SfxItemSet* pArgs = rReq.GetArgs();

    switch ( rReq.GetSlot() )
    {
        case SID_ATTR_SIZE:
        {
            GetViewFrame()->GetDispatcher()->Execute( SID_ATTR_TRANSFORM, SFX_CALLMODE_ASYNCHRON );
        }
        break;

        case SID_STATUS_LAYOUT:
        {
            GetViewFrame()->GetDispatcher()->Execute( SID_PRESENTATION_LAYOUT, SFX_CALLMODE_ASYNCHRON );
        }
        break;
    }
}

/*************************************************************************
|*
|* Status der Snap-Objekt-Eintraege im Popup setzen
|*
\************************************************************************/

void __EXPORT SdDrawViewShell::GetSnapItemState( SfxItemSet &rSet )
{
    SdrPageView* pPV;
    Point   aMPos = pWindow->PixelToLogic(aMousePos);
    USHORT  nHitLog = (USHORT) pWindow->PixelToLogic(Size(HITPIX,0)).Width();
    USHORT  nHelpLine;

    if ( pDrView->PickHelpLine(aMPos, nHitLog, *pWindow, nHelpLine, pPV) )
    {
        const SdrHelpLine& rHelpLine = (pPV->GetHelpLines())[nHelpLine];

        if ( rHelpLine.GetKind() == SDRHELPLINE_POINT )
        {
            rSet.Put( SfxStringItem( SID_SET_SNAPITEM,
                                String( SdResId( STR_POPUP_EDIT_SNAPPOINT))) );
            rSet.Put( SfxStringItem( SID_DELETE_SNAPITEM,
                                String( SdResId( STR_POPUP_DELETE_SNAPPOINT))) );
        }
        else
        {
            rSet.Put( SfxStringItem( SID_SET_SNAPITEM,
                                String( SdResId( STR_POPUP_EDIT_SNAPLINE))) );
            rSet.Put( SfxStringItem( SID_DELETE_SNAPITEM,
                                String( SdResId( STR_POPUP_DELETE_SNAPLINE))) );
        }
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SdDrawViewShell::AddWindow(SdWindow* pWin)
{
    pDrView->AddWin(pWin);
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SdDrawViewShell::RemoveWindow(SdWindow* pWin)
{
    pDrView->DelWin(pWin);
}


