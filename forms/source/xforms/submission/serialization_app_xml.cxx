/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "serialization.hxx"
#include "serialization_app_xml.hxx"

#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/xml/xpath/XPathObjectType.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>

#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>

#include <boost/scoped_ptr.hpp>
#include <limits>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::xml::dom::DocumentBuilder;
using ::com::sun::star::xml::dom::XNode;
using ::com::sun::star::xml::dom::XDocument;
using ::com::sun::star::xml::sax::XSAXSerializable;
using ::com::sun::star::beans::StringPair;
using ::com::sun::star::io::XActiveDataSource;
using ::com::sun::star::xml::dom::NodeType_DOCUMENT_NODE;
using ::com::sun::star::xml::dom::NodeType_ELEMENT_NODE;
using ::com::sun::star::xml::dom::XDocumentBuilder;
using ::com::sun::star::xml::sax::Writer;
using ::com::sun::star::xml::sax::XDocumentHandler;

CSerializationAppXML::CSerializationAppXML()
    : m_xBuffer(CSS::io::Pipe::create(comphelper::getProcessComponentContext()))
{
}

Reference< CSS::io::XInputStream >
CSerializationAppXML::getInputStream()
{
    
    
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
            
            Reference< XNode > xNode = rNode;
            if ( xNode->getNodeType() == NodeType_DOCUMENT_NODE )
            {
                Reference< XDocument > const xDoc( xNode, UNO_QUERY_THROW );
                xNode.set( xDoc->getDocumentElement(), UNO_QUERY_THROW );
            }
            ENSURE_OR_RETURN_VOID( xNode->getNodeType() == NodeType_ELEMENT_NODE,
                "CSerializationAppXML::serialize_node: invalid node type!" );

            
            Reference< XDocumentBuilder > const xDocBuilder = DocumentBuilder::create( comphelper::getProcessComponentContext() );
            Reference< XDocument > const xDocument( xDocBuilder->newDocument(), UNO_SET_THROW );

            
            Reference< XNode > const xImportedNode( xDocument->importNode( xNode, true ), UNO_SET_THROW );
            xDocument->appendChild( xImportedNode );

            
            xSerializer.set( xDocument, UNO_QUERY );
        }

        ENSURE_OR_RETURN_VOID( xSerializer.is(),
            "CSerializationAppXML::serialize_node: no serialization access to the node/document!" );

        
        Reference< CSS::xml::sax::XWriter > const xSaxWriter = Writer::create( comphelper::getProcessComponentContext() );
        xSaxWriter->setOutputStream( Reference< CSS::io::XOutputStream >( m_xBuffer, UNO_QUERY_THROW) );

        
        xSerializer->serialize( Reference< XDocumentHandler >(xSaxWriter, UNO_QUERY_THROW), Sequence< StringPair >() );
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
