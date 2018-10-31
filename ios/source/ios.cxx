/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <iostream>

#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/ucb/InteractiveAugmentedIOException.hpp"
#include "com/sun/star/ucb/NameClashException.hpp"
#include "ios/ios.hxx"

namespace
{
template <class E> void tryThrow(css::uno::Any const& aException)
{
    E aSpecificException;
    if (aException >>= aSpecificException)
        throw aSpecificException;
}
}

void lo_ios_throwException(css::uno::Any const& aException)
{
    assert(aException.getValueTypeClass() == css::uno::TypeClass_EXCEPTION);

    tryThrow<css::ucb::InteractiveAugmentedIOException>(aException);
    tryThrow<css::ucb::NameClashException>(aException);
    tryThrow<css::uno::RuntimeException>(aException);

    std::cerr << "lo_ios_throwException: Unhandled exception type " << aException.getValueTypeName()
              << std::endl;

    assert(false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
