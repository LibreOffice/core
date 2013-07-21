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
#include <sal/macros.h>

#include <xml/menudocumenthandler.hxx>
#include <framework/menuconfiguration.hxx>
#include <framework/addonmenu.hxx>

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/attributelist.hxx>


#define XMLNS_MENU                  "http://openoffice.org/2001/menu"

#define ELEMENT_MENUBAR             "http://openoffice.org/2001/menu^menubar"
#define ELEMENT_MENU                "http://openoffice.org/2001/menu^menu"
#define ELEMENT_MENUPOPUP           "http://openoffice.org/2001/menu^menupopup"
#define ELEMENT_MENUITEM            "http://openoffice.org/2001/menu^menuitem"
#define ELEMENT_MENUSEPARATOR       "http://openoffice.org/2001/menu^menuseparator"

#define ELEMENT_NS_MENUBAR          "menu:menubar"
#define ELEMENT_NS_MENU             "menu:menu"
#define ELEMENT_NS_MENUPOPUP        "menu:menupopup"
#define ELEMENT_NS_MENUITEM         "menu:menuitem"
#define ELEMENT_NS_MENUSEPARATOR    "menu:menuseparator"

#define ATTRIBUTE_ID                "http://openoffice.org/2001/menu^id"
#define ATTRIBUTE_LABEL             "http://openoffice.org/2001/menu^label"
#define ATTRIBUTE_HELPID            "http://openoffice.org/2001/menu^helpid"
#define ATTRIBUTE_STYLE         "http://openoffice.org/2001/menu^style"

#define ATTRIBUTE_NS_ID             "menu:id"
#define ATTRIBUTE_NS_LABEL          "menu:label"
#define ATTRIBUTE_NS_HELPID         "menu:helpid"
#define ATTRIBUTE_NS_STYLE          "menu:style"

#define ATTRIBUTE_XMLNS_MENU        "xmlns:menu"

#define ATTRIBUTE_TYPE_CDATA        "CDATA"

#define MENUBAR_DOCTYPE             "<!DOCTYPE menu:menubar PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"menubar.dtd\">"

#define ATTRIBUTE_ITEMSTYLE_TEXT    "text"
#define ATTRIBUTE_ITEMSTYLE_IMAGE    "image"
#define ATTRIBUTE_ITEMSTYLE_RADIO    "radio"

// Property names of a menu/menu item ItemDescriptor
static const char ITEM_DESCRIPTOR_COMMANDURL[]  = "CommandURL";
static const char ITEM_DESCRIPTOR_HELPURL[]     = "HelpURL";
static const char ITEM_DESCRIPTOR_CONTAINER[]   = "ItemDescriptorContainer";
static const char ITEM_DESCRIPTOR_LABEL[]       = "Label";
static const char ITEM_DESCRIPTOR_TYPE[]        = "Type";
static const char ITEM_DESCRIPTOR_STYLE[]       = "Style";

// special popup menus (filled during runtime) must be saved as an empty popup menu or menuitem!!!
static const sal_Int32 CMD_PROTOCOL_SIZE        = 5;
static const char CMD_PROTOCOL[]                = ".uno:";
static const char ADDDIRECT_CMD[]               = ".uno:AddDirect" ;
static const char AUTOPILOTMENU_CMD[]           = ".uno:AutoPilotMenu" ;
static const char FILEMENU_CMD[]                = ".uno:Picklist" ;
static const char WINDOWMENU_CMD[]              = ".uno:WindowList" ;

//_________________________________________________________________________________________________________________
//  using namespaces
//_________________________________________________________________________________________________________________

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui;

