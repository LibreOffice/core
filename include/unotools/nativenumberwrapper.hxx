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
#include <unotools/unotoolsdllapi.h>

#ifndef INCLUDED_UNOTOOLS_NATIVENUMBERWRAPPER_HXX
#define INCLUDED_UNOTOOLS_NATIVENUMBERWRAPPER_HXX
#include <com/sun/star/i18n/XNativeNumberSupplier.hpp>

namespace com { namespace sun { namespace star {
    namespace uno {
        class XComponentContext;
    }
}}}

class UNOTOOLS_DLLPUBLIC NativeNumberWrapper
{
    css::uno::Reference< css::i18n::XNativeNumberSupplier >   xNNS;
                                NativeNumberWrapper( const NativeNumberWrapper& ) = delete;
            NativeNumberWrapper&    operator=( const NativeNumberWrapper& ) = delete;

public:
                                NativeNumberWrapper(
                                    const css::uno::Reference< css::uno::XComponentContext > & rxContext
                                    );

                                ~NativeNumberWrapper();

    // Wrapper implementations of XNativeNumberSupplier

    OUString getNativeNumberString(
                        const OUString& rNumberString,
                        const css::lang::Locale& rLocale,
                        sal_Int16 nNativeNumberMode ) const;

    css::i18n::NativeNumberXmlAttributes convertToXmlAttributes(
                        const css::lang::Locale& rLocale,
                        sal_Int16 nNativeNumberMode ) const;

    sal_Int16       convertFromXmlAttributes(
                        const css::i18n::NativeNumberXmlAttributes& rAttr ) const;

};

#endif // INCLUDED_UNOTOOLS_NATIVENUMBERWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
