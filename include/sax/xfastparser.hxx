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
#pragma once

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/Pair.hpp>
#include <sax/saxdllapi.h>

namespace com::sun::star::xml::sax
{
struct InputSource;
class XErrorHandler;
class XFastDocumentHandler;
class XEntityResolver;
class XFastTokenHandler;
class XFastNamespaceHandler;
}
namespace com::sun::star::lang
{
struct Locale;
}

class SAX_DLLPUBLIC SAL_LOPLUGIN_ANNOTATE("crosscast") XFastParser
{
public:
    virtual ~XFastParser();

    // XFastParser
    virtual void parseStream(const css::xml::sax::InputSource& aInputSource) = 0;
    virtual void
    setFastDocumentHandler(const css::uno::Reference<css::xml::sax::XFastDocumentHandler>& Handler)
        = 0;
    virtual void
    setTokenHandler(const css::uno::Reference<css::xml::sax::XFastTokenHandler>& Handler)
        = 0;
    virtual void registerNamespace(const OUString& NamespaceURL, sal_Int32 NamespaceToken) = 0;
    virtual OUString getNamespaceURL(std::u16string_view Prefix) = 0;
    virtual void setErrorHandler(const css::uno::Reference<css::xml::sax::XErrorHandler>& Handler)
        = 0;
    virtual void
    setEntityResolver(const css::uno::Reference<css::xml::sax::XEntityResolver>& Resolver)
        = 0;
    virtual void setLocale(const css::lang::Locale& rLocale) = 0;
    virtual void
    setNamespaceHandler(const css::uno::Reference<css::xml::sax::XFastNamespaceHandler>& Handler)
        = 0;
    virtual void setCustomEntityNames(
        const ::css::uno::Sequence<::css::beans::Pair<::rtl::OUString, ::rtl::OUString>>&
            replacements)
        = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
