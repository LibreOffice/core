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

#include "componenttools.hxx"

#include <com/sun/star/container/XChild.hpp>
#include <comphelper/sequence.hxx>

#include <algorithm>
#include <iterator>


namespace frm
{


    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::container::XChild;

    TypeBag::TypeBag( const TypeSequence& _rTypes1 )
    {
        addTypes( _rTypes1 );
    }


    TypeBag::TypeBag( const TypeSequence& _rTypes1, const TypeSequence& _rTypes2 )
    {
        addTypes( _rTypes1 );
        addTypes( _rTypes2 );
    }


    TypeBag::TypeBag( const TypeSequence& _rTypes1, const TypeSequence& _rTypes2, const TypeSequence& _rTypes3 )
    {
        addTypes( _rTypes1 );
        addTypes( _rTypes2 );
        addTypes( _rTypes3 );
    }


    void TypeBag::addTypes( const TypeSequence& _rTypes )
    {
        ::std::copy(
            _rTypes.begin(),
            _rTypes.end(),
            ::std::insert_iterator< TypeSet >( m_aTypes, m_aTypes.begin() )
        );
    }


    void TypeBag::addType( const css::uno::Type& i_rType )
    {
        m_aTypes.insert( i_rType );
    }


    void TypeBag::removeType( const css::uno::Type& i_rType )
    {
        m_aTypes.erase( i_rType );
    }


    TypeBag::TypeSequence TypeBag::getTypes() const
    {
        return comphelper::containerToSequence(m_aTypes);
    }


    Reference< XModel >  getXModel( const Reference< XInterface >& _rxComponent )
    {
        Reference< XInterface > xParent = _rxComponent;
        Reference< XModel > xModel( xParent, UNO_QUERY );
        while ( xParent.is() && !xModel.is() )
        {
            Reference< XChild > xChild( xParent, UNO_QUERY );
            xParent.set( xChild.is() ? xChild->getParent() : Reference< XInterface >(), UNO_QUERY );
            xModel.set( xParent, UNO_QUERY );
        }
        return xModel;
    }


} // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
