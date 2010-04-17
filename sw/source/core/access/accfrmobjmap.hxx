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

#include <map>

class SwDoc;

class SwFrmOrObjMapKey
{
public:
    enum LayerId { INVALID, HELL, TEXT, HEAVEN, CONTROLS };
private:

    LayerId eLayerId;
    sal_uInt32 nOrdNum;

public:

    inline SwFrmOrObjMapKey();
    inline SwFrmOrObjMapKey( LayerId eId, sal_uInt32 nOrd );

    inline sal_Bool operator()( const SwFrmOrObjMapKey& r1,
                                 const SwFrmOrObjMapKey& r2 ) const;
};

typedef ::std::map < SwFrmOrObjMapKey, SwFrmOrObj, SwFrmOrObjMapKey >
    _SwFrmOrObjMap;

class SwFrmOrObjMap : public _SwFrmOrObjMap
{
    SdrLayerID nHellId;
    SdrLayerID nControlsId;
    sal_Bool bLayerIdsValid;

    ::std::pair< iterator, bool > insert( sal_uInt32 nPos,
                                          const SwFrmOrObj& rLower );
    ::std::pair< iterator, bool > insert( const SdrObject *pObj,
                                              const SwFrmOrObj& rLower,
                                             const SwDoc *pDoc  );

public:

    SwFrmOrObjMap( const SwRect& rVisArea, const SwFrm *pFrm );

    inline static sal_Bool IsSortingRequired( const SwFrm *pFrm );
};

inline SwFrmOrObjMapKey::SwFrmOrObjMapKey() :
    eLayerId( INVALID ),
    nOrdNum( 0 )
{
}

inline SwFrmOrObjMapKey::SwFrmOrObjMapKey(
        LayerId eId, sal_uInt32 nOrd ) :
    eLayerId( eId ),
    nOrdNum( nOrd )
{
}

inline sal_Bool SwFrmOrObjMapKey::operator()(
        const SwFrmOrObjMapKey& r1,
        const SwFrmOrObjMapKey& r2 ) const
{
    return (r1.eLayerId == r2.eLayerId) ? (r1.nOrdNum < r2.nOrdNum) :
           (r1.eLayerId < r2.eLayerId);
}

inline sal_Bool SwFrmOrObjMap::IsSortingRequired( const SwFrm *pFrm )
{
    return ( pFrm->IsPageFrm() &&
             static_cast< const SwPageFrm * >( pFrm )->GetSortedObjs() ) ||
            (pFrm->IsTxtFrm() && pFrm->GetDrawObjs() );
}

#endif

