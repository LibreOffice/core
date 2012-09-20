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
#ifndef _ACCFRMOBJMAP_HXX
#define _ACCFRMOBJMAP_HXX

#include <accfrmobj.hxx>

#include <svx/svdtypes.hxx>

#include <map>

class SwAccessibleMap;
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

    static bool IsSortingRequired( const SwFrm& rFrm );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
