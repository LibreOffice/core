/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "odfemitter.hxx"

#include <rtl/ustrbuf.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <boost/bind.hpp>

using namespace com::sun::star;

namespace pdfi
{

class OdfEmitter : public XmlEmitter
{
private:
    uno::Reference<io::XOutputStream> m_xOutput;
    uno::Sequence<sal_Int8>           m_aLineFeed;
    uno::Sequence<sal_Int8>           m_aBuf;

public:
    explicit OdfEmitter( const uno::Reference<io::XOutputStream>& xOutput );

    virtual void beginTag( const char* pTag, const PropertyMap& rProperties );
    virtual void write( const rtl::OUString& rString );
    virtual void endTag( const char* pTag );
};

OdfEmitter::OdfEmitter( const uno::Reference<io::XOutputStream>& xOutput ) :
    m_xOutput( xOutput ),
    m_aLineFeed(1),
    m_aBuf()
{
    OSL_PRECOND(m_xOutput.is(), "OdfEmitter(): invalid output stream");
    m_aLineFeed[0] = '\n';

    rtl::OUStringBuffer aElement;
    aElement.appendAscii("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    write(aElement.makeStringAndClear());
}

void OdfEmitter::beginTag( const char* pTag, const PropertyMap& rProperties )
{
    OSL_PRECOND(pTag,"Invalid tag string");

    rtl::OUStringBuffer aElement;
    aElement.appendAscii("<");
    aElement.appendAscii(pTag);
    aElement.appendAscii(" ");

    std::vector<rtl::OUString>        aAttributes;
    PropertyMap::const_iterator       aCurr(rProperties.begin());
    const PropertyMap::const_iterator aEnd(rProperties.end());
    while( aCurr != aEnd )
    {
        rtl::OUStringBuffer aAttribute;
        aAttribute.append(aCurr->first);
        aAttribute.appendAscii("=\"");
        aAttribute.append(aCurr->second);
        aAttribute.appendAscii("\" ");
        aAttributes.push_back(aAttribute.makeStringAndClear());
        ++aCurr;
    }

    // since the hash map's sorting is undefined (and varies across
    // platforms, and even between different compile-time settings),
    // sort the attributes.
    std::sort(aAttributes.begin(), aAttributes.end());
    std::for_each(aAttributes.begin(),
                  aAttributes.end(),
                  boost::bind( (rtl::OUStringBuffer& (rtl::OUStringBuffer::*)(const rtl::OUString&))
                               (&rtl::OUStringBuffer::append),
                               boost::ref(aElement),
                               _1 ));
    aElement.appendAscii(">");

    write(aElement.makeStringAndClear());
}

void OdfEmitter::write( const rtl::OUString& rText )
{
    const rtl::OString aStr = rtl::OUStringToOString(rText,RTL_TEXTENCODING_UTF8);
    const sal_Int32 nLen( aStr.getLength() );
    m_aBuf.realloc( nLen );
    const sal_Char* pStr = aStr.getStr();
    std::copy(pStr,pStr+nLen,m_aBuf.getArray());

    m_xOutput->writeBytes(m_aBuf);
    m_xOutput->writeBytes(m_aLineFeed);
}

void OdfEmitter::endTag( const char* pTag )
{
    rtl::OUStringBuffer aElement;
    aElement.appendAscii("</");
    aElement.appendAscii(pTag);
    aElement.appendAscii(">");
    write(aElement.makeStringAndClear());
}

XmlEmitterSharedPtr createOdfEmitter( const uno::Reference<io::XOutputStream>& xOut )
{
    return XmlEmitterSharedPtr(new OdfEmitter(xOut));
}

}
