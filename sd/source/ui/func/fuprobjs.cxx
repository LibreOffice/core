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

#include "fuprobjs.hxx"

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <svl/style.hxx>
#include <editeng/outliner.hxx>
#include <svl/smplhint.hxx>


#include "app.hrc"
#include "res_bmp.hrc"
#include "strings.hrc"
#include "glob.hrc"
#include "prltempl.hrc"

#include "sdresid.hxx"
#include "drawdoc.hxx"
#ifndef SD_OUTLINE_VIEW_SHELL_HX
#include "OutlineViewShell.hxx"
#endif
#include "ViewShell.hxx"
#include "Window.hxx"
#include "glob.hxx"
#include "prlayout.hxx"
#include "unchss.hxx"
#include "sdabstdlg.hxx"
namespace sd {

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuPresentationObjects::FuPresentationObjects (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
     : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuPresentationObjects::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuPresentationObjects( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuPresentationObjects::DoExecute( SfxRequest& )
{
    OutlineViewShell* pOutlineViewShell = dynamic_cast< OutlineViewShell* >( mpViewShell );
    DBG_ASSERT( pOutlineViewShell, "sd::FuPresentationObjects::DoExecute(), does not work without an OutlineViewShell!");
    if( !pOutlineViewShell )
        return;

    // ergibt die Selektion ein eindeutiges Praesentationslayout?
    // wenn nicht, duerfen die Vorlagen nicht bearbeitet werden
    SfxItemSet aSet(mpDoc->GetItemPool(), SID_STATUS_LAYOUT, SID_STATUS_LAYOUT);
    pOutlineViewShell->GetStatusBarState( aSet );
    String aLayoutName = (((SfxStringItem&)aSet.Get(SID_STATUS_LAYOUT)).GetValue());
    DBG_ASSERT(aLayoutName.Len(), "Layout unbestimmt");

    bool    bUnique = false;
    sal_Int16   nDepth, nTmp;
    OutlineView* pOlView = static_cast<OutlineView*>(pOutlineViewShell->GetView());
    OutlinerView* pOutlinerView = pOlView->GetViewByWindow( (Window*) mpWindow );
    ::Outliner* pOutl = pOutlinerView->GetOutliner();
    List* pList = pOutlinerView->CreateSelectionList();
    Paragraph* pPara = (Paragraph*)pList->First();
    nDepth = pOutl->GetDepth((sal_uInt16)pOutl->GetAbsPos( pPara ) );
    bool bPage = pOutl->HasParaFlag( pPara, PARAFLAG_ISPAGE );

    while( pPara )
    {
        nTmp = pOutl->GetDepth((sal_uInt16) pOutl->GetAbsPos( pPara ) );

        if( nDepth != nTmp )
        {
            bUnique = false;
            break;
        }

        if( pOutl->HasParaFlag( pPara, PARAFLAG_ISPAGE ) != bPage )
        {
            bUnique = false;
            break;
        }
        bUnique = true;

        pPara = (Paragraph*) pList->Next();
    }

    if( bUnique )
    {
        String aStyleName = aLayoutName;
        aStyleName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ) );
        sal_uInt16 nDlgId = TAB_PRES_LAYOUT_TEMPLATE;
        PresentationObjects ePO;

        if( bPage )
        {
            ePO = PO_TITLE;
            String aStr(SdResId( STR_LAYOUT_TITLE ));
            aStyleName.Append( aStr );
        }
        else
        {
            ePO = (PresentationObjects) ( PO_OUTLINE_1 + nDepth - 1 );
            String aStr(SdResId( STR_LAYOUT_OUTLINE ));
            aStyleName.Append( aStr );
            aStyleName.Append( sal_Unicode(' ') );
            aStyleName.Append( UniString::CreateFromInt32( nDepth ) );
        }

        SfxStyleSheetBasePool* pStyleSheetPool = mpDocSh->GetStyleSheetPool();
        SfxStyleSheet* pStyleSheet = dynamic_cast< SfxStyleSheet* >(pStyleSheetPool->Find(aStyleName, SD_STYLE_FAMILY_MASTERPAGE));
        OSL_ENSURE(pStyleSheet, "StyleSheet not found or not based on SfxStyleSheet");

        if( pStyleSheet )
        {
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            SfxAbstractTabDialog* pDlg = pFact ? pFact->CreateSdPresLayoutTemplateDlg( mpDocSh, NULL, SdResId( nDlgId ), *pStyleSheet, ePO, pStyleSheetPool ) : 0;
            if( pDlg && (pDlg->Execute() == RET_OK) )
            {
                const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                // Undo-Action
                StyleSheetUndoAction* pAction = new StyleSheetUndoAction(*mpDoc, *pStyleSheet, *pOutSet);

                mpDocSh->GetUndoManager()->AddUndoAction(pAction);
                pStyleSheet->GetItemSet().Put( *pOutSet );
                pStyleSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
            }
            delete( pDlg );
        }
    }
}

} // end of namespace sd
