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

#include "docprophandler.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include <osl/time.h>

#include "tokens.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"

using namespace ::com::sun::star;

namespace oox {
namespace docprop {

// ------------------------------------------------
OOXMLDocPropHandler::OOXMLDocPropHandler( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< document::XDocumentProperties > xDocProp )
: m_xContext( xContext )
, m_xDocProp( xDocProp )
, m_nState( 0 )
, m_nBlock( 0 )
, m_nType( 0 )
, m_nInBlock( 0 )
{
    if ( !xContext.is() || !xDocProp.is() )
        throw uno::RuntimeException();
}

// ------------------------------------------------
OOXMLDocPropHandler::~OOXMLDocPropHandler()
{
}

// ------------------------------------------------
void OOXMLDocPropHandler::InitNew()
{
    m_nState = 0;
    m_nBlock = 0;
    m_aCustomPropertyName = ::rtl::OUString();
    m_nType = 0;
    m_nInBlock = 0;
}

// ------------------------------------------------
void OOXMLDocPropHandler::AddCustomProperty( const uno::Any& aAny )
{
    if ( m_aCustomPropertyName.getLength() )
    {
        const uno::Reference< beans::XPropertyContainer > xUserProps =
            m_xDocProp->getUserDefinedProperties();
        if ( !xUserProps.is() )
            throw uno::RuntimeException();

        try
        {
            xUserProps->addProperty( m_aCustomPropertyName,
                    beans::PropertyAttribute::REMOVEABLE, aAny );
        }
        catch( beans::PropertyExistException& )
        {
            // conflicts with core and extended properties are possible
        }
        catch( uno::Exception& )
        {
            OSL_ASSERT( "Can not add custom property!" );
        }
    }
}

// ------------------------------------------------
util::DateTime OOXMLDocPropHandler::GetDateTimeFromW3CDTF( const ::rtl::OUString& aChars )
{
    oslDateTime aOslDTime = { 0, 0, 0, 0, 0, 0, 0, 0 };
    sal_Int32 nLen = aChars.getLength();
    if ( nLen >= 4 )
    {
        aOslDTime.Year = (sal_uInt16)aChars.copy( 0, 4 ).toInt32();

        if ( nLen >= 7 && aChars.getStr()[4] == (sal_Unicode)'-' )
        {
            aOslDTime.Month = (sal_uInt16)aChars.copy( 5, 2 ).toInt32();

            if ( nLen >= 10 && aChars.getStr()[7] == (sal_Unicode)'-' )
            {
                aOslDTime.Day = (sal_uInt16)aChars.copy( 8, 2 ).toInt32();

                if ( nLen >= 16 && aChars.getStr()[10] == (sal_Unicode)'T' && aChars.getStr()[13] == (sal_Unicode)':' )
                {
                    aOslDTime.Hours = (sal_uInt16)aChars.copy( 11, 2 ).toInt32();
                    aOslDTime.Minutes = (sal_uInt16)aChars.copy( 14, 2 ).toInt32();

                    sal_Int32 nOptTime = 0;
                    if ( nLen >= 19 && aChars.getStr()[16] == (sal_Unicode)':' )
                    {
                        aOslDTime.Seconds = (sal_uInt16)aChars.copy( 17, 2 ).toInt32();
                        nOptTime += 3;
                        if ( nLen >= 21 && aChars.getStr()[19] == (sal_Unicode)'.' )
                        {
                            aOslDTime.NanoSeconds = (sal_uInt32)(aChars.copy( 20, 1 ).toInt32() * 10e8);
                            nOptTime += 2;
                        }
                    }

                    sal_Int32 nModif = 0;
                    if ( nLen >= 16 + nOptTime + 6 )
                    {
                        if ( ( aChars.getStr()[16 + nOptTime] == (sal_Unicode)'+' || aChars.getStr()[16 + nOptTime] == (sal_Unicode)'-' )
                          && aChars.getStr()[16 + nOptTime + 3] == (sal_Unicode)':' )

                        {
                            nModif = aChars.copy( 16 + nOptTime + 1, 2 ).toInt32() * 3600;
                            nModif += aChars.copy( 16 + nOptTime + 4, 2 ).toInt32() * 60;
                            if ( aChars.getStr()[16 + nOptTime] == (sal_Unicode)'-' )
                                nModif *= -1;
                        }
                    }

                    if ( nModif )
                    {
                        // convert to UTC time
                        TimeValue aTmp;
                        if ( osl_getTimeValueFromDateTime( &aOslDTime, &aTmp ) )
                        {
                            aTmp.Seconds += nModif;
                            osl_getDateTimeFromTimeValue( &aTmp, &aOslDTime );
                        }
                    }
                }
            }
        }
    }

    return util::DateTime( (sal_uInt16)( aOslDTime.NanoSeconds / 1e7 ), aOslDTime.Seconds, aOslDTime.Minutes, aOslDTime.Hours, aOslDTime.Day, aOslDTime.Month, aOslDTime.Year );
}

// ------------------------------------------------
uno::Sequence< ::rtl::OUString > OOXMLDocPropHandler::GetKeywordsSet( const ::rtl::OUString& aChars )
{
    if ( aChars.getLength() )
    {
        uno::Sequence< ::rtl::OUString > aResult( 20 );
        sal_Int32 nCounter = 0;

        const sal_Unicode* pStr = aChars.getStr();
        for( sal_Int32 nInd = 0; nInd < aChars.getLength() && pStr[nInd] != 0; nInd++ )
        {
            switch( pStr[nInd] )
            {
                case (sal_Unicode)' ':
                case (sal_Unicode)',':
                case (sal_Unicode)';':
                case (sal_Unicode)':':
                case (sal_Unicode)'\t':
                    // this is a delimiter
                    // unfortunately I did not find any specification for the possible delimiters
                    if ( aResult[nCounter].getLength() )
                    {
                        if ( nCounter >= aResult.getLength() )
                            aResult.realloc( nCounter + 10 );
                        nCounter++;
                    }
                    break;

                default:
                    // this should be a part of keyword
                        aResult[nCounter] += ::rtl::OUString( (sal_Unicode)pStr[nInd] );
            }
        }

        aResult.realloc( nCounter + 1 );
        return aResult;
    }

    return uno::Sequence< ::rtl::OUString >();
}
// ------------------------------------------------
lang::Locale OOXMLDocPropHandler::GetLanguage( const ::rtl::OUString& aChars )
{
    lang::Locale aResult;
    if ( aChars.getLength() >= 2 )
    {
        aResult.Language = aChars.copy( 0, 2 );
        if ( aChars.getLength() >= 5 && aChars.getStr()[2] == (sal_Unicode)'-' )
            aResult.Country = aChars.copy( 3, 2 );

        // TODO/LATER: the variant could be also detected
    }

    return aResult;
}

// ------------------------------------------------
void OOXMLDocPropHandler::UpdateDocStatistic( const ::rtl::OUString& aChars )
{
    uno::Sequence< beans::NamedValue > aSet = m_xDocProp->getDocumentStatistics();
    ::rtl::OUString aName;

    switch( m_nBlock )
    {
        case XML_Characters|NMSP_EXTPR:
            aName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharacterCount" ) );
            break;

        case XML_Pages|NMSP_EXTPR:
            aName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PageCount" ) );
            break;

        case XML_Words|NMSP_EXTPR:
            aName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "WordCount" ) );
            break;

