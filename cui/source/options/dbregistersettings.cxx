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

#include "dbregistersettings.hxx"

#include <rtl/ustring.hxx>

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= DatabaseMapItem
    //====================================================================
    TYPEINIT1( DatabaseMapItem, SfxPoolItem )
    //--------------------------------------------------------------------
    DatabaseMapItem::DatabaseMapItem( sal_uInt16 _nId, const DatabaseRegistrations& _rRegistrations )
        :SfxPoolItem( _nId )
        ,m_aRegistrations( _rRegistrations )
    {
    }

    //--------------------------------------------------------------------
    int DatabaseMapItem::operator==( const SfxPoolItem& _rCompare ) const
    {
        const DatabaseMapItem* pItem = PTR_CAST(DatabaseMapItem, &_rCompare);
        if ( !pItem )
            return sal_False;

        if ( m_aRegistrations.size() != pItem->m_aRegistrations.size() )
            return sal_False;

        return m_aRegistrations == pItem->m_aRegistrations;
    }

    //--------------------------------------------------------------------
    SfxPoolItem* DatabaseMapItem::Clone( SfxItemPool* ) const
    {
        return new DatabaseMapItem( Which(), m_aRegistrations );
    }

    //--------------------------------------------------------------------

//........................................................................
}   // namespace svx
//........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
