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


    struct TypeCompareLess : public ::std::binary_function< css::uno::Type, css::uno::Type, bool >
    {
        bool operator()( const css::uno::Type& _rLHS, const css::uno::Type& _rRHS ) const
        {
            return _rLHS.getTypeName() < _rRHS.getTypeName();
        }
    };

    /** a helper class which merges sequences of <type scope="css::uno">Type</type>s,
        so that the resulting sequence contains every type at most once
    */
    class TypeBag
    {
    public:
        typedef css::uno::Sequence< css::uno::Type >            TypeSequence;
        typedef ::std::set< css::uno::Type, TypeCompareLess >   TypeSet;

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

        void    addType( const css::uno::Type& i_rType );
        void    addTypes( const TypeSequence& _rTypes );
        void    removeType( const css::uno::Type& i_rType );

        /** returns the types represented by this bag
        */
        TypeSequence    getTypes() const;
    };

    css::uno::Reference< css::frame::XModel >  getXModel(
        const css::uno::Reference< css::uno::XInterface >& _rxComponent );


} // namespace frm


#endif // INCLUDED_FORMS_SOURCE_INC_COMPONENTTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
