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

#include "optHeaderTabListbox.hxx"
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svtools/headbar.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/treelistentry.hxx>

namespace svx
{
// class OptLBoxString_Impl ----------------------------------------------

class OptLBoxString_Impl : public SvLBoxString
{
public:
    OptLBoxString_Impl( SvTreeListEntry* pEntry, sal_uInt16 nFlags, const OUString& rTxt ) :
        SvLBoxString( pEntry, nFlags, rTxt ) {}

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry* pEntry) SAL_OVERRIDE;
};



void OptLBoxString_Impl::Paint(const Point& rPos, SvTreeListBox& /*rDev*/, vcl::RenderContext& rRenderContext,
                               const SvViewDataEntry* /*pView*/, const SvTreeListEntry* pEntry)
{
    rRenderContext.Push(PushFlags::FONT);
    vcl::Font aFont(rRenderContext.GetFont());
    //detect readonly state by asking for a valid Image
    if (pEntry && !(!SvTreeListBox::GetCollapsedEntryBmp(pEntry)))
        aFont.SetColor(Application::GetSettings().GetStyleSettings().GetDeactiveTextColor());
    rRenderContext.SetFont(aFont);
    rRenderContext.DrawText(rPos, GetText());
    rRenderContext.Pop();
}


OptHeaderTabListBox::OptHeaderTabListBox(SvSimpleTableContainer& rParent, WinBits nWinStyle)
    : SvSimpleTable(rParent, nWinStyle)
{
}


void OptHeaderTabListBox::InitEntry( SvTreeListEntry* pEntry, const OUString& rTxt,
                                     const Image& rImg1, const Image& rImg2,
                                     SvLBoxButtonKind eButtonKind )
{
    SvTabListBox::InitEntry( pEntry, rTxt, rImg1, rImg2, eButtonKind );
    sal_uInt16 _nTabCount = TabCount();

    for ( sal_uInt16 nCol = 1; nCol < _nTabCount; ++nCol )
    {
        // initialize all columns with own class (column 0 == Bitmap)
        SvLBoxString* pCol = static_cast<SvLBoxString*>(pEntry->GetItem( nCol ));
        OptLBoxString_Impl* pStr = new OptLBoxString_Impl( pEntry, 0, pCol->GetText() );
        pEntry->ReplaceItem( pStr, nCol );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
