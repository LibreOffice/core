/*************************************************************************
 *
 *  $RCSfile: txtnum.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:45:06 $
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
        SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, FN_PARAM_1 , sal_False );
        BOOL bMode = !GetShell().GetCurNumRule();
        if ( pItem )
            bMode = pItem->GetValue();
        else
            rReq.AppendItem( SfxBoolItem( FN_PARAM_1, bMode ) );
        if ( bMode != (GetShell().GetCurNumRule()!=NULL) )
        {
            rReq.Done();
            if( bMode )
                GetShell().NumOn();
            else
                GetShell().DelNumRules();
        }
    }
    break;
    case FN_NUM_BULLET_ON:
    {
        SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, FN_PARAM_1 , sal_False );
        BOOL bMode = !GetShell().GetCurNumRule();
        if ( pItem )
            bMode = pItem->GetValue();
        else
            rReq.AppendItem( SfxBoolItem( FN_PARAM_1, bMode ) );
        if ( bMode != (GetShell().GetCurNumRule()!=NULL) )
        {
            rReq.Done();
            if( bMode )
                GetShell().BulletOn();
            else
                GetShell().DelNumRules();
        }
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
        const SfxPoolItem* pItem;
        if( RET_OK == nRet )
        {
            if( SFX_ITEM_SET == pDlg->GetOutputItemSet()->GetItemState( SID_ATTR_NUMBERING_RULE, FALSE, &pItem ))
            {
                rReq.AppendItem(*pItem);
                rReq.Done();
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
            else if( !pCurRule && SFX_ITEM_SET == aSet.GetItemState( SID_ATTR_NUMBERING_RULE, FALSE, &pItem ))
            {
                rReq.AppendItem( *pItem );
                rReq.Done();
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
    default:
        ASSERT(FALSE,  falscher Dispatcher);
        return;
    }
}


