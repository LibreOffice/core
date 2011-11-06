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


#ifndef _SW_HF_EAT_SPACINGITEM_HXX
#define _SW_HF_EAT_SPACINGITEM_HXX


#include <svl/eitem.hxx>
#include <hintids.hxx>
#include <format.hxx>

class IntlWrapper;

class SW_DLLPUBLIC SwHeaderAndFooterEatSpacingItem : public SfxBoolItem
{
public:
    SwHeaderAndFooterEatSpacingItem( sal_uInt16 nId = RES_HEADER_FOOTER_EAT_SPACING,
                                     sal_Bool bPrt = sal_False ) : SfxBoolItem( nId, bPrt ) {}

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
};

inline const SwHeaderAndFooterEatSpacingItem &SwAttrSet::GetHeaderAndFooterEatSpacing(sal_Bool bInP) const
    { return (const SwHeaderAndFooterEatSpacingItem&)Get( RES_HEADER_FOOTER_EAT_SPACING,bInP); }

inline const SwHeaderAndFooterEatSpacingItem &SwFmt::GetHeaderAndFooterEatSpacing(sal_Bool bInP) const
    { return aSet.GetHeaderAndFooterEatSpacing(bInP); }

#endif

