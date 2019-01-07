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

#include <xmlsec/saxhelper.hxx>
#include <libxml/parserInternals.h>

#ifndef XMLSEC_NO_XSLT
#include "libxslt/xslt.h"
#endif

namespace cssu = com::sun::star::uno;
namespace cssxs = com::sun::star::xml::sax;
namespace cssxcsax = com::sun::star::xml::csax;

/**
 * The return value is NULL terminated. The application has the responsibility to
 * deallocate the return value.
 */
static xmlChar* ous_to_xmlstr( const OUString& oustr )
{
    OString ostr = OUStringToOString( oustr , RTL_TEXTENCODING_UTF8 ) ;
    return xmlStrndup( reinterpret_cast<xmlChar const *>(ostr.getStr()), static_cast<int>(ostr.getLength()) ) ;
}

/**
 * The return value is NULL terminated. The application has the responsibility to
 * deallocate the return value.
 */
static xmlChar* ous_to_nxmlstr( const OUString& oustr, int& length )
{
    OString ostr = OUStringToOString( oustr , RTL_TEXTENCODING_UTF8 ) ;
    length = ostr.getLength();

    return xmlStrndup( reinterpret_cast<xmlChar const *>(ostr.getStr()), length ) ;
}

/**
 * The return value and the referenced value must be NULL terminated.
 * The application has the responsibility to deallocate the return value.
 */
