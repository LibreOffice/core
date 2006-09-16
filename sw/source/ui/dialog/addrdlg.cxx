/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: addrdlg.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 22:48:32 $
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

#include "addrdlg.hxx"

//CHINA001 #ifndef _SVX_OPTGENRL_HXX //autogen
//CHINA001 #include <svx/optgenrl.hxx>
//CHINA001 #endif
#ifndef _SVX_DIALOG_HXX
#include <svx/svxdlg.hxx>
#endif
#ifndef _SFX_HRC
#include <sfx2/sfx.hrc>
#endif

/****************************************************************************
Ctor
****************************************************************************/


SwAddrDlg::SwAddrDlg(Window* pParent, SfxItemSet& rSet ) :

    SfxSingleTabDialog(pParent, rSet, 0)

{
    // TabPage erzeugen
    //CHINA001 SvxGeneralTabPage* pPage = (SvxGeneralTabPage*) SvxGeneralTabPage::Create(this, rSet);
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
    ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( RID_SFXPAGE_GENERAL );
    if ( fnCreatePage )
    {
        SfxTabPage* pPage = (*fnCreatePage)( this, rSet );
        SetTabPage(pPage);
    }
}

/****************************************************************************
Dtor
****************************************************************************/


__EXPORT SwAddrDlg::~SwAddrDlg()
{
}


