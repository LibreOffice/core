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


#ifndef _FMTURL_HXX
#define _FMTURL_HXX

#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <format.hxx>

class ImageMap;
class IntlWrapper;

// URL, ServerMap und ClientMap

class SW_DLLPUBLIC SwFmtURL: public SfxPoolItem
{
    String    sTargetFrameName; // in diesen Frame soll die URL
    String    sURL;             //Einfache URL
    String    sName;            // Name des Anchors
    ImageMap *pMap;             //ClientSide Images

    sal_Bool      bIsServerMap;     //mit der URL eine ServerSideImageMap

    SwFmtURL& operator=( const SwFmtURL& );

public:
    SwFmtURL();

    // @@@ copy construction allowed, but assigment is not? @@@
    SwFmtURL( const SwFmtURL& );

    virtual ~SwFmtURL();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    void SetTargetFrameName( const String& rStr ) { sTargetFrameName = rStr; }
    void SetURL( const String &rURL, sal_Bool bServerMap );
    void SetMap( const ImageMap *pM );  //Pointer wird kopiert!

    const String   &GetTargetFrameName()const { return sTargetFrameName; }
    const String   &GetURL()            const { return sURL; }
          sal_Bool      IsServerMap()       const { return bIsServerMap; }
    const ImageMap *GetMap()            const { return pMap; }
          ImageMap *GetMap()                  { return pMap; }

    const String& GetName() const           { return sName; }
    void SetName( const String& rNm )       { sName = rNm; }
};


inline const SwFmtURL &SwAttrSet::GetURL(sal_Bool bInP) const
    { return (const SwFmtURL&)Get( RES_URL,bInP); }

inline const SwFmtURL &SwFmt::GetURL(sal_Bool bInP) const
    { return aSet.GetURL(bInP); }

#endif

