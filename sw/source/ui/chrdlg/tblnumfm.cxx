/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tblnumfm.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 10:20:46 $
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

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svtools/itemset.hxx>
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif

SwNumFmtDlg::SwNumFmtDlg(Window* pParent, SfxItemSet& rSet)
    : SfxSingleTabDialog( pParent, rSet, 0 )
{
    // TabPage erzeugen
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
    ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT );
    if ( fnCreatePage )
    {
        SfxTabPage* pNewPage = (*fnCreatePage)( this, rSet );
        SfxAllItemSet aSet(*(rSet.GetPool()));
        aSet.Put ( SvxNumberInfoItem( (const SvxNumberInfoItem&)pNewPage->GetItemSet().Get( SID_ATTR_NUMBERFORMAT_INFO )));
        pNewPage->PageCreated(aSet);
        SetTabPage(pNewPage);
    }
}


SwNumFmtDlg::~SwNumFmtDlg()
{
}


