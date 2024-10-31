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

#include <svl/voiditem.hxx>
#include <libxml/xmlwriter.h>
#include <sal/log.hxx>

SfxPoolItem* SfxVoidItem::CreateDefault() { return new SfxVoidItem(0); }

SfxVoidItem::SfxVoidItem(sal_uInt16 which)
    : SfxPoolItem(which)
{
#ifdef DBG_UTIL
    if (0 == which)
        SAL_INFO("svl.items", "ITEM: SfxVoidItem with 0 == WhichID gets constructed (!)");
#endif
}

SfxVoidItem::SfxVoidItem(const SfxVoidItem& rCopy)
    : SfxPoolItem(rCopy.Which())
{
}

SfxVoidItem::SfxVoidItem(SfxVoidItem&& rOrig)
    : SfxPoolItem(rOrig)
{
}

bool SfxVoidItem::operator==(const SfxPoolItem& rCmp) const
{
    assert(SfxPoolItem::operator==(rCmp));
    (void)rCmp;
    return true;
}

bool SfxVoidItem::GetPresentation(SfxItemPresentation /*ePresentation*/, MapUnit /*eCoreMetric*/,
                                  MapUnit /*ePresentationMetric*/, OUString& rText,
                                  const IntlWrapper&) const
{
    rText = "Void";
    return true;
}

void SfxVoidItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SfxVoidItem"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"),
                                      BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

SfxVoidItem* SfxVoidItem::Clone(SfxItemPool*) const { return new SfxVoidItem(*this); }

SfxVoidItem::~SfxVoidItem() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
