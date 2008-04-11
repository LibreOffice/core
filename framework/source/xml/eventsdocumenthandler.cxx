/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: eventsdocumenthandler.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <stdio.h>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/resetableguard.hxx>
#include <xml/eventsdocumenthandler.hxx>
#include <macros/debug.hxx>
#include <xml/attributelist.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef __COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <sal/config.h>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;


#define XMLNS_EVENT             "http://openoffice.org/2001/event"
#define XMLNS_XLINK             "http://www.w3.org/1999/xlink"
#define XMLNS_EVENT_PREFIX      "event:"
#define XMLNS_XLINK_PREFIX      "xlink:"

#define ATTRIBUTE_XMLNS_EVENT   "xmlns:event"
#define ATTRIBUTE_XMLNS_XLINK   "xmlns:xlink"

#define XMLNS_FILTER_SEPARATOR  "^"

#define ELEMENT_EVENTS          "events"
#define ELEMENT_EVENT           "event"

#define ATTRIBUTE_LANGUAGE      "language"
#define ATTRIBUTE_LIBRARY       "library"
#define ATTRIBUTE_NAME          "name"
#define ATTRIBUTE_HREF          "href"
#define ATTRIBUTE_TYPE          "type"
#define ATTRIBUTE_MACRONAME     "macro-name"

#define ELEMENT_NS_EVENTS       "event:events"
#define ELEMENT_NS_EVENT        "event:event"

#define ATTRIBUTE_TYPE_CDATA    "CDATA"

#define EVENTS_DOCTYPE          "<!DOCTYPE event:events PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"event.dtd\">"

// Property names for events
#define PROP_EVENT_TYPE     "EventType"
#define PROP_LIBRARY        "Library"
#define PROP_SCRIPT         "Script"
#define PROP_MACRO_NAME     "MacroName"
#define STAR_BASIC          "StarBasic"
#define JAVA_SCRIPT         "JavaScript"


namespace framework
{

struct EventEntryProperty
{
    OReadEventsDocumentHandler::Event_XML_Namespace nNamespace;
    char                                            aEntryName[20];
};

static EventEntryProperty EventEntries[OReadEventsDocumentHandler::EV_XML_ENTRY_COUNT] =
{
    { OReadEventsDocumentHandler::EV_NS_EVENT,  ELEMENT_EVENTS          },
    { OReadEventsDocumentHandler::EV_NS_EVENT,  ELEMENT_EVENT           },
    { OReadEventsDocumentHandler::EV_NS_EVENT,  ATTRIBUTE_LANGUAGE      },
    { OReadEventsDocumentHandler::EV_NS_EVENT,  ATTRIBUTE_NAME          },
    { OReadEventsDocumentHandler::EV_NS_XLINK,  ATTRIBUTE_HREF          },
    { OReadEventsDocumentHandler::EV_NS_XLINK,  ATTRIBUTE_TYPE          },
    { OReadEventsDocumentHandler::EV_NS_EVENT,  ATTRIBUTE_MACRONAME     },
    { OReadEventsDocumentHandler::EV_NS_EVENT,  ATTRIBUTE_LIBRARY       }
};


OReadEventsDocumentHandler::OReadEventsDocumentHandler( EventsConfig& aItems ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    ::cppu::OWeakObject(),
    m_aEventItems( aItems )
{
    OUString aNamespaceEvent( RTL_CONSTASCII_USTRINGPARAM( XMLNS_EVENT ));
    OUString aNamespaceXLink( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK ));
    OUString aSeparator( RTL_CONSTASCII_USTRINGPARAM( XMLNS_FILTER_SEPARATOR ));

    // create hash map
    for ( int i = 0; i < (int)EV_XML_ENTRY_COUNT; i++ )
    {
        if ( EventEntries[i].nNamespace == EV_NS_EVENT )
        {
            OUString temp( aNamespaceEvent );
            temp += aSeparator;
            temp += OUString::createFromAscii( EventEntries[i].aEntryName );
            m_aEventsMap.insert( EventsHashMap::value_type( temp, (Events_XML_Entry)i ) );
        }
        else
        {
            OUString temp( aNamespaceXLink );
            temp += aSeparator;
            temp += OUString::createFromAscii( EventEntries[i].aEntryName );
            m_aEventsMap.insert( EventsHashMap::value_type( temp, (Events_XML_Entry)i ) );
        }
    }

    m_bEventsStartFound             = sal_False;
    m_bEventsEndFound               = sal_False;
    m_bEventStartFound              = sal_False;
}

OReadEventsDocumentHandler::~OReadEventsDocumentHandler()
{
}

Any SAL_CALL OReadEventsDocumentHandler::queryInterface( const Type & rType )
throw( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                rType ,
                SAL_STATIC_CAST( XDocumentHandler*, this ));
    if ( a.hasValue() )
        return a;

    return OWeakObject::queryInterface( rType );
}

// XDocumentHandler
void SAL_CALL OReadEventsDocumentHandler::startDocument(void)
throw ( SAXException, RuntimeException )
{
}

