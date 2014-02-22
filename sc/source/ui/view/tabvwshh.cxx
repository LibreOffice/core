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

#include <svx/svdmark.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <basic/sbxcore.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>

#include "tabvwsh.hxx"
#include "client.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "sc.hrc"
#include "drwlayer.hxx"
#include "retypepassdlg.hxx"
#include "tabprotection.hxx"

#include <boost/scoped_ptr.hpp>

using namespace com::sun::star;



void ScTabViewShell::ExecuteSbx( SfxRequest& /* rReq */ )
{
    
    
}

void ScTabViewShell::GetSbxState( SfxItemSet& /* rSet */ )
{
    
}



void ScTabViewShell::ExecuteObject( SfxRequest& rReq )
{
    sal_uInt16 nSlotId = rReq.GetSlot();
    const SfxItemSet* pReqArgs = rReq.GetArgs();

        

    ScTabViewShell* pVisibleSh = this;
    if ( nSlotId == SID_OLE_SELECT || nSlotId == SID_OLE_ACTIVATE || nSlotId == SID_OLE_DEACTIVATE )
    {
        OSL_FAIL("old slot SID_OLE...");
    }

    switch (nSlotId)
    {
        case SID_OLE_SELECT:
        case SID_OLE_ACTIVATE:
            {
                

                OUString aName;
                SdrView* pDrView = GetSdrView();
                if (pDrView)
                {
                    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
                    if (rMarkList.GetMarkCount() == 1)
                        aName = ScDrawLayer::GetVisibleName( rMarkList.GetMark(0)->GetMarkedSdrObj() );
                }
                pVisibleSh->SelectObject( aName );

                

                if ( nSlotId == SID_OLE_ACTIVATE )
                    pVisibleSh->DoVerb( 0 );
            }
            break;
        case SID_OLE_DEACTIVATE:
            pVisibleSh->DeactivateOle();
            break;

        case SID_OBJECT_LEFT:
        case SID_OBJECT_TOP:
        case SID_OBJECT_WIDTH:
        case SID_OBJECT_HEIGHT:
            {
                bool bDone = false;
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState( nSlotId, true, &pItem ) == SFX_ITEM_SET )
                {
                    long nNewVal = ((const SfxInt32Item*)pItem)->GetValue();
                    if ( nNewVal < 0 )
                        nNewVal = 0;

                    

                    SdrView* pDrView = GetSdrView();
                    if ( pDrView )
                    {
                        const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
                        if (rMarkList.GetMarkCount() == 1)
                        {
                            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                            Rectangle aRect = pObj->GetLogicRect();

                            if ( nSlotId == SID_OBJECT_LEFT )
                                pDrView->MoveMarkedObj( Size( nNewVal - aRect.Left(), 0 ) );
                            else if ( nSlotId == SID_OBJECT_TOP )
                                pDrView->MoveMarkedObj( Size( 0, nNewVal - aRect.Top() ) );
                            else if ( nSlotId == SID_OBJECT_WIDTH )
                                pDrView->ResizeMarkedObj( aRect.TopLeft(),
                                                Fraction( nNewVal, aRect.GetWidth() ),
                                                Fraction( 1, 1 ) );
                            else 
                                pDrView->ResizeMarkedObj( aRect.TopLeft(),
                                                Fraction( 1, 1 ),
                                                Fraction( nNewVal, aRect.GetHeight() ) );
                            bDone = true;
                        }
                    }
                }
#ifndef DISABLE_SCRIPTING
                if (!bDone)
                    SbxBase::SetError( SbxERR_BAD_PARAMETER );  
#endif
            }
            break;

    }
}

static uno::Reference < embed::XEmbeddedObject > lcl_GetSelectedObj( SdrView* pDrView )       
{
    uno::Reference < embed::XEmbeddedObject > xRet;
    if (pDrView)
    {
        const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
        if (rMarkList.GetMarkCount() == 1)
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            if (pObj->GetObjIdentifier() == OBJ_OLE2)
            {
                SdrOle2Obj* pOle2Obj = (SdrOle2Obj*) pObj;
                xRet = pOle2Obj->GetObjRef();
            }
        }
    }

    return xRet;
}

void ScTabViewShell::GetObjectState( SfxItemSet& rSet )
{
    

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch (nWhich)
        {
            case SID_ACTIVE_OBJ_NAME:
                {
                    OUString aName;
                    uno::Reference < embed::XEmbeddedObject > xOLE = lcl_GetSelectedObj( GetSdrView() );
                    if (xOLE.is())
                    {
                        aName = GetViewData()->GetSfxDocShell()->GetEmbeddedObjectContainer().GetEmbeddedObjectName( xOLE );
                    }
                    rSet.Put( SfxStringItem( nWhich, aName ) );
                }
                break;
            case SID_OBJECT_LEFT:
            case SID_OBJECT_TOP:
            case SID_OBJECT_WIDTH:
            case SID_OBJECT_HEIGHT:
                {
                    SdrView* pDrView = GetSdrView();
                    if ( pDrView )
                    {
                        const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
                        if (rMarkList.GetMarkCount() == 1)
                        {
                            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                            Rectangle aRect = pObj->GetLogicRect();

                            long nVal;
                            if ( nWhich == SID_OBJECT_LEFT )
                                nVal = aRect.Left();
                            else if ( nWhich == SID_OBJECT_TOP )
                                nVal = aRect.Top();
                            else if ( nWhich == SID_OBJECT_WIDTH )
                                nVal = aRect.GetWidth();
                            else 
                                nVal = aRect.GetHeight();

                            

                            rSet.Put( SfxInt32Item( nWhich, nVal ) );
                        }
                    }
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

void ScTabViewShell::AddAccessibilityObject( SfxListener& rObject )
{
    if (!pAccessibilityBroadcaster)
        pAccessibilityBroadcaster = new SfxBroadcaster;

    rObject.StartListening( *pAccessibilityBroadcaster );
    ScDocument* pDoc = GetViewData()->GetDocument();
    if (pDoc)
        pDoc->AddUnoObject(rObject);
}

void ScTabViewShell::RemoveAccessibilityObject( SfxListener& rObject )
{
    if (pAccessibilityBroadcaster)
    {
        rObject.EndListening( *pAccessibilityBroadcaster );
        ScDocument* pDoc = GetViewData()->GetDocument();
        if (pDoc)
            pDoc->RemoveUnoObject(rObject);
    }
    else
    {
        OSL_FAIL("kein Accessibility-Broadcaster?");
    }
}

void ScTabViewShell::BroadcastAccessibility( const SfxHint &rHint )
{
    if (pAccessibilityBroadcaster)
        pAccessibilityBroadcaster->Broadcast( rHint );
}

bool ScTabViewShell::HasAccessibilityObjects()
{
    return pAccessibilityBroadcaster != NULL;
}

bool ScTabViewShell::ExecuteRetypePassDlg(ScPasswordHash eDesiredHash)
{
    ScDocument* pDoc = GetViewData()->GetDocument();

    boost::scoped_ptr<ScRetypePassDlg> pDlg(new ScRetypePassDlg(GetDialogParent()));
    pDlg->SetDataFromDocument(*pDoc);
    pDlg->SetDesiredHash(eDesiredHash);
    if (pDlg->Execute() != RET_OK)
        return false;

    pDlg->WriteNewDataToDocument(*pDoc);
    return true;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
