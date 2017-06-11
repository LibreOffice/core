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
#include <svx/papersizelistbox.hxx>
#include <vcl/builderfactory.hxx>
#include "page.hrc"

PaperSizeListBox::PaperSizeListBox(vcl::Window* pParent)
    : ListBox( pParent, WB_BORDER | WB_DROPDOWN)
{
    SetDropDownLineCount(6);
}

VCL_BUILDER_FACTORY(PaperSizeListBox);

void PaperSizeListBox::FillPaperSizeEntries( PaperSizeApp eApp )
{
    const std::pair<const char*, int>* pPaperAry = eApp == PaperSizeApp::Std ?
        RID_SVXSTRARY_PAPERSIZE_STD : RID_SVXSTRARY_PAPERSIZE_DRAW;
    sal_uInt32 nCnt = eApp == PaperSizeApp::Std ?
        SAL_N_ELEMENTS(RID_SVXSTRARY_PAPERSIZE_STD) : SAL_N_ELEMENTS(RID_SVXSTRARY_PAPERSIZE_DRAW);

    for ( sal_uInt32 i = 0; i < nCnt; ++i )
    {
        OUString aStr = SvxResId(pPaperAry[i].first);
        Paper eSize = (Paper)pPaperAry[i].second;
        sal_Int32 nPos = InsertEntry( aStr );
        SetEntryData( nPos, reinterpret_cast<void*>((sal_uLong)eSize) );
    }
}

void PaperSizeListBox::SetSelection( Paper ePreselectPaper )
{
    sal_Int32 nEntryCount = GetEntryCount();
    sal_Int32 nSelPos = LISTBOX_ENTRY_NOTFOUND;
    sal_Int32 nUserPos = LISTBOX_ENTRY_NOTFOUND;

    for (sal_Int32 i = 0; i < nEntryCount; ++i )
    {
        Paper eTmp = (Paper)reinterpret_cast<sal_uLong>(GetEntryData(i));

        if ( eTmp == ePreselectPaper )
        {
            nSelPos = i;
            break;
        }

        if ( eTmp == PAPER_USER )
           nUserPos = i;
    }

    // preselect current paper format - #115915#: ePaper might not be in aPaperSizeBox so use PAPER_USER instead
    SelectEntryPos( ( nSelPos != LISTBOX_ENTRY_NOTFOUND ) ? nSelPos : nUserPos );
}

Paper PaperSizeListBox::GetSelection() const
{
    const sal_Int32 nPos = GetSelectEntryPos();
    Paper ePaper = (Paper)reinterpret_cast<sal_uLong>(GetEntryData( nPos ));

    return ePaper;
}

Size PaperSizeListBox::GetOptimalSize() const
{
    return Size(150, ListBox::GetOptimalSize().Height());
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

