/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "CipherContext.hxx"

using namespace css;
using namespace css::uno;
using namespace css::lang;

Sequence< sal_Int8 > SAL_CALL CipherContext::convertWithCipherContext( const Sequence< sal_Int8 >& aData )
    throw ( IllegalArgumentException, DisposedException, RuntimeException, std::exception)
{
}

uno::Sequence< sal_Int8 > SAL_CALL CipherContext::finalizeCipherContextAndDispose()
    throw (DisposedException, RuntimeException, std::exception)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
