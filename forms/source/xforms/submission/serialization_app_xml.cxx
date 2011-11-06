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
#include "precompiled_forms.hxx"

#include "serialization.hxx"
#include "serialization_app_xml.hxx"

/** === begin UNO includes === **/
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
    :m_aContext( ::comphelper::getProcessServiceFactory() )
{
    m_aContext.createComponent( "com.sun.star.io.Pipe", m_xBuffer );
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
                m_aContext.createComponent( "com.sun.star.xml.dom.DocumentBuilder" ), UNO_QUERY_THROW );
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
            m_aContext.createComponent( "com.sun.star.xml.sax.Writer" ), UNO_QUERY_THROW );
        Reference< XActiveDataSource > const xDataSource( xSaxWriter, UNO_QUERY_THROW );
        xDataSource->setOutputStream( m_xBuffer );

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
