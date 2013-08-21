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


#include <stdio.h>

#include <threadhelp/resetableguard.hxx>
#include <xml/statusbardocumenthandler.hxx>
#include <macros/debug.hxx>

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/svapp.hxx>
#include <vcl/status.hxx>

#include <comphelper/attributelist.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::container;

#define XMLNS_STATUSBAR             "http://openoffice.org/2001/statusbar"
#define XMLNS_XLINK                 "http://www.w3.org/1999/xlink"
#define XMLNS_STATUSBAR_PREFIX      "statusbar:"
#define XMLNS_XLINK_PREFIX          "xlink:"

#define XMLNS_FILTER_SEPARATOR      "^"

#define ELEMENT_STATUSBAR           "statusbar"
#define ELEMENT_STATUSBARITEM       "statusbaritem"

#define ATTRIBUTE_ALIGN             "align"
#define ATTRIBUTE_STYLE             "style"
#define ATTRIBUTE_URL               "href"
#define ATTRIBUTE_WIDTH             "width"
#define ATTRIBUTE_OFFSET            "offset"
#define ATTRIBUTE_AUTOSIZE          "autosize"
#define ATTRIBUTE_OWNERDRAW         "ownerdraw"
#define ATTRIBUTE_HELPURL           "helpid"

#define ELEMENT_NS_STATUSBAR        "statusbar:statusbar"
#define ELEMENT_NS_STATUSBARITEM    "statusbar:statusbaritem"

#define ATTRIBUTE_XMLNS_STATUSBAR   "xmlns:statusbar"
#define ATTRIBUTE_XMLNS_XLINK       "xmlns:xlink"

#define ATTRIBUTE_TYPE_CDATA        "CDATA"

#define ATTRIBUTE_BOOLEAN_TRUE      "true"
#define ATTRIBUTE_BOOLEAN_FALSE     "false"

#define ATTRIBUTE_ALIGN_LEFT        "left"
#define ATTRIBUTE_ALIGN_RIGHT       "right"
#define ATTRIBUTE_ALIGN_CENTER      "center"

#define ATTRIBUTE_STYLE_IN          "in"
#define ATTRIBUTE_STYLE_OUT         "out"
#define ATTRIBUTE_STYLE_FLAT        "flat"

#define STATUSBAR_DOCTYPE           "<!DOCTYPE statusbar:statusbar PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"statusbar.dtd\">"

namespace framework
{

// Property names of a menu/menu item ItemDescriptor
static const char ITEM_DESCRIPTOR_COMMANDURL[]  = "CommandURL";
static const char ITEM_DESCRIPTOR_HELPURL[]     = "HelpURL";
static const char ITEM_DESCRIPTOR_OFFSET[]      = "Offset";
static const char ITEM_DESCRIPTOR_STYLE[]       = "Style";
static const char ITEM_DESCRIPTOR_WIDTH[]       = "Width";
static const char ITEM_DESCRIPTOR_TYPE[]        = "Type";

static void ExtractStatusbarItemParameters(
    const Sequence< PropertyValue > rProp,
    OUString&                       rCommandURL,
    OUString&                       rHelpURL,
    sal_Int16&                      rOffset,
    sal_Int16&                      rStyle,
    sal_Int16&                      rWidth )
{
    for ( sal_Int32 i = 0; i < rProp.getLength(); i++ )
    {
        if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_COMMANDURL ))
        {
            rProp[i].Value >>= rCommandURL;
            rCommandURL = rCommandURL.intern();
        }
        else if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_HELPURL ))
        {
            rProp[i].Value >>= rHelpURL;
        }
        else if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_OFFSET ))
        {
            rProp[i].Value >>= rOffset;
        }
        else if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_STYLE ))
        {
            rProp[i].Value >>= rStyle;
        }
        else if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_WIDTH ))
        {
            rProp[i].Value >>= rWidth;
        }
    }
}

struct StatusBarEntryProperty
{
    OReadStatusBarDocumentHandler::StatusBar_XML_Namespace  nNamespace;
    char                                                    aEntryName[20];
};

