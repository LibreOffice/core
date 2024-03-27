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
#ifndef INCLUDED_SVX_SXMUITM_HXX
#define INCLUDED_SVX_SXMUITM_HXX

#include <svx/svddef.hxx>
#include <svl/eitem.hxx>
#include <svx/svxdllapi.h>
#include <tools/fldunit.hxx>

// specification of the unit if measurement. The numerical quantity value is converted in this unity.
// (based on the facts of the MapUnit of the model). This unity is displayed if necessary.
class SVXCORE_DLLPUBLIC SdrMeasureUnitItem final : public SfxEnumItem<FieldUnit> {
public:
    SdrMeasureUnitItem(FieldUnit eUnit=FieldUnit::NONE): SfxEnumItem(SDRATTR_MEASUREUNIT, eUnit) {}
    SAL_DLLPRIVATE virtual SdrMeasureUnitItem* Clone(SfxItemPool* pPool=nullptr) const override;
    SAL_DLLPRIVATE virtual sal_uInt16   GetValueCount() const override; // { return 14; }

    SAL_DLLPRIVATE virtual bool         QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    SAL_DLLPRIVATE virtual bool         PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    SAL_DLLPRIVATE static OUString GetValueTextByPos(sal_uInt16 nPos);
    SAL_DLLPRIVATE virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric, MapUnit ePresMetric, OUString& rText, const IntlWrapper&) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
