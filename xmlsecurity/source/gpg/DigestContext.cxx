/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "DigestContext.hxx"

using namespace css;
using namespace css::uno;
using namespace css::lang;


void SAL_CALL DigestContext::updateDigest( const uno::Sequence< ::sal_Int8 >& /*aData*/ )
    throw (DisposedException, RuntimeException, std::exception)
{
}

uno::Sequence< sal_Int8 > SAL_CALL DigestContext::finalizeDigestAndDispose()
    throw (DisposedException, RuntimeException, std::exception)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
