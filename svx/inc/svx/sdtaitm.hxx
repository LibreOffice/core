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


#ifndef _SDTAITM_HXX
#define _SDTAITM_HXX

#include <svl/eitem.hxx>
#include <svx/svddef.hxx>
#include "svx/svxdllapi.h"

//------------------------------
// class SdrTextVertAdjustItem
//------------------------------

// Implementiert werden zunaechst nur TOP und CENTER, neustens auch BOTTOM
enum SdrTextVertAdjust {SDRTEXTVERTADJUST_TOP,      // Kopfbuendig (so wie man es normalerweise kennt)
                        SDRTEXTVERTADJUST_CENTER,   // Zentriert (z.B. fuer die Titeltexte im Draw)
                        SDRTEXTVERTADJUST_BOTTOM,   // Fussbuendig
                        SDRTEXTVERTADJUST_BLOCK    // #103516# support vertical full with supported now
                        /*,SDRTEXTVERTADJUST_STRETCH*/}; // Auch die Buchstaben in der Hoehe verzerren (ni)

class SVX_DLLPUBLIC SdrTextVertAdjustItem: public SfxEnumItem {
public:
    SdrTextVertAdjustItem(SdrTextVertAdjust eAdj=SDRTEXTVERTADJUST_TOP): SfxEnumItem(SDRATTR_TEXT_VERTADJUST,(sal_uInt16)eAdj) {}
    SdrTextVertAdjustItem(SvStream& rIn)                               : SfxEnumItem(SDRATTR_TEXT_VERTADJUST,rIn)  {}
    virtual SfxPoolItem*      Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem*      Create(SvStream& rIn, sal_uInt16 nVer) const;
    virtual sal_uInt16            GetValueCount() const; // { return 5; }
            SdrTextVertAdjust GetValue() const      { return (SdrTextVertAdjust)SfxEnumItem::GetValue(); }

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual String  GetValueTextByPos(sal_uInt16 nPos) const;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0) const;
};

//------------------------------
// class SdrTextHorzAdjustItem
//------------------------------

enum SdrTextHorzAdjust {SDRTEXTHORZADJUST_LEFT,     // Linksbuendig verankert
                        SDRTEXTHORZADJUST_CENTER,   // Zentriert (z.B. fuer die Titeltexte im Draw)
                        SDRTEXTHORZADJUST_RIGHT,    // Rechtsbuendig verankert
                        SDRTEXTHORZADJUST_BLOCK    // So wie es frueher war: Gesamte Textrahmenbreite verwenden, Absatzformatierung macht wieder Sinn
                        /*,SDRTEXTHORZADJUST_STRETCH*/}; // FitToSize in X-Richtung (ni).

class SVX_DLLPUBLIC SdrTextHorzAdjustItem: public SfxEnumItem {
public:
    SdrTextHorzAdjustItem(SdrTextHorzAdjust eAdj=SDRTEXTHORZADJUST_BLOCK): SfxEnumItem(SDRATTR_TEXT_HORZADJUST,(sal_uInt16)eAdj) {}
    SdrTextHorzAdjustItem(SvStream& rIn)                                 : SfxEnumItem(SDRATTR_TEXT_HORZADJUST,rIn)  {}
    virtual SfxPoolItem*      Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem*      Create(SvStream& rIn, sal_uInt16 nVer) const;
    virtual sal_uInt16            GetValueCount() const; // { return 5; }
            SdrTextHorzAdjust GetValue() const      { return (SdrTextHorzAdjust)SfxEnumItem::GetValue(); }

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual String  GetValueTextByPos(sal_uInt16 nPos) const;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0) const;
};

#endif
