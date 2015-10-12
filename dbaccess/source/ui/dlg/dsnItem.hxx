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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_DSNITEM_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_DSNITEM_HXX

#include <svl/poolitem.hxx>

namespace dbaccess
{
    class ODsnTypeCollection;
}
namespace dbaui
{
    // DbuTypeCollectionItem
    /** allows an ODsnTypeCollection to be transported in an SfxItemSet
    */
    class DbuTypeCollectionItem : public SfxPoolItem
    {
        ::dbaccess::ODsnTypeCollection* m_pCollection;

    public:
        TYPEINFO_OVERRIDE();
        DbuTypeCollectionItem(sal_Int16 nWhich = 0, ::dbaccess::ODsnTypeCollection* _pCollection = NULL);
        DbuTypeCollectionItem(const DbuTypeCollectionItem& _rSource);

        virtual bool             operator==(const SfxPoolItem& _rItem) const override;
        virtual SfxPoolItem*     Clone(SfxItemPool* _pPool = NULL) const override;

        ::dbaccess::ODsnTypeCollection* getCollection() const { return m_pCollection; }
    };
}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_DSNITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
