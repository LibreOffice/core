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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/FastParser.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/attributelist.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ref.hxx>
#include <sax/fastparser.hxx>
#include <vector>
#include <o3tl/make_unique.hxx>

using namespace std;
using namespace ::cppu;
using namespace css;
using namespace uno;
using namespace lang;
using namespace xml::sax;
using namespace io;

namespace {

class NamespaceHandler : public WeakImplHelper< XFastNamespaceHandler >
{
private:
    struct NamespaceDefine
    {
        OUString    m_aPrefix;
        OUString    m_aNamespaceURI;

        NamespaceDefine( const OUString& rPrefix, const OUString& rNamespaceURI ) : m_aPrefix( rPrefix ), m_aNamespaceURI( rNamespaceURI ) {}
    };
    vector< unique_ptr< NamespaceDefine > > m_aNamespaceDefines;

public:
    NamespaceHandler();
    void addNSDeclAttributes( rtl::Reference < comphelper::AttributeList >& rAttrList );

    //XFastNamespaceHandler
    virtual void SAL_CALL registerNamespace( const OUString& rNamespacePrefix, const OUString& rNamespaceURI )
                            throw (RuntimeException, exception) override;
    virtual OUString SAL_CALL getNamespaceURI( const OUString& rNamespacePrefix )
                            throw (RuntimeException, exception) override;
};

NamespaceHandler::NamespaceHandler()
{
}

void NamespaceHandler::addNSDeclAttributes( rtl::Reference < comphelper::AttributeList >& rAttrList )
{
    for(const auto& aNamespaceDefine : m_aNamespaceDefines)
    {
        OUString& rPrefix = aNamespaceDefine.get()->m_aPrefix;
        OUString& rNamespaceURI = aNamespaceDefine.get()->m_aNamespaceURI;
        OUString sDecl;
        if ( rPrefix.isEmpty() )
            sDecl = "xmlns";
        else
            sDecl = "xmlns:" + rPrefix;
        rAttrList->AddAttribute( sDecl, "CDATA", rNamespaceURI );
    }
    m_aNamespaceDefines.clear();
}

void NamespaceHandler::registerNamespace( const OUString& rNamespacePrefix, const OUString& rNamespaceURI )
                            throw (RuntimeException, exception)
{
    m_aNamespaceDefines.push_back( o3tl::make_unique<NamespaceDefine>(
                                    rNamespacePrefix, rNamespaceURI) );
}

OUString NamespaceHandler::getNamespaceURI( const OUString&/* rNamespacePrefix */ )
                            throw (RuntimeException, exception)
{
    return OUString();
}

class SaxLegacyFastParser : public WeakImplHelper< XInitialization, XServiceInfo, XParser >
{
private:
    rtl::Reference< NamespaceHandler > m_aNamespaceHandler;
public:
    SaxLegacyFastParser();

// css::lang::XInitialization:
    virtual void SAL_CALL initialize(css::uno::Sequence<css::uno::Any> const& rArguments)
        throw (RuntimeException, Exception, exception) override;

// The SAX-Parser-Interface
    virtual void SAL_CALL parseStream(  const InputSource& structSource)
        throw ( SAXException, IOException, RuntimeException, exception) override;
    virtual void SAL_CALL setDocumentHandler(const Reference< XDocumentHandler > & xHandler)
        throw (RuntimeException, exception) override;
    virtual void SAL_CALL setErrorHandler(const Reference< XErrorHandler > & xHandler)
        throw (RuntimeException, exception) override;
    virtual void SAL_CALL setDTDHandler(const Reference < XDTDHandler > & xHandler)
        throw (RuntimeException, exception) override;
    virtual void SAL_CALL setEntityResolver(const Reference<  XEntityResolver >& xResolver)
        throw (RuntimeException, exception) override;
    virtual void SAL_CALL setLocale( const Locale &locale )
        throw (RuntimeException, exception) override;

// XServiceInfo
    OUString SAL_CALL getImplementationName() throw (exception) override;
    Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (exception) override;
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw (exception) override;

private:
    Reference< XFastParser > m_xParser;
    Reference< XDocumentHandler > m_xDocumentHandler;

};

class CallbackDocumentHandler : public WeakImplHelper< XFastDocumentHandler >
{
private:
    Reference< XDocumentHandler > m_xDocumentHandler;
    rtl::Reference< NamespaceHandler > m_aNamespaceHandler;
public:
    CallbackDocumentHandler( Reference< XDocumentHandler > const & xDocumentHandler, rtl::Reference< NamespaceHandler > const & rNamespaceHandler );

