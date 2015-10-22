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

#ifndef INCLUDED_SFX2_SFXMODELFACTORY_HXX
#define INCLUDED_SFX2_SFXMODELFACTORY_HXX

#include <sfx2/dllapi.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <o3tl/typed_flags_set.hxx>

enum class SfxModelFlags
{
    NONE                       = 0x00,
    EMBEDDED_OBJECT            = 0x01,
    EXTERNAL_LINK              = 0x02,
    DISABLE_EMBEDDED_SCRIPTS   = 0x04,
    DISABLE_DOCUMENT_RECOVERY  = 0x08,
};
namespace o3tl
{
    template<> struct typed_flags<SfxModelFlags> : is_typed_flags<SfxModelFlags, 0x0f> {};
}

namespace sfx2
{
    typedef css::uno::Reference< css::uno::XInterface > ( SAL_CALL * SfxModelFactoryFunc ) (
        const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxFactory,
        SfxModelFlags _nCreationFlags
    );


    //= createSfxModelFactory

    /** creates a XSingleServiceFactory which can be used to created instances
        of classes derived from SfxBaseModel

        In opposite to the default implementations from module cppuhelper, this
        factory evaluates certain creation arguments (passed to createInstanceWithArguments)
        and passes them to the factory function of the derived class.
    */
    css::uno::Reference< css::lang::XSingleServiceFactory >
        SFX2_DLLPUBLIC createSfxModelFactory(
            const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxServiceFactory,
            const OUString& _rImplementationName,
            const SfxModelFactoryFunc _pComponentFactoryFunc,
            const css::uno::Sequence< OUString >& _rServiceNames
        );


} // namespace sfx2


#endif // INCLUDED_SFX2_SFXMODELFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
