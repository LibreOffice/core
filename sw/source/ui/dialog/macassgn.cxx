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
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include "hintids.hxx"

#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>
#include <svx/htmlmode.hxx>
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#include <sfx2/app.hxx>
#include <vcl/msgbox.hxx>

#include "swtypes.hxx"
#include "wrtsh.hxx"
#include "viewopt.hxx"
#include "macassgn.hxx"
#include "swevent.hxx"
#include "docsh.hxx"
#include "globals.hrc"
#include "view.hxx"
#include <sfx2/viewfrm.hxx>

#include <svx/svxdlg.hxx>

#include <doc.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XFrame;

SfxEventNamesItem SwMacroAssignDlg::AddEvents( DlgEventType eType )
{
    // const SfxItemSet& rSet = rPg.GetItemSet();
    SfxEventNamesItem aItem(SID_EVENTCONFIG);

    sal_Bool bHtmlMode = sal_False;
    sal_uInt16 nHtmlMode = ::GetHtmlMode((const SwDocShell*)SfxObjectShell::Current());
    bHtmlMode = nHtmlMode & HTMLMODE_ON ? sal_True : sal_False;

    switch( eType )
    {
    case MACASSGN_TEXTBAUST:            // Textbausteine
        // rPg.SetGetRangeLink( &_GetRangeHdl );
        aItem.AddEvent( String( SW_RES(STR_EVENT_START_INS_GLOSSARY) ), String(),
                            SW_EVENT_START_INS_GLOSSARY );
        aItem.AddEvent( String( SW_RES(STR_EVENT_END_INS_GLOSSARY) ), String(),
                            SW_EVENT_END_INS_GLOSSARY);
        // damit der neue Handler aktiv wird!
        // rPg.Reset( rSet );
        break;
    case MACASSGN_ALLFRM:
    case MACASSGN_GRAPHIC:          // Grafiken
        {
            aItem.AddEvent( String( SW_RES(STR_EVENT_IMAGE_ERROR) ), String(),
                                SVX_EVENT_IMAGE_ERROR);
            aItem.AddEvent( String( SW_RES(STR_EVENT_IMAGE_ABORT) ), String(),
                                SVX_EVENT_IMAGE_ABORT);
            aItem.AddEvent( String( SW_RES(STR_EVENT_IMAGE_LOAD) ), String(),
                                SVX_EVENT_IMAGE_LOAD);
        }
        // kein break;
    case MACASSGN_FRMURL:           // Frm - URL-Attribute
        {
            if( !bHtmlMode &&
                (MACASSGN_FRMURL == eType || MACASSGN_ALLFRM == eType))
            {
                aItem.AddEvent( String( SW_RES( STR_EVENT_FRM_KEYINPUT_A ) ), String(),
                                SW_EVENT_FRM_KEYINPUT_ALPHA );
                aItem.AddEvent( String( SW_RES( STR_EVENT_FRM_KEYINPUT_NOA ) ), String(),
                                SW_EVENT_FRM_KEYINPUT_NOALPHA );
                aItem.AddEvent( String( SW_RES( STR_EVENT_FRM_RESIZE ) ), String(),
                                SW_EVENT_FRM_RESIZE );
                aItem.AddEvent( String( SW_RES( STR_EVENT_FRM_MOVE ) ), String(),
                                SW_EVENT_FRM_MOVE );
            }
        }
        // kein break;
    case MACASSGN_OLE:              // OLE
        {
            if( !bHtmlMode )
                aItem.AddEvent( String( SW_RES(STR_EVENT_OBJECT_SELECT) ), String(),
                                SW_EVENT_OBJECT_SELECT );
        }
        // kein break;
    case MACASSGN_INETFMT:          // INetFmt-Attribute
        {
            aItem.AddEvent( String( SW_RES(STR_EVENT_MOUSEOVER_OBJECT) ), String(),
                                SFX_EVENT_MOUSEOVER_OBJECT );
            aItem.AddEvent( String( SW_RES(STR_EVENT_MOUSECLICK_OBJECT) ), String(),
                                SFX_EVENT_MOUSECLICK_OBJECT);
            aItem.AddEvent( String( SW_RES(STR_EVENT_MOUSEOUT_OBJECT) ), String(),
                                SFX_EVENT_MOUSEOUT_OBJECT);
        }
        break;
    }

    return aItem;
}


sal_Bool SwMacroAssignDlg::INetFmtDlg( Window* pParent, SwWrtShell& rSh,
                                    SvxMacroItem*& rpINetItem )
{
    sal_Bool bRet = sal_False;
    SfxItemSet aSet( rSh.GetAttrPool(), RES_FRMMACRO, RES_FRMMACRO, SID_EVENTCONFIG, SID_EVENTCONFIG, 0 );
    SvxMacroItem aItem( RES_FRMMACRO );
    if( !rpINetItem )
        rpINetItem = new SvxMacroItem( RES_FRMMACRO );
    else
        aItem.SetMacroTable( rpINetItem->GetMacroTable() );

    aSet.Put( aItem );
    aSet.Put( AddEvents( MACASSGN_INETFMT ) );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    SfxAbstractDialog* pMacroDlg = pFact->CreateSfxDialog( pParent, aSet,
        rSh.GetView().GetViewFrame()->GetFrame().GetFrameInterface(),
        SID_EVENTCONFIG );
    if ( pMacroDlg && pMacroDlg->Execute() == RET_OK )
    {
        const SfxItemSet* pOutSet = pMacroDlg->GetOutputItemSet();
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == pOutSet->GetItemState( RES_FRMMACRO, sal_False, &pItem ))
        {
            rpINetItem->SetMacroTable( ((SvxMacroItem*)pItem)->GetMacroTable() );
            bRet = sal_True;
        }
    }
    return bRet;
}
