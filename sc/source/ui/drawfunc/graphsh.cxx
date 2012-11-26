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



//------------------------------------------------------------------

#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <svl/whiter.hxx>
#include <svx/svdograf.hxx>
#include <svx/grfflt.hxx>
#include <svx/grafctrl.hxx>

#include "graphsh.hxx"
#include "sc.hrc"
#include "viewdata.hxx"
#include "drawview.hxx"
#include "scresid.hxx"

#define ScGraphicShell
#include "scslots.hxx"

#define ITEMVALUE(ItemSet,Id,Cast) ((const Cast&)(ItemSet).Get(Id)).GetValue()


SFX_IMPL_INTERFACE(ScGraphicShell, ScDrawShell, ScResId(SCSTR_GRAPHICSHELL) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT|SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                ScResId(RID_GRAPHIC_OBJECTBAR) );
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_GRAPHIC) );
}

ScGraphicShell::ScGraphicShell(ScViewData* pData) :
    ScDrawShell(pData)
{
    SetHelpId(HID_SCSHELL_GRAPHIC);
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("GraphicObject")));
}

ScGraphicShell::~ScGraphicShell()
{
}

void ScGraphicShell::GetAttrState( SfxItemSet& rSet )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();

    if( pView )
        SvxGrafAttrHelper::GetGrafAttrState( rSet, *pView );
}

void ScGraphicShell::Execute( SfxRequest& rReq )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();

    if( pView )
    {
        SvxGrafAttrHelper::ExecuteGrafAttr( rReq, *pView );
        Invalidate();
    }
}

void ScGraphicShell::GetFilterState( SfxItemSet& rSet )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    const SdrGrafObj* pSelected = dynamic_cast< SdrGrafObj* >(pView->getSelectedIfSingle());
    const sal_Bool bEnable( pSelected && GRAPHIC_BITMAP == pSelected->GetGraphicType() );

    if( !bEnable )
        SvxGraphicFilter::DisableGraphicFilterSlots( rSet );
}

void ScGraphicShell::ExecuteFilter( SfxRequest& rReq )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    SdrGrafObj* pSelected = dynamic_cast< SdrGrafObj* >(pView->getSelectedIfSingle());

    if( pSelected && GRAPHIC_BITMAP == pSelected->GetGraphicType() )
    {
        GraphicObject aFilterObj( pSelected->GetGraphicObject() );

        if( SVX_GRAPHICFILTER_ERRCODE_NONE ==
            SvxGraphicFilter::ExecuteGrfFilterSlot( rReq, aFilterObj ) )
        {
            SdrGrafObj* pFilteredObj = static_cast< SdrGrafObj* >(pSelected->CloneSdrObject());
            String      aStr;

            pSelected->TakeObjNameSingul(aStr);
            aStr.Append( sal_Unicode(' ') );
            aStr.Append( String( ScResId( SCSTR_UNDO_GRAFFILTER ) ) );
            pView->BegUndo( aStr );
            pFilteredObj->SetGraphicObject( aFilterObj );
            pView->ReplaceObjectAtView( *pSelected, *pFilteredObj );
            pView->EndUndo();
        }
    }

    Invalidate();
}

