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

#include <cbnumberformat.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/intitem.hxx>
#include <sc.hrc>

ScNumberFormat::ScNumberFormat(vcl::Window* pParent, WinBits nStyle) :
    ListBox(pParent, nStyle | WB_DROPDOWN|WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK)
{
    SetSelectHdl(LINK(this, ScNumberFormat, NumFormatSelectHdl));
    AdaptDropDownLineCountToMaximum();

    InsertEntry(ScResId(STR_GENERAL));
    InsertEntry(ScResId(STR_NUMBER));
    InsertEntry(ScResId(STR_PERCENT));
    InsertEntry(ScResId(STR_CURRENCY));
    InsertEntry(ScResId(STR_DATE));
    InsertEntry(ScResId(STR_TIME));
    InsertEntry(ScResId(STR_SCIENTIFIC));
    InsertEntry(ScResId(STR_FRACTION));
    InsertEntry(ScResId(STR_BOOLEAN_VALUE));
    InsertEntry(ScResId(STR_TEXT));
}

IMPL_STATIC_LINK(ScNumberFormat, NumFormatSelectHdl, ListBox&, rBox, void)
{
    if(SfxViewFrame::Current())
    {
        SfxDispatcher* pDisp = SfxViewFrame::Current()->GetBindings().GetDispatcher();
        if(pDisp)
        {
            const sal_Int32 nVal = rBox.GetSelectedEntryPos();
            SfxUInt16Item aItem(SID_NUMBER_TYPE_FORMAT, nVal);
            pDisp->ExecuteList(SID_NUMBER_TYPE_FORMAT,
                    SfxCallMode::RECORD, {&aItem});
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
