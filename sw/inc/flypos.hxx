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
#ifndef INCLUDED_SW_INC_FLYPOS_HXX
#define INCLUDED_SW_INC_FLYPOS_HXX

#include <sal/types.h>
#include <memory>
#include <set>

class SwFrameFormat;
class SwNode;

/// For querying current flys in document.
class SwPosFlyFrame final
{
    const SwFrameFormat* m_pFrameFormat;    ///< FlyFrameFormat
    const SwNode* m_pNode;
    sal_uInt32 m_nOrdNum;

public:
    SwPosFlyFrame(const SwNode& rNd, const SwFrameFormat* pFormat, sal_uInt16 nArrPos);

    const SwFrameFormat& GetFormat() const { return *m_pFrameFormat; }
    const SwNode& GetNode() const { return *m_pNode; }
    sal_uInt32 GetOrdNum() const { return m_nOrdNum; }
};

struct SwPosFlyFrameCmp { bool operator()(const SwPosFlyFrame& rA, const SwPosFlyFrame& rB) const; };
typedef std::set< SwPosFlyFrame, SwPosFlyFrameCmp > SwPosFlyFrames;

#endif // INCLUDED_SW_INC_FLYPOS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
