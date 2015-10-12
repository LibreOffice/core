/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVX_SDTAITM_HXX
#define INCLUDED_SVX_SDTAITM_HXX

#include <svl/eitem.hxx>
#include <svx/svddef.hxx>
#include <svx/svxdllapi.h>


// class SdrTextVertAdjustItem


enum SdrTextVertAdjust {SDRTEXTVERTADJUST_TOP,      // aligned to top (normally used)
                        SDRTEXTVERTADJUST_CENTER,   // centered (for example for the title texts in Draw)
                        SDRTEXTVERTADJUST_BOTTOM,   // aligned to bottom
                        SDRTEXTVERTADJUST_BLOCK    // support vertical full with supported now
                        /*,SDRTEXTVERTADJUST_STRETCH*/}; // also stretch letters in their height (ni)

class SVX_DLLPUBLIC SdrTextVertAdjustItem: public SfxEnumItem {
public:
    TYPEINFO_OVERRIDE();
    SdrTextVertAdjustItem(SdrTextVertAdjust eAdj=SDRTEXTVERTADJUST_TOP): SfxEnumItem(SDRATTR_TEXT_VERTADJUST,(sal_uInt16)eAdj) {}
    SdrTextVertAdjustItem(SdrTextVertAdjust eAdj, sal_uInt16 nWhich): SfxEnumItem(nWhich,(sal_uInt16)eAdj) {}
    SdrTextVertAdjustItem(SvStream& rIn)                               : SfxEnumItem(SDRATTR_TEXT_VERTADJUST,rIn)  {}
    virtual SfxPoolItem*      Clone(SfxItemPool* pPool=NULL) const override;
    virtual SfxPoolItem*      Create(SvStream& rIn, sal_uInt16 nVer) const override;
    virtual sal_uInt16            GetValueCount() const override; // { return 5; }
            SdrTextVertAdjust GetValue() const      { return (SdrTextVertAdjust)SfxEnumItem::GetValue(); }

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual OUString GetValueTextByPos(sal_uInt16 nPos) const override;
    virtual bool GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, OUString& rText, const IntlWrapper * = 0) const override;
};


// class SdrTextHorzAdjustItem


enum SdrTextHorzAdjust {SDRTEXTHORZADJUST_LEFT,     // left adjusted
                        SDRTEXTHORZADJUST_CENTER,   // centered (for example for title texts in Draw)
                        SDRTEXTHORZADJUST_RIGHT,    // right adjusted
                        SDRTEXTHORZADJUST_BLOCK    // use the whole text frame width
                        /*,SDRTEXTHORZADJUST_STRETCH*/}; // FitToSize in X direction (ni).

class SVX_DLLPUBLIC SdrTextHorzAdjustItem: public SfxEnumItem {
public:
    TYPEINFO_OVERRIDE();
    SdrTextHorzAdjustItem(SdrTextHorzAdjust eAdj=SDRTEXTHORZADJUST_BLOCK): SfxEnumItem(SDRATTR_TEXT_HORZADJUST,(sal_uInt16)eAdj) {}
    SdrTextHorzAdjustItem(SvStream& rIn)                                 : SfxEnumItem(SDRATTR_TEXT_HORZADJUST,rIn)  {}
    virtual SfxPoolItem*      Clone(SfxItemPool* pPool=NULL) const override;
    virtual SfxPoolItem*      Create(SvStream& rIn, sal_uInt16 nVer) const override;
    virtual sal_uInt16        GetValueCount() const override;
            SdrTextHorzAdjust GetValue() const      { return (SdrTextHorzAdjust)SfxEnumItem::GetValue(); }

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual OUString GetValueTextByPos(sal_uInt16 nPos) const override;
    virtual bool GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, OUString& rText, const IntlWrapper * = 0) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
