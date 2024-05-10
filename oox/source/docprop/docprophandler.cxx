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
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <o3tl/string_view.hxx>

#include <o3tl/safeint.hxx>
#include <osl/time.h>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <vector>
#include <boost/algorithm/string.hpp>

#include <oox/helper/attributelist.hxx>

using namespace ::com::sun::star;

namespace oox::docprop {

OOXMLDocPropHandler::OOXMLDocPropHandler( const uno::Reference< uno::XComponentContext >& xContext,
                                          const uno::Reference< document::XDocumentProperties >& rDocProp )
    : m_xDocProp( rDocProp )
    , m_nState( 0 )
    , m_nBlock( 0 )
    , m_nType( 0 )
    , m_nInBlock( 0 )
    , m_CustomStringPropertyState(NONE)
{
    if ( !xContext.is() || !rDocProp.is() )
        throw uno::RuntimeException();
}

OOXMLDocPropHandler::~OOXMLDocPropHandler()
{
}

void OOXMLDocPropHandler::InitNew()
{
    m_nState = 0;
    m_nBlock = 0;
    m_aCustomPropertyName.clear();
    m_nType = 0;
    m_nInBlock = 0;
    m_CustomStringPropertyState = NONE;
}

void OOXMLDocPropHandler::AddCustomProperty( const uno::Any& aAny )
{
    if ( m_aCustomPropertyName.isEmpty() )
        return;

    const uno::Reference< beans::XPropertyContainer > xUserProps =
        m_xDocProp->getUserDefinedProperties();
    if ( !xUserProps.is() )
        throw uno::RuntimeException();

    try
    {
        xUserProps->addProperty( m_aCustomPropertyName,
                beans::PropertyAttribute::REMOVABLE, aAny );
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

util::DateTime OOXMLDocPropHandler::GetDateTimeFromW3CDTF( std::u16string_view aChars )
{
    oslDateTime aOslDTime = { 0, 0, 0, 0, 0, 0, 0, 0 };
    const size_t nLen = aChars.size();
    if ( nLen >= 4 )
    {
        aOslDTime.Year = static_cast<sal_Int16>(o3tl::toInt32(aChars.substr( 0, 4 )));

        if ( nLen >= 7 && aChars[4] == '-' )
        {
            aOslDTime.Month = static_cast<sal_uInt16>(o3tl::toInt32(aChars.substr( 5, 2 )));

            if ( nLen >= 10 && aChars[7] == '-' )
            {
                aOslDTime.Day = static_cast<sal_uInt16>(o3tl::toInt32(aChars.substr( 8, 2 )));

                if ( nLen >= 16 && aChars[10] == 'T' && aChars[13] == ':' )
                {
                    aOslDTime.Hours = static_cast<sal_uInt16>(o3tl::toInt32(aChars.substr( 11, 2 )));
                    aOslDTime.Minutes = static_cast<sal_uInt16>(o3tl::toInt32(aChars.substr( 14, 2 )));

                    size_t nOptTime = 0;
                    if ( nLen >= 19 && aChars[16] == ':' )
                    {
                        aOslDTime.Seconds = static_cast<sal_uInt16>(o3tl::toInt32(aChars.substr( 17, 2 )));
                        nOptTime += 3;
                        if ( nLen >= 20 && aChars[19] == '.' )
                        {
                            nOptTime += 1;
                            size_t digitPos = 20;
                            while (nLen > digitPos && digitPos < 29)
                            {
                                sal_Unicode c = aChars[digitPos];
                                if ( c < '0' || c > '9')
                                    break;
                                aOslDTime.NanoSeconds *= 10;
                                aOslDTime.NanoSeconds += c - '0';
                                ++digitPos;
                            }
                            if ( digitPos < 29 )
                            {
                                // read less digits than 9
                                // add correct exponent of 10
                                nOptTime += digitPos - 20;
                                for(; digitPos<29; ++digitPos)
                                {
                                    aOslDTime.NanoSeconds *= 10;
                                }
                            }
                            else
                            {
                                //skip digits with more precision than we can handle
                                while(nLen > digitPos)
                                {
                                    sal_Unicode c = aChars[digitPos];
                                    if ( c < '0' || c > '9')
                                        break;
                                    ++digitPos;
                                }
                                nOptTime += digitPos - 20;
                            }
                        }
                    }

                    sal_Int32 nModif = 0;
                    if ( nLen >= 16 + nOptTime + 6 )
                    {
                        if ( ( aChars[16 + nOptTime] == '+' || aChars[16 + nOptTime] == '-' )
                          && aChars[16 + nOptTime + 3] == ':' )
                        {
                            nModif = o3tl::toInt32(aChars.substr( 16 + nOptTime + 1, 2 )) * 3600;
                            nModif += o3tl::toInt32(aChars.substr( 16 + nOptTime + 4, 2 )) * 60;
                            if ( aChars[16 + nOptTime] == '-' )
                                nModif *= -1;
                        }
                    }

                    if ( nModif )
                    {
                        // convert to UTC time
                        TimeValue aTmp;
                        if ( osl_getTimeValueFromDateTime( &aOslDTime, &aTmp ) )
                        {
                            aTmp.Seconds -= nModif;
                            osl_getDateTimeFromTimeValue( &aTmp, &aOslDTime );
                        }
                    }
                }
            }
        }
    }

    return util::DateTime( aOslDTime.NanoSeconds, aOslDTime.Seconds,
            aOslDTime.Minutes, aOslDTime.Hours,
            aOslDTime.Day, aOslDTime.Month, aOslDTime.Year, false);
}

uno::Sequence< OUString > OOXMLDocPropHandler::GetKeywordsSet( std::u16string_view aChars )
{
    if ( !aChars.empty() )
    {
        std::string aUtf8Chars( OUStringToOString( aChars, RTL_TEXTENCODING_UTF8 ) );
        std::vector<std::string> aUtf8Result;
        boost::split( aUtf8Result, aUtf8Chars, boost::is_any_of(" ,;:\t"), boost::token_compress_on );

        if (!aUtf8Result.empty())
        {
            uno::Sequence< OUString > aResult( aUtf8Result.size() );
            OUString* pResultValues = aResult.getArray();
            for (auto const& elem : aUtf8Result)
            {
                *pResultValues = OUString( elem.c_str(), static_cast< sal_Int32 >( elem.size() ),RTL_TEXTENCODING_UTF8 );
                ++pResultValues;
            }

            return aResult;
        }
    }
    return uno::Sequence< OUString >();
}

void OOXMLDocPropHandler::UpdateDocStatistic( std::u16string_view aChars )
{
    uno::Sequence< beans::NamedValue > aSet = m_xDocProp->getDocumentStatistics();
    OUString aName;

    switch( m_nBlock )
    {
    case EXTPR_TOKEN( Characters ):
        aName = "NonWhitespaceCharacterCount";
        break;

    case EXTPR_TOKEN( CharactersWithSpaces ):
        aName = "CharacterCount";
        break;

    case EXTPR_TOKEN( Pages ):
        aName = "PageCount";
        break;

    case EXTPR_TOKEN( Words ):
        aName = "WordCount";
        break;

    case EXTPR_TOKEN( Paragraphs ):
        aName = "ParagraphCount";
        break;

    default:
        OSL_FAIL( "Unexpected statistic!" );
        break;
    }

    if ( aName.isEmpty() )
        return;

    sal_Int32 nInd = 0;
    for ( auto pProp = aSet.getConstArray(); nInd < aSet.getLength(); ++nInd )
        if ( pProp[nInd].Name == aName )
            break;

    if (nInd == aSet.getLength())
        aSet.realloc( nInd + 1 );

    aSet.getArray()[nInd] = { aName, uno::Any(o3tl::toInt32(aChars)) };

    m_xDocProp->setDocumentStatistics( aSet );
}

// com.sun.star.xml.sax.XFastDocumentHandler

void SAL_CALL OOXMLDocPropHandler::startDocument()
{
}

void SAL_CALL OOXMLDocPropHandler::endDocument()
{
    InitNew();
}

void OOXMLDocPropHandler::processingInstruction( const OUString& /*rTarget*/, const OUString& /*rData*/ )
{
}

void SAL_CALL OOXMLDocPropHandler::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& )
{
}

// com.sun.star.xml.sax.XFastContextHandler

void SAL_CALL OOXMLDocPropHandler::startFastElement( ::sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs )
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
    else if ( m_nState && m_nInBlock == 2 && getNamespace( nElement ) == NMSP_officeDocPropsVT )
    {
        m_nType = nElement;
    }
    // variant tags in vector
    else if ( m_nState && m_nInBlock == 3 && getNamespace( nElement ) == NMSP_officeDocPropsVT )
    {
        m_nType = nElement;
    }
    // lpstr or i4 tags in vector
    else if ( m_nState && m_nInBlock == 4 && getNamespace( nElement ) == NMSP_officeDocPropsVT )
    {
        m_nType = nElement;
    }
    else
    {
        SAL_WARN("oox", "OOXMLDocPropHandler::startFastElement: unknown element " << getBaseToken(nElement) << " m_nState=" << m_nState << " m_nInBlock=" << m_nInBlock);
    }

    if ( m_nInBlock == SAL_MAX_INT32 )
        throw uno::RuntimeException();

    m_nInBlock++;
}

void SAL_CALL OOXMLDocPropHandler::startUnknownElement( const OUString& aNamespace, const OUString& aName, const uno::Reference< xml::sax::XFastAttributeList >& )
{
    SAL_WARN("oox", "Unknown element " << aNamespace << ":" << aName);

    if ( m_nInBlock == SAL_MAX_INT32 )
        throw uno::RuntimeException();

    m_nInBlock++;
}

void SAL_CALL OOXMLDocPropHandler::endFastElement( ::sal_Int32 )
{
    if ( !m_nInBlock )
        return;

    m_nInBlock--;

    if ( !m_nInBlock )
        m_nState = 0;
    else if ( m_nInBlock == 1 )
    {
        m_nBlock = 0;
        m_aCustomPropertyName.clear();
    }
    else if ( m_nInBlock == 2 )
    {
        if (   m_nState == CUSTPR_TOKEN(Properties)
            && m_nBlock == CUSTPR_TOKEN(property))
        {
            switch (m_nType)
            {
                case VT_TOKEN(bstr):
                case VT_TOKEN(lpstr):
                case VT_TOKEN(lpwstr):
                    if (!m_aCustomPropertyName.isEmpty() &&
                        INSERTED != m_CustomStringPropertyState)
                    {
                        // the property has string type, so it is valid
                        // even with an empty value - characters() has
                        // not been called in that case
                        AddCustomProperty(uno::Any(OUString()));
                    }
                break;
            }
        }
        m_CustomStringPropertyState = NONE;
        m_nType = 0;
    }
}

void SAL_CALL OOXMLDocPropHandler::endUnknownElement( const OUString&, const OUString& )
{
    if ( m_nInBlock )
        m_nInBlock--;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL OOXMLDocPropHandler::createFastChildContext( ::sal_Int32, const uno::Reference< xml::sax::XFastAttributeList >& )
{
    // Should the arguments be parsed?
    return uno::Reference< xml::sax::XFastContextHandler >( static_cast< xml::sax::XFastContextHandler* >( this ) );
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL OOXMLDocPropHandler::createUnknownChildContext( const OUString&, const OUString&, const uno::Reference< xml::sax::XFastAttributeList >& )
{
    return uno::Reference< xml::sax::XFastContextHandler >( static_cast< xml::sax::XFastContextHandler* >( this ) );
}

void SAL_CALL OOXMLDocPropHandler::characters( const OUString& aChars )
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
                    m_aCustomPropertyName = "OOXMLCorePropertyCategory";
                    AddCustomProperty( uno::Any( aChars ) ); // the property has string type
                    break;

                case COREPR_TOKEN( contentStatus ):
                    m_aCustomPropertyName = "OOXMLCorePropertyContentStatus";
                    AddCustomProperty( uno::Any( aChars ) ); // the property has string type
                    break;

                case COREPR_TOKEN( contentType ):
                    m_aCustomPropertyName = "OOXMLCorePropertyContentType";
                    AddCustomProperty( uno::Any( aChars ) ); // the property has string type
                    break;

                case DC_TOKEN( identifier ):
                    m_aCustomPropertyName = "OOXMLCorePropertyIdentifier";
                    AddCustomProperty( uno::Any( aChars ) ); // the property has string type
                    break;

                case COREPR_TOKEN( version ):
                    m_aCustomPropertyName = "OOXMLCorePropertyVersion";
                    AddCustomProperty( uno::Any( aChars ) ); // the property has string type
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
                        m_xDocProp->setLanguage( LanguageTag::convertToLocale( aChars) );
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
                    m_xDocProp->setSubject( m_xDocProp->getSubject() + aChars );
                    break;

                case DC_TOKEN( title ):
                    m_xDocProp->setTitle( m_xDocProp->getTitle() + aChars );
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
                {
                    sal_Int32 nDuration;
                    if (!o3tl::checked_multiply<sal_Int32>(aChars.toInt32(), 60, nDuration))
                    {
                        try
                        {
                            // The TotalTime is in mins as per ECMA specification.
                            m_xDocProp->setEditingDuration(nDuration);
                        }
                        catch (const lang::IllegalArgumentException&)
                        {
                            // ignore
                        }
                    }
                    break;
                }
                case EXTPR_TOKEN( Characters ):
                case EXTPR_TOKEN( CharactersWithSpaces ):
                case EXTPR_TOKEN( Pages ):
                case EXTPR_TOKEN( Words ):
                case EXTPR_TOKEN( Paragraphs ):
                    UpdateDocStatistic( aChars );
                break;

                case EXTPR_TOKEN( HyperlinksChanged ):
                    m_aCustomPropertyName = "HyperlinksChanged";
                    // tdf#103987 Don't create custom property if the value is default
                    if ( aChars.toBoolean() )
                        AddCustomProperty( uno::Any( aChars.toBoolean() ) ); // the property has boolean type
                    break;

                case EXTPR_TOKEN( LinksUpToDate ):
                    m_aCustomPropertyName = "LinksUpToDate";
                    // tdf#103987 Don't create custom property if the value is default
                    if ( aChars.toBoolean() )
                        AddCustomProperty( uno::Any( aChars.toBoolean() ) ); // the property has boolean type
                    break;

                case EXTPR_TOKEN( ScaleCrop ):
                    m_aCustomPropertyName = "ScaleCrop";
                    // tdf#103987 Don't create custom property if the value is default
                    if ( aChars.toBoolean() )
                        AddCustomProperty( uno::Any( aChars.toBoolean() ) ); // the property has boolean type
                    break;

                case EXTPR_TOKEN( SharedDoc ):
                    m_aCustomPropertyName = "ShareDoc";
                    // tdf#103987 Don't create custom property if the value is default
                    if ( aChars.toBoolean() )
                        AddCustomProperty( uno::Any( aChars.toBoolean() ) ); // the property has boolean type
                    break;

                case EXTPR_TOKEN( DocSecurity ):
                    m_aCustomPropertyName = "DocSecurity";
                    // tdf#103987 Don't create custom property if the value is default
                    // OOXTODO Instead of storing value, enable security
                    // 1 - password protected, 2 - recommended read-only
                    // 4 - enforced read-only, 8 - locked for annotation
                    if ( aChars.toInt32() != 0 )
                        AddCustomProperty( uno::Any( aChars.toInt32() ) ); // the property has sal_Int32 type
                    break;

                case EXTPR_TOKEN( HiddenSlides ):
                    m_aCustomPropertyName = "HiddenSlides";
                    // tdf#103987 Don't create custom property if the value is default
                    if ( aChars.toInt32() != 0 )
                        AddCustomProperty( uno::Any( aChars.toInt32() ) ); // the property has sal_Int32 type
                    break;

                case EXTPR_TOKEN( MMClips ):
                    m_aCustomPropertyName = "MMClips";
                    // tdf#103987 Don't create custom property if the value is default
                    if ( aChars.toInt32() != 0 )
                        AddCustomProperty( uno::Any( aChars.toInt32() ) ); // the property has sal_Int32 type
                    break;

                case EXTPR_TOKEN( Notes ):
                    m_aCustomPropertyName = "Notes";
                    // tdf#103987 Don't create custom property if the value is default
                    if ( aChars.toInt32() != 0 )
                        AddCustomProperty( uno::Any( aChars.toInt32() ) ); // the property has sal_Int32 type
                    break;

                case EXTPR_TOKEN( Slides ):
                    m_aCustomPropertyName = "Slides";
                    // tdf#103987 Don't create custom property if the value is default
                    if ( aChars.toInt32() != 0 )
                        AddCustomProperty( uno::Any( aChars.toInt32() ) ); // the property has sal_Int32 type
                    break;

                case EXTPR_TOKEN( AppVersion ):
                    m_aCustomPropertyName = "AppVersion";
                    AddCustomProperty( uno::Any( aChars ) ); // the property has string type
                    break;

                case EXTPR_TOKEN( Company ):
                    m_aCustomPropertyName = "Company";
                    AddCustomProperty( uno::Any( aChars ) ); // the property has string type
                    break;

                case EXTPR_TOKEN( HyperlinkBase ):
                    m_aCustomPropertyName = "HyperlinkBase";
                    AddCustomProperty( uno::Any( aChars ) ); // the property has string type
                    break;

                case EXTPR_TOKEN( Manager ):
                    m_aCustomPropertyName = "Manager";
                    AddCustomProperty( uno::Any( aChars ) ); // the property has string type
                    break;

                case EXTPR_TOKEN( PresentationFormat ):
                    m_aCustomPropertyName = "PresentationFormat";
                    AddCustomProperty( uno::Any( aChars ) ); // the property has string type
                    break;

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
                        AddCustomProperty( uno::Any( aChars.toBoolean() ) );
                        break;

                    case VT_TOKEN( bstr ):
                    case VT_TOKEN( lpstr ):
                    case VT_TOKEN( lpwstr ):
                        // the property has string type
                        AddCustomProperty( uno::Any( AttributeConversion::decodeXString( aChars ) ) );
                        m_CustomStringPropertyState = INSERTED;
                        break;

                    case VT_TOKEN( date ):
                    case VT_TOKEN( filetime ):
                        AddCustomProperty( uno::Any( GetDateTimeFromW3CDTF( aChars ) ) );
                        break;

                    case VT_TOKEN( i1 ):
                    case VT_TOKEN( i2 ):
                        AddCustomProperty( uno::Any( static_cast<sal_Int16>(aChars.toInt32()) ) );
                        break;

                    case VT_TOKEN( i4 ):
                    case VT_TOKEN( int ):
                        AddCustomProperty( uno::Any( aChars.toInt32() ) );
                        break;

                    case VT_TOKEN( i8 ):
                        AddCustomProperty( uno::Any( aChars.toInt64() ) );
                        break;

                    case VT_TOKEN( r4 ):
                        AddCustomProperty( uno::Any( aChars.toFloat() ) );
                        break;

                    case VT_TOKEN( r8 ):
                        AddCustomProperty( uno::Any( aChars.toDouble() ) );
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
    catch( uno::Exception& )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw xml::sax::SAXException(
            u"Error while setting document property!"_ustr,
            uno::Reference< uno::XInterface >(),
            anyEx );
    }
}

} // namespace oox::docprop

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
