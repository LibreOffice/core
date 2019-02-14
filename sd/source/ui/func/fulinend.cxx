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

#include <fulinend.hxx>
#include <svx/xtable.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/svdobj.hxx>
#include <svx/svdopath.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <strings.hrc>
#include <ViewShell.hxx>
#include <helpids.h>
#include <sdresid.hxx>
#include <drawdoc.hxx>
#include <View.hxx>
#include <Window.hxx>
#include <memory>

namespace sd {


FuLineEnd::FuLineEnd(ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
                    SdDrawDocument* pDoc, SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuLineEnd::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuLineEnd( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuLineEnd::DoExecute( SfxRequest& )
{
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() != 1 )
        return;

    const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
    const SdrObject* pNewObj;
    SdrObject* pConvPolyObj = nullptr;

    if( dynamic_cast< const SdrPathObj *>( pObj ) !=  nullptr )
    {
        pNewObj = pObj;
    }
    else
    {
        SdrObjTransformInfoRec aInfoRec;
        pObj->TakeObjInfo( aInfoRec );

        if( aInfoRec.bCanConvToPath &&
            pObj->GetObjInventor() == SdrInventor::Default &&
            pObj->GetObjIdentifier() != OBJ_GRUP )
            // bCanConvToPath is sal_True for group objects,
            // but it crashes on ConvertToPathObj()!
        {
            pNewObj = pConvPolyObj = pObj->ConvertToPolyObj( true, false );

            if( !pNewObj || dynamic_cast< const SdrPathObj *>( pNewObj ) ==  nullptr )
                return; // Cancel, additional security, but it does not help
                        // for group objects
        }
        else return; // Cancel
    }

    const ::basegfx::B2DPolyPolygon aPolyPolygon = static_cast<const SdrPathObj*>(pNewObj)->GetPathPoly();

    // Delete the created poly-object
    SdrObject::Free( pConvPolyObj );

    XLineEndListRef pLineEndList = mpDoc->GetLineEndList();

    OUString aNewName( SdResId( STR_LINEEND ) );
    OUString aDesc( SdResId( STR_DESC_LINEEND ) );
    OUString aName;

    long nCount = pLineEndList->Count();
    long j = 1;
    bool bDifferent = false;

    while( !bDifferent )
    {
        aName = aNewName + " " + OUString::number(j++);
        bDifferent = true;
        for( long i = 0; i < nCount && bDifferent; i++ )
        {
            if( aName == pLineEndList->GetLineEnd( i )->GetName() )
                bDifferent = false;
        }
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxNameDialog> pDlg( pFact->CreateSvxNameDialog( nullptr, aName, aDesc ) );

    pDlg->SetEditHelpId( HID_SD_NAMEDIALOG_LINEEND );

    if( pDlg->Execute() != RET_OK )
        return;

    pDlg->GetName( aName );
    bDifferent = true;

    for( long i = 0; i < nCount && bDifferent; i++ )
    {
        if( aName == pLineEndList->GetLineEnd( i )->GetName() )
            bDifferent = false;
    }

    if( bDifferent )
    {
        pLineEndList->Insert(std::make_unique<XLineEndEntry>(aPolyPolygon, aName));
    }
    else
    {
        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(mpWindow ? mpWindow->GetFrameWeld() : nullptr,
                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                   SdResId(STR_WARN_NAME_DUPLICATE)));
        xWarn->run();
    }
}

void FuLineEnd::Activate()
{
}

void FuLineEnd::Deactivate()
{
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
