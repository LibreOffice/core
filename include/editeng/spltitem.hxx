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
#ifndef INCLUDED_EDITENG_SPLTITEM_HXX
#define INCLUDED_EDITENG_SPLTITEM_HXX

#include <svl/eitem.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;

// class SvxFormatSplitItem -------------------------------------------------

/*  [Description]

    This item describes a logical variable "split paragraph yes or no".
*/

class EDITENG_DLLPUBLIC SvxFormatSplitItem : public SfxBoolItem
{
public:
    static SfxPoolItem* CreateDefault();
    virtual ~SvxFormatSplitItem() override;

    inline SvxFormatSplitItem( const bool bSplit /*= true*/,
                            const sal_uInt16 nWh  );

    SvxFormatSplitItem(SvxFormatSplitItem const &) = default;
    SvxFormatSplitItem(SvxFormatSplitItem &&) = default;
    SvxFormatSplitItem & operator =(SvxFormatSplitItem const &) = default;
    SvxFormatSplitItem & operator =(SvxFormatSplitItem &&) = default;

    // "pure virtual Methods" from SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;
};

inline SvxFormatSplitItem::SvxFormatSplitItem( const bool bSplit, const sal_uInt16 nWh ) :
    SfxBoolItem( nWh, bSplit )
{}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
