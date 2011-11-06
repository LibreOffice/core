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


#ifndef _FMTSRND_HXX
#define _FMTSRND_HXX

#include "swdllapi.h"
#include <hintids.hxx>
#include <format.hxx>
#include <svl/eitem.hxx>

// --> OD 2006-08-15 #i68520# - refactoring
// separate enumeration <SwSurround> in own header file
#include <fmtsrndenum.hxx>
// <--
class IntlWrapper;

//SwFmtSurround, wie soll sich der ---------------
//  Dokumentinhalt unter dem Rahmen verhalten ---

class SW_DLLPUBLIC SwFmtSurround: public SfxEnumItem
{
    sal_Bool    bAnchorOnly :1;
    sal_Bool    bContour    :1;
    sal_Bool    bOutside    :1;
public:
    SwFmtSurround( SwSurround eNew = SURROUND_PARALLEL );
    SwFmtSurround( const SwFmtSurround & );
    inline SwFmtSurround &operator=( const SwFmtSurround &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual sal_uInt16          GetValueCount() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );


    SwSurround GetSurround()const { return SwSurround( GetValue() ); }
    sal_Bool    IsAnchorOnly()  const { return bAnchorOnly; }
    sal_Bool    IsContour()     const { return bContour; }
    sal_Bool    IsOutside()     const { return bOutside; }
    void    SetSurround  ( SwSurround eNew ){ SfxEnumItem::SetValue( sal_uInt16( eNew ) ); }
    void    SetAnchorOnly( sal_Bool bNew )      { bAnchorOnly = bNew; }
    void    SetContour( sal_Bool bNew )         { bContour = bNew; }
    void    SetOutside( sal_Bool bNew )         { bOutside = bNew; }
};

inline SwFmtSurround &SwFmtSurround::operator=( const SwFmtSurround &rCpy )
{
    bAnchorOnly = rCpy.IsAnchorOnly();
    bContour = rCpy.IsContour();
    bOutside = rCpy.IsOutside();
    SfxEnumItem::SetValue( rCpy.GetValue() );
    return *this;
}

inline const SwFmtSurround &SwAttrSet::GetSurround(sal_Bool bInP) const
    { return (const SwFmtSurround&)Get( RES_SURROUND,bInP); }

inline const SwFmtSurround &SwFmt::GetSurround(sal_Bool bInP) const
    { return aSet.GetSurround(bInP); }

#endif

