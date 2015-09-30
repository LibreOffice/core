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

#include "odfserializer.hxx"
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <boost/noncopyable.hpp>

using namespace ::com::sun::star;

namespace svgi
{

typedef ::cppu::WeakComponentImplHelper<
    css::xml::sax::XDocumentHandler> ODFSerializerBase;

class ODFSerializer : private cppu::BaseMutex,
                public ODFSerializerBase,
                boost::noncopyable
{
public:
    explicit ODFSerializer(const uno::Reference<io::XOutputStream>& xOut) :
        ODFSerializerBase(m_aMutex),
        m_xOutStream(xOut),
        m_aLineFeed(1),
        m_aBuf()
    {
        m_aLineFeed[0] = '\n';
    }

    virtual void SAL_CALL startDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL endDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL startElement( const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL endElement( const OUString& aName ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL characters( const OUString& aChars ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDocumentLocator( const uno::Reference< xml::sax::XLocator >& xLocator ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    uno::Reference<io::XOutputStream> m_xOutStream;
    uno::Sequence<sal_Int8>           m_aLineFeed;
    uno::Sequence<sal_Int8>           m_aBuf;
};

void SAL_CALL ODFSerializer::startDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    OSL_PRECOND(m_xOutStream.is(), "ODFSerializer(): invalid output stream");

    OUStringBuffer aElement;
    aElement.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    characters(aElement.makeStringAndClear());
}

void SAL_CALL ODFSerializer::endDocument() throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{}

void SAL_CALL ODFSerializer::startElement( const OUString& aName,
                                           const uno::Reference< xml::sax::XAttributeList >& xAttribs ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    OUStringBuffer aElement("<" + aName + " ");

    const sal_Int16 nLen=xAttribs->getLength();
    for( sal_Int16 i=0; i<nLen; ++i )
        aElement.append(xAttribs->getNameByIndex(i) + "=\"" +
                        xAttribs->getValueByIndex(i) + "\" ");

    characters(aElement.makeStringAndClear() + ">");
}

void SAL_CALL ODFSerializer::endElement( const OUString& aName ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    characters("</" + aName + ">");
}

void SAL_CALL ODFSerializer::characters( const OUString& aChars ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    const OString aStr = OUStringToOString(aChars,
                                                     RTL_TEXTENCODING_UTF8);
    const sal_Int32 nLen( aStr.getLength() );
    m_aBuf.realloc( nLen );
    const sal_Char* pStr = aStr.getStr();
    std::copy(pStr,pStr+nLen,m_aBuf.getArray());

    m_xOutStream->writeBytes(m_aBuf);
    // TODO(F1): Make pretty printing configurable
    m_xOutStream->writeBytes(m_aLineFeed);
}

void SAL_CALL ODFSerializer::ignorableWhitespace( const OUString& aWhitespaces ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    // TODO(F1): Make pretty printing configurable
    characters(aWhitespaces);
}

void SAL_CALL ODFSerializer::processingInstruction( const OUString&,
                                                    const OUString& ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{}

void SAL_CALL ODFSerializer::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{}

uno::Reference< xml::sax::XDocumentHandler> createSerializer(const uno::Reference<io::XOutputStream>& xOut )
{
    return uno::Reference<xml::sax::XDocumentHandler>(new ODFSerializer(xOut));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