StatusBarEntryProperty StatusBarEntries[OReadStatusBarDocumentHandler::SB_XML_ENTRY_COUNT] =
{
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ELEMENT_STATUSBAR       },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ELEMENT_STATUSBARITEM   },
    { OReadStatusBarDocumentHandler::SB_NS_XLINK,       ATTRIBUTE_URL           },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ATTRIBUTE_ALIGN         },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ATTRIBUTE_STYLE         },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ATTRIBUTE_AUTOSIZE      },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ATTRIBUTE_OWNERDRAW     },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ATTRIBUTE_WIDTH         },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ATTRIBUTE_OFFSET        },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ATTRIBUTE_HELPURL       }
};


OReadStatusBarDocumentHandler::OReadStatusBarDocumentHandler(
    const Reference< XIndexContainer >& rStatusBarItems ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_aStatusBarItems( rStatusBarItems )
{
    OUString aNamespaceStatusBar( XMLNS_STATUSBAR );
    OUString aNamespaceXLink( XMLNS_XLINK );
    OUString aSeparator( XMLNS_FILTER_SEPARATOR );

    // create hash map
    for ( int i = 0; i < (int)SB_XML_ENTRY_COUNT; i++ )
    {
        if ( StatusBarEntries[i].nNamespace == SB_NS_STATUSBAR )
        {
            OUString temp( aNamespaceStatusBar );
            temp += aSeparator;
            temp += OUString::createFromAscii( StatusBarEntries[i].aEntryName );
            m_aStatusBarMap.insert( StatusBarHashMap::value_type( temp, (StatusBar_XML_Entry)i ) );
        }
        else
        {
            OUString temp( aNamespaceXLink );
            temp += aSeparator;
            temp += OUString::createFromAscii( StatusBarEntries[i].aEntryName );
            m_aStatusBarMap.insert( StatusBarHashMap::value_type( temp, (StatusBar_XML_Entry)i ) );
        }
    }

    m_bStatusBarStartFound          = sal_False;
    m_bStatusBarEndFound            = sal_False;
    m_bStatusBarItemStartFound      = sal_False;
}

OReadStatusBarDocumentHandler::~OReadStatusBarDocumentHandler()
{
}

// XDocumentHandler
void SAL_CALL OReadStatusBarDocumentHandler::startDocument(void)
throw ( SAXException, RuntimeException )
{
}

void SAL_CALL OReadStatusBarDocumentHandler::endDocument(void)
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if (( m_bStatusBarStartFound && !m_bStatusBarEndFound ) ||
        ( !m_bStatusBarStartFound && m_bStatusBarEndFound )     )
    {
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += "No matching start or end element 'statusbar' found!";
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}

void SAL_CALL OReadStatusBarDocumentHandler::startElement(
    const OUString& aName, const Reference< XAttributeList > &xAttribs )
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    StatusBarHashMap::const_iterator pStatusBarEntry = m_aStatusBarMap.find( aName ) ;
    if ( pStatusBarEntry != m_aStatusBarMap.end() )
    {
        switch ( pStatusBarEntry->second )
        {
            case SB_ELEMENT_STATUSBAR:
            {
                if ( m_bStatusBarStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'statusbar:statusbar' cannot be embeded into 'statusbar:statusbar'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bStatusBarStartFound = sal_True;
            }
            break;

            case SB_ELEMENT_STATUSBARITEM:
            {
                if ( !m_bStatusBarStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'statusbar:statusbaritem' must be embeded into element 'statusbar:statusbar'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_bStatusBarItemStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element statusbar:statusbaritem is not a container!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                OUString    aCommandURL;
                OUString    aHelpURL;
                sal_Int16   nItemBits( ItemStyle::ALIGN_CENTER|ItemStyle::DRAW_IN3D );
                sal_Int16   nWidth( 0 );
                sal_Int16   nOffset( STATUSBAR_OFFSET );
                sal_Bool    bCommandURL( sal_False );

                m_bStatusBarItemStartFound = sal_True;
                for ( sal_Int16 n = 0; n < xAttribs->getLength(); n++ )
                {
                    pStatusBarEntry = m_aStatusBarMap.find( xAttribs->getNameByIndex( n ) );
                    if ( pStatusBarEntry != m_aStatusBarMap.end() )
                    {
                        switch ( pStatusBarEntry->second )
                        {
                            case SB_ATTRIBUTE_URL:
                            {
                                bCommandURL = sal_True;
                                aCommandURL = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case SB_ATTRIBUTE_ALIGN:
                            {
                                if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_ALIGN_LEFT )
                                {
                                    nItemBits |= ItemStyle::ALIGN_LEFT;
                                    nItemBits &= ~ItemStyle::ALIGN_CENTER;
                                }
                                else if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_ALIGN_CENTER )
                                {
                                    nItemBits |= ItemStyle::ALIGN_CENTER;
                                    nItemBits &= ~ItemStyle::ALIGN_LEFT;
                                }
                                else if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_ALIGN_RIGHT )
                                {
                                    nItemBits |= ItemStyle::ALIGN_RIGHT;
                                }
                                else
                                {
                                    OUString aErrorMessage = getErrorLineString();
                                    aErrorMessage += "Attribute statusbar:align must have one value of 'left','right' or 'center'!";
                                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                                }
                            }
                            break;

                            case SB_ATTRIBUTE_STYLE:
                            {
                                if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_STYLE_IN )
                                {
                                    nItemBits |= ItemStyle::DRAW_IN3D;
                                    nItemBits &= ~ItemStyle::DRAW_OUT3D;
                                }
                                else if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_STYLE_OUT )
                                {
                                    nItemBits |= ItemStyle::DRAW_OUT3D;
                                    nItemBits &= ~ItemStyle::DRAW_IN3D;
                                }
                                else if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_STYLE_FLAT )
                                {
                                    nItemBits |= ItemStyle::DRAW_FLAT;
                                }
                                else
                                {
                                    OUString aErrorMessage = getErrorLineString();
                                    aErrorMessage += "Attribute statusbar:autosize must have value 'true' or 'false'!";
                                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                                }
                            }
                            break;

                            case SB_ATTRIBUTE_AUTOSIZE:
                            {
                                if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_BOOLEAN_TRUE )
                                    nItemBits |= ItemStyle::AUTO_SIZE;
                                else if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_BOOLEAN_FALSE )
                                    nItemBits &= ~ItemStyle::AUTO_SIZE;
                                else
                                {
                                    OUString aErrorMessage = getErrorLineString();
                                    aErrorMessage += "Attribute statusbar:autosize must have value 'true' or 'false'!";
                                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                                }
                            }
                            break;

                            case SB_ATTRIBUTE_OWNERDRAW:
                            {
                                if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_BOOLEAN_TRUE )
                                    nItemBits |= ItemStyle::OWNER_DRAW;
                                else if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_BOOLEAN_FALSE )
                                    nItemBits &= ~ItemStyle::OWNER_DRAW;
                                else
                                {
                                    OUString aErrorMessage = getErrorLineString();
                                    aErrorMessage += "Attribute statusbar:ownerdraw must have value 'true' or 'false'!";
                                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                                }
                            }
                            break;

                            case SB_ATTRIBUTE_WIDTH:
                            {
                                nWidth = (sal_Int16)(xAttribs->getValueByIndex( n ).toInt32());
                            }
                            break;

                            case SB_ATTRIBUTE_OFFSET:
                            {
                                nOffset = (sal_Int16)(xAttribs->getValueByIndex( n ).toInt32());
                            }
                            break;

                                          case SB_ATTRIBUTE_HELPURL:
                                          {
                                                aHelpURL = xAttribs->getValueByIndex( n );
                                          }
                                          break;

                                          default:
                                              break;
                        }
                    }
                } // for

                if ( !bCommandURL )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Required attribute statusbar:url must have a value!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }
                        else
                        {
                            Sequence< PropertyValue > aStatusbarItemProp( 6 );
                            aStatusbarItemProp[0].Name = OUString( ITEM_DESCRIPTOR_COMMANDURL );
                            aStatusbarItemProp[1].Name = OUString( ITEM_DESCRIPTOR_HELPURL );
                            aStatusbarItemProp[2].Name = OUString( ITEM_DESCRIPTOR_OFFSET );
                            aStatusbarItemProp[3].Name = OUString( ITEM_DESCRIPTOR_STYLE );
                            aStatusbarItemProp[4].Name = OUString( ITEM_DESCRIPTOR_WIDTH );
                            aStatusbarItemProp[5].Name = OUString( ITEM_DESCRIPTOR_TYPE );

                            aStatusbarItemProp[0].Value <<= aCommandURL;
                            aStatusbarItemProp[1].Value <<= aHelpURL;
                            aStatusbarItemProp[2].Value <<= nOffset;
                            aStatusbarItemProp[3].Value <<= nItemBits;
                            aStatusbarItemProp[4].Value <<= nWidth;
                            aStatusbarItemProp[5].Value = makeAny( ::com::sun::star::ui::ItemType::DEFAULT );

                            m_aStatusBarItems->insertByIndex( m_aStatusBarItems->getCount(), makeAny( aStatusbarItemProp ) );
                       }
            }
            break;

                  default:
                      break;
        }
    }
}

