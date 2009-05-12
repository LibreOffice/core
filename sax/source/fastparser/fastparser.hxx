/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SAX_FASTPARSER_HXX_
#define _SAX_FASTPARSER_HXX_

#include <vector>
#include <stack>
#include <hash_map>
#include <boost/shared_ptr.hpp>
#include <rtl/ref.hxx>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase2.hxx>

#ifdef SYSTEM_EXPAT
#include <expat.h>
#else
#include "expat/xmlparse.h"
#endif
#include "xml2utf.hxx"

#include <sax/fastattribs.hxx>

#define PARSER_IMPLEMENTATION_NAME "com.sun.star.comp.extensions.xml.sax.FastParser"
#define PARSER_SERVICE_NAME        "com.sun.star.xml.sax.FastParser"

namespace sax_fastparser
{

    class FastLocatorImpl;
    struct NamespaceDefine;
    struct SaxContextImpl;
    typedef boost::shared_ptr< SaxContextImpl > SaxContextImplPtr;
    typedef ::std::hash_map< ::rtl::OUString, sal_Int32,
        ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString >
    > NamespaceMap;

// --------------------------------------------------------------------

// Entity binds all information neede for a single file
struct Entity
{
    ::com::sun::star::xml::sax::InputSource         maStructSource;
    XML_Parser                                      mpParser;
    sax_expatwrap::XMLFile2UTFConverter             maConverter;
};

// --------------------------------------------------------------------

// This class implements the external Parser interface
class FastSaxParser : public ::cppu::WeakImplHelper2< ::com::sun::star::xml::sax::XFastParser, ::com::sun::star::lang::XServiceInfo >
{
public:
    FastSaxParser();
    ~FastSaxParser();

    // The implementation details
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void);

    // XFastParser
    virtual void SAL_CALL parseStream( const ::com::sun::star::xml::sax::InputSource& aInputSource ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFastDocumentHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastDocumentHandler >& Handler ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTokenHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler >& Handler ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL registerNamespace( const ::rtl::OUString& NamespaceURL, sal_Int32 NamespaceToken ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setErrorHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XErrorHandler >& Handler ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setEntityResolver( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XEntityResolver >& Resolver ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLocale( const ::com::sun::star::lang::Locale& rLocale ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

public:
    // the C-Callbacks for the expat parser
    void static callbackStartElement(void *userData, const XML_Char *name , const XML_Char **atts);
    void static callbackEndElement(void *userData, const XML_Char *name);
    void static callbackCharacters( void *userData , const XML_Char *s , int nLen );
    int static callbackExternalEntityRef( XML_Parser parser, const XML_Char *openEntityNames, const XML_Char *base, const XML_Char *systemId, const XML_Char *publicId);

public:
    void pushEntity( const struct Entity &entity ) { vecEntity.push_back( entity ); }
    void popEntity()                               { vecEntity.pop_back( ); }
    struct Entity &getEntity()                     { return vecEntity.back(); }

private:
    void parse();

    sal_Int32 GetToken( const ::rtl::OString& rToken );
    sal_Int32 GetToken( const sal_Char* pToken, sal_Int32 nTokenLen = 0 );
    sal_Int32 GetTokenWithPrefix( const ::rtl::OString& rPrefix, const ::rtl::OString& rName ) throw (::com::sun::star::xml::sax::SAXException);
    sal_Int32 GetTokenWithPrefix( const sal_Char*pPrefix, int nPrefixLen, const sal_Char* pName, int nNameLen ) throw (::com::sun::star::xml::sax::SAXException);
    ::rtl::OUString GetNamespaceURL( const ::rtl::OString& rPrefix ) throw (::com::sun::star::xml::sax::SAXException);
    ::rtl::OUString GetNamespaceURL( const sal_Char*pPrefix, int nPrefixLen ) throw (::com::sun::star::xml::sax::SAXException);
    sal_Int32 GetNamespaceToken( const ::rtl::OUString& rNamespaceURL );
    sal_Int32 GetTokenWithNamespaceURL( const ::rtl::OUString& rNamespaceURL, const sal_Char* pName, int nNameLen );
    void DefineNamespace( const ::rtl::OString& rPrefix, const sal_Char* pNamespaceURL );
    sal_Int32 CreateCustomToken( const sal_Char* pToken, int len = 0 );

    void pushContext();
    void popContext();

    void splitName( const XML_Char *pwName, const XML_Char *&rpPrefix, sal_Int32 &rPrefixLen, const XML_Char *&rpName, sal_Int32 &rNameLen );

private:
    ::osl::Mutex maMutex;

    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastDocumentHandler > mxDocumentHandler;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler >    mxTokenHandler;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XErrorHandler >        mxErrorHandler;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XEntityResolver >      mxEntityResolver;

    rtl::Reference < FastLocatorImpl >   mxDocumentLocator;
    rtl::Reference < FastAttributeList > mxAttributes;

    // External entity stack
    std::vector< struct Entity > vecEntity;

    // Exception cannot be thrown through the C-XmlParser (possible resource leaks),
    // therefor the maSavedException must be saved somewhere.
    ::com::sun::star::uno::Any maSavedException;
    sal_Bool                   mbExceptionWasThrown;

    ::com::sun::star::lang::Locale maLocale;

    std::stack< SaxContextImplPtr > maContextStack;
    std::vector< boost::shared_ptr< NamespaceDefine > > maNamespaceDefines;
    NamespaceMap maNamespaceMap;
};

}

#endif // _SAX_FASTPARSER_HXX_
