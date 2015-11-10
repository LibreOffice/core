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

#include <sfx2/request.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>

#include "view.hxx"
#include "cmdid.h"
#include "textsh.hxx"
#include "initui.hxx"
#include "gloshdl.hxx"
#include "glosdoc.hxx"
#include "gloslst.hxx"
#include "swabstdlg.hxx"
#include <misc.hrc>

void SwTextShell::ExecGlossary(SfxRequest &rReq)
{
    const sal_uInt16 nSlot = rReq.GetSlot();
    ::GetGlossaries()->UpdateGlosPath(!rReq.IsAPI() ||
                                        FN_GLOSSARY_DLG == nSlot );
    SwGlossaryHdl* pGlosHdl = GetView().GetGlosHdl();
    // Update SwGlossaryList?
    bool bUpdateList = false;

    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = nullptr;
    if(pArgs)
       pArgs->GetItemState(nSlot, false, &pItem );

    switch( nSlot )
    {
        case FN_GLOSSARY_DLG:
            pGlosHdl->GlossaryDlg();
            bUpdateList = true;
            rReq.Ignore();
            break;
        case FN_EXPAND_GLOSSARY:
        {
            bool bReturn;
            bReturn = pGlosHdl->ExpandGlossary();
            rReq.SetReturnValue( SfxBoolItem( nSlot, bReturn ) );
            rReq.Done();
        }
        break;
        case FN_NEW_GLOSSARY:
            if(pItem && pArgs->Count() == 3 )
            {
                OUString aGroup = static_cast<const SfxStringItem *>(pItem)->GetValue();
                OUString aName;
                if(SfxItemState::SET ==  pArgs->GetItemState(FN_PARAM_1, false, &pItem ))
                    aName = static_cast<const SfxStringItem *>(pItem)->GetValue();
                OUString aShortName;
                if(SfxItemState::SET ==  pArgs->GetItemState(FN_PARAM_2, false, &pItem ))
                    aShortName = static_cast<const SfxStringItem *>(pItem)->GetValue();

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialog creation failed!");
                ::GlossarySetActGroup fnSetActGroup = pFact->SetGlossaryActGroupFunc();
                if ( fnSetActGroup )
                    (*fnSetActGroup)( aGroup );
                pGlosHdl->SetCurGroup(aGroup, true);
                // Chosen group must be created in NewGlossary if necessary!
                pGlosHdl->NewGlossary( aName, aShortName, true );
                rReq.Done();
            }
            bUpdateList = true;
        break;
        case FN_SET_ACT_GLOSSARY:
            if(pItem)
            {
                OUString aGroup = static_cast<const SfxStringItem *>(pItem)->GetValue();
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialog creation failed!");
                ::GlossarySetActGroup fnSetActGroup = pFact->SetGlossaryActGroupFunc();
                if ( fnSetActGroup )
                    (*fnSetActGroup)( aGroup );
                rReq.Done();
            }
        break;
        case FN_INSERT_GLOSSARY:
        {
            if(pItem && pArgs->Count() > 1)
            {
                OUString aGroup = static_cast<const SfxStringItem *>(pItem)->GetValue();
                OUString aName;
                if(SfxItemState::SET ==  pArgs->GetItemState(FN_PARAM_1, false, &pItem ))
                    aName = static_cast<const SfxStringItem *>(pItem)->GetValue();
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialog creation failed!");
                ::GlossarySetActGroup fnSetActGroup = pFact->SetGlossaryActGroupFunc();
                if ( fnSetActGroup )
                    (*fnSetActGroup)( aGroup );
                pGlosHdl->SetCurGroup(aGroup, true);
                rReq.SetReturnValue(SfxBoolItem(nSlot, pGlosHdl->InsertGlossary( aName )));
                rReq.Done();
            }
        }
        break;
        default:
            OSL_FAIL("wrong dispatcher");
            return;
    }
    if(bUpdateList)
    {
        SwGlossaryList* pList = ::GetGlossaryList();
        if(pList->IsActive())
            pList->Update();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
