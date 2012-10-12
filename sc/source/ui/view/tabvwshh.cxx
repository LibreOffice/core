/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include "drwlayer.hxx"     // GetVisibleName
#include "retypepassdlg.hxx"
#include "tabprotection.hxx"

#include <memory>

using namespace com::sun::star;

//------------------------------------------------------------------

void ScTabViewShell::ExecuteSbx( SfxRequest& /* rReq */ )
{
    //  SID_RANGE_OFFSET (Offset),
    //  SID_PIVOT_CREATE (DataPilotCreate) - removed (old Basic)
}

void ScTabViewShell::GetSbxState( SfxItemSet& /* rSet */ )
{
    //  SID_RANGE_REGION (CurrentRegion) - removed (old Basic)
}

//------------------------------------------------------------------

void ScTabViewShell::ExecuteObject( SfxRequest& rReq )
{
    sal_uInt16 nSlotId = rReq.GetSlot();
    const SfxItemSet* pReqArgs = rReq.GetArgs();

        //  Objekte aktivieren/deaktivieren immer auf der sichtbaren View

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
                //  in beiden Faellen erstmal auf der sichtbaren View selektieren

                String aName;
                SdrView* pDrView = GetSdrView();
                if (pDrView)
                {
                    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
                    if (rMarkList.GetMarkCount() == 1)
                        aName = ScDrawLayer::GetVisibleName( rMarkList.GetMark(0)->GetMarkedSdrObj() );
                }
                pVisibleSh->SelectObject( aName );

                //  aktivieren

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
                sal_Bool bDone = false;
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState( nSlotId, sal_True, &pItem ) == SFX_ITEM_SET )
                {
                    long nNewVal = ((const SfxInt32Item*)pItem)->GetValue();
                    if ( nNewVal < 0 )
                        nNewVal = 0;

                    //! von irgendwas in 1/100mm umrechnen ??????

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
                            else // if ( nSlotId == SID_OBJECT_HEIGHT )
                                pDrView->ResizeMarkedObj( aRect.TopLeft(),
                                                Fraction( 1, 1 ),
                                                Fraction( nNewVal, aRect.GetHeight() ) );
                            bDone = sal_True;
                        }
                    }
                }
#ifndef DISABLE_SCRIPTING
                if (!bDone)
                    SbxBase::SetError( SbxERR_BAD_PARAMETER );  // Basic-Fehler
#endif
            }
            break;

    }
}

static uno::Reference < embed::XEmbeddedObject > lcl_GetSelectedObj( SdrView* pDrView )       //! Member von ScDrawView?
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
    //  SID_OLE_OBJECT - removed (old Basic)

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch (nWhich)
        {
            case SID_ACTIVE_OBJ_NAME:
                {
                    String aName;
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
                            else // if ( nWhich == SID_OBJECT_HEIGHT )
                                nVal = aRect.GetHeight();

                            //! von 1/100mm in irgendwas umrechnen ??????

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

sal_Bool ScTabViewShell::HasAccessibilityObjects()
{
    return pAccessibilityBroadcaster != NULL;
}

bool ScTabViewShell::ExecuteRetypePassDlg(ScPasswordHash eDesiredHash)
{
    using ::std::auto_ptr;

    ScDocument* pDoc = GetViewData()->GetDocument();

    auto_ptr<ScRetypePassDlg> pDlg(new ScRetypePassDlg(GetDialogParent()));
    pDlg->SetDataFromDocument(*pDoc);
    pDlg->SetDesiredHash(eDesiredHash);
    if (pDlg->Execute() != RET_OK)
        return false;

    pDlg->WriteNewDataToDocument(*pDoc);
    return true;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
