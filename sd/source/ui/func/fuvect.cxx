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


#include "fuvect.hxx"
#include <tools/poly.hxx>
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#include <vcl/msgbox.hxx>
#ifndef _SVX_SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVX_SVDEDTV_HXX //autogen
#include <svx/svdedtv.hxx>
#endif


#include "View.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"
#include "strings.hrc"
#include "sdresid.hxx"
#include "sdabstdlg.hxx"

namespace sd
{

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuVectorize::FuVectorize (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor (pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuVectorize::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuVectorize( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuVectorize::DoExecute( SfxRequest& )
{
    SdrGrafObj* pObj = dynamic_cast< SdrGrafObj* >(mpView->getSelectedIfSingle());

    if( pObj )
    {
        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        AbstractSdVectorizeDlg* pDlg = pFact ? pFact->CreateSdVectorizeDlg( mpWindow, pObj->GetGraphic().GetBitmap(), mpDocSh ) : 0;
        if( pDlg && pDlg->Execute() == RET_OK )
        {
            const GDIMetaFile&  rMtf = pDlg->GetGDIMetaFile();

            if( rMtf.GetActionCount() )
            {
                SdrGrafObj* pVectObj = static_cast< SdrGrafObj* >(pObj->CloneSdrObject());
                String      aStr;

                pObj->TakeObjNameSingul(aStr);
                    aStr.Append( sal_Unicode(' ') );
                    aStr.Append( String( SdResId( STR_UNDO_VECTORIZE ) ) );
                    mpView->BegUndo( aStr );
                    pVectObj->SetGraphic( rMtf );
                mpView->ReplaceObjectAtView( *pObj, *pVectObj );
                    mpView->EndUndo();
            }
        }
        delete pDlg;
    }
}

} // end of namespace sd
