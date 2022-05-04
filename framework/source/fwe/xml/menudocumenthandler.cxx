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

#include <sal/macros.h>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>

#include <xml/menudocumenthandler.hxx>

#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/attributelist.hxx>

#ifdef ATTRIBUTE_HELPID
#undef ATTRIBUTE_HELPID
#endif

constexpr OUStringLiteral XMLNS_MENU = u"http://openoffice.org/2001/menu";

constexpr OUStringLiteral ELEMENT_MENUBAR = u"http://openoffice.org/2001/menu^menubar";
constexpr OUStringLiteral ELEMENT_MENU = u"http://openoffice.org/2001/menu^menu";
constexpr OUStringLiteral ELEMENT_MENUPOPUP = u"http://openoffice.org/2001/menu^menupopup";
constexpr OUStringLiteral ELEMENT_MENUITEM = u"http://openoffice.org/2001/menu^menuitem";
constexpr OUStringLiteral ELEMENT_MENUSEPARATOR = u"http://openoffice.org/2001/menu^menuseparator";

constexpr OUStringLiteral ELEMENT_NS_MENUBAR = u"menu:menubar";
constexpr OUStringLiteral ELEMENT_NS_MENU = u"menu:menu";
constexpr OUStringLiteral ELEMENT_NS_MENUPOPUP = u"menu:menupopup";
constexpr OUStringLiteral ELEMENT_NS_MENUITEM = u"menu:menuitem";
constexpr OUStringLiteral ELEMENT_NS_MENUSEPARATOR = u"menu:menuseparator";

constexpr OUStringLiteral ATTRIBUTE_ID = u"http://openoffice.org/2001/menu^id";
constexpr OUStringLiteral ATTRIBUTE_LABEL = u"http://openoffice.org/2001/menu^label";
constexpr OUStringLiteral ATTRIBUTE_HELPID = u"http://openoffice.org/2001/menu^helpid";
constexpr OUStringLiteral ATTRIBUTE_STYLE = u"http://openoffice.org/2001/menu^style";

constexpr OUStringLiteral ATTRIBUTE_NS_ID = u"menu:id";
constexpr OUStringLiteral ATTRIBUTE_NS_LABEL = u"menu:label";
constexpr OUStringLiteral ATTRIBUTE_NS_HELPID = u"menu:helpid";
constexpr OUStringLiteral ATTRIBUTE_NS_STYLE = u"menu:style";

constexpr OUStringLiteral ATTRIBUTE_XMLNS_MENU = u"xmlns:menu";

constexpr OUStringLiteral ATTRIBUTE_TYPE_CDATA = u"CDATA";

constexpr OUStringLiteral MENUBAR_DOCTYPE = u"<!DOCTYPE menu:menubar PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"menubar.dtd\">";

#define ATTRIBUTE_ITEMSTYLE_TEXT    "text"
#define ATTRIBUTE_ITEMSTYLE_IMAGE    "image"
#define ATTRIBUTE_ITEMSTYLE_RADIO    "radio"

// Property names of a menu/menu item ItemDescriptor
constexpr OUStringLiteral ITEM_DESCRIPTOR_COMMANDURL = u"CommandURL";
constexpr OUStringLiteral ITEM_DESCRIPTOR_HELPURL = u"HelpURL";
constexpr OUStringLiteral ITEM_DESCRIPTOR_CONTAINER = u"ItemDescriptorContainer";
constexpr OUStringLiteral ITEM_DESCRIPTOR_LABEL = u"Label";
constexpr OUStringLiteral ITEM_DESCRIPTOR_TYPE = u"Type";
constexpr OUStringLiteral ITEM_DESCRIPTOR_STYLE = u"Style";

//  using namespaces

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui;

