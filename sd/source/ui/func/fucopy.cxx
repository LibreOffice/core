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

#include <fucopy.hxx>
#include <sfx2/progress.hxx>
#include <svx/svxids.hrc>

#include <sdresid.hxx>
#include <sdattr.hxx>
#include <strings.hrc>
#include <ViewShell.hxx>
#include <View.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <vcl/wrkwin.hxx>
#include <svx/svdobj.hxx>
#include <sfx2/app.hxx>
#include <svx/xcolit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xdef.hxx>
#include <svx/xfillit0.hxx>
#include <sfx2/request.hxx>
#include <sdabstdlg.hxx>
#include <memory>

using namespace com::sun::star;

namespace sd {


FuCopy::FuCopy (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuCopy::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuCopy( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuCopy::DoExecute( SfxRequest& rReq )
{
    if( !mpView->AreObjectsMarked() )
        return;

    // Undo
    OUString aString( mpView->GetDescriptionOfMarkedObjects() );
    aString += " " + SdResId( STR_UNDO_COPYOBJECTS );
    mpView->BegUndo( aString );

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        SfxItemSet aSet( mpViewShell->GetPool(),
                            svl::Items<ATTR_COPY_START, ATTR_COPY_END>{} );

        // indicate color attribute
        SfxItemSet aAttr( mpDoc->GetPool() );
        mpView->GetAttributes( aAttr );
        const SfxPoolItem*  pPoolItem = nullptr;

        if( SfxItemState::SET == aAttr.GetItemState( XATTR_FILLSTYLE, true, &pPoolItem ) )
        {
            drawing::FillStyle eStyle = static_cast<const XFillStyleItem*>(pPoolItem)->GetValue();

            if( eStyle == drawing::FillStyle_SOLID &&
                SfxItemState::SET == aAttr.GetItemState( XATTR_FILLCOLOR, true, &pPoolItem ) )
            {
                const XFillColorItem* pItem = static_cast<const XFillColorItem*>(pPoolItem);
                XColorItem aXColorItem( ATTR_COPY_START_COLOR, pItem->GetName(),
                                                    pItem->GetColorValue() );
                aSet.Put( aXColorItem );

            }
        }

        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractCopyDlg> pDlg(pFact->CreateCopyDlg(mpViewShell->GetFrameWeld(), aSet, mpView ));

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
                pDlg.disposeAndClear();
                mpView->EndUndo();
                return; // Cancel
            }
        }
    }

    ::tools::Rectangle           aRect;
    sal_Int32               lWidth = 0, lHeight = 0, lSizeX = 0, lSizeY = 0, lAngle = 0;
    sal_uInt16              nNumber = 0;
    Color               aStartColor, aEndColor;
    bool                bColor = false;
    const SfxPoolItem*  pPoolItem = nullptr;

    // Count
    if( pArgs && SfxItemState::SET == pArgs->GetItemState( ATTR_COPY_NUMBER, true, &pPoolItem ) )
        nNumber = static_cast<const SfxUInt16Item*>( pPoolItem )->GetValue();

    // translation
    if( pArgs && SfxItemState::SET == pArgs->GetItemState( ATTR_COPY_MOVE_X, true, &pPoolItem ) )
        lSizeX = static_cast<const SfxInt32Item*>( pPoolItem )->GetValue();
    if( pArgs && SfxItemState::SET == pArgs->GetItemState( ATTR_COPY_MOVE_Y, true, &pPoolItem ) )
        lSizeY = static_cast<const SfxInt32Item*>( pPoolItem )->GetValue();
    if( pArgs && SfxItemState::SET == pArgs->GetItemState( ATTR_COPY_ANGLE, true, &pPoolItem ) )
        lAngle = static_cast<const SfxInt32Item*>( pPoolItem )->GetValue();

    // scale
    if( pArgs && SfxItemState::SET == pArgs->GetItemState( ATTR_COPY_WIDTH, true, &pPoolItem ) )
        lWidth = static_cast<const SfxInt32Item*>( pPoolItem )->GetValue();
    if( pArgs && SfxItemState::SET == pArgs->GetItemState( ATTR_COPY_HEIGHT, true, &pPoolItem ) )
        lHeight = static_cast<const SfxInt32Item*>( pPoolItem )->GetValue();

    // start/end color
    if( pArgs && SfxItemState::SET == pArgs->GetItemState( ATTR_COPY_START_COLOR, true, &pPoolItem ) )
    {
        aStartColor = static_cast<const XColorItem*>( pPoolItem )->GetColorValue();
        bColor = true;
    }
    if( pArgs && SfxItemState::SET == pArgs->GetItemState( ATTR_COPY_END_COLOR, true, &pPoolItem ) )
    {
        aEndColor = static_cast<const XColorItem*>( pPoolItem )->GetColorValue();
        if( aStartColor == aEndColor )
            bColor = false;
    }
    else
        bColor = false;