void SAL_CALL OReadEventsDocumentHandler::endDocument(void)
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if (( m_bEventsStartFound && !m_bEventsEndFound ) ||
        ( !m_bEventsStartFound && m_bEventsEndFound )       )
    {
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "No matching start or end element 'event:events' found!" ));
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}

void SAL_CALL OReadEventsDocumentHandler::startElement(
    const OUString& aName, const Reference< XAttributeList > &xAttribs )
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    EventsHashMap::const_iterator pEventEntry = m_aEventsMap.find( aName );
    if ( pEventEntry != m_aEventsMap.end() )
    {
        switch ( pEventEntry->second )
        {
            case EV_ELEMENT_EVENTS:
            {
                if ( m_bEventsStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'event:events' cannot be embeded into 'event:events'!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bEventsStartFound = sal_True;
            }
            break;

            case EV_ELEMENT_EVENT:
            {
                if ( !m_bEventsStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'event:event' must be embeded into element 'event:events'!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_bEventStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element event:event is not a container!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                OUString aLanguage;
                OUString aURL;
                OUString aMacroName;
                OUString aLibrary;
                OUString aEventName;

                m_bEventStartFound = sal_True;

                long                      nIndex = m_aEventItems.aEventNames.getLength();
                long                      nPropCount = 2; // every event config entry needs at least 2 properties
                Sequence< PropertyValue > aEventProperties( nPropCount );

                m_aEventItems.aEventNames.realloc(  nIndex + 1 );

                for ( sal_Int16 n = 0; n < xAttribs->getLength(); n++ )
                {
                    pEventEntry = m_aEventsMap.find( xAttribs->getNameByIndex( n ) );
                    if ( pEventEntry != m_aEventsMap.end() )
                    {
                        switch ( pEventEntry->second )
                        {
                            case EV_ATTRIBUTE_TYPE:
                            {
                                aLanguage = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case EV_ATTRIBUTE_NAME:
                            {
                                aEventName = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case XL_ATTRIBUTE_HREF:
                            {
                                aURL = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case EV_ATTRIBUTE_MACRONAME:
                            {
                                aMacroName = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case EV_ATTRIBUTE_LIBRARY:
                            {
                                aLibrary = xAttribs->getValueByIndex( n );
                            }
                            break;

                                          default:
                                              break; // nothing to do
                        }
                    }
                } // for

                OUString aRequiredAttributeName;
                if ( aLanguage.getLength() == 0 )
                    aRequiredAttributeName = OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_TYPE ));
                else if ( aEventName.getLength() == 0 )
                    aRequiredAttributeName = OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NAME ));

                // check for missing attribute values
                if ( aRequiredAttributeName.getLength() > 0 )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Required attribute "));
                    aErrorMessage += aRequiredAttributeName;
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( " must have a value!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                Any a;

                // set properties
                a <<= aLanguage;
                aEventProperties[0].Value <<= a;
                aEventProperties[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( PROP_EVENT_TYPE ));

                a <<= aMacroName;
                aEventProperties[1].Value <<= a;
                aEventProperties[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( PROP_MACRO_NAME ));

                if ( aLibrary.getLength() > 0 )
                {
                    ++nPropCount;
                    aEventProperties.realloc( nPropCount );
                    a <<= aLibrary;
                    aEventProperties[nPropCount-1].Value <<= a;
                    aEventProperties[nPropCount-1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( PROP_LIBRARY ));
                }

                if ( aURL.getLength() > 0 )
                {
                    ++nPropCount;
                    aEventProperties.realloc( nPropCount );
                    a <<= aURL;
                    aEventProperties[nPropCount-1].Value <<= a;
                    aEventProperties[nPropCount-1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( PROP_SCRIPT ));
                }

                // set event name
                m_aEventItems.aEventNames[ nIndex ] = aEventName;

                m_aEventItems.aEventsProperties.realloc( nIndex + 1 );
                a <<= aEventProperties;
                m_aEventItems.aEventsProperties[ nIndex ] = a;
            }
            break;

                  default:
                      break;
        }
    }
}

void SAL_CALL OReadEventsDocumentHandler::endElement(const OUString& aName)
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    EventsHashMap::const_iterator pEventEntry = m_aEventsMap.find( aName );
    if ( pEventEntry != m_aEventsMap.end() )
    {
        switch ( pEventEntry->second )
        {
            case EV_ELEMENT_EVENTS:
            {
                if ( !m_bEventsStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "End element 'event:events' found, but no start element" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bEventsStartFound = sal_False;
            }
            break;

            case EV_ELEMENT_EVENT:
            {
                if ( !m_bEventStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "End element 'event:event' found, but no start element" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bEventStartFound = sal_False;
            }
            break;

                  default:
                      break; // impossible case
        }
    }
}

void SAL_CALL OReadEventsDocumentHandler::characters(const OUString&)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadEventsDocumentHandler::ignorableWhitespace(const OUString&)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadEventsDocumentHandler::processingInstruction(
    const OUString& /*aTarget*/, const OUString& /*aData*/ )
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadEventsDocumentHandler::setDocumentLocator(
    const Reference< XLocator > &xLocator)
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    m_xLocator = xLocator;
}

::rtl::OUString OReadEventsDocumentHandler::getErrorLineString()
{
    ResetableGuard aGuard( m_aLock );

    char buffer[32];

    if ( m_xLocator.is() )
    {
        snprintf( buffer, sizeof(buffer), "Line: %ld - ", static_cast<long>(m_xLocator->getLineNumber() ));
        return OUString::createFromAscii( buffer );
    }
    else
        return OUString();
}


//_________________________________________________________________________________________________________________
//  OWriteEventsDocumentHandler
//_________________________________________________________________________________________________________________

OWriteEventsDocumentHandler::OWriteEventsDocumentHandler(
    const EventsConfig& aItems,
    Reference< XDocumentHandler > rWriteDocumentHandler ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_aItems( aItems ),
    m_xWriteDocumentHandler( rWriteDocumentHandler )
{
    m_xEmptyList        = Reference< XAttributeList >( (XAttributeList *)new AttributeListImpl, UNO_QUERY );
    m_aAttributeType    = OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_TYPE_CDATA ));
    m_aXMLXlinkNS       = OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK_PREFIX ));
    m_aXMLEventNS       = OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_EVENT_PREFIX ));
}

