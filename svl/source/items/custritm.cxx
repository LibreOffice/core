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

#include <osl/diagnose.h>
#include <unotools/intlwrapper.hxx>
#include <tools/stream.hxx>
#include <svl/custritm.hxx>


// virtual
bool CntUnencodedStringItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_ASSERT(dynamic_cast<const CntUnencodedStringItem*>( &rItem ) !=  nullptr,
               "CntUnencodedStringItem::operator ==(): Bad type");
    return m_aValue
            == (static_cast< const CntUnencodedStringItem * >(&rItem))->
                m_aValue;
}

// virtual
bool CntUnencodedStringItem::GetPresentation(SfxItemPresentation, SfxMapUnit,
                                        SfxMapUnit, OUString & rText,
                                        const IntlWrapper *) const
{
    rText = m_aValue;
    return true;
}

// virtual
bool CntUnencodedStringItem::QueryValue(css::uno::Any& rVal, sal_uInt8)
    const
{
    rVal <<= OUString(m_aValue);
    return true;
}

// virtual
bool CntUnencodedStringItem::PutValue(const css::uno::Any& rVal,
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
    return new CntUnencodedStringItem(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
