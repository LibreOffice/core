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
#ifndef INCLUDED_SW_INC_FORMATLINEBREAK_HXX
#define INCLUDED_SW_INC_FORMATLINEBREAK_HXX

#include "swdllapi.h"
#include <svl/eitem.hxx>
#include "calbck.hxx"

#include <cppuhelper/weakref.hxx>
#include <unotools/weakref.hxx>
#include <com/sun/star/text/XTextContent.hpp>

class SwDoc;
class SwTextLineBreak;
class SwXLineBreak;
class SwXTextRange;

/// Defines the location of a line break text wrapping restart.
enum class SwLineBreakClear
{
    NONE,
    LEFT,
    RIGHT,
    ALL,
    LAST = ALL
};

/// SfxPoolItem subclass that wraps an SwLineBreakClear.
class SW_DLLPUBLIC SwFormatLineBreak final : public SfxEnumItem<SwLineBreakClear>,
                                             public SvtBroadcaster
{
    /// The SwTextAttr that knows the position of the line break in the doc model.
    SwTextLineBreak* m_pTextAttr;

    unotools::WeakReference<SwXLineBreak> m_wXLineBreak;

    SwFormatLineBreak& operator=(const SwFormatLineBreak& rLineBreak) = delete;

    SwFormatLineBreak(const SwFormatLineBreak&) = delete;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SwFormatLineBreak)
    SwFormatLineBreak(SwLineBreakClear eClear);
    virtual ~SwFormatLineBreak() override;

    /// See SfxPoolItem::operator ==().
    bool operator==(const SfxPoolItem&) const override;

    /// See SfxPoolItem::Clone().
    SwFormatLineBreak* Clone(SfxItemPool* pPool = nullptr) const override;

    sal_uInt16 GetValueCount() const override;

    rtl::Reference<SwXTextRange> GetAnchor() const;

    void SetTextLineBreak(SwTextLineBreak* pTextAttr) { m_pTextAttr = pTextAttr; }

    unotools::WeakReference<SwXLineBreak> const& GetXTextContent() const { return m_wXLineBreak; }

    void SetXLineBreak(rtl::Reference<SwXLineBreak> const& xLineBreak);

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
