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


#ifndef _SVX_SHADITEM_HXX
#define _SVX_SHADITEM_HXX

// include ---------------------------------------------------------------

#include <tools/color.hxx>
#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>

namespace rtl
{
    class OUString;
}

// class SvxShadowItem ---------------------------------------------------

/*  [Beschreibung]

    Dieses Item beschreibt ein Schattenattribut (Farbe, Breite, Lage).
*/

#define SHADOW_TOP      ((sal_uInt16)0)
#define SHADOW_BOTTOM   ((sal_uInt16)1)
#define SHADOW_LEFT     ((sal_uInt16)2)
#define SHADOW_RIGHT    ((sal_uInt16)3)

class EDITENG_DLLPUBLIC SvxShadowItem : public SfxEnumItemInterface
{
    Color               aShadowColor;
    sal_uInt16              nWidth;
    SvxShadowLocation   eLocation;
public:
    POOLITEM_FACTORY()
    SvxShadowItem( const sal_uInt16 nId = 0,
                 const Color *pColor = 0, const sal_uInt16 nWidth = 100 /*5pt*/,
                 const SvxShadowLocation eLoc = SVX_SHADOW_NONE );

    inline SvxShadowItem& operator=( const SvxShadowItem& rFmtShadow );

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
    virtual void             ScaleMetrics( long nMult, long nDiv );
    virtual bool             HasMetrics() const;

    const Color& GetColor() const { return aShadowColor;}
    void SetColor( const Color &rNew ) { aShadowColor = rNew; }

    sal_uInt16 GetWidth() const { return nWidth; }
    SvxShadowLocation GetLocation() const { return eLocation; }

    void SetWidth( sal_uInt16 nNew ) { nWidth = nNew; }
    void SetLocation( SvxShadowLocation eNew ) { eLocation = eNew; }

        //Breite des Schattens auf der jeweiligen Seite berechnen.
    sal_uInt16 CalcShadowSpace( sal_uInt16 nShadow ) const;

    virtual sal_uInt16          GetValueCount() const;
    virtual String          GetValueTextByPos( sal_uInt16 nPos ) const;
    virtual sal_uInt16          GetEnumValue() const;
    virtual void            SetEnumValue( sal_uInt16 nNewVal );
};

inline SvxShadowItem &SvxShadowItem::operator=( const SvxShadowItem& rFmtShadow )
{
    aShadowColor = rFmtShadow.aShadowColor;
    nWidth = rFmtShadow.GetWidth();
    eLocation = rFmtShadow.GetLocation();
    return *this;
}

#endif // #ifndef _SVX_SHADITEM_HXX