void SAL_CALL OReadStatusBarDocumentHandler::endElement(const OUString& aName)
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    StatusBarHashMap::const_iterator pStatusBarEntry = m_aStatusBarMap.find( aName ) ;
    if ( pStatusBarEntry != m_aStatusBarMap.end() )
    {
        switch ( pStatusBarEntry->second )
        {
            case SB_ELEMENT_STATUSBAR:
            {
                if ( !m_bStatusBarStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "End element 'statusbar' found, but no start element 'statusbar'";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bStatusBarStartFound = sal_False;
            }
            break;

            case SB_ELEMENT_STATUSBARITEM:
            {
                if ( !m_bStatusBarItemStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "End element 'statusbar:statusbaritem' found, but no start element 'statusbar:statusbaritem'";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bStatusBarItemStartFound = sal_False;
            }
            break;

                  default:
                      break;
        }
    }
}

void SAL_CALL OReadStatusBarDocumentHandler::characters(const OUString&)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadStatusBarDocumentHandler::ignorableWhitespace(const OUString&)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadStatusBarDocumentHandler::processingInstruction(
    const OUString& /*aTarget*/, const OUString& /*aData*/ )
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadStatusBarDocumentHandler::setDocumentLocator(
    const Reference< XLocator > &xLocator)
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    m_xLocator = xLocator;
}

