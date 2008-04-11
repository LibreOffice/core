/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: componenttools.hxx,v $
 * $Revision: 1.4 $
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
        bool operator()( const Type& _rLHS, const Type& _rRHS )
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
        TypeBag(
            const TypeSequence& _rTypes1,
            const TypeSequence& _rTypes2,
            const TypeSequence& _rTypes3,
            const TypeSequence& _rTypes4
        );
        TypeBag(
            const TypeSequence& _rTypes1,
            const TypeSequence& _rTypes2,
            const TypeSequence& _rTypes3,
            const TypeSequence& _rTypes4,
            const TypeSequence& _rTypes5
        );

        void addTypes( const TypeSequence& _rTypes );

        /** returns the types represented by this bag
        */
        TypeSequence    getTypes() const;
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_INC_COMPONENTTOOLS_HXX

