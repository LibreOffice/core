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
#ifndef _SXMUITM_HXX
#define _SXMUITM_HXX

#include <vcl/field.hxx>
#include <svx/svddef.hxx>
#include <svl/eitem.hxx>
#include "svx/svxdllapi.h"

// Vorgabe einer Masseinheit. Der Zahlenwert wird in diese Einheit umgerechnet
// (ausgehend von der MapUnit des Models). Diese Einheit wird dann ggf. auch angezeigt.
class SVX_DLLPUBLIC SdrMeasureUnitItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrMeasureUnitItem(FieldUnit eUnit=FUNIT_NONE): SfxEnumItem(SDRATTR_MEASUREUNIT,sal::static_int_cast< sal_uInt16 >(eUnit)) {}
    SdrMeasureUnitItem(SvStream& rIn)             : SfxEnumItem(SDRATTR_MEASUREUNIT,rIn)   {}
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const;
    virtual sal_uInt16       GetValueCount() const; // { return 14; }
            FieldUnit    GetValue() const { return (FieldUnit)SfxEnumItem::GetValue(); }

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual OUString GetValueTextByPos(sal_uInt16 nPos) const;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, OUString& rText, const IntlWrapper * = 0) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
