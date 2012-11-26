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


#ifndef _SVX_OPAQITEM_HXX
#define _SVX_OPAQITEM_HXX

// include ---------------------------------------------------------------

#include <svl/eitem.hxx>
#include "editeng/editengdllapi.h"

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxOpaqueItem ---------------------------------------------------


/*
[Beschreibung]
Dieses Item beschreibt eine logische Variable "Undurchsichtig ja oder nein".
*/

class EDITENG_DLLPUBLIC SvxOpaqueItem : public SfxBoolItem
{
public:
    SvxOpaqueItem( const sal_uInt16 nId , const sal_Bool bOpa = sal_True );
    inline SvxOpaqueItem &operator=( const SvxOpaqueItem &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;
};

inline SvxOpaqueItem::SvxOpaqueItem( const sal_uInt16 nId, const sal_Bool bOpa )
    : SfxBoolItem( nId, bOpa )
{}

inline SvxOpaqueItem &SvxOpaqueItem::operator=( const SvxOpaqueItem &rCpy )
{
    SetValue( rCpy.GetValue() );
    return *this;
}



#endif