namespace framework
{

namespace {

struct MenuStyleItem
{
    sal_Int16 nBit;
    const char* attrName;
};

}

const MenuStyleItem MenuItemStyles[ ] = {
    { css::ui::ItemStyle::ICON, ATTRIBUTE_ITEMSTYLE_IMAGE },
    { css::ui::ItemStyle::TEXT, ATTRIBUTE_ITEMSTYLE_TEXT },
    { css::ui::ItemStyle::RADIO_CHECK, ATTRIBUTE_ITEMSTYLE_RADIO }
};

sal_Int32 const nMenuStyleItemEntries = SAL_N_ELEMENTS(MenuItemStyles);

static void ExtractMenuParameters( const Sequence< PropertyValue >& rProp,
                                   OUString&                       rCommandURL,
                                   OUString&                       rLabel,
                                   OUString&                       rHelpURL,
                                   Reference< XIndexAccess >&      rSubMenu,
                                   sal_Int16&                      rType,
                                   sal_Int16&                      rStyle )
{
    for ( const PropertyValue& p : rProp )
    {
        if ( p.Name == ITEM_DESCRIPTOR_COMMANDURL )
        {
            p.Value >>= rCommandURL;
            rCommandURL = rCommandURL.intern();
        }
        else if ( p.Name == ITEM_DESCRIPTOR_HELPURL )
        {
            p.Value >>= rHelpURL;
        }
        else if ( p.Name == ITEM_DESCRIPTOR_CONTAINER )
        {
            p.Value >>= rSubMenu;
        }
        else if ( p.Name == ITEM_DESCRIPTOR_LABEL )
        {
            p.Value >>= rLabel;
        }
        else if ( p.Name == ITEM_DESCRIPTOR_TYPE )
        {
            p.Value >>= rType;
        }
        else if ( p.Name == ITEM_DESCRIPTOR_STYLE )
        {
            p.Value >>= rStyle;
        }
    }
}

// Base class implementation

ReadMenuDocumentHandlerBase::ReadMenuDocumentHandlerBase() :
    m_aType( ITEM_DESCRIPTOR_TYPE ),
    m_aLabel( ITEM_DESCRIPTOR_LABEL ),
    m_aContainer( ITEM_DESCRIPTOR_CONTAINER ),
    m_aHelpURL( ITEM_DESCRIPTOR_HELPURL ),
    m_aCommandURL( ITEM_DESCRIPTOR_COMMANDURL ),
    m_aStyle( ITEM_DESCRIPTOR_STYLE )
{
}

ReadMenuDocumentHandlerBase::~ReadMenuDocumentHandlerBase()
{
}

void SAL_CALL ReadMenuDocumentHandlerBase::ignorableWhitespace(
    const OUString& )
{
}

void SAL_CALL ReadMenuDocumentHandlerBase::processingInstruction(
    const OUString& /*aTarget*/, const OUString& /*aData*/ )
{
}

void SAL_CALL ReadMenuDocumentHandlerBase::setDocumentLocator(
    const Reference< XLocator > &xLocator)
{
    m_xLocator = xLocator;
}

OUString ReadMenuDocumentHandlerBase::getErrorLineString()
{
    if ( m_xLocator.is() )
        return "Line: " + OUString::number( m_xLocator->getLineNumber() ) + " - ";
    else
        return OUString();
}

void ReadMenuDocumentHandlerBase::initPropertyCommon(
    Sequence< PropertyValue > &rProps, const OUString &rCommandURL,
    const OUString &rHelpId, const OUString &rLabel, sal_Int16 nItemStyleBits )
{
    auto pProps = rProps.getArray();

    pProps[0].Name = m_aCommandURL;
    pProps[1].Name = m_aHelpURL;
    pProps[2].Name = m_aContainer;
    pProps[3].Name = m_aLabel;
    pProps[4].Name = m_aStyle;
    pProps[5].Name = m_aType;

    // Common values
    pProps[0].Value <<= rCommandURL.intern();
    pProps[1].Value <<= rHelpId;
    pProps[2].Value <<= Reference< XIndexContainer >();
    pProps[3].Value <<= rLabel;
    pProps[4].Value <<= nItemStyleBits;
    pProps[5].Value <<= css::ui::ItemType::DEFAULT;
}

OReadMenuDocumentHandler::OReadMenuDocumentHandler(
    const Reference< XIndexContainer >& rMenuBarContainer )
:   m_nElementDepth( 0 ),
    m_eReaderMode( ReaderMode::None ),
    m_xMenuBarContainer( rMenuBarContainer ),
    m_xContainerFactory( rMenuBarContainer, UNO_QUERY )
{
}

OReadMenuDocumentHandler::~OReadMenuDocumentHandler()
{
}

void SAL_CALL OReadMenuDocumentHandler::startDocument()
{
}

void SAL_CALL OReadMenuDocumentHandler::endDocument()
{
    if ( m_nElementDepth > 0 )
    {
        OUString aErrorMessage = getErrorLineString() +
            "A closing element is missing!";
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}

void SAL_CALL OReadMenuDocumentHandler::startElement(
    const OUString& aName, const Reference< XAttributeList > &xAttrList )
{
    if ( m_eReaderMode != ReaderMode::None )
    {
        ++m_nElementDepth;
        m_xReader->startElement( aName, xAttrList );
    }
    else
    {
        if ( aName == ELEMENT_MENUBAR )
        {
            m_eReaderMode = ReaderMode::MenuBar;
            m_xReader.set( new OReadMenuBarHandler( m_xMenuBarContainer, m_xContainerFactory ));
        }
        else if ( aName == ELEMENT_MENUPOPUP )
        {
            m_eReaderMode = ReaderMode::MenuPopup;
            m_xReader.set( new OReadMenuPopupHandler( m_xMenuBarContainer, m_xContainerFactory ));
        }
        ++m_nElementDepth;
        m_xReader->startDocument();
    }
}

void SAL_CALL OReadMenuDocumentHandler::characters(const OUString&)
{
}

void SAL_CALL OReadMenuDocumentHandler::endElement( const OUString& aName )
{
    if ( m_eReaderMode == ReaderMode::None )
        return;

    --m_nElementDepth;
    m_xReader->endElement( aName );
    if ( 0 != m_nElementDepth )
        return;

    m_xReader->endDocument();
    m_xReader.clear();
    if ( m_eReaderMode == ReaderMode::MenuBar && aName != ELEMENT_MENUBAR )
    {
        OUString aErrorMessage = getErrorLineString() +
            "closing element menubar expected!";
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
    else if ( m_eReaderMode == ReaderMode::MenuPopup && aName != ELEMENT_MENUPOPUP )
    {
        OUString aErrorMessage = getErrorLineString() +
            "closing element menupopup expected!";
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
    m_eReaderMode = ReaderMode::None;
}

OReadMenuBarHandler::OReadMenuBarHandler(
    const Reference< XIndexContainer >& rMenuBarContainer,
    const Reference< XSingleComponentFactory >& rFactory          )
:   m_nElementDepth( 0 ),
    m_bMenuMode( false ),
    m_xMenuBarContainer( rMenuBarContainer ),
      m_xContainerFactory( rFactory )
{
}

OReadMenuBarHandler::~OReadMenuBarHandler()
{
}

void SAL_CALL OReadMenuBarHandler::startDocument()
{
}

void SAL_CALL OReadMenuBarHandler::endDocument()
{
}

void SAL_CALL OReadMenuBarHandler::startElement(
    const OUString& rName, const Reference< XAttributeList > &xAttrList )
{
    if ( m_bMenuMode )
    {
        ++m_nElementDepth;
        m_xReader->startElement( rName, xAttrList );
    }
    else if ( rName == ELEMENT_MENU )
    {
        ++m_nElementDepth;

        OUString aHelpId;
        OUString aCommandId;
        OUString aLabel;
        sal_Int16 nItemBits(0);

        m_bMenuMode = true;

        // Container must be factory to create sub container
        Reference< XComponentContext > xComponentContext(
            comphelper::getProcessComponentContext() );

        Reference< XIndexContainer > xSubItemContainer;
        if ( m_xContainerFactory.is() )
            xSubItemContainer.set( m_xContainerFactory->createInstanceWithContext( xComponentContext ), UNO_QUERY );

        if ( xSubItemContainer.is() )
        {
            // read attributes for menu
            for ( sal_Int16 i=0; i< xAttrList->getLength(); i++ )
            {
                OUString aName = xAttrList->getNameByIndex( i );
                const OUString aValue = xAttrList->getValueByIndex( i );
                if ( aName == ATTRIBUTE_ID )
                    aCommandId = aValue;
                else if ( aName == ATTRIBUTE_LABEL )
                    aLabel = aValue;
                else if ( aName == ATTRIBUTE_HELPID )
                    aHelpId = aValue;
                else if ( aName == ATTRIBUTE_STYLE )
                {
                    sal_Int32 nIndex = 0;
                    do
                    {
                        OUString aToken = aValue.getToken( 0, '+', nIndex );
                        if ( !aToken.isEmpty() )
                        {
                            if ( aToken == ATTRIBUTE_ITEMSTYLE_TEXT )
                                nItemBits |= css::ui::ItemStyle::TEXT;
                            else if ( aToken == ATTRIBUTE_ITEMSTYLE_IMAGE )
                                nItemBits |= css::ui::ItemStyle::ICON;
                            else if ( aToken == ATTRIBUTE_ITEMSTYLE_RADIO )
                                nItemBits |= css::ui::ItemStyle::RADIO_CHECK;
                        }
                    }
                    while ( nIndex >= 0 );
                }
            }

            if ( !aCommandId.isEmpty() )
            {
                Sequence< PropertyValue > aSubMenuProp( 6 );
                initPropertyCommon( aSubMenuProp, aCommandId, aHelpId, aLabel, nItemBits );
                aSubMenuProp.getArray()[2].Value <<= xSubItemContainer;

                m_xMenuBarContainer->insertByIndex( m_xMenuBarContainer->getCount(), Any( aSubMenuProp ) );
            }
            else
            {
                OUString aErrorMessage = getErrorLineString() +
                    "attribute id for element menu required!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_xReader.set( new OReadMenuHandler( xSubItemContainer, m_xContainerFactory ));
            m_xReader->startDocument();
        }
    }
    else
    {
        OUString aErrorMessage = getErrorLineString() +
            "element menu expected!";
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}

void SAL_CALL OReadMenuBarHandler::characters(const OUString&)
{
}

void OReadMenuBarHandler::endElement( const OUString& aName )
{
    if ( !m_bMenuMode )
        return;

    --m_nElementDepth;
    if ( 0 == m_nElementDepth )
    {
        m_xReader->endDocument();
        m_xReader.clear();
        m_bMenuMode = false;
        if ( aName != ELEMENT_MENU )
        {
            OUString aErrorMessage = getErrorLineString() +
                "closing element menu expected!";
            throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
        }
    }
    else
        m_xReader->endElement( aName );
}

OReadMenuHandler::OReadMenuHandler(
    const Reference< XIndexContainer >& rMenuContainer,
    const Reference< XSingleComponentFactory >& rFactory          ) :
    m_nElementDepth( 0 ),
    m_bMenuPopupMode( false ),
    m_xMenuContainer( rMenuContainer ),
    m_xContainerFactory( rFactory )
{
}

OReadMenuHandler::~OReadMenuHandler()
{
}

void SAL_CALL OReadMenuHandler::startDocument()
{
}

void SAL_CALL OReadMenuHandler::endDocument()
{
}

void SAL_CALL OReadMenuHandler::startElement(
    const OUString& aName, const Reference< XAttributeList > &xAttrList )
{
    if ( m_bMenuPopupMode )
    {
        ++m_nElementDepth;
        m_xReader->startElement( aName, xAttrList );
    }
    else if ( aName == ELEMENT_MENUPOPUP )
    {
        ++m_nElementDepth;
        m_bMenuPopupMode = true;
        m_xReader.set( new OReadMenuPopupHandler( m_xMenuContainer, m_xContainerFactory ));
        m_xReader->startDocument();
    }
    else
    {
        OUString aErrorMessage = getErrorLineString() +
            "unknown element found!";
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}

void SAL_CALL OReadMenuHandler::characters(const OUString&)
{
}

void SAL_CALL OReadMenuHandler::endElement( const OUString& aName )
{
    if ( !m_bMenuPopupMode )
        return;

    --m_nElementDepth;
    if ( 0 == m_nElementDepth )
    {
        m_xReader->endDocument();
        m_xReader.clear();
        m_bMenuPopupMode = false;
        if ( aName != ELEMENT_MENUPOPUP )
        {
            OUString aErrorMessage = getErrorLineString() +
                "closing element menupopup expected!";
            throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
        }
    }
    else
        m_xReader->endElement( aName );
}

OReadMenuPopupHandler::OReadMenuPopupHandler(
    const Reference< XIndexContainer >& rMenuContainer,
    const Reference< XSingleComponentFactory >& rFactory          ) :
    m_nElementDepth( 0 ),
    m_bMenuMode( false ),
    m_xMenuContainer( rMenuContainer ),
    m_xContainerFactory( rFactory ),
    m_xComponentContext( comphelper::getProcessComponentContext() ),
    m_nNextElementExpected( ELEM_CLOSE_NONE )
{
}

OReadMenuPopupHandler::~OReadMenuPopupHandler()
{
}

void SAL_CALL OReadMenuPopupHandler::startDocument()
{
}

void SAL_CALL OReadMenuPopupHandler::endDocument()
{
}

void SAL_CALL OReadMenuPopupHandler::startElement(
    const OUString& rName, const Reference< XAttributeList > &xAttrList )
{
    ++m_nElementDepth;

    if ( m_bMenuMode )
        m_xReader->startElement( rName, xAttrList );
    else if ( rName == ELEMENT_MENU )
    {
        OUString aHelpId;
        OUString aCommandId;
        OUString aLabel;
        sal_Int16 nItemBits(0);

        m_bMenuMode = true;

        // Container must be factory to create sub container
        Reference< XIndexContainer > xSubItemContainer;
        if ( m_xContainerFactory.is() )
            xSubItemContainer.set( m_xContainerFactory->createInstanceWithContext( m_xComponentContext ), UNO_QUERY );

        // read attributes for menu
        for ( sal_Int16 i=0; i< xAttrList->getLength(); i++ )
        {
            OUString aName = xAttrList->getNameByIndex( i );
            const OUString aValue = xAttrList->getValueByIndex( i );
            if ( aName == ATTRIBUTE_ID )
                aCommandId = aValue;
            else if ( aName == ATTRIBUTE_LABEL )
                aLabel = aValue;
            else if ( aName == ATTRIBUTE_HELPID )
                aHelpId = aValue;
            else if ( aName == ATTRIBUTE_STYLE )
            {
                sal_Int32 nIndex = 0;
                do
                {
                    OUString aToken = aValue.getToken( 0, '+', nIndex );
                    if ( !aToken.isEmpty() )
                    {
                        if ( aToken == ATTRIBUTE_ITEMSTYLE_TEXT )
                            nItemBits |= css::ui::ItemStyle::TEXT;
                        else if ( aToken == ATTRIBUTE_ITEMSTYLE_IMAGE )
                            nItemBits |= css::ui::ItemStyle::ICON;
                        else if ( aToken == ATTRIBUTE_ITEMSTYLE_RADIO )
                            nItemBits |= css::ui::ItemStyle::RADIO_CHECK;
                    }
                }
                while ( nIndex >= 0 );
            }

        }

        if ( !aCommandId.isEmpty() )
        {
            Sequence< PropertyValue > aSubMenuProp( 6 );
            initPropertyCommon( aSubMenuProp, aCommandId, aHelpId, aLabel, nItemBits );
            aSubMenuProp.getArray()[2].Value <<= xSubItemContainer;

            m_xMenuContainer->insertByIndex( m_xMenuContainer->getCount(), Any( aSubMenuProp ) );
        }
        else
        {
            OUString aErrorMessage = getErrorLineString() +
                "attribute id for element menu required!";
            throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
        }

        m_xReader.set( new OReadMenuHandler( xSubItemContainer, m_xContainerFactory ));
        m_xReader->startDocument();
    }
    else if ( rName == ELEMENT_MENUITEM )
    {
        OUString aHelpId;
        OUString aCommandId;
        OUString aLabel;
        sal_Int16 nItemBits(0);
        // read attributes for menu item
        for ( sal_Int16 i=0; i< xAttrList->getLength(); i++ )
        {
            OUString aName = xAttrList->getNameByIndex( i );
            const OUString aValue = xAttrList->getValueByIndex( i );
            if ( aName == ATTRIBUTE_ID )
                aCommandId = aValue;
            else if ( aName == ATTRIBUTE_LABEL )
                aLabel = aValue;
            else if ( aName == ATTRIBUTE_HELPID )
                aHelpId = aValue;
            else if ( aName == ATTRIBUTE_STYLE )
            {
                sal_Int32 nIndex = 0;
                do
                {
                    OUString aToken = aValue.getToken( 0, '+', nIndex );
                    if ( !aToken.isEmpty() )
                    {
                        if ( aToken == ATTRIBUTE_ITEMSTYLE_TEXT )
                            nItemBits |= css::ui::ItemStyle::TEXT;
                        else if ( aToken == ATTRIBUTE_ITEMSTYLE_IMAGE )
                            nItemBits |= css::ui::ItemStyle::ICON;
                        else if ( aToken == ATTRIBUTE_ITEMSTYLE_RADIO )
                            nItemBits |= css::ui::ItemStyle::RADIO_CHECK;
                    }
                }
                while ( nIndex >= 0 );
            }

        }

        if ( !aCommandId.isEmpty() )
        {
            Sequence< PropertyValue > aMenuItem( 6 );
            initPropertyCommon( aMenuItem, aCommandId, aHelpId, aLabel, nItemBits );
            aMenuItem.getArray()[2].Value <<= Reference< XIndexContainer >();

            m_xMenuContainer->insertByIndex( m_xMenuContainer->getCount(), Any( aMenuItem ) );
        }

        m_nNextElementExpected = ELEM_CLOSE_MENUITEM;
    }
    else if ( rName == ELEMENT_MENUSEPARATOR )
    {
        Sequence< PropertyValue > aMenuSeparator{ comphelper::makePropertyValue(
            ITEM_DESCRIPTOR_TYPE, css::ui::ItemType::SEPARATOR_LINE) };

        m_xMenuContainer->insertByIndex( m_xMenuContainer->getCount(), Any( aMenuSeparator ) );

        m_nNextElementExpected = ELEM_CLOSE_MENUSEPARATOR;
    }
    else
    {
        OUString aErrorMessage = getErrorLineString() +
            "unknown element found!";
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}

void SAL_CALL OReadMenuPopupHandler::characters(const OUString&)
{
}

void SAL_CALL OReadMenuPopupHandler::endElement( const OUString& aName )
{
    --m_nElementDepth;
    if ( m_bMenuMode )
    {
        if ( 0 == m_nElementDepth )
        {
            m_xReader->endDocument();
            m_xReader.clear();
            m_bMenuMode = false;
            if ( aName != ELEMENT_MENU )
            {
                OUString aErrorMessage = getErrorLineString() +
                    "closing element menu expected!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }
        }
        else
            m_xReader->endElement( aName );
    }
    else
    {
        if ( m_nNextElementExpected == ELEM_CLOSE_MENUITEM )
        {
            if ( aName != ELEMENT_MENUITEM )
            {
                OUString aErrorMessage = getErrorLineString() +
                    "closing element menuitem expected!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }
        }
        else if ( m_nNextElementExpected == ELEM_CLOSE_MENUSEPARATOR )
        {
            if ( aName != ELEMENT_MENUSEPARATOR )
            {
                OUString aErrorMessage = getErrorLineString() +
                    "closing element menuseparator expected!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }
        }

        m_nNextElementExpected = ELEM_CLOSE_NONE;
    }
}

// --------------------------------- Write XML ---------------------------------

OWriteMenuDocumentHandler::OWriteMenuDocumentHandler(
    const Reference< XIndexAccess >& rMenuBarContainer,
    const Reference< XDocumentHandler >& rDocumentHandler,
    bool bIsMenuBar ) :
    m_xMenuBarContainer( rMenuBarContainer ),
    m_xWriteDocumentHandler( rDocumentHandler ),
    m_bIsMenuBar( bIsMenuBar )
{
    m_xEmptyList = new ::comphelper::AttributeList;
    m_aAttributeType = ATTRIBUTE_TYPE_CDATA;
}

OWriteMenuDocumentHandler::~OWriteMenuDocumentHandler()
{
}

void OWriteMenuDocumentHandler::WriteMenuDocument()
{
    rtl::Reference<::comphelper::AttributeList> pList = new ::comphelper::AttributeList;

    m_xWriteDocumentHandler->startDocument();

    // write DOCTYPE line!
    Reference< XExtendedDocumentHandler > xExtendedDocHandler( m_xWriteDocumentHandler, UNO_QUERY );
    if ( m_bIsMenuBar /*FIXME*/ && xExtendedDocHandler.is() )
    {
        xExtendedDocHandler->unknown( MENUBAR_DOCTYPE );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    }

    pList->AddAttribute( ATTRIBUTE_XMLNS_MENU,
                         m_aAttributeType,
                         XMLNS_MENU );

    if ( m_bIsMenuBar ) //FIXME
        pList->AddAttribute( ATTRIBUTE_NS_ID,
                             m_aAttributeType,
                             "menubar" );

    OUString aRootElement;
    if ( m_bIsMenuBar )
        aRootElement = ELEMENT_NS_MENUBAR;
    else
        aRootElement = ELEMENT_NS_MENUPOPUP;
    m_xWriteDocumentHandler->startElement( aRootElement, pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    WriteMenu( m_xMenuBarContainer );

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( aRootElement );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endDocument();
}

void OWriteMenuDocumentHandler::WriteMenu( const Reference< XIndexAccess >& rMenuContainer )
{
    sal_Int32  nItemCount = rMenuContainer->getCount();
    bool   bSeparator = false;
    Any        aAny;

    for ( sal_Int32 nItemPos = 0; nItemPos < nItemCount; nItemPos++ )
    {
        Sequence< PropertyValue > aProps;
        aAny = rMenuContainer->getByIndex( nItemPos );
        if ( aAny >>= aProps )
        {
            OUString    aCommandURL;
            OUString    aLabel;
            OUString    aHelpURL;
            sal_Int16   nType( css::ui::ItemType::DEFAULT );
            sal_Int16   nItemBits( 0 );
            Reference< XIndexAccess > xSubMenu;

            ExtractMenuParameters( aProps, aCommandURL, aLabel, aHelpURL, xSubMenu, nType, nItemBits );
            if ( xSubMenu.is() )
            {
                if ( !aCommandURL.isEmpty() )
                {
                    rtl::Reference<::comphelper::AttributeList> pListMenu = new ::comphelper::AttributeList;

                    pListMenu->AddAttribute( ATTRIBUTE_NS_ID,
                                            m_aAttributeType,
                                            aCommandURL );

                    if ( !aLabel.isEmpty() )
                        pListMenu->AddAttribute( ATTRIBUTE_NS_LABEL,
                                                 m_aAttributeType,
                                                 aLabel );

                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    m_xWriteDocumentHandler->startElement( ELEMENT_NS_MENU, pListMenu );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    m_xWriteDocumentHandler->startElement( ELEMENT_NS_MENUPOPUP, m_xEmptyList );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

                    WriteMenu( xSubMenu );

                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    m_xWriteDocumentHandler->endElement( ELEMENT_NS_MENUPOPUP );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    m_xWriteDocumentHandler->endElement( ELEMENT_NS_MENU );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    bSeparator = false;
                }
            }
            else
            {
                if ( nType == css::ui::ItemType::DEFAULT )
                {
                    if ( !aCommandURL.isEmpty() )
                    {
                        bSeparator = false;
                        WriteMenuItem( aCommandURL, aLabel, aHelpURL, nItemBits );
                    }
                }
                else if ( !bSeparator )
                {
                    // Don't write two separators together
                    WriteMenuSeparator();
                    bSeparator = true;
                }
            }
        }
    }
}

void OWriteMenuDocumentHandler::WriteMenuItem( const OUString& aCommandURL, const OUString& aLabel, const OUString& aHelpURL, sal_Int16 nStyle )
{
    rtl::Reference<::comphelper::AttributeList> pList = new ::comphelper::AttributeList;

    pList->AddAttribute( ATTRIBUTE_NS_ID,
                                m_aAttributeType,
                                aCommandURL );

    if ( !aHelpURL.isEmpty() )
    {
        pList->AddAttribute( ATTRIBUTE_NS_HELPID,
                             m_aAttributeType,
                             aHelpURL );
    }

    if ( !aLabel.isEmpty() )
    {
        pList->AddAttribute( ATTRIBUTE_NS_LABEL,
                                m_aAttributeType,
                                aLabel );
    }
    if ( nStyle > 0 )
    {
        OUStringBuffer aValue;
        const MenuStyleItem* pStyle = MenuItemStyles;

        for ( sal_Int32 nIndex = 0; nIndex < nMenuStyleItemEntries; ++nIndex, ++pStyle )
        {
            if ( nStyle & pStyle->nBit )
            {
                if ( !aValue.isEmpty() )
                    aValue.append("+");
                aValue.appendAscii( pStyle->attrName );
            }
        }
        pList->AddAttribute( ATTRIBUTE_NS_STYLE,
                                m_aAttributeType,
                                aValue.makeStringAndClear() );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( ELEMENT_NS_MENUITEM, pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( ELEMENT_NS_MENUITEM );
}

void OWriteMenuDocumentHandler::WriteMenuSeparator()
{
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( ELEMENT_NS_MENUSEPARATOR, m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( ELEMENT_NS_MENUSEPARATOR );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
