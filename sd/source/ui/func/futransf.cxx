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

#include <svx/svxids.hrc>
#include <sfx2/request.hxx>

#include <strings.hrc>
#include <ViewShell.hxx>
#include <View.hxx>
#include <sdresid.hxx>
#include <drawdoc.hxx>
#include <svx/svxdlg.hxx>
#include <comphelper/lok.hxx>

#include <memory>

using namespace sd;

FuTransform::FuTransform(ViewShell& rViewSh, ::sd::Window* pWin, ::sd::View* pView,
                         SdDrawDocument& rDoc, SfxRequest& rReq)
    : FuPoor(rViewSh, pWin, pView, rDoc, rReq)
{
}

rtl::Reference<FuPoor> FuTransform::Create( ViewShell& rViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument& rDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuTransform( rViewSh, pWin, pView, rDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

namespace {

void setUndo(::sd::View* pView, const SfxItemSet* pArgs, bool addPageMargin)
{
    // Undo
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    OUString aString = rMarkList.GetMarkDescription() +
        " " + SdResId(STR_TRANSFORM);
    pView->BegUndo(aString);
    pView->SetGeoAttrToMarked(*pArgs, addPageMargin);
    pView->SetAttributes(*pArgs);
    pView->EndUndo();
}

}

void FuTransform::DoExecute( SfxRequest& rReq )
{
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
    if (rMarkList.GetMarkCount() == 0)
        return;

    const SfxItemSet* pArgs = rReq.GetArgs();

    if (pArgs)
    {
        // If this comes from LOK, that means the shape is moved by mouse
        // only then pArgs is pre-set.
        setUndo(mpView, pArgs, comphelper::LibreOfficeKit::isActive());
        return;
    }

    // --------- itemset for size and position --------
    SfxItemSet aSet( mpView->GetGeoAttrFromMarked() );
    VclPtr<SfxAbstractTabDialog> pDlg;

    bool bWelded = false;
    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
    if( rMarkList.GetMarkCount() == 1 &&
        pObj->GetObjInventor() == SdrInventor::Default &&
        pObj->GetObjIdentifier() == SdrObjKind::Caption )
    {
        // --------- itemset for caption --------
        SfxItemSet aNewAttr( mrDoc.GetPool() );
        mpView->GetAttributes( aNewAttr );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        pDlg.reset(pFact->CreateCaptionDialog(mrViewShell.GetFrameWeld(), mpView));

        const WhichRangesContainer aRange = pDlg->GetInputRanges( *aNewAttr.GetPool() );
        SfxItemSet aCombSet( *aNewAttr.GetPool(), aRange );
        aCombSet.Put( aNewAttr );
        aCombSet.Put( aSet );
        pDlg->SetInputSet( &aCombSet );
    }
    else
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        pDlg.reset(pFact->CreateSvxTransformTabDialog(mrViewShell.GetFrameWeld(), &aSet, mpView));
        bWelded = true;
    }

    assert(pDlg && "there must be a dialog at this point");

    auto xRequest = std::make_shared<SfxRequest>(rReq);
    rReq.Ignore(); // the 'old' request is not relevant any more

    pDlg->StartExecuteAsync([bWelded, pDlg, xRequest=std::move(xRequest), this](sal_Int32 nResult){
        if (nResult == RET_OK)
        {
            xRequest->Done(*(pDlg->GetOutputItemSet()));
            // Page margin is already calculated at this point.
            setUndo(mpView, xRequest->GetArgs(), false);
        }

        // deferred until the dialog ends
        mrViewShell.Invalidate(SID_RULER_OBJECT);
        mrViewShell.Cancel();
        if (bWelded)
            pDlg->disposeOnce();
    });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
