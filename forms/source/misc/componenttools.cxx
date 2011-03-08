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
#include "precompiled_forms.hxx"
#include "componenttools.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/container/XChild.hpp>
/** === end UNO includes === **/

#include <algorithm>
#include <iterator>

//........................................................................
namespace frm
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::container::XChild;
    /** === end UNO using === **/

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
    void TypeBag::addTypes( const TypeSequence& _rTypes )
    {
        ::std::copy(
            _rTypes.getConstArray(),
            _rTypes.getConstArray() + _rTypes.getLength(),
            ::std::insert_iterator< TypeSet >( m_aTypes, m_aTypes.begin() )
        );
    }

    //--------------------------------------------------------------------
    void TypeBag::addType( const Type& i_rType )
    {
        m_aTypes.insert( i_rType );
    }

    //--------------------------------------------------------------------
    void TypeBag::removeType( const TypeBag::Type& i_rType )
    {
        m_aTypes.erase( i_rType );
    }

    //--------------------------------------------------------------------
    TypeBag::TypeSequence TypeBag::getTypes() const
    {
        TypeSequence aTypes( m_aTypes.size() );
        ::std::copy( m_aTypes.begin(), m_aTypes.end(), aTypes.getArray() );
        return aTypes;
    }

    //====================================================================
    Reference< XModel >  getXModel( const Reference< XInterface >& _rxComponent )
    {
        Reference< XInterface > xParent = _rxComponent;
        Reference< XModel > xModel( xParent, UNO_QUERY );;
        while ( xParent.is() && !xModel.is() )
        {
            Reference< XChild > xChild( xParent, UNO_QUERY );
            xParent.set( xChild.is() ? xChild->getParent() : Reference< XInterface >(), UNO_QUERY );
            xModel.set( xParent, UNO_QUERY );
        }
        return xModel;
    }

//........................................................................
} // namespace frm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
