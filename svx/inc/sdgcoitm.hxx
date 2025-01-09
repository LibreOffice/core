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

#pragma once

#include <config_options.h>
#include <svx/sdprcitm.hxx>
#include <svx/svddef.hxx>
#include <svx/svxdllapi.h>


// SdrGrafRedItem -


class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) SdrGrafRedItem final : public SdrSignedPercentItem
{
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrGrafRedItem)
    SdrGrafRedItem( short nRedPercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFRED, nRedPercent ) {}

    virtual SdrGrafRedItem* Clone( SfxItemPool* pPool = nullptr ) const override;
};


// SdrGrafGreenItem -


class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) SdrGrafGreenItem final : public SdrSignedPercentItem
{
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrGrafGreenItem)
    SdrGrafGreenItem( short nGreenPercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFGREEN, nGreenPercent ) {}

    virtual SdrGrafGreenItem* Clone( SfxItemPool* pPool = nullptr ) const override;
};


// SdrGrafBlueItem -


class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) SdrGrafBlueItem final : public SdrSignedPercentItem
{
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrGrafBlueItem)
    SdrGrafBlueItem( short nBluePercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFBLUE, nBluePercent ) {}

    virtual SdrGrafBlueItem* Clone( SfxItemPool* pPool = nullptr ) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
