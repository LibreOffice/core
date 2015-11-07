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

#ifndef INCLUDED_SVX_XLINEIT0_HXX
#define INCLUDED_SVX_XLINEIT0_HXX

#include <svx/xenum.hxx>
#include <svl/eitem.hxx>

// class LineStyleItem

#include <svx/svxdllapi.h>
#include <com/sun/star/drawing/LineStyle.hpp>

class SVX_DLLPUBLIC XLineStyleItem : public SfxEnumItem
{
public:
                            TYPEINFO_OVERRIDE();
                            static SfxPoolItem* CreateDefault();
                            XLineStyleItem(css::drawing::LineStyle = css::drawing::LineStyle_SOLID);
                            XLineStyleItem(SvStream& rIn);
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const override;
    virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const override;
    virtual sal_uInt16          GetValueCount() const override;
    css::drawing::LineStyle     GetValue() const { return (css::drawing::LineStyle) SfxEnumItem::GetValue(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
