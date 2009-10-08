/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: macassgn.cxx,v $
 * $Revision: 1.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include "hintids.hxx"

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
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

#include <doc.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XFrame;

SvStringsDtor* __EXPORT _GetRangeHdl( _SfxMacroTabPage*, const String& );


SwMacroAssignDlg::SwMacroAssignDlg( Window* pParent, SfxItemSet& rSet, const SwWrtShell& rSh,
                                        DlgEventType eType )
    : SfxMacroAssignDlg( pParent, rSh.GetDoc() ? rSh.GetDoc()->GetDocShell() : NULL, rSet )
{
     // TabPage holen
    SwMacroAssignDlg::AddEvents( *(SfxMacroTabPage*) GetTabPage(), eType );
}


SwMacroAssignDlg::~SwMacroAssignDlg()
{
}


void SwMacroAssignDlg::AddEvents( SfxMacroTabPage& rPg, DlgEventType eType )
{
    const SfxItemSet& rSet = rPg.GetItemSet();

    BOOL bHtmlMode = FALSE;
    USHORT nHtmlMode = ::GetHtmlMode((const SwDocShell*)SfxObjectShell::Current());
    bHtmlMode = nHtmlMode & HTMLMODE_ON ? TRUE : FALSE;

    switch( eType )
    {
    case MACASSGN_TEXTBAUST:            // Textbausteine
        rPg.SetGetRangeLink( &_GetRangeHdl );
        rPg.AddEvent( String( SW_RES(STR_EVENT_START_INS_GLOSSARY) ),
                            SW_EVENT_START_INS_GLOSSARY );
        rPg.AddEvent( String( SW_RES(STR_EVENT_END_INS_GLOSSARY) ),
                            SW_EVENT_END_INS_GLOSSARY);
        // damit der neue Handler aktiv wird!
        rPg.Reset( rSet );
        break;
    case MACASSGN_ALLFRM:
    case MACASSGN_GRAPHIC:          // Grafiken
        {
            rPg.AddEvent( String( SW_RES(STR_EVENT_IMAGE_ERROR) ),
                                SVX_EVENT_IMAGE_ERROR);
            rPg.AddEvent( String( SW_RES(STR_EVENT_IMAGE_ABORT) ),
                                SVX_EVENT_IMAGE_ABORT);
            rPg.AddEvent( String( SW_RES(STR_EVENT_IMAGE_LOAD) ),
                                SVX_EVENT_IMAGE_LOAD);
        }
        // kein break;
    case MACASSGN_FRMURL:           // Frm - URL-Attribute
        {
            if( !bHtmlMode &&
                (MACASSGN_FRMURL == eType || MACASSGN_ALLFRM == eType))
            {
                rPg.AddEvent( String( SW_RES( STR_EVENT_FRM_KEYINPUT_A ) ),
                                SW_EVENT_FRM_KEYINPUT_ALPHA );
                rPg.AddEvent( String( SW_RES( STR_EVENT_FRM_KEYINPUT_NOA ) ),
                                SW_EVENT_FRM_KEYINPUT_NOALPHA );
                rPg.AddEvent( String( SW_RES( STR_EVENT_FRM_RESIZE ) ),
                                SW_EVENT_FRM_RESIZE );
                rPg.AddEvent( String( SW_RES( STR_EVENT_FRM_MOVE ) ),
                                SW_EVENT_FRM_MOVE );
            }
        }
        // kein break;
    case MACASSGN_OLE:              // OLE
        {
            if( !bHtmlMode )
                rPg.AddEvent( String( SW_RES(STR_EVENT_OBJECT_SELECT) ),
                                SW_EVENT_OBJECT_SELECT );
        }
        // kein break;
    case MACASSGN_INETFMT:          // INetFmt-Attribute
        {
            rPg.AddEvent( String( SW_RES(STR_EVENT_MOUSEOVER_OBJECT) ),
                                SFX_EVENT_MOUSEOVER_OBJECT );
            rPg.AddEvent( String( SW_RES(STR_EVENT_MOUSECLICK_OBJECT) ),
                                SFX_EVENT_MOUSECLICK_OBJECT);
            rPg.AddEvent( String( SW_RES(STR_EVENT_MOUSEOUT_OBJECT) ),
                                SFX_EVENT_MOUSEOUT_OBJECT);
        }
        break;
    }
}


BOOL SwMacroAssignDlg::INetFmtDlg( Window* pParent, SwWrtShell& rSh,
                                    SvxMacroItem*& rpINetItem )
{
    BOOL bRet = FALSE;
    SfxItemSet aSet( rSh.GetAttrPool(), RES_FRMMACRO, RES_FRMMACRO );
    SvxMacroItem aItem( RES_FRMMACRO );
    if( !rpINetItem )
        rpINetItem = new SvxMacroItem( RES_FRMMACRO );
    else
        aItem.SetMacroTable( rpINetItem->GetMacroTable() );

    aSet.Put( aItem );

    SwMacroAssignDlg aMacDlg( pParent, aSet, rSh, MACASSGN_INETFMT );
    if( aMacDlg.Execute() == RET_OK )
    {
        const SfxItemSet* pOutSet = aMacDlg.GetOutputItemSet();
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == pOutSet->GetItemState( RES_FRMMACRO, FALSE, &pItem ))
        {
            rpINetItem->SetMacroTable( ((SvxMacroItem*)pItem)->GetMacroTable() );
            bRet = TRUE;
        }
    }
    return bRet;
}


SvStringsDtor* __EXPORT _GetRangeHdl( _SfxMacroTabPage* /*pTbPg*/, const String& rLanguage )
{
    SvStringsDtor* pNew = new SvStringsDtor;

    SfxApplication* pSfxApp = SFX_APP();
    if ( !rLanguage.EqualsAscii(SVX_MACRO_LANGUAGE_JAVASCRIPT) )
    {
        pSfxApp->EnterBasicCall();

        String* pNewEntry = new String( pSfxApp->GetName() );
        pNew->Insert( pNewEntry, pNew->Count() );

        TypeId aType( TYPE( SwDocShell ));
        SfxObjectShell* pDoc = SfxObjectShell::GetFirst( &aType );
        while( pDoc )
        {
            pNewEntry = new String( pDoc->GetTitle() );
            pNew->Insert( pNewEntry, pNew->Count() );
            pDoc = SfxObjectShell::GetNext( *pDoc, &aType );
        }
        pSfxApp->LeaveBasicCall();
    }

    return pNew;
}




