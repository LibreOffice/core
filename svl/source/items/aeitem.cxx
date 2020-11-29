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

#include <rtl/ustring.hxx>
#include <svl/aeitem.hxx>

SfxAllEnumItem::SfxAllEnumItem(sal_uInt16 which):
    SfxPoolItem(which)
{
}

SfxAllEnumItem::SfxAllEnumItem(const SfxAllEnumItem &rCopy):
    SfxPoolItem(rCopy),
    m_Values(rCopy.m_Values)
{
}

SfxAllEnumItem::~SfxAllEnumItem()
{
}

sal_Int32 SfxAllEnumItem::GetTextCount() const
{
    return m_Values.size();
}

OUString const & SfxAllEnumItem::GetTextByPos( sal_uInt16 nPos ) const
{
    return m_Values[nPos];
}

SfxAllEnumItem* SfxAllEnumItem::Clone( SfxItemPool * ) const
{
    return new SfxAllEnumItem(*this);
}

void SfxAllEnumItem::SetTextByPos( sal_uInt16 nPos, const OUString &rText )
{
    if (nPos >= m_Values.size())
        m_Values.resize(nPos);
    m_Values[nPos] = rText;
}

bool SfxAllEnumItem::operator==( const SfxPoolItem& rCmp ) const
{
    if (!SfxPoolItem::operator==(rCmp))
        return false;
    const SfxAllEnumItem& rOther = static_cast<const SfxAllEnumItem&>(rCmp);
    return m_Values == rOther.m_Values;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
