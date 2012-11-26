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


#ifndef _SVX_PROTITEM_HXX
#define _SVX_PROTITEM_HXX

// include ---------------------------------------------------------------


#include <svl/poolitem.hxx>
#include "editeng/editengdllapi.h"

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxProtectItem --------------------------------------------------


/*
[Beschreibung]
Dieses Item beschreibt, ob Inhalt, Groesse oder Position geschuetzt werden
sollen.
*/

class EDITENG_DLLPUBLIC SvxProtectItem : public SfxPoolItem
{
    sal_Bool bCntnt :1;     //Inhalt geschuetzt
    sal_Bool bSize  :1;     //Groesse geschuetzt
    sal_Bool bPos   :1;     //Position geschuetzt

public:
    POOLITEM_FACTORY()
    inline SvxProtectItem( const sal_uInt16 nId = 0 );
    inline SvxProtectItem &operator=( const SvxProtectItem &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;


    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const;

    sal_Bool IsCntntProtected() const { return bCntnt; }
    sal_Bool IsSizeProtected()  const { return bSize;  }
    sal_Bool IsPosProtected()   const { return bPos;   }
    void SetCntntProtect( sal_Bool bNew ) { bCntnt = bNew; }
    void SetSizeProtect ( sal_Bool bNew ) { bSize  = bNew; }
    void SetPosProtect  ( sal_Bool bNew ) { bPos   = bNew; }

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

inline SvxProtectItem::SvxProtectItem( const sal_uInt16 nId )
    : SfxPoolItem( nId )
{
    bCntnt = bSize = bPos = sal_False;
}

inline SvxProtectItem &SvxProtectItem::operator=( const SvxProtectItem &rCpy )
{
    bCntnt = rCpy.IsCntntProtected();
    bSize  = rCpy.IsSizeProtected();
    bPos   = rCpy.IsPosProtected();
    return *this;
}




#endif

