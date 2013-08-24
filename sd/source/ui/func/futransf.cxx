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

#include "futransf.hxx"

#include <svx/dialogs.hrc>
#include <svx/polysc3d.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>

#include "strings.hrc"
#include "ViewShell.hxx"
#include "View.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include <svx/svxdlg.hxx>

#include <boost/scoped_ptr.hpp>

namespace sd {

TYPEINIT1( FuTransform, FuPoor );

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
    if( mpView->AreObjectsMarked() )
    {
        const SfxItemSet* pArgs = rReq.GetArgs();

        if( !pArgs )
        {
            // --------- itemset for size and position --------
            SfxItemSet aSet( mpView->GetGeoAttrFromMarked() );

            const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            if( rMarkList.GetMarkCount() == 1 &&
                pObj->GetObjInventor() == SdrInventor &&
                pObj->GetObjIdentifier() == OBJ_CAPTION )
            {
                // --------- itemset for caption --------
                SfxItemSet aNewAttr( mpDoc->GetPool() );
                mpView->GetAttributes( aNewAttr );

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if ( pFact )
                {
                    boost::scoped_ptr< SfxAbstractTabDialog > pDlg( pFact->CreateCaptionDialog( NULL, mpView ) );

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
                    boost::scoped_ptr< SfxAbstractTabDialog > pDlg( pFact->CreateSvxTransformTabDialog( NULL, &aSet, mpView ) );
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
            OUString aString( mpView->GetDescriptionOfMarkedObjects() );
            aString += " " + SD_RESSTR( STR_TRANSFORM );
            mpView->BegUndo( aString );

            mpView->SetGeoAttrToMarked( *pArgs );
            mpView->SetAttributes( *pArgs );
            mpView->EndUndo();
        }
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
