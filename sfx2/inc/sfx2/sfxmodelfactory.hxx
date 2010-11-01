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

#ifndef SFX2_SFXMODELFACTORY_HXX
#define SFX2_SFXMODELFACTORY_HXX

#include "sfx2/dllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
/** === end UNO includes === **/

//........................................................................
namespace sfx2
{
//........................................................................

    #define SFXMODEL_STANDARD                   (sal_uInt64)(0x0000)
    #define SFXMODEL_EMBEDDED_OBJECT            (sal_uInt64)(0x0001)
    #define SFXMODEL_DISABLE_EMBEDDED_SCRIPTS   (sal_uInt64)(0x0002)
    #define SFXMODEL_DISABLE_DOCUMENT_RECOVERY  (sal_uInt64)(0x0004)

    typedef ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ( SAL_CALL * SfxModelFactoryFunc ) (
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory,
        const sal_uInt64 _nCreationFlags
    );

    //====================================================================
    //= createSfxModelFactory
    //====================================================================
    /** creates a XSingleServiceFactory which can be used to created instances
        of classes derived from SfxBaseModel

        In opposite to the default implementations from module cppuhelper, this
        factory evaluates certain creation arguments (passed to createInstanceWithArguments)
        and passes them to the factory function of the derived class.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >
        SFX2_DLLPUBLIC createSfxModelFactory(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxServiceFactory,
            const ::rtl::OUString& _rImplementationName,
            const SfxModelFactoryFunc _pComponentFactoryFunc,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rServiceNames
        );

//........................................................................
} // namespace sfx2
//........................................................................

#endif // SFX2_SFXMODELFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
