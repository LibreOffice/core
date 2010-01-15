/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: odfemitter.cxx,v $
 *
 * $Revision: 1.2 $
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
