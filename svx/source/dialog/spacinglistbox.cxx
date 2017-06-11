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
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/spacinglistbox.hxx>
#include <vcl/builderfactory.hxx>
#include "spacing.hrc"

SpacingListBox::SpacingListBox(vcl::Window* pParent)
    : ListBox( pParent, WB_BORDER | WB_DROPDOWN)
{
    for (size_t i = 0; i < SAL_N_ELEMENTS(RID_SVXSTRARY_SPACING); ++i)
    {
        OUString aStr = SvxResId(RID_SVXSTRARY_SPACING[i].first);
        sal_uInt16 nData = RID_SVXSTRARY_SPACING[i].second;
        sal_Int32 nPos = InsertEntry( aStr );
        SetEntryData( nPos, reinterpret_cast<void*>((sal_uLong)nData) );
    }
    SetDropDownLineCount(8);
    SelectEntryPos(0);
}

VCL_BUILDER_FACTORY(SpacingListBox);

Size SpacingListBox::GetOptimalSize() const
{
    return Size(150, ListBox::GetOptimalSize().Height());
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
