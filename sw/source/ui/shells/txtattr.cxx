/*************************************************************************
 *
 *  $RCSfile: txtattr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-23 20:08:52 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXITEMITER_HXX
#include <svtools/itemiter.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif


#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _UIITEMS_HXX
#include <uiitems.hxx>
#endif
#ifndef _TEXTSH_HXX
#include <textsh.hxx>
#endif
#ifndef _DRPCPS_HXX
#include <drpcps.hxx>
#endif
#ifndef _NUM_HXX
#include <num.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _GLOBALS_H
#include <globals.h>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif

const SwTwips lFontInc = 2 * 20;           // ==> PointToTwips(2)
const SwTwips lFontMaxSz = 72 * 20;        // ==> PointToTwips(72)




void SwTextShell::ExecCharAttr(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    const SfxItemSet  *pArgs   = rReq.GetArgs();
          int          eState = STATE_TOGGLE;
    USHORT nWhich = rReq.GetSlot();

    if(pArgs )
    {
        const SfxPoolItem* pItem;
        pArgs->GetItemState(nWhich, FALSE, &pItem);
        eState =  ((const SfxBoolItem &) pArgs->
                                Get( nWhich )).GetValue() ? STATE_ON : STATE_OFF;
    }


    SfxItemSet aSet( GetPool(), RES_CHRATR_BEGIN, RES_CHRATR_END-1 );
    if (STATE_TOGGLE == eState)
        rSh.GetAttr( aSet );

    switch ( nWhich )
    {
        case FN_SET_SUB_SCRIPT:
        case FN_SET_SUPER_SCRIPT:
        {
            SvxEscapement eEscape;
            switch (eState)
            {
            case STATE_TOGGLE:
            {
                short nTmpEsc = ((const SvxEscapementItem&)
                            aSet.Get( RES_CHRATR_ESCAPEMENT )).GetEsc();
                eEscape = nWhich == FN_SET_SUPER_SCRIPT ?
                                SVX_ESCAPEMENT_SUPERSCRIPT:
                                SVX_ESCAPEMENT_SUBSCRIPT;
                if( nWhich == FN_SET_SUB_SCRIPT && nTmpEsc < 0 ||
                            nWhich == FN_SET_SUPER_SCRIPT && nTmpEsc > 0 )
                    eEscape = SVX_ESCAPEMENT_OFF;

                SfxBindings& rBind = GetView().GetViewFrame()->GetBindings();
                if( nWhich == FN_SET_SUB_SCRIPT )
                    rBind.SetState( SfxBoolItem( FN_SET_SUPER_SCRIPT,
                                                                    FALSE ) );
                else
                    rBind.SetState( SfxBoolItem( FN_SET_SUB_SCRIPT,
                                                                    FALSE ) );

            }
            break;
            case STATE_ON:
                eEscape = nWhich == FN_SET_SUPER_SCRIPT ?
                                SVX_ESCAPEMENT_SUPERSCRIPT:
                                SVX_ESCAPEMENT_SUBSCRIPT;
                break;
            case STATE_OFF:
                eEscape = SVX_ESCAPEMENT_OFF;
                break;
            }
            SvxEscapementItem aEscape( eEscape );
            if(eEscape == SVX_ESCAPEMENT_SUPERSCRIPT)
                aEscape.GetEsc() = DFLT_ESC_AUTO_SUPER;
            else if(eEscape == SVX_ESCAPEMENT_SUBSCRIPT)
                aEscape.GetEsc() = DFLT_ESC_AUTO_SUB;
            if(eState != STATE_OFF )
            {
                if(eEscape == FN_SET_SUPER_SCRIPT)
                    aEscape.GetEsc() *= -1;
            }
            rSh.SetAttr( aEscape );
        }
        break;

        case FN_UPDATE_STYLE_BY_EXAMPLE:
            rSh.QuickUpdateStyle();
            break;
        case FN_UNDERLINE_DOUBLE:
        {
            FontUnderline eUnderline =  ((const SvxUnderlineItem&)
                            aSet.Get(RES_CHRATR_UNDERLINE)).GetUnderline();
            switch( eState )
            {
                case STATE_TOGGLE:
                    eUnderline = eUnderline == UNDERLINE_DOUBLE ?
                        UNDERLINE_NONE :
                            UNDERLINE_DOUBLE;
                break;
                case STATE_ON:
                    eUnderline = UNDERLINE_DOUBLE;
                break;
                case STATE_OFF:
                    eUnderline = UNDERLINE_NONE;
                break;
            }
            rSh.SetAttr(SvxUnderlineItem(eUnderline));
        }
        break;
        default:
            ASSERT(FALSE, falscher Dispatcher);
            return;
    }
}


void SwTextShell::ExecCharAttrArgs(SfxRequest &rReq)
{
    int nSlot = rReq.GetSlot();
    const SfxItemSet* pArgs = rReq.GetArgs();
    BOOL bArgs = pArgs != 0 && pArgs->Count() > 0;
    int bGrow = FALSE;
    SwWrtShell& rWrtSh = GetShell();
    SwTxtFmtColl* pColl = 0;

    // nur gesetzt, wenn gesamter Absatz selektiert ist und AutoUpdateFmt gesetzt ist
    if( rWrtSh.HasSelection() && rWrtSh.IsSelFullPara() )
    {
        pColl = rWrtSh.GetCurTxtFmtColl();
        if(pColl && !pColl->IsAutoUpdateFmt())
            pColl = 0;
    }
    SfxItemPool& rPool = GetPool();
    USHORT nWhich = rPool.GetWhich(nSlot);
    switch ( nSlot )
    {
        case FN_TXTATR_INET:
        // Sonderbehandlung der PoolId des SwFmtInetFmt
        if(bArgs)
        {
            const SfxPoolItem& rItem = pArgs->Get(nWhich );

            SwFmtINetFmt aINetFmt((const SwFmtINetFmt&)rItem);
            if( USHRT_MAX == aINetFmt.GetVisitedFmtId() )
            {
                aINetFmt.SetVisitedFmtId(
                        rWrtSh.GetPoolId( aINetFmt.GetVisitedFmt(), GET_POOLID_CHRFMT));
            }
            if( USHRT_MAX == aINetFmt.GetINetFmtId() )
            {
                aINetFmt.SetINetFmtId(
                        rWrtSh.GetPoolId( aINetFmt.GetINetFmt(), GET_POOLID_CHRFMT));
            }

            if ( pColl )
                pColl->SetAttr( aINetFmt );
            else rWrtSh.SetAttr( aINetFmt );
        }
        break;

        case FN_GROW_FONT_SIZE:
            bGrow = TRUE;
            // kein break !!
        case FN_SHRINK_FONT_SIZE:
        {
            SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONTHEIGHT, rPool );
            rWrtSh.GetAttr( aSetItem.GetItemSet() );
            SfxItemSet aAttrSet( rPool, aSetItem.GetItemSet().GetRanges() );

            const SfxPoolItem* pI;
            static const USHORT aScrTypes[] = {
                SCRIPTTYPE_LATIN, SCRIPTTYPE_ASIAN, SCRIPTTYPE_COMPLEX, 0 };
            USHORT nScriptType = rWrtSh.GetScriptType();
            for( const USHORT* pScrpTyp = aScrTypes; *pScrpTyp; ++pScrpTyp )
                if( ( nScriptType & *pScrpTyp ) &&
                    0 != ( pI = aSetItem.GetItemOfScript( *pScrpTyp )))
                {
                    SvxFontHeightItem aSize( *(const SvxFontHeightItem*)pI );
                    SwTwips lSize = (SwTwips) aSize.GetHeight();

                    if (bGrow)
                    {
                        if( lSize == lFontMaxSz )
                            break;      // das wars, hoeher gehts nicht
                        if( ( lSize += lFontInc ) > lFontMaxSz )
                            lSize = lFontMaxSz;
                    }
                    else
                    {
                        if( 4 == lSize )
                            break;
                        if( ( lSize -= lFontInc ) < 4 )
                            lSize = 4;
                    }
                    aSize.SetHeight( lSize );
                    aAttrSet.Put( aSize );
                }
                if( aAttrSet.Count() )
                {
                    if( pColl )
                        pColl->SetAttr( aAttrSet );
                    else
                        rWrtSh.SetAttr( aAttrSet );
                }
        }
        break;

        default:
            ASSERT(FALSE, falscher Dispatcher);
            return;
    }
}



#ifdef CFRONT

void lcl_SetAdjust(SvxAdjust eAdjst, SfxItemSet& rSet)
{
    rSet.Put(SvxAdjustItem(eAdjst,RES_PARATR_ADJUST ));
}



void lcl_SetLineSpace(BYTE ePropL,SfxItemSet& rSet)
{
    SvxLineSpacingItem aLineSpacing(ePropL, RES_PARATR_LINESPACING );
    aLineSpacing.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
    if( 100 == ePropL )
        aLineSpacing.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
    else
        aLineSpacing.SetPropLineSpace(ePropL);
    rSet.Put( aLineSpacing );
}



void SwTextShell::ExecParaAttr(SfxRequest &rReq)
{
    // gleiche beide Attribute holen, ist nicht teuerer !!
    SfxItemSet aSet( GetPool(), RES_PARATR_LINESPACING, RES_PARATR_ADJUST );

    switch (rReq.GetSlot())
    {
    case FN_SET_LEFT_PARA:          lcl_SetAdjust(ADJLEFT,aSet);    break;
    case FN_SET_RIGHT_PARA:         lcl_SetAdjust(ADJRIGHT,aSet);   break;
    case FN_SET_CENTER_PARA:        lcl_SetAdjust(ADJCENTER,aSet);  break;
    case SID_ATTR_PARA_ADJUST_BLOCK:lcl_SetAdjust(ADJBLOCK,aSet);   break;

    case FN_SET_LINE_SPACE_1:   lcl_SetLineSpace(100,aSet);     break;
    case FN_SET_LINE_SPACE_15:  lcl_SetLineSpace(150,aSet);     break;
    case FN_SET_LINE_SPACE_2:   lcl_SetLineSpace(200,aSet);     break;

    default:
        DBG_ERROR("SwTextShell::ExecParaAttr falscher Dispatcher");
        return;
    }
    SwWrtShell& rWrtSh = GetShell();
    SwTxtFmtColl* pColl = rWrtSh.GetCurTxtFmtColl();
    if(pColl && pColl->IsAutoUpdateFmt())
    {
        rWrtSh.AutoUpdatePara(pColl, *pSet);
    }
    else
        rWrtSh.SetAttr( aSet );
}

#else



void SwTextShell::ExecParaAttr(SfxRequest &rReq)
{
    SvxAdjust eAdjst;
    BYTE ePropL;
    const SfxItemSet* pArgs = rReq.GetArgs();

    // gleich beide Attribute holen, ist nicht teuerer !!
    SfxItemSet aSet( GetPool(), RES_PARATR_LINESPACING, RES_PARATR_ADJUST );

    USHORT nSlot = rReq.GetSlot();
    switch (nSlot)
    {
        case SID_ATTR_PARA_ADJUST:
            if(pArgs &&  SFX_ITEM_SET == pArgs->GetItemState( RES_PARATR_ADJUST) )
            {
                eAdjst = ((const SvxAdjustItem&)pArgs->Get(
                                RES_PARATR_ADJUST)).GetAdjust();
                goto SET_ADJUST;
            }
        break;
        case SID_ATTR_PARA_ADJUST_LEFT:     eAdjst =  SVX_ADJUST_LEFT;      goto SET_ADJUST;
        case SID_ATTR_PARA_ADJUST_RIGHT:    eAdjst =  SVX_ADJUST_RIGHT;     goto SET_ADJUST;
        case SID_ATTR_PARA_ADJUST_CENTER:   eAdjst =  SVX_ADJUST_CENTER;    goto SET_ADJUST;
        case SID_ATTR_PARA_ADJUST_BLOCK:    eAdjst =  SVX_ADJUST_BLOCK;     goto SET_ADJUST;
SET_ADJUST:
        {
            aSet.Put(SvxAdjustItem(eAdjst,RES_PARATR_ADJUST ));
        }
        break;

        case SID_ATTR_PARA_LINESPACE:
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState( GetPool().GetWhich(nSlot) ))
            {
                SvxLineSpacingItem aLineSpace = (const SvxLineSpacingItem&)pArgs->Get(
                                                            GetPool().GetWhich(nSlot));
                aSet.Put( aLineSpace );
            }
        break;
        case SID_ATTR_PARA_LINESPACE_10:    ePropL = 100;   goto SET_LINESPACE;
        case SID_ATTR_PARA_LINESPACE_15:    ePropL = 150;   goto SET_LINESPACE;
        case SID_ATTR_PARA_LINESPACE_20:    ePropL = 200;   goto SET_LINESPACE;

SET_LINESPACE:
        {

            SvxLineSpacingItem aLineSpacing(ePropL, RES_PARATR_LINESPACING );
            aLineSpacing.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            if( 100 == ePropL )
                aLineSpacing.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
            else
                aLineSpacing.SetPropLineSpace(ePropL);
            aSet.Put( aLineSpacing );
        }
        break;

        default:
            ASSERT(FALSE, falscher Dispatcher);
            return;
    }
    SwWrtShell& rWrtSh = GetShell();
    SwTxtFmtColl* pColl = rWrtSh.GetCurTxtFmtColl();
    if(pColl && pColl->IsAutoUpdateFmt())
    {
        rWrtSh.AutoUpdatePara(pColl, aSet);
    }
    else
        rWrtSh.SetAttr( aSet );
}

#endif



void SwTextShell::ExecParaAttrArgs(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem *pItem = 0;

    USHORT nSlot = rReq.GetSlot();
    if(pArgs)
        pArgs->GetItemState(GetPool().GetWhich(nSlot), FALSE, &pItem);
    switch ( nSlot )
    {
        case FN_FORMAT_DROPCAPS:
        {
            if(pItem)
                rSh.SetAttr(*pItem);
            else
            {
                SfxItemSet aSet(GetPool(), RES_PARATR_DROP, RES_PARATR_DROP,
                                           HINT_END, HINT_END);
                rSh.GetAttr(aSet);
                SwDropCapsDlg *pDlg = new SwDropCapsDlg(GetView().GetWindow(), aSet);
                if (pDlg->Execute() == RET_OK)
                {
                    rSh.StartAction();
                    rSh.StartUndo( UNDO_START );
                    if ( SFX_ITEM_SET == aSet.GetItemState(HINT_END,FALSE,&pItem) )
                    {
                        if ( ((SfxStringItem*)pItem)->GetValue().Len() )
                            rSh.ReplaceDropTxt(((SfxStringItem*)pItem)->GetValue());
                    }
                    rSh.SetAttr(*pDlg->GetOutputItemSet());
                    rSh.StartUndo( UNDO_END );
                    rSh.EndAction();
                }
                delete pDlg;
            }
        }
         break;
        case SID_ATTR_PARA_PAGEBREAK:
            if(pItem)
            {
                rSh.SetAttr( *pItem );
            }
        break;
        case SID_ATTR_PARA_MODEL:
        {
            if(pItem)
            {
                SfxItemSet aCoreSet( GetPool(),
                    RES_PAGEDESC,   RES_PAGEDESC,
                    SID_ATTR_PARA_MODEL, SID_ATTR_PARA_MODEL, 0);
                aCoreSet.Put(*pItem);
                SfxToSwPageDescAttr( rSh, aCoreSet);
                rSh.SetAttr(aCoreSet);
            }
        }
        break;
        default:
            ASSERT(FALSE, falscher Dispatcher);
            return;
    }
}



void SwTextShell::GetAttrState(SfxItemSet &rSet)
{
    SwWrtShell &rSh = GetShell();
    SfxItemPool& rPool = GetPool();
    SfxItemSet aCoreSet(rPool, aTxtFmtCollSetRange);
    rSh.GetAttr(aCoreSet);  // *alle* Textattribute von der Core erfragen

    SfxWhichIter aIter(rSet);
    register USHORT nSlot = aIter.FirstWhich();
    register int bFlag;
    SfxBoolItem aFlagItem;
    const SfxPoolItem* pItem = 0;
    int eAdjust = -1;   // Illegaler Wert, um DONTCARE zu erkennen
    SfxItemState eState = aCoreSet.GetItemState(RES_PARATR_ADJUST, FALSE, &pItem);

    if( SFX_ITEM_DEFAULT == eState )
        pItem = &rPool.GetDefaultItem(RES_PARATR_ADJUST);
    if( SFX_ITEM_DEFAULT <= eState )
        eAdjust = (int)(( SvxAdjustItem* ) pItem)->GetAdjust();

    short nEsc = 0;
    eState =  aCoreSet.GetItemState(RES_CHRATR_ESCAPEMENT, FALSE, &pItem);
    if( SFX_ITEM_DEFAULT == eState )
        pItem = &rPool.GetDefaultItem(RES_CHRATR_ESCAPEMENT);
    if( eState >= SFX_ITEM_DEFAULT )
        nEsc = ((SvxEscapementItem* )pItem)->GetEsc();

    USHORT nLineSpace = 0;
    eState =  aCoreSet.GetItemState(RES_PARATR_LINESPACING, FALSE, &pItem);
    if( SFX_ITEM_DEFAULT == eState )
        pItem = &rPool.GetDefaultItem(RES_PARATR_LINESPACING);
    if( SFX_ITEM_DEFAULT <= eState &&
            ((SvxLineSpacingItem* )pItem)->GetLineSpaceRule() == SVX_LINE_SPACE_AUTO )
    {
        if(SVX_INTER_LINE_SPACE_OFF ==
                    ((SvxLineSpacingItem* )pItem)->GetInterLineSpaceRule())
            nLineSpace = 100;
        else
            nLineSpace = ((SvxLineSpacingItem* )pItem)->GetPropLineSpace();
    }

    while (nSlot)
    {
        switch(nSlot)
        {
            case FN_SET_SUPER_SCRIPT:
                    bFlag = 0 < nEsc;
                break;
            case FN_SET_SUB_SCRIPT:
                    bFlag = 0 > nEsc;
                break;
            case SID_ATTR_PARA_ADJUST_LEFT:
                if (eAdjust == -1)
                {
                    rSet.InvalidateItem( nSlot );
                    nSlot = 0;
                }
                else
                    bFlag = SVX_ADJUST_LEFT == eAdjust;
                break;
            case SID_ATTR_PARA_ADJUST_RIGHT:
                if (eAdjust == -1)
                {
                    rSet.InvalidateItem( nSlot );
                    nSlot = 0;
                }
                else
                    bFlag = SVX_ADJUST_RIGHT == eAdjust;
                break;
            case SID_ATTR_PARA_ADJUST_CENTER:
                if (eAdjust == -1)
                {
                    rSet.InvalidateItem( nSlot );
                    nSlot = 0;
                }
                else
                    bFlag = SVX_ADJUST_CENTER == eAdjust;
                break;
            case SID_ATTR_PARA_ADJUST_BLOCK:
            {
                if (eAdjust == -1)
                {
                    rSet.InvalidateItem( nSlot );
                    nSlot = 0;
                }
                else
                {
                    bFlag = SVX_ADJUST_BLOCK == eAdjust;
                    USHORT nHtmlMode = GetHtmlMode(rSh.GetView().GetDocShell());
                    if((nHtmlMode & HTMLMODE_ON) && !(nHtmlMode & (HTMLMODE_FULL_STYLES|HTMLMODE_FIRSTLINE) ))
                    {
                        rSet.DisableItem( nSlot );
                        nSlot = 0;
                    }
                }
            }
            break;
            case SID_ATTR_PARA_LINESPACE_10:
                bFlag = nLineSpace == 100;
            break;
            case SID_ATTR_PARA_LINESPACE_15:
                bFlag = nLineSpace == 150;
            break;
            case SID_ATTR_PARA_LINESPACE_20:
                bFlag = nLineSpace == 200;
            break;
            case FN_GROW_FONT_SIZE:
            case FN_SHRINK_FONT_SIZE:
            {
                SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONTHEIGHT,
                                            *rSet.GetPool() );
                aSetItem.GetItemSet().Put( aCoreSet, FALSE );
                if( !aSetItem.GetItemOfScript( rSh.GetScriptType() ))
                    rSet.DisableItem( nSlot );
                nSlot = 0;
            }
            break;
            case FN_UNDERLINE_DOUBLE:
            {
                eState = aCoreSet.GetItemState(RES_CHRATR_UNDERLINE);
                if( eState >= SFX_ITEM_DEFAULT )
                {
                    FontUnderline eUnderline =  ((const SvxUnderlineItem&)
                            aCoreSet.Get(RES_CHRATR_UNDERLINE)).GetUnderline();
                    rSet.Put(SfxBoolItem(nSlot, eUnderline == UNDERLINE_DOUBLE));
                }
                else
                    rSet.InvalidateItem(nSlot);
                nSlot = 0;
            }
            break;
            case SID_ATTR_PARA_ADJUST:
                if (eAdjust == -1)
                    rSet.InvalidateItem( nSlot );
                else
                    rSet.Put(SvxAdjustItem((SvxAdjust)eAdjust, SID_ATTR_PARA_ADJUST ));
                nSlot = 0;
            break;

            case SID_ATTR_CHAR_LANGUAGE:
            case SID_ATTR_CHAR_KERNING:
            case RES_PARATR_DROP:
            {
#ifdef DEBUG
                const SfxPoolItem& rItem = aCoreSet.Get(GetPool().GetWhich(nSlot), TRUE);
                rSet.Put(rItem);
#else
                rSet.Put(aCoreSet.Get( GetPool().GetWhich(nSlot), TRUE));
#endif
                nSlot = 0;
            }
            break;
            case SID_ATTR_PARA_MODEL:
            {
                SfxItemSet aTemp(GetPool(),
                        RES_PAGEDESC,RES_PAGEDESC,
                        SID_ATTR_PARA_MODEL,SID_ATTR_PARA_MODEL,
                        0L);
                aTemp.Put(aCoreSet);
                ::SwToSfxPageDescAttr(aTemp);
                rSet.Put(aTemp.Get(SID_ATTR_PARA_MODEL));
                nSlot = 0;
            }
            break;
            case RES_TXTATR_INETFMT:
            {
                SfxItemSet aSet(GetPool(), RES_TXTATR_INETFMT, RES_TXTATR_INETFMT);
                rSh.GetAttr(aSet);
#ifdef DEBUG
                const SfxPoolItem& rItem = aSet.Get(RES_TXTATR_INETFMT, TRUE);
                rSet.Put(rItem);
#else
                rSet.Put(aSet.Get( RES_TXTATR_INETFMT, TRUE));
#endif
                nSlot = 0;
            }
            break;
            default:
            // Nichts tun
            nSlot = 0;
            break;

        }
        if( nSlot )
        {
            aFlagItem.SetWhich( nSlot );
            aFlagItem.SetValue( bFlag );
            rSet.Put( aFlagItem );
        }
        nSlot = aIter.NextWhich();
    }

    rSet.Put(aCoreSet,FALSE);
}
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.2  2000/11/13 13:21:12  jp
    support CJK attributes

    Revision 1.1.1.1  2000/09/18 17:14:47  hr
    initial import

    Revision 1.122  2000/09/18 16:06:07  willem.vandorp
    OpenOffice header added.

    Revision 1.121  2000/09/07 15:59:30  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.120  2000/08/31 11:32:12  jp
    add missing include

    Revision 1.119  2000/05/26 07:21:33  os
    old SW Basic API Slots removed

    Revision 1.118  2000/05/10 11:53:02  os
    Basic API removed

    Revision 1.117  2000/02/11 14:58:08  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.116  1999/01/04 13:29:16  OS
    #60575# Shrink/GrowFontSize: keinen falschen Status liefern


      Rev 1.115   04 Jan 1999 14:29:16   OS
   #60575# Shrink/GrowFontSize: keinen falschen Status liefern

      Rev 1.114   29 Nov 1997 15:54:02   MA
   includes

      Rev 1.113   24 Nov 1997 09:47:40   MA
   includes

      Rev 1.112   03 Nov 1997 13:55:52   MA
   precomp entfernt

      Rev 1.111   22 Oct 1997 12:31:56   OS
   StateMethod fuer SID_ATTR_PARA_MODEL #44910#

      Rev 1.110   04 Sep 1997 17:12:04   MA
   includes

      Rev 1.109   15 Aug 1997 11:48:40   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.108   11 Aug 1997 10:06:46   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.107   25 Jun 1997 13:49:50   OS
   Ids des SwFmtINetFmt korrigieren #40977#

      Rev 1.106   17 Jun 1997 16:11:58   MA
   DrawTxtShell nicht von BaseShell ableiten + Opts

      Rev 1.105   28 May 1997 15:06:32   OM
   #40067# DontCare-State fuer Absatzausrichtung beruecksichtigen

      Rev 1.104   24 Mar 1997 15:51:22   OS
   Blocksatzerweiterung fuer Html

      Rev 1.103   10 Mar 1997 12:01:20   NF
   OS2-Anpassung ...

      Rev 1.102   07 Mar 1997 08:09:38   OS
   AutoUpdate fuer Vorlagen: harte Attribute loeschen

      Rev 1.101   06 Mar 1997 16:46:30   OS
   AutoUpdate fuer Absatz- und Rahmenvorlagen

      Rev 1.100   05 Mar 1997 21:00:24   HJS
   muss 357

      Rev 1.99   19 Feb 1997 19:10:14   MA
   fix: DropCap, Text per Item

      Rev 1.98   27 Jan 1997 16:30:38   OS
   HtmlMode entfernt

      Rev 1.97   16 Dec 1996 19:10:38   HJS
   includes

      Rev 1.96   14 Dec 1996 14:48:46   OS
   kein Blocksatz im HTML-Mode

      Rev 1.95   22 Nov 1996 16:24:46   NF
   zwei identische case-bl”cke in einer switch-anweisung -> doppelten entfernt..

      Rev 1.94   22 Nov 1996 14:53:34   OS
   FN_SET_JUSTIFY_PARA -> SID_ATTR_PARA_ADJUST_BLOCK

      Rev 1.93   14 Nov 1996 19:17:56   OS
   Umstellung SlotIds

      Rev 1.92   24 Sep 1996 06:58:14   OS
   Hoch-/Tiefstellung auf automatisch

      Rev 1.91   09 Sep 1996 15:11:10   OS
   SID_APPLY_TEMPLATE mit WrtShell

      Rev 1.90   28 Aug 1996 15:55:16   OS
   includes

      Rev 1.89   15 Aug 1996 18:17:16   OS
   URL als Attribut - auch fuer die API

      Rev 1.88   26 Jun 1996 15:20:54   OS
   Aufruf von Dispatcher.Execute an 324 angepasst

      Rev 1.87   25 Jun 1996 19:52:10   HJS
   includes

      Rev 1.86   22 Mar 1996 15:34:38   TRI
   sfxiiter.hxx included

      Rev 1.85   21 Mar 1996 11:58:26   MA
   Umstellung SV311

      Rev 1.84   20 Feb 1996 17:02:56   JP
   Umbau SfxItemPool -> SwAttrPool

      Rev 1.83   15 Feb 1996 18:04:58   OS
   UnderlineDouble und Sub/Superscript arbeiten mit BoolItems

      Rev 1.82   19 Jan 1996 14:39:54   OS
   Cast fuer SvxLanguageItem nicht noetig

      Rev 1.81   20 Dec 1995 15:50:26   OS
   Status fuer Dropcaps gerichtet

      Rev 1.80   14 Dec 1995 17:25:52   OS
   Standard-Adjust auf LEFT

------------------------------------------------------------------------*/


