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

#include "docprophandler.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include <osl/time.h>

#include <vector>
#include <boost/algorithm/string.hpp>

#include "oox/helper/attributelist.hxx"

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
    if ( !m_aCustomPropertyName.isEmpty() )
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
            OSL_FAIL( "Can not add custom property!" );
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
    if ( !aChars.isEmpty() )
    {
        std::string aUtf8Chars = rtl::OUStringToOString( aChars, RTL_TEXTENCODING_UTF8 ).getStr();
        std::vector<std::string> aUtf8Result;
        boost::split( aUtf8Result, aUtf8Chars, boost::is_any_of(" ,;:\t"), boost::token_compress_on );

        if (!aUtf8Result.empty())
        {
            uno::Sequence< rtl::OUString > aResult( aUtf8Result.size() );
            OUString* pResultValues = aResult.getArray();
            for ( std::vector< std::string >::const_iterator i = aUtf8Result.begin();
                  i != aUtf8Result.end(); ++i, ++pResultValues )
                *pResultValues = rtl::OUString( i->c_str(), static_cast< sal_Int32 >( i->size() ),RTL_TEXTENCODING_UTF8 );

            return aResult;
        }
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
        case EXTPR_TOKEN( Characters ):
            aName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharacterCount" ) );
            break;

        case EXTPR_TOKEN( Pages ):
            aName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PageCount" ) );
            break;

        case EXTPR_TOKEN( Words ):
            aName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "WordCount" ) );
            break;

        case EXTPR_TOKEN( Paragraphs ):
            aName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ParagraphCount" ) );
            break;

        default:
            OSL_FAIL( "Unexpected statistic!" );
            break;
    }

    if ( !aName.isEmpty() )
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
        if ( nElement == COREPR_TOKEN( coreProperties )
          || nElement == EXTPR_TOKEN( Properties )
          || nElement == CUSTPR_TOKEN( Properties ) )
        {
            m_nState = nElement;
        }
        else
        {
                OSL_FAIL( "Unexpected file format!" );
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
    else if ( m_nState && m_nInBlock && m_nInBlock == 2 && getNamespace( nElement ) == NMSP_officeDocPropsVT )
    {
        m_nType = nElement;
    }
    else
    {
        OSL_FAIL( "For now unexpected tags are ignored!" );
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
    OSL_FAIL( ::rtl::OUStringToOString( aUnknown, RTL_TEXTENCODING_UTF8 ).getStr() );

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
            if ( m_nState == COREPR_TOKEN( coreProperties ) )
            {
                switch( m_nBlock )
                {
                    case COREPR_TOKEN( category ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "category" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case COREPR_TOKEN( contentStatus ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "contentStatus" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case COREPR_TOKEN( contentType ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "contentType" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case COREPR_TOKEN( identifier ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "identifier" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case COREPR_TOKEN( version ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "version" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case DCT_TOKEN( created ):
                        if ( aChars.getLength() >= 4 )
                            m_xDocProp->setCreationDate( GetDateTimeFromW3CDTF( aChars ) );
                        break;

                    case DC_TOKEN( creator ):
                        m_xDocProp->setAuthor( aChars );
                        break;

                    case DC_TOKEN( description ):
                        m_xDocProp->setDescription( aChars );
                        break;

                    case COREPR_TOKEN( keywords ):
                        m_xDocProp->setKeywords( GetKeywordsSet( aChars ) );
                        break;

                    case DC_TOKEN( language ):
                        if ( aChars.getLength() >= 2 )
                            m_xDocProp->setLanguage( GetLanguage( aChars ) );
                        break;

                    case COREPR_TOKEN( lastModifiedBy ):
                        m_xDocProp->setModifiedBy( aChars );
                        break;

                    case COREPR_TOKEN( lastPrinted ):
                        if ( aChars.getLength() >= 4 )
                            m_xDocProp->setPrintDate( GetDateTimeFromW3CDTF( aChars ) );
                        break;

                    case DCT_TOKEN( modified ):
                        if ( aChars.getLength() >= 4 )
                            m_xDocProp->setModificationDate( GetDateTimeFromW3CDTF( aChars ) );
                        break;

                    case COREPR_TOKEN( revision ):
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

                    case DC_TOKEN( subject ):
                        m_xDocProp->setSubject( aChars );
                        break;

                    case DC_TOKEN( title ):
                        m_xDocProp->setTitle( aChars );
                        break;

                    default:
                        OSL_FAIL( "Unexpected core property!" );
                }
            }
            else if ( m_nState == EXTPR_TOKEN( Properties ) )
            {
                switch( m_nBlock )
                {
                    case EXTPR_TOKEN( Application ):
                        m_xDocProp->setGenerator( aChars );
                        break;

                    case EXTPR_TOKEN( Template ):
                        m_xDocProp->setTemplateName( aChars );
                        break;

                    case EXTPR_TOKEN( TotalTime ):
                        try
                        {
                            m_xDocProp->setEditingDuration( aChars.toInt32() );
                        }
                        catch (lang::IllegalArgumentException &)
                        {
                            // ignore
                        }
                        break;

                    case EXTPR_TOKEN( Characters ):
                    case EXTPR_TOKEN( Pages ):
                    case EXTPR_TOKEN( Words ):
                    case EXTPR_TOKEN( Paragraphs ):
                        UpdateDocStatistic( aChars );
                        break;

                    case EXTPR_TOKEN( HyperlinksChanged ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HyperlinksChanged" ) );
                        AddCustomProperty( uno::makeAny( aChars.toBoolean() ) ); // the property has boolean type
                        break;

                    case EXTPR_TOKEN( LinksUpToDate ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LinksUpToDate" ) );
                        AddCustomProperty( uno::makeAny( aChars.toBoolean() ) ); // the property has boolean type
                        break;

                    case EXTPR_TOKEN( ScaleCrop ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ScaleCrop" ) );
                        AddCustomProperty( uno::makeAny( aChars.toBoolean() ) ); // the property has boolean type
                        break;

                    case EXTPR_TOKEN( SharedDoc ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ShareDoc" ) );
                        AddCustomProperty( uno::makeAny( aChars.toBoolean() ) ); // the property has boolean type
                        break;

                    case EXTPR_TOKEN( DocSecurity ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DocSecurity" ) );
                        AddCustomProperty( uno::makeAny( aChars.toInt32() ) ); // the property has sal_Int32 type
                        break;

                    case EXTPR_TOKEN( HiddenSlides ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HiddenSlides" ) );
                        AddCustomProperty( uno::makeAny( aChars.toInt32() ) ); // the property has sal_Int32 type
                        break;

                    case EXTPR_TOKEN( MMClips ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MMClips" ) );
                        AddCustomProperty( uno::makeAny( aChars.toInt32() ) ); // the property has sal_Int32 type
                        break;

                    case EXTPR_TOKEN( Notes ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Notes" ) );
                        AddCustomProperty( uno::makeAny( aChars.toInt32() ) ); // the property has sal_Int32 type
                        break;

                    case EXTPR_TOKEN( Slides ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Slides" ) );
                        AddCustomProperty( uno::makeAny( aChars.toInt32() ) ); // the property has sal_Int32 type
                        break;

                    case EXTPR_TOKEN( AppVersion ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AppVersion" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case EXTPR_TOKEN( Company ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Company" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case EXTPR_TOKEN( HyperlinkBase ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HyperlinkBase" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case EXTPR_TOKEN( Manager ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Manager" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case EXTPR_TOKEN( PresentationFormat ):
                        m_aCustomPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PresentationFormat" ) );
                        AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                        break;

                    case EXTPR_TOKEN( CharactersWithSpaces ):
                    case EXTPR_TOKEN( Lines ):
                    case EXTPR_TOKEN( DigSig ):
                    case EXTPR_TOKEN( HeadingPairs ):
                    case EXTPR_TOKEN( HLinks ):
                    case EXTPR_TOKEN( TitlesOfParts ):
                        // ignored during the import currently
                        break;

                    default:
                        OSL_FAIL( "Unexpected extended property!" );
                }
            }
            else if ( m_nState == CUSTPR_TOKEN( Properties ) )
            {
                if ( m_nBlock == CUSTPR_TOKEN( property ) )
                {
                    // this is a custom property
                    switch( m_nType )
                    {
                        case VT_TOKEN( bool ):
                            AddCustomProperty( uno::makeAny( aChars.toBoolean() ) );
                            break;

                        case VT_TOKEN( bstr ):
                        case VT_TOKEN( lpstr ):
                        case VT_TOKEN( lpwstr ):
                            AddCustomProperty( uno::makeAny( AttributeConversion::decodeXString( aChars ) ) ); // the property has string type
                            break;

                        case VT_TOKEN( date ):
                        case VT_TOKEN( filetime ):
                            AddCustomProperty( uno::makeAny( GetDateTimeFromW3CDTF( aChars ) ) );

                        case VT_TOKEN( i1 ):
                        case VT_TOKEN( i2 ):
                            AddCustomProperty( uno::makeAny( (sal_Int16)aChars.toInt32() ) );
                            break;

                        case VT_TOKEN( i4 ):
                        case VT_TOKEN( int ):
                            AddCustomProperty( uno::makeAny( aChars.toInt32() ) );
                            break;

                        case VT_TOKEN( i8 ):
                            AddCustomProperty( uno::makeAny( aChars.toInt64() ) );
                            break;

                        case VT_TOKEN( r4 ):
                            AddCustomProperty( uno::makeAny( aChars.toFloat() ) );
                            break;

                        case VT_TOKEN( r8 ):
                            AddCustomProperty( uno::makeAny( aChars.toDouble() ) );
                            break;

                        default:
                            // all the other types are ignored;
                            break;
                    }
                }
                else
                {
                    OSL_FAIL( "Unexpected tag in custom property!" );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
