/*************************************************************************
 *
 *  $RCSfile: ScriptMetadataImporter.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dfoster $ $Date: 2002-10-17 10:04:12 $
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

#include "ScriptMetadataImporter.hxx"
#include <osl/mutex.hxx>
#include <com/sun/star/xml/sax/XParser.hpp>

#ifdef _DEBUG
#include <stdio.h>
#endif
#include <util/util.hxx>
#include <rtl/string.h>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace scripting_impl
{

//*************************************************************************
ScriptMetadataImporter::ScriptMetadataImporter(
    const Reference< XComponentContext > & xContext )
    : m_xContext( xContext )
{
    OSL_TRACE( "< ScriptMetadataImporter ctor called >\n" );
}

//*************************************************************************
ScriptMetadataImporter::~ScriptMetadataImporter() SAL_THROW( () )
{
    OSL_TRACE( "< ScriptMetadataImporter dtor called >\n" );
}


//*************************************************************************
void ScriptMetadataImporter::parseMetaData(
    Reference< io::XInputStream > const & xInput,
    const ::rtl::OUString & parcelURI,
    Datas_vec &  io_ScriptDatas )
    throw ( xml::sax::SAXException, io::IOException, RuntimeException )
{
    mpv_scriptDatas=&io_ScriptDatas;
    Datas_vec & ms_scriptDatas=*mpv_scriptDatas;
    //Clear the vector of parsed information
    ms_scriptDatas.clear();

    //Set the placeholder for the parcel URI
    ms_parcelURI = parcelURI;

    //Get the parser service
    validateXRef(m_xContext,
        "ScriptMetadataImporter::parseMetaData: No context available");

    Reference< lang::XMultiComponentFactory > xMgr =
        m_xContext->getServiceManager();

    validateXRef(xMgr,
        "ScriptMetadataImporter::parseMetaData: No service manager available");

    Reference< XInterface > xx = xMgr->createInstanceWithContext(
        OUString::createFromAscii("com.sun.star.xml.sax.Parser"), m_xContext );

    validateXRef(xMgr, "ScriptMetadataImporter::parseMetaData: cannot get SAX Parser");
    Reference< xml::sax::XParser > xParser(xx,UNO_QUERY_THROW);

    // xxx todo: error handler, entity resolver omitted
    // This class is the document handler for the parser
    Reference< xml::sax::XDocumentHandler > t_smI( this );
    xParser->setDocumentHandler( t_smI );

    //Set up the input for the parser, the XInputStream
    xml::sax::InputSource source;
    source.aInputStream = xInput;
    source.sSystemId = OUSTR("virtual file");

    OSL_TRACE("ScriptMetadataImporter: Start the parser\n");

    try
    {
        xParser->parseStream( source );
    }
    catch ( xml::sax::SAXException & saxe )
    {
        OUString msg = OUString::createFromAscii(
            "ScriptMetadata:Importer::parserMetaData SAXException");
        msg.concat(saxe.Message);
        throw xml::sax::SAXException( msg, Reference< XInterface > (),
            saxe.WrappedException);
    }
    catch ( io::IOException & ioe )
    {
        throw io::IOException( OUString::createFromAscii(
            "ScriptMetadataImporter::parseMetaData IOException: ")+ioe.Message,
            Reference< XInterface > ());
    }

#ifdef _DEBUG
    catch ( ... )
    {
        throw RuntimeException(OUString::createFromAscii(
            "ScriptMetadataImporter::parseMetadata UnknownException: "),
            Reference< XInterface > ());
    }
#endif

    OSL_TRACE("ScriptMetadataImporter: Parser finished\n");

#ifdef _DEBUG
    fprintf(stderr, "ScriptMetadataImporter: vector size is %d\n",
        ms_scriptDatas.size());
#endif
}

//*************************************************************************
// XExtendedDocumentHandler impl
void ScriptMetadataImporter::startCDATA()
    throw (xml::sax::SAXException, RuntimeException)
{
    OSL_TRACE("ScriptMetadataImporter: startCDATA()\n");
}

//*************************************************************************
void ScriptMetadataImporter::endCDATA()
    throw (RuntimeException)
{
    OSL_TRACE("ScriptMetadataImporter: endDATA()\n");
}

//*************************************************************************
void ScriptMetadataImporter::comment( const ::rtl::OUString & sComment )
    throw (xml::sax::SAXException, RuntimeException)
{
    OSL_TRACE("ScriptMetadataImporter: comment()\n");
}

//*************************************************************************
void ScriptMetadataImporter::allowLineBreak()
    throw (xml::sax::SAXException, RuntimeException)
{
    OSL_TRACE("ScriptMetadataImporter: allowLineBreak()\n");
}

//*************************************************************************
void ScriptMetadataImporter::unknown( const ::rtl::OUString & sString )
    throw (xml::sax::SAXException, RuntimeException)
{
    OSL_TRACE("ScriptMetadataImporter: unknown()\n");
}

//*************************************************************************
// XDocumentHandler impl
void ScriptMetadataImporter::startDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
    // Ignore for now
    OSL_TRACE("ScriptMetadataImporter: startDocument()\n");
}

//*************************************************************************
void ScriptMetadataImporter::endDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
    // Ignore for now
    OSL_TRACE("ScriptMetadataImporter: endDocument()\n");
}

//*************************************************************************
void ScriptMetadataImporter::startElement(
    const ::rtl::OUString& tagName,
    const Reference< xml::sax::XAttributeList >& xAttribs )
    throw (xml::sax::SAXException, RuntimeException)
{

#ifdef _DEBUG
    fprintf(stderr, "Trace Message : ScriptMetadataImporter: startElement() %s\n",
           ::rtl::OUStringToOString(tagName,
                                    RTL_TEXTENCODING_ASCII_US).pData->buffer);
#endif

    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );

    //Set the state of the state machine
    setState(tagName);

    //Temporay variables
    ::rtl::OUString t_delivered = ::rtl::OUString::createFromAscii("false");

    //Processing the elements
    switch(m_state)
    {
    case PARCEL:
        break;

    case SCRIPT:
        {
            //Assign a new empty struct to the member struct to clear
            //all values in the struct
            ScriptData t_implInfo;
            m_scriptData = t_implInfo;
            m_scriptData.parcelURI = ms_parcelURI;
            if(xAttribs->getLength() == 2)
            {
                //Get the script tag attributes
                OSL_TRACE("ScriptMetadataImporter: Get language and deployment dir\n");

                //script language
                m_scriptData.scriptLanguage = xAttribs->getValueByName(
                    ::rtl::OUString::createFromAscii("language"));

#ifdef _DEBUG
                fprintf(stderr, "ScriptMetadataImporter: Got language: %s\n",
                    ::rtl::OUStringToOString(m_scriptData.scriptLanguage,
                        RTL_TEXTENCODING_ASCII_US).pData->buffer);
#endif

                //script root directory
                m_scriptData.scriptRoot = xAttribs->getValueByName(
                    ::rtl::OUString::createFromAscii( "deploymentdir" ));

#ifdef _DEBUG
                fprintf(stderr, "ScriptMetadataImporter: Got dir: %s\n",
                    ::rtl::OUStringToOString(m_scriptData.scriptRoot,
                        RTL_TEXTENCODING_ASCII_US).pData->buffer);
#endif

            }
            break;
        }

    case LOGICALNAME:
        //logical name
        m_scriptData.logicalName =
            xAttribs->getValueByName(
                ::rtl::OUString::createFromAscii("value"));

#ifdef _DEBUG
        fprintf(stderr, "ScriptMetadataImporter: Got  logicalname: %s\n",
            ::rtl::OUStringToOString(m_scriptData.logicalName,
                RTL_TEXTENCODING_ASCII_US).pData->buffer);
#endif

        break;

    case LANGUAGENAME:
        //language(function) name
        m_scriptData.functionName =
            xAttribs->getValueByName(
                ::rtl::OUString::createFromAscii("value"));
        m_scriptData.scriptLocation =
            xAttribs->getValueByName(
                ::rtl::OUString::createFromAscii("location"));

#ifdef _DEBUG
        fprintf(stderr, "ScriptMetadataImporter: Got language: %s\n",
            ::rtl::OUStringToOString(m_scriptData.functionName,
                RTL_TEXTENCODING_ASCII_US).pData->buffer);
#endif

        break;

    case DELIVERFILE:
        {
            //Get Info about delivered files
            ::std::pair < ::rtl::OUString, ::rtl::OUString > deliveryFile(
                xAttribs->getValueByName(
                    ::rtl::OUString::createFromAscii( "name" ) ),
                xAttribs->getValueByName(
                    ::rtl::OUString::createFromAscii( "type" ) ) );
            m_scriptData.parcelDelivers.push_back( deliveryFile );

            break;
        }

    case DEPENDFILE:
        {
            //push the dependency into the the vector
            ::std::pair < ::rtl::OUString, bool > dependFile(
                xAttribs->getValueByName(
                    ::rtl::OUString::createFromAscii( "name" ) ),
                false );
            ::rtl::OUString t_delivered = xAttribs->getValueByName(
                    ::rtl::OUString::createFromAscii( "isdeliverable" ) );
            if( t_delivered.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("yes")) )
            {
                dependFile.second = true;
            }
            m_scriptData.scriptDependencies.push_back( dependFile );

#ifdef _DEBUG
            fprintf(stderr, "ScriptMetadataImporter: Got dependency: %s\n",
                ::rtl::OUStringToOString( xAttribs->getValueByName(
                   ::rtl::OUString::createFromAscii("name")),
                       RTL_TEXTENCODING_ASCII_US).pData->buffer);
#endif

            break;
        }

        //Needs to be here to circumvent bypassing of initialization of
        //local(global) variables affecting other cases
    }
}

//*************************************************************************
void ScriptMetadataImporter::endElement( const ::rtl::OUString & aName )
    throw (xml::sax::SAXException, RuntimeException)
{

    //The end tag of an element
#ifdef _DEBUG
    fprintf(stderr, "ScriptMetadataImporter: endElement() %s\n",
        ::rtl::OUStringToOString(aName,
            RTL_TEXTENCODING_ASCII_US).pData->buffer);
#endif

    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );

    //Set the state
    setState(aName);

    //Temporary variables
    int t_delSize = 0;

    switch (m_state)
    {
    case PARCEL:
        break;
    case SCRIPT:

#ifdef _DEBUG
        OSL_TRACE("ScriptMetadataImporter: Got a scriptData\n");
        fprintf(stderr, "ScriptMetadataImporter: \t %s\n", ::rtl::OUStringToOString(
            m_scriptData.scriptLanguage, RTL_TEXTENCODING_ASCII_US).pData->buffer);
#endif

        //Push the struct into the vector
        mpv_scriptDatas->push_back(m_scriptData);
        break;

    case LOGICALNAME:
        break;

    case LANGUAGENAME:
        break;

    case DELIVERY:
        break;

    case DEPENDENCIES:
        break;
    }
}

//*************************************************************************
void ScriptMetadataImporter::characters( const ::rtl::OUString & aChars )
    throw (xml::sax::SAXException, RuntimeException)
{
    OSL_TRACE("ScriptMetadataImporter: characters()\n");

    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );

    switch (m_state)
    {
    case PARCEL:
        break;
    case SCRIPT:
        break;
    case LANGUAGENAME:
        break;
    case LOGICALNAME:
        break;
    case DEPENDENCIES:
        break;
    case DESCRIPTION:
        //Put description into the struct
        m_scriptData.scriptDescription = aChars;
        break;
    case DELIVERY:
        break;
    case DELIVERFILE:
        break;
    case DEPENDFILE:
        break;
    }
}

//*************************************************************************
void ScriptMetadataImporter::ignorableWhitespace(
    const ::rtl::OUString & aWhitespaces )
    throw (xml::sax::SAXException, RuntimeException)
{
    OSL_TRACE("ScriptMetadataImporter: ignorableWhiteSpace()\n");
}

//*************************************************************************
void ScriptMetadataImporter::processingInstruction(
    const ::rtl::OUString & aTarget, const ::rtl::OUString & aData )
    throw (xml::sax::SAXException, RuntimeException)
{
    OSL_TRACE("ScriptMetadataImporter: processingInstruction()\n");
}

//*************************************************************************
void ScriptMetadataImporter::setDocumentLocator(
    const Reference< xml::sax::XLocator >& xLocator )
    throw (xml::sax::SAXException, RuntimeException)
{
    OSL_TRACE("ScriptMetadataImporter: setDocumentLocator()\n");
}

//*************************************************************************
void ScriptMetadataImporter::setState(const ::rtl::OUString & tagName)
{
    //Set the state depending on the tag name of the current
    //element the parser has arrived at
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );

    if(tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("parcel") ))
    {
        //Parcel tag
        m_state = PARCEL;
    }
    else if (tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("script") ))
    {
        //Script tag
        m_state = SCRIPT;
    }
    else if (tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("logicalname") ))
    {
        //logicalname tag
        m_state = LOGICALNAME;
    }
    else if (tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("languagename") ))
    {
        //languagename tag
        m_state = LANGUAGENAME;
    }
    else if(tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dependencies") ))
    {
        //dependencies tag
        m_state = DEPENDENCIES;
    }
    else if(tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("description") ))
    {
        //Description tag
        m_state = DESCRIPTION;
    }
    else if(tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("delivery") ))
    {
        //delivery tag, nothing to be done here
        m_state = DELIVERY;
    }
    else if(tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("deliverfile") ))
    {
        //deliverfile tag, nothing to be done here
        m_state = DELIVERFILE;
    }
    else if(tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dependfile") ))
    {
        m_state = DEPENDFILE;
    }
    else
    {
        //If there is a tag we don't know about, throw a exception (wobbler) :)
        ::rtl::OUString str_sax = ::rtl::OUString::createFromAscii( "No Such Tag" );

#ifdef _DEBUG
        fprintf(stderr, "ScriptMetadataImporter: No Such Tag: %s\n",
            ::rtl::OUStringToOString(
                tagName, RTL_TEXTENCODING_ASCII_US).pData->buffer);
#endif

        throw xml::sax::SAXException(
            str_sax, Reference< XInterface >(), Any() );
    }
}
}
