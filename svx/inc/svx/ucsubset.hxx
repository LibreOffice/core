/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ucsubset.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:43:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVX_SUBSETMAP_HXX
#define _SVX_SUBSETMAP_HXX

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#include <svx/ucsubset.hrc>
#include <vcl/metric.hxx>

#include <list>

class FontCharMap;

// classes Subset & SubsetMap --------------------------------------------
// TODO: should be moved into Font Attributes territory,
// we let them mature here though because this is currently the only use

class Subset
{
public:
    Subset( sal_UCS4 nMin, sal_UCS4 nMax, const String& aName )
    : mnRangeMin(nMin), mnRangeMax(nMax), maRangeName(aName)
    {}

    Subset( sal_UCS4 nMin, sal_UCS4 nMax, int resId );

    sal_UCS4        GetRangeMin() const { return mnRangeMin;}
    sal_UCS4        GetRangeMax() const { return mnRangeMax;}
    const String    GetName()     const { return maRangeName;}

private:
    sal_UCS4        mnRangeMin;
    sal_UCS4        mnRangeMax;
    String          maRangeName;
};

typedef ::std::list<Subset> SubsetList;

class SVX_DLLPUBLIC SubsetMap : private Resource
{
public:
    SubsetMap( const FontCharMap* );

    const Subset*   GetSubsetByUnicode( sal_UCS4 ) const;
    const Subset*   GetNextSubset( bool bFirst ) const;

private:
    SubsetList      maSubsets;
    mutable SubsetList::const_iterator maSubsetIterator;

    SVX_DLLPRIVATE void            InitList();
    SVX_DLLPRIVATE void            ApplyCharMap( const FontCharMap* );
};

#endif
