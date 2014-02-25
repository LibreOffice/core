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

class SwTxtFtn;
class SwNodeIndex;
class SwSectionNode;

// Everywhere where NodeIndex is used, the header files missing here
// are already included. Therefore put here as defines only and
// not as inline methods (saves compile time).
#define _SwTxtFtn_GetIndex( pFIdx ) (pFIdx->GetTxtNode().GetIndex())

struct CompareSwFtnIdxs
{
    bool operator()(SwTxtFtn* const& lhs, SwTxtFtn* const& rhs) const;
};

class SwFtnIdxs : public o3tl::sorted_vector<SwTxtFtn*, CompareSwFtnIdxs>
{
public:
    SwFtnIdxs() {}

    void UpdateFtn( const SwNodeIndex& rStt ); // Update all from pos.
    void UpdateAllFtn();                       // Update all footnotes.

    SwTxtFtn* SeekEntry( const SwNodeIndex& rIdx, sal_uInt16* pPos = 0 ) const;
};

class SwUpdFtnEndNtAtEnd
{
    std::vector<const SwSectionNode*> aFtnSects, aEndSects;
    std::vector<sal_uInt16> aFtnNums, aEndNums;

public:
    SwUpdFtnEndNtAtEnd() : aFtnSects(), aEndSects() {}

    static const SwSectionNode* FindSectNdWithEndAttr(
                                            const SwTxtFtn& rTxtFtn );

    sal_uInt16 GetNumber( const SwTxtFtn& rTxtFtn, const SwSectionNode& rNd );
    sal_uInt16 ChkNumber( const SwTxtFtn& rTxtFtn );
};

#endif // INCLUDED_SW_INC_FTNIDX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
