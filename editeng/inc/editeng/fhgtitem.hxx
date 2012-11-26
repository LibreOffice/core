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


#ifndef _SVX_FHGTITEM_HXX
#define _SVX_FHGTITEM_HXX

// include ---------------------------------------------------------------

#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxFontHeightItem -----------------------------------------------

// Achtung: Twips-Werte

/*  [Beschreibung]

    Dieses Item beschreibt die Font-Hoehe.
*/

#define FONTHEIGHT_16_VERSION   ((sal_uInt16)0x0001)
#define FONTHEIGHT_UNIT_VERSION ((sal_uInt16)0x0002)

class EDITENG_DLLPUBLIC SvxFontHeightItem : public SfxPoolItem
{
    sal_uInt32  nHeight;
    sal_uInt16  nProp;              // default 100%
    SfxMapUnit ePropUnit;       // Percent, Twip, ...
public:
    POOLITEM_FACTORY()
    SvxFontHeightItem( const sal_uLong nSz = 240, const sal_uInt16 nPropHeight = 100, const sal_uInt16 nId = 0 );

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
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion) const;
    virtual sal_uInt16       GetVersion( sal_uInt16 nItemVersion) const;
    virtual void             ScaleMetrics( long nMult, long nDiv );
    virtual bool             HasMetrics() const;

    inline SvxFontHeightItem& operator=(const SvxFontHeightItem& rSize)
        {
            SetHeightValue( rSize.GetHeight() );
            SetProp( rSize.GetProp(), ePropUnit );
            return *this;
        }

    void SetHeight( sal_uInt32 nNewHeight, const sal_uInt16 nNewProp = 100,
                     SfxMapUnit eUnit = SFX_MAPUNIT_RELATIVE );

    void SetHeight( sal_uInt32 nNewHeight, sal_uInt16 nNewProp,
                     SfxMapUnit eUnit, SfxMapUnit eCoreUnit );

    sal_uInt32 GetHeight() const { return nHeight; }

    void SetHeightValue( sal_uInt32 nNewHeight )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nHeight = nNewHeight;
        }

    void SetProp( const sal_uInt16 nNewProp,
                    SfxMapUnit eUnit = SFX_MAPUNIT_RELATIVE )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nProp = nNewProp;
            ePropUnit = eUnit;
        }

    sal_uInt16 GetProp() const { return nProp; }

    SfxMapUnit GetPropUnit() const { return ePropUnit;  }   // Percent, Twip, ...
};

#endif

