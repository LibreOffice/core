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

#ifndef INCLUDED_SVL_RNGITEM_HXX
#define INCLUDED_SVL_RNGITEM_HXX

#include <config_options.h>
#include <svl/svldllapi.h>
#include <svl/poolitem.hxx>

class SvStream;


class UNLESS_MERGELIBS_MORE(SVL_DLLPUBLIC) SfxRangeItem final : public SfxPoolItem
{
private:
    sal_uInt16                  nFrom;
    sal_uInt16                  nTo;
public:
                                SfxRangeItem( sal_uInt16 nWID, sal_uInt16 nFrom, sal_uInt16 nTo );
    virtual bool                operator==( const SfxPoolItem& ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& ) const override;
    virtual SfxRangeItem*       Clone( SfxItemPool *pPool = nullptr ) const override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
