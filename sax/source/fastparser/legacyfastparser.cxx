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
#include <comphelper/attributelist.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ref.hxx>
#include <sax/fastparser.hxx>
#include <unordered_map>

using namespace std;
using namespace ::cppu;
using namespace css;
using namespace uno;
using namespace lang;
using namespace xml::sax;
using namespace io;

namespace {


class SaxLegacyFastParser : public WeakImplHelper< XServiceInfo, XParser >
{
public:
    SaxLegacyFastParser();
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

class CallbackTokenHandler : public cppu::WeakImplHelper< XFastTokenHandler >
{
public:
    virtual sal_Int32 SAL_CALL getTokenFromUTF8( const Sequence<sal_Int8>& )
        throw (RuntimeException, exception) override
    {
        return FastToken::DONTKNOW;
    }
    virtual Sequence< sal_Int8 > SAL_CALL getUTF8Identifier( sal_Int32 )
        throw (RuntimeException, exception) override
    {
        return Sequence<sal_Int8>();
    }
};

class CallbackDocumentHandler : public WeakImplHelper< XFastDocumentHandler >
{
private:
    typedef std::unordered_map< OUString, OUString,
        OUStringHash > NamespaceMap;
    Reference< XDocumentHandler > m_xDocumentHandler;
    NamespaceMap m_aNamespaceMap;
    const OUString& getNamespacePrefix( const OUString& rNamespaceURL );
    const OUString aEmptyNS;
    const OUString aNamespaceDecl;
public:
    CallbackDocumentHandler( Reference< XDocumentHandler > xDocumentHandler ) : aEmptyNS(""), aNamespaceDecl("xmlns")
    { m_xDocumentHandler.set( xDocumentHandler ); }

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

const OUString& CallbackDocumentHandler::getNamespacePrefix( const OUString& rNamespaceURL )
{
    NamespaceMap::iterator aIter( m_aNamespaceMap.find( rNamespaceURL ) );
    if( aIter != m_aNamespaceMap.end() )
        return (*aIter).second;
    else
        return aEmptyNS;
}

void SAL_CALL CallbackDocumentHandler::startDocument()
        throw (SAXException, RuntimeException, exception)
{
    m_aNamespaceMap.clear();
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
        Sequence< xml::Attribute > unknownAttribs = Attribs->getUnknownAttributes();
        sal_uInt16 len = unknownAttribs.getLength();

        for (sal_uInt16 i = 0; i < len; i++)
        {
            OUString& rAttrNamespaceURL = unknownAttribs[i].NamespaceURL;
            if (rAttrNamespaceURL == aNamespaceDecl)
            {
                OUString& rAttrValue = unknownAttribs[i].Value;
                OUString& rAttrName = unknownAttribs[i].Name;
                rAttrList->AddAttribute( aNamespaceDecl + ":" + rAttrName, "CDATA", rAttrValue);
                m_aNamespaceMap[rAttrValue] = rAttrName;
            }
        }

        elementName = Name;
        if ( !Namespace.isEmpty() )
        {
            const OUString& rNamespacePrefix = getNamespacePrefix(Namespace);
            if ( !rNamespacePrefix.isEmpty() )
                elementName =  rNamespacePrefix + ":" + Name;
            else
                rAttrList->AddAttribute( aNamespaceDecl, "CDATA", Namespace);
        }

        for (sal_uInt16 i = 0; i < len; i++)
        {
            OUString& rAttrValue = unknownAttribs[i].Value;
            OUString sAttrName = unknownAttribs[i].Name;
            OUString& rAttrNamespaceURL = unknownAttribs[i].NamespaceURL;

            if (rAttrNamespaceURL != aNamespaceDecl)
            {
                if ( !rAttrNamespaceURL.isEmpty() )
                {
                    const OUString& rAttrNamespacePrefix = getNamespacePrefix( rAttrNamespaceURL );
                    if ( !rAttrNamespacePrefix.isEmpty() )
                        sAttrName = rAttrNamespacePrefix + ":" + sAttrName;
                }
                rAttrList->AddAttribute( sAttrName, "CDATA", rAttrValue );
            }
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
        OUString elementName = Name;
        if ( !Namespace.isEmpty() )
        {
            const OUString& rNamespacePrefix = getNamespacePrefix( Namespace );
            if ( !rNamespacePrefix.isEmpty() )
                elementName =  rNamespacePrefix + ":" + Name;
        }
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

SaxLegacyFastParser::SaxLegacyFastParser( )
{
    m_xParser = FastParser::create(
        ::comphelper::getProcessComponentContext() );
    m_xParser->setTokenHandler( new CallbackTokenHandler() );
}

void SaxLegacyFastParser::parseStream( const InputSource& structSource )
        throw ( SAXException,
                IOException,
                RuntimeException, exception)
{
    m_xParser->setFastDocumentHandler( new CallbackDocumentHandler( m_xDocumentHandler.get() ) );
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
