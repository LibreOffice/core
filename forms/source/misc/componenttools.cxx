/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

