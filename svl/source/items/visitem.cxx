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

#include <svl/visitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <osl/diagnose.h>
#include <tools/stream.hxx>


SfxVisibilityItem::SfxVisibilityItem(sal_uInt16 which, SvStream & rStream):
    SfxPoolItem(which)
{
    bool bValue = false;
    rStream.ReadCharAsBool( bValue );
    m_nValue.bVisible = bValue;
}

// virtual
bool SfxVisibilityItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_ASSERT(SfxPoolItem::operator ==(rItem), "unequal type");
    return m_nValue.bVisible == (static_cast< const SfxVisibilityItem * >(&rItem))->
                        m_nValue.bVisible;
}

// virtual
bool SfxVisibilityItem::GetPresentation(SfxItemPresentation,
                                                  SfxMapUnit, SfxMapUnit,
                                                  OUString & rText,
                                                  const IntlWrapper *) const
{
    rText = GetValueTextByVal(m_nValue.bVisible);
    return true;
}


// virtual
bool SfxVisibilityItem::QueryValue(css::uno::Any& rVal, sal_uInt8) const
{
    rVal <<= m_nValue;
    return true;
}

// virtual
bool SfxVisibilityItem::PutValue(const css::uno::Any& rVal, sal_uInt8)
{
    if (rVal >>= m_nValue)
        return true;

    OSL_FAIL( "SfxInt16Item::PutValue - Wrong type!" );
    return false;
}

// virtual
SfxPoolItem * SfxVisibilityItem::Create(SvStream & rStream, sal_uInt16) const
{
    return new SfxVisibilityItem(Which(), rStream);
}

// virtual
SvStream & SfxVisibilityItem::Store(SvStream & rStream, sal_uInt16) const
{
    rStream.WriteUChar( m_nValue.bVisible );
    return rStream;
}

// virtual
SfxPoolItem * SfxVisibilityItem::Clone(SfxItemPool *) const
{
    return new SfxVisibilityItem(*this);
}

// virtual
OUString SfxVisibilityItem::GetValueTextByVal(bool bTheValue) const
{
    return bTheValue ? OUString("TRUE") : OUString("FALSE");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
