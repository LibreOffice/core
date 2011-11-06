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


#ifndef _FMTANCHR_HXX
#define _FMTANCHR_HXX

#include "swdllapi.h"
#include <hintids.hxx>
#include <swtypes.hxx>
#include <format.hxx>
#include <svl/poolitem.hxx>

struct SwPosition;
class IntlWrapper;
#define IVER_FMTANCHOR_LONGIDX ((sal_uInt16)1)

//FlyAnchor, Anker des Freifliegenden Rahmen ----

class SW_DLLPUBLIC SwFmtAnchor: public SfxPoolItem
{
    SwPosition *pCntntAnchor;   //0 Fuer Seitengebundene Rahmen.
                                //Index fuer Absatzgebundene Rahmen.
                                //Position fuer Zeichengebundene Rahmen
    RndStdIds  nAnchorId;
    sal_uInt16     nPageNum;        //Seitennummer bei Seitengeb. Rahmen.

    // OD 2004-05-05 #i28701# - getting anchor positions ordered
    sal_uInt32 mnOrder;
    static sal_uInt32 mnOrderCounter;

public:
    SwFmtAnchor( RndStdIds eRnd = FLY_AT_PAGE, sal_uInt16 nPageNum = 0 );
    SwFmtAnchor( const SwFmtAnchor &rCpy );
    ~SwFmtAnchor();

    //  Zuweisungsoperator
    SwFmtAnchor &operator=( const SwFmtAnchor& );

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

    RndStdIds GetAnchorId() const { return nAnchorId; }
    sal_uInt16 GetPageNum() const { return nPageNum; }
    const SwPosition *GetCntntAnchor() const { return pCntntAnchor; }
    // OD 2004-05-05 #i28701#
    sal_uInt32 GetOrder() const;

    void SetType( RndStdIds nRndId ) { nAnchorId = nRndId; }
    void SetPageNum( sal_uInt16 nNew ) { nPageNum = nNew; }
    void SetAnchor( const SwPosition *pPos );
};

inline const SwFmtAnchor &SwAttrSet::GetAnchor(sal_Bool bInP) const
    { return static_cast<const SwFmtAnchor&>(Get(RES_ANCHOR, bInP)); }

 inline const SwFmtAnchor &SwFmt::GetAnchor(sal_Bool bInP) const
     { return aSet.GetAnchor(bInP); }

#endif

