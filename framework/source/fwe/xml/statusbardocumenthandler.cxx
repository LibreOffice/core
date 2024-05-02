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

#include <xml/statusbardocumenthandler.hxx>

#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>

#include <vcl/status.hxx>

#include <comphelper/attributelist.hxx>
#include <comphelper/propertyvalue.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::container;

constexpr OUString XMLNS_STATUSBAR = u"http://openoffice.org/2001/statusbar"_ustr;
constexpr OUString XMLNS_XLINK = u"http://www.w3.org/1999/xlink"_ustr;
constexpr OUStringLiteral XMLNS_STATUSBAR_PREFIX = u"statusbar:";
constexpr OUStringLiteral XMLNS_XLINK_PREFIX = u"xlink:";

constexpr OUString XMLNS_FILTER_SEPARATOR = u"^"_ustr;

constexpr OUString ATTRIBUTE_ALIGN = u"align"_ustr;
constexpr OUString ATTRIBUTE_STYLE = u"style"_ustr;
constexpr OUString ATTRIBUTE_URL = u"href"_ustr;
constexpr OUString ATTRIBUTE_WIDTH = u"width"_ustr;
constexpr OUString ATTRIBUTE_OFFSET = u"offset"_ustr;
constexpr OUString ATTRIBUTE_AUTOSIZE = u"autosize"_ustr;
constexpr OUString ATTRIBUTE_OWNERDRAW = u"ownerdraw"_ustr;
constexpr OUString ATTRIBUTE_HELPURL = u"helpid"_ustr;
constexpr OUString ATTRIBUTE_MANDATORY = u"mandatory"_ustr;

constexpr OUString ELEMENT_NS_STATUSBAR = u"statusbar:statusbar"_ustr;
constexpr OUString ELEMENT_NS_STATUSBARITEM = u"statusbar:statusbaritem"_ustr;

constexpr OUStringLiteral ATTRIBUTE_XMLNS_STATUSBAR = u"xmlns:statusbar";
constexpr OUStringLiteral ATTRIBUTE_XMLNS_XLINK = u"xmlns:xlink";

constexpr OUString ATTRIBUTE_BOOLEAN_TRUE = u"true"_ustr;
constexpr OUString ATTRIBUTE_BOOLEAN_FALSE = u"false"_ustr;

constexpr OUString ATTRIBUTE_ALIGN_LEFT = u"left"_ustr;
constexpr OUString ATTRIBUTE_ALIGN_RIGHT = u"right"_ustr;
constexpr OUString ATTRIBUTE_ALIGN_CENTER = u"center"_ustr;

constexpr OUStringLiteral ATTRIBUTE_STYLE_IN = u"in";
constexpr OUString ATTRIBUTE_STYLE_OUT = u"out"_ustr;
constexpr OUString ATTRIBUTE_STYLE_FLAT = u"flat"_ustr;

constexpr OUStringLiteral STATUSBAR_DOCTYPE = u"<!DOCTYPE statusbar:statusbar PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"statusbar.dtd\">";

namespace framework
{

// Property names of a menu/menu item ItemDescriptor
constexpr OUString ITEM_DESCRIPTOR_COMMANDURL  = u"CommandURL"_ustr;
constexpr OUString ITEM_DESCRIPTOR_HELPURL     = u"HelpURL"_ustr;
constexpr OUString ITEM_DESCRIPTOR_OFFSET      = u"Offset"_ustr;
constexpr OUString ITEM_DESCRIPTOR_STYLE       = u"Style"_ustr;
constexpr OUString ITEM_DESCRIPTOR_WIDTH       = u"Width"_ustr;
constexpr OUString ITEM_DESCRIPTOR_TYPE        = u"Type"_ustr;

static void ExtractStatusbarItemParameters(
    const Sequence< PropertyValue >& rProp,
    OUString&                        rCommandURL,
    OUString&                        rHelpURL,
    sal_Int16&                       rOffset,
    sal_Int16&                       rStyle,
    sal_Int16&                       rWidth )
{
    for ( const PropertyValue& rEntry : rProp )
    {
        if ( rEntry.Name == ITEM_DESCRIPTOR_COMMANDURL )
        {
            rEntry.Value >>= rCommandURL;
        }
        else if ( rEntry.Name == ITEM_DESCRIPTOR_HELPURL )
        {
            rEntry.Value >>= rHelpURL;
        }
        else if ( rEntry.Name == ITEM_DESCRIPTOR_OFFSET )
        {
            rEntry.Value >>= rOffset;
        }
        else if ( rEntry.Name == ITEM_DESCRIPTOR_STYLE )
        {
            rEntry.Value >>= rStyle;
        }
        else if ( rEntry.Name == ITEM_DESCRIPTOR_WIDTH )
        {
            rEntry.Value >>= rWidth;
        }
    }
}

namespace {

struct StatusBarEntryProperty
{
    OReadStatusBarDocumentHandler::StatusBar_XML_Namespace  nNamespace;
    OUString aEntryName;
};

}

StatusBarEntryProperty constexpr StatusBarEntries[OReadStatusBarDocumentHandler::SB_XML_ENTRY_COUNT] =
{
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   u"statusbar"_ustr       },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   u"statusbaritem"_ustr   },
    { OReadStatusBarDocumentHandler::SB_NS_XLINK,       ATTRIBUTE_URL           },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ATTRIBUTE_ALIGN         },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ATTRIBUTE_STYLE         },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ATTRIBUTE_AUTOSIZE      },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ATTRIBUTE_OWNERDRAW     },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ATTRIBUTE_WIDTH         },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ATTRIBUTE_OFFSET        },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ATTRIBUTE_HELPURL       },
    { OReadStatusBarDocumentHandler::SB_NS_STATUSBAR,   ATTRIBUTE_MANDATORY     }
};