        case XML_Paragraphs|NMSP_EXTPR:
            aName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ParagraphCount" ) );
            break;

        default:
            OSL_ASSERT( "Unexpected statistic!" );
            break;
    }

    if ( aName.getLength() )
    {
        sal_Bool bFound = sal_False;
        sal_Int32 nLen = aSet.getLength();
        for ( sal_Int32 nInd = 0; nInd < nLen; nInd++ )
            if ( aSet[nInd].Name.equals( aName ) )
            {
                aSet[nInd].Value = uno::makeAny( aChars.toInt32() );
                bFound = sal_True;
                break;
            }

        if ( !bFound )
        {
            aSet.realloc( nLen + 1 );
            aSet[nLen].Name = aName;
            aSet[nLen].Value = uno::makeAny( aChars.toInt32() );
        }

        m_xDocProp->setDocumentStatistics( aSet );
    }
}

// ------------------------------------------------
// com.sun.star.xml.sax.XFastDocumentHandler
// ------------------------------------------------
void SAL_CALL OOXMLDocPropHandler::startDocument()
    throw (xml::sax::SAXException, uno::RuntimeException)
{
}

// ------------------------------------------------
void SAL_CALL OOXMLDocPropHandler::endDocument()
    throw (xml::sax::SAXException, uno::RuntimeException)
{
    InitNew();
}

