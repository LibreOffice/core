/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/xml/xpath/XPathObjectType.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XText.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <stdio.h>

#include "serialization_urlencoded.hxx"

using namespace CSS::uno;
using namespace CSS::io;
using namespace CSS::xml::xpath;
using namespace CSS::xml::dom;

CSerializationURLEncoded::CSerializationURLEncoded()
    : m_aFactory(comphelper::getProcessServiceFactory())
    , m_aPipe(Pipe::create(comphelper::getProcessComponentContext()), UNO_QUERY_THROW)
{
}


/*
 rfc2396
 reserved    = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" |
                    "$" | ","
 mark        = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"
 unreserved = alphanum | mark
*/
sal_Bool CSerializationURLEncoded::is_unreserved(sal_Char c)
{
    if (comphelper::string::isalnumAscii(c))
        return sal_True;
    switch (c) {
        case '-':
        case '_':
        case '.':
        case '!':
        case '~':
        case '*':
        case '\'':
        case '(':
        case ')':
            return sal_True;
    }
    return sal_False;
}
void  CSerializationURLEncoded::encode_and_append(const ::rtl::OUString& aString, ::rtl::OStringBuffer& aBuffer)
{
    ::rtl::OString utf8String = OUStringToOString(aString, RTL_TEXTENCODING_UTF8);
    const sal_uInt8 *pString = reinterpret_cast< const sal_uInt8 * >( utf8String.getStr() );
    sal_Char tmpChar[4]; tmpChar[3] = 0;

    while( *pString != 0)
    {
        if( *pString < 0x80 )
        {
            if ( is_unreserved(*pString) ) {
                aBuffer.append(*pString);
            } else if (*pString == 0x20) {
                aBuffer.append('+');
            } else if (*pString == 0x0d && *(pString+1) == 0x0a) {
                aBuffer.append("%0D%0A");
                pString++;
            } else if (*pString == 0x0a) {
                aBuffer.append("%0D%0A");
            } else {
                snprintf(tmpChar, 3, "%%%X", *pString % 0x100);
                aBuffer.append(tmpChar);
            }
        } else {
            snprintf(tmpChar, 3, "%%%X", *pString % 0x100);
            aBuffer.append(tmpChar);
            while (*pString >= 0x80) {
                // continuation...
                pString++;
                snprintf(tmpChar, 3, "%%%X", *pString % 0x100);
                aBuffer.append(tmpChar);
            }
        }
        pString++;
    }
}

void CSerializationURLEncoded::serialize_node(const Reference< XNode >& aNode)
{
    // serialize recursive
    // every element node E that has a text child T will be serialized in document order
    // <E1>T1<E2>T2</E2></E1><E3>T3</E3> -> E1=T2&E2=T2&E3=T3 (En := local name)

    // this node
    Reference< XNodeList > aChildList = aNode->getChildNodes();
    Reference< XNode > aChild;
    // is this an element node?
    if (aNode->getNodeType() == NodeType_ELEMENT_NODE)
    {
        ::rtl::OUString  aName = aNode->getNodeName();
        // find any text children
        ::rtl::OUStringBuffer aValue;
        Reference< XText > aText;
        for(sal_Int32 i=0; i < aChildList->getLength(); i++)
        {
            aChild = aChildList->item(i);
            if (aChild->getNodeType() == NodeType_TEXT_NODE)
            {
                aText = Reference< XText >(aChild, UNO_QUERY);
                aValue.append(aText->getData());
            }
        }

        // found anything?
        if (aValue.getLength() > 0)
        {
            ::rtl::OUString aUnencValue = aValue.makeStringAndClear();
            ::rtl::OStringBuffer aEncodedBuffer;
            encode_and_append(aName, aEncodedBuffer);
            aEncodedBuffer.append("=");
            encode_and_append(aUnencValue, aEncodedBuffer);
            aEncodedBuffer.append("&");
            sal_Int8 *pData = (sal_Int8*)aEncodedBuffer.getStr();
            Sequence< sal_Int8 > sData(pData, aEncodedBuffer.getLength());
            m_aPipe->writeBytes(sData);
        }
    }

    // element children...
    for(sal_Int32 i=0; i < aChildList->getLength(); i++)
    {
        aChild = aChildList->item(i);
        // if this is an element node, it might be a candidate for serialization
        if (aChild.is() && aChild->getNodeType() == NodeType_ELEMENT_NODE)
            serialize_node(aChild);
    }
}

void CSerializationURLEncoded::serialize()
{

    // output stream to the pipe buffer
    Reference< XOutputStream > out(m_aPipe, UNO_QUERY);

    CSS::uno::Reference< CSS::xml::dom::XNode > cur = m_aFragment->getFirstChild();
    while (cur.is())
    {
        serialize_node(cur);
        cur = cur->getNextSibling();
    }
    m_aPipe->closeOutput();
}

Reference< XInputStream > CSerializationURLEncoded::getInputStream()
{
    return Reference< XInputStream >(m_aPipe, UNO_QUERY);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
