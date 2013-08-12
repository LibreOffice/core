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

#ifndef _DBAUI_LISTVIEWITEMS_HXX_
#define _DBAUI_LISTVIEWITEMS_HXX_

#include <svtools/svlbitm.hxx>

//........................................................................
namespace dbaui
{
//........................................................................

    #define SV_ITEM_ID_BOLDLBSTRING SV_ITEM_ID_LBOXSTRING

    //====================================================================
    //= OBoldListboxString
    //====================================================================
    class OBoldListboxString : public SvLBoxString
    {
        sal_Bool    m_bEmphasized;

    public:
        OBoldListboxString(SvTreeListEntry* _pEntry, sal_uInt16 _nFlags, const OUString& _rStr)
            :SvLBoxString(_pEntry, _nFlags, _rStr)
            ,m_bEmphasized(sal_False)
        {
        }

        virtual sal_uInt16 GetType() const;

        virtual void Paint(
            const Point& rPos, SvTreeListBox& rOutDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry);
        virtual void InitViewData( SvTreeListBox* pView,SvTreeListEntry* pEntry, SvViewDataItem* _pViewData);

        sal_Bool    isEmphasized() const { return m_bEmphasized; }
        void        emphasize(sal_Bool _bEmphasize) { m_bEmphasized = _bEmphasize; }
    };

//........................................................................
}   // namespace dbaui
//........................................................................

#endif // _DBAUI_LISTVIEWITEMS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
