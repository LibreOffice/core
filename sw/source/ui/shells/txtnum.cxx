/*************************************************************************
 *
 *  $RCSfile: txtnum.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2001-02-23 12:45:29 $
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

#include <hintids.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_NUMITEM_HXX //autogen
#include <svx/numitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen wg. SvxBrushItem
#include <svx/brshitem.hxx>
#endif

#include "cmdid.h"
#include "wrtsh.hxx"
#include "view.hxx"
#include "viewopt.hxx"
#include "wdocsh.hxx"
#include "textsh.hxx"
#include "uiitems.hxx"
#include "num.hxx"

void SwTextShell::ExecEnterNum(SfxRequest &rReq)
{
    //wg. Aufzeichnung schon vor dem evtl. Shellwechsel
    switch(rReq.GetSlot())
    {
    case FN_NUM_NUMBERING_ON:
    {
        if(!GetShell().GetCurNumRule())
            GetShell().NumOn();
        else
            GetShell().DelNumRules();
    }
    break;
    case FN_NUMBER_BULLETS:
    {
        // per default TRUE, damit die Schleife im Dialog richtig arbeitet!
        BOOL bHasChild = TRUE;
        SfxItemSet aSet(GetPool(),
                SID_HTML_MODE, SID_HTML_MODE,
                SID_ATTR_NUMBERING_RULE, SID_PARAM_CUR_NUM_LEVEL,
                0 );
        SwDocShell* pDocSh = GetView().GetDocShell();
        BOOL bHtml = 0 != PTR_CAST(SwWebDocShell, pDocSh);
        const SwNumRule* pCurRule = GetShell().GetCurNumRule();
        if( pCurRule )
        {
            SvxNumRule aRule = pCurRule->MakeSvxNumRule();

            //convert type of linked bitmaps from SVX_NUM_BITMAP to (SVX_NUM_BITMAP|LINK_TOKEN)
            for(USHORT i = 0; i < aRule.GetLevelCount(); i++)
            {
                SvxNumberFormat aFmt(aRule.GetLevel(i));
                if(SVX_NUM_BITMAP == aFmt.GetNumberingType())
                {
                    const SvxBrushItem* pBrush = aFmt.GetBrush();
                    const String* pLinkStr;
                    if(pBrush &&
                        0 != (pLinkStr = pBrush->GetGraphicLink()) &&
                            pLinkStr->Len())
                        aFmt.SetNumberingType(SvxExtNumType(SVX_NUM_BITMAP|LINK_TOKEN));
                    aRule.SetLevel(i, aFmt, aRule.Get(i) != 0);
                }
            }
            if(bHtml)
                aRule.SetFeatureFlag(NUM_ENABLE_EMBEDDED_BMP, FALSE);

            aSet.Put(SvxNumBulletItem(aRule));
            USHORT nLevel = GetShell().GetNumLevel( &bHasChild ) & ~NO_NUMLEVEL;
            if( nLevel < MAXLEVEL )
            {
                nLevel = 1<<nLevel;
                aSet.Put( SfxUInt16Item( SID_PARAM_CUR_NUM_LEVEL, nLevel ));
            }
        }
        else
        {
            SwNumRule aRule( GetShell().GetUniqueNumRuleName() );
            SvxNumRule aSvxRule = aRule.MakeSvxNumRule();
            if(bHtml)
            {
                for( BYTE n = 1; n < MAXLEVEL; ++n )
                {
                    SvxNumberFormat aFmt( aSvxRule.GetLevel( n ) );
                    // 1/2" fuer HTML
                    aFmt.SetLSpace(720);
                    aFmt.SetAbsLSpace(n * 720);
                    aSvxRule.SetLevel( n, aFmt, FALSE );
                }
                aSvxRule.SetFeatureFlag(NUM_ENABLE_EMBEDDED_BMP, FALSE);
            }
            aSet.Put(SvxNumBulletItem(aSvxRule));
        }

        aSet.Put( SfxBoolItem( SID_PARAM_CHILD_LEVELS, bHasChild ));
        aSet.Put( SfxBoolItem( SID_PARAM_NUM_PRESET,FALSE ));

        // vor dem Dialog wird der HtmlMode an der DocShell versenkt
        pDocSh->PutItem(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(pDocSh)));

        SwSvxNumBulletTabDialog* pDlg = new SwSvxNumBulletTabDialog(
                            GetView().GetWindow(), &aSet, GetShell() );

        USHORT nRet = pDlg->Execute();
        if(RET_OK == nRet )
        {
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pDlg->GetOutputItemSet()->GetItemState(
                                    SID_ATTR_NUMBERING_RULE, FALSE, &pItem ))
            {
                SvxNumRule* pSetRule = ((SvxNumBulletItem*)pItem)->GetNumRule();
                pSetRule->UnLinkGraphics();
                SwNumRule aSetRule( pCurRule
                                        ? pCurRule->GetName()
                                        : GetShell().GetUniqueNumRuleName());
                aSetRule.SetSvxRule( *pSetRule, GetShell().GetDoc());
                aSetRule.SetAutoRule( TRUE );
                GetShell().SetCurNumRule( aSetRule );
            }
            // wenn der Dialog mit OK verlassen wurde, aber nichts ausgewaehlt
            // wurde dann muss die Numerierung zumindest eingeschaltet werden,
            // wenn sie das noch nicht ist
            else if( !pCurRule &&
                SFX_ITEM_SET == aSet.GetItemState(
                                    SID_ATTR_NUMBERING_RULE, FALSE, &pItem ))
            {
                SvxNumRule* pSetRule = ((SvxNumBulletItem*)pItem)->GetNumRule();
                SwNumRule aSetRule(GetShell().GetUniqueNumRuleName());
                aSetRule.SetSvxRule(*pSetRule, GetShell().GetDoc());
                aSetRule.SetAutoRule( TRUE );
                GetShell().SetCurNumRule( aSetRule );
            }
        }
        else if(RET_USER == nRet)
            GetShell().DelNumRules();

        delete pDlg;
/*
#else
        SfxItemSet aSet(GetPool(),
                SID_HTML_MODE, SID_HTML_MODE,
                FN_PARAM_ACT_NUMBER,    FN_PARAM_ACT_NUMBER,
                FN_PARAM_CHILD_LEVELS,  FN_PARAM_CHILD_LEVELS,
                FN_PARAM_NUM_PRESET,    FN_PARAM_NUM_PRESET,
                FN_PARAM_ACT_NUMLEVEL,  FN_PARAM_ACT_NUMLEVEL,
                0 );

        // per default TRUE, damit die Schleife im num.cxx richtig arbeitet!
        BOOL bHasChild = TRUE;
        const SwNumRule* pCurRule = GetShell().GetCurNumRule();
        if( pCurRule )
        {
            aSet.Put( SwUINumRuleItem( *pCurRule ));
            USHORT nLevel = GetShell().GetNumLevel( &bHasChild ) & ~NO_NUMLEVEL;
            if( nLevel < MAXLEVEL )
            {
                nLevel = 1<<nLevel;
                aSet.Put( SfxUInt16Item( FN_PARAM_ACT_NUMLEVEL, nLevel ));
            }
        }
        else
        {
            SwNumRule aRule( GetShell().GetUniqueNumRuleName() );
            SwDocShell* pDocSh = GetView().GetDocShell();
            BOOL bHtml = 0 != PTR_CAST(SwWebDocShell, pDocSh);
            for( BYTE n = 0; n < MAXLEVEL; ++n )
            {
                SwNumFmt aFmt( aRule.Get( n ) );
                if(bHtml && n)
                {
                    // 1/2" fuer HTML
                    aFmt.SetLSpace(720);
                    aFmt.SetAbsLSpace(n * 720);
                }
                aRule.Set( n, aFmt );
            }
            aSet.Put( SwUINumRuleItem( aRule ));
        }

        aSet.Put( SfxBoolItem( FN_PARAM_CHILD_LEVELS, bHasChild ));
        aSet.Put( SfxBoolItem( FN_PARAM_NUM_PRESET,FALSE ));

        SwNumBulletTabDialog* pDlg = new SwNumBulletTabDialog(
                            GetView().GetWindow(), &aSet, GetShell() );

        if(RET_OK == pDlg->Execute() )
        {
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pDlg->GetOutputItemSet()->GetItemState(
                                    FN_PARAM_ACT_NUMBER, FALSE, &pItem ))
            {
                SwNumRule* pSetRule = ((SwUINumRuleItem*)pItem)->GetNumRule();
                pSetRule->SetAutoRule( TRUE );
                GetShell().SetCurNumRule( *pSetRule );
            }
            else
                GetShell().DelNumRules();
        }

        delete pDlg;
#endif
*/
    }
    break;
    case FN_NUM_BULLET_ON:
    {
        if ( !GetShell().GetCurNumRule() )
            GetShell().BulletOn();
        else
            GetShell().DelNumRules();

    }
    break;
    default:
        ASSERT(FALSE,  falscher Dispatcher);
        return;
    }
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 17:14:47  hr
    initial import

    Revision 1.45  2000/09/18 16:06:07  willem.vandorp
    OpenOffice header added.

    Revision 1.44  2000/05/30 12:35:15  os
    #74997# fill SvxNumRule with invalid levels

    Revision 1.43  2000/05/26 07:21:33  os
    old SW Basic API Slots removed

    Revision 1.42  2000/02/11 14:58:14  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.41  1999/08/19 14:25:46  OS
    #67374# embedded graphics in numberings


      Rev 1.40   19 Aug 1999 16:25:46   OS
   #67374# embedded graphics in numberings

      Rev 1.39   04 Jun 1999 13:50:58   OS
   #64956# keine vollst. Numerierungen im HTML

      Rev 1.38   15 Mar 1999 23:28:54   JP
   Task #63049#: Numerierung mit rel. Einzuegen

      Rev 1.37   06 Jan 1999 15:53:08   OS
   #58263# Numerierung bei OK ohne Aenderungen mindestens einschalten

      Rev 1.36   02 Dec 1998 09:38:54   OS
   #58263# Entfernen-Button fuer Numerierungen

      Rev 1.35   17 Nov 1998 10:59:00   OS
   #58263# Numerierungs-Tabseiten aus dem Svx

      Rev 1.34   10 Nov 1998 15:24:38   OS
   #59238# neue Defaults fuer HTML-Numerierung

      Rev 1.33   06 Nov 1998 14:42:12   OS
   #57903# NumOff kann weg

      Rev 1.32   03 Nov 1998 07:26:16   OS
   #58263# Numerierungsdialog in den Svx

      Rev 1.31   17 Aug 1998 16:09:10   OS
   GPF nach Shellwechsel waehrend Recording #55041#

      Rev 1.30   18 May 1998 12:13:46   OS
   Multiselektion im Numerierungsdialog

      Rev 1.29   24 Mar 1998 13:46:44   JP
   SwUINumRule: CTORen mit Name oder NumRule, nicht mehr mit NumRulePointer

      Rev 1.28   12 Mar 1998 13:08:08   OS
   Numerierungsbutton nicht mehr gecheckt und nicht disabled

      Rev 1.27   18 Feb 1998 19:58:44   JP
   wenn ueber den Dialog kommt, ist es immer eine AutoRule

      Rev 1.26   08 Dec 1997 16:08:06   OS
   benannte Numerierungen entfernt

      Rev 1.25   03 Dec 1997 17:31:38   OS
   Numerierung und Bullets ueber Button nur noch einschalten

      Rev 1.24   24 Nov 1997 09:47:56   MA
   includes

      Rev 1.23   17 Nov 1997 10:21:08   JP
   Umstellung Numerierung

      Rev 1.22   03 Nov 1997 13:55:44   MA
   precomp entfernt

      Rev 1.21   06 Aug 1997 11:07:54   MH
   chg: header

      Rev 1.20   26 Jun 1997 16:15:46   OS
   Numerierung per Controller anwenden

      Rev 1.19   05 Jun 1997 13:11:52   OS
   Num./Aufzaehlungsbuttons koennen Numerierung auch ausschalten

      Rev 1.18   30 May 1997 13:52:04   OS
   Preset fuer Numerierungsdialog

      Rev 1.17   07 Apr 1997 17:48:34   MH
   chg: header

      Rev 1.16   22 Feb 1997 20:52:26   OS
   ohne Numerierung muss bHasChild auf TRUE gesetzt werden, sonst kommt der Dialog durcheinander

      Rev 1.15   20 Feb 1997 17:19:04   OS
   Numerierung: Flag fuer benutzte Ebenen auswerten

      Rev 1.14   18 Feb 1997 08:54:12   OS
   Numerierungsdialog: auch den Return-Wert des ModalDialogs auswerten

      Rev 1.13   11 Feb 1997 11:53:32   OS
   ItemSet vor dem Dialog fuellen

      Rev 1.12   08 Feb 1997 18:00:34   OS
   NumRule loeschen, wenn kein Item kommt, aber RET_OK

      Rev 1.11   08 Feb 1997 16:01:54   OS
   Online-Layout->Print-Layout

      Rev 1.10   10 Oct 1996 13:18:26   OS
   ItemState immer pruefen

      Rev 1.9   28 Aug 1996 15:55:18   OS
   includes

      Rev 1.8   24 Nov 1995 16:59:32   OM
   PCH->PRECOMPILED

      Rev 1.7   28 Aug 1995 19:11:16   MA
   Renovierung: IDL, Shells, Textshell-Doktrin aufgegeben

      Rev 1.6   24 Aug 1995 14:33:38   MA
   swstddlg -> svxstandarddialog

      Rev 1.5   17 Feb 1995 14:20:56   MS
   Umstellung SwView

      Rev 1.4   18 Jan 1995 13:47:58   MS
   Precomp Hd

      Rev 1.3   18 Jan 1995 13:43:26   MS
   Precomp Hd

      Rev 1.2   25 Oct 1994 18:46:30   ER
   add: PCH

      Rev 1.1   06 Oct 1994 10:52:12   VB
   Segmentierung

      Rev 1.0   15 Sep 1994 15:54:16   VB
   Initial revision.

------------------------------------------------------------------------*/


