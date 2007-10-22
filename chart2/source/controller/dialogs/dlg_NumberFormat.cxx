/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_NumberFormat.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 16:45:15 $
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
#include "precompiled_chart2.hxx"

#include "dlg_NumberFormat.hxx"

// header for class SfxItemSet
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

// header for class SvxNumberInfoItem
#define ITEMID_NUMBERINFO SID_ATTR_NUMBERFORMAT_INFO
#ifndef _SVX_NUMINF_HXX
#include <svx/numinf.hxx>
#endif

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif

// header for SvNumberFormatter
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;

NumberFormatDialog::NumberFormatDialog(Window* pParent, SfxItemSet& rSet)
    : SfxSingleTabDialog( pParent, rSet, 0 )
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT );
    if ( fnCreatePage )
    {
        SfxTabPage* pTabPage = (*fnCreatePage)( this, rSet );
        pTabPage->PageCreated(rSet);
        SetTabPage(pTabPage);
    }
}

NumberFormatDialog::~NumberFormatDialog()
{
}

//static
SfxItemSet NumberFormatDialog::CreateEmptyItemSetForNumberFormatDialog( SfxItemPool& rItemPool )
{
    static const USHORT nWhichPairs[] =
    {
        SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_INFO,
        SID_ATTR_NUMBERFORMAT_NOLANGUAGE, SID_ATTR_NUMBERFORMAT_NOLANGUAGE,
        SID_ATTR_NUMBERFORMAT_ONE_AREA, SID_ATTR_NUMBERFORMAT_ONE_AREA,
        SID_ATTR_NUMBERFORMAT_SOURCE, SID_ATTR_NUMBERFORMAT_SOURCE,
        0
    };
    return SfxItemSet( rItemPool, nWhichPairs );
}

//.............................................................................
} //namespace chart
//.............................................................................
