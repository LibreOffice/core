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


#include "serialization.hxx"
#include "serialization_app_xml.hxx"

#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/xml/xpath/XPathObjectType.hpp>

#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>

#include <libxml/tree.h>

#include <limits>

CSerializationAppXML::CSerializationAppXML()
    : m_aFactory(comphelper::getProcessServiceFactory())
    , m_aPipe(CSS::io::Pipe::create(comphelper::getProcessComponentContext()))
{}

CSS::uno::Reference< CSS::io::XInputStream >
CSerializationAppXML::getInputStream()
{
    // The pipes output is provided through it's
    // XOutputStream interface aspect
    return CSS::uno::Reference< CSS::io::XInputStream >(m_aPipe, CSS::uno::UNO_QUERY);
}

void
CSerializationAppXML::serialize_node(const CSS::uno::Reference< CSS::xml::dom::XNode >& rNode)
{
    CSS::uno::Reference< CSS::xml::dom::XNode > aNode = rNode;
    if (aNode->getNodeType() == CSS::xml::dom::NodeType_DOCUMENT_NODE)
    {
        CSS::uno::Reference< CSS::xml::dom::XDocument > aDoc(rNode, CSS::uno::UNO_QUERY_THROW);
        aNode = CSS::uno::Reference< CSS::xml::dom::XNode >(aDoc->getDocumentElement(), CSS::uno::UNO_QUERY_THROW);
    }
    ENSURE_OR_RETURN_VOID( aNode->getNodeType() == CSS::xml::dom::NodeType_ELEMENT_NODE,
        "CSerializationAppXML::serialize_node: invalid node type!" );

    // clone the node to a new document and serialize that document
    CSS::uno::Reference< CSS::lang::XUnoTunnel > xTunnel( aNode, CSS::uno::UNO_QUERY );
    ENSURE_OR_RETURN_VOID( xTunnel.is(), "CSerializationAppXML::serialize_node: unknown implementation, cannot serialize!" );

    xmlNodePtr aNodePtr = reinterpret_cast< xmlNodePtr >( xTunnel->getSomething(CSS::uno::Sequence< sal_Int8 >()) );
    ENSURE_OR_RETURN_VOID( aNodePtr != NULL, "CSerializationAppXML::serialize_node: unable to obtain the xmlNodePtr!" );

    xmlDocPtr aDocPtr = xmlNewDoc((xmlChar*)"1.0");
    ENSURE_OR_RETURN_VOID( aDocPtr != NULL, "CSerializationAppXML::serialize_node: unable to create a temporary doc!" );

    xmlNodePtr aDocNodePtr = xmlDocCopyNode(aNodePtr, aDocPtr, 1);
    if (aDocNodePtr != NULL)
    {
        xmlAddChild( (xmlNodePtr)aDocPtr, aDocNodePtr );

        xmlChar *buffer = NULL;
        int size = 0;
        xmlDocDumpMemory( aDocPtr, &buffer, &size );

        if ( size > ::std::numeric_limits< sal_Int32 >::max() )
        {
            OSL_ENSURE( false, "CSerializationAppXML::serialize_node: document too large, doesn't fit into a UNO sequence!" );
            size = ::std::numeric_limits< sal_Int32 >::max();
        }

        // write the xml into the pipe through it's XOutputStream interface
        m_aPipe->writeBytes(CSS::uno::Sequence< sal_Int8 >((sal_Int8*)buffer, size));
        xmlFree(buffer);
    }
    xmlFreeDoc( aDocPtr );
}

void
CSerializationAppXML::serialize()
{
    if (!m_aFragment.is()) return;

    CSS::uno::Reference< CSS::xml::dom::XNode > cur = m_aFragment->getFirstChild();
    while (cur.is())
    {
        serialize_node(cur);
        cur = cur->getNextSibling();
    }
    m_aPipe->closeOutput();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
