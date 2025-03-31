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

#include <svx/xdef.hxx>
#include <svx/xit.hxx>

#include <svx/xdash.hxx>
#include <svx/svxdllapi.h>

class SdrModel;



class SVXCORE_DLLPUBLIC XLineDashItem final : public NameOrIndex
{
    XDash   m_aDash;

public:
                            static SfxPoolItem* CreateDefault();
                            DECLARE_ITEM_TYPE_FUNCTION(XLineDashItem)
                            XLineDashItem() : NameOrIndex(XATTR_LINEDASH, -1) {}
                            XLineDashItem(const OUString& rName, const XDash& rTheDash);
                            XLineDashItem(const XDash& rTheDash);
                            XLineDashItem(const XLineDashItem& rItem);

    virtual bool            operator==(const SfxPoolItem& rItem) const override;
    virtual XLineDashItem*  Clone(SfxItemPool* pPool = nullptr) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;
    virtual bool            HasMetrics() const override;
    virtual void            ScaleMetrics(tools::Long nMul, tools::Long nDiv) override;

    const XDash&            GetDashValue() const { return m_aDash;}
    void                    SetDashValue(const XDash& rNew)   { m_aDash = rNew; Detach(); } // SetValue -> SetDashValue

    static bool CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 );
    std::unique_ptr<XLineDashItem> checkForUniqueItem( SdrModel& rModel ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
