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


#ifndef _AEITEM_HXX
#define _AEITEM_HXX

#include "svl/svldllapi.h"
#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>

class SfxAllEnumValueArr;
class SvUShorts;

class SVL_DLLPUBLIC SfxAllEnumItem: public SfxEnumItem
{
    SfxAllEnumValueArr*     pValues;
    SvUShorts*              pDisabledValues;

protected:
    sal_uInt16                  _GetPosByValue( sal_uInt16 nValue ) const;

public:
    POOLITEM_FACTORY()
                            SfxAllEnumItem();
                            SfxAllEnumItem( sal_uInt16 nWhich);
                            SfxAllEnumItem( sal_uInt16 nWhich, sal_uInt16 nVal );
                            SfxAllEnumItem( sal_uInt16 nWhich, sal_uInt16 nVal, const XubString &rText );
                            SfxAllEnumItem( sal_uInt16 nWhich, SvStream &rStream );
                            SfxAllEnumItem( const SfxAllEnumItem & );
                            ~SfxAllEnumItem();

    void                    InsertValue( sal_uInt16 nValue );
    void                    InsertValue( sal_uInt16 nValue, const XubString &rText );
    void                    RemoveValue( sal_uInt16 nValue );
    void                    RemoveAllValues();

    sal_uInt16                  GetPosByValue( sal_uInt16 nValue ) const;

    virtual sal_uInt16          GetValueCount() const;
    virtual sal_uInt16          GetValueByPos( sal_uInt16 nPos ) const;
    virtual XubString       GetValueTextByPos( sal_uInt16 nPos ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16 nVersion) const;
    virtual sal_Bool            IsEnabled( sal_uInt16 ) const;
    void                    DisableValue( sal_uInt16 );
};

#endif
