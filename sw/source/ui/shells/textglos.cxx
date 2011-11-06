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


#include <sfx2/request.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>

#include "errhdl.hxx"
#include "view.hxx"
#include "initui.hxx"
#include "cmdid.h"
#include "textsh.hxx"
#include "initui.hxx"
#include "gloshdl.hxx"
#include "glosdoc.hxx"
#include "gloslst.hxx"
#include "swabstdlg.hxx"
#include <misc.hrc>

// STATIC DATA -----------------------------------------------------------

void SwTextShell::ExecGlossary(SfxRequest &rReq)
{
    sal_uInt16 nSlot = rReq.GetSlot();
    ::GetGlossaries()->UpdateGlosPath(!rReq.IsAPI() ||
                                        FN_GLOSSARY_DLG == nSlot );
    SwGlossaryHdl* pGlosHdl = GetView().GetGlosHdl();
    // SwGlossaryList updaten?
    sal_Bool bUpdateList = sal_False;

    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;
    if(pArgs)
       pArgs->GetItemState(nSlot, sal_False, &pItem );

    switch( nSlot )
    {
        case FN_GLOSSARY_DLG:
            pGlosHdl->GlossaryDlg();
            bUpdateList = sal_True;
            rReq.Ignore();
            break;
        case FN_EXPAND_GLOSSARY:
        {
            sal_Bool bReturn;
            bReturn = pGlosHdl->ExpandGlossary();
            rReq.SetReturnValue( SfxBoolItem( nSlot, bReturn ) );
            rReq.Done();
        }
        break;
        case FN_NEW_GLOSSARY:
            if(pItem && pArgs->Count() == 3 )
            {
                String aGroup = (( const SfxStringItem *)pItem)->GetValue();
                String aName;
                if(SFX_ITEM_SET ==  pArgs->GetItemState(FN_PARAM_1, sal_False, &pItem ))
                    aName = (( const SfxStringItem *)pItem)->GetValue();
                String aShortName;
                if(SFX_ITEM_SET ==  pArgs->GetItemState(FN_PARAM_2, sal_False, &pItem ))
                    aShortName = (( const SfxStringItem *)pItem)->GetValue();

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "Dialogdiet fail!");
                ::GlossarySetActGroup fnSetActGroup = pFact->SetGlossaryActGroupFunc( DLG_RENAME_GLOS );
                if ( fnSetActGroup )
                    (*fnSetActGroup)( aGroup );
                pGlosHdl->SetCurGroup(aGroup, sal_True);
                //eingestellte Gruppe muss in NewGlossary ggf. erzeugt werden!
                pGlosHdl->NewGlossary( aName, aShortName, sal_True );
                rReq.Done();
            }
            bUpdateList = sal_True;
        break;
        case FN_SET_ACT_GLOSSARY:
            if(pItem)
            {
                String aGroup = (( const SfxStringItem *)pItem)->GetValue();
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "Dialogdiet fail!");
                ::GlossarySetActGroup fnSetActGroup = pFact->SetGlossaryActGroupFunc( DLG_RENAME_GLOS );
                if ( fnSetActGroup )
                    (*fnSetActGroup)( aGroup );
                rReq.Done();
            }
        break;
        case FN_INSERT_GLOSSARY:
        {
            if(pItem && pArgs->Count() > 1)
            {
                String aGroup = (( const SfxStringItem *)pItem)->GetValue();
                String aName;
                if(SFX_ITEM_SET ==  pArgs->GetItemState(FN_PARAM_1, sal_False, &pItem ))
                    aName = (( const SfxStringItem *)pItem)->GetValue();
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "Dialogdiet fail!");
                ::GlossarySetActGroup fnSetActGroup = pFact->SetGlossaryActGroupFunc( DLG_RENAME_GLOS );
                if ( fnSetActGroup )
                    (*fnSetActGroup)( aGroup );
                pGlosHdl->SetCurGroup(aGroup, sal_True);
                rReq.SetReturnValue(SfxBoolItem(nSlot, pGlosHdl->InsertGlossary( aName )));
                rReq.Done();
            }
        }
        break;
        default:
            ASSERT(sal_False, falscher Dispatcher);
            return;
    }
    if(bUpdateList)
    {
        SwGlossaryList* pList = ::GetGlossaryList();
        if(pList->IsActive())
            pList->Update();
    }
}


