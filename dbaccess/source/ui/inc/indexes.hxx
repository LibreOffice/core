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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_INDEXES_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_INDEXES_HXX

#include <sal/config.h>

#include <vector>

namespace dbaui
{
    // OIndexField
    struct OIndexField
    {
        OUString            sFieldName;
        bool            bSortAscending;

        OIndexField() : bSortAscending(true) { }
    };

    typedef std::vector<OIndexField> IndexFields;

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
        bool            bModified;

    public:
        OUString     sName;
        OUString     sDescription;
        bool            bPrimaryKey;
        bool            bUnique;
        IndexFields         aFields;

    public:
        OIndex(const OUString& _rOriginalName)
            : sOriginalName(_rOriginalName), bModified(false), sName(_rOriginalName), bPrimaryKey(false), bUnique(false)
        {
        }

        const OUString& getOriginalName() const { return sOriginalName; }

        bool    isModified() const { return bModified; }
        void        setModified(bool _bModified) { bModified = _bModified; }
        void        clearModified() { setModified(false); }

        bool    isNew() const { return getOriginalName().isEmpty(); }
        void        flagAsNew(const GrantIndexAccess&) { sOriginalName.clear(); }
        void        flagAsCommitted(const GrantIndexAccess&) { sOriginalName = sName; }

    private:
        OIndex();   // not implemented
    };

    typedef std::vector<OIndex> Indexes;

}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_INDEXES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
