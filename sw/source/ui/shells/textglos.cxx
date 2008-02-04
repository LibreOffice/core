/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textglos.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 15:35:22 $
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


#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif

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
    USHORT nSlot = rReq.GetSlot();
    ::GetGlossaries()->UpdateGlosPath(!rReq.IsAPI() ||
                                        FN_GLOSSARY_DLG == nSlot );
    SwGlossaryHdl* pGlosHdl = GetView().GetGlosHdl();
    // SwGlossaryList updaten?
    BOOL bUpdateList = FALSE;

    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;
    if(pArgs)
       pArgs->GetItemState(nSlot, FALSE, &pItem );

    switch( nSlot )
    {
        case FN_GLOSSARY_DLG:
            pGlosHdl->GlossaryDlg();
            bUpdateList = TRUE;
            rReq.Ignore();
            break;
        case FN_EXPAND_GLOSSARY:
        {
            BOOL bReturn;
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
                if(SFX_ITEM_SET ==  pArgs->GetItemState(FN_PARAM_1, FALSE, &pItem ))
                    aName = (( const SfxStringItem *)pItem)->GetValue();
                String aShortName;
                if(SFX_ITEM_SET ==  pArgs->GetItemState(FN_PARAM_2, FALSE, &pItem ))
                    aShortName = (( const SfxStringItem *)pItem)->GetValue();

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "Dialogdiet fail!");
                ::GlossarySetActGroup fnSetActGroup = pFact->SetGlossaryActGroupFunc( DLG_RENAME_GLOS );
                if ( fnSetActGroup )
                    (*fnSetActGroup)( aGroup );
                pGlosHdl->SetCurGroup(aGroup, TRUE);
                //eingestellte Gruppe muss in NewGlossary ggf. erzeugt werden!
                pGlosHdl->NewGlossary( aName, aShortName, TRUE );
                rReq.Done();
            }
            bUpdateList = TRUE;
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
                if(SFX_ITEM_SET ==  pArgs->GetItemState(FN_PARAM_1, FALSE, &pItem ))
                    aName = (( const SfxStringItem *)pItem)->GetValue();
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "Dialogdiet fail!");
                ::GlossarySetActGroup fnSetActGroup = pFact->SetGlossaryActGroupFunc( DLG_RENAME_GLOS );
                if ( fnSetActGroup )
                    (*fnSetActGroup)( aGroup );
                pGlosHdl->SetCurGroup(aGroup, TRUE);
                rReq.SetReturnValue(SfxBoolItem(nSlot, pGlosHdl->InsertGlossary( aName )));
                rReq.Done();
            }
        }
        break;
        default:
            ASSERT(FALSE, falscher Dispatcher);
            return;
    }
    if(bUpdateList)
    {
        SwGlossaryList* pList = ::GetGlossaryList();
        if(pList->IsActive())
            pList->Update();
    }
}