namespace framework
{

struct MenuStyleItem
{
    sal_Int16 nBit;
    const char* attrName;
};

MenuStyleItem MenuItemStyles[ ] = {
    { ::com::sun::star::ui::ItemStyle::ICON, ATTRIBUTE_ITEMSTYLE_IMAGE },
    { ::com::sun::star::ui::ItemStyle::TEXT, ATTRIBUTE_ITEMSTYLE_TEXT },
    { ::com::sun::star::ui::ItemStyle::RADIO_CHECK, ATTRIBUTE_ITEMSTYLE_RADIO }
};


sal_Int32 nMenuStyleItemEntries = (sizeof (MenuItemStyles) / sizeof (MenuItemStyles[0]));

static void ExtractMenuParameters( const Sequence< PropertyValue > rProp,
                                   OUString&                       rCommandURL,
                                   OUString&                       rLabel,
                                   OUString&                       rHelpURL,
                                   Reference< XIndexAccess >&      rSubMenu,
                                   sal_Int16&                      rType,
                                   sal_Int16&                      rStyle )
{
    for ( sal_Int32 i = 0; i < rProp.getLength(); i++ )
    {
        if ( rProp[i].Name == ITEM_DESCRIPTOR_COMMANDURL )
        {
            rProp[i].Value >>= rCommandURL;
            rCommandURL = rCommandURL.intern();
        }
        else if ( rProp[i].Name == ITEM_DESCRIPTOR_HELPURL )
        {
            rProp[i].Value >>= rHelpURL;
        }
        else if ( rProp[i].Name == ITEM_DESCRIPTOR_CONTAINER )
        {
            rProp[i].Value >>= rSubMenu;
        }
        else if ( rProp[i].Name == ITEM_DESCRIPTOR_LABEL )
        {
            rProp[i].Value >>= rLabel;
        }
        else if ( rProp[i].Name == ITEM_DESCRIPTOR_TYPE )
        {
            rProp[i].Value >>= rType;
        }
        else if ( rProp[i].Name == ITEM_DESCRIPTOR_STYLE )
        {
            rProp[i].Value >>= rStyle;
        }
    }
}


// -----------------------------------------------------------------------------
// Base class implementation

ReadMenuDocumentHandlerBase::ReadMenuDocumentHandlerBase() :
    m_xLocator( 0 ),
    m_xReader( 0 ),
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
throw( SAXException, RuntimeException )
{
}

void SAL_CALL ReadMenuDocumentHandlerBase::processingInstruction(
    const OUString& /*aTarget*/, const OUString& /*aData*/ )
throw( SAXException, RuntimeException )
{
}

void SAL_CALL ReadMenuDocumentHandlerBase::setDocumentLocator(
    const Reference< XLocator > &xLocator)
throw(  SAXException, RuntimeException )
{
    m_xLocator = xLocator;
}

OUString ReadMenuDocumentHandlerBase::getErrorLineString()
{
    char buffer[32];

    if ( m_xLocator.is() )
    {
        snprintf( buffer, sizeof(buffer), "Line: %ld - ", static_cast<long>( m_xLocator->getLineNumber() ));
        return OUString::createFromAscii( buffer );
    }
    else
        return OUString();
}

void ReadMenuDocumentHandlerBase::initPropertyCommon(
    Sequence< PropertyValue > &rProps, const OUString &rCommandURL,
    const OUString &rHelpId, const OUString &rLabel, sal_Int16 nItemStyleBits )
{
    rProps[0].Name = m_aCommandURL;
    rProps[1].Name = m_aHelpURL;
    rProps[2].Name = m_aContainer;
    rProps[3].Name = m_aLabel;
    rProps[4].Name = m_aStyle;
    rProps[5].Name = m_aType;

    // Common values
    rProps[0].Value <<= rCommandURL.intern();
    rProps[1].Value <<= rHelpId;
    rProps[2].Value <<= Reference< XIndexContainer >();
    rProps[3].Value <<= rLabel;
    rProps[4].Value <<= nItemStyleBits;
    rProps[5].Value <<= ::com::sun::star::ui::ItemType::DEFAULT;
}

// -----------------------------------------------------------------------------

OReadMenuDocumentHandler::OReadMenuDocumentHandler(
    const Reference< XIndexContainer >& rMenuBarContainer )
:   m_nElementDepth( 0 ),
    m_bMenuBarMode( sal_False ),
    m_xMenuBarContainer( rMenuBarContainer ),
    m_xContainerFactory( rMenuBarContainer, UNO_QUERY )
{
}

OReadMenuDocumentHandler::~OReadMenuDocumentHandler()
{
}


void SAL_CALL OReadMenuDocumentHandler::startDocument(void)
    throw ( SAXException, RuntimeException )
{
}


void SAL_CALL OReadMenuDocumentHandler::endDocument(void)
    throw( SAXException, RuntimeException )
{
    if ( m_nElementDepth > 0 )
    {
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += "A closing element is missing!";
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}


void SAL_CALL OReadMenuDocumentHandler::startElement(
    const OUString& aName, const Reference< XAttributeList > &xAttrList )
throw( SAXException, RuntimeException )
{
    if ( m_bMenuBarMode )
    {
        ++m_nElementDepth;
        m_xReader->startElement( aName, xAttrList );
    }
    else if ( aName == ELEMENT_MENUBAR )
    {
        ++m_nElementDepth;
        m_bMenuBarMode = sal_True;
        m_xReader = Reference< XDocumentHandler >( new OReadMenuBarHandler( m_xMenuBarContainer, m_xContainerFactory ));

        m_xReader->startDocument();
    }
}


void SAL_CALL OReadMenuDocumentHandler::characters(const OUString&)
throw(  SAXException, RuntimeException )
{
}


void SAL_CALL OReadMenuDocumentHandler::endElement( const OUString& aName )
    throw( SAXException, RuntimeException )
{
    if ( m_bMenuBarMode )
    {
        --m_nElementDepth;
        m_xReader->endElement( aName );
        if ( 0 == m_nElementDepth )
        {
            m_xReader->endDocument();
            m_xReader.clear();
            m_bMenuBarMode = sal_False;
            if ( !aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENUBAR )))
            {
                OUString aErrorMessage = getErrorLineString();
                aErrorMessage += "closing element menubar expected!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }
        }
    }
}


// -----------------------------------------------------------------------------

OReadMenuBarHandler::OReadMenuBarHandler(
    const Reference< XIndexContainer >& rMenuBarContainer,
    const Reference< XSingleComponentFactory >& rFactory          )
:   m_nElementDepth( 0 ),
    m_bMenuMode( sal_False ),
    m_xMenuBarContainer( rMenuBarContainer ),
      m_xContainerFactory( rFactory )
{
}


OReadMenuBarHandler::~OReadMenuBarHandler()
{
}


void SAL_CALL OReadMenuBarHandler::startDocument(void)
    throw ( SAXException, RuntimeException )
{
}


void SAL_CALL OReadMenuBarHandler::endDocument(void)
    throw( SAXException, RuntimeException )
{
}


void SAL_CALL OReadMenuBarHandler::startElement(
    const OUString& rName, const Reference< XAttributeList > &xAttrList )
throw( SAXException, RuntimeException )
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

