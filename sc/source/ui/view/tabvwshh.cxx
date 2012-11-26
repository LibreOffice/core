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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <basic/sbxcore.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdlegacy.hxx>

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
        DBG_ERROR("old slot SID_OLE...");
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
                    SdrObject* pSelected = pDrView->getSelectedIfSingle();

                    if (pSelected)
                        aName = ScDrawLayer::GetVisibleName( pSelected );
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
                sal_Bool bDone = sal_False;
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
                        SdrObject* pSelected = pDrView->getSelectedIfSingle();

                        if (pSelected)
                        {
                            const basegfx::B2DRange aRange(sdr::legacy::GetLogicRange(*pSelected));

                            if ( nSlotId == SID_OBJECT_LEFT )
                                pDrView->MoveMarkedObj( basegfx::B2DVector( nNewVal - aRange.getMinX(), 0 ) );
                            else if ( nSlotId == SID_OBJECT_TOP )
                                pDrView->MoveMarkedObj( basegfx::B2DVector( 0, nNewVal - aRange.getMinY() ) );
                            else if ( nSlotId == SID_OBJECT_WIDTH )
                                pDrView->ResizeMarkedObj( aRange.getMinimum(), basegfx::B2DVector(nNewVal /aRange.getWidth(), 1.0));
                            else // if ( nSlotId == SID_OBJECT_HEIGHT )
                                pDrView->ResizeMarkedObj( aRange.getMinimum(), basegfx::B2DVector(1.0, nNewVal / aRange.getHeight()));
                            bDone = sal_True;
                        }
                    }
                }
                if (!bDone)
                    SbxBase::SetError( SbxERR_BAD_PARAMETER );  // Basic-Fehler
            }
            break;

    }
}

uno::Reference < embed::XEmbeddedObject > lcl_GetSelectedObj( SdrView* pDrView )       //! Member von ScDrawView?
{
    uno::Reference < embed::XEmbeddedObject > xRet;
    if (pDrView)
    {
        SdrOle2Obj* pSelected = dynamic_cast< SdrOle2Obj* >(pDrView->getSelectedIfSingle());

        if (pSelected)
        {
            xRet = pSelected->GetObjRef();
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
                        SdrObject* pSelected = pDrView->getSelectedIfSingle();

                        if (pSelected)
                        {
                            const basegfx::B2DRange aRange(sdr::legacy::GetLogicRange(*pSelected));

                            double fVal(0.0);
                            if ( nWhich == SID_OBJECT_LEFT )
                                fVal = aRange.getMinX();
                            else if ( nWhich == SID_OBJECT_TOP )
                                fVal = aRange.getMinY();
                            else if ( nWhich == SID_OBJECT_WIDTH )
                                fVal = aRange.getWidth();
                            else // if ( nWhich == SID_OBJECT_HEIGHT )
                                fVal = aRange.getHeight();

                            //! von 1/100mm in irgendwas umrechnen ??????

                            rSet.Put( SfxInt32Item( nWhich, basegfx::fround(fVal) ) );
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
        DBG_ERROR("kein Accessibility-Broadcaster?");
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




