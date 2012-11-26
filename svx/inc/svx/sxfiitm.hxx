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


#ifndef _SFXIITM_HXX
#define _SFXIITM_HXX

#include <svl/poolitem.hxx>
#include <tools/fract.hxx>

/*************************************************************************/
/* FractionItem                                                          */
/*************************************************************************/

class SdrFractionItem: public SfxPoolItem {
    Fraction nValue;
public:
    SdrFractionItem(sal_uInt16 nId=0): SfxPoolItem(nId) {}
    SdrFractionItem(sal_uInt16 nId, const Fraction& rVal): SfxPoolItem(nId), nValue(rVal) {}
    SdrFractionItem(sal_uInt16 nId, SvStream& rIn);
    virtual int              operator==(const SfxPoolItem&) const;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePresentation, SfxMapUnit eCoreMetric, SfxMapUnit ePresentationMetric, String &rText, const IntlWrapper * = 0) const;
    virtual SfxPoolItem*     Create(SvStream&, sal_uInt16 nVer) const;
    virtual SvStream&        Store(SvStream&, sal_uInt16 nItemVers) const;
    virtual SfxPoolItem*     Clone(SfxItemPool *pPool=NULL) const;
            const Fraction&  GetValue() const { return nValue; }
            void             SetValue(const Fraction& rVal) { nValue = rVal; }
#ifdef SDR_ISPOOLABLE
    virtual int IsPoolable() const;
#endif
};


#endif
