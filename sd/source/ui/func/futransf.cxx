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


#include "futransf.hxx"

#include <svx/dialogs.hrc>
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>

#include "strings.hrc"
#include "ViewShell.hxx"
#include "View.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/svdocapt.hxx>

namespace sd {

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuTransform::FuTransform(ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
                         SdDrawDocument* pDoc, SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuTransform::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuTransform( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuTransform::DoExecute( SfxRequest& rReq )
{
    if( mpView->areSdrObjectsSelected() )
    {
        const SfxItemSet* pArgs = rReq.GetArgs();

        if( !pArgs )
        {
            // --------- itemset for size and position --------
            SfxItemSet aSet( mpView->GetGeoAttrFromMarked() );
            SdrCaptionObj* pSelected = dynamic_cast< SdrCaptionObj* >(mpView->getSelectedIfSingle());

            if( pSelected )
            {
                // --------- itemset for caption --------
                SfxItemSet aNewAttr( mpDoc->GetItemPool() );
                mpView->GetAttributes( aNewAttr );

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if ( pFact )
                {
                    std::auto_ptr< SfxAbstractTabDialog > pDlg( pFact->CreateCaptionDialog( NULL, mpView ) );

                    const sal_uInt16* pRange = pDlg->GetInputRanges( *aNewAttr.GetPool() );
                    SfxItemSet aCombSet( *aNewAttr.GetPool(), pRange );
                    aCombSet.Put( aNewAttr );
                    aCombSet.Put( aSet );
                    pDlg->SetInputSet( &aCombSet );

                    if( pDlg.get() && (pDlg->Execute() == RET_OK) )
                    {
                        rReq.Done( *( pDlg->GetOutputItemSet() ) );
                        pArgs = rReq.GetArgs();
                    }
                }
            }
            else
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    std::auto_ptr< SfxAbstractTabDialog > pDlg( pFact->CreateSvxTransformTabDialog( NULL, &aSet, mpView ) );
                    if( pDlg.get() && (pDlg->Execute() == RET_OK) )
                    {
                        rReq.Done( *( pDlg->GetOutputItemSet() ) );
                        pArgs = rReq.GetArgs();
                    }
                }
            }
        }

        if( pArgs )
        {
            // Undo
            const SdrObjectVector aSelection(mpView->getSelectedSdrObjectVectorFromSdrMarkView());
            String aString( getSelectionDescription(aSelection) );
            aString.Append( sal_Unicode(' ') );
            aString.Append( String( SdResId( STR_TRANSFORM ) ) );
            mpView->BegUndo( aString );

            mpView->SetGeoAttrToMarked( *pArgs );
            mpView->SetAttributes( *pArgs );
            mpView->EndUndo();
        }
    }
}

} // end of namespace sd
