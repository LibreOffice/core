/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_chart2.hxx"

#include "dlg_NumberFormat.hxx"

// header for class SfxItemSet
#include <svl/itemset.hxx>

// header for class SvxNumberInfoItem
#define ITEMID_NUMBERINFO SID_ATTR_NUMBERFORMAT_INFO
#include <svx/numinf.hxx>

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <sfx2/tabdlg.hxx>

// header for SvNumberFormatter
#include <svl/zforlist.hxx>

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

SfxItemSet NumberFormatDialog::CreateEmptyItemSetForNumberFormatDialog( SfxItemPool& rItemPool )
{
    static const sal_uInt16 nWhichPairs[] =
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
