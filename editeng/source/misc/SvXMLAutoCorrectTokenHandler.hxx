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

#ifndef EDITENG_SOURCE_MISC_SVXMLAUTOCORRECTTOKENHANDLER_HXX
#define EDITENG_SOURCE_MISC_SVXMLAUTOCORRECTTOKENHANDLER_HXX

#include <sal/types.h>
#include <xmloff/dllapi.h>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>

using namespace ::css::xml::sax;
using namespace ::xmloff::token;

enum SvXMLAutoCorrectToken : sal_Int32
{
    NAMESPACE = FastToken::NAMESPACE | XML_NAMESPACE_BLOCKLIST, //65553
    ABBREVIATED_NAME = FastToken::NAMESPACE | XML_NAMESPACE_BLOCKLIST | XML_ABBREVIATED_NAME,   //65655
    BLOCK = FastToken::NAMESPACE | XML_NAMESPACE_BLOCKLIST | XML_BLOCK, //65791
    BLOCKLIST = FastToken::NAMESPACE | XML_NAMESPACE_BLOCKLIST | XML_BLOCK_LIST, //65792
    NAME = FastToken::NAMESPACE | XML_NAMESPACE_BLOCKLIST | XML_NAME    //66737
};

class XMLOFF_DLLPUBLIC SvXMLAutoCorrectTokenHandler : public
        cppu::WeakImplHelper1< css::xml::sax::XFastTokenHandler >
{
public:
    SvXMLAutoCorrectTokenHandler();
    ~SvXMLAutoCorrectTokenHandler();

    //XFastTokenHandler
    sal_Int32 SAL_CALL getTokenFromUTF8( const css::uno::Sequence<signed char>& Identifier )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    css::uno::Sequence<signed char> SAL_CALL getUTF8Identifier( sal_Int32 Token )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif // EDITENG_SOURCE_MISC_SVXMLAUTOCORRECTTOKENHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

