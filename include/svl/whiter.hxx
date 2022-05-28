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
#pragma once

#include <svl/svldllapi.h>
#include <svl/whichranges.hxx>

class SfxItemSet;
class SfxPoolItem;
enum class SfxItemState;

/**
 * Iterates over the which ids and the pool items arrays together (which are stored in parallel arrays).
 * Primarily so that we can call GetItemSet on the SfxItemSet and pass in a hint, which avoids
 * searching the array the SfxItemSet, which speeds up GetItemState greatly.
 */
class SVL_DLLPUBLIC SfxWhichIter
{
    const SfxItemSet& m_rItemSet;
    const WhichPair* m_pCurrentWhichPair;
    sal_uInt16 m_nOffsetFromStartOfCurrentWhichPair;
    /// Offset into m_ppItems array in SfxItemSet
    sal_uInt16 m_nItemsOffset;

public:
    SfxWhichIter(const SfxItemSet& rSet);

    sal_uInt16 GetCurWhich() const;
    sal_uInt16 NextWhich();
    sal_uInt16 FirstWhich();
    SfxItemState GetItemState(bool bSrchInParent = true,
                              const SfxPoolItem** ppItem = nullptr) const;
    void ClearItem();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
