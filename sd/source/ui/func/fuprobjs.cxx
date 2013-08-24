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


#include "fuprobjs.hxx"

#include <vcl/msgbox.hxx>
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
#include "OutlineViewShell.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"
#include "glob.hxx"
#include "prlayout.hxx"
#include "unchss.hxx"
#include "sdabstdlg.hxx"
namespace sd {

TYPEINIT1( FuPresentationObjects, FuPoor );


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

    /* does the selections end in a unique presentation layout?
       if not, it is not allowed to edit the templates */
    SfxItemSet aSet(mpDoc->GetItemPool(), SID_STATUS_LAYOUT, SID_STATUS_LAYOUT);
    pOutlineViewShell->GetStatusBarState( aSet );
    OUString aLayoutName = ((SfxStringItem&)aSet.Get(SID_STATUS_LAYOUT)).GetValue();
    DBG_ASSERT(!aLayoutName.isEmpty(), "Layout not defined");

    sal_Bool    bUnique = sal_False;
    sal_Int16   nDepth, nTmp;
    OutlineView* pOlView = static_cast<OutlineView*>(pOutlineViewShell->GetView());
    OutlinerView* pOutlinerView = pOlView->GetViewByWindow( (Window*) mpWindow );
    ::Outliner* pOutl = pOutlinerView->GetOutliner();

    std::vector<Paragraph*> aSelList;
    pOutlinerView->CreateSelectionList(aSelList);

    std::vector<Paragraph*>::const_iterator iter = aSelList.begin();
    Paragraph* pPara = aSelList.empty() ? NULL : *iter;

    nDepth = pOutl->GetDepth(pOutl->GetAbsPos( pPara ) );
    bool bPage = pOutl->HasParaFlag( pPara, PARAFLAG_ISPAGE );

    while( iter != aSelList.end() )
    {
        pPara = *iter;

        nTmp = pOutl->GetDepth( pOutl->GetAbsPos( pPara ) );

        if( nDepth != nTmp )
        {
            bUnique = sal_False;
            break;
        }

        if( pOutl->HasParaFlag( pPara, PARAFLAG_ISPAGE ) != bPage )
        {
            bUnique = sal_False;
            break;
        }
        bUnique = sal_True;
        ++iter;
    }

    if( bUnique )
    {
        OUString aStyleName = aLayoutName;
        aStyleName += SD_LT_SEPARATOR ;
        sal_uInt16 nDlgId = TAB_PRES_LAYOUT_TEMPLATE;
        PresentationObjects ePO;

        if( bPage )
        {
            ePO = PO_TITLE;
            aStyleName += SD_RESSTR(STR_LAYOUT_TITLE);
        }
        else
        {
            ePO = (PresentationObjects) ( PO_OUTLINE_1 + nDepth - 1 );
            aStyleName += SD_RESSTR(STR_LAYOUT_OUTLINE);
            aStyleName += OUString(' ') ;
            aStyleName += OUString::number( nDepth ) ;
        }

        SfxStyleSheetBasePool* pStyleSheetPool = mpDocSh->GetStyleSheetPool();
        SfxStyleSheetBase* pStyleSheet = pStyleSheetPool->Find( aStyleName, SD_STYLE_FAMILY_MASTERPAGE );
        DBG_ASSERT(pStyleSheet, "StyleSheet missing");

        if( pStyleSheet )
        {
            SfxStyleSheetBase& rStyleSheet = *pStyleSheet;

            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            SfxAbstractTabDialog* pDlg = pFact ? pFact->CreateSdPresLayoutTemplateDlg( mpDocSh, NULL, SdResId( nDlgId ), rStyleSheet, ePO, pStyleSheetPool ) : 0;
            if( pDlg && (pDlg->Execute() == RET_OK) )
            {
                const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                // Undo-Action
                StyleSheetUndoAction* pAction = new StyleSheetUndoAction
                                                (mpDoc, (SfxStyleSheet*)pStyleSheet,
                                                    pOutSet);
                mpDocSh->GetUndoManager()->AddUndoAction(pAction);

                pStyleSheet->GetItemSet().Put( *pOutSet );
                ( (SfxStyleSheet*) pStyleSheet )->Broadcast( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
            }
            delete( pDlg );
        }
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
