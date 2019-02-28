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

class SVX_DLLPUBLIC SdrTextVertAdjustItem: public SfxEnumItem<SdrTextVertAdjust> {
public:
    SdrTextVertAdjustItem(SdrTextVertAdjust eAdj=SDRTEXTVERTADJUST_TOP): SfxEnumItem(SDRATTR_TEXT_VERTADJUST, eAdj) {}
    SdrTextVertAdjustItem(SdrTextVertAdjust eAdj, sal_uInt16 nWhich): SfxEnumItem(nWhich, eAdj) {}
    virtual SfxPoolItem*      Clone(SfxItemPool* pPool=nullptr) const override;
    virtual sal_uInt16        GetValueCount() const override; // { return 5; }

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    static OUString GetValueTextByPos(sal_uInt16 nPos);
    virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric, MapUnit ePresMetric, OUString& rText, const IntlWrapper&) const override;
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};


// class SdrTextHorzAdjustItem


enum SdrTextHorzAdjust {SDRTEXTHORZADJUST_LEFT,     // left adjusted
                        SDRTEXTHORZADJUST_CENTER,   // centered (for example for title texts in Draw)
                        SDRTEXTHORZADJUST_RIGHT,    // right adjusted
                        SDRTEXTHORZADJUST_BLOCK    // use the whole text frame width
                        /*,SDRTEXTHORZADJUST_STRETCH*/}; // FitToSize in X direction (ni).

class SVX_DLLPUBLIC SdrTextHorzAdjustItem: public SfxEnumItem<SdrTextHorzAdjust> {
public:
    SdrTextHorzAdjustItem(SdrTextHorzAdjust eAdj=SDRTEXTHORZADJUST_BLOCK): SfxEnumItem(SDRATTR_TEXT_HORZADJUST, eAdj) {}
    virtual SfxPoolItem*      Clone(SfxItemPool* pPool=nullptr) const override;
    virtual sal_uInt16        GetValueCount() const override;

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    static OUString GetValueTextByPos(sal_uInt16 nPos);
    virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric, MapUnit ePresMetric, OUString& rText, const IntlWrapper&) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
