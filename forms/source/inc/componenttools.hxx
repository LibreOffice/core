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

#ifndef FORMS_SOURCE_INC_COMPONENTTOOLS_HXX
#define FORMS_SOURCE_INC_COMPONENTTOOLS_HXX

/** === begin UNO includes === **/
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/frame/XModel.hpp>
/** === end UNO includes === **/

#include <set>
#include <functional>

//........................................................................
namespace frm
{
//........................................................................

    struct TypeCompareLess : public ::std::binary_function< ::com::sun::star::uno::Type, ::com::sun::star::uno::Type, bool >
    {
    private:
        typedef ::com::sun::star::uno::Type             Type;

    public:
        bool operator()( const Type& _rLHS, const Type& _rRHS ) const
        {
            return _rLHS.getTypeName() < _rRHS.getTypeName();
        }
    };

    //====================================================================
    //= TypeBag
    //====================================================================
    /** a helper class which merges sequences of <type scope="com::sun::star::uno">Type</type>s,
        so that the resulting sequence contains every type at most once
    */
    class TypeBag
    {
    public:
        typedef ::com::sun::star::uno::Type             Type;
        typedef ::com::sun::star::uno::Sequence< Type > TypeSequence;
        typedef ::std::set< Type, TypeCompareLess >     TypeSet;

    private:
        TypeSet     m_aTypes;

    public:
        TypeBag(
            const TypeSequence& _rTypes1
        );

        TypeBag(
            const TypeSequence& _rTypes1,
            const TypeSequence& _rTypes2
        );
        TypeBag(
            const TypeSequence& _rTypes1,
            const TypeSequence& _rTypes2,
            const TypeSequence& _rTypes3
        );

        void    addType( const Type& i_rType );
        void    addTypes( const TypeSequence& _rTypes );
        void    removeType( const Type& i_rType );

        /** returns the types represented by this bag
        */
        TypeSequence    getTypes() const;
    };

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >  getXModel(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxComponent );

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_INC_COMPONENTTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
