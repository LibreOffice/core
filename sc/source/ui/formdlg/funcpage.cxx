/*************************************************************************
 *
 *  $RCSfile: funcpage.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:57 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//----------------------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/zforlist.hxx>

#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif

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

//============================================================================

ScListBox::ScListBox( Window* pParent, WinBits nWinStyle):
    ListBox(pParent,nWinStyle)
{}

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

        ScFuncDesc* pDesc = pFuncMgr->First( nCategory );
        while ( pDesc )
        {
            aLbFunction.SetEntryData(
                aLbFunction.InsertEntry(*(pDesc->pFuncName) ),pDesc );
            pDesc = pFuncMgr->Next();
        }
    }
    else // LRU-Liste
    {
        for ( USHORT i=0; i<LRU_MAX && aLRUList[i]; i++ )
        {
            ScFuncDesc* pDesc = aLRUList[i];
            aLbFunction.SetEntryData(
                aLbFunction.InsertEntry( *(pDesc->pFuncName) ),pDesc );
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

IMPL_LINK( ScFuncPage, DblClkHdl, ListBox*, pLb )
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

ScFuncDesc* ScFuncPage::GetFuncDesc( USHORT nPos ) const
{
    // nicht schoen, aber hoffentlich selten
    return (ScFuncDesc*) aLbFunction.GetEntryData(nPos);
}

USHORT ScFuncPage::GetFuncPos(ScFuncDesc* pFuncDesc)
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