        m_bMenuMode = sal_True;

        // Container must be factory to create sub container
        Reference< XComponentContext > xComponentContext(
            comphelper::getProcessComponentContext() );

        Reference< XIndexContainer > xSubItemContainer;
        if ( m_xContainerFactory.is() )
            xSubItemContainer = Reference< XIndexContainer >( m_xContainerFactory->createInstanceWithContext( xComponentContext ), UNO_QUERY );

        if ( xSubItemContainer.is() )
        {
            // read attributes for menu
            for ( sal_Int16 i=0; i< xAttrList->getLength(); i++ )
            {
                OUString aName = xAttrList->getNameByIndex( i );
                OUString aValue = xAttrList->getValueByIndex( i );
                if ( aName == ATTRIBUTE_ID )
                    aCommandId = aValue;
                else if ( aName == ATTRIBUTE_LABEL )
                    aLabel = aValue;
                else if ( aName == ATTRIBUTE_HELPID )
                    aHelpId = aValue;
                else if ( aName == ATTRIBUTE_STYLE )
                {
                    OUString aTemp( aValue );
                    sal_Int32 nIndex = 0;
                    do
                    {
                        OUString aToken = aTemp.getToken( 0, '+', nIndex );
                        if ( !aToken.isEmpty() )
                        {
                            if ( aToken == ATTRIBUTE_ITEMSTYLE_TEXT )
                                nItemBits |= ::com::sun::star::ui::ItemStyle::TEXT;
                            else if ( aToken == ATTRIBUTE_ITEMSTYLE_IMAGE )
                                nItemBits |= ::com::sun::star::ui::ItemStyle::ICON;
                            else if ( aToken == ATTRIBUTE_ITEMSTYLE_RADIO )
                                nItemBits |= ::com::sun::star::ui::ItemStyle::RADIO_CHECK;
                        }
                    }
                    while ( nIndex >= 0 );
                }
            }

            if ( !aCommandId.isEmpty() )
            {
                Sequence< PropertyValue > aSubMenuProp( 6 );
                initPropertyCommon( aSubMenuProp, aCommandId, aHelpId, aLabel, nItemBits );
                aSubMenuProp[2].Value <<= xSubItemContainer;

                m_xMenuBarContainer->insertByIndex( m_xMenuBarContainer->getCount(), makeAny( aSubMenuProp ) );
            }
            else
            {
                OUString aErrorMessage = getErrorLineString();
                aErrorMessage += "attribute id for element menu required!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_xReader = Reference< XDocumentHandler >( new OReadMenuHandler( xSubItemContainer, m_xContainerFactory ));
            m_xReader->startDocument();
        }
    }
    else
    {
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += "element menu expected!";
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}


void SAL_CALL OReadMenuBarHandler::characters(const OUString&)
throw(  SAXException, RuntimeException )
{
}


void OReadMenuBarHandler::endElement( const OUString& aName )
    throw( SAXException, RuntimeException )
{
    if ( m_bMenuMode )
    {
        --m_nElementDepth;
        if ( 0 == m_nElementDepth )
        {
            m_xReader->endDocument();
            m_xReader.clear();
            m_bMenuMode = sal_False;
            if ( !aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENU )))
            {
                OUString aErrorMessage = getErrorLineString();
                aErrorMessage += "closing element menu expected!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }
        }
        else
            m_xReader->endElement( aName );
    }
}


