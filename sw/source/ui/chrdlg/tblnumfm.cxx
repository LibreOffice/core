/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tblnumfm.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif




#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <svx/numinf.hxx>
#include <swtypes.hxx>
#include <tblnumfm.hxx>

#ifndef _CHRDLG_HRC
#include <chrdlg.hrc>
#endif

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svtools/itemset.hxx>
#include <sfx2/tabdlg.hxx>

SwNumFmtDlg::SwNumFmtDlg(Window* pParent, const SfxItemSet& rSet)
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


