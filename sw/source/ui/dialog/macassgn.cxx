/*************************************************************************
 *
 *  $RCSfile: macassgn.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:34 $
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
#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

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
    SwMacroAssignDlg::AddEvents( *(SfxSmallMacroTabPage*) GetTabPage(), eType );
}


SwMacroAssignDlg::~SwMacroAssignDlg()
{
}


void SwMacroAssignDlg::AddEvents( SfxSmallMacroTabPage& rPg, DlgEventType eType )
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
    SvxMacroItem aItem;
    if( !rpINetItem )
        rpINetItem = new SvxMacroItem;
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


SvStringsDtor* __EXPORT _GetRangeHdl( _SfxMacroTabPage* pTbPg, const String& rLanguage )
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


/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.40  2000/09/18 16:05:20  willem.vandorp
      OpenOffice header added.

      Revision 1.39  2000/04/20 12:55:01  os
      GetName() returns String&

      Revision 1.38  2000/02/11 14:44:39  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.37  1998/07/17 08:17:28  OS
      SfxxSmallMacroTabPage einsetzen #52809#


      Rev 1.36   17 Jul 1998 10:17:28   OS
   SfxxSmallMacroTabPage einsetzen #52809#

      Rev 1.35   10 Jul 1998 11:04:14   MBA
   GetRangeHdl jetzt mit String

      Rev 1.34   28 Nov 1997 17:57:10   MA
   includes

      Rev 1.33   25 Nov 1997 10:43:20   MA
   includes

      Rev 1.32   03 Nov 1997 13:12:26   MA
   precomp entfernt

      Rev 1.31   06 Aug 1997 11:59:46   TRI
   VCL: includes

      Rev 1.30   07 Apr 1997 14:24:32   MH
   chg: header

      Rev 1.29   21 Feb 1997 09:23:42   OS
   ALL-define fuer Rahmenvorlagen

      Rev 1.28   18 Feb 1997 17:07:52   JP
   Erweiterung fuer Events am Frame

      Rev 1.27   28 Jan 1997 16:31:18   OS
   HtmlMode mit GetHtmlMode ermitteln

      Rev 1.26   27 Jan 1997 15:34:46   OS
   InetFmtDlg returnt Bool

      Rev 1.25   08 Jan 1997 11:01:00   OS
   SID_HTML_MODE jetzt UInt16Item

      Rev 1.24   04 Dec 1996 15:04:52   JP
   SW_EVENT -> SVX_EVENT/SFX_EVENT

      Rev 1.23   03 Dec 1996 19:56:32   JP
   Macrozuweisen-TabPage/-Dialog aus dem Writer exportiert, eigene Ableitung fuer setzen der Events

*************************************************************************/


