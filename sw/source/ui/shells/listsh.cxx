/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "cmdid.h"
#include "hintids.hxx"
#include <editeng/sizeitem.hxx>
#include <editeng/brushitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/bindings.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/whiter.hxx>
#include <svl/intitem.hxx>
#include <tools/shl.hxx>
#include <svl/srchitem.hxx>

#include <numrule.hxx>
#include <fmtornt.hxx>
#include "wrtsh.hxx"
#include "swmodule.hxx"
#include "frmatr.hxx"
#include "helpid.h"
#include "globals.hrc"
#include "shells.hrc"
#include "uinums.hxx"
#include "listsh.hxx"
#include "poolfmt.hxx"
#include "view.hxx"
#include "edtwin.hxx"

#define SwListShell
#include <sfx2/msg.hxx>
#include "swslots.hxx"

#include <IDocumentOutlineNodes.hxx>

SFX_IMPL_INTERFACE(SwListShell, SwBaseShell, SW_RES(STR_SHELLNAME_LIST))
{
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_NUM_TOOLBOX));
}


TYPEINIT1(SwListShell,SwBaseShell)





static void lcl_OutlineUpDownWithSubPoints( SwWrtShell& rSh, bool bMove, bool bUp )
{
    const sal_uInt16 nActPos = rSh.GetOutlinePos();
    if ( nActPos < USHRT_MAX && rSh.IsOutlineMovable( nActPos ) )
    {
        rSh.Push();
        rSh.MakeOutlineSel( nActPos, nActPos, sal_True );

        if ( bMove )
        {
            const IDocumentOutlineNodes* pIDoc( rSh.getIDocumentOutlineNodesAccess() );
            const sal_uInt16 nActLevel = static_cast<sal_uInt16>(pIDoc->getOutlineLevel( nActPos ));
            sal_uInt16 nActEndPos = nActPos + 1;
            sal_Int16 nDir = 0;

            if ( !bUp )
            {
                
                while ( nActEndPos < pIDoc->getOutlineNodesCount() &&
                        pIDoc->getOutlineLevel( nActEndPos ) > nActLevel )
                    ++nActEndPos;

                if ( nActEndPos < pIDoc->getOutlineNodesCount() )
                {
                    
                    
                    --nActEndPos;
                    sal_uInt16 nDest = nActEndPos + 2;
                    while ( nDest < pIDoc->getOutlineNodesCount() &&
                            pIDoc->getOutlineLevel( nDest ) > nActLevel )
                        ++nDest;

                    nDir = nDest - 1 - nActEndPos;
                }
            }
            else
            {
                
                if ( nActPos > 0 )
                {
                    --nActEndPos;
                    sal_uInt16 nDest = nActPos - 1;
                    while ( nDest > 0 && pIDoc->getOutlineLevel( nDest ) > nActLevel )
                        --nDest;

                    nDir = nDest - nActPos;
                }
            }

            if ( nDir )
            {
                rSh.MoveOutlinePara( nDir );
                rSh.GotoOutline( nActPos + nDir );
            }
        }
        else
        {
            
            rSh.OutlineUpDown( bUp ? -1 : 1 );
        }

        rSh.ClearMark();
        rSh.Pop( sal_False );
    }
}

