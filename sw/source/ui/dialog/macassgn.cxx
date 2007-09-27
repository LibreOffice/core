/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macassgn.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:37:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#include "swtypes.hxx"
#include "wrtsh.hxx"
#include "viewopt.hxx"
#include "macassgn.hxx"
#include "swevent.hxx"
#include "docsh.hxx"
#include "globals.hrc"


SvStringsDtor* __EXPORT _GetRangeHdl( _SfxMacroTabPage*, const String& );


SwMacroAssignDlg::SwMacroAssignDlg( Window* pParent, SfxItemSet& rSet,
                                        DlgEventType eType )
    : SfxMacroAssignDlg( pParent, rSet )
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

    SwMacroAssignDlg aMacDlg( pParent, aSet, MACASSGN_INETFMT );
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




