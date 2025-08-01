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

#include <o3tl/sorted_vector.hxx>

#include "nodeoffset.hxx"

class SwPaM;
class SwNodeIndex;

struct SwPamRange
{
    SwNodeOffset nStart, nEnd;

    SwPamRange( SwNodeOffset nS, SwNodeOffset nE ) : nStart( nS ), nEnd( nE ) {}

    bool operator==( const SwPamRange& rRg ) const
        { return nStart == rRg.nStart; }
    bool operator<( const SwPamRange& rRg ) const
        { return nStart < rRg.nStart; }
};

class SwPamRanges
{
public:
    SwPamRanges( const SwPaM& rRing );

    void Insert( const SwNode& rIdx1, const SwNode& rIdx2 );
    SwPaM& SetPam( size_t nArrPos, SwPaM& rPam );

    size_t Count() const
                { return maVector.size(); }
private:
    o3tl::sorted_vector<SwPamRange> maVector;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
