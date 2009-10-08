/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: funcpage.cxx,v $
 * $Revision: 1.7.32.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//----------------------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/stritem.hxx>

#include "funcpage.hxx"
#include "formdlgs.hrc"
#include "formdata.hxx"
#include "globstr.hrc"
#include "scresid.hxx"
#include "reffact.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "tabvwsh.hxx"
#include "appoptio.hxx"
#include "funcdesc.hxx"

//============================================================================

ScListBox::ScListBox( Window* pParent, const ResId& rResId ):
    ListBox(pParent,rResId)
{}

void ScListBox::KeyInput( const KeyEvent& rKEvt )
{
    KeyEvent aKEvt=rKEvt;
    //ListBox::KeyInput(rKEvt);

    if(aKEvt.GetCharCode()==' ')
        DoubleClick();
}

long ScListBox::PreNotify( NotifyEvent& rNEvt )
{
    NotifyEvent aNotifyEvt=rNEvt;

    long nResult=ListBox::PreNotify(rNEvt);

    USHORT nSwitch=aNotifyEvt.GetType();
    if(nSwitch==EVENT_KEYINPUT)
    {
        KeyInput(*aNotifyEvt.GetKeyEvent());
    }
    return nResult;
}



//============================================================================

inline USHORT Lb2Cat( USHORT nLbPos )
{
    // Kategorie 0 == LRU, sonst Categories == LbPos-1
    if ( nLbPos > 0 )
        nLbPos -= 1;

    return nLbPos;
}

//============================================================================

ScFuncPage::ScFuncPage(Window* pParent):
    TabPage(pParent,ScResId(RID_SCTAB_FUNCTION)),
    //
    aFtCategory     ( this, ScResId( FT_CATEGORY ) ),
    aLbCategory     ( this, ScResId( LB_CATEGORY ) ),
    aFtFunction     ( this, ScResId( FT_FUNCTION ) ),
    aLbFunction     ( this, ScResId( LB_FUNCTION ) ),
    aIBFunction     ( this, ScResId( IB_FUNCTION ) )
{
    FreeResource();
    InitLRUList();
    aLbCategory.SelectEntryPos(1);
    UpdateFunctionList();
    aLbCategory.SetSelectHdl( LINK( this, ScFuncPage, SelHdl ) );
    aLbFunction.SetSelectHdl( LINK( this, ScFuncPage, SelHdl ) );
    aLbFunction.SetDoubleClickHdl( LINK( this, ScFuncPage, DblClkHdl ) );
    aIBFunction.SetClickHdl( LINK( this, ScFuncPage, DblClkHdl ) );
}

void ScFuncPage::UpdateFunctionList()
{
    USHORT  nSelPos   = aLbCategory.GetSelectEntryPos();
    USHORT  nCategory = ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                            ? Lb2Cat( nSelPos ) : 0;

    aLbFunction.Clear();
    aLbFunction.SetUpdateMode( FALSE );
    //------------------------------------------------------

    if ( nSelPos > 0 )
    {
        ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();

        const ScFuncDesc*   pDesc = pFuncMgr->First( nCategory );
        while ( pDesc )
        {
            aLbFunction.SetEntryData(
                aLbFunction.InsertEntry(*(pDesc->pFuncName) ),(void*)pDesc );
            pDesc = pFuncMgr->Next();
        }
    }
    else // LRU-Liste
    {
        for ( USHORT i=0; i<LRU_MAX && aLRUList[i]; i++ )
        {
            const ScFuncDesc*   pDesc = aLRUList[i];
            aLbFunction.SetEntryData(
                aLbFunction.InsertEntry( *(pDesc->pFuncName) ),(void*)pDesc );
        }
    }

    //------------------------------------------------------
    aLbFunction.SetUpdateMode( TRUE );
    aLbFunction.SelectEntryPos(0);

    if(IsVisible()) SelHdl(&aLbFunction);
}

IMPL_LINK( ScFuncPage, SelHdl, ListBox*, pLb )
{
    if(pLb==&aLbFunction)
    {
        aSelectionLink.Call(this);
    }
    else
    {
        UpdateFunctionList();
    }
    return 0;
}

IMPL_LINK( ScFuncPage, DblClkHdl, ListBox*, EMPTYARG )
{
    aDoubleClickLink.Call(this);
    return 0;
}

void ScFuncPage::SetCategory(USHORT nCat)
{
    aLbCategory.SelectEntryPos(nCat);
    UpdateFunctionList();
}

void ScFuncPage::SetFunction(USHORT nFunc)
{
    aLbFunction.SelectEntryPos(nFunc);
}

void ScFuncPage::SetFocus()
{
    aLbFunction.GrabFocus();
}

USHORT ScFuncPage::GetCategory()
{
    return aLbCategory.GetSelectEntryPos();
}

USHORT ScFuncPage::GetFunction()
{
    return aLbFunction.GetSelectEntryPos();
}

USHORT ScFuncPage::GetFunctionEntryCount()
{
    return aLbFunction.GetSelectEntryCount();
}

const ScFuncDesc*   ScFuncPage::GetFuncDesc( USHORT nPos ) const
{
    // nicht schoen, aber hoffentlich selten
    return (const ScFuncDesc*) aLbFunction.GetEntryData(nPos);
}

USHORT ScFuncPage::GetFuncPos(const ScFuncDesc* pFuncDesc)
{
    if(pFuncDesc!=NULL && pFuncDesc->pFuncName!=NULL)
    {
        return aLbFunction.GetEntryPos(*(pFuncDesc->pFuncName));
    }
    else
    {
        return LISTBOX_ENTRY_NOTFOUND;
    }
}
String ScFuncPage::GetSelFunctionName() const
{
    return aLbFunction.GetSelectEntry();
}

void ScFuncPage::InitLRUList()
{
    const ScAppOptions& rAppOpt = SC_MOD()->GetAppOptions();
    USHORT nLRUFuncCount = Min( rAppOpt.GetLRUFuncListCount(), (USHORT)LRU_MAX );
    USHORT* pLRUListIds = rAppOpt.GetLRUFuncList();

    USHORT i;
    for ( i=0; i<LRU_MAX; i++ )
        aLRUList[i] = NULL;

    if ( pLRUListIds )
    {
        ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();
        for ( i=0; i<nLRUFuncCount; i++ )
            aLRUList[i] = pFuncMgr->Get( pLRUListIds[i] );
    }
}




