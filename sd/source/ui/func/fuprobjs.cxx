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

#include <fuprobjs.hxx>

#include <svl/style.hxx>
#include <editeng/outliner.hxx>
#include <svl/hint.hxx>

#include <app.hrc>

#include <strings.hxx>

#include <drawdoc.hxx>
#include <sfx2/sfxdlg.hxx>
#include <DrawDocShell.hxx>
#include <OutlineView.hxx>
#include <OutlineViewShell.hxx>
#include <ViewShell.hxx>
#include <Window.hxx>
#include <glob.hxx>
#include <prlayout.hxx>
#include <unchss.hxx>
#include <sdabstdlg.hxx>
#include <memory>

namespace sd {


FuPresentationObjects::FuPresentationObjects (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
     : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuPresentationObjects::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuPresentationObjects( pViewSh, pWin, pView, pDoc, rReq ) );
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
    SfxItemSet aSet(mpDoc->GetItemPool(), svl::Items<SID_STATUS_LAYOUT, SID_STATUS_LAYOUT>{});
    pOutlineViewShell->GetStatusBarState( aSet );
    OUString aLayoutName = static_cast<const SfxStringItem&>(aSet.Get(SID_STATUS_LAYOUT)).GetValue();
    DBG_ASSERT(!aLayoutName.isEmpty(), "Layout not defined");

    bool    bUnique = false;
    sal_Int16   nDepth, nTmp;
    OutlineView* pOlView = static_cast<OutlineView*>(pOutlineViewShell->GetView());
    OutlinerView* pOutlinerView = pOlView->GetViewByWindow( static_cast<Window*>(mpWindow) );
    ::Outliner* pOutl = pOutlinerView->GetOutliner();

    std::vector<Paragraph*> aSelList;
    pOutlinerView->CreateSelectionList(aSelList);

    Paragraph* pPara = aSelList.empty() ? nullptr : aSelList.front();

    nDepth = pOutl->GetDepth(pOutl->GetAbsPos( pPara ) );
    bool bPage = ::Outliner::HasParaFlag( pPara, ParaFlag::ISPAGE );

    for( const auto& rpPara : aSelList )
    {
        nTmp = pOutl->GetDepth( pOutl->GetAbsPos( rpPara ) );

        if( nDepth != nTmp )
        {
            bUnique = false;
            break;
        }

        if( ::Outliner::HasParaFlag( rpPara, ParaFlag::ISPAGE ) != bPage )
        {
            bUnique = false;
            break;
        }
        bUnique = true;
    }

    if( !bUnique )
        return;

    OUString aStyleName = aLayoutName + SD_LT_SEPARATOR;
    PresentationObjects ePO;

    if( bPage )
    {
        ePO = PO_TITLE;
        aStyleName += STR_LAYOUT_TITLE;
    }
    else
    {
        ePO = static_cast<PresentationObjects>( PO_OUTLINE_1 + nDepth - 1 );
        aStyleName += STR_LAYOUT_OUTLINE " "
            + OUString::number(nDepth);
    }

    SfxStyleSheetBasePool* pStyleSheetPool = mpDocSh->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStyleSheetPool->Find( aStyleName, SfxStyleFamily::Page );
    DBG_ASSERT(pStyleSheet, "StyleSheet missing");

    if( !pStyleSheet )
        return;

    SfxStyleSheetBase& rStyleSheet = *pStyleSheet;

    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateSdPresLayoutTemplateDlg(mpDocSh, mpViewShell->GetFrameWeld(),
                                                        false, rStyleSheet, ePO, pStyleSheetPool));
    if( pDlg->Execute() == RET_OK )
    {
        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
        // Undo-Action
        mpDocSh->GetUndoManager()->AddUndoAction(
            std::make_unique<StyleSheetUndoAction>(mpDoc, static_cast<SfxStyleSheet*>(pStyleSheet), pOutSet));

        pStyleSheet->GetItemSet().Put( *pOutSet );
        static_cast<SfxStyleSheet*>( pStyleSheet )->Broadcast( SfxHint( SfxHintId::DataChanged ) );
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
