/*************************************************************************
 *
 *  $RCSfile: ScriptMetadataImporter.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: npower $ $Date: 2003-10-15 08:29:37 $
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

#ifdef _DEBUG
#include <stdio.h>
#endif

#include "ScriptMetadataImporter.hxx"

#include <osl/mutex.hxx>

#include <com/sun/star/xml/sax/XParser.hpp>
#include <rtl/string.h>


#include <util/util.hxx>

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
    ms_localeDesc = new OUStringBuffer();
}

//*************************************************************************
ScriptMetadataImporter::~ScriptMetadataImporter() SAL_THROW( () )
{
    OSL_TRACE( "< ScriptMetadataImporter dtor called >\n" );
    delete ms_localeDesc;
}


//*************************************************************************
void ScriptMetadataImporter::parseMetaData(
    Reference< io::XInputStream > const & xInput,
    const ::rtl::OUString & parcelURI,
    InfoImpls_vec &  io_ScriptDatas )
    throw ( xml::sax::SAXException, io::IOException, RuntimeException )
{

    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );

    mpv_ScriptDatas = &io_ScriptDatas;
    mpv_ScriptDatas->clear();

    //Set the placeholder for the parcel URI
    ms_parcelURI = parcelURI;

    //Get the parser service
    validateXRef( m_xContext,
        "ScriptMetadataImporter::parseMetaData: No context available" );

    Reference< lang::XMultiComponentFactory > xMgr =
        m_xContext->getServiceManager();

    validateXRef( xMgr,
        "ScriptMetadataImporter::parseMetaData: No service manager available" );

    Reference< XInterface > xInterface = xMgr->createInstanceWithContext(
        OUString::createFromAscii( "com.sun.star.xml.sax.Parser" ), m_xContext );

    validateXRef( xInterface, "ScriptMetadataImporter::parseMetaData: cannot get SAX Parser" );
    Reference< xml::sax::XParser > xParser;
    try
    {
        xParser.set( xInterface ,UNO_QUERY_THROW );
    }
    catch (RuntimeException & re )
    {
        OUString msg = OUString::createFromAscii(
            "ScriptMetadata:Importer::parserMetaData cannot get XParser" );
        msg.concat( re.Message );
        throw RuntimeException( msg, Reference< XInterface > () );
    }

    // xxx todo: error handler, entity resolver omitted
    // This class is the document handler for the parser
    Reference< xml::sax::XDocumentHandler > t_smI( this );
    xParser->setDocumentHandler( t_smI );

    //Set up the input for the parser, the XInputStream
    xml::sax::InputSource source;
    source.aInputStream = xInput;
    source.sSystemId = OUSTR( "virtual file" );

    OSL_TRACE( "ScriptMetadataImporter: Start the parser\n" );

    try
    {
        xParser->parseStream( source );
    }
    catch ( xml::sax::SAXException & saxe )
    {
        OUString msg = OUString::createFromAscii(
            "ScriptMetadata:Importer::parserMetaData SAXException" );
        msg.concat( saxe.Message );
        throw xml::sax::SAXException( msg, Reference< XInterface > (),
            saxe.WrappedException );
    }
    catch ( io::IOException & ioe )
    {
        throw io::IOException( OUString::createFromAscii(
            "ScriptMetadataImporter::parseMetaData IOException: " ) + ioe.Message,
            Reference< XInterface > () );
    }

#ifdef _DEBUG
    catch ( ... )
    {
        throw RuntimeException( OUString::createFromAscii(
            "ScriptMetadataImporter::parseMetadata UnknownException: " ),
            Reference< XInterface > () );
    }
#endif

    OSL_TRACE( "ScriptMetadataImporter: Parser finished\n ");

    OSL_TRACE(  "ScriptMetadataImporter: vector size is %d\n",
        mpv_ScriptDatas->size() );
}

//*************************************************************************
// XExtendedDocumentHandler impl
void ScriptMetadataImporter::startCDATA()
    throw ( xml::sax::SAXException, RuntimeException )
{
    OSL_TRACE( "ScriptMetadataImporter: startCDATA()\n" );
}

//*************************************************************************
void ScriptMetadataImporter::endCDATA()
    throw ( RuntimeException )
{
    OSL_TRACE( "ScriptMetadataImporter: endDATA()\n" );
}

//*************************************************************************
void ScriptMetadataImporter::comment( const ::rtl::OUString & sComment )
    throw ( xml::sax::SAXException, RuntimeException )
{
    OSL_TRACE( "ScriptMetadataImporter: comment()\n" );
}

//*************************************************************************
void ScriptMetadataImporter::allowLineBreak()
    throw ( xml::sax::SAXException, RuntimeException )
{
    OSL_TRACE( "ScriptMetadataImporter: allowLineBreak()\n" );
}

//*************************************************************************
void ScriptMetadataImporter::unknown( const ::rtl::OUString & sString )
    throw ( xml::sax::SAXException, RuntimeException )
{
    OSL_TRACE( "ScriptMetadataImporter: unknown()\n" );
}

//*************************************************************************
// XDocumentHandler impl
void ScriptMetadataImporter::startDocument()
    throw ( xml::sax::SAXException, RuntimeException )
{
    // Ignore for now
    OSL_TRACE( "ScriptMetadataImporter: startDocument()\n" );
}

//*************************************************************************
void ScriptMetadataImporter::endDocument()
    throw ( xml::sax::SAXException, RuntimeException )
{
    // Ignore for now
    OSL_TRACE( "ScriptMetadataImporter: endDocument()\n" );
}

//*************************************************************************
void ScriptMetadataImporter::startElement(
    const ::rtl::OUString& tagName,
    const Reference< xml::sax::XAttributeList >& xAttribs )
    throw ( xml::sax::SAXException, RuntimeException )
{

    OSL_TRACE(  "Trace Message : ScriptMetadataImporter: startElement() %s\n",
           ::rtl::OUStringToOString( tagName,
                                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );

    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );

    //Set the state of the state machine
    setState( tagName );

    //Processing the elements
    switch( m_state )
    {
        case SCRIPT:
            m_ScriptData.parcelURI = ms_parcelURI;
            m_ScriptData.language = xAttribs->getValueByName(
                ::rtl::OUString::createFromAscii( "language" ));
            OSL_TRACE(  "Trace Message: language is %s\n",
                ::rtl::OUStringToOString( m_ScriptData.language,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            break;
        case LOCALE:
            ms_localeLang = xAttribs->getValueByName(
               ::rtl::OUString::createFromAscii( "lang" ) );
            OSL_TRACE(  "Trace Message: Locale is %s\n",
                ::rtl::OUStringToOString( ms_localeLang,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            break;
        case DISPLAYNAME:
            ms_localeDisName = xAttribs->getValueByName(
               ::rtl::OUString::createFromAscii( "value" ));
            OSL_TRACE(  "Trace Message: Displyaname is %s\n",
                ::rtl::OUStringToOString( ms_localeDisName,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            break;
        case FUNCTIONNAME:
            m_ScriptData.functionname = xAttribs->getValueByName(
               ::rtl::OUString::createFromAscii( "value" ) );
            OSL_TRACE(  "Trace Message: Functionname is %s\n",
                ::rtl::OUStringToOString( m_ScriptData.functionname,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            break;
        case LOGICALNAME:
            m_ScriptData.logicalname = xAttribs->getValueByName(
               ::rtl::OUString::createFromAscii( "value" ));
#ifdef _DEBUG
            fprintf(stderr, "Trace Message: logicalname is %s\n",
                ::rtl::OUStringToOString( m_ScriptData.logicalname,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
#endif
            break;
        case LANGDEPPROPS:
             m_ScriptData.languagedepprops.push_back(
                ::std::make_pair( xAttribs->getValueByName(
                     ::rtl::OUString::createFromAscii( "name" ) ),
                 xAttribs->getValueByName(
                     ::rtl::OUString::createFromAscii( "value" ) )
                 ));
            OSL_TRACE(  "Trace Message: Langdepprops is %s\t%s\n",
                ::rtl::OUStringToOString( xAttribs->getValueByName(
                   ::rtl::OUString::createFromAscii( "name" ) ),
                   RTL_TEXTENCODING_ASCII_US ).pData->buffer,
                ::rtl::OUStringToOString( xAttribs->getValueByName(
                   ::rtl::OUString::createFromAscii( "value" ) ),
                   RTL_TEXTENCODING_ASCII_US ).pData->buffer );
             break;
        case FILESET:
            ms_filesetname = xAttribs->getValueByName(
               ::rtl::OUString::createFromAscii( "name" ) );
            OSL_TRACE(  "Trace Message: filesetname is %s\n",
                ::rtl::OUStringToOString(ms_filesetname,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            break;
        case FILESETPROPS:
            mv_filesetprops.push_back( ::std::make_pair(
                xAttribs->getValueByName(
                    ::rtl::OUString::createFromAscii( "name" ) ),
                xAttribs->getValueByName(
                    ::rtl::OUString::createFromAscii( "value" ) )
                ));
            OSL_TRACE(  "Trace Message: filesetprops is %s\t%s\n",
                ::rtl::OUStringToOString( xAttribs->getValueByName(
                   ::rtl::OUString::createFromAscii( "name" ) ),
                   RTL_TEXTENCODING_ASCII_US ).pData->buffer,
                ::rtl::OUStringToOString( xAttribs->getValueByName(
                   ::rtl::OUString::createFromAscii( "value" ) ),
                   RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            break;
        case FILES:
            ms_filename = xAttribs->getValueByName(
                ::rtl::OUString::createFromAscii( "name" ) );
            OSL_TRACE(  "Trace Message: filename is %s\n",
                ::rtl::OUStringToOString( ms_filename,
                    RTL_TEXTENCODING_ASCII_US).pData->buffer );
            break;
        case FILEPROPS:
            /**
            mm_files.insert( strpair_pair( ms_filename,
                str_pair( xAttribs->getValueByName(
                    ::rtl::OUString::createFromAscii( "name" ) ),
                xAttribs->getValueByName(
                    ::rtl::OUString::createFromAscii( "value") ) )
                )
            );
            */
            mv_fileprops.push_back(str_pair( xAttribs->getValueByName(
                ::rtl::OUString::createFromAscii( "name" ) ),
                xAttribs->getValueByName(
                ::rtl::OUString::createFromAscii( "value") ) ) );
            OSL_TRACE(  "Trace Message: fileprops is %s\t%s\n",
                ::rtl::OUStringToOString( xAttribs->getValueByName(
                   ::rtl::OUString::createFromAscii( "name" ) ),
                   RTL_TEXTENCODING_ASCII_US ).pData->buffer,
                ::rtl::OUStringToOString( xAttribs->getValueByName(
                   ::rtl::OUString::createFromAscii( "value" ) ),
                   RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            break;

        // to prevent compiler warnings
        case PARCEL:
        case DESCRIPTION:
        case LANGUAGEDEPPROPS:
            break;
    }
}

