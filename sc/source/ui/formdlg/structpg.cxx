/*************************************************************************
 *
 *  $RCSfile: structpg.cxx,v $
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

#include "structpg.hxx"
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

//----------------------------------------------------------------------------

ScStructListBox::ScStructListBox(Window* pParent, const ResId& rResId ):
    SvTreeListBox(pParent,rResId )
{
    bActiveFlag=FALSE;
}

void ScStructListBox::SetActiveFlag(BOOL bFlag)
{
    bActiveFlag=bFlag;
}

BOOL ScStructListBox::GetActiveFlag()
{
    return bActiveFlag;
}

void ScStructListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    bActiveFlag=TRUE;
    SvTreeListBox::MouseButtonDown(rMEvt);
}

void ScStructListBox::GetFocus()
{
    bActiveFlag=TRUE;
    SvTreeListBox::GetFocus();
}

void ScStructListBox::LoseFocus()
{
    bActiveFlag=FALSE;
    SvTreeListBox::LoseFocus();
}

//==============================================================================

ScStructPage::ScStructPage(Window* pParent):
    TabPage(pParent,ScResId(RID_SCTAB_STRUCT)),
    //
    aFtStruct       ( this, ScResId( FT_STRUCT ) ),
    aTlbStruct      ( this, ScResId( TLB_STRUCT ) ),
    aExpBmp         ( ScResId( RID_BMP_EXPAND ) ),
    aCollBmp        ( ScResId( RID_BMP_COLLAPSE ) ),
    aCloseBmp       ( ScResId( BMP_STR_CLOSE ) ),
    aOpenBmp        ( ScResId( BMP_STR_OPEN  ) ),
    aEndBmp         ( ScResId( BMP_STR_END   ) ),
    aErrorBmp       ( ScResId( BMP_STR_ERROR ) ),
    pSelectedToken  ( NULL )
{
    FreeResource();
    aTlbStruct.SetWindowBits(WB_HASLINES|WB_CLIPCHILDREN|
                        WB_HASBUTTONS|WB_HSCROLL|WB_NOINITIALSELECTION);

    aTlbStruct.SetNodeBitmaps( aExpBmp, aCollBmp );
    aTlbStruct.SetDefaultExpandedEntryBmp( aOpenBmp);
    aTlbStruct.SetDefaultCollapsedEntryBmp( aCloseBmp);
    Font aFont=GetFont();
    Size aSize=aFont.GetSize();
    aSize.Height()-=2;
    aFont.SetSize(aSize);
    aTlbStruct.SetFont(aFont);
    aTlbStruct.SetSelectHdl(LINK( this, ScStructPage, SelectHdl ) );

}

void ScStructPage::ClearStruct()
{
    aTlbStruct.SetActiveFlag(FALSE);
    aTlbStruct.Clear();
}

SvLBoxEntry* ScStructPage::InsertEntryWithError(USHORT nError,SvLBoxEntry* pParent,ULONG nPos)
{

    SvLBoxEntry* pEntry;
    switch(nError)
    {
        case STRUCT_ERR_C1:
                    pEntry=InsertEntry( STR_STRUCT_ERR1,pParent,STRUCT_ERROR,nPos);
                    break;
        case STRUCT_ERR_C2:
                    pEntry=InsertEntry( STR_STRUCT_ERR2,pParent,STRUCT_ERROR,nPos);
                    break;
    }
    return pEntry;
}


SvLBoxEntry* ScStructPage::InsertEntry( const XubString& rText, SvLBoxEntry* pParent,
                                       USHORT nFlag,ULONG nPos,ScToken* pScToken)
{
    aTlbStruct.SetActiveFlag(FALSE);

    SvLBoxEntry *pEntry=NULL;
    switch(nFlag)
    {
        case STRUCT_FOLDER:
                    pEntry=aTlbStruct.InsertEntry(rText,pParent,FALSE,nPos,pScToken);
                    if(pParent!=NULL) aTlbStruct.Expand(pParent);
                    break;
        case STRUCT_END:
                    pEntry=aTlbStruct.InsertEntry(rText,aEndBmp,aEndBmp,
                                        pParent,FALSE,nPos,pScToken);
                    if(pParent!=NULL) aTlbStruct.Expand(pParent);
                    break;
        case STRUCT_ERROR:
                    pEntry=aTlbStruct.InsertEntry(rText,aErrorBmp,aErrorBmp,
                                        pParent,FALSE,nPos,pScToken);
                    if(pParent!=NULL) aTlbStruct.Expand(pParent);
                    break;
    }
    return pEntry;
}

String ScStructPage::GetEntryText(SvLBoxEntry* pEntry)
{
    String aString;
    if(pEntry!=NULL)
        aString=aTlbStruct.GetEntryText(pEntry);
    return  aString;
}

ScToken* ScStructPage::GetFunctionEntry(SvLBoxEntry* pEntry)
{
    if(pEntry!=NULL)
    {
        ScToken * pToken=(ScToken *)pEntry->GetUserData();
        if(pToken!=NULL)
        {
            OpCode eOp = pToken->GetOpCode();
            if(!(pToken->IsFunction()|| ocArcTan2<=eOp))
            {
                return GetFunctionEntry(aTlbStruct.GetParent(pEntry));
            }
            else
            {
                return pToken;
            }
        }
    }
    return NULL;
}

IMPL_LINK( ScStructPage, SelectHdl, SvTreeListBox*, pTlb )
{
    if(aTlbStruct.GetActiveFlag())
    {
        if(pTlb==&aTlbStruct)
        {
            SvLBoxEntry*    pCurEntry=aTlbStruct.GetCurEntry();
            if(pCurEntry!=NULL)
            {
                pSelectedToken=(ScToken *)pCurEntry->GetUserData();
                if(pSelectedToken!=NULL)
                {
                    OpCode eOp = pSelectedToken->GetOpCode();
                    if(!(pSelectedToken->IsFunction()|| ocArcTan2<=eOp))
                    {
                        pSelectedToken=GetFunctionEntry(pCurEntry);
                    }
                }
            }
        }

        aSelLink.Call(this);
    }
    return 0;
}

ScToken * ScStructPage::GetSelectedToken()
{
    return pSelectedToken;
}

String ScStructPage::GetSelectedEntryText()
{
    return aTlbStruct.GetEntryText(aTlbStruct.GetCurEntry());
}



