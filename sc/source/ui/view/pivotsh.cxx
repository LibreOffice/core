/*************************************************************************
 *
 *  $RCSfile: pivotsh.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 12:04:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/srchitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <svtools/whiter.hxx>
#include <vcl/msgbox.hxx>

#include "sc.hrc"
#include "pivotsh.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "scresid.hxx"
#include "document.hxx"
#include "dpobject.hxx"
#include "dpshttab.hxx"
#include "dbdocfun.hxx"
#include "uiitems.hxx"
//CHINA001 #include "pfiltdlg.hxx"
#include "scabstdlg.hxx" //CHINA001
//------------------------------------------------------------------------

#define ScPivotShell
#include "scslots.hxx"

//------------------------------------------------------------------------

TYPEINIT1( ScPivotShell, SfxShell );

SFX_IMPL_INTERFACE(ScPivotShell, SfxShell, ScResId(SCSTR_PIVOTSHELL))
{
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_PIVOT) );
}


//------------------------------------------------------------------------

ScPivotShell::ScPivotShell( ScTabViewShell* pViewSh ) :
    SfxShell(pViewSh),
    pViewShell( pViewSh )
{
    SetPool( &pViewSh->GetPool() );
    SetUndoManager( pViewSh->GetViewData()->GetSfxDocShell()->GetUndoManager() );
    SetHelpId( HID_SCSHELL_PIVOTSH );
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Pivot")));
}

//------------------------------------------------------------------------
ScPivotShell::~ScPivotShell()
{
}

//------------------------------------------------------------------------
void ScPivotShell::Execute( SfxRequest& rReq )
{
    switch ( rReq.GetSlot() )
    {
        case SID_PIVOT_RECALC:
            pViewShell->RecalcPivotTable();
            break;

        case SID_PIVOT_KILL:
            pViewShell->DeletePivotTable();
            break;

        case SID_DP_FILTER:
        {
            ScDPObject* pDPObj = GetCurrDPObject();
            if( pDPObj )
            {
                ScQueryParam aQueryParam;
                SCTAB nSrcTab = 0;
                const ScSheetSourceDesc* pDesc = pDPObj->GetSheetDesc();
                DBG_ASSERT( pDesc, "no sheet source for DP filter dialog" );
                if( pDesc )
                {
                    aQueryParam = pDesc->aQueryParam;
                    nSrcTab = pDesc->aSourceRange.aStart.Tab();
                }

                ScViewData* pViewData = pViewShell->GetViewData();
                SfxItemSet aArgSet( pViewShell->GetPool(),
                    SCITEM_QUERYDATA, SCITEM_QUERYDATA );
                aArgSet.Put( ScQueryItem( SCITEM_QUERYDATA, pViewData, &aQueryParam ) );

                //CHINA001 ScPivotFilterDlg* pDlg = new ScPivotFilterDlg(
                //CHINA001     pViewShell->GetDialogParent(), aArgSet, nSrcTab );

                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

                AbstractScPivotFilterDlg* pDlg = pFact->CreateScPivotFilterDlg( pViewShell->GetDialogParent(),
                                                                                aArgSet, nSrcTab,
                                                                                ResId(RID_SCDLG_PIVOTFILTER));
                DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001

                if( pDlg->Execute() == RET_OK )
                {
                    ScSheetSourceDesc aNewDesc;
                    if( pDesc )
                        aNewDesc = *pDesc;

                    const ScQueryItem& rQueryItem = pDlg->GetOutputItem();
                    aNewDesc.aQueryParam = rQueryItem.GetQueryData();

                    ScDPObject aNewObj( *pDPObj );
                    aNewObj.SetSheetDesc( aNewDesc );
                    ScDBDocFunc aFunc( *pViewData->GetDocShell() );
                    aFunc.DataPilotUpdate( pDPObj, &aNewObj, TRUE, FALSE );
                    pViewData->GetView()->CursorPosChanged();       // shells may be switched
                }
                delete pDlg;
            }
        }
        break;
    }
}

//------------------------------------------------------------------------
void __EXPORT ScPivotShell::GetState( SfxItemSet& rSet )
{
    ScDocShell* pDocSh = pViewShell->GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    BOOL bDisable = pDocSh->IsReadOnly() || pDoc->GetChangeTrack();

    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_PIVOT_RECALC:
            case SID_PIVOT_KILL:
            {
                //! move ReadOnly check to idl flags
                if ( bDisable )
                {
                    rSet.DisableItem( nWhich );
                }
            }
            break;
            case SID_DP_FILTER:
            {
                ScDPObject* pDPObj = GetCurrDPObject();
                if( bDisable || !pDPObj || !pDPObj->IsSheetData() )
                    rSet.DisableItem( nWhich );
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}


//------------------------------------------------------------------------

ScDPObject* ScPivotShell::GetCurrDPObject()
{
    const ScViewData& rViewData = *pViewShell->GetViewData();
    return rViewData.GetDocument()->GetDPAtCursor(
        rViewData.GetCurX(), rViewData.GetCurY(), rViewData.GetTabNo() );
}