// -----------------------------------------------------------------------------


OReadMenuHandler::OReadMenuHandler(
    const Reference< XIndexContainer >& rMenuContainer,
    const Reference< XSingleComponentFactory >& rFactory          ) :
    m_nElementDepth( 0 ),
    m_bMenuPopupMode( sal_False ),
    m_xMenuContainer( rMenuContainer ),
    m_xContainerFactory( rFactory )
{
}


OReadMenuHandler::~OReadMenuHandler()
{
}


void SAL_CALL OReadMenuHandler::startDocument(void)
    throw ( SAXException, RuntimeException )
{
}


void SAL_CALL OReadMenuHandler::endDocument(void)
    throw( SAXException, RuntimeException)
{
}


void SAL_CALL OReadMenuHandler::startElement(
    const OUString& aName, const Reference< XAttributeList > &xAttrList )
throw( SAXException, RuntimeException )
{
    if ( m_bMenuPopupMode )
    {
        ++m_nElementDepth;
        m_xReader->startElement( aName, xAttrList );
    }
    else if ( aName == ELEMENT_MENUPOPUP )
    {
        ++m_nElementDepth;
        m_bMenuPopupMode = sal_True;
        m_xReader = Reference< XDocumentHandler >( new OReadMenuPopupHandler( m_xMenuContainer, m_xContainerFactory ));
        m_xReader->startDocument();
    }
    else
    {
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += "unknown element found!";
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}


void SAL_CALL OReadMenuHandler::characters(const OUString&)
throw(  SAXException, RuntimeException )
{
}


void SAL_CALL OReadMenuHandler::endElement( const OUString& aName )
    throw( SAXException, RuntimeException )
{
    if ( m_bMenuPopupMode )
    {
        --m_nElementDepth;
        if ( 0 == m_nElementDepth )
        {
            m_xReader->endDocument();
            m_xReader.clear();
            m_bMenuPopupMode = sal_False;
            if ( !aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENUPOPUP )))
            {
                OUString aErrorMessage = getErrorLineString();
                aErrorMessage += "closing element menupopup expected!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }
        }
        else
            m_xReader->endElement( aName );
    }
}


// -----------------------------------------------------------------------------


OReadMenuPopupHandler::OReadMenuPopupHandler(
    const Reference< XIndexContainer >& rMenuContainer,
    const Reference< XSingleComponentFactory >& rFactory          ) :
    m_nElementDepth( 0 ),
    m_bMenuMode( sal_False ),
    m_xMenuContainer( rMenuContainer ),
    m_xContainerFactory( rFactory ),
    m_xComponentContext( comphelper::getProcessComponentContext() ),
    m_nNextElementExpected( ELEM_CLOSE_NONE )
{
}


