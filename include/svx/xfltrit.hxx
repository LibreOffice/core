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

#ifndef INCLUDED_SVX_XFLTRIT_HXX
#define INCLUDED_SVX_XFLTRIT_HXX

#include <svl/poolitem.hxx>
#include <svx/svxdllapi.h>
#include <boost/property_tree/ptree_fwd.hpp>

/*************************************************************************
|*
|* transparency-Item for contents (Solid)
|*
\************************************************************************/

class SVXCORE_DLLPUBLIC XFillTransparenceItem final : public SfxPoolItem
{
    sal_uInt16 m_nValue;
public:
//    static SfxPoolItem* CreateDefault();

    DECLARE_ITEM_TYPE_FUNCTION(XFillTransparenceItem)
    XFillTransparenceItem(sal_uInt16 nFillTransparence = 0);
    virtual XFillTransparenceItem* Clone(SfxItemPool* pPool = nullptr) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;
    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    virtual boost::property_tree::ptree dumpAsJSON() const override;

    sal_uInt16 GetValue() const { return m_nValue; }

    virtual bool operator ==(const SfxPoolItem & rItem) const override;

    virtual bool supportsHashCode() const override final;
    virtual size_t hashCode() const override final;

    virtual bool QueryValue(css::uno::Any& rVal,
                            sal_uInt8 nMemberId = 0) const override;

    virtual bool PutValue(const css::uno::Any& rVal,
                          sal_uInt8 nMemberId) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
