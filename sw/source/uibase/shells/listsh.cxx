/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <cmdid.h>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/eitem.hxx>
#include <svl/whiter.hxx>
#include <osl/diagnose.h>

#include <numrule.hxx>
#include <wrtsh.hxx>
#include <listsh.hxx>
#include <view.hxx>

#define ShellClass_SwListShell
#include <sfx2/msg.hxx>
#include <swslots.hxx>

#include <IDocumentOutlineNodes.hxx>

SFX_IMPL_INTERFACE(SwListShell, SwBaseShell)

void SwListShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Invisible, ToolbarId::Num_Toolbox);
}


// #i35572# Functionality of Numbering/Bullet toolbar
// for outline numbered paragraphs should match the functions for outlines
// available in the navigator. Therefore the code in the following
// function is quite similar the code in SwContentTree::ExecCommand.
static void lcl_OutlineUpDownWithSubPoints( SwWrtShell& rSh, bool bMove, bool bUp )
{
    const SwOutlineNodes::size_type nActPos = rSh.GetOutlinePos();
    if ( !(nActPos < SwOutlineNodes::npos && rSh.IsOutlineMovable( nActPos )) )
        return;

    rSh.Push();
    rSh.MakeOutlineSel( nActPos, nActPos, true );

    if ( bMove )
    {
        const IDocumentOutlineNodes* pIDoc( rSh.getIDocumentOutlineNodesAccess() );
        const int nActLevel = pIDoc->getOutlineLevel( nActPos );
        SwOutlineNodes::difference_type nDir = 0;

        if ( !bUp )
        {
            // Move down with subpoints:
            SwOutlineNodes::size_type nActEndPos = nActPos + 1;
            while ( nActEndPos < pIDoc->getOutlineNodesCount() &&
                   (!pIDoc->isOutlineInLayout(nActEndPos, *rSh.GetLayout())
                    || nActLevel < pIDoc->getOutlineLevel(nActEndPos)))
            {
                ++nActEndPos;
            }

            if ( nActEndPos < pIDoc->getOutlineNodesCount() )
            {
                // The current subpoint which should be moved
                // starts at nActPos and ends at nActEndPos - 1
                --nActEndPos;
                SwOutlineNodes::size_type nDest = nActEndPos + 2;
                while ( nDest < pIDoc->getOutlineNodesCount() &&
                       (!pIDoc->isOutlineInLayout(nDest, *rSh.GetLayout())
                        || nActLevel < pIDoc->getOutlineLevel(nDest)))
                {
                    ++nDest;
                }

                nDir = nDest - 1 - nActEndPos;
            }
        }
        else
        {
            // Move up with subpoints:
            if ( nActPos > 0 )
            {
                SwOutlineNodes::size_type nDest = nActPos - 1;
                while (nDest > 0 &&
                       (!pIDoc->isOutlineInLayout(nDest, *rSh.GetLayout())
                        || nActLevel < pIDoc->getOutlineLevel(nDest)))
                {
                    --nDest;
                }

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
        // Up/down with subpoints:
        rSh.OutlineUpDown( bUp ? -1 : 1 );
    }

    rSh.ClearMark();
    rSh.Pop(SwCursorShell::PopMode::DeleteCurrent);
}

void SwListShell::Execute(SfxRequest &rReq)
{
    const sal_uInt16 nSlot = rReq.GetSlot();
    SwWrtShell& rSh = GetShell();

    // #i35572#
    const SwNumRule* pCurRule = rSh.GetNumRuleAtCurrCursorPos();
    OSL_ENSURE( pCurRule, "SwListShell::Execute without NumRule" );
    bool bOutline = pCurRule && pCurRule->IsOutlineRule();

    switch (nSlot)
    {
        case FN_NUM_BULLET_DOWN:
        case FN_NUM_BULLET_UP:
            {
                SfxViewFrame& rFrame = GetView().GetViewFrame();

                rReq.Done();
                rSh.NumUpDown( nSlot == FN_NUM_BULLET_DOWN );
                rFrame.GetBindings().Invalidate( SID_TABLE_CELL ); // Update status line!
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
        default:
            OSL_ENSURE(false, "wrong dispatcher");
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
                if ( rSh.CursorInsideInputField() )
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
    SetName("List");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
