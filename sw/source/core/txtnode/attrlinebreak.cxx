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
#include <pam.hxx>
#include <textlinebreak.hxx>
#include <ndtxt.hxx>
#include <unotextrange.hxx>
#include <unolinebreak.hxx>

using namespace com::sun::star;

SwFormatLineBreak::SwFormatLineBreak(SwLineBreakClear eClear)
    : SfxEnumItem(RES_TXTATR_LINEBREAK, eClear)
    , sw::BroadcastingModify()
    , m_pTextAttr(nullptr)
{
    setNonShareable();
}

SwFormatLineBreak::~SwFormatLineBreak() {}

void SwFormatLineBreak::SetXLineBreak(rtl::Reference<SwXLineBreak> const& xLineBreak)
{
    m_wXLineBreak = xLineBreak.get();
}

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
    if (SfxHintId::SwRemoveUnoObject == rHint.GetId())
    {
        CallSwClientNotify(rHint);
        SetXLineBreak(nullptr);
    }
    else if (rHint.GetId() == SfxHintId::SwLegacyModify
             || rHint.GetId() == SfxHintId::SwFormatChange
             || rHint.GetId() == SfxHintId::SwAttrSetChange)
        CallSwClientNotify(rHint);
}

sal_uInt16 SwFormatLineBreak::GetValueCount() const
{
    return static_cast<sal_uInt16>(SwLineBreakClear::LAST) + 1;
}

rtl::Reference<SwXTextRange> SwFormatLineBreak::GetAnchor() const
{
    SolarMutexGuard aGuard;

    if (!m_pTextAttr)
        return {};

    SwPaM aPam(m_pTextAttr->GetTextNode(), m_pTextAttr->GetStart());
    aPam.SetMark();
    aPam.GetMark()->AdjustContent(+1);
    rtl::Reference<SwXTextRange> xRet
        = SwXTextRange::CreateXTextRange(aPam.GetDoc(), *aPam.Start(), aPam.End());
    return xRet;
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

SwTextLineBreak::SwTextLineBreak(const SfxPoolItemHolder& rAttr, sal_Int32 nStartPos)
    : SwTextAttr(rAttr, nStartPos)
    , m_pTextNode(nullptr)
{
    SwFormatLineBreak& rSwFormatLineBreak(static_cast<SwFormatLineBreak&>(GetAttr()));
    rSwFormatLineBreak.SetTextLineBreak(this);
    SetHasDummyChar(true);
}

SwTextLineBreak::~SwTextLineBreak() {}

void SwTextLineBreak::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextLineBreak"));
    if (m_pTextNode)
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("m_pTextNode"));
        (void)xmlTextWriterWriteAttribute(
            pWriter, BAD_CAST("index"),
            BAD_CAST(OString::number(sal_Int32(m_pTextNode->GetIndex())).getStr()));
        (void)xmlTextWriterEndElement(pWriter);
    }

    SwTextAttr::dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}

void SwTextLineBreak::SetTextNode(SwTextNode* pNew) { m_pTextNode = pNew; }

const SwTextNode& SwTextLineBreak::GetTextNode() const
{
    assert(m_pTextNode);
    return *m_pTextNode;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
