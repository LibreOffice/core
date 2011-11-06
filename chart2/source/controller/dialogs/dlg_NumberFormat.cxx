/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
