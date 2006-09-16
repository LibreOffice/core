/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: componenttools.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 23:56:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#ifndef FORMS_SOURCE_INC_COMPONENTTOOLS_HXX
#include "componenttools.hxx"
#endif

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