// ------------------------------------------------
void SAL_CALL OOXMLDocPropHandler::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& )
    throw (xml::sax::SAXException, uno::RuntimeException)
{
}


// com.sun.star.xml.sax.XFastContextHandler
// ------------------------------------------------
void SAL_CALL OOXMLDocPropHandler::startFastElement( ::sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs )
    throw (xml::sax::SAXException, uno::RuntimeException)
{
    if ( !m_nInBlock && !m_nState )
    {
        if ( nElement == ( XML_coreProperties|NMSP_COREPR )
          || nElement == ( XML_Properties|NMSP_EXTPR )
          || nElement == ( XML_Properties|NMSP_CUSTPR ) )
        {
            m_nState = nElement;
        }
        else
        {
                OSL_ASSERT( "Unexpected file format!" );
        }
    }
    else if ( m_nState && m_nInBlock == 1 ) // that tag should contain the property name
    {
        // Currently the attributes are ignored for the core properties since the only
        // known attribute is xsi:type that can only be used with dcterms:created and
        // dcterms:modified, and this element is allowed currently to have only one value dcterms:W3CDTF
        m_nBlock = nElement;

        if ( xAttribs.is() && xAttribs->hasAttribute( XML_name ) )
            m_aCustomPropertyName = xAttribs->getValue( XML_name );
    }
    else if ( m_nState && m_nInBlock && m_nInBlock == 2 && ( nElement >> 16 ) == ( NMSP_VT >> 16 ) )
    {
        m_nType = nElement;
    }
    else
    {
        OSL_ASSERT( "For now unexpected tags are ignored!" );
    }

    if ( m_nInBlock == SAL_MAX_INT32 )
        throw uno::RuntimeException();

    m_nInBlock++;
}

// ------------------------------------------------
void SAL_CALL OOXMLDocPropHandler::startUnknownElement( const ::rtl::OUString& aNamespace, const ::rtl::OUString& aName, const uno::Reference< xml::sax::XFastAttributeList >& )
    throw (xml::sax::SAXException, uno::RuntimeException)
{
    ::rtl::OUString aUnknown = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unknown element" ) );
    aUnknown += aNamespace;
    aUnknown += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":" ) );
    aUnknown += aName;
    OSL_ASSERT( ::rtl::OUStringToOString( aUnknown, RTL_TEXTENCODING_UTF8 ).getStr() );

    if ( m_nInBlock == SAL_MAX_INT32 )
        throw uno::RuntimeException();

    m_nInBlock++;
}

// ------------------------------------------------
void SAL_CALL OOXMLDocPropHandler::endFastElement( ::sal_Int32 )
    throw (xml::sax::SAXException, uno::RuntimeException)
{
    if ( m_nInBlock )
    {
        m_nInBlock--;

        if ( !m_nInBlock )
            m_nState = 0;
        else if ( m_nInBlock == 1 )
        {
            m_nBlock = 0;
            m_aCustomPropertyName = ::rtl::OUString();
        }
        else if ( m_nInBlock == 2 )
            m_nType = 0;
    }
}

// ------------------------------------------------
void SAL_CALL OOXMLDocPropHandler::endUnknownElement( const ::rtl::OUString&, const ::rtl::OUString& )
    throw (xml::sax::SAXException, uno::RuntimeException)
{
    if ( m_nInBlock )
        m_nInBlock--;
}

// ------------------------------------------------
uno::Reference< xml::sax::XFastContextHandler > SAL_CALL OOXMLDocPropHandler::createFastChildContext( ::sal_Int32, const uno::Reference< xml::sax::XFastAttributeList >& )
    throw (xml::sax::SAXException, uno::RuntimeException)
{
    // Should the arguments be parsed?
    return uno::Reference< xml::sax::XFastContextHandler >( static_cast< xml::sax::XFastContextHandler* >( this ) );
}

// ------------------------------------------------
uno::Reference< xml::sax::XFastContextHandler > SAL_CALL OOXMLDocPropHandler::createUnknownChildContext( const ::rtl::OUString&, const ::rtl::OUString&, const uno::Reference< xml::sax::XFastAttributeList >& )
    throw (xml::sax::SAXException, uno::RuntimeException)
{
    return uno::Reference< xml::sax::XFastContextHandler >( static_cast< xml::sax::XFastContextHandler* >( this ) );
}

