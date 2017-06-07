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

#ifndef INCLUDED_FRAMEWORK_INC_MACROS_XTYPEPROVIDER_HXX
#define INCLUDED_FRAMEWORK_INC_MACROS_XTYPEPROVIDER_HXX

#include <com/sun/star/lang/XTypeProvider.hpp>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <cppuhelper/typeprovider.hxx>

namespace framework{

/*_________________________________________________________________________________________________________________

    macros for declaration and definition of XTypeProvider
    Please use follow public macros only!

    1)  DEFINE_XTYPEPROVIDER                                                            => use it in header to declare XTypeProvider and his methods
    2)  DECLARE_TYPEPROVIDER_0( CLASS )                                                 => use it to define implementation of XTypeProvider for 0 supported type
        DECLARE_TYPEPROVIDER_1( CLASS, TYPE1 )                                          => use it to define implementation of XTypeProvider for 1 supported type
        ...
        DECLARE_TYPEPROVIDER_16( CLASS, TYPE1, ... , TYPE16 )
    3)  DEFINE_XTYPEPROVIDER_1_WITH_BASECLASS( CLASS, BASECLASS, TYPE1 )                => use it to define implementation of XTypeProvider for 1 additional supported type to baseclass
        ...
        DEFINE_XTYPEPROVIDER_5_WITH_BASECLASS( CLASS, BASECLASS, TYPE1, ..., TYPE5 )

_________________________________________________________________________________________________________________*/

//  private
//  implementation of XTypeProvider::getImplementationId()

#define PRIVATE_DEFINE_XTYPEPROVIDER_GETIMPLEMENTATIONID( CLASS )                                                                               \
    css::uno::Sequence< sal_Int8 > SAL_CALL CLASS::getImplementationId()          \
    {                                                                                                                                           \
        return css::uno::Sequence<sal_Int8>();                                                                                                  \
    }

//  private
//  help macros to replace TYPES in getTypes() [see before]

#define PRIVATE_DEFINE_TYPE_11( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11 )                                 \
    cppu::UnoType<TYPE1>::get(), \
    cppu::UnoType<TYPE2>::get(), \
    cppu::UnoType<TYPE3>::get(), \
    cppu::UnoType<TYPE4>::get(), \
    cppu::UnoType<TYPE5>::get(), \
    cppu::UnoType<TYPE6>::get(), \
    cppu::UnoType<TYPE7>::get(), \
    cppu::UnoType<TYPE8>::get(), \
    cppu::UnoType<TYPE9>::get(), \
    cppu::UnoType<TYPE10>::get(), \
    cppu::UnoType<TYPE11>::get()

//  private
//  complete implementation of XTypeProvider with max. 12 interfaces!

#define PRIVATE_DEFINE_XTYPEPROVIDER( CLASS, TYPES )                 \
    PRIVATE_DEFINE_XTYPEPROVIDER_GETIMPLEMENTATIONID( CLASS )        \
    css::uno::Sequence< css::uno::Type > SAL_CALL CLASS::getTypes()  \
    {                                                                \
        /* Attention: "TYPES" will expand to "(...)"!   */           \
        static cppu::OTypeCollection ourTypeCollection TYPES;        \
        return ourTypeCollection.getTypes();                         \
    }

//  public
//  declaration of XTypeProvider

#define FWK_DECLARE_XTYPEPROVIDER                                                                                                                               \
    virtual css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes           () override;\
    virtual css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

//  public
//  implementation of XTypeProvider

//  implementation of XTypeProvider without additional interface for getTypes()
//  XTypeProvider is used as the only one interface automatically.


//  implementation of XTypeProvider with 11 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_11( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11 ) \
    PRIVATE_DEFINE_XTYPEPROVIDER    (   CLASS,                                                                          \
                                        (PRIVATE_DEFINE_TYPE_11 (   TYPE1   ,                                           \
                                                                    TYPE2   ,                                           \
                                                                    TYPE3   ,                                           \
                                                                    TYPE4   ,                                           \
                                                                    TYPE5   ,                                           \
                                                                    TYPE6   ,                                           \
                                                                    TYPE7   ,                                           \
                                                                    TYPE8   ,                                           \
                                                                    TYPE9   ,                                           \
                                                                    TYPE10  ,                                           \
                                                                    TYPE11                                              \
                                                                ))                                                      \
                                    )

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_MACROS_XTYPEPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
