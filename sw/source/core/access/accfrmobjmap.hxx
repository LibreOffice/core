/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _ACCFRMOBJMAP_HXX
#define _ACCFRMOBJMAP_HXX

#include <accfrmobj.hxx>

#include <svx/svdtypes.hxx>
#include <tools/gen.hxx>

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
        , nPosNum( 0, 0 )
    {}

    inline SwAccessibleChildMapKey( LayerId eId, sal_uInt32 nOrd )
        : eLayerId( eId )
        , nOrdNum( nOrd )
        , nPosNum( 0, 0 )
    {}

    inline SwAccessibleChildMapKey( LayerId eId, sal_uInt32 nOrd, Point nPos )
        : eLayerId( eId )
        , nOrdNum( nOrd )
        , nPosNum( nPos )
    {}

    inline bool operator()( const SwAccessibleChildMapKey& r1,
                            const SwAccessibleChildMapKey& r2 ) const
    {
//        return (r1.eLayerId == r2.eLayerId)
//               ? (r1.nOrdNum < r2.nOrdNum)
//               : (r1.eLayerId < r2.eLayerId);
    return (r1.eLayerId == r2.eLayerId) ?
           ( (r1.nPosNum == r2.nPosNum) ?(r1.nOrdNum < r2.nOrdNum) :
           (r1.nPosNum.getY() == r2.nPosNum.getY()? r1.nPosNum.getX() < r2.nPosNum.getX() :
            r1.nPosNum.getY() < r2.nPosNum.getY()) ) :
           (r1.eLayerId < r2.eLayerId);
    }

    /* MT: Need to get this position parameter stuff in dev300 somehow...
    //This methods are used to insert an object to the map, adding a position parameter.
    ::std::pair< iterator, bool > insert( sal_uInt32 nOrd, Point nPos,
                                          const SwFrmOrObj& rLower );
    ::std::pair< iterator, bool > insert( const SdrObject *pObj,
                                          const SwFrmOrObj& rLower,
                                          const SwDoc *pDoc,
                                          Point nPos);
    */

private:

    LayerId eLayerId;
    sal_uInt32 nOrdNum;

    Point nPosNum;

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