// ------------------------------------------------
void SAL_CALL OOXMLDocPropHandler::characters( const ::rtl::OUString& aChars )
    throw (xml::sax::SAXException, uno::RuntimeException)
{
    try
    {
        if ( (m_nInBlock == 2) || ((m_nInBlock == 3) && m_nType) )
        {
            if ( m_nState == ( XML_coreProperties|NMSP_COREPR ) )
            {
                switch( m_nBlock )
                {
                    case XML_category|NMSP_COREPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "category" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case XML_contentStatus|NMSP_COREPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "contentStatus" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case XML_contentType|NMSP_COREPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "contentType" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case XML_identifier|NMSP_COREPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "identifier" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case XML_version|NMSP_COREPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "version" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case XML_created|NMSP_DCTERMS:
                        if ( aChars.getLength() >= 4 )
                            m_xDocProp->setCreationDate( GetDateTimeFromW3CDTF( aChars ) );
                        break;

                    case XML_creator|NMSP_DC:
                        m_xDocProp->setAuthor( aChars );
                        break;

                    case XML_description|NMSP_DC:
                        m_xDocProp->setDescription( aChars );
                        break;

                    case XML_keywords|NMSP_COREPR:
                        m_xDocProp->setKeywords( GetKeywordsSet( aChars ) );
                        break;

                    case XML_language|NMSP_DC:
                        if ( aChars.getLength() >= 2 )
                            m_xDocProp->setLanguage( GetLanguage( aChars ) );
                        break;

                    case XML_lastModifiedBy|NMSP_COREPR:
                        m_xDocProp->setModifiedBy( aChars );
                        break;

                    case XML_lastPrinted|NMSP_COREPR:
                        if ( aChars.getLength() >= 4 )
                            m_xDocProp->setPrintDate( GetDateTimeFromW3CDTF( aChars ) );
                        break;

                    case XML_modified|NMSP_DCTERMS:
                        if ( aChars.getLength() >= 4 )
                            m_xDocProp->setModificationDate( GetDateTimeFromW3CDTF( aChars ) );
                        break;

                    case XML_revision|NMSP_COREPR:
                        try
                        {
                            m_xDocProp->setEditingCycles(
                                static_cast<sal_Int16>(aChars.toInt32()) );
                        }
                        catch (lang::IllegalArgumentException &)
                        {
                            // ignore
                        }
                        break;

                    case XML_subject|NMSP_DC:
                        m_xDocProp->setSubject( aChars );
                        break;

                    case XML_title|NMSP_DC:
                        m_xDocProp->setTitle( aChars );
                        break;

                    default:
                        OSL_ASSERT( "Unexpected core property!" );
                }
            }
            else if ( m_nState == ( XML_Properties|NMSP_EXTPR ) )
            {
                switch( m_nBlock )
                {
                    case XML_Application|NMSP_EXTPR:
                        m_xDocProp->setGenerator( aChars );
                        break;

                    case XML_Template|NMSP_EXTPR:
                        m_xDocProp->setTemplateName( aChars );
                        break;

                    case XML_TotalTime|NMSP_EXTPR:
                        try
                        {
                            m_xDocProp->setEditingDuration( aChars.toInt32() );
                        }
                        catch (lang::IllegalArgumentException &)
                        {
                            // ignore
                        }
                        break;

                    case XML_Characters|NMSP_EXTPR:
                    case XML_Pages|NMSP_EXTPR:
                    case XML_Words|NMSP_EXTPR:
                    case XML_Paragraphs|NMSP_EXTPR:
                        UpdateDocStatistic( aChars );
                        break;

                    case XML_HyperlinksChanged|NMSP_EXTPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HyperlinksChanged" ) );
                        AddCustomProperty( uno::makeAny( aChars.toBoolean() ) ); // the property has boolean type
                        break;

                    case XML_LinksUpToDate|NMSP_EXTPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LinksUpToDate" ) );
                        AddCustomProperty( uno::makeAny( aChars.toBoolean() ) ); // the property has boolean type
                        break;

                    case XML_ScaleCrop|NMSP_EXTPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ScaleCrop" ) );
                        AddCustomProperty( uno::makeAny( aChars.toBoolean() ) ); // the property has boolean type
                        break;

                    case XML_SharedDoc|NMSP_EXTPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ShareDoc" ) );
                        AddCustomProperty( uno::makeAny( aChars.toBoolean() ) ); // the property has boolean type
                        break;

                    case XML_DocSecurity|NMSP_EXTPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DocSecurity" ) );
                        AddCustomProperty( uno::makeAny( aChars.toInt32() ) ); // the property has sal_Int32 type
                        break;

                    case XML_HiddenSlides|NMSP_EXTPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HiddenSlides" ) );
                        AddCustomProperty( uno::makeAny( aChars.toInt32() ) ); // the property has sal_Int32 type
                        break;

                    case XML_MMClips|NMSP_EXTPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MMClips" ) );
                        AddCustomProperty( uno::makeAny( aChars.toInt32() ) ); // the property has sal_Int32 type
                        break;

                    case XML_Notes|NMSP_EXTPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Notes" ) );
                        AddCustomProperty( uno::makeAny( aChars.toInt32() ) ); // the property has sal_Int32 type
                        break;

                    case XML_Slides|NMSP_EXTPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Slides" ) );
                        AddCustomProperty( uno::makeAny( aChars.toInt32() ) ); // the property has sal_Int32 type
                        break;

                    case XML_AppVersion|NMSP_EXTPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AppVersion" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case XML_Company|NMSP_EXTPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Company" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case XML_HyperlinkBase|NMSP_EXTPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HyperlinkBase" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case XML_Manager|NMSP_EXTPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Manager" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case XML_PresentationFormat|NMSP_EXTPR:
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PresentationFormat" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case XML_CharactersWithSpaces|NMSP_EXTPR:
                    case XML_Lines|NMSP_EXTPR:
                    case XML_DigSig|NMSP_EXTPR:
                    case XML_HeadingPairs|NMSP_EXTPR:
                    case XML_HLinks|NMSP_EXTPR:
                    case XML_TitlesOfParts|NMSP_EXTPR:
                        // ignored during the import currently
                        break;

                    default:
                        OSL_ASSERT( "Unexpected extended property!" );
                }
            }
            else if ( m_nState == ( XML_Properties|NMSP_CUSTPR ) )
            {
                if ( m_nBlock == ( XML_property|NMSP_CUSTPR ) )
                {
                    // this is a custom property
                    switch( m_nType )
                    {
                        case XML_bool|NMSP_VT:
                            AddCustomProperty( uno::makeAny( aChars.toBoolean() ) );
                            break;

                        case XML_bstr|NMSP_VT:
                        case XML_lpstr|NMSP_VT:
                        case XML_lpwstr|NMSP_VT:
                            AddCustomProperty( uno::makeAny( AttributeConversion::decodeXString( aChars ) ) ); // the property has string type
                            break;

                        case XML_date|NMSP_VT:
                        case XML_filetime|NMSP_VT:
                            AddCustomProperty( uno::makeAny( GetDateTimeFromW3CDTF( aChars ) ) );

                        case XML_i1|NMSP_VT:
                        case XML_i2|NMSP_VT:
                            AddCustomProperty( uno::makeAny( (sal_Int16)aChars.toInt32() ) );
                            break;

                        case XML_i4|NMSP_VT:
                        case XML_int|NMSP_VT:
                            AddCustomProperty( uno::makeAny( aChars.toInt32() ) );
                            break;

                        case XML_i8|NMSP_VT:
                            AddCustomProperty( uno::makeAny( aChars.toInt64() ) );
                            break;

                        case XML_r4|NMSP_VT:
                            AddCustomProperty( uno::makeAny( aChars.toFloat() ) );
                            break;

                        case XML_r8|NMSP_VT:
                            AddCustomProperty( uno::makeAny( aChars.toDouble() ) );
                            break;

                        default:
                            // all the other types are ignored;
                            break;
                    }
                }
                else
                {
                    OSL_ASSERT( "Unexpected tag in custom property!" );
                }
            }
        }
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( xml::sax::SAXException& )
    {
        throw;
    }
    catch( uno::Exception& e )
    {
        throw xml::sax::SAXException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Error while setting document property!" ) ),
            uno::Reference< uno::XInterface >(),
            uno::makeAny( e ) );
    }
}

// ------------------------------------------------
void SAL_CALL OOXMLDocPropHandler::ignorableWhitespace( const ::rtl::OUString& )
    throw (xml::sax::SAXException, uno::RuntimeException)
{
}

// ------------------------------------------------
void SAL_CALL OOXMLDocPropHandler::processingInstruction( const ::rtl::OUString&, const ::rtl::OUString& )
    throw (xml::sax::SAXException, uno::RuntimeException)
{
}

} // namespace docprop
} // namespace oox

