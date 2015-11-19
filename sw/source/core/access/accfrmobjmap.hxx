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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCFRMOBJMAP_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCFRMOBJMAP_HXX

#include <tools/gen.hxx>
#include <svx/svdtypes.hxx>
#include <accfrmobj.hxx>
#include <map>

class SwAccessibleMap;
class SwRect;
class SwFrame;
class SdrObject;

class SwAccessibleChildMapKey
{
public:
    enum LayerId { INVALID, HELL, TEXT, HEAVEN, CONTROLS, XWINDOW };

    SwAccessibleChildMapKey()
        : eLayerId( INVALID )
        , nOrdNum( 0 )
        , nPosNum( 0, 0 )
    {}

    SwAccessibleChildMapKey( LayerId eId, sal_uInt32 nOrd )
        : eLayerId( eId )
        , nOrdNum( nOrd )
        , nPosNum( 0, 0 )
    {}

    SwAccessibleChildMapKey( LayerId eId, sal_uInt32 nOrd, Point nPos )
        : eLayerId( eId )
        , nOrdNum( nOrd )
        , nPosNum( nPos )
    {}

    bool operator()( const SwAccessibleChildMapKey& r1,
                            const SwAccessibleChildMapKey& r2 ) const
    {
        if(r1.eLayerId == r2.eLayerId)
        {
            if(r1.nPosNum == r2.nPosNum)
                return r1.nOrdNum < r2.nOrdNum;
            else
            {
                if(r1.nPosNum.getY() == r2.nPosNum.getY())
                    return r1.nPosNum.getX() < r2.nPosNum.getX();
                else
                    return r1.nPosNum.getY() < r2.nPosNum.getY();
            }
        }
        else
            return r1.eLayerId < r2.eLayerId;
    }

    /* MT: Need to get this position parameter stuff in dev300 somehow...
    //This methods are used to insert an object to the map, adding a position parameter.
    ::std::pair< iterator, bool > insert( sal_uInt32 nOrd, Point nPos,
                                          const SwFrameOrObj& rLower );
    ::std::pair< iterator, bool > insert( const SdrObject *pObj,
                                          const SwFrameOrObj& rLower,
                                          const SwDoc *pDoc,
                                          Point nPos);
    */

private:
    LayerId eLayerId;
    sal_uInt32 nOrdNum;
    Point nPosNum;
};


class SwAccessibleChildMap
{
public:
    typedef SwAccessibleChildMapKey                                             key_type;
    typedef sw::access::SwAccessibleChild                                       mapped_type;
    typedef std::pair<const key_type,mapped_type>                               value_type;
    typedef SwAccessibleChildMapKey                                             key_compare;
    typedef std::map<key_type,mapped_type,key_compare>::iterator                iterator;
    typedef std::map<key_type,mapped_type,key_compare>::const_iterator          const_iterator;
    typedef std::map<key_type,mapped_type,key_compare>::const_reverse_iterator  const_reverse_iterator;

private:
    const SdrLayerID nHellId;
    const SdrLayerID nControlsId;
    std::map<key_type,mapped_type,key_compare> maMap;

    ::std::pair< iterator, bool > insert( const sal_uInt32 nPos,
                                          const SwAccessibleChildMapKey::LayerId eLayerId,
                                          const sw::access::SwAccessibleChild& rLower );
    ::std::pair< iterator, bool > insert( const SdrObject* pObj,
                                          const sw::access::SwAccessibleChild& rLower );

public:
    SwAccessibleChildMap( const SwRect& rVisArea,
                          const SwFrame& rFrame,
                          SwAccessibleMap& rAccMap );

    static bool IsSortingRequired( const SwFrame& rFrame );

    const_iterator cbegin() const { return maMap.cbegin(); }
    const_iterator cend() const { return maMap.cend(); }
    const_reverse_iterator crbegin() const { return maMap.crbegin(); }
    const_reverse_iterator crend() const { return maMap.crend(); }

    std::pair<iterator,bool> insert(const value_type& value) { return maMap.insert(value); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
