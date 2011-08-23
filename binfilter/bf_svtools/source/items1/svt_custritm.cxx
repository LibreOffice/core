/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove


#include <com/sun/star/uno/Any.hxx>

#include <unotools/intlwrapper.hxx>

#include <tools/stream.hxx>

#include <bf_svtools/custritm.hxx>

namespace binfilter
{

//============================================================================
//
//  class CntUnencodedStringItem
//
//============================================================================

DBG_NAME(CntUnencodedStringItem)

//============================================================================
TYPEINIT1_AUTOFACTORY(CntUnencodedStringItem, SfxPoolItem)

//============================================================================
// virtual
int CntUnencodedStringItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    DBG_ASSERT(rItem.ISA(CntUnencodedStringItem),
               "CntUnencodedStringItem::operator ==(): Bad type");
    return m_aValue
            == SAL_STATIC_CAST(const CntUnencodedStringItem *, &rItem)->
                m_aValue;
}

//============================================================================
// virtual
int CntUnencodedStringItem::Compare(SfxPoolItem const & rWith) const
{
    DBG_ERROR("CntUnencodedStringItem::Compare(): No international");
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    DBG_ASSERT(rWith.ISA(CntUnencodedStringItem),
                "CntUnencodedStringItem::Compare(): Bad type");
    switch (m_aValue.CompareTo(static_cast< CntUnencodedStringItem const * >(
                                       &rWith)->
                                   m_aValue))
    {
        case COMPARE_LESS:
            return -1;

        case COMPARE_EQUAL:
            return 0;

        default: // COMPARE_GREATER
            return 1;
    }
}

//============================================================================
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

//============================================================================
// virtual
SfxItemPresentation
CntUnencodedStringItem::GetPresentation(SfxItemPresentation, SfxMapUnit,
                                        SfxMapUnit, XubString & rText,
                                        const ::IntlWrapper *) const
{
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    rText = m_aValue;
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
// virtual
bool CntUnencodedStringItem::QueryValue(com::sun::star::uno::Any& rVal, BYTE)
    const
{
    rVal <<= rtl::OUString(m_aValue);
    return true;
}

//============================================================================
// virtual
bool CntUnencodedStringItem::PutValue(const com::sun::star::uno::Any& rVal,
                                         BYTE)
{
    rtl::OUString aTheValue;
    if (rVal >>= aTheValue)
    {
        m_aValue = UniString(aTheValue);
        return true;
    }
    DBG_ERROR("CntUnencodedStringItem::PutValue(): Wrong type");
    return false;
}

//============================================================================
// virtual
SfxPoolItem * CntUnencodedStringItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    return new CntUnencodedStringItem(*this);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
