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

#include <editeng/memberids.h>
#include <editeng/rubyitem.hxx>

using namespace ::com::sun::star;

SfxPoolItem* SvxRubyItem::CreateDefault() { return new SvxRubyItem(0); }

SvxRubyItem::SvxRubyItem(const sal_uInt16 nId)
    : SfxPoolItem(nId)
{
}

const OUString& SvxRubyItem::GetText() const { return m_aText; }

void SvxRubyItem::SetText(OUString aValue)
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    m_aText = std::move(aValue);
}

css::text::RubyAdjust SvxRubyItem::GetAdjustment() const { return m_eAdjustment; }

void SvxRubyItem::SetAdjustment(css::text::RubyAdjust eValue)
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    m_eAdjustment = eValue;
}

sal_Int16 SvxRubyItem::GetPosition() const { return m_ePosition; }

void SvxRubyItem::SetPosition(sal_Int16 eValue)
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    m_ePosition = eValue;
}

bool SvxRubyItem::operator==(const SfxPoolItem& rItem) const
{
    assert(SfxPoolItem::operator==(rItem));
    const auto& rCastItem = static_cast<const SvxRubyItem&>(rItem);

    return std::tie(m_aText, m_eAdjustment, m_ePosition)
           == std::tie(rCastItem.m_aText, rCastItem.m_eAdjustment, rCastItem.m_ePosition);
}

SvxRubyItem* SvxRubyItem::Clone(SfxItemPool*) const { return new SvxRubyItem(*this); }

bool SvxRubyItem::GetPresentation(SfxItemPresentation /*ePres*/, MapUnit /*eCoreUnit*/,
                                  MapUnit /*ePresUnit*/, OUString& rText,
                                  const IntlWrapper& /*rIntl*/
                                  ) const
{
    rText = m_aText;
    return true;
}

bool SvxRubyItem::QueryValue(uno::Any& rVal, sal_uInt8 nMemberId) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case MID_RUBY_TEXT:
            rVal <<= m_aText;
            return true;

        case MID_RUBY_ADJUST:
            rVal <<= static_cast<sal_Int16>(m_eAdjustment);
            return true;

        case MID_RUBY_POSITION:
            rVal <<= m_ePosition;
            return true;
    }

    return false;
}

bool SvxRubyItem::PutValue(const uno::Any& rVal, sal_uInt8 nMemberId)
{
    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case MID_RUBY_TEXT:
            if (OUString aValue; rVal >>= aValue)
            {
                SetText(std::move(aValue));
                return true;
            }
            break;

        case MID_RUBY_ADJUST:
            if (sal_Int16 eValue; rVal >>= eValue)
            {
                if (eValue >= sal_Int16(css::text::RubyAdjust_LEFT)
                    && eValue <= sal_Int16(css::text::RubyAdjust_INDENT_BLOCK))
                {
                    SetAdjustment(css::text::RubyAdjust{ eValue });
                    return true;
                }
            }
            break;

        case MID_RUBY_POSITION:
            if (sal_Int16 eValue; rVal >>= eValue)
            {
                SetPosition(eValue);
                return true;
            }
            break;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
