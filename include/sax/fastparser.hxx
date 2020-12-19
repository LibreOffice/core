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

#ifndef INCLUDED_SAX_FASTPARSER_HXX
#define INCLUDED_SAX_FASTPARSER_HXX

#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XInitialization.hpp>

#include <sax/fastsaxdllapi.h>
#include <memory>

namespace com::sun::star::xml::sax {
    class XFastDocumentHandler;
    class XFastTokenHandler;
}

namespace sax_fastparser {


class FastSaxParserImpl;

// This class implements the external Parser interface
class FASTSAX_DLLPUBLIC FastSaxParser final
    : public ::cppu::WeakImplHelper<
                css::lang::XInitialization,
                css::xml::sax::XFastParser,
                css::lang::XServiceInfo >
{
    std::unique_ptr<FastSaxParserImpl> mpImpl;

public:
    FastSaxParser();
    virtual ~FastSaxParser() override;

    // css::lang::XInitialization:
    virtual void SAL_CALL initialize(css::uno::Sequence<css::uno::Any> const& rArguments) override;

    // XFastParser
    virtual void SAL_CALL parseStream( const css::xml::sax::InputSource& aInputSource ) override;
    virtual void SAL_CALL setFastDocumentHandler( const css::uno::Reference< css::xml::sax::XFastDocumentHandler >& Handler ) override;
    virtual void SAL_CALL setTokenHandler( const css::uno::Reference< css::xml::sax::XFastTokenHandler >& Handler ) override;
    virtual void SAL_CALL registerNamespace( const OUString& NamespaceURL, sal_Int32 NamespaceToken ) override;
    virtual OUString SAL_CALL getNamespaceURL( const OUString& rPrefix ) override;
    virtual void SAL_CALL setErrorHandler( const css::uno::Reference< css::xml::sax::XErrorHandler >& Handler ) override;
    virtual void SAL_CALL setEntityResolver( const css::uno::Reference< css::xml::sax::XEntityResolver >& Resolver ) override;
    virtual void SAL_CALL setLocale( const css::lang::Locale& rLocale ) override;
    virtual void SAL_CALL setNamespaceHandler( const css::uno::Reference< css::xml::sax::XFastNamespaceHandler >& Handler) override;
    virtual void SAL_CALL setCustomEntityNames( const ::css::uno::Sequence< ::css::beans::Pair<::rtl::OUString, ::rtl::OUString> >& replacements )  override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

}

#endif // _SAX_FASTPARSER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
