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
#include "accfrmobj.hxx"
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
        : m_eLayerId( INVALID )
        , m_nOrdNum( 0 )
    {}

    SwAccessibleChildMapKey( LayerId eId, sal_uInt32 nOrd )
        : m_eLayerId( eId )
        , m_nOrdNum( nOrd )
    {}

    bool operator()( const SwAccessibleChildMapKey& r1,
                            const SwAccessibleChildMapKey& r2 ) const
    {
        if(r1.m_eLayerId == r2.m_eLayerId)
            return r1.m_nOrdNum < r2.m_nOrdNum;
        else
            return r1.m_eLayerId < r2.m_eLayerId;
    }

private:
    LayerId m_eLayerId;
    sal_uInt32 m_nOrdNum;
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
    const SdrLayerID mnHellId;
    const SdrLayerID mnControlsId;
    std::map<key_type,mapped_type,key_compare> maMap;

    std::pair< iterator, bool > insert( const sal_uInt32 nPos,
                                          const SwAccessibleChildMapKey::LayerId eLayerId,
                                          const sw::access::SwAccessibleChild& rLower );
    std::pair< iterator, bool > insert( const SdrObject* pObj,
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

    template<class... Args>
    std::pair<iterator,bool> emplace(Args&&... args) { return maMap.emplace(std::forward<Args>(args)...); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
