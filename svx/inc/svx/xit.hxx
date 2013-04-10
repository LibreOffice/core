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



#ifndef _SVX_XIT_HXX
#define _SVX_XIT_HXX

#include <svl/stritem.hxx>
#include <svx/xdef.hxx>
#include "svx/svxdllapi.h"

/************************************************************************/

class XColorList;
class XDashList;
class XLineEndList;
class XHatchList;
class XBitmapList;
class XGradientList;
class SfxItemPool;
class NameOrIndex;
class XPropertyList;

typedef sal_Bool (*SvxCompareValueFunc)( const NameOrIndex* p1, const NameOrIndex* p2 );

//-------------------
// class NameOrIndex
//-------------------
class SVX_DLLPUBLIC NameOrIndex : public SfxStringItem
{
    sal_Int32    nPalIndex;

protected:
    void    Detach()    { nPalIndex = -1; }

public:
            TYPEINFO();
            NameOrIndex() { nPalIndex = -1; }
            NameOrIndex(sal_uInt16 nWhich, sal_Int32 nIndex);
            NameOrIndex(sal_uInt16 nWhich,
                        const String& rName= String());
            NameOrIndex(sal_uInt16 nWhich, SvStream& rIn);
            NameOrIndex(const NameOrIndex& rNameOrIndex);
           ~NameOrIndex() {};

    virtual int          operator==(const SfxPoolItem& rItem) const;
    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const;
    virtual SvStream&    Store(SvStream& rOut, sal_uInt16 nItemVersion ) const;

            String       GetName() const              { return GetValue();   }
            void         SetName(const String& rName) { SetValue(rName);     }
            sal_Int32        GetIndex() const             { return nPalIndex;    }
            void         SetIndex(sal_Int32 nIndex)        { nPalIndex = nIndex;  }
            sal_Bool         IsIndex() const          { return (nPalIndex >= 0); }

    /** this static checks if the given NameOrIndex item has a unique name for its value.
        The returned String is a unique name for an item with this value in both given pools.
        Argument pPool2 can be null.
        If returned string equals NameOrIndex->GetName(), the name was already unique.
    */
    static String CheckNamedItem( const NameOrIndex* pCheckItem, const sal_uInt16 nWhich, const SfxItemPool* pPool1, const SfxItemPool* pPool2, SvxCompareValueFunc pCompareValueFunc, sal_uInt16 nPrefixResId, XPropertyList* pDefaults = NULL );
};

#endif
