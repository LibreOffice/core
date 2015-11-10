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
#ifndef INCLUDED_SW_INC_FTNIDX_HXX
#define INCLUDED_SW_INC_FTNIDX_HXX

#include <vector>
#include <sal/types.h>
#include <o3tl/sorted_vector.hxx>

class SwTextFootnote;
class SwNodeIndex;
class SwSectionNode;

// Everywhere where NodeIndex is used, the header files missing here
// are already included. Therefore put here as defines only and
// not as inline methods (saves compile time).
#define _SwTextFootnote_GetIndex( pFIdx ) (pFIdx->GetTextNode().GetIndex())

struct CompareSwFootnoteIdxs
{
    bool operator()(SwTextFootnote* const& lhs, SwTextFootnote* const& rhs) const;
};

class SwFootnoteIdxs : public o3tl::sorted_vector<SwTextFootnote*, CompareSwFootnoteIdxs>
{
public:
    SwFootnoteIdxs() {}

    void UpdateFootnote( const SwNodeIndex& rStt ); // Update all from pos.
    void UpdateAllFootnote();                       // Update all footnotes.

    SwTextFootnote* SeekEntry( const SwNodeIndex& rIdx, size_t* pPos = nullptr ) const;
};

class SwUpdFootnoteEndNtAtEnd
{
    std::vector<const SwSectionNode*> aFootnoteSects, aEndSects;
    std::vector<sal_uInt16> aFootnoteNums, aEndNums;

public:
    SwUpdFootnoteEndNtAtEnd() : aFootnoteSects(), aEndSects() {}

    static const SwSectionNode* FindSectNdWithEndAttr(
                                            const SwTextFootnote& rTextFootnote );

    sal_uInt16 GetNumber( const SwTextFootnote& rTextFootnote, const SwSectionNode& rNd );
    sal_uInt16 ChkNumber( const SwTextFootnote& rTextFootnote );
};

#endif // INCLUDED_SW_INC_FTNIDX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
