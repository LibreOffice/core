/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _FTNIDX_HXX
#define _FTNIDX_HXX

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



#endif // _FTNIDX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
