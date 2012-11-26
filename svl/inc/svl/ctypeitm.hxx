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


#ifndef _SVTOOLS_CTYPEITM_HXX
#define _SVTOOLS_CTYPEITM_HXX

#include <svl/inettype.hxx>
#include <svl/custritm.hxx>

//=========================================================================

class CntContentTypeItem : public CntUnencodedStringItem
{
private:
    INetContentType _eType;
    XubString       _aPresentation;

public:
    CntContentTypeItem();
    CntContentTypeItem( sal_uInt16 nWhich, const XubString& rType );
    CntContentTypeItem( sal_uInt16 nWhich, const INetContentType eType );
    CntContentTypeItem( const CntContentTypeItem& rOrig );

    virtual SfxPoolItem* Create( SvStream& rStream,
                                 sal_uInt16 nItemVersion ) const;
    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const;

    virtual int          operator==( const SfxPoolItem& rOrig ) const;

    virtual sal_uInt16 GetVersion(sal_uInt16) const;

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = NULL ) const;

    void SetValue( const XubString& rNewVal );
    void SetPresentation( const XubString& rNewVal );

    using SfxPoolItem::Compare;
    virtual int Compare( const SfxPoolItem &rWith, const IntlWrapper& rIntlWrapper ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreMetric,
                                                 SfxMapUnit ePresMetric,
                                                 XubString &rText,
                                                 const IntlWrapper* pIntlWrapper = 0 ) const;

    virtual sal_Bool QueryValue( com::sun::star::uno::Any& rVal,
                             sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool PutValue  ( const com::sun::star::uno::Any& rVal,
                             sal_uInt8 nMemberId = 0);

    INetContentType GetEnumValue() const;

    void                  SetValue( const INetContentType eType );
};

#endif /* !_SVTOOLS_CTYPEITM_HXX */

