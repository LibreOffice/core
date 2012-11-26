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


#ifndef _SDTFCHIM_HXX
#define _SDTFCHIM_HXX

#include <svx/svddef.hxx>

class SVX_DLLPUBLIC SdrTextFixedCellHeightItem : public SfxBoolItem
{
public:

    SdrTextFixedCellHeightItem( sal_Bool bUseFixedCellHeight = sal_False );
    SVX_DLLPRIVATE SdrTextFixedCellHeightItem( SvStream & rStream, sal_uInt16 nVersion );

    SVX_DLLPRIVATE virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePresentation,
                                    SfxMapUnit eCoreMetric, SfxMapUnit ePresentationMetric,
                                        String &rText, const IntlWrapper * = 0) const;

    SVX_DLLPRIVATE virtual SfxPoolItem*     Create( SvStream&, sal_uInt16 nItem ) const;
    SVX_DLLPRIVATE virtual SvStream&            Store( SvStream&, sal_uInt16 nVersion ) const;
    SVX_DLLPRIVATE virtual SfxPoolItem*     Clone( SfxItemPool* pPool = NULL ) const;
    SVX_DLLPRIVATE virtual  sal_uInt16          GetVersion( sal_uInt16 nFileFormatVersion ) const;

    SVX_DLLPRIVATE virtual  sal_Bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    SVX_DLLPRIVATE virtual  sal_Bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

#ifdef SDR_ISPOOLABLE
    SVX_DLLPRIVATE virtual int IsPoolable() const;
#endif
};

#endif