    // remove handles
    //HMHmpView->HideMarkHdl();

    std::unique_ptr<SfxProgress> pProgress;
    bool            bWaiting = false;

    if( nNumber > 1 )
    {
        OUString aStr( SdResId( STR_OBJECTS ) );
        aStr += " " + SdResId( STR_UNDO_COPYOBJECTS );

        pProgress.reset(new SfxProgress( mpDocSh, aStr, nNumber ));
        mpDocSh->SetWaitCursor( true );
        bWaiting = true;
    }

    const SdrMarkList   aMarkList( mpView->GetMarkedObjectList() );
    const size_t nMarkCount = aMarkList.GetMarkCount();
    SdrObject*          pObj = nullptr;

    // calculate number of possible copies
    aRect = mpView->GetAllMarkedRect();

    if( lWidth < 0 )
    {
        long nTmp = ( aRect.Right() - aRect.Left() ) / -lWidth;
        nNumber = static_cast<sal_uInt16>(std::min( nTmp, static_cast<long>(nNumber) ));
    }

    if( lHeight < 0 )
    {
        long nTmp = ( aRect.Bottom() - aRect.Top() ) / -lHeight;
        nNumber = static_cast<sal_uInt16>(std::min( nTmp, static_cast<long>(nNumber) ));
    }

    for( sal_uInt16 i = 1; i <= nNumber; i++ )
    {
        if( pProgress )
            pProgress->SetState( i );

        aRect = mpView->GetAllMarkedRect();

        if( ( 1 == i ) && bColor )
        {
            SfxItemSet aNewSet( mpViewShell->GetPool(), svl::Items<XATTR_FILLSTYLE, XATTR_FILLCOLOR>{} );
            aNewSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
            aNewSet.Put( XFillColorItem( OUString(), aStartColor ) );
            mpView->SetAttributes( aNewSet );
        }

        // make a copy of selected objects
        mpView->CopyMarked();

        // get newly selected objects
        SdrMarkList aCopyMarkList( mpView->GetMarkedObjectList() );
        const size_t nCopyMarkCount = aMarkList.GetMarkCount();

        // set protection flags at marked copies to null
        for( size_t j = 0; j < nCopyMarkCount; ++j )
        {
            pObj = aCopyMarkList.GetMark( j )->GetMarkedSdrObj();

            if( pObj )
            {
                pObj->SetMoveProtect( false );
                pObj->SetResizeProtect( false );
            }
        }

        Fraction aWidth( aRect.Right() - aRect.Left() + lWidth, aRect.Right() - aRect.Left() );
        Fraction aHeight( aRect.Bottom() - aRect.Top() + lHeight, aRect.Bottom() - aRect.Top() );

        if( mpView->IsResizeAllowed() )
            mpView->ResizeAllMarked( aRect.TopLeft(), aWidth, aHeight );

        if( mpView->IsRotateAllowed() )
            mpView->RotateAllMarked( aRect.Center(), lAngle );

        if( mpView->IsMoveAllowed() )
            mpView->MoveAllMarked( Size( lSizeX, lSizeY ) );

        // set protection flags at marked copies to original values
        if( nMarkCount == nCopyMarkCount )
        {
            for( size_t j = 0; j < nMarkCount; ++j )
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
            sal_uInt8 nRed = aStartColor.GetRed() + static_cast<sal_uInt8>( ( static_cast<long>(aEndColor.GetRed()) - static_cast<long>(aStartColor.GetRed()) ) * static_cast<long>(i) / static_cast<long>(nNumber)  );
            sal_uInt8 nGreen = aStartColor.GetGreen() + static_cast<sal_uInt8>( ( static_cast<long>(aEndColor.GetGreen()) - static_cast<long>(aStartColor.GetGreen()) ) *  static_cast<long>(i) / static_cast<long>(nNumber) );
            sal_uInt8 nBlue = aStartColor.GetBlue() + static_cast<sal_uInt8>( ( static_cast<long>(aEndColor.GetBlue()) - static_cast<long>(aStartColor.GetBlue()) ) * static_cast<long>(i) / static_cast<long>(nNumber) );
            Color aNewColor( nRed, nGreen, nBlue );
            SfxItemSet aNewSet( mpViewShell->GetPool(), svl::Items<XATTR_FILLSTYLE, XATTR_FILLCOLOR>{} );
            aNewSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
            aNewSet.Put( XFillColorItem( OUString(), aNewColor ) );
            mpView->SetAttributes( aNewSet );
        }
    }

    pProgress.reset();

    if ( bWaiting )
        mpDocSh->SetWaitCursor( false );

    // show handles
    mpView->AdjustMarkHdl(); //HMH sal_True );
    //HMHpView->ShowMarkHdl();

    mpView->EndUndo();
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
