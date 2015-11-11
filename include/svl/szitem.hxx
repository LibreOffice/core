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
#ifndef INCLUDED_SVL_SZITEM_HXX
#define INCLUDED_SVL_SZITEM_HXX

#include <tools/gen.hxx>

#include <svl/poolitem.hxx>

class SvStream;

class SfxSizeItem : public SfxPoolItem
{
private:
    Size                     aVal;

public:
                             SfxSizeItem();
                             SfxSizeItem( sal_uInt16 nWhich, const Size& rVal );
                             SfxSizeItem( const SfxSizeItem& );
                             virtual ~SfxSizeItem() {}

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper * = nullptr ) const override;

    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal,
                                          sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal,
                                          sal_uInt8 nMemberId ) override;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16 nItemVersion) const override;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
