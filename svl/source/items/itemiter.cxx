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


#include <svl/itemiter.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>

SfxItemIter::SfxItemIter( const SfxItemSet& rItemSet )
    : m_rSet( rItemSet )
{
    if (!m_rSet.m_nCount)
    {
        m_nStart = 1;
        m_nEnd = 0;
    }
    else
    {
        SfxItemArray ppFnd = m_rSet.m_pItems;

        // Find the first Item that is set
        for (m_nStart = 0; !*(ppFnd + m_nStart ); ++m_nStart)
            ; // empty loop
        if (1 < m_rSet.Count())
            for (m_nEnd = m_rSet.TotalCount(); !*(ppFnd + --m_nEnd); )
                ; // empty loop
        else
            m_nEnd = m_nStart;
    }

    m_nCurrent = m_nStart;
}

SfxItemIter::~SfxItemIter()
{
}

const SfxPoolItem* SfxItemIter::NextItem()
{
    SfxItemArray ppFnd = m_rSet.m_pItems;

    if (m_nCurrent < m_nEnd)
    {
        do {
            m_nCurrent++;
        } while (m_nCurrent < m_nEnd && !*(ppFnd + m_nCurrent ));
        return *(ppFnd+m_nCurrent);
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