//*************************************************************************
void ScriptMetadataImporter::endElement( const ::rtl::OUString & aName )
    throw ( xml::sax::SAXException, RuntimeException )
{

    //The end tag of an element
    OSL_TRACE(  "ScriptMetadataImporter: endElement() %s\n",
        ::rtl::OUStringToOString( aName,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer );

    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );

    //Set the state
    setState( aName );


    switch ( m_state )
    {
        case PARCEL:
            break;
        case SCRIPT:
        // np adjust logicalName to be equal to function name
        // as logical name concept has been removed
        m_ScriptData.logicalname = m_ScriptData.functionname;
            mpv_ScriptDatas->push_back( m_ScriptData );
            m_ScriptData =  ScriptData();
            break;
        case LOCALE:
            m_ScriptData.locales[ ms_localeLang ] = ::std::make_pair(
                ms_localeDisName, ms_localeDesc->makeStringAndClear().trim() );
            break;
        case FILESET:
            OSL_TRACE("adding fileset %s to filesets map",
                   ::rtl::OUStringToOString( ms_filesetname,
                   RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            m_ScriptData.filesets[ ms_filesetname ] = ::std::make_pair(
                mv_filesetprops, mm_files );
            mm_files.clear();
            mv_filesetprops.clear();
            break;
        case FILES:
            OSL_TRACE("adding files %s to files map",
                   ::rtl::OUStringToOString( ms_filename,
                   RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            mm_files[ ms_filename ] = mv_fileprops;
            mv_fileprops.clear();
            break;
            //
        // to prevent compiler warnings
        case DISPLAYNAME:
        case DESCRIPTION:
        case FUNCTIONNAME:
        case LOGICALNAME:
        case LANGUAGEDEPPROPS:
        case LANGDEPPROPS:
        case FILESETPROPS:
        case FILEPROPS:
            break;
    }
}

//*************************************************************************
void ScriptMetadataImporter::characters( const ::rtl::OUString & aChars )
    throw ( xml::sax::SAXException, RuntimeException )
{
    OSL_TRACE( "ScriptMetadataImporter: characters()\n");

    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );

    switch ( m_state )
    {
        case DESCRIPTION:
            //Put description into the struct
            ms_localeDesc->append(aChars);
            break;
        case PARCEL:
        case SCRIPT:
        case LOCALE:
        case DISPLAYNAME:
        case FUNCTIONNAME:
        case LOGICALNAME:
        case LANGUAGEDEPPROPS:
        case LANGDEPPROPS:
        case FILESETPROPS:
        case FILEPROPS:
            break;
    }
}

//*************************************************************************
void ScriptMetadataImporter::ignorableWhitespace(
    const ::rtl::OUString & aWhitespaces )
    throw ( xml::sax::SAXException, RuntimeException )
{
    OSL_TRACE( "ScriptMetadataImporter: ignorableWhiteSpace()\n" );
}

//*************************************************************************
void ScriptMetadataImporter::processingInstruction(
    const ::rtl::OUString & aTarget, const ::rtl::OUString & aData )
    throw ( xml::sax::SAXException, RuntimeException )
{
    OSL_TRACE( "ScriptMetadataImporter: processingInstruction()\n" );
}

//*************************************************************************
void ScriptMetadataImporter::setDocumentLocator(
    const Reference< xml::sax::XLocator >& xLocator )
    throw ( xml::sax::SAXException, RuntimeException )
{
    OSL_TRACE( "ScriptMetadataImporter: setDocumentLocator()\n" );
}

//*************************************************************************
void ScriptMetadataImporter::setState( const ::rtl::OUString & tagName )
{
    //Set the state depending on the tag name of the current
    //element the parser has arrived at
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );

    if( tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "parcel" ) ) )
    {
        //Parcel tag
        m_state = PARCEL;
    }
    else if( tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "script" ) ) )
    {
        m_state = SCRIPT;
    }
    else if( tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "locale" ) ) )
    {
        m_state = LOCALE;
    }
    else if( tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "displayname" ) ) )
    {
        m_state = DISPLAYNAME;
    }
    else if( tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "description" ) ) )
    {
        m_state = DESCRIPTION;
    }
    else if( tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "functionname" ) ) )
    {
        m_state = FUNCTIONNAME;
    }
    else if( tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "logicalname" ) ) )
    {
        m_state = LOGICALNAME;
    }
    else if( tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "languagedepprops" ) ) )
    {
        m_state = LANGUAGEDEPPROPS;
    }
    else if( tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "prop" ) ) )
    {
        if( m_state == LANGUAGEDEPPROPS )
        {
            m_state = LANGDEPPROPS;
        }
        else if( m_state == FILESET )
        {
            m_state = FILESETPROPS;
        }
        else if( m_state == FILES )
        {
            m_state = FILEPROPS;
        }
    }
    else if( tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "fileset" ) ) )
    {
        m_state = FILESET;
    }
    else if( tagName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "file" ) ) )
    {
        m_state = FILES;
    }
    else
    {
        //If there is a tag we don't know about, throw a exception (wobbler) :)
        ::rtl::OUString str_sax = ::rtl::OUString::createFromAscii( "No Such Tag" );

        OSL_TRACE(  "ScriptMetadataImporter: No Such Tag: %s\n",
            ::rtl::OUStringToOString(
                tagName, RTL_TEXTENCODING_ASCII_US ).pData->buffer );

        throw xml::sax::SAXException(
            str_sax, Reference< XInterface >(), Any() );
    }
}

}
