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

#ifndef _SAX_FASTPARSER_HXX_
#define _SAX_FASTPARSER_HXX_

#include <vector>
#include <stack>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <rtl/ref.hxx>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase2.hxx>

#include <expat.h>
#include "xml2utf.hxx"

#include <sax/fastattribs.hxx>

#define PARSER_IMPLEMENTATION_NAME "com.sun.star.comp.extensions.xml.sax.FastParser"
#define PARSER_SERVICE_NAME        "com.sun.star.xml.sax.FastParser"

namespace sax_fastparser {

class FastLocatorImpl;
struct NamespaceDefine;
struct SaxContextImpl;

typedef ::boost::shared_ptr< SaxContextImpl > SaxContextImplPtr;
typedef ::boost::shared_ptr< NamespaceDefine > NamespaceDefineRef;

typedef ::boost::unordered_map< OUString, sal_Int32,
        OUStringHash, ::std::equal_to< OUString > > NamespaceMap;

// --------------------------------------------------------------------

struct ParserData
{
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastDocumentHandler > mxDocumentHandler;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler >    mxTokenHandler;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XErrorHandler >        mxErrorHandler;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XEntityResolver >      mxEntityResolver;
    ::com::sun::star::lang::Locale          maLocale;

    ParserData();
    ~ParserData();
};

// --------------------------------------------------------------------

// Entity binds all information needed for a single file
struct Entity : public ParserData
{
    ::com::sun::star::xml::sax::InputSource maStructSource;
    XML_Parser                              mpParser;
    ::sax_expatwrap::XMLFile2UTFConverter   maConverter;
    ::rtl::Reference< FastAttributeList >   mxAttributes;

    // Exceptions cannot be thrown through the C-XmlParser (possible resource leaks),
    // therefore the exception must be saved somewhere.
    ::com::sun::star::uno::Any              maSavedException;

    ::std::stack< SaxContextImplPtr >       maContextStack;
    ::std::vector< NamespaceDefineRef >     maNamespaceDefines;

    explicit Entity( const ParserData& rData );
    ~Entity();
};

// --------------------------------------------------------------------

// This class implements the external Parser interface
class FastSaxParser : public ::cppu::WeakImplHelper2< ::com::sun::star::xml::sax::XFastParser, ::com::sun::star::lang::XServiceInfo >
{
public:
    FastSaxParser();
    virtual ~FastSaxParser();

    // The implementation details
    static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(void);

    // XFastParser
    virtual void SAL_CALL parseStream( const ::com::sun::star::xml::sax::InputSource& aInputSource ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFastDocumentHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastDocumentHandler >& Handler ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTokenHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler >& Handler ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL registerNamespace( const OUString& NamespaceURL, sal_Int32 NamespaceToken ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getNamespaceURL( const OUString& rPrefix ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setErrorHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XErrorHandler >& Handler ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setEntityResolver( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XEntityResolver >& Resolver ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLocale( const ::com::sun::star::lang::Locale& rLocale ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    // called by the C callbacks of the expat parser
    void callbackStartElement( const XML_Char* name, const XML_Char** atts );
    void callbackEndElement( const XML_Char* name );
    void callbackCharacters( const XML_Char* s, int nLen );
    int callbackExternalEntityRef( XML_Parser parser, const XML_Char *openEntityNames, const XML_Char *base, const XML_Char *systemId, const XML_Char *publicId);
    void callbackEntityDecl(const XML_Char *entityName, int is_parameter_entity,
            const XML_Char *value, int value_length, const XML_Char *base,
            const XML_Char *systemId, const XML_Char *publicId,
            const XML_Char *notationName);

    inline void pushEntity( const Entity& rEntity ) { maEntities.push( rEntity ); }
    inline void popEntity()                         { maEntities.pop(); }
    Entity& getEntity()                             { return maEntities.top(); }

private:
    void parse();

    sal_Int32 GetToken( const OString& rToken );
    sal_Int32 GetToken( const sal_Char* pToken, sal_Int32 nTokenLen = 0 );
    sal_Int32 GetTokenWithPrefix( const OString& rPrefix, const OString& rName ) throw (::com::sun::star::xml::sax::SAXException);
    sal_Int32 GetTokenWithPrefix( const sal_Char*pPrefix, int nPrefixLen, const sal_Char* pName, int nNameLen ) throw (::com::sun::star::xml::sax::SAXException);
    OUString GetNamespaceURL( const OString& rPrefix ) throw (::com::sun::star::xml::sax::SAXException);
    OUString GetNamespaceURL( const sal_Char*pPrefix, int nPrefixLen ) throw (::com::sun::star::xml::sax::SAXException);
    sal_Int32 GetNamespaceToken( const OUString& rNamespaceURL );
    sal_Int32 GetTokenWithNamespaceURL( const OUString& rNamespaceURL, const sal_Char* pName, int nNameLen );
    void DefineNamespace( const OString& rPrefix, const sal_Char* pNamespaceURL );
    sal_Int32 CreateCustomToken( const sal_Char* pToken, int len = 0 );

    void pushContext();
    void popContext();

    void splitName( const XML_Char *pwName, const XML_Char *&rpPrefix, sal_Int32 &rPrefixLen, const XML_Char *&rpName, sal_Int32 &rNameLen );

private:
    ::osl::Mutex maMutex;

    ::rtl::Reference< FastLocatorImpl >     mxDocumentLocator;
    NamespaceMap                            maNamespaceMap;

    ParserData maData;                      /// Cached parser configuration for next call of parseStream().
    ::std::stack< Entity > maEntities;      /// Entity stack for each call of parseStream().
};

}

#endif // _SAX_FASTPARSER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
