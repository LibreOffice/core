/*************************************************************************
 *
 *  $RCSfile: fupage.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 10:57:50 $
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

#pragma hdrstop

// Seite einrichten Tab-Page
#define ITEMID_PAGE         SID_ATTR_PAGE
#define ITEMID_LRSPACE      SID_ATTR_LRSPACE
#define ITEMID_ULSPACE      SID_ATTR_ULSPACE
#define ITEMID_SIZE         SID_ATTR_PAGE_SIZE
#define ITEMID_PAPERBIN     SID_ATTR_PAGE_PAPERBIN

#define ITEMID_BOX          SID_ATTR_BORDER_OUTER
#define ITEMID_SHADOW       SID_ATTR_BORDER_SHADOW

#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SV_PRNTYPES_HXX //autogen
#include <vcl/prntypes.hxx>
#endif
#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif
#ifndef _SD_STLSHEET_HXX
#include <stlsheet.hxx>
#endif
#ifndef _SVX_SVDORECT_HXX
#include <svx/svdorect.hxx>
#endif
#ifndef _SVX_SVDUNDO_HXX
#include <svx/svdundo.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#define ITEMID_FRAMEDIR             EE_PARA_WRITINGDIR
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#include "glob.hrc"
#include <svx/shaditem.hxx>
#include <svx/boxitem.hxx>
#include <svx/sizeitem.hxx>
#include <svx/ulspitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/pbinitem.hxx>
#include <sfx2/app.hxx>


#include "strings.hrc"
#include "dlgpage.hxx"
#include "sdpage.hxx"
#include "sdview.hxx"
#include "pres.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "viewshel.hxx"
#include "drviewsh.hxx"
#include "fupage.hxx"
#include "app.hrc"
#include "preview.hxx"
#include "prevchld.hxx"
#include "unchss.hxx"
#include "undoback.hxx"

// 50 cm 28350
// erstmal vom Writer uebernommen
#define MAXHEIGHT 28350
#define MAXWIDTH  28350

class SdWindow;

TYPEINIT1( FuPage, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuPage::FuPage( SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
                 SdDrawDocument* pDoc, SfxRequest& rReq )
       : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    const SfxItemSet*           pArgs = rReq.GetArgs();
    SdPage*                     pPage = NULL;
    SdBackgroundObjUndoAction*  pBackgroundObjUndoAction = NULL;
    Size                        aSize;
    PageKind                    ePageKind = PK_STANDARD;
    BOOL                        bMasterPage = TRUE;
    BOOL                        bPageBckgrdDeleted = FALSE;

    ///////////////////////////////////////////////////////////////////////////
    //
    // Retrieve current page
    //
    if ( pViewSh->ISA(SdDrawViewShell) )
        ePageKind = ((SdDrawViewShell*) pViewSh)->GetPageKind();

     // shall we display background area tabpage?
    bool bDisplayBackgroundTabPage = (ePageKind == PK_STANDARD) ? TRUE : FALSE;

    if( ( (SdDrawViewShell*) pViewSh )->GetEditMode() == EM_MASTERPAGE )
    {
        pPage = pDoc->GetSdPage(0, ePageKind);
    }
    else
    {
        bMasterPage = FALSE;
        pPage = ( (SdDrawViewShell*) pViewSh )->GetActualPage();
    }

    if( !pArgs )
    {
        pView->EndTextEdit();

        SfxItemSet aNewAttr(pDoc->GetPool(),
                            pDoc->GetPool().GetWhich(SID_ATTR_LRSPACE),
                            pDoc->GetPool().GetWhich(SID_ATTR_ULSPACE),
                            SID_ATTR_PAGE, SID_ATTR_PAGE_BSP,
                            SID_ATTR_BORDER_OUTER, SID_ATTR_BORDER_OUTER,
                            SID_ATTR_BORDER_SHADOW, SID_ATTR_BORDER_SHADOW,
                            XATTR_FILL_FIRST, XATTR_FILL_LAST,
                            EE_PARA_WRITINGDIR, EE_PARA_WRITINGDIR,
                            0);

        SfxItemSet *pDialogItems = 0;

        ///////////////////////////////////////////////////////////////////////
        //
        // Retrieve additional data for dialog
        //
        SvxShadowItem aShadowItem;
        aNewAttr.Put( aShadowItem );
        SvxBoxItem aBoxItem;
        aNewAttr.Put( aBoxItem );


        aNewAttr.Put( SvxFrameDirectionItem( pDoc->GetDefaultWritingMode() == ::com::sun::star::text::WritingMode_RL_TB ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP ) );

        ///////////////////////////////////////////////////////////////////////
        //
        // Retrieve page-data for dialog
        //
        SvxPageItem aPageItem;
        aPageItem.SetDescName( pPage->GetName() );
        aPageItem.SetPageUsage( (SvxPageUsage) SVX_PAGE_ALL );

        Orientation eOrientation = pPage->GetOrientation();

        if (eOrientation == ORIENTATION_LANDSCAPE)
        {
            aPageItem.SetLandscape(TRUE);
        }
        else
        {
            aPageItem.SetLandscape(FALSE);
        }

        aPageItem.SetNumType( pDoc->GetPageNumType() );
        aNewAttr.Put( aPageItem );

        // size
        aSize = pPage->GetSize();
        SvxSizeItem aSizeItem( SID_ATTR_PAGE_SIZE, aSize );
        aNewAttr.Put( aSizeItem );

        // Max size
        SvxSizeItem aMaxSizeItem( SID_ATTR_PAGE_MAXSIZE, Size( MAXWIDTH, MAXHEIGHT ) );
        aNewAttr.Put( aMaxSizeItem );

        // get printer
        SfxPrinter* pPrinter = ( (SdDrawDocShell*) pViewSh->GetViewFrame()->GetObjectShell() )->GetPrinter(TRUE);

        // paperbin
        SvxPaperBinItem aPaperBinItem( SID_ATTR_PAGE_PAPERBIN, pPage->GetPaperBin() );
        aNewAttr.Put( aPaperBinItem );

        // Raender, Umrandung und das andere Zeug
        //
        SvxLRSpaceItem aLRSpaceItem(
                            (USHORT) pPage->GetLftBorder(),
                            (USHORT) pPage->GetRgtBorder(), 0, 0,
                                pDoc->GetPool().GetWhich(SID_ATTR_LRSPACE));
        aNewAttr.Put( aLRSpaceItem );

        SvxULSpaceItem aULSpaceItem(
                            (USHORT) pPage->GetUppBorder(),
                            (USHORT) pPage->GetLwrBorder(),
                            pDoc->GetPool().GetWhich(SID_ATTR_ULSPACE));
        aNewAttr.Put( aULSpaceItem );


        // Applikation
        BOOL bScale = TRUE;
        if( pDoc->GetDocumentType() == DOCUMENT_TYPE_DRAW )
            bScale = FALSE;
        aNewAttr.Put( SfxBoolItem( SID_ATTR_PAGE_EXT1, bScale ) );

        BOOL bFullSize = ( (SdPage*)( pPage->GetMasterPage( 0 ) ) )->IsBackgroundFullSize();
        aNewAttr.Put( SfxBoolItem( SID_ATTR_PAGE_EXT2, bFullSize ) );

        ///////////////////////////////////////////////////////////////////////
        //
        // Merge ItemSet for dialog
        //
        const USHORT* pPtr = aNewAttr.GetRanges();
        USHORT p1 = pPtr[0], p2 = pPtr[1];
        while(pPtr[2] && (pPtr[2] - p2 == 1))
        {
            p2 = pPtr[3];
            pPtr += 2;
        }
        pPtr += 2;
        SfxItemSet aMergedAttr( *aNewAttr.GetPool(), p1, p2 );

        while( *pPtr )
        {
            p1 = pPtr[0];
            p2 = pPtr[1];

            // erstmal das ganze discret machen
            while(pPtr[2] && (pPtr[2] - p2 == 1))
            {
                p2 = pPtr[3];
                pPtr += 2;
            }
            aMergedAttr.MergeRange( p1, p2 );
            pPtr += 2;
        }

        SfxStyleSheetBasePool* pSSPool = pDoc->GetDocSh()->GetStyleSheetPool();
        SfxStyleSheetBase* pStyleSheet = NULL;
        if(pSSPool)
        {
            String aStr(SdResId(STR_PSEUDOSHEET_BACKGROUND));
            pStyleSheet = pSSPool->Find( aStr, SFX_STYLE_FAMILY_PSEUDO);
        }

        if( bDisplayBackgroundTabPage )
        {
            if( bMasterPage )
            {
                if(pStyleSheet)
                {
                    SfxItemSet aStyleSet( pStyleSheet->GetItemSet());

                    pPtr = aStyleSet.GetRanges();
                    while( *pPtr )
                    {
                        p1 = pPtr[0];
                        p2 = pPtr[1];

                        // erstmal das ganze discret machen
                        while(pPtr[2] && (pPtr[2] - p2 == 1))
                        {
                            p2 = pPtr[3];
                            pPtr += 2;
                        }
                        aMergedAttr.MergeRange( p1, p2 );
                        pPtr += 2;
                    }

                    aMergedAttr.Put(aStyleSet);
                }
            }
            else
            {
                // Only this page
                SdrObject* pObj = pPage->GetBackgroundObj();
                if( pObj )
                {
    //-/                pObj->TakeAttributes( aMergedAttr, TRUE, TRUE );
                    aMergedAttr.Put(pObj->GetItemSet());
                }
                else
                {
                    // if the page hasn't got a background-object, than use
                    // the fillstyle-settings of the masterpage for the dialog
                    if( pStyleSheet->GetItemSet().GetItemState( XATTR_FILLSTYLE ) != SFX_ITEM_DEFAULT )
                        aMergedAttr.Put( pStyleSheet->GetItemSet() );
                    else
                        aMergedAttr.Put( XFillStyleItem( XFILL_NONE ) );
                }
            }
        }

        aMergedAttr.Put(aNewAttr);

        SdPageDlg* pDlg = new SdPageDlg( pDocSh, NULL, &aMergedAttr, bDisplayBackgroundTabPage );

        USHORT nResult = pDlg->Execute();

        switch( nResult )
        {
            case RET_OK:
            {
                SfxItemSet aTempSet(*pDlg->GetOutputItemSet());
                ((SdStyleSheet*)pStyleSheet)->AdjustToFontHeight(aTempSet);

                if( bDisplayBackgroundTabPage )
                {
                    // if some fillstyle-items are not set in the dialog, then
                    // try to use the items before
                    BOOL bChanges = FALSE;
                    for( int i=XATTR_FILL_FIRST; i<XATTR_FILL_LAST; i++ )
                    {
                        if( aMergedAttr.GetItemState( i ) != SFX_ITEM_DEFAULT )
                        {
                            if( aTempSet.GetItemState( i ) == SFX_ITEM_DEFAULT )
                                aTempSet.Put( aMergedAttr.Get( i ) );
                            else
                                if( aMergedAttr.GetItem( i ) != aTempSet.GetItem( i ) )
                                    bChanges = TRUE;
                        }
                    }

                    // if the background for this page was set to invisible, the background-object has to be deleted, too.
                    if( ( ( (XFillStyleItem*) aTempSet.GetItem( XATTR_FILLSTYLE ) )->GetValue() == XFILL_NONE ) ||
                        ( ( aTempSet.GetItemState( XATTR_FILLSTYLE ) == SFX_ITEM_DEFAULT ) &&
                          ( ( (XFillStyleItem*) aMergedAttr.GetItem( XATTR_FILLSTYLE ) )->GetValue() == XFILL_NONE ) ) )
                        bPageBckgrdDeleted = TRUE;

                    // Ask, wether the setting are for the background-page or for the current page
                    if( !bMasterPage && bChanges )
                    {
                        // But don't ask in notice-view, because we can't change the background of
                        // notice-masterpage (at the moment)
                        if( ePageKind != PK_NOTES )
                        {
                            String aTit(SdResId( STR_PAGE_BACKGROUND_TITLE ));
                            String aTxt(SdResId( STR_PAGE_BACKGROUND_TXT ));
                            MessBox aQuestionBox( (Window*)pWin, WB_YES_NO | WB_DEF_YES,
                                                  aTit,
                                                  aTxt );
                            aQuestionBox.SetImage( QueryBox::GetStandardImage() );
                            bMasterPage = ( RET_YES == aQuestionBox.Execute() );
                        }

                        if( bPageBckgrdDeleted )
                        {
                            pBackgroundObjUndoAction = new SdBackgroundObjUndoAction( *pDoc, *pPage, pPage->GetBackgroundObj() );
                            pPage->SetBackgroundObj( NULL );
                        }

                    }

                    // Sonderbehandlung: die INVALIDS auf NULL-Pointer
                    // zurueckgesetzen (sonst landen INVALIDs oder
                    // Pointer auf die DefaultItems in der Vorlage;
                    // beides wuerde die Attribut-Vererbung unterbinden)
                    aTempSet.ClearInvalidItems();

                    if( bMasterPage )
                    {
                        StyleSheetUndoAction* pAction = new StyleSheetUndoAction(pDoc, (SfxStyleSheet*)pStyleSheet, &aTempSet);
                        pDocSh->GetUndoManager()->AddUndoAction(pAction);
                        pStyleSheet->GetItemSet().Put( aTempSet );
                        SdStyleSheet* pRealSheet =((SdStyleSheet*)pStyleSheet)->GetRealStyleSheet();
                        pRealSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                    }

                    const SfxPoolItem *pItem;
                    if( SFX_ITEM_SET == aTempSet.GetItemState( EE_PARA_WRITINGDIR, sal_False, &pItem ) )
                    {
                        sal_uInt32 nVal = ((SvxFrameDirectionItem*)pItem)->GetValue();
                        pDoc->SetDefaultWritingMode( nVal == FRMDIR_HORI_RIGHT_TOP ? ::com::sun::star::text::WritingMode_RL_TB : ::com::sun::star::text::WritingMode_LR_TB );
                    }

                    pDoc->SetChanged(TRUE);

                    SdrObject* pObj = ( (SdPage*)( pPage->GetMasterPage( 0 ) ) )->GetPresObj( PRESOBJ_BACKGROUND );
                    if( pObj )
                    {
                        // BackgroundObj: no hard attributes allowed
                        SfxItemSet aSet( pDoc->GetPool() );
    //-/                    pObj->NbcSetAttributes( aSet, TRUE );
                        pObj->SetItemSet(aSet);
                    }
                }

                aNewAttr.Put(aTempSet);
                rReq.Done( aNewAttr );

                pArgs = rReq.GetArgs();
            }
            break;

            default:
            {
                delete pDlg;
            }
            return; // Abbruch
        }
        delete( pDlg );
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // Set new page-attributes
    //
    const SfxPoolItem*  pPoolItem;
    BOOL                bSetPageSizeAndBorder = FALSE;
    Size                aNewSize(aSize);
    INT32               nLeft  = -1, nRight = -1, nUpper = -1, nLower = -1;
    BOOL                bScaleAll = TRUE;
    Orientation         eOrientation = pPage->GetOrientation();
    BOOL                bFullSize = ( (SdPage*)( pPage->GetMasterPage( 0 ) ) )->IsBackgroundFullSize();
    USHORT              nPaperBin = pPage->GetPaperBin();

    if( pArgs->GetItemState(SID_ATTR_PAGE, TRUE, &pPoolItem) == SFX_ITEM_SET )
    {
        pDoc->SetPageNumType(((const SvxPageItem*) pPoolItem)->GetNumType());

        if (((const SvxPageItem*) pPoolItem)->IsLandscape() == ORIENTATION_LANDSCAPE)
        {
            eOrientation = ORIENTATION_LANDSCAPE;
        }
        else
        {
            eOrientation = ORIENTATION_PORTRAIT;
        }

        if( pPage->GetOrientation() != eOrientation )
            bSetPageSizeAndBorder = TRUE;

        if ( pViewSh->ISA(SdDrawViewShell) )
            ((SdDrawViewShell*) pViewSh)->ResetActualPage();
    }

    if( pArgs->GetItemState(SID_ATTR_PAGE_SIZE, TRUE, &pPoolItem) == SFX_ITEM_SET )
    {
        aNewSize = ((const SvxSizeItem*) pPoolItem)->GetSize();

        if( pPage->GetSize() != aNewSize )
            bSetPageSizeAndBorder = TRUE;
    }

    if( pArgs->GetItemState(pDoc->GetPool().GetWhich(SID_ATTR_LRSPACE),
                            TRUE, &pPoolItem) == SFX_ITEM_SET )
    {
        nLeft = ((const SvxLRSpaceItem*) pPoolItem)->GetLeft();
        nRight = ((const SvxLRSpaceItem*) pPoolItem)->GetRight();

        if( pPage->GetLftBorder() != nLeft || pPage->GetRgtBorder() != nRight )
            bSetPageSizeAndBorder = TRUE;

    }

    if( pArgs->GetItemState(pDoc->GetPool().GetWhich(SID_ATTR_ULSPACE),
                            TRUE, &pPoolItem) == SFX_ITEM_SET )
    {
        nUpper = ((const SvxULSpaceItem*) pPoolItem)->GetUpper();
        nLower = ((const SvxULSpaceItem*) pPoolItem)->GetLower();

        if( pPage->GetUppBorder() != nUpper || pPage->GetLwrBorder() != nLower )
            bSetPageSizeAndBorder = TRUE;
    }

    if( pArgs->GetItemState(pDoc->GetPool().GetWhich(SID_ATTR_PAGE_EXT1),
                            TRUE, &pPoolItem) == SFX_ITEM_SET )
    {
        bScaleAll = ((const SfxBoolItem*) pPoolItem)->GetValue();
    }

    if( pArgs->GetItemState(pDoc->GetPool().GetWhich(SID_ATTR_PAGE_EXT2),
                            TRUE, &pPoolItem) == SFX_ITEM_SET )
    {
        bFullSize = ((const SfxBoolItem*) pPoolItem)->GetValue();

        if( ( (SdPage*)( pPage->GetMasterPage( 0 ) ) )->IsBackgroundFullSize() != bFullSize )
            bSetPageSizeAndBorder = TRUE;
    }

    // Papierschacht (PaperBin)
    if( pArgs->GetItemState(pDoc->GetPool().GetWhich(SID_ATTR_PAGE_PAPERBIN),
                            TRUE, &pPoolItem) == SFX_ITEM_SET )
    {
        nPaperBin = ((const SvxPaperBinItem*) pPoolItem)->GetValue();

        if( pPage->GetPaperBin() != nPaperBin )
            bSetPageSizeAndBorder = TRUE;
    }

    if (nLeft == -1 && nUpper != -1)
    {
        bSetPageSizeAndBorder = TRUE;
        nLeft  = pPage->GetLftBorder();
        nRight = pPage->GetRgtBorder();
    }
    else if (nLeft != -1 && nUpper == -1)
    {
        bSetPageSizeAndBorder = TRUE;
        nUpper = pPage->GetUppBorder();
        nLower = pPage->GetLwrBorder();
    }

    if( bSetPageSizeAndBorder || !bMasterPage )
        pViewSh->SetPageSizeAndBorder(ePageKind, aNewSize, nLeft, nRight, nUpper,
                                      nLower, bScaleAll, eOrientation, nPaperBin, bFullSize );

    ////////////////////////////////////////////////////////////////////////////////
    //
    // if bMasterPage==FALSE then create a background-object for this page with the
    // properties set in the dialog before, but if bPageBckgrdDeleted==TRUE then
    // the background of this page was set to invisible, so it would be a mistake
    // to create a new background-object for this page !
    //

    if( bDisplayBackgroundTabPage )
    {
        if ( !bMasterPage && !bPageBckgrdDeleted )
        {
            // Only this page
            SdrObject* pObj = pPage->GetBackgroundObj();

            delete pBackgroundObjUndoAction;
            pBackgroundObjUndoAction = new SdBackgroundObjUndoAction( *pDoc, *pPage, pObj );

            if( !pObj )
            {
                pObj = new SdrRectObj();
                pPage->SetBackgroundObj( pObj );
            }

            Point aPos ( nLeft, nUpper );
            Size aSize( pPage->GetSize() );
            aSize.Width()  -= nLeft  + nRight - 1;
            aSize.Height() -= nUpper + nLower - 1;
            Rectangle aRect( aPos, aSize );
            pObj->SetLogicRect( aRect );
            pObj->SetItemSet(*pArgs);
        }
    }

    // add undo action for background object
    if( pBackgroundObjUndoAction )
    {
        // set merge flag, because a SdUndoGroupAction could have been inserted before
        pDocSh->GetUndoManager()->AddUndoAction( pBackgroundObjUndoAction, TRUE );
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // Objekte koennen max. so gross wie die ViewSize werden
    //
    Size aPageSize = pDoc->GetSdPage(0, ePageKind)->GetSize();
    Size aViewSize = Size(aPageSize.Width() * 3, aPageSize.Height() * 2);
    pDoc->SetMaxObjSize(aViewSize);

    ///////////////////////////////////////////////////////////////////////////
    //
    // ggfs. Preview den neuen Kontext mitteilen
    //
    pViewSh->UpdatePreview( pViewSh->GetActualPage() );
}


