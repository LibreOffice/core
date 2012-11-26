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


#ifndef _DATETIMEITEM_HXX
#define _DATETIMEITEM_HXX

#include <tools/datetime.hxx>
#include <svl/poolitem.hxx>

class SvStream;

DBG_NAMEEX(SfxDateTimeItem)

// class SfxDateTimeItem -------------------------------------------------

class SfxDateTimeItem : public SfxPoolItem
{
private:
    DateTime                aDateTime;

public:
                            SfxDateTimeItem( sal_uInt16 nWhich );
                            SfxDateTimeItem( sal_uInt16 nWhich,
                                             const DateTime& rDT );
                            SfxDateTimeItem( const SfxDateTimeItem& rCpy );

                            ~SfxDateTimeItem() {
                                DBG_DTOR(SfxDateTimeItem, 0); }

    virtual int             operator==( const SfxPoolItem& ) const;
    using SfxPoolItem::Compare;
    virtual int             Compare( const SfxPoolItem &rWith ) const;
    virtual SfxPoolItem*    Create( SvStream&, sal_uInt16 nItemVersion ) const;
    virtual SvStream&       Store( SvStream&, sal_uInt16 nItemVersion ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * pIntlWrapper = 0 )
        const;

    const DateTime&         GetDateTime() const { return aDateTime; }
    void                    SetDateTime( const DateTime& rDT ) {
                                DBG_ASSERT( GetRefCount() == 0,
                                            "SetDateTime() with pooled item" );
                                aDateTime = rDT; }

    virtual sal_Bool PutValue  ( const com::sun::star::uno::Any& rVal,
                             sal_uInt8 nMemberId = 0 );
    virtual sal_Bool QueryValue( com::sun::star::uno::Any& rVal,
                             sal_uInt8 nMemberId = 0 ) const;
};

class SfxColumnDateTimeItem : public SfxDateTimeItem
{
public:
                            SfxColumnDateTimeItem( sal_uInt16 nWhich );
                            SfxColumnDateTimeItem( sal_uInt16 nWhich,
                                             const DateTime& rDT );
                            SfxColumnDateTimeItem( const SfxDateTimeItem& rCpy );

                            ~SfxColumnDateTimeItem() {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * pIntlWrapper = 0 )
        const;
};

#endif

