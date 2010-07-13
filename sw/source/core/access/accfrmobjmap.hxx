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
#ifndef _ACCFRMOBJMAP_HXX
#define _ACCFRMOBJMAP_HXX

#include <accfrmobj.hxx>

#include <svx/svdtypes.hxx>

#include <map>

class SwAccessibleMap;
class SwDoc;
class SwRect;
class SwFrm;
class SdrObject;

class SwAccessibleChildMapKey
{
public:
    enum LayerId { INVALID, HELL, TEXT, HEAVEN, CONTROLS, XWINDOW };

    inline SwAccessibleChildMapKey()
        : eLayerId( INVALID )
        , nOrdNum( 0 )
    {}

    inline SwAccessibleChildMapKey( LayerId eId, sal_uInt32 nOrd )
        : eLayerId( eId )
        , nOrdNum( nOrd )
    {}

    inline bool operator()( const SwAccessibleChildMapKey& r1,
                            const SwAccessibleChildMapKey& r2 ) const
    {
        return (r1.eLayerId == r2.eLayerId)
               ? (r1.nOrdNum < r2.nOrdNum)
               : (r1.eLayerId < r2.eLayerId);
    }

private:

    LayerId eLayerId;
    sal_uInt32 nOrdNum;

};

typedef ::std::map < SwAccessibleChildMapKey, sw::access::SwAccessibleChild, SwAccessibleChildMapKey >
    _SwAccessibleChildMap;

class SwAccessibleChildMap : public _SwAccessibleChildMap
{
    const SdrLayerID nHellId;
    const SdrLayerID nControlsId;

    ::std::pair< iterator, bool > insert( const sal_uInt32 nPos,
                                          const SwAccessibleChildMapKey::LayerId eLayerId,
                                          const sw::access::SwAccessibleChild& rLower );
    ::std::pair< iterator, bool > insert( const SdrObject* pObj,
                                          const sw::access::SwAccessibleChild& rLower );

public:

    SwAccessibleChildMap( const SwRect& rVisArea,
                          const SwFrm& rFrm,
                          SwAccessibleMap& rAccMap );

    static sal_Bool IsSortingRequired( const SwFrm& rFrm );
};

#endif