OWriteEventsDocumentHandler::~OWriteEventsDocumentHandler()
{
}

void OWriteEventsDocumentHandler::WriteEventsDocument() throw
( SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    m_xWriteDocumentHandler->startDocument();

    // write DOCTYPE line!
    Reference< XExtendedDocumentHandler > xExtendedDocHandler( m_xWriteDocumentHandler, UNO_QUERY );
    if ( xExtendedDocHandler.is() )
    {
        xExtendedDocHandler->unknown( OUString( RTL_CONSTASCII_USTRINGPARAM( EVENTS_DOCTYPE )) );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    }

    AttributeListImpl* pList = new AttributeListImpl;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XMLNS_EVENT )),
                         m_aAttributeType,
                         OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_EVENT )) );
    pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XMLNS_XLINK )),
                         m_aAttributeType,
                         OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK )) );

    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_EVENTS )), pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    Sequence< PropertyValue > aEventProperties;

    for ( int i = 0; i < m_aItems.aEventNames.getLength(); i++ )
    {
        if ( m_aItems.aEventsProperties[i] >>= aEventProperties )
            WriteEvent( m_aItems.aEventNames[i], aEventProperties );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_EVENTS )) );

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endDocument();
}

//_________________________________________________________________________________________________________________
//  protected member functions
//_________________________________________________________________________________________________________________

void OWriteEventsDocumentHandler::WriteEvent( const OUString& aEventName, const Sequence< PropertyValue >& aPropertyValues ) throw
( SAXException, RuntimeException )
{
    if ( aPropertyValues.getLength() > 0 )
    {
        AttributeListImpl* pList = new AttributeListImpl;
        Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

        if ( m_aAttributeURL.getLength() == 0 )
        {
            m_aAttributeURL = m_aXMLXlinkNS;
            m_aAttributeURL += OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_HREF ));
            m_aAttributeLinkType = m_aXMLXlinkNS;
            m_aAttributeLinkType += OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_TYPE ));
            m_aAttributeLanguage = m_aXMLEventNS;
            m_aAttributeLanguage += OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_LANGUAGE ));
            m_aAttributeMacroName = m_aXMLEventNS;
            m_aAttributeMacroName += OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MACRONAME ));
            m_aAttributeLibrary = m_aXMLEventNS;
            m_aAttributeLibrary += OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_LIBRARY ));
            m_aAttributeName = m_aXMLEventNS;
            m_aAttributeName += OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NAME ));
        }

        pList->addAttribute( m_aAttributeName, m_aAttributeType, aEventName );

        sal_Bool    bURLSet = sal_False;
        OUString    aValue;
        OUString    aName;

        // save attributes
        for ( int i = 0; i < aPropertyValues.getLength(); i++ )
        {
            aPropertyValues[i].Value >>= aValue;
            if ( aPropertyValues[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( PROP_EVENT_TYPE )))
                pList->addAttribute( m_aAttributeLanguage, m_aAttributeType, aValue );
            else if ( aPropertyValues[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( PROP_MACRO_NAME )) &&
                      aValue.getLength() > 0 )
                pList->addAttribute( m_aAttributeMacroName, m_aAttributeType, aValue );
            else if ( aPropertyValues[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( PROP_LIBRARY )) &&
                      aValue.getLength() > 0 )
                pList->addAttribute( m_aAttributeLibrary, m_aAttributeType, aValue );
            else if ( aPropertyValues[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( PROP_SCRIPT )))
            {
                pList->addAttribute( m_aAttributeURL, m_aAttributeType, aValue );
                bURLSet = sal_True;
            }
        }

        if ( bURLSet )
            pList->addAttribute( m_aAttributeLinkType, m_aAttributeType, OUString( RTL_CONSTASCII_USTRINGPARAM( "simple" )) );

        m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_EVENT )), xList );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

        m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_EVENT )) );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    }
}

} // namespace framework
