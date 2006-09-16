/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tblnumfm.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 22:38:55 $
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




#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
//CHINA001 #ifndef _SVX_NUMFMT_HXX
//CHINA001 #include <svx/numfmt.hxx>
//CHINA001 #endif
#define ITEMID_NUMBERINFO SID_ATTR_NUMBERFORMAT_INFO
#ifndef _SVX_NUMINF_HXX
#include <svx/numinf.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _TBLNUMFM_HXX
#include <tblnumfm.hxx>
#endif

#ifndef _CHRDLG_HRC
#include <chrdlg.hrc>
#endif

#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001
#include <svtools/itemset.hxx> //CHINA001
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif

SwNumFmtDlg::SwNumFmtDlg(Window* pParent, SfxItemSet& rSet)
    : SfxSingleTabDialog( pParent, rSet, 0 )
{
//    SetText(SW_RESSTR(STR_NUMFMT));

    // TabPage erzeugen
    //CHINA001 SvxNumberFormatTabPage* pPage = (SvxNumberFormatTabPage*)
                                //CHINA001 SvxNumberFormatTabPage::Create( this, rSet );
    //CHINA001 pPage->SetNumberFormatList( (const SvxNumberInfoItem&)pPage->
                            //CHINA001 GetItemSet().Get( SID_ATTR_NUMBERFORMAT_INFO ) );
    //CHINA001 SetTabPage( pPage );
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
    ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT );
    if ( fnCreatePage )
    {
        SfxTabPage* pPage = (*fnCreatePage)( this, rSet );
        SfxAllItemSet aSet(*(rSet.GetPool())); //CHINA001
        aSet.Put ( SvxNumberInfoItem( (const SvxNumberInfoItem&)pPage->GetItemSet().Get( SID_ATTR_NUMBERFORMAT_INFO )));
        pPage->PageCreated(aSet);
        SetTabPage(pPage);
    }
}


SwNumFmtDlg::~SwNumFmtDlg()
{
}


