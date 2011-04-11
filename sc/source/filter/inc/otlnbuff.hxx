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

#ifndef SC_OTLNBUFF_HXX
#define SC_OTLNBUFF_HXX

#include <tools/solar.h>
#include <mdds/flat_segment_tree.hpp>
#include <set>

class ScOutlineArray;

class XclImpOutlineBuffer
{
public:
    XclImpOutlineBuffer( SCSIZE nNewSize );
    ~XclImpOutlineBuffer();

    void SetLevel( SCSIZE nIndex, sal_uInt8 nVal, bool bCollapsed );
    void SetOutlineArray( ScOutlineArray* pOArray );
    void MakeScOutline();
    void SetLevelRange( SCSIZE nF, SCSIZE nL, sal_uInt8 nVal, bool bCollapsed );
    void SetButtonMode( bool bRightOrUnder );

private:
    typedef ::mdds::flat_segment_tree<SCSIZE, sal_uInt8> OutlineLevels;
    OutlineLevels       maLevels;
    ::std::set<SCSIZE>  maCollapsedPosSet;
    ScOutlineArray*     mpOutlineArray;
    SCSIZE              mnEndPos;
    sal_uInt8           mnMaxLevel;
    bool                mbButtonAfter:1;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