static const xmlChar** attrlist_to_nxmlstr( const cssu::Sequence< cssxcsax::XMLAttribute >& aAttributes )
{
    xmlChar* attname = nullptr ;
    xmlChar* attvalue = nullptr ;
    const xmlChar** attrs = nullptr ;

    sal_Int32 nLength = aAttributes.getLength();

    if( nLength != 0 )
    {
        attrs = static_cast<const xmlChar**>(xmlMalloc( ( nLength * 2 + 2 ) * sizeof( xmlChar* ) ));
    }
    else
    {
        return nullptr ;
    }

    for( int i = 0 , j = 0 ; j < nLength ; ++j )
    {
        attname = ous_to_xmlstr( aAttributes[j].sName ) ;
        attvalue = ous_to_xmlstr( aAttributes[j].sValue ) ;

        if( attname != nullptr && attvalue != nullptr )
        {
            attrs[i++] = attname ;
            attrs[i++] = attvalue ;
            attrs[i] = nullptr ;
            attrs[i+1] = nullptr ;
        }
        else
        {
            if( attname != nullptr )
                xmlFree( attname ) ;
            if( attvalue != nullptr )
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
    : m_pParserCtxt( nullptr ),
      m_pSaxHandler( nullptr )
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

    if( m_pParserCtxt == nullptr )
    {
//      see issue i74334, we cannot call xmlCleanupParser when libxml is still used
//      in other parts of the office.
//      xmlCleanupParser() ;
//      and neither can we call xsltCleanupGlobals()
        throw cssu::RuntimeException() ;
    }

    xmlSAXVersion(m_pParserCtxt->sax, 1);

    if (m_pParserCtxt->inputTab != nullptr)
    {
        m_pParserCtxt->inputTab[0] = nullptr ;
    }

    if( m_pParserCtxt->sax == nullptr )
    {
        xmlFreeParserCtxt( m_pParserCtxt ) ;

//      see issue i74334, we cannot call xmlCleanupParser when libxml is still used
//      in other parts of the office.
//      xmlCleanupParser() ;
//      and neither can we call xsltCleanupGlobals()
        m_pParserCtxt = nullptr ;
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
 * In this destructor, a libxml sax parser context is destructed. The XML tree
 * in the context is not deallocated because the tree is bind with a document
 * model by the setTargetDocument method, which delegate the target document to
 * destruct the xml tree.
 */
SAXHelper::~SAXHelper() {
    if( m_pParserCtxt != nullptr )
    {
        /*
         * In the situation that no object refer the Document, this destructor
         * must deallocate the Document memory
         */
        if( m_pSaxHandler == m_pParserCtxt->sax )
        {
            m_pSaxHandler = nullptr ;
        }

        xmlFreeParserCtxt( m_pParserCtxt ) ;
        m_pParserCtxt = nullptr ;
    }

    if( m_pSaxHandler != nullptr )
    {
        xmlFree( m_pSaxHandler ) ;
        m_pSaxHandler = nullptr ;
    }
//      see issue i74334, we cannot call xmlCleanupParser when libxml is still used
//      in other parts of the office.
//  xmlCleanupParser() ;
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


/**
 * XDocumentHandler -- start an xml document
 */
void SAXHelper::startDocument()
{
    if( m_pParserCtxt == nullptr)
    {
        throw cssu::RuntimeException() ;
    }
    /*
     * Adjust inputTab
     */
    xmlParserInputPtr pInput = xmlNewInputStream( m_pParserCtxt ) ;

    if( m_pParserCtxt->inputTab != nullptr && m_pParserCtxt->inputMax != 0 )
    {
        m_pParserCtxt->inputTab[0] = pInput ;
        m_pParserCtxt->input = pInput ;
    }

    m_pSaxHandler->startDocument( m_pParserCtxt ) ;

    if( m_pParserCtxt->myDoc == nullptr )
    {
        throw cssu::RuntimeException() ;
    }
}

/**
 * XDocumentHandler -- end an xml document
 */
void SAXHelper::endDocument()
{
    m_pSaxHandler->endDocument( m_pParserCtxt ) ;
}

/**
 * XDocumentHandler -- start an xml element
 */
void SAXHelper::startElement(
    const OUString& aName,
    const cssu::Sequence< cssxcsax::XMLAttribute >& aAttributes )
{
    const xmlChar* fullName = nullptr ;
    const xmlChar** attrs = nullptr ;

    fullName = ous_to_xmlstr( aName ) ;
    attrs = attrlist_to_nxmlstr( aAttributes ) ;

    if( fullName != nullptr || attrs != nullptr )
    {
        m_pSaxHandler->startElement( m_pParserCtxt , fullName , attrs ) ;
    }

    if( fullName != nullptr )
    {
        xmlFree( const_cast<xmlChar*>(fullName) ) ;
        fullName = nullptr ;
    }

    if( attrs != nullptr )
    {
        for( int i = 0 ; attrs[i] != nullptr ; ++i )
        {
            xmlFree( const_cast<xmlChar*>(attrs[i]) ) ;
            attrs[i] = nullptr ;
        }

        xmlFree( static_cast<void*>(attrs) ) ;
        attrs = nullptr ;
    }
}

/**
 * XDocumentHandler -- end an xml element
 */
void SAXHelper::endElement( const OUString& aName )
{
    xmlChar* fullname = nullptr ;

    fullname = ous_to_xmlstr( aName ) ;
    m_pSaxHandler->endElement( m_pParserCtxt , fullname ) ;

    if( fullname != nullptr )
    {
        xmlFree( fullname ) ;
        fullname = nullptr ;
    }
}

/**
 * XDocumentHandler -- an xml element or cdata characters
 */
void SAXHelper::characters( const OUString& aChars )
{
    const xmlChar* chars = nullptr ;
    int length = 0 ;

    chars = ous_to_nxmlstr( aChars, length ) ;
    m_pSaxHandler->characters( m_pParserCtxt , chars , length ) ;

    if( chars != nullptr )
    {
        xmlFree( const_cast<xmlChar*>(chars) ) ;
    }
}

/**
 * XDocumentHandler -- ignorable xml white space
 */
void SAXHelper::ignorableWhitespace( const OUString& aWhitespaces )
{
    const xmlChar* chars = nullptr ;
    int length = 0 ;

    chars = ous_to_nxmlstr( aWhitespaces, length ) ;
    m_pSaxHandler->ignorableWhitespace( m_pParserCtxt , chars , length ) ;

    if( chars != nullptr )
    {
        xmlFree( const_cast<xmlChar*>(chars) ) ;
    }
}

/**
 * XDocumentHandler -- preprocessing instruction
 */
void SAXHelper::processingInstruction(
    const OUString& aTarget,
    const OUString& aData )
{
    xmlChar* target = nullptr ;
    xmlChar* data = nullptr ;

    target = ous_to_xmlstr( aTarget ) ;
    data = ous_to_xmlstr( aData ) ;

    m_pSaxHandler->processingInstruction( m_pParserCtxt , target , data ) ;

    if( target != nullptr )
    {
        xmlFree( target ) ;
        target = nullptr ;
    }

    if( data != nullptr )
    {
        xmlFree( data ) ;
        data = nullptr ;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