OUString OReadStatusBarDocumentHandler::getErrorLineString()
{
    ResetableGuard aGuard( m_aLock );

    char buffer[32];

    if ( m_xLocator.is() )
    {
        snprintf( buffer, sizeof(buffer), "Line: %ld - ", static_cast<long>( m_xLocator->getLineNumber() ));
        return OUString::createFromAscii( buffer );
    }
    else
        return OUString();
}


//_________________________________________________________________________________________________________________
//  OWriteStatusBarDocumentHandler
//_________________________________________________________________________________________________________________

OWriteStatusBarDocumentHandler::OWriteStatusBarDocumentHandler(
    const Reference< XIndexAccess >& aStatusBarItems,
    const Reference< XDocumentHandler >& rWriteDocumentHandler ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_aStatusBarItems( aStatusBarItems ),
    m_xWriteDocumentHandler( rWriteDocumentHandler )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    m_xEmptyList        = Reference< XAttributeList >( (XAttributeList *) pList, UNO_QUERY );
    m_aAttributeType    = OUString( ATTRIBUTE_TYPE_CDATA );
    m_aXMLXlinkNS       = OUString( XMLNS_XLINK_PREFIX );
    m_aXMLStatusBarNS   = OUString( XMLNS_STATUSBAR_PREFIX );
}

OWriteStatusBarDocumentHandler::~OWriteStatusBarDocumentHandler()
{
}

