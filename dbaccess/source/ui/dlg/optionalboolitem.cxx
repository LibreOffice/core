/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "optionalboolitem.hxx"

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= OptionalBoolItem
    //====================================================================
    TYPEINIT1( OptionalBoolItem, SfxPoolItem );
    //--------------------------------------------------------------------
    OptionalBoolItem::OptionalBoolItem( sal_Int16 _nWhich )
        :SfxPoolItem( _nWhich )
        ,m_aValue()
    {
    }

    //--------------------------------------------------------------------
    OptionalBoolItem::OptionalBoolItem( const OptionalBoolItem& _rSource )
        :SfxPoolItem( _rSource )
        ,m_aValue( _rSource.m_aValue )
    {
    }

    //--------------------------------------------------------------------
    int OptionalBoolItem::operator==( const SfxPoolItem& _rItem ) const
    {
        const OptionalBoolItem* pCompare = PTR_CAST( OptionalBoolItem, &_rItem );
        if ( !pCompare )
            return 0;

        if ( m_aValue == pCompare->m_aValue )
            return 1;

        return 0;
    }

    //--------------------------------------------------------------------
    SfxPoolItem* OptionalBoolItem::Clone( SfxItemPool* /*_pPool*/ ) const
    {
        return new OptionalBoolItem( *this );
    }

//........................................................................
} // namespace dbaui
//........................................................................
