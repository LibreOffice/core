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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_OTLNBUFF_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_OTLNBUFF_HXX

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
