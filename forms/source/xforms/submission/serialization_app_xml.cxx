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

/** === begin UNO includes === **/
#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/xml/xpath/XPathObjectType.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>

#include <boost/scoped_ptr.hpp>
#include <limits>

/** === begin UNO using === **/
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::xml::dom::XNode;
using ::com::sun::star::xml::dom::XDocument;
using ::com::sun::star::xml::sax::XSAXSerializable;
using ::com::sun::star::beans::StringPair;
using ::com::sun::star::io::XActiveDataSource;
using ::com::sun::star::xml::dom::NodeType_DOCUMENT_NODE;
using ::com::sun::star::xml::dom::NodeType_ELEMENT_NODE;
using ::com::sun::star::xml::dom::XDocumentBuilder;
using ::com::sun::star::xml::sax::XDocumentHandler;
/** === end UNO using === **/

CSerializationAppXML::CSerializationAppXML()
    : m_xFactory(comphelper::getProcessServiceFactory())
    , m_xBuffer(CSS::io::Pipe::create(comphelper::getProcessComponentContext()))
{
}

Reference< CSS::io::XInputStream >
CSerializationAppXML::getInputStream()
{
    // The pipes output is provided through it's
    // XOutputStream interface aspect
    return Reference< CSS::io::XInputStream >(m_xBuffer, UNO_QUERY);
}

void
CSerializationAppXML::serialize_node(const Reference< XNode >& rNode)
{
    try
    {
        Reference< XSAXSerializable > xSerializer( rNode, UNO_QUERY );
        if ( !xSerializer.is() )
        {
            // ensure we have a "real" node
            Reference< XNode > xNode = rNode;
            if ( xNode->getNodeType() == NodeType_DOCUMENT_NODE )
            {
                Reference< XDocument > const xDoc( xNode, UNO_QUERY_THROW );
                xNode.set( xDoc->getDocumentElement(), UNO_QUERY_THROW );
            }
            ENSURE_OR_RETURN_VOID( xNode->getNodeType() == NodeType_ELEMENT_NODE,
                "CSerializationAppXML::serialize_node: invalid node type!" );

            // create a new document
            Reference< XDocumentBuilder > const xDocBuilder(
                m_xFactory->createInstance( "com.sun.star.xml.dom.DocumentBuilder" ), UNO_QUERY_THROW );
            Reference< XDocument > const xDocument( xDocBuilder->newDocument(), UNO_SET_THROW );

            // copy the to-be-serialized node
            Reference< XNode > const xImportedNode( xDocument->importNode( xNode, true ), UNO_SET_THROW );
            xDocument->appendChild( xImportedNode );

            // ask the doc for the serializer
            xSerializer.set( xDocument, UNO_QUERY );
        }

        ENSURE_OR_RETURN_VOID( xSerializer.is(),
            "CSerializationAppXML::serialize_node: no serialization access to the node/document!" );

        // create a SAXWriter to take the serialization events, and connect it to our pipe
        Reference< XDocumentHandler > const xSaxWriter(
            m_xFactory->createInstance( "com.sun.star.xml.sax.Writer" ), UNO_QUERY_THROW );
        Reference< XActiveDataSource > const xDataSource( xSaxWriter, UNO_QUERY_THROW );
        xDataSource->setOutputStream( Reference< CSS::io::XOutputStream >( m_xBuffer, UNO_QUERY_THROW) );

        // do the serialization
        xSerializer->serialize( xSaxWriter, Sequence< StringPair >() );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void
CSerializationAppXML::serialize()
{
    if (!m_aFragment.is()) return;

    Reference< XNode > cur = m_aFragment->getFirstChild();
    while (cur.is())
    {
        serialize_node(cur);
        cur = cur->getNextSibling();
    }
    m_xBuffer->closeOutput();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
