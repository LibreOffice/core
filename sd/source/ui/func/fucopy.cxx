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

#include "fucopy.hxx"
#include <sfx2/progress.hxx>
#include <svx/svxids.hrc>

#include "sdresid.hxx"
#include "sdattr.hxx"
#include "strings.hrc"
#include "ViewShell.hxx"
#include "View.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include <vcl/wrkwin.hxx>
#include <svx/svdobj.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>
#include <svx/xcolit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xdef.hxx>
#include <svx/xfillit0.hxx>
#include <sfx2/request.hxx>
#include "sdabstdlg.hxx"
namespace sd {

TYPEINIT1( FuCopy, FuPoor );


FuCopy::FuCopy (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuCopy::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuCopy( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuCopy::DoExecute( SfxRequest& rReq )
{
    if( mpView->AreObjectsMarked() )
    {
        // Undo
        String aString( mpView->GetDescriptionOfMarkedObjects() );
        aString.Append( sal_Unicode(' ') );
        aString.Append( SD_RESSTR( STR_UNDO_COPYOBJECTS ) );
        mpView->BegUndo( aString );

        const SfxItemSet* pArgs = rReq.GetArgs();

        if( !pArgs )
        {
            SfxItemSet aSet( mpViewShell->GetPool(),
                                ATTR_COPY_START, ATTR_COPY_END, 0 );

            // indicate color attribute
            SfxItemSet aAttr( mpDoc->GetPool() );
            mpView->GetAttributes( aAttr );
            const SfxPoolItem*  pPoolItem = NULL;

            if( SFX_ITEM_SET == aAttr.GetItemState( XATTR_FILLSTYLE, sal_True, &pPoolItem ) )
            {
                XFillStyle eStyle = ( ( const XFillStyleItem* ) pPoolItem )->GetValue();

                if( eStyle == XFILL_SOLID &&
                    SFX_ITEM_SET == aAttr.GetItemState( XATTR_FILLCOLOR, sal_True, &pPoolItem ) )
                {
                    const XFillColorItem* pItem = ( const XFillColorItem* ) pPoolItem;
                    XColorItem aXColorItem( ATTR_COPY_START_COLOR, pItem->GetName(),
                                                        pItem->GetColorValue() );
                    aSet.Put( aXColorItem );

                }
            }

            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            if( pFact )
            {
                AbstractCopyDlg* pDlg = pFact->CreateCopyDlg(NULL, aSet, mpDoc->GetColorList(), mpView );
                if( pDlg )
                {
                    sal_uInt16 nResult = pDlg->Execute();

                    switch( nResult )
                    {
                        case RET_OK:
                            pDlg->GetAttr( aSet );
                            rReq.Done( aSet );
                            pArgs = rReq.GetArgs();
                        break;

                        default:
                        {
                            delete pDlg;
                            mpView->EndUndo();
                        }
                        return; // Cancel
                    }
                    delete( pDlg );
                }
            }
        }

        Rectangle           aRect;
        sal_Int32               lWidth = 0, lHeight = 0, lSizeX = 0L, lSizeY = 0L, lAngle = 0L;
        sal_uInt16              nNumber = 0;
        Color               aStartColor, aEndColor;
        sal_Bool                bColor = sal_False;
        const SfxPoolItem*  pPoolItem = NULL;

        // Count
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_NUMBER, sal_True, &pPoolItem ) )
            nNumber = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();

        // translation
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_MOVE_X, sal_True, &pPoolItem ) )
            lSizeX = ( ( const SfxInt32Item* ) pPoolItem )->GetValue();
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_MOVE_Y, sal_True, &pPoolItem ) )
            lSizeY = ( ( const SfxInt32Item* ) pPoolItem )->GetValue();
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_ANGLE, sal_True, &pPoolItem ) )
            lAngle = ( ( const SfxInt32Item* )pPoolItem )->GetValue();

        // scale
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_WIDTH, sal_True, &pPoolItem ) )
            lWidth = ( ( const SfxInt32Item* ) pPoolItem )->GetValue();
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_HEIGHT, sal_True, &pPoolItem ) )
            lHeight = ( ( const SfxInt32Item* ) pPoolItem )->GetValue();

        // start/end color
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_START_COLOR, sal_True, &pPoolItem ) )
        {
            aStartColor = ( ( const XColorItem* ) pPoolItem )->GetColorValue();
            bColor = sal_True;
        }
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_END_COLOR, sal_True, &pPoolItem ) )
        {
            aEndColor = ( ( const XColorItem* ) pPoolItem )->GetColorValue();
            if( aStartColor == aEndColor )
                bColor = sal_False;
        }
        else
            bColor = sal_False;

        // remove handles
        //HMHmpView->HideMarkHdl();

        SfxProgress*    pProgress = NULL;
        sal_Bool            bWaiting = sal_False;

        if( nNumber > 1 )
        {
            String aStr( SdResId( STR_OBJECTS ) );
            aStr.Append( sal_Unicode(' ') );
            aStr.Append( SD_RESSTR( STR_UNDO_COPYOBJECTS ) );

            pProgress = new SfxProgress( mpDocSh, aStr, nNumber );
            mpDocSh->SetWaitCursor( sal_True );
            bWaiting = sal_True;
        }

        const SdrMarkList   aMarkList( mpView->GetMarkedObjectList() );
        const sal_uLong         nMarkCount = aMarkList.GetMarkCount();
        SdrObject*          pObj = NULL;

        // calculate number of possible copies
        aRect = mpView->GetAllMarkedRect();

        if( lWidth < 0L )
        {
            long nTmp = ( aRect.Right() - aRect.Left() ) / -lWidth;
            nNumber = (sal_uInt16) std::min( nTmp, (long)nNumber );
        }

        if( lHeight < 0L )
        {
            long nTmp = ( aRect.Bottom() - aRect.Top() ) / -lHeight;
            nNumber = (sal_uInt16) std::min( nTmp, (long)nNumber );
        }

        for( sal_uInt16 i = 1; i <= nNumber; i++ )
        {
            if( pProgress )
                pProgress->SetState( i );

            aRect = mpView->GetAllMarkedRect();

            if( ( 1 == i ) && bColor )
            {
                SfxItemSet aNewSet( mpViewShell->GetPool(), XATTR_FILLSTYLE, XATTR_FILLCOLOR, 0L );
                aNewSet.Put( XFillStyleItem( XFILL_SOLID ) );
                aNewSet.Put( XFillColorItem( String(), aStartColor ) );
                mpView->SetAttributes( aNewSet );
            }

            // make a copy of selected objects
            mpView->CopyMarked();

            // get newly selected objects
            SdrMarkList aCopyMarkList( mpView->GetMarkedObjectList() );
            sal_uLong       j, nCopyMarkCount = aMarkList.GetMarkCount();

            // set protection flags at marked copies to null
            for( j = 0; j < nCopyMarkCount; j++ )
            {
                pObj = aCopyMarkList.GetMark( j )->GetMarkedSdrObj();

                if( pObj )
                {
                    pObj->SetMoveProtect( sal_False );
                    pObj->SetResizeProtect( sal_False );
                }
            }

            Fraction aWidth( aRect.Right() - aRect.Left() + lWidth, aRect.Right() - aRect.Left() );
            Fraction aHeight( aRect.Bottom() - aRect.Top() + lHeight, aRect.Bottom() - aRect.Top() );

            if( mpView->IsResizeAllowed() )
                mpView->ResizeAllMarked( aRect.TopLeft(), aWidth, aHeight );

            if( mpView->IsRotateAllowed() )
                mpView->RotateAllMarked( aRect.Center(), lAngle * 100 );

            if( mpView->IsMoveAllowed() )
                mpView->MoveAllMarked( Size( lSizeX, lSizeY ) );

            // set protection flags at marked copies to original values
            if( nMarkCount == nCopyMarkCount )
            {
                for( j = 0; j < nMarkCount; j++ )
                {
                    SdrObject* pSrcObj = aMarkList.GetMark( j )->GetMarkedSdrObj();
                    SdrObject* pDstObj = aCopyMarkList.GetMark( j )->GetMarkedSdrObj();

                    if( pSrcObj && pDstObj &&
                        ( pSrcObj->GetObjInventor() == pDstObj->GetObjInventor() ) &&
                        ( pSrcObj->GetObjIdentifier() == pDstObj->GetObjIdentifier() ) )
                    {
                        pDstObj->SetMoveProtect( pSrcObj->IsMoveProtect() );
                        pDstObj->SetResizeProtect( pSrcObj->IsResizeProtect() );
                    }
                }
            }

            if( bColor )
            {
                // probably room for optimizations, but may can lead to rounding errors
                sal_uInt8 nRed = aStartColor.GetRed() + (sal_uInt8) ( ( (long) aEndColor.GetRed() - (long) aStartColor.GetRed() ) * (long) i / (long) nNumber  );
                sal_uInt8 nGreen = aStartColor.GetGreen() + (sal_uInt8) ( ( (long) aEndColor.GetGreen() - (long) aStartColor.GetGreen() ) *  (long) i / (long) nNumber );
                sal_uInt8 nBlue = aStartColor.GetBlue() + (sal_uInt8) ( ( (long) aEndColor.GetBlue() - (long) aStartColor.GetBlue() ) * (long) i / (long) nNumber );
                Color aNewColor( nRed, nGreen, nBlue );
                SfxItemSet aNewSet( mpViewShell->GetPool(), XATTR_FILLSTYLE, XATTR_FILLCOLOR, 0L );
                aNewSet.Put( XFillStyleItem( XFILL_SOLID ) );
                aNewSet.Put( XFillColorItem( String(), aNewColor ) );
                mpView->SetAttributes( aNewSet );
            }
        }

        if ( pProgress )
            delete pProgress;

        if ( bWaiting )
            mpDocSh->SetWaitCursor( sal_False );

        // show handles
        mpView->AdjustMarkHdl(); //HMH sal_True );
        //HMHpView->ShowMarkHdl();

        mpView->EndUndo();
    }
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