    // XFastDocumentHandler
    virtual void SAL_CALL startDocument() throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL endDocument() throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL setDocumentLocator( const Reference< XLocator >& xLocator ) throw (SAXException, RuntimeException, exception) override;

    // XFastContextHandler
    virtual void SAL_CALL startFastElement( sal_Int32 nElement, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL startUnknownElement( const OUString& Namespace, const OUString& Name, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL endFastElement( sal_Int32 Element ) throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL endUnknownElement( const OUString& Namespace, const OUString& Name ) throw (SAXException, RuntimeException, exception) override;
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException, exception) override;
    virtual Reference< XFastContextHandler > SAL_CALL createUnknownChildContext( const OUString& Namespace, const OUString& Name, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL characters( const OUString& aChars ) throw (SAXException, RuntimeException, exception) override;

};

CallbackDocumentHandler::CallbackDocumentHandler( Reference< XDocumentHandler > const & xDocumentHandler, rtl::Reference< NamespaceHandler > const & rNamespaceHandler )
{
   m_xDocumentHandler.set( xDocumentHandler );
   m_aNamespaceHandler.set( rNamespaceHandler.get() );
}

void SAL_CALL CallbackDocumentHandler::startDocument()
        throw (SAXException, RuntimeException, exception)
{
    if ( m_xDocumentHandler.is() )
        m_xDocumentHandler->startDocument();
}

void SAL_CALL CallbackDocumentHandler::endDocument()
        throw (SAXException, RuntimeException, exception)
{
    if ( m_xDocumentHandler.is() )
        m_xDocumentHandler->endDocument();
}

void SAL_CALL CallbackDocumentHandler::setDocumentLocator( const Reference< XLocator >& xLocator )
        throw (SAXException, RuntimeException, exception)
{
    if ( m_xDocumentHandler.is() )
        m_xDocumentHandler->setDocumentLocator( xLocator );
}

void SAL_CALL CallbackDocumentHandler::startFastElement( sal_Int32/* nElement */, const Reference< XFastAttributeList >&/* Attribs */ )
        throw (SAXException, RuntimeException, exception)
{
}

void SAL_CALL CallbackDocumentHandler::startUnknownElement( const OUString& Namespace, const OUString& Name, const Reference< XFastAttributeList >& Attribs  )
        throw (SAXException, RuntimeException, exception)
{
    if ( m_xDocumentHandler.is() )
    {
        OUString elementName;
        rtl::Reference < comphelper::AttributeList > rAttrList = new comphelper::AttributeList;
        m_aNamespaceHandler->addNSDeclAttributes( rAttrList );
        if ( !Namespace.isEmpty() )
            elementName =  Namespace + ":" + Name;
        else
            elementName = Name;

        Sequence< xml::Attribute > unknownAttribs = Attribs->getUnknownAttributes();
        sal_uInt16 len = unknownAttribs.getLength();
        for (sal_uInt16 i = 0; i < len; i++)
        {
            OUString& rAttrValue = unknownAttribs[i].Value;
            OUString sAttrName = unknownAttribs[i].Name;
            OUString& rAttrNamespaceURL = unknownAttribs[i].NamespaceURL;
            if ( !rAttrNamespaceURL.isEmpty() )
                sAttrName = rAttrNamespaceURL + ":" + sAttrName;

            rAttrList->AddAttribute( sAttrName, "CDATA", rAttrValue );
        }
        m_xDocumentHandler->startElement( elementName, rAttrList.get() );
    }
}

void SAL_CALL CallbackDocumentHandler::endFastElement( sal_Int32/* nElement */)
        throw (SAXException, RuntimeException, exception)
{
}


void SAL_CALL CallbackDocumentHandler::endUnknownElement( const OUString& Namespace, const OUString& Name )
        throw (SAXException, RuntimeException, exception)
{
    if ( m_xDocumentHandler.is() )
    {
        OUString elementName;
        if ( !Namespace.isEmpty() )
            elementName = Namespace + ":" + Name;
        else
            elementName = Name;
        m_xDocumentHandler->endElement( elementName );
    }
}

Reference< XFastContextHandler > SAL_CALL CallbackDocumentHandler::createFastChildContext( sal_Int32/* nElement */, const Reference< XFastAttributeList >&/* Attribs */ )
        throw (SAXException, RuntimeException, exception)
{
    return this;
}


Reference< XFastContextHandler > SAL_CALL CallbackDocumentHandler::createUnknownChildContext( const OUString&/* Namespace */, const OUString&/* Name */, const Reference< XFastAttributeList >&/* Attribs */ )
        throw (SAXException, RuntimeException, exception)
{
    return this;
}

void SAL_CALL CallbackDocumentHandler::characters( const OUString& aChars )
        throw (SAXException, RuntimeException, exception)
{
    if ( m_xDocumentHandler.is() )
        m_xDocumentHandler->characters( aChars );
}

SaxLegacyFastParser::SaxLegacyFastParser( ) : m_aNamespaceHandler( new NamespaceHandler )
{
    m_xParser = FastParser::create(
        ::comphelper::getProcessComponentContext() );
    m_xParser->setNamespaceHandler( m_aNamespaceHandler.get() );
}

void SAL_CALL SaxLegacyFastParser::initialize(Sequence< Any > const& rArguments )
    throw (RuntimeException, Exception, exception)
{
    uno::Reference<lang::XInitialization> const xInit(m_xParser,
                            uno::UNO_QUERY_THROW);
    xInit->initialize( rArguments );
}

void SaxLegacyFastParser::parseStream( const InputSource& structSource )
        throw ( SAXException,
                IOException,
                RuntimeException, exception)
{
    m_xParser->setFastDocumentHandler( new CallbackDocumentHandler( m_xDocumentHandler.get(), m_aNamespaceHandler.get() ) );
    m_xParser->parseStream( structSource );
}

void SaxLegacyFastParser::setDocumentHandler( const Reference< XDocumentHandler > & xHandler )
        throw (RuntimeException, exception)
{
    m_xDocumentHandler = xHandler;
}

void SaxLegacyFastParser::setErrorHandler( const Reference< XErrorHandler > & xHandler )
        throw (RuntimeException, exception)
{
    m_xParser->setErrorHandler( xHandler );
}

void SaxLegacyFastParser::setDTDHandler( const Reference < XDTDHandler > &/* xHandler */ )
        throw (RuntimeException, exception)
{

}

void SaxLegacyFastParser::setEntityResolver( const Reference<  XEntityResolver >& xResolver )
        throw (RuntimeException, exception)
{
    m_xParser->setEntityResolver( xResolver );
}

void SaxLegacyFastParser::setLocale( const Locale &locale )
        throw (RuntimeException, exception)
{
    m_xParser->setLocale( locale );
}

OUString SaxLegacyFastParser::getImplementationName() throw (exception)
{
    return OUString("com.sun.star.comp.extensions.xml.sax.LegacyFastParser");
}

sal_Bool SaxLegacyFastParser::supportsService(const OUString& ServiceName) throw (exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SaxLegacyFastParser::getSupportedServiceNames() throw (exception)
{
    Sequence<OUString> seq { "com.sun.star.xml.sax.LegacyFastParser" };
    return seq;
}

} //namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_extensions_xml_sax_LegacyFastParser_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SaxLegacyFastParser);
}

 /* vim:set shiftwidth=4 softtabstop=4 expandtab: */
