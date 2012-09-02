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
#ifndef _FMTCNTNT_HXX
#define _FMTCNTNT_HXX

#include <svl/poolitem.hxx>
#include <hintids.hxx>
#include <format.hxx>

class SwNodeIndex;
class SwStartNode;


/// Cntnt, content of frame (header, footer, fly).
class SW_DLLPUBLIC SwFmtCntnt: public SfxPoolItem
{
    SwNodeIndex *pStartNode;

    SwFmtCntnt &operator=( const SwFmtCntnt & ); ///< Copying not allowed.

public:
    SwFmtCntnt( const SwStartNode* pStartNode = 0 );
    SwFmtCntnt( const SwFmtCntnt &rCpy );
    ~SwFmtCntnt();

    /// "Pure virtual methods" of SfxPoolItem.
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    const SwNodeIndex *GetCntntIdx() const { return pStartNode; }
    void SetNewCntntIdx( const SwNodeIndex *pIdx );
};

inline const SwFmtCntnt &SwAttrSet::GetCntnt(sal_Bool bInP) const
    { return (const SwFmtCntnt&)Get( RES_CNTNT,bInP); }

inline const SwFmtCntnt &SwFmt::GetCntnt(sal_Bool bInP) const
    { return aSet.GetCntnt(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
