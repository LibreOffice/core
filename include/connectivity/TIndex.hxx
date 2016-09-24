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

#ifndef INCLUDED_CONNECTIVITY_TINDEX_HXX
#define INCLUDED_CONNECTIVITY_TINDEX_HXX

#include <connectivity/dbtoolsdllapi.hxx>
#include <connectivity/sdbcx/VIndex.hxx>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>

namespace connectivity
{
    class OTableHelper;
    class OOO_DLLPUBLIC_DBTOOLS OIndexHelper : public connectivity::sdbcx::OIndex
    {
        OTableHelper*   m_pTable;
    public:
        virtual void refreshColumns() override;
    public:
        OIndexHelper(   OTableHelper* _pTable);
        OIndexHelper(   OTableHelper* _pTable,
                const OUString& Name,
                const OUString& Catalog,
                bool _isUnique,
                bool _isPrimaryKeyIndex,
                bool _isClustered
            );
        inline OTableHelper* getTable() const { return m_pTable; }
    };
}
#endif // INCLUDED_CONNECTIVITY_TINDEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
