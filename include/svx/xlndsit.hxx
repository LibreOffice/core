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

#ifndef INCLUDED_SVX_XLNDSIT_HXX
#define INCLUDED_SVX_XLNDSIT_HXX

#include <svx/xit.hxx>

#include <svx/xdash.hxx>
#include <svx/svxdllapi.h>

class SdrModel;


// class XLineDashItem

class SVX_DLLPUBLIC XLineDashItem : public NameOrIndex
{
    XDash   aDash;

public:
                            TYPEINFO_OVERRIDE();
                            XLineDashItem() : NameOrIndex(XATTR_LINEDASH, -1) {}
                            XLineDashItem(const OUString& rName, const XDash& rTheDash);
                            XLineDashItem(SfxItemPool* pPool, const XDash& rTheDash);
                            XLineDashItem(const XLineDashItem& rItem);
                            XLineDashItem(SvStream& rIn);

    virtual bool            operator==(const SfxPoolItem& rItem) const override;
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const override;
    virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const override;
    virtual SvStream&       Store(SvStream& rOut, sal_uInt16 nItemVersion ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const override;
    virtual bool            HasMetrics() const override;
    virtual bool            ScaleMetrics(long nMul, long nDiv) override;

    const XDash&            GetDashValue() const { return aDash;}
    void                    SetDashValue(const XDash& rNew)   { aDash = rNew; Detach(); } // SetValue -> SetDashValue

    static bool CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 );
    XLineDashItem* checkForUniqueItem( SdrModel* pModel ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
