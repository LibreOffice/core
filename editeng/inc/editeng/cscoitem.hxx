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


#ifndef _SVX_CSCOITEM_HXX
#define _SVX_CSCOITEM_HXX

// include ---------------------------------------------------------------

#include <tools/string.hxx>
#include <editeng/colritem.hxx>
#include <editeng/editengdllapi.h>

// class SvxCharSetColorItem ---------------------------------------------

/*  [Beschreibung]

    PB: wird nur intern im Reader des Writers benoetigt
*/

class EDITENG_DLLPUBLIC SvxCharSetColorItem : public SvxColorItem
{
    rtl_TextEncoding eFrom;
public:
    SvxCharSetColorItem( const sal_uInt16 nId  );
    SvxCharSetColorItem( const Color& aColor, const rtl_TextEncoding eFrom,
                     const sal_uInt16 nId  );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion) const;

    inline rtl_TextEncoding&    GetCharSet() { return eFrom; }
    inline rtl_TextEncoding     GetCharSet() const { return eFrom; }

    inline SvxCharSetColorItem& operator=(const SvxCharSetColorItem& rColor)
    {
        SetValue( rColor.GetValue() );
        eFrom = rColor.GetCharSet();
        return *this;
    }
};

#endif

