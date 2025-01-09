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

#ifndef INCLUDED_SVX_XFLBMSXY_HXX
#define INCLUDED_SVX_XFLBMSXY_HXX

#include <svl/metitem.hxx>
#include <svx/svxdllapi.h>

class SVXCORE_DLLPUBLIC XFillBmpSizeXItem final : public SfxMetricItem
{
public:
                            DECLARE_ITEM_TYPE_FUNCTION(XFillBmpSizeXItem)
                            XFillBmpSizeXItem( tools::Long nSizeX = 0 );

    SVX_DLLPRIVATE virtual XFillBmpSizeXItem* Clone( SfxItemPool* pPool = nullptr ) const override;

    SVX_DLLPRIVATE virtual bool GetPresentation( SfxItemPresentation ePres,
                                                 MapUnit eCoreMetric,
                                                 MapUnit ePresMetric,
                                                 OUString &rText, const IntlWrapper& ) const override;
    // This item records both metrical and percentage quotation (negative);
    // We have to consider that.
    SVX_DLLPRIVATE virtual bool HasMetrics() const override;
};

class SVXCORE_DLLPUBLIC XFillBmpSizeYItem final : public SfxMetricItem
{
public:
                            DECLARE_ITEM_TYPE_FUNCTION(XFillBmpSizeYItem)
                            XFillBmpSizeYItem( tools::Long nSizeY = 0 );

    SVX_DLLPRIVATE virtual XFillBmpSizeYItem* Clone( SfxItemPool* pPool = nullptr ) const override;

    SVX_DLLPRIVATE virtual bool GetPresentation( SfxItemPresentation ePres,
                                                 MapUnit eCoreMetric,
                                                 MapUnit ePresMetric,
                                                 OUString &rText, const IntlWrapper& ) const override;
    // This item records both metrical and percentage quotation (negative);
    // We have to consider that.
    SVX_DLLPRIVATE virtual bool HasMetrics() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
