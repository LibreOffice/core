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

#include <listviewitems.hxx>
#include <svtools/viewdataentry.hxx>

namespace dbaui
{

    // class OBoldListboxString
    void OBoldListboxString::InitViewData( SvTreeListBox* pView,SvTreeListEntry* pEntry, SvViewDataItem* _pViewData)
    {
        SvLBoxString::InitViewData( pView, pEntry, _pViewData );
        if ( !m_bEmphasized )
            return;
        if (!_pViewData)
            _pViewData = pView->GetViewDataItem( pEntry, this );
        pView->Push();
        vcl::Font aFont( pView->GetFont());
        aFont.SetWeight(WEIGHT_BOLD);
        pView->Control::SetFont( aFont );
        _pViewData->maSize = Size(pView->GetTextWidth(GetText()), pView->GetTextHeight());
        pView->Pop();
    }

    SvLBoxItemType OBoldListboxString::GetType() const
    {
        return SvLBoxItemType::String;
    }

    void OBoldListboxString::Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                                   const SvViewDataEntry* pView, const SvTreeListEntry& rEntry)
    {
        if (m_bEmphasized)
        {
            rRenderContext.Push();
            vcl::Font aFont(rRenderContext.GetFont());
            aFont.SetWeight(WEIGHT_BOLD);
            rRenderContext.SetFont(aFont);
            Point aPos(rPos);
            rRenderContext.DrawText(aPos, GetText());
            rRenderContext.Pop();
        }
        else
        {
            SvLBoxString::Paint(rPos, rDev, rRenderContext, pView, rEntry);
        }
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
