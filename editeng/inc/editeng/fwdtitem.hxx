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


#ifndef _SVX_FWDTITEM_HXX
#define _SVX_FWDTITEM_HXX

// include ---------------------------------------------------------------


#include <svl/poolitem.hxx>


// class SvxFontWidthItem -----------------------------------------------


/*  [Beschreibung]

    Dieses Item beschreibt die Font-Breite.
*/

class SvxFontWidthItem : public SfxPoolItem
{
    sal_uInt16  nWidth;         // 0 = default
    sal_uInt16  nProp;          // default 100%
public:
    SvxFontWidthItem(   const sal_uInt16 nSz /*= 0*/,
                        const sal_uInt16 nPropWidth /*= 100*/,
                        const sal_uInt16 nId  );

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
    virtual void             ScaleMetrics( long nMult, long nDiv );
    virtual bool             HasMetrics() const;

    inline SvxFontWidthItem& operator=(const SvxFontWidthItem& rItem )
        {
            SetWidthValue( rItem.GetWidth() );
            SetProp( rItem.GetProp() );
            return *this;
        }

    void SetWidth( sal_uInt16 nNewWidth, const sal_uInt16 nNewProp = 100 )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nWidth = sal_uInt16(( (sal_uInt32)nNewWidth * nNewProp ) / 100 );
            nProp = nNewProp;
        }

    sal_uInt16 GetWidth() const { return nWidth; }

    void SetWidthValue( sal_uInt16 nNewWidth )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nWidth = nNewWidth;
        }

    void SetProp( const sal_uInt16 nNewProp )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nProp = nNewProp;
        }

    sal_uInt16 GetProp() const { return nProp; }
};





#endif


