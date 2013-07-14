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

#include <com/sun/star/uno/Any.hxx>

#include <unotools/intlwrapper.hxx>
#include <tools/stream.hxx>
#include <svl/custritm.hxx>

//
//  class CntUnencodedStringItem
//

DBG_NAME(CntUnencodedStringItem)

TYPEINIT1_AUTOFACTORY(CntUnencodedStringItem, SfxPoolItem)

// virtual
int CntUnencodedStringItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    DBG_ASSERT(rItem.ISA(CntUnencodedStringItem),
               "CntUnencodedStringItem::operator ==(): Bad type");
    return m_aValue
            == (static_cast< const CntUnencodedStringItem * >(&rItem))->
                m_aValue;
}

// virtual
int CntUnencodedStringItem::Compare(SfxPoolItem const & rWith) const
{
    OSL_FAIL("CntUnencodedStringItem::Compare(): No international");
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    DBG_ASSERT(rWith.ISA(CntUnencodedStringItem),
                "CntUnencodedStringItem::Compare(): Bad type");
    sal_Int32 nCmp = m_aValue.compareTo(
        static_cast< CntUnencodedStringItem const * >(&rWith)->m_aValue);
    return (nCmp == 0) ? 0 : (nCmp < 0) ? -1 : 1;
}

// virtual
int CntUnencodedStringItem::Compare(SfxPoolItem const & rWith,
                                    IntlWrapper const & rIntlWrapper)
    const
{
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    DBG_ASSERT(rWith.ISA(CntUnencodedStringItem),
               "CntUnencodedStringItem::Compare(): Bad type");
    return rIntlWrapper.getCollator()->compareString( m_aValue,
        static_cast< CntUnencodedStringItem const * >(&rWith)->m_aValue );
}

// virtual
SfxItemPresentation
CntUnencodedStringItem::GetPresentation(SfxItemPresentation, SfxMapUnit,
                                        SfxMapUnit, OUString & rText,
                                        const IntlWrapper *) const
{
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    rText = m_aValue;
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// virtual
bool CntUnencodedStringItem::QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8)
    const
{
    rVal <<= OUString(m_aValue);
    return true;
}

// virtual
bool CntUnencodedStringItem::PutValue(const com::sun::star::uno::Any& rVal,
                                         sal_uInt8)
{
    OUString aTheValue;
    if (rVal >>= aTheValue)
    {
        m_aValue = aTheValue;
        return true;
    }
    OSL_FAIL("CntUnencodedStringItem::PutValue(): Wrong type");
    return false;
}

// virtual
SfxPoolItem * CntUnencodedStringItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    return new CntUnencodedStringItem(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
