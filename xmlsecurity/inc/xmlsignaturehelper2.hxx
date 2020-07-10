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

#ifndef INCLUDED_XMLSECURITY_INC_XMLSIGNATUREHELPER2_HXX
#define INCLUDED_XMLSECURITY_INC_XMLSIGNATUREHELPER2_HXX

#include <rtl/ustring.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/xml/crypto/XUriBinding.hpp>

namespace com::sun::star {
    namespace io { class XInputStream; }
    namespace embed { class XStorage; }
}

// XUriBinding

class UriBindingHelper final : public cppu::WeakImplHelper< css::xml::crypto::XUriBinding >
{
private:
    css::uno::Reference < css::embed::XStorage > mxStorage;

public:
    UriBindingHelper();
    explicit UriBindingHelper( const css::uno::Reference < css::embed::XStorage >& rxStorage );

    void SAL_CALL setUriBinding( const OUString& uri, const css::uno::Reference< css::io::XInputStream >& aInputStream ) override;

    css::uno::Reference< css::io::XInputStream > SAL_CALL getUriBinding( const OUString& uri ) override;

    static css::uno::Reference < css::io::XInputStream > OpenInputStream( const css::uno::Reference < css::embed::XStorage >& rxStore, const OUString& rURI );
};

#endif // INCLUDED_XMLSECURITY_INC_XMLSIGNATUREHELPER2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
