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


#ifndef _SVX_ESCPITEM_HXX
#define _SVX_ESCPITEM_HXX

// include ---------------------------------------------------------------

#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxEscapementItem -----------------------------------------------

#define DFLT_ESC_SUPER   33     // 1/3
#define DFLT_ESC_SUB    -33     // auch 1/3 fr"uher 8/100
#define DFLT_ESC_PROP    58
#define DFLT_ESC_AUTO_SUPER 101
#define DFLT_ESC_AUTO_SUB  -101

/*  [Beschreibung]

    Dieses Item beschreibt die Schrift-Position.
*/

class EDITENG_DLLPUBLIC SvxEscapementItem : public SfxEnumItemInterface
{
    short nEsc;
    sal_uInt8  nProp;
public:
    POOLITEM_FACTORY()
    SvxEscapementItem( const sal_uInt16 nId = 0 );
    SvxEscapementItem( const SvxEscapement eEscape,
                   const sal_uInt16 nId  );
    SvxEscapementItem( const short nEsc, const sal_uInt8 nProp,
                   const sal_uInt16 nId  );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion) const;

    inline void SetEscapement( const SvxEscapement eNew )
    {
        if( SVX_ESCAPEMENT_OFF == eNew )
            nEsc = 0, nProp = 100;
        else
            if( SVX_ESCAPEMENT_SUPERSCRIPT == eNew )
                nEsc = DFLT_ESC_SUPER, nProp = DFLT_ESC_PROP;
            else
                nEsc = DFLT_ESC_SUB, nProp = DFLT_ESC_PROP;
    }
    inline SvxEscapement GetEscapement() const { return static_cast< SvxEscapement >( GetEnumValue() ); }

    inline short &GetEsc() { return nEsc; }
    inline short GetEsc() const { return nEsc; }

    inline sal_uInt8 &GetProp() { return nProp; }
    inline sal_uInt8 GetProp() const { return nProp; }

    inline SvxEscapementItem& operator=(const SvxEscapementItem& rEsc)
        {
            nEsc  = rEsc.GetEsc();
            nProp = rEsc.GetProp();
            return *this;
        }

    virtual sal_uInt16          GetValueCount() const;
    virtual String          GetValueTextByPos( sal_uInt16 nPos ) const;
    virtual sal_uInt16          GetEnumValue() const;
    virtual void            SetEnumValue( sal_uInt16 nNewVal );
};

#endif

