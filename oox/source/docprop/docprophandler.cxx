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
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>

#include <o3tl/safeint.hxx>
#include <osl/time.h>
#include <osl/diagnose.h>
#include <i18nlangtag/languagetag.hxx>

#include <vector>
#include <boost/algorithm/string.hpp>

#include <oox/helper/attributelist.hxx>

using namespace ::com::sun::star;

namespace oox {
namespace docprop {

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
    if ( !m_aCustomPropertyName.isEmpty() )
    {
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
}

util::DateTime OOXMLDocPropHandler::GetDateTimeFromW3CDTF( const OUString& aChars )
{
    oslDateTime aOslDTime = { 0, 0, 0, 0, 0, 0, 0, 0 };
    const sal_Int32 nLen = aChars.getLength();
    if ( nLen >= 4 )
    {
        aOslDTime.Year = (sal_Int16)aChars.copy( 0, 4 ).toInt32();

        if ( nLen >= 7 && aChars[4] == '-' )
        {
            aOslDTime.Month = (sal_uInt16)aChars.copy( 5, 2 ).toInt32();

            if ( nLen >= 10 && aChars[7] == '-' )
            {
                aOslDTime.Day = (sal_uInt16)aChars.copy( 8, 2 ).toInt32();

                if ( nLen >= 16 && aChars[10] == 'T' && aChars[13] == ':' )
                {
                    aOslDTime.Hours = (sal_uInt16)aChars.copy( 11, 2 ).toInt32();
                    aOslDTime.Minutes = (sal_uInt16)aChars.copy( 14, 2 ).toInt32();

                    sal_Int32 nOptTime = 0;
                    if ( nLen >= 19 && aChars[16] == ':' )
                    {
                        aOslDTime.Seconds = (sal_uInt16)aChars.copy( 17, 2 ).toInt32();
                        nOptTime += 3;
                        if ( nLen >= 20 && aChars[19] == '.' )
                        {
                            nOptTime += 1;
                            sal_Int32 digitPos = 20;
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
                            nModif = aChars.copy( 16 + nOptTime + 1, 2 ).toInt32() * 3600;
                            nModif += aChars.copy( 16 + nOptTime + 4, 2 ).toInt32() * 60;
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

uno::Sequence< OUString > OOXMLDocPropHandler::GetKeywordsSet( const OUString& aChars )
{
    if ( !aChars.isEmpty() )
    {
        std::string aUtf8Chars = OUStringToOString( aChars, RTL_TEXTENCODING_UTF8 ).getStr();
        std::vector<std::string> aUtf8Result;
        boost::split( aUtf8Result, aUtf8Chars, boost::is_any_of(" ,;:\t"), boost::token_compress_on );

        if (!aUtf8Result.empty())
        {
            uno::Sequence< OUString > aResult( aUtf8Result.size() );
            OUString* pResultValues = aResult.getArray();
            for ( std::vector< std::string >::const_iterator i = aUtf8Result.begin();
                  i != aUtf8Result.end(); ++i, ++pResultValues )
                *pResultValues = OUString( i->c_str(), static_cast< sal_Int32 >( i->size() ),RTL_TEXTENCODING_UTF8 );

            return aResult;
        }
    }
    return uno::Sequence< OUString >();
}

void OOXMLDocPropHandler::UpdateDocStatistic( const OUString& aChars )
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

    if ( !aName.isEmpty() )
    {
        bool bFound = false;
        sal_Int32 nLen = aSet.getLength();
        for ( sal_Int32 nInd = 0; nInd < nLen; nInd++ )
            if ( aSet[nInd].Name == aName )
            {
                aSet[nInd].Value <<= aChars.toInt32();
                bFound = true;
                break;
            }

        if ( !bFound )
        {
            aSet.realloc( nLen + 1 );
            aSet[nLen].Name = aName;
            aSet[nLen].Value <<= aChars.toInt32();
        }

        m_xDocProp->setDocumentStatistics( aSet );
    }
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
    else
    {
        SAL_WARN("oox", "OOXMLDocPropHandler::startFastElement: unknown element " << getBaseToken(nElement));
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
    if ( m_nInBlock )
    {
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
                            AddCustomProperty(uno::makeAny(OUString()));
                        }
                    break;
                }
            }
            m_CustomStringPropertyState = NONE;
            m_nType = 0;
        }
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
                    m_aCustomPropertyName = "category";
                    AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                    break;

                case COREPR_TOKEN( contentStatus ):
                    m_aCustomPropertyName = "contentStatus";
                    AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                    break;

                case COREPR_TOKEN( contentType ):
                    m_aCustomPropertyName = "contentType";
                    AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                    break;

                case COREPR_TOKEN( identifier ):
                    m_aCustomPropertyName = "identifier";
                    AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                    break;

                case COREPR_TOKEN( version ):
                    m_aCustomPropertyName = "version";
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
                    AddCustomProperty( uno::makeAny( aChars.toBoolean() ) ); // the property has boolean type
                    break;

                case EXTPR_TOKEN( LinksUpToDate ):
                    m_aCustomPropertyName = "LinksUpToDate";
                    AddCustomProperty( uno::makeAny( aChars.toBoolean() ) ); // the property has boolean type
                    break;

                case EXTPR_TOKEN( ScaleCrop ):
                    m_aCustomPropertyName = "ScaleCrop";
                    AddCustomProperty( uno::makeAny( aChars.toBoolean() ) ); // the property has boolean type
                    break;

                case EXTPR_TOKEN( SharedDoc ):
                    m_aCustomPropertyName = "ShareDoc";
                    AddCustomProperty( uno::makeAny( aChars.toBoolean() ) ); // the property has boolean type
                    break;

                case EXTPR_TOKEN( DocSecurity ):
                    m_aCustomPropertyName = "DocSecurity";
                    AddCustomProperty( uno::makeAny( aChars.toInt32() ) ); // the property has sal_Int32 type
                    break;

                case EXTPR_TOKEN( HiddenSlides ):
                    m_aCustomPropertyName = "HiddenSlides";
                    AddCustomProperty( uno::makeAny( aChars.toInt32() ) ); // the property has sal_Int32 type
                    break;

                case EXTPR_TOKEN( MMClips ):
                    m_aCustomPropertyName = "MMClips";
                    AddCustomProperty( uno::makeAny( aChars.toInt32() ) ); // the property has sal_Int32 type
                    break;

                case EXTPR_TOKEN( Notes ):
                    m_aCustomPropertyName = "Notes";
                    AddCustomProperty( uno::makeAny( aChars.toInt32() ) ); // the property has sal_Int32 type
                    break;

                case EXTPR_TOKEN( Slides ):
                    m_aCustomPropertyName = "Slides";
                    AddCustomProperty( uno::makeAny( aChars.toInt32() ) ); // the property has sal_Int32 type
                    break;

                case EXTPR_TOKEN( AppVersion ):
                    m_aCustomPropertyName = "AppVersion";
                    AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                    break;

                case EXTPR_TOKEN( Company ):
                    m_aCustomPropertyName = "Company";
                    AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                    break;

                case EXTPR_TOKEN( HyperlinkBase ):
                    m_aCustomPropertyName = "HyperlinkBase";
                    AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                    break;

                case EXTPR_TOKEN( Manager ):
                    m_aCustomPropertyName = "Manager";
                    AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
                    break;

                case EXTPR_TOKEN( PresentationFormat ):
                    m_aCustomPropertyName = "PresentationFormat";
                    AddCustomProperty( uno::makeAny( aChars ) ); // the property has string type
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
                        AddCustomProperty( uno::makeAny( aChars.toBoolean() ) );
                        break;

                    case VT_TOKEN( bstr ):
                    case VT_TOKEN( lpstr ):
                    case VT_TOKEN( lpwstr ):
                        // the property has string type
                        AddCustomProperty( uno::makeAny( AttributeConversion::decodeXString( aChars ) ) );
                        m_CustomStringPropertyState = INSERTED;
                        break;

                    case VT_TOKEN( date ):
                    case VT_TOKEN( filetime ):
                        AddCustomProperty( uno::makeAny( GetDateTimeFromW3CDTF( aChars ) ) );
                        break;

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
            "Error while setting document property!",
            uno::Reference< uno::XInterface >(),
            uno::makeAny( e ) );
    }
}

} // namespace docprop
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
