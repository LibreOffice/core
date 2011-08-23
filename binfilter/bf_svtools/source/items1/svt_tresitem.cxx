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


#ifndef SVTOOLS_TRESITEM_HXX
#include <bf_svtools/tresitem.hxx>
#endif

using namespace com::sun::star;

namespace binfilter
{

//============================================================================
//
//  CntTransferResultItem
//
//============================================================================

TYPEINIT1_AUTOFACTORY(CntTransferResultItem, SfxPoolItem)

//============================================================================
// virtual
int CntTransferResultItem::operator ==(SfxPoolItem const & rItem) const
{
    if (CntTransferResultItem * pResultItem = PTR_CAST(CntTransferResultItem,
                                                       &rItem))
        return m_aResult.Source == pResultItem->m_aResult.Source
               && m_aResult.Target == pResultItem->m_aResult.Target
               && m_aResult.Result == pResultItem->m_aResult.Result;
    return false;
}

//============================================================================
// virtual
BOOL CntTransferResultItem::QueryValue(uno::Any & rVal, BYTE) const
{
    rVal <<= m_aResult;
    return true;
}

//============================================================================
// virtual
BOOL CntTransferResultItem::PutValue(uno::Any const & rVal, BYTE)
{
    return rVal >>= m_aResult;
}

//============================================================================
// virtual
SfxPoolItem * CntTransferResultItem::Clone(SfxItemPool *) const
{
    return new CntTransferResultItem(*this);
}

}
