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


#ifndef _SVX_UDLNITEM_HXX
#define _SVX_UDLNITEM_HXX

// include ---------------------------------------------------------------

#include <svl/eitem.hxx>
#include <vcl/vclenum.hxx>
#include <tools/color.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxTextLineItem ------------------------------------------------

/* Value container for underline and overline font effects */

class EDITENG_DLLPUBLIC SvxTextLineItem : public SfxEnumItem
{
    Color mColor;
public:
    POOLITEM_FACTORY()
    SvxTextLineItem( const FontUnderline eSt = UNDERLINE_NONE, const sal_uInt16 nId = 0 );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                            SfxMapUnit eCoreMetric,
                            SfxMapUnit ePresMetric,
                            String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion) const;
    virtual String          GetValueTextByPos( sal_uInt16 nPos ) const;
    virtual sal_uInt16          GetValueCount() const;

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 );

    // MS VC4.0 kommt durcheinander
    void                    SetValue( sal_uInt16 nNewVal )
                                {SfxEnumItem::SetValue(nNewVal); }
    virtual int             HasBoolValue() const;
    virtual sal_Bool            GetBoolValue() const;
    virtual void            SetBoolValue( sal_Bool bVal );

    virtual int             operator==( const SfxPoolItem& ) const;

    inline SvxTextLineItem& operator=(const SvxTextLineItem& rTextLine)
        {
            SetValue( rTextLine.GetValue() );
            SetColor( rTextLine.GetColor() );
            return *this;
        }

    // enum cast
    FontUnderline           GetLineStyle() const
                                { return (FontUnderline)GetValue(); }
    void                    SetLineStyle( FontUnderline eNew )
                                { SetValue((sal_uInt16) eNew); }

    const Color&            GetColor() const                { return mColor; }
    void                    SetColor( const Color& rCol )   { mColor = rCol; }
};

// class SvxUnderlineItem ------------------------------------------------

/* Value container for underline font effects */

class EDITENG_DLLPUBLIC SvxUnderlineItem : public SvxTextLineItem
{
public:
    SvxUnderlineItem( const FontUnderline eSt,
                      const sal_uInt16 nId );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual String          GetValueTextByPos( sal_uInt16 nPos ) const;
};

// class SvxOverlineItem ------------------------------------------------

/* Value container for overline font effects */

class EDITENG_DLLPUBLIC SvxOverlineItem : public SvxTextLineItem
{
public:
    SvxOverlineItem( const FontUnderline eSt,
                     const sal_uInt16 nId );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual String          GetValueTextByPos( sal_uInt16 nPos ) const;
};

#endif // #ifndef _SVX_UDLNITEM_HXX
