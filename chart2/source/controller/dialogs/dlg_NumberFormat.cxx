/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "dlg_NumberFormat.hxx"

#include <svl/itemset.hxx>
#include <svx/numinf.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <sfx2/tabdlg.hxx>
#include <svl/zforlist.hxx>

namespace chart
{
using namespace ::com::sun::star;

NumberFormatDialog::NumberFormatDialog(vcl::Window* pParent, SfxItemSet& rSet)
    : SfxSingleTabDialog(pParent, rSet, "FormatNumberDialog",
        "cui/ui/formatnumberdialog.ui")
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT );
    if ( fnCreatePage )
    {
        SfxTabPage* pTabPage = (*fnCreatePage)( get_content_area(), &rSet );
        pTabPage->PageCreated(rSet);
        SetTabPage(pTabPage);
    }
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
