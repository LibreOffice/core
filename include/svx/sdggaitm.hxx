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

#ifndef INCLUDED_SVX_SDGGAITM_HXX
#define INCLUDED_SVX_SDGGAITM_HXX

#include <svl/intitem.hxx>
#include <svx/svddef.hxx>
#include <svx/svxdllapi.h>


// SdrGrafGamma100Item -


class SVX_DLLPUBLIC SdrGrafGamma100Item : public SfxUInt32Item
{
public:


                            SdrGrafGamma100Item( sal_uIntPtr nGamma100 = 100 ) : SfxUInt32Item( SDRATTR_GRAFGAMMA, nGamma100 ) {}
                            SdrGrafGamma100Item( SvStream& rIn ) : SfxUInt32Item( SDRATTR_GRAFGAMMA, rIn ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const override;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};

#endif // INCLUDED_SVX_SDGGAITM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
