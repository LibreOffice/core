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

#ifndef _DBAUI_INDEXES_HXX_
#define _DBAUI_INDEXES_HXX_

#include <tools/string.hxx>
#include <comphelper/stl_types.hxx>

namespace dbaui
{
    // OIndexField
    struct OIndexField
    {
        String              sFieldName;
        sal_Bool            bSortAscending;

        OIndexField() : bSortAscending(sal_True) { }
    };

    DECLARE_STL_VECTOR( OIndexField, IndexFields );

    // OIndex
    struct GrantIndexAccess
    {
        friend class OIndexCollection;
    private:
        GrantIndexAccess() { }
    };

    struct OIndex
    {
    protected:
        OUString     sOriginalName;
        sal_Bool            bModified;

    public:
        OUString     sName;
        OUString     sDescription;
        sal_Bool            bPrimaryKey;
        sal_Bool            bUnique;
        IndexFields         aFields;

    public:
        OIndex(const OUString& _rOriginalName)
            : sOriginalName(_rOriginalName), bModified(sal_False), sName(_rOriginalName), bPrimaryKey(sal_False), bUnique(sal_False)
        {
        }

        const OUString& getOriginalName() const { return sOriginalName; }

        sal_Bool    isModified() const { return bModified; }
        void        setModified(sal_Bool _bModified) { bModified = _bModified; }
        void        clearModified() { setModified(sal_False); }

        sal_Bool    isNew() const { return getOriginalName().isEmpty(); }
        void        flagAsNew(const GrantIndexAccess&) { sOriginalName = OUString(); }
        void        flagAsCommitted(const GrantIndexAccess&) { sOriginalName = sName; }

    private:
        OIndex();   // not implemented
    };

    DECLARE_STL_VECTOR( OIndex, Indexes );

}   // namespace dbaui

#endif // _DBAUI_INDEXES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
