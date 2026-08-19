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
#include <svl/intitem.hxx>

#include <sdattr.hrc>
#include <sdresid.hxx>
#include <strings.hrc>
#include <ViewShell.hxx>
#include <View.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <svx/svdobj.hxx>
#include <svx/xcolit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xdef.hxx>
#include <svx/xfillit0.hxx>
#include <svx/sdangitm.hxx>
#include <sfx2/request.hxx>
#include <sdabstdlg.hxx>
#include <sdenumdef.hxx>

#include <memory>

using namespace com::sun::star;

namespace sd {


FuCopy::FuCopy (
    ViewShell& rViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument& rDoc,
    SfxRequest& rReq)
    : FuPoor(rViewSh, pWin, pView, rDoc, rReq)
{
}

rtl::Reference<FuPoor> FuCopy::Create( ViewShell& rViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument& rDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuCopy( rViewSh, pWin, pView, rDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuCopy::DoExecute( SfxRequest& rReq )
{
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 0 )
        return;

    // Undo
    OUString aString = rMarkList.GetMarkDescription() +
        " " + SdResId( STR_UNDO_COPYOBJECTS );
    mpView->BegUndo( aString );

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        SfxItemSet aSet(SfxItemSet::makeFixedSfxItemSet<XATTR_FILLSTYLE, XATTR_FILLSTYLE, ATTR_COPY_START, ATTR_COPY_END>( mrViewShell.GetPool() ));

        // indicate color attribute
        SfxItemSet aAttr( mrDoc.GetPool() );
        mpView->GetAttributes( aAttr );

        if( const XFillStyleItem* pFillStyleItem = aAttr.GetItemIfSet( XATTR_FILLSTYLE ) )
        {
            drawing::FillStyle eStyle = pFillStyleItem->GetValue();

            const XFillColorItem* pFillColorItem;
            if( eStyle == drawing::FillStyle_SOLID &&
                (pFillColorItem = aAttr.GetItemIfSet( XATTR_FILLCOLOR )) )
            {
                XColorItem aXColorItem( ATTR_COPY_START_COLOR, pFillColorItem->GetName(),
                                                    pFillColorItem->GetColorValue() );
                aSet.Put(*pFillStyleItem);
                aSet.Put( aXColorItem );

            }
        }

        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractCopyDlg> pDlg(pFact->CreateCopyDlg(mrViewShell.GetFrameWeld(), aSet, mpView ));

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
    sal_Int32               lWidth = 0, lHeight = 0, lSizeX = 0, lSizeY = 0;
    Degree100           lAngle(0);
    sal_uInt16              nNumber = 0;
    Color               aStartColor, aEndColor;
    bool                bColor = false;
    ColorInterpolationKind eKind = ColorInterpolationKind::RGB;

    if (pArgs)
    {
        // Count
        if( const SfxUInt16Item* pPoolItem = pArgs->GetItemIfSet( ATTR_COPY_NUMBER ) )
            nNumber = pPoolItem->GetValue();

        // translation
        if( const SfxInt32Item* pPoolItem =  pArgs->GetItemIfSet( ATTR_COPY_MOVE_X ) )
            lSizeX = pPoolItem->GetValue();
        if( const SfxInt32Item* pPoolItem =  pArgs->GetItemIfSet( ATTR_COPY_MOVE_Y ) )
            lSizeY = pPoolItem->GetValue();
        if( const SdrAngleItem* pPoolItem =  pArgs->GetItemIfSet( ATTR_COPY_ANGLE ) )
            lAngle = pPoolItem->GetValue();

        // scale
        if( const SfxInt32Item* pPoolItem =  pArgs->GetItemIfSet( ATTR_COPY_WIDTH ) )
            lWidth = pPoolItem->GetValue();
        if( const SfxInt32Item* pPoolItem =  pArgs->GetItemIfSet( ATTR_COPY_HEIGHT ) )
            lHeight = pPoolItem->GetValue();

        // start/end color
        if( const XColorItem* pPoolItem =  pArgs->GetItemIfSet( ATTR_COPY_START_COLOR ) )
        {
            aStartColor = pPoolItem->GetColorValue();
            bColor = true;
        }
        if( const XColorItem* pPoolItem = pArgs->GetItemIfSet( ATTR_COPY_END_COLOR ) )
        {
            aEndColor = pPoolItem->GetColorValue();
            if( aStartColor == aEndColor )
                bColor = false;
        }

        if (const SfxUInt16Item* pPoolItem = pArgs->GetItemIfSet( ATTR_COPY_COLOR_TRANSKIND ))
        {
            eKind = static_cast<ColorInterpolationKind>(pPoolItem->GetValue());
        }

    }

    // remove handles
    //HMHmpView->HideMarkHdl();

    std::unique_ptr<SfxProgress> pProgress;
    bool            bWaiting = false;

    if( nNumber > 1 )
    {
        OUString aStr = SdResId( STR_OBJECTS ) +
            " " + SdResId( STR_UNDO_COPYOBJECTS );

        pProgress.reset(new SfxProgress( mpDocSh, aStr, nNumber ));
        mpDocSh->SetWaitCursor( true );
        bWaiting = true;
    }

    const size_t nMarkCount = rMarkList.GetMarkCount();
    SdrObject*          pObj = nullptr;

    // calculate number of possible copies
    aRect = mpView->GetAllMarkedRect();

    if( lWidth < 0 )
    {
        ::tools::Long nTmp = ( aRect.Right() - aRect.Left() ) / -lWidth;
        nNumber = static_cast<sal_uInt16>(std::min( nTmp, static_cast<::tools::Long>(nNumber) ));
    }

    if( lHeight < 0 )
    {
        ::tools::Long nTmp = ( aRect.Bottom() - aRect.Top() ) / -lHeight;
        nNumber = static_cast<sal_uInt16>(std::min( nTmp, static_cast<::tools::Long>(nNumber) ));
    }

    for( sal_uInt16 i = 1; i <= nNumber; i++ )
    {
        if( pProgress )
            pProgress->SetState( i );

        aRect = mpView->GetAllMarkedRect();

        if( ( 1 == i ) && bColor )
        {
            SfxItemSet aNewSet(SfxItemSet::makeFixedSfxItemSet<XATTR_FILLSTYLE, XATTR_FILLCOLOR>( mrViewShell.GetPool() ));
            aNewSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
            aNewSet.Put( XFillColorItem( OUString(), aStartColor ) );
            mpView->SetAttributes( aNewSet );
        }

        // make a copy of selected objects
        mpView->CopyMarked();

        // get newly selected objects
        SdrMarkList aCopyMarkList( rMarkList );
        const size_t nCopyMarkCount = rMarkList.GetMarkCount();

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

        // vertical/horizontal lines have zero width/height; leave that axis
        // unscaled to avoid 0/0=NaN scale propagating into ResizeRect.
        const tools::Long nRectW = aRect.Right() - aRect.Left();
        const tools::Long nRectH = aRect.Bottom() - aRect.Top();
        double aWidth  = nRectW ? double(nRectW + lWidth)  / nRectW : 1.0;
        double aHeight = nRectH ? double(nRectH + lHeight) / nRectH : 1.0;

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
                SdrObject* pSrcObj = rMarkList.GetMark( j )->GetMarkedSdrObj();
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
            basegfx::BColor aStartBColor(aStartColor.getBColor());
            basegfx::BColor aEndBColor(aEndColor.getBColor());
            basegfx::BColor aNewBColor;
            switch(eKind)
            {
                case ColorInterpolationKind::RGB:
                    aNewBColor = basegfx::interpolate(aStartBColor, aEndBColor,
                                                      static_cast<double>(i) / nNumber);
                    break;
                case ColorInterpolationKind::HSLInc:
                    aNewBColor = basegfx::utils::interpolateInHSL(
                        aStartBColor, aEndBColor, static_cast<double>(i) / nNumber, true);
                break;
                case ColorInterpolationKind::HSLDec:
                    aNewBColor = basegfx::utils::interpolateInHSL(
                        aStartBColor, aEndBColor, static_cast<double>(i) / nNumber, false);
                break;
            }
            Color aNewColor(aNewBColor);
            SfxItemSet aNewSet(SfxItemSet::makeFixedSfxItemSet<XATTR_FILLSTYLE, XATTR_FILLCOLOR>( mrViewShell.GetPool() ));
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
