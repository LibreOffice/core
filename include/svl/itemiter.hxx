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
#ifndef INCLUDED_SVL_ITEMITER_HXX
#define INCLUDED_SVL_ITEMITER_HXX

#include <svl/svldllapi.h>
#include <svl/itemset.hxx>

class SfxPoolItem;
class SfxItemSet;
class SfxItemPool;

class SVL_DLLPUBLIC SfxItemIter
{
    const SfxItemSet&   m_rSet;
    sal_uInt16 m_nStart;
    sal_uInt16 m_nEnd;
    sal_uInt16 m_nCurrent;

public:
    SfxItemIter( const SfxItemSet& rSet );
    ~SfxItemIter();

    /// get item, or null if no items
    const SfxPoolItem* FirstItem()
    {
        m_nCurrent = m_nStart;
        return m_rSet.m_nCount ? *(m_rSet.m_pItems + m_nCurrent) : nullptr;
    }
    const SfxPoolItem* GetCurItem()
    {
        return m_rSet.m_nCount ? *(m_rSet.m_pItems + m_nCurrent) : nullptr;
    }
    const SfxPoolItem* NextItem();

    bool       IsAtEnd()     const { return m_nCurrent == m_nEnd; }

    sal_uInt16 GetCurPos()   const { return m_nCurrent; }
    sal_uInt16 GetFirstPos() const { return m_nStart; }
    sal_uInt16 GetLastPos()  const { return m_nEnd; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
