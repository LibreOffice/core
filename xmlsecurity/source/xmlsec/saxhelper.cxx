/*************************************************************************
 *
 *  $RCSfile: saxhelper.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-12 13:15:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
 * The input parameter isn't necessaryly NULL terminated.
 */
rtl::OUString xmlchar_to_ous( const xmlChar* pChar, int length )
{
    if( pChar != NULL )
    {
        return rtl::OUString( ( sal_Char* )pChar , length , RTL_TEXTENCODING_UTF8 ) ;
    }
    else
    {
        return rtl::OUString() ;
    }
}

/**
 * The input parameter is NULL terminated
 */
rtl::OUString xmlstr_to_ous( const xmlChar* pStr )
{
    if( pStr != NULL )
    {
        return xmlchar_to_ous( pStr , xmlStrlen( pStr ) ) ;
    }
    else
    {
        return rtl::OUString() ;
    }
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
    rtl::OUString oustr ;

    sal_Int32 nLength = aAttributes.getLength();;

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
    if( m_pParserCtxt->inputTab[0] != NULL )
    {
        m_pParserCtxt->inputTab[0] = NULL ;
    }

    if( m_pParserCtxt == NULL )
    {
#ifndef XMLSEC_NO_XSLT
        xsltCleanupGlobals() ;
#endif
        xmlCleanupParser() ;
        throw cssu::RuntimeException() ;
    }
    else if( m_pParserCtxt->sax == NULL )
    {
        xmlFreeParserCtxt( m_pParserCtxt ) ;

#ifndef XMLSEC_NO_XSLT
        xsltCleanupGlobals() ;
#endif
        xmlCleanupParser() ;
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

    xmlCleanupParser() ;
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
    const cssu::Reference< cssxs::XLocator > & xLocator )
    throw( cssxs::SAXException , cssu::RuntimeException )
{
    //--Pseudo code if necessary
    //--m_pSaxLocator is a member defined as xmlSAXHabdlerPtr
    //--m_pSaxLocatorHdl is a member defined as Sax_Locator

    //if( m_pSaxLocator != NULL ) {
    //  //Deallocate the memory
    //}
    //if( m_pSaxLocatorHdl != NULL ) {
    //  //Deallocate the memory
    //}

    //m_pSaxLocatorHdl = new Sax_Locator( xLocator ) ;
    //m_pSaxLocator = { m_pSaxLocatorHdl->getPublicId , m_pSaxLocatorHdl->getSystemId , m_pSaxLocatorHdl->getLineNumber , m_pSaxLocatorHdl->getColumnNumber } ;

    //m_pSaxHandler->setDocumentLocator( m_pParserCtxt , m_pSaxLocator ) ;
}