void SwListShell::Execute(SfxRequest &rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();
    SwWrtShell& rSh = GetShell();

    
    const SwNumRule* pCurRule = rSh.GetCurNumRule();
    OSL_ENSURE( pCurRule, "SwListShell::Execute without NumRule" );
    bool bOutline = pCurRule && pCurRule->IsOutlineRule();

    switch (nSlot)
    {
        case FN_NUM_BULLET_DOWN:
        case FN_NUM_BULLET_UP:
            {
                SfxViewFrame * pFrame = GetView().GetViewFrame();

                rReq.Done();
                rSh.NumUpDown( nSlot == FN_NUM_BULLET_DOWN );
                pFrame->GetBindings().Invalidate( SID_TABLE_CELL ); 
            }
            break;

        case FN_NUM_BULLET_NEXT:
            rSh.GotoNextNum();
            rReq.Done();
            break;

        case FN_NUM_BULLET_NONUM:
            rSh.NoNum();
            rReq.Done();
            break;

        case FN_NUM_BULLET_OFF:
        {
            rReq.Ignore();
            SfxRequest aReq( GetView().GetViewFrame(), FN_NUM_BULLET_ON );
            aReq.AppendItem( SfxBoolItem( FN_PARAM_1, false ) );
            aReq.Done();
            rSh.DelNumRules();
            break;
        }

        case FN_NUM_BULLET_OUTLINE_DOWN:
            if ( bOutline )
                lcl_OutlineUpDownWithSubPoints( rSh, false, false );
            else
                rSh.MoveNumParas(false, false);
            rReq.Done();
            break;

        case FN_NUM_BULLET_OUTLINE_MOVEDOWN:
            if ( bOutline )
                lcl_OutlineUpDownWithSubPoints( rSh, true, false );
            else
                rSh.MoveNumParas(true, false);
            rReq.Done();
            break;

        case FN_NUM_BULLET_OUTLINE_MOVEUP:
            if ( bOutline )
                lcl_OutlineUpDownWithSubPoints( rSh, true, true );
            else
                rSh.MoveNumParas(true, true);
            rReq.Done();
            break;

        case FN_NUM_BULLET_OUTLINE_UP:
            if ( bOutline )
                lcl_OutlineUpDownWithSubPoints( rSh, false, true );
            else
                rSh.MoveNumParas(false, true);
            rReq.Done();
            break;

        case FN_NUM_BULLET_PREV:
            rSh.GotoPrevNum();
            rReq.Done();
            break;

        case FN_NUM_OR_NONUM:
        {
            sal_Bool bApi = rReq.IsAPI();
            sal_Bool bDelete = !rSh.IsNoNum(!bApi);
            if(pArgs )
                bDelete = ((SfxBoolItem &)pArgs->Get(rReq.GetSlot())).GetValue();
            rSh.NumOrNoNum( bDelete, !bApi );
            rReq.AppendItem( SfxBoolItem( nSlot, bDelete ) );
            rReq.Done();
        }
        break;
        default:
            OSL_ENSURE(!this, "wrong dispatcher");
            return;
    }
}

void SwListShell::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    SwWrtShell& rSh = GetShell();
    sal_uInt8 nCurrentNumLevel = rSh.GetNumLevel();
    while ( nWhich )
    {
        switch( nWhich )
        {
            case FN_NUM_OR_NONUM:
                rSet.Put(SfxBoolItem(nWhich, GetShell().IsNoNum(sal_False)));
            break;
            case FN_NUM_BULLET_OUTLINE_UP:
            case FN_NUM_BULLET_UP:
                if(!nCurrentNumLevel)
                    rSet.DisableItem(nWhich);
            break;
            case FN_NUM_BULLET_OUTLINE_DOWN :
            {
                sal_uInt8 nUpper = 0;
                sal_uInt8 nLower = 0;
                rSh.GetCurrentOutlineLevels( nUpper, nLower );
                if(nLower == (MAXLEVEL - 1))
                    rSet.DisableItem(nWhich);
            }
            break;
            case FN_NUM_BULLET_DOWN:
                if(nCurrentNumLevel == (MAXLEVEL - 1))
                    rSet.DisableItem(nWhich);
            break;

            case FN_NUM_BULLET_NONUM:
                if ( rSh.CrsrInsideInputFld() )
                {
                    rSet.DisableItem(nWhich);
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

SwListShell::SwListShell(SwView &_rView) :
    SwBaseShell(_rView)
{
    SetName(OUString("List"));
    SetHelpId(SW_LISTSHELL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
