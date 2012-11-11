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


#include <rtl/ustring.hxx>

#include "saxhelper.hxx"
#include "libxml/parserInternals.h"

#ifndef XMLSEC_NO_XSLT
#include "libxslt/xslt.h"
#endif

namespace cssu = com::sun::star::uno;
namespace cssxs = com::sun::star::xml::sax;
namespace cssxcsax = com::sun::star::xml::csax;

/**
 * The return value is NULL terminated. The application has the responsibilty to
 * deallocte the return value.
 */
xmlChar* ous_to_xmlstr( const rtl::OUString& oustr )
{
    rtl::OString ostr = rtl::OUStringToOString( oustr , RTL_TEXTENCODING_UTF8 ) ;
    return xmlStrndup( ( xmlChar* )ostr.getStr(), ( int )ostr.getLength() ) ;
}

/**
 * The return value is NULL terminated. The application has the responsibilty to
 * deallocte the return value.
 */
xmlChar* ous_to_nxmlstr( const rtl::OUString& oustr, int& length )
{
    rtl::OString ostr = rtl::OUStringToOString( oustr , RTL_TEXTENCODING_UTF8 ) ;
    length = ostr.getLength();

    return xmlStrndup( ( xmlChar* )ostr.getStr(), length ) ;
}

/**
 * The return value and the referenced value must be NULL terminated.
 * The application has the responsibilty to deallocte the return value.
 */
const xmlChar** attrlist_to_nxmlstr( const cssu::Sequence< cssxcsax::XMLAttribute >& aAttributes )
{
    xmlChar* attname = NULL ;
    xmlChar* attvalue = NULL ;
    const xmlChar** attrs = NULL ;

    sal_Int32 nLength = aAttributes.getLength();

    if( nLength != 0 )
    {
        attrs = ( const xmlChar** )xmlMalloc( ( nLength * 2 + 2 ) * sizeof( xmlChar* ) ) ;
    }
    else
    {
        return NULL ;
    }

    for( int i = 0 , j = 0 ; j < nLength ; ++j )
    {
        attname = ous_to_xmlstr( aAttributes[j].sName ) ;
        attvalue = ous_to_xmlstr( aAttributes[j].sValue ) ;

        if( attname != NULL && attvalue != NULL )
        {
            attrs[i++] = attname ;
            attrs[i++] = attvalue ;
            attrs[i] = NULL ;
            attrs[i+1] = NULL ;
        }
        else
        {
            if( attname != NULL )
                xmlFree( attname ) ;
            if( attvalue != NULL )
                xmlFree( attvalue ) ;
        }
    }

    return attrs ;
}

/**
 * Constructor
 *
 * In this constructor, a libxml sax parser context is initialized. a libxml
 * default sax handler is initialized with the context.
 */
SAXHelper::SAXHelper( )
    : m_pParserCtxt( NULL ),
      m_pSaxHandler( NULL )
{
    xmlInitParser() ;
    LIBXML_TEST_VERSION ;

    /*
     * compile error:
     * xmlLoadExtDtdDefaultValue = XML_DETECT_IDS | XML_COMPLETE_ATTRS ;
     */
    xmlSubstituteEntitiesDefault( 1 ) ;

#ifndef XMLSEC_NO_XSLT
    xmlIndentTreeOutput = 1 ;
#endif /* XMLSEC_NO_XSLT */

    m_pParserCtxt = xmlNewParserCtxt() ;

    /*
     * i41748
     *
     * mmi : re-initialize the SAX handler to version 1
     */

    xmlSAXVersion(m_pParserCtxt->sax, 1);

    /* end */

    if( m_pParserCtxt->inputTab[0] != NULL )
    {
        m_pParserCtxt->inputTab[0] = NULL ;
    }

    if( m_pParserCtxt == NULL )
    {
#ifndef XMLSEC_NO_XSLT
        xsltCleanupGlobals() ;
#endif
//      see issue i74334, we cannot call xmlCleanupParser when libxml is still used
//      in other parts of the office.
//      xmlCleanupParser() ;
        throw cssu::RuntimeException() ;
    }
    else if( m_pParserCtxt->sax == NULL )
    {
        xmlFreeParserCtxt( m_pParserCtxt ) ;

#ifndef XMLSEC_NO_XSLT
        xsltCleanupGlobals() ;
#endif
//      see issue i74334, we cannot call xmlCleanupParser when libxml is still used
//      in other parts of the office.
//      xmlCleanupParser() ;
        m_pParserCtxt = NULL ;
        throw cssu::RuntimeException() ;
    }
    else
    {
        m_pSaxHandler = m_pParserCtxt->sax ;

        //Adjust the context
        m_pParserCtxt->recovery = 1 ;
    }
}

/**
 * Destructor
 *
 * In this destructor, a libxml sax parser context is desturcted. The XML tree
 * in the context is not deallocated because the tree is bind with a document
 * model by the setTargetDocument method, which delegate the target document to
 * destruct the xml tree.
 */
SAXHelper::~SAXHelper() {
    if( m_pParserCtxt != NULL )
    {
        /*
         * In the situation that no object refer the Document, this destructor
         * must deallocate the Document memory
         */
        if( m_pSaxHandler == m_pParserCtxt->sax )
        {
            m_pSaxHandler = NULL ;
        }

        xmlFreeParserCtxt( m_pParserCtxt ) ;
        m_pParserCtxt = NULL ;
    }

    if( m_pSaxHandler != NULL )
    {
        xmlFree( m_pSaxHandler ) ;
        m_pSaxHandler = NULL ;
    }
//      see issue i74334, we cannot call xmlCleanupParser when libxml is still used
//      in other parts of the office.
//  xmlCleanupParser() ;
}

