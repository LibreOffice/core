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

#include <formatlinebreak.hxx>

#include <libxml/xmlwriter.h>

#include <vcl/svapp.hxx>

#include <hintids.hxx>
#include <hints.hxx>
#include <pam.hxx>
#include <textlinebreak.hxx>

using namespace com::sun::star;

SwFormatLineBreak::SwFormatLineBreak(SwLineBreakClear eClear)
    : SfxEnumItem(RES_TXTATR_LINEBREAK, eClear)
    , sw::BroadcastingModify()
    , m_pTextAttr(nullptr)
{
}

SwFormatLineBreak::~SwFormatLineBreak() {}

bool SwFormatLineBreak::operator==(const SfxPoolItem& rAttr) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return GetValue() == static_cast<const SwFormatLineBreak&>(rAttr).GetValue();
}

SwFormatLineBreak* SwFormatLineBreak::Clone(SfxItemPool*) const
{
    return new SwFormatLineBreak(GetValue());
}

void SwFormatLineBreak::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    if (rHint.GetId() != SfxHintId::SwLegacyModify)
        return;
    auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
    CallSwClientNotify(rHint);
    if (RES_REMOVE_UNO_OBJECT == pLegacy->GetWhich())
    {
        SetXLineBreak(css::uno::Reference<css::text::XTextContent>(nullptr));
    }
}

sal_uInt16 SwFormatLineBreak::GetValueCount() const
{
    return static_cast<sal_uInt16>(SwLineBreakClear::LAST) + 1;
}

void SwFormatLineBreak::InvalidateLineBreak()
{
    SwPtrMsgPoolItem const aItem(RES_REMOVE_UNO_OBJECT,
                                 &static_cast<sw::BroadcastingModify&>(*this));
    CallSwClientNotify(sw::LegacyModifyHint(&aItem, &aItem));
}

uno::Reference<text::XTextRange> SwFormatLineBreak::getAnchor(SwDoc& /*rDoc*/) const
{
    SolarMutexGuard aGuard;

    SAL_WARN("sw.core", "SwFormatLineBreak::getAnchor: not implemented");
    if (!m_pTextAttr)
    {
        return uno::Reference<text::XTextRange>();
    }

    return {};
}

void SwFormatLineBreak::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatLineBreak"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                      BAD_CAST(OString::number(GetEnumValue()).getStr()));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("m_pTextAttr"), "%p", m_pTextAttr);

    SfxPoolItem::dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

SwTextLineBreak::SwTextLineBreak(SwFormatLineBreak& rAttr, sal_Int32 nStartPos)
    : SwTextAttr(rAttr, nStartPos)
{
    rAttr.SetTextLineBreak(this);
    SetHasDummyChar(true);
}

SwTextLineBreak::~SwTextLineBreak() {}

void SwTextLineBreak::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextLineBreak"));
    SwTextAttr::dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
