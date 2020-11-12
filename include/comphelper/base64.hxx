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

#ifndef INCLUDED_COMPHELPER_BASE64_HXX
#define INCLUDED_COMPHELPER_BASE64_HXX

#include <comphelper/comphelperdllapi.h>

#include <sal/types.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

namespace com::sun::star::uno
{
template <class E> class Sequence;
}

namespace comphelper
{
class COMPHELPER_DLLPUBLIC Base64
{
public:
    /** encodes the given byte sequence into Base64 */
    static void encode(OUStringBuffer& aStrBuffer, const css::uno::Sequence<sal_Int8>& aPass);

    static void encode(OStringBuffer& aStrBuffer, const css::uno::Sequence<sal_Int8>& aPass);

    // Decode a base 64 encoded string into a sequence of bytes. The first
    // version can be used for attribute values only, because it does not
    // return any chars left from conversion.
    // For text submitted through the SAX characters call, the later method
    // must be used!
    static void decode(css::uno::Sequence<sal_Int8>& aPass, const OUString& sBuffer);

    static sal_Int32 decodeSomeChars(css::uno::Sequence<sal_Int8>& aPass, const OUString& sBuffer);
};
}

#endif // INCLUDED_COMPHELPER_BASE64_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
