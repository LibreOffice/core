/*************************************************************************
 *
 *  $RCSfile: marktree.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-05 10:00:43 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_MARKTREE_HXX_
#include "marktree.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

//========================================================================
//= OMarkableTreeListBox
//========================================================================
//------------------------------------------------------------------------
OMarkableTreeListBox::OMarkableTreeListBox( Window* pParent, WinBits nWinStyle ) : SvTreeListBox(pParent,nWinStyle)
{
    SetNodeBitmaps( Bitmap(ModuleRes(BMP_PLUSBUTTON)),Bitmap(ModuleRes(BMP_MINUSBUTTON)));
    InitButtonData();
}
//------------------------------------------------------------------------
OMarkableTreeListBox::OMarkableTreeListBox( Window* pParent, const ResId& rResId ) : SvTreeListBox(pParent,rResId)
{
    SetNodeBitmaps( Bitmap(ModuleRes(BMP_PLUSBUTTON)),Bitmap(ModuleRes(BMP_MINUSBUTTON)));
    InitButtonData();
}
//------------------------------------------------------------------------
OMarkableTreeListBox::~OMarkableTreeListBox()
{
    delete m_pCheckButton;
}
//------------------------------------------------------------------------
void OMarkableTreeListBox::InitButtonData()
{
    m_pCheckButton = new SvLBoxButtonData();
    m_pCheckButton->aBmps[SV_BMP_UNCHECKED] = Bitmap( ResId( BMP_CHECKBUTTON_UNCHECKED ) );
    m_pCheckButton->aBmps[SV_BMP_CHECKED]       = Bitmap( ResId( BMP_CHECKBUTTON_CHECKED ) );
    m_pCheckButton->aBmps[SV_BMP_HICHECKED] = Bitmap( ResId( BMP_CHECKBUTTON_HICHECKED ) );
    m_pCheckButton->aBmps[SV_BMP_HIUNCHECKED]   = Bitmap( ResId( BMP_CHECKBUTTON_UNCHECKED ) );
    m_pCheckButton->aBmps[SV_BMP_TRISTATE]  = Bitmap( ResId( BMP_CHECKBUTTON_TRISTATE ) );
    m_pCheckButton->aBmps[SV_BMP_HITRISTATE]    = Bitmap( ResId( BMP_CHECKBUTTON_HITRISTATE ) );
    EnableCheckButton( m_pCheckButton );
}
//------------------------------------------------------------------------
void OMarkableTreeListBox::KeyInput( const KeyEvent& rKEvt )
{
    // nur wenn space
    if (rKEvt.GetKeyCode().GetCode() == KEY_SPACE && !rKEvt.GetKeyCode().IsShift() && !rKEvt.GetKeyCode().IsMod1())
    {
        SvLBoxEntry* pHdlEntry = GetHdlEntry();
        if(pHdlEntry)
        {
            SvButtonState eState = GetCheckButtonState( pHdlEntry);
            if(eState == SV_BUTTON_CHECKED)
                SetCheckButtonState( pHdlEntry,SV_BUTTON_UNCHECKED);
            else
                SetCheckButtonState( pHdlEntry,SV_BUTTON_CHECKED);

            CheckButtonHdl();
        }
        else
            SvTreeListBox::KeyInput(rKEvt);
    }
    else
        SvTreeListBox::KeyInput(rKEvt);

}
//------------------------------------------------------------------------
void OMarkableTreeListBox::CheckButtons()
{
    // Plausibilit"atspr"ufung
    SvButtonState eState;
    SvLBoxEntry* pEntry = GetModel()->First();
    while(pEntry)
    {
        if(!GetModel()->HasChilds(pEntry))
        {
            sal_uInt16 nCheck=0;
            sal_uInt16 nCount=0;
            SvLBoxEntry* pChildEntry = pEntry;
            while(pChildEntry)
            {
                if(GetCheckButtonState(pChildEntry) == SV_BUTTON_CHECKED)
                    nCheck++;
                nCount++;
                pChildEntry = GetModel()->NextSibling(pChildEntry);
            }

            if(nCheck && nCount != nCheck)
                eState = SV_BUTTON_TRISTATE;
            else if(nCheck)
                eState = SV_BUTTON_CHECKED;
            else
                eState = SV_BUTTON_UNCHECKED;

            SvLBoxEntry* pSchema = GetModel()->GetParent(pEntry);
            if(pSchema)
            {
                pEntry = GetModel()->NextSibling(pSchema);
                SetCheckButtonState( pSchema,eState);
            }
            else
                pEntry = NULL; // wenn kein Schema dann sind bereits alle pEntry's durchlaufen worden
        }
        else
            pEntry = GetModel()->Next(pEntry);
    }

    SvLBoxEntry* pCatalog = GetModel()->First();
    SvLBoxEntry* pSchema = NULL;
    if(GetModel()->HasChilds(pCatalog) && GetModel()->HasChilds(pSchema = GetModel()->Next(pCatalog)))
    {
        sal_uInt16 nCheck   =0;
        sal_uInt16 nTri     =0;
        sal_uInt16 nCount   =0;
        while(pSchema)
        {
            if((eState = GetCheckButtonState(pSchema)) == SV_BUTTON_TRISTATE)
                break;

            if(eState == SV_BUTTON_CHECKED)
                nCheck++;
            nCount++;
            pSchema = GetModel()->NextSibling(pSchema);
        }

        if(eState != SV_BUTTON_TRISTATE)
        {
            if(nCheck && nCount == nCheck)
                eState = SV_BUTTON_CHECKED;
            else if(nCheck)
                eState = SV_BUTTON_TRISTATE;
            else
                eState = SV_BUTTON_UNCHECKED;
        }
        SetCheckButtonState( pCatalog,eState);
    }
}
//------------------------------------------------------------------------
void OMarkableTreeListBox::CheckButtonHdl()
{
    SvLBoxEntry* pHdlEntry = GetHdlEntry();
    SvButtonState eState = GetCheckButtonState( pHdlEntry);
    if(GetModel()->HasChilds(pHdlEntry))    // Falls Kinder, dann diese auch checken
    {
        SvLBoxEntry* pChildEntry = GetModel()->Next(pHdlEntry);
        SvLBoxEntry* pSiblingEntry = GetModel()->NextSibling(pHdlEntry);
        while(pChildEntry && pChildEntry != pSiblingEntry)
        {
            SetCheckButtonState( pChildEntry,eState);
            pChildEntry = GetModel()->Next(pChildEntry);
        }
    }

    SvLBoxEntry* pEntry = IsSelected(pHdlEntry) ? FirstSelected() : NULL;
    while(pEntry)
    {
        SetCheckButtonState( pEntry,eState);
        if(GetModel()->HasChilds(pEntry))   // Falls Kinder, dann diese auch checken
        {
            SvLBoxEntry* pChildEntry = GetModel()->Next(pEntry);
            SvLBoxEntry* pSiblingEntry = GetModel()->NextSibling(pEntry);
            while(pChildEntry && pChildEntry != pSiblingEntry)
            {
                SetCheckButtonState( pChildEntry,eState);
                pChildEntry = GetModel()->Next(pChildEntry);
            }
        }
        pEntry = NextSelected(pEntry);
    }
    CheckButtons();
}
//------------------------------------------------------------------------
SvLBoxEntry* OMarkableTreeListBox::GetEntryPosByName(const String& aName,SvLBoxEntry* pStart) const
{
    SvLBoxEntry* pEntry = pStart ? GetModel()->FirstChild(pStart) : GetModel()->First();
    while(pEntry && !GetEntryText(pEntry).Equals(aName))
        pEntry = GetModel()->Next(pEntry);

    return pEntry;
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 28.09.00 13:22:30  fs
 ************************************************************************/

