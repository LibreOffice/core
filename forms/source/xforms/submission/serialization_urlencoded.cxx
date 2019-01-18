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


#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/xml/xpath/XPathObjectType.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XText.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <rtl/character.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <comphelper/processfactory.hxx>

#include <stdio.h>

#include "serialization_urlencoded.hxx"

using namespace css::uno;
using namespace css::io;
using namespace css::xml::xpath;
using namespace css::xml::dom;

CSerializationURLEncoded::CSerializationURLEncoded()
    : m_aPipe(Pipe::create(comphelper::getProcessComponentContext()))
{
}


/*
 rfc2396
 reserved    = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" |
                    "$" | ","
 mark        = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"
 unreserved = alphanum | mark
*/
bool CSerializationURLEncoded::is_unreserved(sal_Char c)
{
    if (rtl::isAsciiAlphanumeric(static_cast<unsigned char>(c)))
        return true;
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
            return true;
    }
    return false;
}
void  CSerializationURLEncoded::encode_and_append(const OUString& aString, OStringBuffer& aBuffer)
{
    OString utf8String = OUStringToOString(aString, RTL_TEXTENCODING_UTF8);
    const sal_uInt8 *pString = reinterpret_cast< const sal_uInt8 * >( utf8String.getStr() );
    sal_Char tmpChar[4];

    while( *pString != 0)
    {
        if( *pString < 0x80 )
        {
            if ( is_unreserved(*pString) ) {
                aBuffer.append(char(*pString));
            } else if (*pString == 0x20) {
                aBuffer.append('+');
            } else if (*pString == 0x0d && *(pString+1) == 0x0a) {
                aBuffer.append("%0D%0A");
                pString++;
            } else if (*pString == 0x0a) {
                aBuffer.append("%0D%0A");
            } else {
                snprintf(tmpChar, 4, "%%%X", *pString % 0x100);
                aBuffer.append(tmpChar);
            }
        } else {
            snprintf(tmpChar, 4, "%%%X", *pString % 0x100);
            aBuffer.append(tmpChar);
            while (*pString >= 0x80) {
                // continuation...
                pString++;
                snprintf(tmpChar, 4, "%%%X", *pString % 0x100);
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
        OUString  aName = aNode->getNodeName();
        // find any text children
        OUStringBuffer aValue;
        Reference< XText > aText;
        for(sal_Int32 i=0; i < aChildList->getLength(); i++)
        {
            aChild = aChildList->item(i);
            if (aChild->getNodeType() == NodeType_TEXT_NODE)
            {
                aText.set(aChild, UNO_QUERY);
                aValue.append(aText->getData());
            }
        }

        // found anything?
        if (!aValue.isEmpty())
        {
            OUString aUnencValue = aValue.makeStringAndClear();
            OStringBuffer aEncodedBuffer;
            encode_and_append(aName, aEncodedBuffer);
            aEncodedBuffer.append("=");
            encode_and_append(aUnencValue, aEncodedBuffer);
            aEncodedBuffer.append("&");
            sal_Int8 const *pData = reinterpret_cast<sal_Int8 const *>(aEncodedBuffer.getStr());
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

    css::uno::Reference< css::xml::dom::XNode > cur = m_aFragment->getFirstChild();
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
