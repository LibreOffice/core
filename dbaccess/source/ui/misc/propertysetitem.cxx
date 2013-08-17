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

#include "propertysetitem.hxx"

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    // OPropertySetItem
    TYPEINIT1(OPropertySetItem, SfxPoolItem);
    OPropertySetItem::OPropertySetItem(sal_Int16 _nWhich)
        :SfxPoolItem(_nWhich)
    {
    }

    OPropertySetItem::OPropertySetItem(sal_Int16 _nWhich, const Reference< XPropertySet >& _rxSet)
        :SfxPoolItem(_nWhich)
        ,m_xSet(_rxSet)
    {
    }

    OPropertySetItem::OPropertySetItem(const OPropertySetItem& _rSource)
        :SfxPoolItem(_rSource)
        ,m_xSet(_rSource.m_xSet)
    {
    }

    int OPropertySetItem::operator==(const SfxPoolItem& _rItem) const
    {
        const OPropertySetItem* pCompare = PTR_CAST(OPropertySetItem, &_rItem);
        if ((!pCompare) || (pCompare->m_xSet.get() != m_xSet.get()))
            return 0;

        return 1;
    }

    SfxPoolItem* OPropertySetItem::Clone(SfxItemPool* /* _pPool */) const
    {
        return new OPropertySetItem(*this);
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