OReadStatusBarDocumentHandler::OReadStatusBarDocumentHandler(
    const Reference< XIndexContainer >& rStatusBarItems ) :
    m_aStatusBarItems( rStatusBarItems )
{
    // create hash map
    for ( int i = 0; i < SB_XML_ENTRY_COUNT; i++ )
    {
        if ( StatusBarEntries[i].nNamespace == SB_NS_STATUSBAR )
        {
            OUString temp = XMLNS_STATUSBAR + XMLNS_FILTER_SEPARATOR +
                StatusBarEntries[i].aEntryName;
            m_aStatusBarMap.emplace( temp, static_cast<StatusBar_XML_Entry>(i) );
        }
        else
        {
            OUString temp = XMLNS_XLINK + XMLNS_FILTER_SEPARATOR +
                StatusBarEntries[i].aEntryName;
            m_aStatusBarMap.emplace( temp, static_cast<StatusBar_XML_Entry>(i) );
        }
    }

    m_bStatusBarStartFound          = false;
    m_bStatusBarItemStartFound      = false;
}

OReadStatusBarDocumentHandler::~OReadStatusBarDocumentHandler()
{
}

// XDocumentHandler
void SAL_CALL OReadStatusBarDocumentHandler::startDocument()
{
}

void SAL_CALL OReadStatusBarDocumentHandler::endDocument()
{
    if ( m_bStatusBarStartFound )
    {
        OUString aErrorMessage = getErrorLineString() + "No matching start or end element 'statusbar' found!";
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}

void SAL_CALL OReadStatusBarDocumentHandler::startElement(
    const OUString& aName, const Reference< XAttributeList > &xAttribs )
{
    StatusBarHashMap::const_iterator pStatusBarEntry = m_aStatusBarMap.find( aName );
    if ( pStatusBarEntry == m_aStatusBarMap.end() )
        return;

    switch ( pStatusBarEntry->second )
    {
        case SB_ELEMENT_STATUSBAR:
        {
            if ( m_bStatusBarStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "Element 'statusbar:statusbar' cannot be embedded into 'statusbar:statusbar'!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_bStatusBarStartFound = true;
        }
        break;

        case SB_ELEMENT_STATUSBARITEM:
        {
            if ( !m_bStatusBarStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "Element 'statusbar:statusbaritem' must be embedded into element 'statusbar:statusbar'!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            if ( m_bStatusBarItemStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "Element statusbar:statusbaritem is not a container!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            OUString    aCommandURL;
            OUString    aHelpURL;
            sal_Int16   nItemBits( ItemStyle::ALIGN_CENTER|ItemStyle::DRAW_IN3D|ItemStyle::MANDATORY );
            sal_Int16   nWidth( 0 );
            sal_Int16   nOffset( STATUSBAR_OFFSET );
            bool    bCommandURL( false );

            m_bStatusBarItemStartFound = true;
            for ( sal_Int16 n = 0; n < xAttribs->getLength(); n++ )
            {
                pStatusBarEntry = m_aStatusBarMap.find( xAttribs->getNameByIndex( n ) );
                if ( pStatusBarEntry != m_aStatusBarMap.end() )
                {
                    switch ( pStatusBarEntry->second )
                    {
                        case SB_ATTRIBUTE_URL:
                        {
                            bCommandURL = true;
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
                                OUString aErrorMessage = getErrorLineString() + "Attribute statusbar:align must have one value of 'left','right' or 'center'!";
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
                                OUString aErrorMessage = getErrorLineString() + "Attribute statusbar:autosize must have value 'true' or 'false'!";
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
                                OUString aErrorMessage = getErrorLineString() + "Attribute statusbar:autosize must have value 'true' or 'false'!";
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
                                OUString aErrorMessage = getErrorLineString() + "Attribute statusbar:ownerdraw must have value 'true' or 'false'!";
                                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                            }
                        }
                        break;

                        case SB_ATTRIBUTE_WIDTH:
                        {
                            nWidth = static_cast<sal_Int16>(xAttribs->getValueByIndex( n ).toInt32());
                        }
                        break;

                        case SB_ATTRIBUTE_OFFSET:
                        {
                            nOffset = static_cast<sal_Int16>(xAttribs->getValueByIndex( n ).toInt32());
                        }
                        break;

                        case SB_ATTRIBUTE_HELPURL:
                        {
                            aHelpURL = xAttribs->getValueByIndex( n );
                        }
                        break;

                        case SB_ATTRIBUTE_MANDATORY:
                        {
                            if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_BOOLEAN_TRUE )
                                nItemBits |= ItemStyle::MANDATORY;
                            else if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_BOOLEAN_FALSE )
                                nItemBits &= ~ItemStyle::MANDATORY;
                            else
                            {
                                OUString aErrorMessage = getErrorLineString() + "Attribute statusbar:mandatory must have value 'true' or 'false'!";
                                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                            }
                        }
                        break;

                        default:
                            break;
                    }
                }
            } // for

            if ( !bCommandURL )
            {
                OUString aErrorMessage = getErrorLineString() + "Required attribute statusbar:url must have a value!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }
            else
            {
                Sequence< PropertyValue > aStatusbarItemProp{
                    comphelper::makePropertyValue(ITEM_DESCRIPTOR_COMMANDURL, aCommandURL),
                    comphelper::makePropertyValue(ITEM_DESCRIPTOR_HELPURL, aHelpURL),
                    comphelper::makePropertyValue(ITEM_DESCRIPTOR_OFFSET, nOffset),
                    comphelper::makePropertyValue(ITEM_DESCRIPTOR_STYLE, nItemBits),
                    comphelper::makePropertyValue(ITEM_DESCRIPTOR_WIDTH, nWidth),
                    comphelper::makePropertyValue(ITEM_DESCRIPTOR_TYPE, css::ui::ItemType::DEFAULT)
                };

                m_aStatusBarItems->insertByIndex( m_aStatusBarItems->getCount(), Any( aStatusbarItemProp ) );
           }
        }
        break;

              default:
                  break;
    }
}

void SAL_CALL OReadStatusBarDocumentHandler::endElement(const OUString& aName)
{
    StatusBarHashMap::const_iterator pStatusBarEntry = m_aStatusBarMap.find( aName );
    if ( pStatusBarEntry == m_aStatusBarMap.end() )
        return;

    switch ( pStatusBarEntry->second )
    {
        case SB_ELEMENT_STATUSBAR:
        {
            if ( !m_bStatusBarStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "End element 'statusbar' found, but no start element 'statusbar'";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_bStatusBarStartFound = false;
        }
        break;

        case SB_ELEMENT_STATUSBARITEM:
        {
            if ( !m_bStatusBarItemStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "End element 'statusbar:statusbaritem' found, but no start element 'statusbar:statusbaritem'";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_bStatusBarItemStartFound = false;
        }
        break;

        default: break;
    }
}

void SAL_CALL OReadStatusBarDocumentHandler::characters(const OUString&)
{
}

void SAL_CALL OReadStatusBarDocumentHandler::ignorableWhitespace(const OUString&)
{
}

void SAL_CALL OReadStatusBarDocumentHandler::processingInstruction(
    const OUString& /*aTarget*/, const OUString& /*aData*/ )
{
}

void SAL_CALL OReadStatusBarDocumentHandler::setDocumentLocator(
    const Reference< XLocator > &xLocator)
{
    m_xLocator = xLocator;
}

OUString OReadStatusBarDocumentHandler::getErrorLineString()
{
    if ( m_xLocator.is() )
        return "Line: " + OUString::number( m_xLocator->getLineNumber() ) + " - ";
    else
        return OUString();
}

//  OWriteStatusBarDocumentHandler

OWriteStatusBarDocumentHandler::OWriteStatusBarDocumentHandler(
    const Reference< XIndexAccess >& aStatusBarItems,
    const Reference< XDocumentHandler >& rWriteDocumentHandler ) :
    m_aStatusBarItems( aStatusBarItems ),
    m_xWriteDocumentHandler( rWriteDocumentHandler )
{
    m_xEmptyList = new ::comphelper::AttributeList;
    m_aXMLXlinkNS       = XMLNS_XLINK_PREFIX;
    m_aXMLStatusBarNS   = XMLNS_STATUSBAR_PREFIX;
}

OWriteStatusBarDocumentHandler::~OWriteStatusBarDocumentHandler()
{
}

void OWriteStatusBarDocumentHandler::WriteStatusBarDocument()
{
    m_xWriteDocumentHandler->startDocument();

    // write DOCTYPE line!
    Reference< XExtendedDocumentHandler > xExtendedDocHandler( m_xWriteDocumentHandler, UNO_QUERY );
    if ( xExtendedDocHandler.is() )
    {
        xExtendedDocHandler->unknown( STATUSBAR_DOCTYPE );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    }

    rtl::Reference<::comphelper::AttributeList> pList = new ::comphelper::AttributeList;

    pList->AddAttribute( ATTRIBUTE_XMLNS_STATUSBAR,
                         XMLNS_STATUSBAR );

    pList->AddAttribute( ATTRIBUTE_XMLNS_XLINK,
                         XMLNS_XLINK );

    m_xWriteDocumentHandler->startElement( ELEMENT_NS_STATUSBAR, pList );
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
                WriteStatusBarItem( aCommandURL, nOffset, nStyle, nWidth );
        }
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( ELEMENT_NS_STATUSBAR );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endDocument();
}

//  protected member functions

void OWriteStatusBarDocumentHandler::WriteStatusBarItem(
    const OUString& rCommandURL,
    sal_Int16            nOffset,
    sal_Int16            nStyle,
    sal_Int16            nWidth )
{
    rtl::Reference<::comphelper::AttributeList> pList = new ::comphelper::AttributeList;

    if (m_aAttributeURL.isEmpty() )
    {
        m_aAttributeURL = m_aXMLXlinkNS + ATTRIBUTE_URL;
    }

    // save required attribute (URL)
    pList->AddAttribute( m_aAttributeURL, rCommandURL );

    // alignment
    if ( nStyle & ItemStyle::ALIGN_RIGHT )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + ATTRIBUTE_ALIGN,
                             ATTRIBUTE_ALIGN_RIGHT );
    }
    else if ( nStyle & ItemStyle::ALIGN_CENTER )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + ATTRIBUTE_ALIGN,
                             ATTRIBUTE_ALIGN_CENTER );
    }
    else
    {
        pList->AddAttribute( m_aXMLStatusBarNS + ATTRIBUTE_ALIGN,
                             ATTRIBUTE_ALIGN_LEFT );
    }

    // style ( StatusBarItemBits::In is default )
    if ( nStyle & ItemStyle::DRAW_FLAT )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + ATTRIBUTE_STYLE,
                             ATTRIBUTE_STYLE_FLAT );
    }
    else if ( nStyle & ItemStyle::DRAW_OUT3D )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + ATTRIBUTE_STYLE,
                             ATTRIBUTE_STYLE_OUT );
    }

    // autosize (default sal_False)
    if ( nStyle & ItemStyle::AUTO_SIZE )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + ATTRIBUTE_AUTOSIZE,
                             ATTRIBUTE_BOOLEAN_TRUE );
    }

    // ownerdraw (default sal_False)
    if ( nStyle & ItemStyle::OWNER_DRAW )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + ATTRIBUTE_OWNERDRAW,
                             ATTRIBUTE_BOOLEAN_TRUE );
    }

    // width (default 0)
    if ( nWidth > 0 )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + ATTRIBUTE_WIDTH,
                             OUString::number( nWidth ) );
    }

    // offset (default STATUSBAR_OFFSET)
    if ( nOffset != STATUSBAR_OFFSET )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + ATTRIBUTE_OFFSET,
                             OUString::number( nOffset ) );
    }

    // mandatory (default sal_True)
    if ( !( nStyle & ItemStyle::MANDATORY ) )
    {
        pList->AddAttribute( m_aXMLStatusBarNS + ATTRIBUTE_MANDATORY,
                             ATTRIBUTE_BOOLEAN_FALSE );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( ELEMENT_NS_STATUSBARITEM, pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( ELEMENT_NS_STATUSBARITEM );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
