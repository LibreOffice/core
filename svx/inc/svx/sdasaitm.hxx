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



#ifndef _SDASAITM_HXX
#define _SDASAITM_HXX

#include <svl/poolitem.hxx>
#include <tools/list.hxx>

class SdrCustomShapeAdjustmentValue
{
    sal_uInt32  nValue;

    friend class SdrCustomShapeAdjustmentItem;

    public :

        void        SetValue( sal_Int32 nVal ) { nValue = nVal; };
        sal_Int32   GetValue() const { return nValue; };

};

class SdrCustomShapeAdjustmentItem : public SfxPoolItem
{
            List    aAdjustmentValueList;

    public :

            SVX_DLLPUBLIC SdrCustomShapeAdjustmentItem();
            SdrCustomShapeAdjustmentItem( SvStream& rIn, sal_uInt16 nVersion );
            SVX_DLLPUBLIC ~SdrCustomShapeAdjustmentItem();

            virtual int                 operator==( const SfxPoolItem& ) const;
            virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePresentation,
                                            SfxMapUnit eCoreMetric, SfxMapUnit ePresentationMetric,
                                                String &rText, const IntlWrapper * = 0) const;
            virtual SfxPoolItem*        Create( SvStream&, sal_uInt16 nItem ) const;
            virtual SvStream&           Store( SvStream&, sal_uInt16 nVersion ) const;
            virtual SfxPoolItem*        Clone( SfxItemPool* pPool = NULL ) const;
            virtual sal_uInt16          GetVersion( sal_uInt16 nFileFormatVersion ) const;

            virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
            virtual sal_Bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );


#ifdef SDR_ISPOOLABLE
            virtual int IsPoolable() const;
#endif

            sal_uInt32                          GetCount() const { return aAdjustmentValueList.Count(); };
            SVX_DLLPUBLIC const SdrCustomShapeAdjustmentValue&  GetValue( sal_uInt32 nIndex ) const;
            SVX_DLLPUBLIC void                              SetValue( sal_uInt32 nIndex,
                                                        const SdrCustomShapeAdjustmentValue& rVal );
};

#endif

