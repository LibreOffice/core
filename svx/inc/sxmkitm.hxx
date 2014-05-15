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
#ifndef INCLUDED_SVX_INC_SXMKITM_HXX
#define INCLUDED_SVX_INC_SXMKITM_HXX

#include <svx/svddef.hxx>
#include <svl/eitem.hxx>

enum SdrMeasureKind     {SDRMEASURE_STD,SDRMEASURE_RADIUS}; // n.i.


// class SdrMeasureKindItem (n.i.)

class SdrMeasureKindItem: public SfxEnumItem {
public:
    TYPEINFO_OVERRIDE();
    SdrMeasureKindItem(SdrMeasureKind eKind=SDRMEASURE_STD): SfxEnumItem(SDRATTR_MEASUREKIND,sal::static_int_cast< sal_uInt16 >(eKind)) {}
    SdrMeasureKindItem(SvStream& rIn)                      : SfxEnumItem(SDRATTR_MEASUREKIND,rIn)    {}
    virtual SfxPoolItem*   Clone(SfxItemPool* pPool=NULL) const SAL_OVERRIDE;
    virtual SfxPoolItem*   Create(SvStream& rIn, sal_uInt16 nVer) const SAL_OVERRIDE;
    virtual sal_uInt16         GetValueCount() const SAL_OVERRIDE; // { return 2; }
            SdrMeasureKind GetValue() const { return (SdrMeasureKind)SfxEnumItem::GetValue(); }

    virtual bool           QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool           PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

    virtual OUString GetValueTextByPos(sal_uInt16 nPos) const SAL_OVERRIDE;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, OUString& rText, const IntlWrapper * = 0) const SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
