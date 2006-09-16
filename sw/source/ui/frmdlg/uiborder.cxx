/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: uiborder.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 23:02:56 $
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

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif



//CHINA001 #ifndef _SVX_BORDER_HXX //autogen
//CHINA001 #include <svx/border.hxx>
//CHINA001 #endif
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/svxids.hrc> //CHINA001
#include <svx/dialogs.hrc> //CHINA001
#include <svtools/itemset.hxx> //CHINA001
#include <svx/flagsdef.hxx> //CHINA001
#include <sfx2/tabdlg.hxx> //CHINA001
#ifndef _SFXINTITEM_HXX //CHINA001
#include <svtools/intitem.hxx> //CHINA001
#endif //CHINA001

#include "swtypes.hxx"
#include "uiborder.hxx"
#include "frmui.hrc"




SwBorderDlg::SwBorderDlg(Window* pParent, SfxItemSet& rSet, USHORT nType) :

    SfxSingleTabDialog(pParent, rSet, 0)

{
    SetText(SW_RESSTR(STR_FRMUI_BORDER));

    // TabPage erzeugen
    //CHINA001 SvxBorderTabPage* pPage = (SvxBorderTabPage*) SvxBorderTabPage::Create(this, rSet);
    //CHINA001 pPage->SetSWMode(nType);
    //CHINA001 if(SW_BORDER_MODE_TABLE == nType)
        //CHINA001 pPage->HideShadowControls();
    //CHINA001 SetTabPage(pPage);
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
    ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER );
    if ( fnCreatePage )
    {
        SfxTabPage* pPage = (*fnCreatePage)( this, rSet );
        SfxAllItemSet aSet(*(rSet.GetPool()));
        aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,nType));
        if(SW_BORDER_MODE_TABLE == nType)
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_HIDESHADOWCTL));
        pPage->PageCreated(aSet);
        SetTabPage(pPage);
    }
}



SwBorderDlg::~SwBorderDlg()
{
}


