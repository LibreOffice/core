/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "fuline.hxx"

#include <svx/svxids.hrc>
#include <svx/tabline.hxx>
#include <svx/xenum.hxx>
#include <vcl/msgbox.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/request.hxx>
#include <svx/xdef.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include "ViewShell.hxx"
#include "View.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "app.hrc"
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

namespace sd {

TYPEINIT1( FuLine, FuPoor );


FuLine::FuLine (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuLine::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuLine( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuLine::DoExecute( SfxRequest& rReq )
{
    sal_Bool        bHasMarked = mpView->AreObjectsMarked();

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        const SdrObject* pObj = NULL;
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
        if( rMarkList.GetMarkCount() == 1 )
            pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

        SfxItemSet* pNewAttr = new SfxItemSet( mpDoc->GetPool() );
        mpView->GetAttributes( *pNewAttr );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        SfxAbstractTabDialog * pDlg = pFact ? pFact->CreateSvxLineTabDialog(NULL,pNewAttr,mpDoc,pObj,bHasMarked) : 0;
        if( pDlg && (pDlg->Execute() == RET_OK) )
        {
            mpView->SetAttributes (*(pDlg->GetOutputItemSet ()));
        }

        
        static sal_uInt16 SidArray[] = {
            SID_ATTR_LINE_STYLE,                
            SID_ATTR_LINE_DASH,                 
            SID_ATTR_LINE_WIDTH,                
            SID_ATTR_LINE_COLOR,                
            SID_ATTR_LINE_START,                
            SID_ATTR_LINE_END,                  
            SID_ATTR_LINE_TRANSPARENCE,         
            SID_ATTR_LINE_JOINT,                
            SID_ATTR_LINE_CAP,                  
            0 };

        mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );

        delete pDlg;
        delete pNewAttr;
    }

    rReq.Ignore ();
}

void FuLine::Activate()
{
}

void FuLine::Deactivate()
{
}


} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
