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

#ifndef INCLUDED_SVX_UCSUBSET_HXX
#define INCLUDED_SVX_UCSUBSET_HXX

#include <svx/svxdllapi.h>

#include <vcl/vclenum.hxx>
#include <vcl/metric.hxx>

#include <vector>

class FontCharMap;

// TODO: should be moved into Font Attributes territory,
// we let them mature here though because this is currently the only use

class Subset
{
public:
    Subset(sal_UCS4 nMin, sal_UCS4 nMax, const OUString& rName);

    sal_UCS4        GetRangeMin() const { return mnRangeMin;}
    sal_UCS4        GetRangeMax() const { return mnRangeMax;}
    const OUString& GetName()     const { return maRangeName;}

private:
    sal_UCS4        mnRangeMin;
    sal_UCS4        mnRangeMax;
    OUString        maRangeName;
};

inline bool operator<(const Subset &rLHS, const Subset &rRHS)
{
    return rLHS.GetRangeMin() < rRHS.GetRangeMin();
}

typedef ::std::vector<Subset> SubsetVec;

class SVX_DLLPUBLIC SubsetMap
{
public:
    SubsetMap( const FontCharMapRef& );

    const Subset*   GetSubsetByUnicode( sal_UCS4 ) const;
    const SubsetVec&   GetSubsetMap() const;

private:
    SubsetVec      maSubsets;

    SVX_DLLPRIVATE void            InitList();
    SVX_DLLPRIVATE void            ApplyCharMap( const FontCharMapRef& );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
