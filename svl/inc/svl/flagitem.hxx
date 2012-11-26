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


#ifndef _SFXFLAGITEM_HXX
#define _SFXFLAGITEM_HXX

#include "svl/svldllapi.h"
#include <tools/solar.h>
#include <svl/poolitem.hxx>

class SvStream;

extern sal_uInt16 nSfxFlagVal[16];

// -----------------------------------------------------------------------

DBG_NAMEEX_VISIBILITY(SfxFlagItem, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC SfxFlagItem: public SfxPoolItem
{
    sal_uInt16                   nVal;

public:
                             SfxFlagItem( sal_uInt16 nWhich = 0, sal_uInt16 nValue = 0 );
                             SfxFlagItem( sal_uInt16 nWhich, SvStream & );
                             SfxFlagItem( const SfxFlagItem& );

                             ~SfxFlagItem() {
                                DBG_DTOR(SfxFlagItem, 0); }

    virtual sal_uInt8            GetFlagCount() const;
    virtual XubString        GetFlagText( sal_uInt8 nFlag ) const;

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16 nVersion) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * = 0 ) const;
            sal_uInt16           GetValue() const { return nVal; }
            void             SetValue( sal_uInt16 nNewVal ) {
                                 DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
                                 nVal = nNewVal;
                             }
            int              GetFlag( sal_uInt8 nFlag ) const {
                                 return ( (nVal & nSfxFlagVal[nFlag]) != 0 ); }
            void             SetFlag( sal_uInt8 nFlag, int bVal );
};

#endif