void OWriteStatusBarDocumentHandler::WriteStatusBarDocument() throw
( SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    m_xWriteDocumentHandler->startDocument();

    // write DOCTYPE line!
    Reference< XExtendedDocumentHandler > xExtendedDocHandler( m_xWriteDocumentHandler, UNO_QUERY );
    if ( xExtendedDocHandler.is() )
    {
        xExtendedDocHandler->unknown( OUString( STATUSBAR_DOCTYPE ) );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    }

    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    pList->AddAttribute( OUString( ATTRIBUTE_XMLNS_STATUSBAR ),
                         m_aAttributeType,
                         OUString( XMLNS_STATUSBAR ) );

    pList->AddAttribute( OUString( ATTRIBUTE_XMLNS_XLINK ),
                         m_aAttributeType,
                         OUString( XMLNS_XLINK ) );

    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_STATUSBAR ), pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    sal_Int32  nItemCount = m_aStatusBarItems->getCount();
    Any        aAny;

    for ( sal_Int32 nItemPos = 0; nItemPos < nItemCount; nItemPos++ )
    {
        Sequence< PropertyValue > aProps;
        aAny = m_aStatusBarItems->getByIndex( nItemPos );
        if ( aAny >>= aProps )
        {
            OUString    aCommandURL;
            OUString    aHelpURL;
            sal_Int16   nStyle( ItemStyle::ALIGN_CENTER|ItemStyle::DRAW_IN3D );
            sal_Int16   nWidth( 0 );
            sal_Int16   nOffset( STATUSBAR_OFFSET );

            ExtractStatusbarItemParameters(
                aProps,
                aCommandURL,
                aHelpURL,
                nOffset,
                nStyle,
                nWidth );

            if ( !aCommandURL.isEmpty() )
                WriteStatusBarItem( aCommandURL, aHelpURL, nOffset, nStyle, nWidth );
        }
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_STATUSBAR ) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endDocument();
}

//_________________________________________________________________________________________________________________
//  protected member functions
//_________________________________________________________________________________________________________________

void OWriteStatusBarDocumentHandler::WriteStatusBarItem(
    const OUString& rCommandURL,
    const OUString& /*rHelpURL*/,
    sal_Int16            nOffset,
    sal_Int16            nStyle,
    sal_Int16            nWidth )
throw ( SAXException, RuntimeException )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    if (m_aAttributeURL.isEmpty() )
    {
        m_aAttributeURL = m_aXMLXlinkNS;
        m_aAttributeURL += OUString( ATTRIBUTE_URL );
    }

    // save required attribute (URL)
    pList->AddAttribute( m_aAttributeURL, m_aAttributeType, rCommandURL );

    // alignment
    if ( nStyle & ItemStyle::ALIGN_RIGHT )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + OUString( ATTRIBUTE_ALIGN ),
                             m_aAttributeType,
                             OUString( ATTRIBUTE_ALIGN_RIGHT ) );
    }
    else if ( nStyle & ItemStyle::ALIGN_CENTER )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + OUString( ATTRIBUTE_ALIGN ),
                             m_aAttributeType,
                             OUString( ATTRIBUTE_ALIGN_CENTER ) );
    }
    else
    {
        pList->AddAttribute( m_aXMLStatusBarNS + OUString( ATTRIBUTE_ALIGN ),
                             m_aAttributeType,
                             OUString( ATTRIBUTE_ALIGN_LEFT ) );
    }

    // style ( SIB_IN is default )
    if ( nStyle & ItemStyle::DRAW_FLAT )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + OUString( ATTRIBUTE_STYLE ),
                             m_aAttributeType,
                             OUString( ATTRIBUTE_STYLE_FLAT ) );
    }
    else if ( nStyle & ItemStyle::DRAW_OUT3D )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + OUString( ATTRIBUTE_STYLE ),
                             m_aAttributeType,
                             OUString( ATTRIBUTE_STYLE_OUT ) );
    }

    // autosize (default sal_False)
    if ( nStyle & ItemStyle::AUTO_SIZE )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + OUString( ATTRIBUTE_AUTOSIZE ),
                             m_aAttributeType,
                             OUString( ATTRIBUTE_BOOLEAN_TRUE ) );
    }

    // ownerdraw (default sal_False)
    if ( nStyle & ItemStyle::OWNER_DRAW )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + OUString( ATTRIBUTE_OWNERDRAW ),
                             m_aAttributeType,
                             OUString( ATTRIBUTE_BOOLEAN_TRUE ) );
    }

    // width (default 0)
    if ( nWidth > 0 )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + OUString( ATTRIBUTE_WIDTH ),
                             m_aAttributeType,
                             OUString::number( nWidth ) );
    }

    // offset (default STATUSBAR_OFFSET)
    if ( nOffset != STATUSBAR_OFFSET )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + OUString( ATTRIBUTE_OFFSET ),
                             m_aAttributeType,
                             OUString::number( nOffset ) );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_STATUSBARITEM ), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_STATUSBARITEM ) );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
