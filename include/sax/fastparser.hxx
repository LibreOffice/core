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
#include <cppuhelper/implbase2.hxx>

#include <sax/fastsaxdllapi.h>

namespace com { namespace sun { namespace star { namespace xml { namespace sax {

class XFastContextHandler;
class XFastDocumentHandler;
class XFastTokenHandler;

}}}}}

namespace sax_fastparser {



class FastSaxParserImpl;

// This class implements the external Parser interface
class FASTSAX_DLLPUBLIC FastSaxParser
    : public ::cppu::WeakImplHelper2<
                ::com::sun::star::xml::sax::XFastParser,
                ::com::sun::star::lang::XServiceInfo >
{
    FastSaxParserImpl* mpImpl;

public:
    FastSaxParser();
    virtual ~FastSaxParser();

    // XFastParser
    virtual void SAL_CALL parseStream( const ::com::sun::star::xml::sax::InputSource& aInputSource ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFastDocumentHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastDocumentHandler >& Handler ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTokenHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler >& Handler ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL registerNamespace( const OUString& NamespaceURL, sal_Int32 NamespaceToken ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getNamespaceURL( const OUString& rPrefix ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setErrorHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XErrorHandler >& Handler ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setEntityResolver( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XEntityResolver >& Resolver ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLocale( const ::com::sun::star::lang::Locale& rLocale ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    bool hasNamespaceURL( const OUString& rPrefix ) const;
};

}

#endif // _SAX_FASTPARSER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