OReadMenuPopupHandler::~OReadMenuPopupHandler()
{
}


void SAL_CALL OReadMenuPopupHandler::startDocument(void)
    throw ( SAXException, RuntimeException )
{
}


void SAL_CALL OReadMenuPopupHandler::endDocument(void)
    throw( SAXException, RuntimeException)
{
}

void SAL_CALL OReadMenuPopupHandler::startElement(
    const OUString& rName, const Reference< XAttributeList > &xAttrList )
throw( SAXException, RuntimeException )
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

        m_bMenuMode = sal_True;

        // Container must be factory to create sub container
        Reference< XIndexContainer > xSubItemContainer;
        if ( m_xContainerFactory.is() )
            xSubItemContainer = Reference< XIndexContainer >( m_xContainerFactory->createInstanceWithContext( m_xComponentContext ), UNO_QUERY );

        // read attributes for menu
        for ( sal_Int16 i=0; i< xAttrList->getLength(); i++ )
        {
            OUString aName = xAttrList->getNameByIndex( i );
            OUString aValue = xAttrList->getValueByIndex( i );
            if ( aName == ATTRIBUTE_ID )
                aCommandId = aValue;
            else if ( aName == ATTRIBUTE_LABEL )
                aLabel = aValue;
            else if ( aName == ATTRIBUTE_HELPID )
                aHelpId = aValue;
            else if ( aName == ATTRIBUTE_STYLE )
            {
                OUString aTemp( aValue );
                sal_Int32 nIndex = 0;
                do
                {
                    OUString aToken = aTemp.getToken( 0, '+', nIndex );
                    if ( !aToken.isEmpty() )
                    {
                        if ( aToken == ATTRIBUTE_ITEMSTYLE_TEXT )
                            nItemBits |= ::com::sun::star::ui::ItemStyle::TEXT;
                        else if ( aToken == ATTRIBUTE_ITEMSTYLE_IMAGE )
                            nItemBits |= ::com::sun::star::ui::ItemStyle::ICON;
                        else if ( aToken == ATTRIBUTE_ITEMSTYLE_RADIO )
                            nItemBits |= ::com::sun::star::ui::ItemStyle::RADIO_CHECK;
                    }
                }
                while ( nIndex >= 0 );
            }

        }

        if ( !aCommandId.isEmpty() )
        {
            Sequence< PropertyValue > aSubMenuProp( 6 );
            initPropertyCommon( aSubMenuProp, aCommandId, aHelpId, aLabel, nItemBits );
            aSubMenuProp[2].Value <<= xSubItemContainer;

            m_xMenuContainer->insertByIndex( m_xMenuContainer->getCount(), makeAny( aSubMenuProp ) );
        }
        else
        {
            OUString aErrorMessage = getErrorLineString();
            aErrorMessage += "attribute id for element menu required!";
            throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
        }

        m_xReader = Reference< XDocumentHandler >( new OReadMenuHandler( xSubItemContainer, m_xContainerFactory ));
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
            OUString aValue = xAttrList->getValueByIndex( i );
            if ( aName == ATTRIBUTE_ID )
                aCommandId = aValue;
            else if ( aName == ATTRIBUTE_LABEL )
                aLabel = aValue;
            else if ( aName == ATTRIBUTE_HELPID )
                aHelpId = aValue;
            else if ( aName == ATTRIBUTE_STYLE )
            {
                OUString aTemp( aValue );
                sal_Int32 nIndex = 0;
                do
                {
                    OUString aToken = aTemp.getToken( 0, '+', nIndex );
                    if ( !aToken.isEmpty() )
                    {
                        if ( aToken == ATTRIBUTE_ITEMSTYLE_TEXT )
                            nItemBits |= ::com::sun::star::ui::ItemStyle::TEXT;
                        else if ( aToken == ATTRIBUTE_ITEMSTYLE_IMAGE )
                            nItemBits |= ::com::sun::star::ui::ItemStyle::ICON;
                        else if ( aToken == ATTRIBUTE_ITEMSTYLE_RADIO )
                            nItemBits |= ::com::sun::star::ui::ItemStyle::RADIO_CHECK;
                    }
                }
                while ( nIndex >= 0 );
            }

        }

        if ( !aCommandId.isEmpty() )
        {
            Sequence< PropertyValue > aMenuItem( 6 );
            initPropertyCommon( aMenuItem, aCommandId, aHelpId, aLabel, nItemBits );
            aMenuItem[2].Value <<= Reference< XIndexContainer >();

            m_xMenuContainer->insertByIndex( m_xMenuContainer->getCount(), makeAny( aMenuItem ) );
        }

        m_nNextElementExpected = ELEM_CLOSE_MENUITEM;
    }
    else if ( rName == ELEMENT_MENUSEPARATOR )
    {
        Sequence< PropertyValue > aMenuSeparator( 1 );
        aMenuSeparator[0].Name = OUString( ITEM_DESCRIPTOR_TYPE );
        aMenuSeparator[0].Value <<= ::com::sun::star::ui::ItemType::SEPARATOR_LINE;

        m_xMenuContainer->insertByIndex( m_xMenuContainer->getCount(), makeAny( aMenuSeparator ) );

        m_nNextElementExpected = ELEM_CLOSE_MENUSEPARATOR;
    }
    else
    {
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += "unknown element found!";
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}


