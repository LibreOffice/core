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
#ifndef INCLUDED_SVX_SDPRCITM_HXX
#define INCLUDED_SVX_SDPRCITM_HXX

#include <config_options.h>
#include <svl/intitem.hxx>
#include <svx/svxdllapi.h>


// class SdrPercentItem
// Integer percents of 0


class SVXCORE_DLLPUBLIC SdrPercentItem : public SfxUInt16Item
{
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrPercentItem)
    SdrPercentItem(TypedWhichId<SdrPercentItem> nId, sal_uInt16 nVal)
        : SfxUInt16Item(nId, nVal) {}
    virtual SdrPercentItem* Clone(SfxItemPool* pPool=nullptr) const override;

    virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric, MapUnit ePresMetric, OUString& rText, const IntlWrapper&) const override;

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};


// class SdrSignedPercentItem
// Integer percents of +/-


class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) SdrSignedPercentItem : public SfxInt16Item
{
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrSignedPercentItem)
    SdrSignedPercentItem( sal_uInt16 nId, sal_Int16 nVal )
        : SfxInt16Item( nId, nVal ) {}
    virtual SdrSignedPercentItem* Clone( SfxItemPool* pPool = nullptr ) const override;

    virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric, MapUnit ePresMetric, OUString& rText, const IntlWrapper&) const override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
