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
#ifndef _FLYPOS_HXX
#define _FLYPOS_HXX


#include <swdllapi.h>
#include <set>

class SwFrmFmt;
class SwNodeIndex;


// For querying current flys in document.
class SW_DLLPUBLIC SwPosFlyFrm
{
    const SwFrmFmt* pFrmFmt;    // FlyFrmFmt
    SwNodeIndex* pNdIdx;        // Index for node is sufficient.
    sal_uInt32 nOrdNum;
public:
    SwPosFlyFrm( const SwNodeIndex& , const SwFrmFmt*, sal_uInt16 nArrPos );
    virtual ~SwPosFlyFrm(); // Virtual for Writer (DLL !!)

    // Operators for sort array.
    sal_Bool operator==( const SwPosFlyFrm& );
    sal_Bool operator<( const SwPosFlyFrm& );

    const SwFrmFmt& GetFmt() const { return *pFrmFmt; }
    const SwNodeIndex& GetNdIndex() const { return *pNdIdx; }
    sal_uInt32 GetOrdNum() const { return nOrdNum; }
};

class SwPosFlyFrms : public std::multiset<SwPosFlyFrm*> {};

#endif // _FLYPOS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
