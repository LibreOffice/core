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

#include "fusnapln.hxx"
#include <svl/aeitem.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>


#include "strings.hrc"

#include "sdattr.hxx"
#include "View.hxx"
#include "ViewShell.hxx"
#include "DrawViewShell.hxx"
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "sdenumdef.hxx"
#include "sdresid.hxx"
#include "sdabstdlg.hxx"
#include "app.hrc"
#include <svx/svdpagv.hxx>

namespace sd {

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuSnapLine::FuSnapLine(ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
                       SdDrawDocument* pDoc, SfxRequest& rReq) :
    FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuSnapLine::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuSnapLine( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuSnapLine::DoExecute( SfxRequest& rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    sal_uInt32 nHelpLine(0);
    bool    bCreateNew = true;

    // Get index of snap line or snap point from the request.
    SFX_REQUEST_ARG (rReq, pHelpLineIndex, SfxUInt32Item, ID_VAL_INDEX );
    if (pHelpLineIndex != NULL)
    {
        nHelpLine = pHelpLineIndex->GetValue();
        // Reset the argument pointer to trigger the display of the dialog.
        pArgs = NULL;
    }

    if ( !pArgs )
    {
        SfxItemSet aNewAttr(mpViewShell->GetPool(), ATTR_SNAPLINE_START, ATTR_SNAPLINE_END);
        bool bLineExist (false);
        basegfx::B2DPoint aLinePos;

        if(mpView->GetSdrPageView())
        {
            if (pHelpLineIndex == NULL)
            {
                // The index of the snap line is not provided as argument to the
                // request.  Determine it from the mouse position.
                static_cast<DrawViewShell*>(mpViewShell)->SetMousePosFreezed( false );
                aLinePos = static_cast<DrawViewShell*>(mpViewShell)->GetMousePos();

                if(basegfx::fTools::moreOrEqual(aLinePos.getX(), 0.0))
                {
                    aLinePos = mpWindow->GetInverseViewTransformation() * aLinePos;
                    const double fHitLog(basegfx::B2DVector(mpWindow->GetInverseViewTransformation() * basegfx::B2DVector(HITPIX, 0.0)).getLength());
                    bLineExist = mpView->PickHelpLine(aLinePos, fHitLog, nHelpLine);

                    if(bLineExist)
                    {
                        aLinePos = (mpView->GetSdrPageView()->GetHelpLines())[nHelpLine].GetPos();
                        aLinePos -= mpView->GetSdrPageView()->GetPageOrigin();
                    }
                }
                else
                {
                    aLinePos = basegfx::B2DPoint(0.0, 0.0);
                }
            }
            else
            {
                aLinePos = (mpView->GetSdrPageView()->GetHelpLines())[nHelpLine].GetPos();
                aLinePos -= mpView->GetSdrPageView()->GetPageOrigin();
                bLineExist = true;
            }
        }

        aNewAttr.Put(SfxUInt32Item(ATTR_SNAPLINE_X, basegfx::fround(aLinePos.getX())));
        aNewAttr.Put(SfxUInt32Item(ATTR_SNAPLINE_Y, basegfx::fround(aLinePos.getY())));

        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        AbstractSdSnapLineDlg* pDlg = pFact ? pFact->CreateSdSnapLineDlg( NULL, aNewAttr, mpView ) : 0;
        if( pDlg )
        {
            if ( bLineExist ) // includes mpView->GetSdrPageView() != 0
            {
                pDlg->HideRadioGroup();

                const SdrHelpLine& rHelpLine = (mpView->GetSdrPageView()->GetHelpLines())[nHelpLine];

                if ( rHelpLine.GetKind() == SDRHELPLINE_POINT )
                {
                    pDlg->SetText(String(SdResId(STR_SNAPDLG_SETPOINT)));
                    pDlg->SetInputFields(true, true);
                }
                else
                {
                    pDlg->SetText(String(SdResId(STR_SNAPDLG_SETLINE)));

                    if ( rHelpLine.GetKind() == SDRHELPLINE_VERTICAL )
                        pDlg->SetInputFields(true, false);
                    else
                        pDlg->SetInputFields(false, true);
                }
                bCreateNew = false;
            }
            else
            {
                pDlg->HideDeleteBtn();
            }

            sal_uInt16 nResult = pDlg->Execute();

            pDlg->GetAttr(aNewAttr);
            delete pDlg;

            switch( nResult )
            {
                case RET_OK:
                    rReq.Done(aNewAttr);
                    pArgs = rReq.GetArgs();
                    break;

                case RET_SNAP_DELETE:
                    // Fangobjekt loeschen
                    if ( !bCreateNew && mpView->GetSdrPageView())
                    {
                        mpView->GetSdrPageView()->DeleteHelpLine(nHelpLine);
                    }
                    // und weiter wie bei default
                default:
                    return;
            }
        }
    }

    if(mpView->GetSdrPageView())
    {
        basegfx::B2DPoint aHlpPos;
        aHlpPos.setX(((const SfxUInt32Item&) pArgs->Get(ATTR_SNAPLINE_X)).GetValue());
        aHlpPos.setY(((const SfxUInt32Item&) pArgs->Get(ATTR_SNAPLINE_Y)).GetValue());
        aHlpPos += mpView->GetSdrPageView()->GetPageOrigin();

        if ( bCreateNew )
        {
            SdrHelpLineKind eKind;

            switch ( (SnapKind) ((const SfxAllEnumItem&)
                     pArgs->Get(ATTR_SNAPLINE_KIND)).GetValue() )
            {
                case SK_HORIZONTAL  : eKind = SDRHELPLINE_HORIZONTAL;   break;
                case SK_VERTICAL    : eKind = SDRHELPLINE_VERTICAL;     break;
                default             : eKind = SDRHELPLINE_POINT;        break;
            }

            mpView->GetSdrPageView()->InsertHelpLine(SdrHelpLine(eKind, aHlpPos));
        }
        else
        {
            const SdrHelpLine& rHelpLine = (mpView->GetSdrPageView()->GetHelpLines())[nHelpLine];

            mpView->GetSdrPageView()->SetHelpLine(nHelpLine, SdrHelpLine(rHelpLine.GetKind(), aHlpPos));
        }
    }
}

void FuSnapLine::Activate()
{
}

void FuSnapLine::Deactivate()
{
}

} // end of namespace sd
