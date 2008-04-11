/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: componenttools.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_forms.hxx"
#include "componenttools.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <algorithm>
#include <iterator>

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= TypeBag
    //====================================================================
    //--------------------------------------------------------------------
    TypeBag::TypeBag( const TypeSequence& _rTypes1 )
    {
        addTypes( _rTypes1 );
    }

    //--------------------------------------------------------------------
    TypeBag::TypeBag( const TypeSequence& _rTypes1, const TypeSequence& _rTypes2 )
    {
        addTypes( _rTypes1 );
        addTypes( _rTypes2 );
    }

    //--------------------------------------------------------------------
    TypeBag::TypeBag( const TypeSequence& _rTypes1, const TypeSequence& _rTypes2, const TypeSequence& _rTypes3 )
    {
        addTypes( _rTypes1 );
        addTypes( _rTypes2 );
        addTypes( _rTypes3 );
    }

    //--------------------------------------------------------------------
    TypeBag::TypeBag( const TypeSequence& _rTypes1, const TypeSequence& _rTypes2, const TypeSequence& _rTypes3,
        const TypeSequence& _rTypes4 )
    {
        addTypes( _rTypes1 );
        addTypes( _rTypes2 );
        addTypes( _rTypes3 );
        addTypes( _rTypes4 );
    }

    //--------------------------------------------------------------------
    TypeBag::TypeBag( const TypeSequence& _rTypes1, const TypeSequence& _rTypes2, const TypeSequence& _rTypes3,
        const TypeSequence& _rTypes4, const TypeSequence& _rTypes5 )
    {
        addTypes( _rTypes1 );
        addTypes( _rTypes2 );
        addTypes( _rTypes3 );
        addTypes( _rTypes4 );
        addTypes( _rTypes5 );
    }

    //--------------------------------------------------------------------
    void TypeBag::addTypes( const TypeSequence& _rTypes )
    {
        ::std::copy(
            _rTypes.getConstArray(),
            _rTypes.getConstArray() + _rTypes.getLength(),
            ::std::insert_iterator< TypeSet >( m_aTypes, m_aTypes.begin() )
        );
    }

    //--------------------------------------------------------------------
    TypeBag::TypeSequence TypeBag::getTypes() const
    {
        TypeSequence aTypes( m_aTypes.size() );
        ::std::copy( m_aTypes.begin(), m_aTypes.end(), aTypes.getArray() );
        return aTypes;
    }

//........................................................................
} // namespace frm
//........................................................................

