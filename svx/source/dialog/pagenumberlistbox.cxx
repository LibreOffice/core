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
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/pagenumberlistbox.hxx>
#include <tools/resary.hxx>
#include <vcl/builderfactory.hxx>

PageNumberListBox::PageNumberListBox(vcl::Window* pParent, WinBits nStyle)
    : ListBox( pParent, nStyle)
{
    ResStringArray aPaperAry( SVX_RES( RID_SVXSTRARY_PAGE_NUMBERING ) );
    sal_uInt32 nCnt = aPaperAry.Count();

    for ( sal_uInt32 i = 0; i < nCnt; ++i )
    {
        OUString aStr = aPaperAry.GetString(i);
        sal_uInt16 nData = aPaperAry.GetValue(i);
        sal_Int32 nPos = InsertEntry( aStr );
        SetEntryData( nPos, reinterpret_cast<void*>((sal_uLong)nData) );
    }
}

VCL_BUILDER_FACTORY(PageNumberListBox);

void PageNumberListBox::SetSelection( sal_uInt16 nPos )
{
    sal_Int32 nEntryCount = GetEntryCount();
    sal_Int32 nSelPos = LISTBOX_ENTRY_NOTFOUND;
    sal_Int32 nUserPos = LISTBOX_ENTRY_NOTFOUND;

    for (sal_Int32 i = 0; i < nEntryCount; ++i )
    {
        sal_uInt16 nTmp = (sal_uInt16)reinterpret_cast<sal_uLong>(GetEntryData(i));

        if ( nTmp == nPos )
        {
            nSelPos = i;
            break;
        }
    }
    SelectEntryPos( ( nSelPos != LISTBOX_ENTRY_NOTFOUND ) ? nSelPos : nUserPos );
}

sal_uInt16 PageNumberListBox::GetSelection() const
{
    const sal_Int32 nPos = GetSelectEntryPos();
    sal_uInt16 nData = (sal_uInt16)reinterpret_cast<sal_uLong>(GetEntryData( nPos ));

    return nData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