xmlNodePtr SAXHelper::getCurrentNode()
{
    return m_pParserCtxt->node;
}

void SAXHelper::setCurrentNode(const xmlNodePtr pNode)
{
    /*
     * This is really a black trick.
     * When the current node is replaced, the nodeTab
     * stack's top has to been replaced with the same
     * node, in order to make compatibility.
     */
    m_pParserCtxt->nodeTab[m_pParserCtxt->nodeNr - 1]
        = m_pParserCtxt->node
            = pNode;
}

xmlDocPtr SAXHelper::getDocument()
{
    return m_pParserCtxt->myDoc;
}

/**
 * XDocumentHandler -- start an xml document
 */
void SAXHelper::startDocument( void )
    throw( cssxs::SAXException , cssu::RuntimeException )
{
    /*
     * Adjust inputTab
     */
    xmlParserInputPtr pInput = xmlNewInputStream( m_pParserCtxt ) ;

    if( m_pParserCtxt->inputTab != NULL && m_pParserCtxt->inputMax != 0 )
    {
        m_pParserCtxt->inputTab[0] = pInput ;
        m_pParserCtxt->input = pInput ;
    }

    m_pSaxHandler->startDocument( m_pParserCtxt ) ;

    if( m_pParserCtxt == NULL || m_pParserCtxt->myDoc == NULL )
    {
        throw cssu::RuntimeException() ;
    }
}

/**
 * XDocumentHandler -- end an xml document
 */
void SAXHelper::endDocument( void )
    throw( cssxs::SAXException , cssu::RuntimeException )
{
    m_pSaxHandler->endDocument( m_pParserCtxt ) ;
}

/**
 * XDocumentHandler -- start an xml element
 */
void SAXHelper::startElement(
    const rtl::OUString& aName,
    const cssu::Sequence< cssxcsax::XMLAttribute >& aAttributes )
    throw( cssxs::SAXException , cssu::RuntimeException )
{
    const xmlChar* fullName = NULL ;
    const xmlChar** attrs = NULL ;

    fullName = ous_to_xmlstr( aName ) ;
    attrs = attrlist_to_nxmlstr( aAttributes ) ;

    if( fullName != NULL || attrs != NULL )
    {
        m_pSaxHandler->startElement( m_pParserCtxt , fullName , attrs ) ;
    }

    if( fullName != NULL )
    {
        xmlFree( ( xmlChar* )fullName ) ;
        fullName = NULL ;
    }

    if( attrs != NULL )
    {
        for( int i = 0 ; attrs[i] != NULL ; ++i )
        {
            xmlFree( ( xmlChar* )attrs[i] ) ;
            attrs[i] = NULL ;
        }

        xmlFree( ( void* ) attrs ) ;
        attrs = NULL ;
    }
}

/**
 * XDocumentHandler -- end an xml element
 */
void SAXHelper::endElement( const rtl::OUString& aName )
    throw( cssxs::SAXException , cssu::RuntimeException )
{
    xmlChar* fullname = NULL ;

    fullname = ous_to_xmlstr( aName ) ;
    m_pSaxHandler->endElement( m_pParserCtxt , fullname ) ;

    if( fullname != NULL )
    {
        xmlFree( ( xmlChar* )fullname ) ;
        fullname = NULL ;
    }
}

/**
 * XDocumentHandler -- an xml element or cdata characters
 */
void SAXHelper::characters( const rtl::OUString& aChars )
    throw( cssxs::SAXException , cssu::RuntimeException )
{
    const xmlChar* chars = NULL ;
    int length = 0 ;

    chars = ous_to_nxmlstr( aChars, length ) ;
    m_pSaxHandler->characters( m_pParserCtxt , chars , length ) ;

    if( chars != NULL )
    {
        xmlFree( ( xmlChar* )chars ) ;
    }
}

/**
 * XDocumentHandler -- ignorable xml white space
 */
void SAXHelper::ignorableWhitespace( const rtl::OUString& aWhitespaces )
    throw( cssxs::SAXException , cssu::RuntimeException )
{
    const xmlChar* chars = NULL ;
    int length = 0 ;

    chars = ous_to_nxmlstr( aWhitespaces, length ) ;
    m_pSaxHandler->ignorableWhitespace( m_pParserCtxt , chars , length ) ;

    if( chars != NULL )
    {
        xmlFree( ( xmlChar* )chars ) ;
    }
}

/**
 * XDocumentHandler -- preaorocessing instruction
 */
void SAXHelper::processingInstruction(
    const rtl::OUString& aTarget,
    const rtl::OUString& aData )
    throw( cssxs::SAXException , cssu::RuntimeException )
{
    xmlChar* target = NULL ;
    xmlChar* data = NULL ;

    target = ous_to_xmlstr( aTarget ) ;
    data = ous_to_xmlstr( aData ) ;

    m_pSaxHandler->processingInstruction( m_pParserCtxt , target , data ) ;

    if( target != NULL )
    {
        xmlFree( ( xmlChar* )target ) ;
        target = NULL ;
    }

    if( data != NULL )
    {
        xmlFree( ( xmlChar* )data ) ;
        data = NULL ;
    }
}

/**
 * XDocumentHandler -- set document locator
 * In this case, locator is useless.
 */
void SAXHelper::setDocumentLocator(
    const cssu::Reference< cssxs::XLocator > &)
    throw( cssxs::SAXException , cssu::RuntimeException )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
