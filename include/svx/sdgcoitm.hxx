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

#ifndef INCLUDED_SVX_SDGCOITM_HXX
#define INCLUDED_SVX_SDGCOITM_HXX

#include <svx/sdprcitm.hxx>
#include <svx/svddef.hxx>
#include <svx/svxdllapi.h>


// SdrGrafRedItem -


class SVX_DLLPUBLIC SdrGrafRedItem : public SdrSignedPercentItem
{
public:

                            TYPEINFO_OVERRIDE();

                            SdrGrafRedItem( short nRedPercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFRED, nRedPercent ) {}
                            SdrGrafRedItem( SvStream& rIn ) : SdrSignedPercentItem( SDRATTR_GRAFRED, rIn ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const override;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const override;
};


// SdrGrafGreenItem -


class SVX_DLLPUBLIC SdrGrafGreenItem : public SdrSignedPercentItem
{
public:

                            TYPEINFO_OVERRIDE();

                            SdrGrafGreenItem( short nGreenPercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFGREEN, nGreenPercent ) {}
                            SdrGrafGreenItem( SvStream& rIn ) : SdrSignedPercentItem( SDRATTR_GRAFGREEN, rIn ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const override;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const override;
};


// SdrGrafBlueItem -


class SVX_DLLPUBLIC SdrGrafBlueItem : public SdrSignedPercentItem
{
public:

                            TYPEINFO_OVERRIDE();

                            SdrGrafBlueItem( short nBluePercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFBLUE, nBluePercent ) {}
                            SdrGrafBlueItem( SvStream& rIn ) : SdrSignedPercentItem( SDRATTR_GRAFBLUE, rIn ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const override;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const override;
};

#endif // INCLUDED_SVX_SDGCOITM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
