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

#ifndef INCLUDED_FORMS_SOURCE_INC_COMPONENTTOOLS_HXX
#define INCLUDED_FORMS_SOURCE_INC_COMPONENTTOOLS_HXX

#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/frame/XModel.hpp>

#include <set>
#include <functional>


namespace frm
{


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


    //= TypeBag

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


} // namespace frm


#endif // INCLUDED_FORMS_SOURCE_INC_COMPONENTTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
