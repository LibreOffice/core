/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


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
#include "copydlg.hrc"
namespace sd {

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

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
    if( mpView->areSdrObjectsSelected() )
    {
        // Undo
        const SdrObjectVector aInitialSelection(mpView->getSelectedSdrObjectVectorFromSdrMarkView());
        String aString(getSelectionDescription(aInitialSelection));
        aString.Append( sal_Unicode(' ') );
        aString.Append( String( SdResId( STR_UNDO_COPYOBJECTS ) ) );
        mpView->BegUndo( aString );

        const SfxItemSet* pArgs = rReq.GetArgs();

        if( !pArgs )
        {
            SfxItemSet aSet( mpViewShell->GetPool(),
                                ATTR_COPY_START, ATTR_COPY_END, 0 );

            // Farb-Attribut angeben
            SfxItemSet aAttr( mpDoc->GetItemPool() );
            mpView->GetAttributes( aAttr );
            const SfxPoolItem*  pPoolItem = NULL;

            if( SFX_ITEM_SET == aAttr.GetItemState( XATTR_FILLSTYLE, true, &pPoolItem ) )
            {
                XFillStyle eStyle = ( ( const XFillStyleItem* ) pPoolItem )->GetValue();

                if( eStyle == XFILL_SOLID &&
                    SFX_ITEM_SET == aAttr.GetItemState( XATTR_FILLCOLOR, true, &pPoolItem ) )
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
                AbstractCopyDlg* pDlg = pFact->CreateCopyDlg(NULL, aSet, mpDoc->GetColorTable(), mpView );
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
                        return; // Abbruch
                    }
                    delete( pDlg );
                }
            }
        }

        basegfx::B2DRange aRange;
        double lWidth(0.0);
        double lHeight(0.0);
        double lSizeX(0.0);
        double lSizeY(0.0);
        double lAngle(0.0);
        sal_uInt16 nNumber(0);
        Color               aStartColor, aEndColor;
        bool bColor(false);
        const SfxPoolItem* pPoolItem = 0;

        // Anzahl
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_NUMBER, true, &pPoolItem ) )
        {
            nNumber = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
        }

        // Verschiebung
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_MOVE_X, true, &pPoolItem ) )
        {
            lSizeX = ( ( const SfxInt32Item* ) pPoolItem )->GetValue();
        }

        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_MOVE_Y, true, &pPoolItem ) )
        {
            lSizeY = ( ( const SfxInt32Item* ) pPoolItem )->GetValue();
        }

        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_ANGLE, true, &pPoolItem ) )
        {
            const sal_Int32 nAngle(((const SfxInt32Item*)pPoolItem)->GetValue());
            lAngle = (((360 - nAngle) % 360) * F_PI) / 180.0;
        }

        // Verrgroesserung / Verkleinerung
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_WIDTH, true, &pPoolItem ) )
        {
            lWidth = ( ( const SfxInt32Item* ) pPoolItem )->GetValue();
        }

        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_HEIGHT, true, &pPoolItem ) )
        {
            lHeight = ( ( const SfxInt32Item* ) pPoolItem )->GetValue();
        }

        // Startfarbe / Endfarbe
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_START_COLOR, true, &pPoolItem ) )
        {
            aStartColor = ( ( const XColorItem* ) pPoolItem )->GetColorValue();
            bColor = true;
        }

        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_END_COLOR, true, &pPoolItem ) )
        {
            aEndColor = ( ( const XColorItem* ) pPoolItem )->GetColorValue();

            if( aStartColor == aEndColor )
                bColor = false;
        }
        else
        {
            bColor = false;
        }

        SfxProgress*    pProgress = NULL;
        bool            bWaiting = false;

        if( nNumber > 1 )
        {
            String aStr( SdResId( STR_OBJECTS ) );
            aStr.Append( sal_Unicode(' ') );
            aStr.Append( String( SdResId( STR_UNDO_COPYOBJECTS ) ) );

            pProgress = new SfxProgress( mpDocSh, aStr, nNumber );
            mpDocSh->SetWaitCursor( true );
            bWaiting = true;
        }

        // Anzahl moeglicher Kopien berechnen
        aRange = mpView->getMarkedObjectSnapRange();

        if( lWidth < 0.0 )
        {
            const double fTmp(aRange.getWidth() / -lWidth);
            nNumber = std::min((sal_uInt16)basegfx::fround(fTmp), nNumber);
        }

        if( lHeight < 0.0 )
        {
            const double fTmp(aRange.getHeight() / -lHeight);
            nNumber = std::min((sal_uInt16)basegfx::fround(fTmp), nNumber);
        }

        for( sal_uInt16 i(1); i <= nNumber; i++ )
        {
            if( pProgress )
            {
                pProgress->SetState( i );
            }

            aRange = mpView->getMarkedObjectSnapRange();

            if( ( 1 == i ) && bColor )
            {
                SfxItemSet aNewSet( mpViewShell->GetPool(), XATTR_FILLSTYLE, XATTR_FILLCOLOR, 0L );

                aNewSet.Put( XFillStyleItem( XFILL_SOLID ) );
                aNewSet.Put( XFillColorItem( String(), aStartColor ) );
                mpView->SetAttributes( aNewSet );
            }

            // make a copy of selected objects
            mpView->CopyMarkedObj();

            // get newly selected objects
            const SdrObjectVector aSelectedSdrObjects(mpView->getSelectedSdrObjectVectorFromSdrMarkView());
            sal_uInt32 j;

            // set protection flags at marked copies to null
            for(j = 0; j < aSelectedSdrObjects.size(); j++)
            {
                SdrObject* pObj = aSelectedSdrObjects[j];

                pObj->SetMoveProtect(false);
                pObj->SetResizeProtect(false);
            }

            if( mpView->IsResizeAllowed() )
            {
                const basegfx::B2DVector aScale(
                    (aRange.getWidth() + lWidth) / (basegfx::fTools::equalZero(aRange.getWidth()) ? 1.0 : aRange.getWidth()),
                    (aRange.getHeight() + lHeight) / (basegfx::fTools::equalZero(aRange.getHeight()) ? 1.0 : aRange.getHeight()));

                mpView->ResizeMarkedObj(aRange.getMinimum(), aScale);
            }

            if( mpView->IsRotateAllowed() )
            {
                mpView->RotateMarkedObj(aRange.getCenter(), lAngle);
            }

            if( mpView->IsMoveAllowed() )
            {
                mpView->MoveMarkedObj(basegfx::B2DVector(lSizeX, lSizeY));
            }

            // set protection flags at marked copies to original values
            if(aInitialSelection.size() == aSelectedSdrObjects.size())
            {
                for(j = 0; j < aInitialSelection.size(); j++)
                {
                    SdrObject* pSrcObj = aInitialSelection[j];
                    SdrObject* pDstObj = aSelectedSdrObjects[j];

                    if(pSrcObj->GetObjInventor() == pDstObj->GetObjInventor()
                        && pSrcObj->GetObjIdentifier() == pDstObj->GetObjIdentifier())
                    {
                        pDstObj->SetMoveProtect( pSrcObj->IsMoveProtect() );
                        pDstObj->SetResizeProtect( pSrcObj->IsResizeProtect() );
                    }
                }
            }

            if( bColor )
            {
                // Koennte man sicher noch optimieren, wuerde aber u.U.
                // zu Rundungsfehlern fuehren
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
        {
            delete pProgress;
        }

        if ( bWaiting )
        {
            mpDocSh->SetWaitCursor( false );
        }

        // adapt handles
        mpView->SetMarkHandles();

        mpView->EndUndo();
    }
}

} // end of namespace
