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


#ifndef _SVX_ULSPITEM_HXX
#define _SVX_ULSPITEM_HXX

// include ---------------------------------------------------------------

#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>

namespace rtl
{
    class OUString;
}

// class SvxULSpaceItem --------------------------------------------------

/*
[Beschreibung]
Dieses Item beschreibt den oberen und unteren Rand einer Seite oder Absatz.
*/

#define ULSPACE_16_VERSION  ((sal_uInt16)0x0001)

class EDITENG_DLLPUBLIC SvxULSpaceItem : public SfxPoolItem
{
    sal_uInt16 nUpper;  //Oberer Rand
    sal_uInt16 nLower;  //Unterer Rand
    sal_uInt16 nPropUpper, nPropLower;      // relativ oder absolut (=100%)
public:
    POOLITEM_FACTORY()
    SvxULSpaceItem( const sal_uInt16 nId = 0 );
    SvxULSpaceItem( const sal_uInt16 nUp, const sal_uInt16 nLow,
                    const sal_uInt16 nId  );
    inline SvxULSpaceItem& operator=( const SvxULSpaceItem &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const;
    virtual sal_uInt16           GetVersion( sal_uInt16 nFileVersion ) const;
    virtual void             ScaleMetrics( long nMult, long nDiv );
    virtual bool             HasMetrics() const;

    inline void SetUpper( const sal_uInt16 nU, const sal_uInt16 nProp = 100 );
    inline void SetLower( const sal_uInt16 nL, const sal_uInt16 nProp = 100 );

    void SetUpperValue( const sal_uInt16 nU ) { nUpper = nU; }
    void SetLowerValue( const sal_uInt16 nL ) { nLower = nL; }
    void SetPropUpper( const sal_uInt16 nU ) { nPropUpper = nU; }
    void SetPropLower( const sal_uInt16 nL ) { nPropLower = nL; }

    sal_uInt16 GetUpper() const { return nUpper; }
    sal_uInt16 GetLower() const { return nLower; }
    sal_uInt16 GetPropUpper() const { return nPropUpper; }
    sal_uInt16 GetPropLower() const { return nPropLower; }
};

inline SvxULSpaceItem &SvxULSpaceItem::operator=( const SvxULSpaceItem &rCpy )
{
    nUpper = rCpy.GetUpper();
    nLower = rCpy.GetLower();
    nPropUpper = rCpy.GetPropUpper();
    nPropLower = rCpy.GetPropLower();
    return *this;
}

inline void SvxULSpaceItem::SetUpper( const sal_uInt16 nU, const sal_uInt16 nProp )
{
    nUpper = sal_uInt16((sal_uInt32(nU) * nProp ) / 100); nPropUpper = nProp;
}
inline void SvxULSpaceItem::SetLower( const sal_uInt16 nL, const sal_uInt16 nProp )
{
    nLower = sal_uInt16((sal_uInt32(nL) * nProp ) / 100); nPropLower = nProp;
}

#endif

