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


#include "odfemitter.hxx"

#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
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

    virtual void beginTag( const char* pTag, const PropertyMap& rProperties ) override;
    virtual void write( const OUString& rString ) override;
    virtual void endTag( const char* pTag ) override;
};

OdfEmitter::OdfEmitter( const uno::Reference<io::XOutputStream>& xOutput ) :
    m_xOutput( xOutput ),
    m_aLineFeed(1),
    m_aBuf()
{
    OSL_PRECOND(m_xOutput.is(), "OdfEmitter(): invalid output stream");
    m_aLineFeed[0] = '\n';

    OUStringBuffer aElement;
    aElement.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    write(aElement.makeStringAndClear());
}

void OdfEmitter::beginTag( const char* pTag, const PropertyMap& rProperties )
{
    OSL_PRECOND(pTag,"Invalid tag string");

    OUStringBuffer aElement;
    aElement.append("<");
    aElement.appendAscii(pTag);
    aElement.append(" ");

    std::vector<OUString>        aAttributes;
    PropertyMap::const_iterator       aCurr(rProperties.begin());
    const PropertyMap::const_iterator aEnd(rProperties.end());
    while( aCurr != aEnd )
    {
        OUStringBuffer aAttribute;
        aAttribute.append(aCurr->first);
        aAttribute.append("=\"");
        aAttribute.append(aCurr->second);
        aAttribute.append("\" ");
        aAttributes.push_back(aAttribute.makeStringAndClear());
        ++aCurr;
    }

    // since the hash map's sorting is undefined (and varies across
    // platforms, and even between different compile-time settings),
    // sort the attributes.
    std::sort(aAttributes.begin(), aAttributes.end());
    std::for_each(aAttributes.begin(),
                  aAttributes.end(),
                  boost::bind( (OUStringBuffer& (OUStringBuffer::*)(const OUString&))
                               (&OUStringBuffer::append),
                               boost::ref(aElement),
                               _1 ));
    aElement.append(">");

    write(aElement.makeStringAndClear());
}

void OdfEmitter::write( const OUString& rText )
{
    const OString aStr = OUStringToOString(rText,RTL_TEXTENCODING_UTF8);
    const sal_Int32 nLen( aStr.getLength() );
    m_aBuf.realloc( nLen );
    const sal_Char* pStr = aStr.getStr();
    std::copy(pStr,pStr+nLen,m_aBuf.getArray());

    m_xOutput->writeBytes(m_aBuf);
    m_xOutput->writeBytes(m_aLineFeed);
}

void OdfEmitter::endTag( const char* pTag )
{
    OUStringBuffer aElement;
    aElement.append("</");
    aElement.appendAscii(pTag);
    aElement.append(">");
    write(aElement.makeStringAndClear());
}

XmlEmitterSharedPtr createOdfEmitter( const uno::Reference<io::XOutputStream>& xOut )
{
    return XmlEmitterSharedPtr(new OdfEmitter(xOut));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