void SAL_CALL OReadMenuPopupHandler::characters(const OUString&)
throw(  SAXException, RuntimeException )
{
}


void SAL_CALL OReadMenuPopupHandler::endElement( const OUString& aName )
    throw( SAXException, RuntimeException )
{
    --m_nElementDepth;
    if ( m_bMenuMode )
    {
        if ( 0 == m_nElementDepth )
        {
            m_xReader->endDocument();
            m_xReader.clear();
            m_bMenuMode = sal_False;
            if ( !aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENU )))
            {
                OUString aErrorMessage = getErrorLineString();
                aErrorMessage += "closing element menu expected!";
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
            if ( !aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENUITEM )))
            {
                OUString aErrorMessage = getErrorLineString();
                aErrorMessage += "closing element menuitem expected!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }
        }
        else if ( m_nNextElementExpected == ELEM_CLOSE_MENUSEPARATOR )
        {
            if ( !aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENUSEPARATOR )))
            {
                OUString aErrorMessage = getErrorLineString();
                aErrorMessage += "closing element menuseparator expected!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }
        }

        m_nNextElementExpected = ELEM_CLOSE_NONE;
    }
}


// --------------------------------- Write XML ---------------------------------


OWriteMenuDocumentHandler::OWriteMenuDocumentHandler(
    const Reference< XIndexAccess >& rMenuBarContainer,
    const Reference< XDocumentHandler >& rDocumentHandler ) :
    m_xMenuBarContainer( rMenuBarContainer ),
    m_xWriteDocumentHandler( rDocumentHandler )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    m_xEmptyList = Reference< XAttributeList >( (XAttributeList *) pList, UNO_QUERY );
    m_aAttributeType =  OUString( ATTRIBUTE_TYPE_CDATA );
}


OWriteMenuDocumentHandler::~OWriteMenuDocumentHandler()
{
}


void OWriteMenuDocumentHandler::WriteMenuDocument()
throw ( SAXException, RuntimeException )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > rList( (XAttributeList *) pList , UNO_QUERY );

    m_xWriteDocumentHandler->startDocument();

    // write DOCTYPE line!
    Reference< XExtendedDocumentHandler > xExtendedDocHandler( m_xWriteDocumentHandler, UNO_QUERY );
    if ( xExtendedDocHandler.is() )
    {
        xExtendedDocHandler->unknown( OUString( MENUBAR_DOCTYPE ) );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    }

    pList->AddAttribute( OUString( ATTRIBUTE_XMLNS_MENU ),
                         m_aAttributeType,
                         OUString( XMLNS_MENU ) );

    pList->AddAttribute( OUString( ATTRIBUTE_NS_ID ),
                         m_aAttributeType,
                         OUString( "menubar" ) );

    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_MENUBAR ), pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    WriteMenu( m_xMenuBarContainer );

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_MENUBAR ) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endDocument();
}


