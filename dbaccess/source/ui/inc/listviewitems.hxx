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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_LISTVIEWITEMS_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_LISTVIEWITEMS_HXX

#include <svtools/svlbitm.hxx>

namespace dbaui
{

    #define SV_ITEM_ID_BOLDLBSTRING SvLBoxItemType::String

    // OBoldListboxString
    class OBoldListboxString : public SvLBoxString
    {
        bool    m_bEmphasized;

    public:
        OBoldListboxString(const OUString& _rStr)
            :SvLBoxString(_rStr)
            ,m_bEmphasized(false)
        {
        }

        virtual SvLBoxItemType GetType() const override;

        virtual void Paint(const Point& rPos, SvTreeListBox& rOutDev, vcl::RenderContext& rRenderContext,
                           const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;
        virtual void InitViewData( SvTreeListBox* pView,SvTreeListEntry* pEntry, SvViewDataItem* _pViewData = nullptr) override;

        bool    isEmphasized() const { return m_bEmphasized; }
        void        emphasize(bool _bEmphasize) { m_bEmphasized = _bEmphasize; }
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_LISTVIEWITEMS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
