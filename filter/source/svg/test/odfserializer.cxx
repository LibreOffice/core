/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *    OpenOffice.org - a multi-platform office productivity suite
 *
 *    Author:
 *      Fridrich Strba  <fridrich.strba@bluewin.ch>
 *      Thorsten Behrens <tbehrens@novell.com>
 *
 *      Copyright (C) 2008, Novell Inc.
 *      Parts copyright 2005 by Sun Microsystems, Inc.
 *
 *   The Contents of this file are made available subject to
 *   the terms of GNU Lesser General Public License Version 3.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include "odfserializer.hxx"
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <boost/noncopyable.hpp>

using namespace ::com::sun::star;

namespace svgi
{

typedef ::cppu::WeakComponentImplHelper1<
    com::sun::star::xml::sax::XDocumentHandler> ODFSerializerBase;

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

    virtual void SAL_CALL startDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException);
    virtual void SAL_CALL endDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException);
    virtual void SAL_CALL startElement( const ::rtl::OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs ) throw (xml::sax::SAXException, uno::RuntimeException);
    virtual void SAL_CALL endElement( const ::rtl::OUString& aName ) throw (xml::sax::SAXException, uno::RuntimeException);
    virtual void SAL_CALL characters( const ::rtl::OUString& aChars ) throw (xml::sax::SAXException, uno::RuntimeException);
    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces ) throw (xml::sax::SAXException, uno::RuntimeException);
    virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData ) throw (xml::sax::SAXException, uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator( const uno::Reference< xml::sax::XLocator >& xLocator ) throw (xml::sax::SAXException, uno::RuntimeException);

private:
    uno::Reference<io::XOutputStream> m_xOutStream;
    uno::Sequence<sal_Int8>           m_aLineFeed;
    uno::Sequence<sal_Int8>           m_aBuf;
};

void SAL_CALL ODFSerializer::startDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException)
{
    OSL_PRECOND(m_xOutStream.is(), "ODFSerializer(): invalid output stream");

    rtl::OUStringBuffer aElement;
    aElement.appendAscii("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    characters(aElement.makeStringAndClear());
}

void SAL_CALL ODFSerializer::endDocument() throw (xml::sax::SAXException, uno::RuntimeException)
{}

void SAL_CALL ODFSerializer::startElement( const ::rtl::OUString& aName,
                                           const uno::Reference< xml::sax::XAttributeList >& xAttribs ) throw (xml::sax::SAXException, uno::RuntimeException)
{
    rtl::OUStringBuffer aElement;
    aElement.appendAscii("<");
    aElement.append(aName);
    aElement.appendAscii(" ");

    const sal_Int16 nLen=xAttribs->getLength();
    for( sal_Int16 i=0; i<nLen; ++i )
    {
        rtl::OUStringBuffer aAttribute;
        aElement.append(xAttribs->getNameByIndex(i));
        aElement.appendAscii("=\"");
        aElement.append(xAttribs->getValueByIndex(i));
        aElement.appendAscii("\" ");
    }

    aElement.appendAscii(">");
    characters(aElement.makeStringAndClear());
}

void SAL_CALL ODFSerializer::endElement( const ::rtl::OUString& aName ) throw (xml::sax::SAXException, uno::RuntimeException)
{
    rtl::OUStringBuffer aElement;
    aElement.appendAscii("</");
    aElement.append(aName);
    aElement.appendAscii(">");
    characters(aElement.makeStringAndClear());
}

void SAL_CALL ODFSerializer::characters( const ::rtl::OUString& aChars ) throw (xml::sax::SAXException, uno::RuntimeException)
{
    const rtl::OString aStr = rtl::OUStringToOString(aChars,
                                                     RTL_TEXTENCODING_UTF8);
    const sal_Int32 nLen( aStr.getLength() );
    m_aBuf.realloc( nLen );
    const sal_Char* pStr = aStr.getStr();
    std::copy(pStr,pStr+nLen,m_aBuf.getArray());

    m_xOutStream->writeBytes(m_aBuf);
    // TODO(F1): Make pretty printing configurable
    m_xOutStream->writeBytes(m_aLineFeed);
}

void SAL_CALL ODFSerializer::ignorableWhitespace( const ::rtl::OUString& aWhitespaces ) throw (xml::sax::SAXException, uno::RuntimeException)
{
    // TODO(F1): Make pretty printing configurable
    characters(aWhitespaces);
}

void SAL_CALL ODFSerializer::processingInstruction( const ::rtl::OUString&,
                                                    const ::rtl::OUString& ) throw (xml::sax::SAXException, uno::RuntimeException)
{}

void SAL_CALL ODFSerializer::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& ) throw (xml::sax::SAXException, uno::RuntimeException)
{}

uno::Reference< xml::sax::XDocumentHandler> createSerializer(const uno::Reference<io::XOutputStream>& xOut )
{
    return uno::Reference<xml::sax::XDocumentHandler>(new ODFSerializer(xOut));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
