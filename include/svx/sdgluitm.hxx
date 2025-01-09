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

#ifndef INCLUDED_SVX_SDGLUITM_HXX
#define INCLUDED_SVX_SDGLUITM_HXX

#include <svx/sdprcitm.hxx>
#include <svx/svddef.hxx>
#include <svx/svxdllapi.h>


// SdrGrafLuminanceItem -


class SVXCORE_DLLPUBLIC SdrGrafLuminanceItem final : public SdrSignedPercentItem
{
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrGrafLuminanceItem)
    SdrGrafLuminanceItem( short nLuminancePercent = 0 )
        : SdrSignedPercentItem( SDRATTR_GRAFLUMINANCE, nLuminancePercent ) {}

    virtual SdrGrafLuminanceItem* Clone( SfxItemPool* pPool = nullptr ) const override;
};


// SdrGrafContrastItem -


class SVXCORE_DLLPUBLIC SdrGrafContrastItem final : public SdrSignedPercentItem
{
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrGrafContrastItem)
    SdrGrafContrastItem( short nContrastPercent = 0 )
        : SdrSignedPercentItem( SDRATTR_GRAFCONTRAST, nContrastPercent ) {}

    virtual SdrGrafContrastItem* Clone( SfxItemPool* pPool = nullptr ) const override;
};

#endif // INCLUDED_SVX_SDGLUITM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
