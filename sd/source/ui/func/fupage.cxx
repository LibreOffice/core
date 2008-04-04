/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fupage.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 12:45:30 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include "fupage.hxx"

#include <sfx2/viewfrm.hxx>

// Seite einrichten Tab-Page

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
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#include <svx/xbtmpit.hxx>
#include <svx/xsetit.hxx>
#include <svtools/itempool.hxx>
#include <svx/ulspitem.hxx>
#include <svx/lrspitem.hxx>

#include "glob.hrc"
#include <svx/shaditem.hxx>
#include <svx/boxitem.hxx>
#include <svx/sizeitem.hxx>
#include <svx/ulspitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/pbinitem.hxx>
#include <sfx2/app.hxx>
#include <svx/opengrf.hxx>

#include "strings.hrc"
#include "sdpage.hxx"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "pres.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#include "app.hrc"
#include "unchss.hxx"
#include "undoback.hxx"
#include "sdabstdlg.hxx"
#include "sdresid.hxx"
#include "helpids.h"

namespace sd {

class Window;

// 50 cm 28350
// erstmal vom Writer uebernommen
#define MAXHEIGHT 28350
#define MAXWIDTH  28350


TYPEINIT1( FuPage, FuPoor );

void mergeItemSetsImpl( SfxItemSet& rTarget, const SfxItemSet& rSource )
{
    const USHORT* pPtr = rSource.GetRanges();
    USHORT p1, p2;
    while( *pPtr )
    {
        p1 = pPtr[0];
        p2 = pPtr[1];

        // make ranges discret
        while(pPtr[2] && (pPtr[2] - p2 == 1))
        {
            p2 = pPtr[3];
            pPtr += 2;
        }
        rTarget.MergeRange( p1, p2 );
        pPtr += 2;
    }

    rTarget.Put(rSource);
}

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuPage::FuPage( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
                 SdDrawDocument* pDoc, SfxRequest& rReq )
:   FuPoor(pViewSh, pWin, pView, pDoc, rReq),
    mrReq(rReq),
    mpArgs( rReq.GetArgs() ),
    mpBackgroundObjUndoAction( 0 ),
    mbPageBckgrdDeleted( false ),
    mbMasterPage( false ),
    mbDisplayBackgroundTabPage( true ),
    mpPage(0)
{
}

FunctionReference FuPage::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuPage( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuPage::DoExecute( SfxRequest& )
{
    mpDrawViewShell = dynamic_cast<DrawViewShell*>(mpViewShell);
    DBG_ASSERT( mpDrawViewShell, "sd::FuPage::FuPage(), called without a current DrawViewShell!" );
    if( mpDrawViewShell )
    {
        mbMasterPage = mpDrawViewShell->GetEditMode() == EM_MASTERPAGE;
        mbDisplayBackgroundTabPage = (mpDrawViewShell->GetPageKind() == PK_STANDARD);
        mpPage = mpDrawViewShell->getCurrentPage();
    }

    if( mpPage )
    {
        // if there are no arguments given, open the dialog
        if( !mpArgs )
        {
            mpView->SdrEndTextEdit();
            mpArgs = ExecuteDialog(mpWindow);
        }

        // if we now have arguments, apply them to current page
        if( mpArgs )
            ApplyItemSet( mpArgs );
    }
}

FuPage::~FuPage()
{
    delete mpBackgroundObjUndoAction;
}

void FuPage::Activate()
{
}

void FuPage::Deactivate()
{
}

const SfxItemSet* FuPage::ExecuteDialog( Window* pParent )
{
    PageKind ePageKind = mpDrawViewShell->GetPageKind();

    SfxItemSet aNewAttr(mpDoc->GetPool(),
                        mpDoc->GetPool().GetWhich(SID_ATTR_LRSPACE),
                        mpDoc->GetPool().GetWhich(SID_ATTR_ULSPACE),
                        SID_ATTR_PAGE, SID_ATTR_PAGE_BSP,
                        SID_ATTR_BORDER_OUTER, SID_ATTR_BORDER_OUTER,
                        SID_ATTR_BORDER_SHADOW, SID_ATTR_BORDER_SHADOW,
                        XATTR_FILL_FIRST, XATTR_FILL_LAST,
                        EE_PARA_WRITINGDIR, EE_PARA_WRITINGDIR,
                        0);

    ///////////////////////////////////////////////////////////////////////
    // Retrieve additional data for dialog

    SvxShadowItem aShadowItem(SID_ATTR_BORDER_SHADOW);
    aNewAttr.Put( aShadowItem );
    SvxBoxItem aBoxItem( SID_ATTR_BORDER_OUTER );
    aNewAttr.Put( aBoxItem );

    aNewAttr.Put( SvxFrameDirectionItem(
        mpDoc->GetDefaultWritingMode() == ::com::sun::star::text::WritingMode_RL_TB ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP,
        EE_PARA_WRITINGDIR ) );

    ///////////////////////////////////////////////////////////////////////
    // Retrieve page-data for dialog

    SvxPageItem aPageItem( SID_ATTR_PAGE );
    aPageItem.SetDescName( mpPage->GetName() );
    aPageItem.SetPageUsage( (SvxPageUsage) SVX_PAGE_ALL );
    aPageItem.SetLandscape( mpPage->GetOrientation() == ORIENTATION_LANDSCAPE ? TRUE: FALSE );
    aPageItem.SetNumType( mpDoc->GetPageNumType() );
    aNewAttr.Put( aPageItem );

    // size
    maSize = mpPage->GetSize();
    SvxSizeItem aSizeItem( SID_ATTR_PAGE_SIZE, maSize );
    aNewAttr.Put( aSizeItem );

    // Max size
    SvxSizeItem aMaxSizeItem( SID_ATTR_PAGE_MAXSIZE, Size( MAXWIDTH, MAXHEIGHT ) );
    aNewAttr.Put( aMaxSizeItem );

    // paperbin
    SvxPaperBinItem aPaperBinItem( SID_ATTR_PAGE_PAPERBIN, (const BYTE)mpPage->GetPaperBin() );
    aNewAttr.Put( aPaperBinItem );

    SvxLRSpaceItem aLRSpaceItem( (USHORT)mpPage->GetLftBorder(), (USHORT)mpPage->GetRgtBorder(), 0, 0, mpDoc->GetPool().GetWhich(SID_ATTR_LRSPACE));
    aNewAttr.Put( aLRSpaceItem );

    SvxULSpaceItem aULSpaceItem( (USHORT)mpPage->GetUppBorder(), (USHORT)mpPage->GetLwrBorder(), mpDoc->GetPool().GetWhich(SID_ATTR_ULSPACE));
    aNewAttr.Put( aULSpaceItem );

    // Applikation
    bool bScale = mpDoc->GetDocumentType() != DOCUMENT_TYPE_DRAW;
    aNewAttr.Put( SfxBoolItem( SID_ATTR_PAGE_EXT1, bScale ? TRUE : FALSE ) );

    BOOL bFullSize = mpPage->IsMasterPage() ?
        mpPage->IsBackgroundFullSize() : ((SdPage&)mpPage->TRG_GetMasterPage()).IsBackgroundFullSize();

    aNewAttr.Put( SfxBoolItem( SID_ATTR_PAGE_EXT2, bFullSize ) );

    ///////////////////////////////////////////////////////////////////////
    // Merge ItemSet for dialog

    const USHORT* pPtr = aNewAttr.GetRanges();
    USHORT p1 = pPtr[0], p2 = pPtr[1];
    while(pPtr[2] && (pPtr[2] - p2 == 1))
    {
        p2 = pPtr[3];
        pPtr += 2;
    }
    pPtr += 2;
    SfxItemSet aMergedAttr( *aNewAttr.GetPool(), p1, p2 );

    mergeItemSetsImpl( aMergedAttr, aNewAttr );

    SdStyleSheet* pStyleSheet = mpPage->getPresentationStyle(HID_PSEUDOSHEET_BACKGROUND);

    // merge page background filling to the dialogs input set
    if( mbDisplayBackgroundTabPage )
    {
        if( mbMasterPage )
        {
            if(pStyleSheet)
                mergeItemSetsImpl( aMergedAttr, pStyleSheet->GetItemSet() );
        }
        else
        {
            // Only this page, check if there is a background-object on that page
            SdrObject* pObj = mpPage->GetBackgroundObj();
            if( pObj )
            {
                aMergedAttr.Put(pObj->GetMergedItemSet());
            }
            else
            {
                // if the page hasn't got a background-object, than use
                // the fillstyle-settings of the masterpage for the dialog
                if( pStyleSheet && pStyleSheet->GetItemSet().GetItemState( XATTR_FILLSTYLE ) != SFX_ITEM_DEFAULT )
                    mergeItemSetsImpl( aMergedAttr, pStyleSheet->GetItemSet() );
                else
                    aMergedAttr.Put( XFillStyleItem( XFILL_NONE ) );
            }
        }
    }

    std::auto_ptr< SfxItemSet > pTempSet;

    if( GetSlotID() == SID_SELECT_BACKGROUND )
    {
        SvxOpenGraphicDialog    aDlg(SdResId(STR_SET_BACKGROUND_PICTURE));

        if( aDlg.Execute() == GRFILTER_OK )
        {
            Graphic     aGraphic;
            int nError = aDlg.GetGraphic(aGraphic);
            if( nError == GRFILTER_OK )
            {
                pTempSet.reset( new SfxItemSet( mpDoc->GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST, 0) );

                pTempSet->Put( XFillStyleItem( XFILL_BITMAP ) );
                pTempSet->Put( XFillBitmapItem( String(RTL_CONSTASCII_USTRINGPARAM("background")), XOBitmap(aGraphic) ) );
                pTempSet->Put( XFillBmpStretchItem( TRUE ));
                pTempSet->Put( XFillBmpTileItem( FALSE ));
            }
        }
    }
    else
    {
        // create the dialog
        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        std::auto_ptr<SfxAbstractTabDialog> pDlg( pFact ? pFact->CreateSdTabPageDialog(NULL, &aMergedAttr, mpDocSh, mbDisplayBackgroundTabPage ) : 0 );
        if( pDlg.get() && pDlg->Execute() == RET_OK )
            pTempSet.reset( new SfxItemSet(*pDlg->GetOutputItemSet()) );
    }

    if( pTempSet.get() )
    {
        pStyleSheet->AdjustToFontHeight(*pTempSet);

        if( mbDisplayBackgroundTabPage )
        {
            // if some fillstyle-items are not set in the dialog, then
            // try to use the items before
            BOOL bChanges = FALSE;
            for( USHORT i=XATTR_FILL_FIRST; i<XATTR_FILL_LAST; i++ )
            {
                if( aMergedAttr.GetItemState( i ) != SFX_ITEM_DEFAULT )
                {
                    if( pTempSet->GetItemState( i ) == SFX_ITEM_DEFAULT )
                        pTempSet->Put( aMergedAttr.Get( i ) );
                    else
                        if( aMergedAttr.GetItem( i ) != pTempSet->GetItem( i ) )
                            bChanges = TRUE;
                }
            }

            // if the background for this page was set to invisible, the background-object has to be deleted, too.
            if( ( ( (XFillStyleItem*) pTempSet->GetItem( XATTR_FILLSTYLE ) )->GetValue() == XFILL_NONE ) ||
                ( ( pTempSet->GetItemState( XATTR_FILLSTYLE ) == SFX_ITEM_DEFAULT ) &&
                    ( ( (XFillStyleItem*) aMergedAttr.GetItem( XATTR_FILLSTYLE ) )->GetValue() == XFILL_NONE ) ) )
                mbPageBckgrdDeleted = TRUE;

            // Ask, wether the setting are for the background-page or for the current page
            if( !mbMasterPage && bChanges )
            {
                // But don't ask in notice-view, because we can't change the background of
                // notice-masterpage (at the moment)
                if( ePageKind != PK_NOTES )
                {
                    String aTit(SdResId( STR_PAGE_BACKGROUND_TITLE ));
                    String aTxt(SdResId( STR_PAGE_BACKGROUND_TXT ));
                    MessBox aQuestionBox (
                        pParent,
                        WB_YES_NO | WB_DEF_YES,
                        aTit,
                        aTxt );
                    aQuestionBox.SetImage( QueryBox::GetStandardImage() );
                    mbMasterPage = ( RET_YES == aQuestionBox.Execute() );
                }

                if( mbPageBckgrdDeleted )
                {
                    mpBackgroundObjUndoAction = new SdBackgroundObjUndoAction( *mpDoc, *mpPage, mpPage->GetBackgroundObj() );
                    mpPage->SetBackgroundObj( NULL );

                    // #110094#-15
                    // tell the page that it's visualization has changed
                    mpPage->ActionChanged();
                }
            }

            // Sonderbehandlung: die INVALIDS auf NULL-Pointer
            // zurueckgesetzen (sonst landen INVALIDs oder
            // Pointer auf die DefaultItems in der Vorlage;
            // beides wuerde die Attribut-Vererbung unterbinden)
            pTempSet->ClearInvalidItems();

            if( mbMasterPage )
            {
                StyleSheetUndoAction* pAction = new StyleSheetUndoAction(mpDoc, (SfxStyleSheet*)pStyleSheet, &(*pTempSet.get()));
                mpDocSh->GetUndoManager()->AddUndoAction(pAction);
                pStyleSheet->GetItemSet().Put( *(pTempSet.get()) );
                pStyleSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
            }

            const SfxPoolItem *pItem;
            if( SFX_ITEM_SET == pTempSet->GetItemState( EE_PARA_WRITINGDIR, sal_False, &pItem ) )
            {
                sal_uInt32 nVal = ((SvxFrameDirectionItem*)pItem)->GetValue();
                mpDoc->SetDefaultWritingMode( nVal == FRMDIR_HORI_RIGHT_TOP ? ::com::sun::star::text::WritingMode_RL_TB : ::com::sun::star::text::WritingMode_LR_TB );
            }

            mpDoc->SetChanged(TRUE);

            SdrObject* pObj = mpPage->IsMasterPage() ?
                mpPage->GetPresObj( PRESOBJ_BACKGROUND ) :
                ((SdPage&)(mpPage->TRG_GetMasterPage())).GetPresObj( PRESOBJ_BACKGROUND );
            if( pObj )
            {
                // BackgroundObj: no hard attributes allowed
                SfxItemSet aSet( mpDoc->GetPool() );
                pObj->SetMergedItemSet(aSet);
            }
        }

        aNewAttr.Put(*(pTempSet.get()));
        mrReq.Done( aNewAttr );

        return mrReq.GetArgs();
    }
    else
    {
        return 0;
    }
}

void FuPage::ApplyItemSet( const SfxItemSet* pArgs )
{
    if( !pArgs )
        return;

    ///////////////////////////////////////////////////////////////////////////
    // Set new page-attributes
    PageKind ePageKind = mpDrawViewShell->GetPageKind();
    const SfxPoolItem*  pPoolItem;
    BOOL                bSetPageSizeAndBorder = FALSE;
    Size                aNewSize(maSize);
    INT32               nLeft  = -1, nRight = -1, nUpper = -1, nLower = -1;
    BOOL                bScaleAll = TRUE;
    Orientation         eOrientation = mpPage->GetOrientation();
    SdPage*             pMasterPage = mpPage->IsMasterPage() ? mpPage : &(SdPage&)(mpPage->TRG_GetMasterPage());
    BOOL                bFullSize = pMasterPage->IsBackgroundFullSize();
    USHORT              nPaperBin = mpPage->GetPaperBin();

    if( pArgs->GetItemState(SID_ATTR_PAGE, TRUE, &pPoolItem) == SFX_ITEM_SET )
    {
        mpDoc->SetPageNumType(((const SvxPageItem*) pPoolItem)->GetNumType());

        eOrientation = (((const SvxPageItem*) pPoolItem)->IsLandscape() == ORIENTATION_LANDSCAPE) ?
            ORIENTATION_LANDSCAPE : ORIENTATION_PORTRAIT;

        if( mpPage->GetOrientation() != eOrientation )
            bSetPageSizeAndBorder = TRUE;

        mpDrawViewShell->ResetActualPage();
    }

    if( pArgs->GetItemState(SID_ATTR_PAGE_SIZE, TRUE, &pPoolItem) == SFX_ITEM_SET )
    {
        aNewSize = ((const SvxSizeItem*) pPoolItem)->GetSize();

        if( mpPage->GetSize() != aNewSize )
            bSetPageSizeAndBorder = TRUE;
    }

    if( pArgs->GetItemState(mpDoc->GetPool().GetWhich(SID_ATTR_LRSPACE),
                            TRUE, &pPoolItem) == SFX_ITEM_SET )
    {
        nLeft = ((const SvxLRSpaceItem*) pPoolItem)->GetLeft();
        nRight = ((const SvxLRSpaceItem*) pPoolItem)->GetRight();

        if( mpPage->GetLftBorder() != nLeft || mpPage->GetRgtBorder() != nRight )
            bSetPageSizeAndBorder = TRUE;

    }

    if( pArgs->GetItemState(mpDoc->GetPool().GetWhich(SID_ATTR_ULSPACE),
                            TRUE, &pPoolItem) == SFX_ITEM_SET )
    {
        nUpper = ((const SvxULSpaceItem*) pPoolItem)->GetUpper();
        nLower = ((const SvxULSpaceItem*) pPoolItem)->GetLower();

        if( mpPage->GetUppBorder() != nUpper || mpPage->GetLwrBorder() != nLower )
            bSetPageSizeAndBorder = TRUE;
    }

    if( pArgs->GetItemState(mpDoc->GetPool().GetWhich(SID_ATTR_PAGE_EXT1), TRUE, &pPoolItem) == SFX_ITEM_SET )
    {
        bScaleAll = ((const SfxBoolItem*) pPoolItem)->GetValue();
    }

    if( pArgs->GetItemState(mpDoc->GetPool().GetWhich(SID_ATTR_PAGE_EXT2), TRUE, &pPoolItem) == SFX_ITEM_SET )
    {
        bFullSize = ((const SfxBoolItem*) pPoolItem)->GetValue();

        if(pMasterPage->IsBackgroundFullSize() != bFullSize )
            bSetPageSizeAndBorder = TRUE;
    }

    // Papierschacht (PaperBin)
    if( pArgs->GetItemState(mpDoc->GetPool().GetWhich(SID_ATTR_PAGE_PAPERBIN), TRUE, &pPoolItem) == SFX_ITEM_SET )
    {
        nPaperBin = ((const SvxPaperBinItem*) pPoolItem)->GetValue();

        if( mpPage->GetPaperBin() != nPaperBin )
            bSetPageSizeAndBorder = TRUE;
    }

    if (nLeft == -1 && nUpper != -1)
    {
        bSetPageSizeAndBorder = TRUE;
        nLeft  = mpPage->GetLftBorder();
        nRight = mpPage->GetRgtBorder();
    }
    else if (nLeft != -1 && nUpper == -1)
    {
        bSetPageSizeAndBorder = TRUE;
        nUpper = mpPage->GetUppBorder();
        nLower = mpPage->GetLwrBorder();
    }

    if( bSetPageSizeAndBorder || !mbMasterPage )
        mpDrawViewShell->SetPageSizeAndBorder(ePageKind, aNewSize, nLeft, nRight, nUpper, nLower, bScaleAll, eOrientation, nPaperBin, bFullSize );

    ////////////////////////////////////////////////////////////////////////////////
    //
    // if bMasterPage==FALSE then create a background-object for this page with the
    // properties set in the dialog before, but if mbPageBckgrdDeleted==TRUE then
    // the background of this page was set to invisible, so it would be a mistake
    // to create a new background-object for this page !
    //

    if( mbDisplayBackgroundTabPage )
    {
        if( !mbMasterPage && !mbPageBckgrdDeleted )
        {
            // Only this page
            SdrObject* pObj = mpPage->GetBackgroundObj();

            delete mpBackgroundObjUndoAction;
            mpBackgroundObjUndoAction = new SdBackgroundObjUndoAction( *mpDoc, *mpPage, pObj );

            if( !pObj )
            {
                pObj = new SdrRectObj();
                mpPage->SetBackgroundObj( pObj );
            }

            Point aPos ( nLeft, nUpper );
            Size aSize( mpPage->GetSize() );
            aSize.Width()  -= nLeft  + nRight - 1;
            aSize.Height() -= nUpper + nLower - 1;
            Rectangle aRect( aPos, aSize );
            pObj->SetLogicRect( aRect );
            pObj->SetMergedItemSet(*pArgs);

            // #110094#-15
            // tell the page that it's visualization has changed
            mpPage->ActionChanged();
        }
    }

    // add undo action for background object
    if( mpBackgroundObjUndoAction )
    {
        // set merge flag, because a SdUndoGroupAction could have been inserted before
        mpDocSh->GetUndoManager()->AddUndoAction( mpBackgroundObjUndoAction, TRUE );
        mpBackgroundObjUndoAction = 0;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // Objekte koennen max. so gross wie die ViewSize werden
    //
    Size aPageSize = mpDoc->GetSdPage(0, ePageKind)->GetSize();
    Size aViewSize = Size(aPageSize.Width() * 3, aPageSize.Height() * 2);
    mpDoc->SetMaxObjSize(aViewSize);

    ///////////////////////////////////////////////////////////////////////////
    //
    // ggfs. Preview den neuen Kontext mitteilen
    //
    mpDrawViewShell->UpdatePreview( mpDrawViewShell->GetActualPage() );
}

} // end of namespace sd
