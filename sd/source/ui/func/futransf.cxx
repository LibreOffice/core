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

#include <futransf.hxx>

#include <comphelper/scopeguard.hxx>
#include <svx/dialogs.hrc>
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>

#include <strings.hrc>
#include <ViewShell.hxx>
#include <View.hxx>
#include <Window.hxx>
#include <sdresid.hxx>
#include <drawdoc.hxx>
#include <svx/svxdlg.hxx>

#include <memory>

using namespace sd;

FuTransform::FuTransform(ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
                         SdDrawDocument* pDoc, SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuTransform::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuTransform( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

namespace {

void setUndo(::sd::View* pView, const SfxItemSet* pArgs)
{
    // Undo
    OUString aString(pView->GetDescriptionOfMarkedObjects());
    aString += " " + SdResId(STR_TRANSFORM);
    pView->BegUndo(aString);

    pView->SetGeoAttrToMarked(*pArgs);
    pView->SetAttributes(*pArgs);
    pView->EndUndo();
}

}

void FuTransform::DoExecute( SfxRequest& rReq )
{
    comphelper::ScopeGuard guard([&]() {
        // cleanup when leaving
        mpViewShell->Invalidate(SID_RULER_OBJECT);
        mpViewShell->Cancel();
    });

    if (!mpView->AreObjectsMarked())
        return;

    const SfxItemSet* pArgs = rReq.GetArgs();

    if (pArgs)
    {
        setUndo(mpView, pArgs);
        return;
    }

    // --------- itemset for size and position --------
    SfxItemSet aSet( mpView->GetGeoAttrFromMarked() );
    VclPtr<SfxAbstractTabDialog> pDlg;

    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
    if( rMarkList.GetMarkCount() == 1 &&
        pObj->GetObjInventor() == SdrInventor::Default &&
        pObj->GetObjIdentifier() == OBJ_CAPTION )
    {
        // --------- itemset for caption --------
        SfxItemSet aNewAttr( mpDoc->GetPool() );
        mpView->GetAttributes( aNewAttr );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if (!pFact)
            return;

        pDlg.reset(pFact->CreateCaptionDialog(mpViewShell->GetActiveWindow(), mpView));

        const sal_uInt16* pRange = pDlg->GetInputRanges( *aNewAttr.GetPool() );
        SfxItemSet aCombSet( *aNewAttr.GetPool(), pRange );
        aCombSet.Put( aNewAttr );
        aCombSet.Put( aSet );
        pDlg->SetInputSet( &aCombSet );
    }
    else
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if (!pFact)
            return;

        pDlg.reset(pFact->CreateSvxTransformTabDialog(mpViewShell->GetActiveWindow(), &aSet, mpView));
    }

    if (!pDlg)
        return;

    std::shared_ptr<SfxRequest> pRequest(new SfxRequest(rReq));
    rReq.Ignore(); // the 'old' request is not relevant any more
    guard.dismiss(); // we'll invalidate explicitly after the dialog ends

    pDlg->StartExecuteAsync([=](sal_Int32 nResult){
        if (nResult == RET_OK)
        {
            pRequest->Done(*(pDlg->GetOutputItemSet()));
            setUndo(mpView, pRequest->GetArgs());
        }

        mpViewShell->Invalidate(SID_RULER_OBJECT);
        mpViewShell->Cancel();
    });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