void OWriteMenuDocumentHandler::WriteMenu( const Reference< XIndexAccess >& rMenuContainer )
throw ( SAXException, RuntimeException )
{
    sal_Int32  nItemCount = rMenuContainer->getCount();
    sal_Bool   bSeparator = sal_False;
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
            sal_Int16   nType( ::com::sun::star::ui::ItemType::DEFAULT );
            sal_Int16   nItemBits( 0 );
            Reference< XIndexAccess > xSubMenu;

            ExtractMenuParameters( aProps, aCommandURL, aLabel, aHelpURL, xSubMenu, nType, nItemBits );
            if ( xSubMenu.is() )
            {
                if ( aCommandURL.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(ADDDIRECT_CMD)) ||
                    aCommandURL.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(AUTOPILOTMENU_CMD)) )
                {
                    WriteMenuItem( aCommandURL, aLabel, aHelpURL, nItemBits );
                    bSeparator = sal_False;
                }
                else if ( !aCommandURL.isEmpty() && !AddonPopupMenu::IsCommandURLPrefix( aCommandURL ))
                {
                    ::comphelper::AttributeList* pListMenu = new ::comphelper::AttributeList;
                    Reference< XAttributeList > xListMenu( (XAttributeList *)pListMenu , UNO_QUERY );

                    pListMenu->AddAttribute( OUString( ATTRIBUTE_NS_ID ),
                                            m_aAttributeType,
                                            aCommandURL );

                    if ( !( aCommandURL.copy( CMD_PROTOCOL_SIZE ).equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(CMD_PROTOCOL))) )
                        pListMenu->AddAttribute( OUString( ATTRIBUTE_NS_LABEL ),
                                                 m_aAttributeType,
                                                 aLabel );

                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_MENU ), xListMenu );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_MENUPOPUP ), m_xEmptyList );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

                    WriteMenu( xSubMenu );

                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_MENUPOPUP ) );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_MENU ) );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    bSeparator = sal_False;
                }
            }
            else
            {
                if ( nType == ::com::sun::star::ui::ItemType::DEFAULT )
                {
                    if ( !aCommandURL.isEmpty() )
                    {
                        bSeparator = sal_False;
                        WriteMenuItem( aCommandURL, aLabel, aHelpURL, nItemBits );
                    }
                }
                else if ( !bSeparator )
                {
                    // Don't write two separators together
                    WriteMenuSeparator();
                    bSeparator = sal_True;
                }
            }
        }
    }
}


void OWriteMenuDocumentHandler::WriteMenuItem( const OUString& aCommandURL, const OUString& aLabel, const OUString& aHelpURL, sal_Int16 nStyle )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    pList->AddAttribute( OUString( ATTRIBUTE_NS_ID ),
                                m_aAttributeType,
                                aCommandURL );

    if ( !aHelpURL.isEmpty() )
    {
        pList->AddAttribute( OUString( ATTRIBUTE_NS_HELPID ),
                             m_aAttributeType,
                             aHelpURL );
    }

    if ( !aLabel.isEmpty() && !aCommandURL.copy( CMD_PROTOCOL_SIZE ).equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(CMD_PROTOCOL)) )
    {
        pList->AddAttribute( OUString( ATTRIBUTE_NS_LABEL ),
                                m_aAttributeType,
                                aLabel );
    }
    if (( nStyle > 0 ) && !( aCommandURL.copy( CMD_PROTOCOL_SIZE ).equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(CMD_PROTOCOL)) ))
    {
        OUString aValue;
        MenuStyleItem* pStyle = MenuItemStyles;

        for ( sal_Int32 nIndex = 0; nIndex < nMenuStyleItemEntries; ++nIndex, ++pStyle )
        {
            if ( nStyle & pStyle->nBit )
            {
                if ( !aValue.isEmpty() )
                    aValue = aValue.concat( OUString( "+" ) );
                aValue += OUString::createFromAscii( pStyle->attrName );
            }
        }
        pList->AddAttribute( OUString( ATTRIBUTE_NS_STYLE ),
                                m_aAttributeType,
                                aValue );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_MENUITEM ), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_MENUITEM ) );
}


void OWriteMenuDocumentHandler::WriteMenuSeparator()
{
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_MENUSEPARATOR ), m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_MENUSEPARATOR ) );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
