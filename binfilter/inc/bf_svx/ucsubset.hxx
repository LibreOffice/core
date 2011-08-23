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

#ifndef _SVX_SUBSETMAP_HXX
#define _SVX_SUBSETMAP_HXX

#include <bf_svx/ucsubset.hrc>
#include <vcl/metric.hxx>

#include <list>
namespace binfilter {

class FontCharMap;

// classes Subset & SubsetMap --------------------------------------------
// TODO: should be moved into Font Attributes territory,
// we let them mature here though because this is currently the only use

class Subset
{
public:
    Subset( sal_Unicode nMin, sal_Unicode nMax, const String& aName )
    : mnRangeMin(nMin), mnRangeMax(nMax), maRangeName(aName)
    {}

    Subset( sal_Unicode nMin, sal_Unicode nMax, int resId );

    sal_Unicode   GetRangeMin() const { return mnRangeMin;}
    sal_Unicode   GetRangeMax() const { return mnRangeMax;}
    const String  GetName()     const { return maRangeName;}

private:
    sal_Unicode   mnRangeMin;
    sal_Unicode   mnRangeMax;
    String        maRangeName;
};

typedef ::std::list<Subset> SubsetList;

class SubsetMap : private Resource
{
public:
    SubsetMap( const FontCharMap* );

    const Subset*   GetSubsetByUnicode( sal_Unicode ) const;
    const Subset*   GetNextSubset( bool bFirst ) const;

private:
    SubsetList      maSubsets;
    mutable SubsetList::const_iterator maSubsetIterator;

    void            InitList();
    void            ApplyCharMap( const FontCharMap* );
};

}//end of namespace binfilter
#endif
