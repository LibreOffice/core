/*************************************************************************
 *
 *  $RCSfile: menudocumenthandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-03 13:23:36 $
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

#include <stdio.h>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_XML_MENUDOCUMENTHANDLER_HXX_
#include <xml/menudocumenthandler.hxx>
#endif
#ifndef __FRAMEWORK_XML_MENUCONFIGURATION_HXX_
#include <xml/menuconfiguration.hxx>
#endif
#ifndef __FRAMEWORK_CLASSES_ADDONMENU_HXX_
#include <classes/addonmenu.hxx>
#endif
#ifndef __FRAMEWORK_XML_ATTRIBUTELIST_HXX_
#include <xml/attributelist.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef __COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLECOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_ITEMTYPE_HPP_
#include <drafts/com/sun/star/ui/ItemType.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

//_________________________________________________________________________________________________________________
//  defines
//_________________________________________________________________________________________________________________

#define XMLNS_MENU                  "http://openoffice.org/2001/menu"
#define XMLNS_PREFIX                "menu:"

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
#define ATTRIBUTE_LINEBREAK         "http://openoffice.org/2001/menu^linebreak"

#define ATTRIBUTE_NS_ID             "menu:id"
#define ATTRIBUTE_NS_LABEL          "menu:label"
#define ATTRIBUTE_NS_HELPID         "menu:helpid"
#define ATTRIBUTE_NS_LINEBREAK      "menu:linebreak"

#define ATTRIBUTE_XMLNS_MENU        "xmlns:menu"

#define ATTRIBUTE_TYPE_CDATA        "CDATA"

#define MENUBAR_DOCTYPE             "<!DOCTYPE menu:menubar PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"menubar.dtd\">"

// Property names of a menu/menu item ItemDescriptor
static const char ITEM_DESCRIPTOR_COMMANDURL[]  = "CommandURL";
static const char ITEM_DESCRIPTOR_HELPURL[]     = "HelpURL";
static const char ITEM_DESCRIPTOR_CONTAINER[]   = "ItemDescriptorContainer";
static const char ITEM_DESCRIPTOR_LABEL[]       = "Label";
static const char ITEM_DESCRIPTOR_TYPE[]        = "Type";

// special popup menus (filled during runtime) must be saved as an empty popup menu or menuitem!!!
static const sal_Int32 CMD_PROTOCOL_SIZE        = 5;
static const char CMD_PROTOCOL[]                = ".uno:";
static const char ADDDIRECT_CMD[]               = ".uno:AddDirect" ;
static const char AUTOPILOTMENU_CMD[]           = ".uno:AutoPilotMenu" ;
static const char FORMATMENU_CMD[]              = ".uno:FormatMenu" ;
static const char FILEMENU_CMD[]                = ".uno:Picklist" ;
static const char WINDOWMENU_CMD[]              = ".uno:WindowList" ;

//_________________________________________________________________________________________________________________
//  using namespaces
//_________________________________________________________________________________________________________________

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;
using namespace ::drafts::com::sun::star::ui;

namespace framework
{

static void ExtractMenuParameters( const Sequence< PropertyValue > rProp,
                                   OUString&                       rCommandURL,
                                   OUString&                       rLabel,
                                   OUString&                       rHelpURL,
                                   Reference< XIndexAccess >&      rSubMenu,
                                   sal_Int16&                      rType )
{
    for ( sal_Int32 i = 0; i < rProp.getLength(); i++ )
    {
        if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_COMMANDURL ))
        {
            rProp[i].Value >>= rCommandURL;
        }
        else if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_HELPURL ))
        {
            rProp[i].Value >>= rHelpURL;
        }
        else if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_CONTAINER ))
        {
            rProp[i].Value >>= rSubMenu;
        }
        else if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_LABEL ))
        {
            rProp[i].Value >>= rLabel;
        }
        else if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_TYPE ))
        {
            rProp[i].Value >>= rType;
        }
    }
}


// -----------------------------------------------------------------------------
// Base class implementation

ReadMenuDocumentHandlerBase::ReadMenuDocumentHandlerBase() :
    m_xLocator( 0 ),
    m_xReader( 0 )
{
}

ReadMenuDocumentHandlerBase::~ReadMenuDocumentHandlerBase()
{
}

Any SAL_CALL ReadMenuDocumentHandlerBase::queryInterface(
    const Type & rType )
throw( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                rType ,
                SAL_STATIC_CAST( XDocumentHandler*, this ));
    if ( a.hasValue() )
        return a;

    return OWeakObject::queryInterface( rType );
}

void SAL_CALL ReadMenuDocumentHandlerBase::ignorableWhitespace(
    const OUString& aWhitespaces )
throw( SAXException, RuntimeException )
{
}

void SAL_CALL ReadMenuDocumentHandlerBase::processingInstruction(
    const OUString& aTarget, const OUString& aData )
throw( SAXException, RuntimeException )
{
}

void SAL_CALL ReadMenuDocumentHandlerBase::setDocumentLocator(
    const Reference< XLocator > &xLocator)
throw(  SAXException, RuntimeException )
{
    m_xLocator = xLocator;
}

::rtl::OUString ReadMenuDocumentHandlerBase::getErrorLineString()
{
    char buffer[32];

    if ( m_xLocator.is() )
    {
        snprintf( buffer, sizeof(buffer), "Line: %ld - ", m_xLocator->getLineNumber() );
        return OUString::createFromAscii( buffer );
    }
    else
        return OUString();
}

// -----------------------------------------------------------------------------

// #110897#
OReadMenuDocumentHandler::OReadMenuDocumentHandler(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    const Reference< XIndexContainer >& rMenuBarContainer )
:   // #110897#
    mxServiceFactory(xServiceFactory),
    m_xMenuBarContainer( rMenuBarContainer ),
    m_xContainerFactory( rMenuBarContainer, UNO_QUERY ),
    m_nElementDepth( 0 ),
    m_bMenuBarMode( sal_False )
{
}

// #110897#
const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& OReadMenuDocumentHandler::getServiceFactory()
{
    // #110897#
    return mxServiceFactory;
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
        aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "A closing element is missing!" ));
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
    else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENUBAR )))
    {
        ++m_nElementDepth;
        m_bMenuBarMode = sal_True;

        // #110897# m_xReader = Reference< XDocumentHandler >( new OReadMenuBarHandler( m_xMenuBarContainer, m_xContainerFactory ));
        m_xReader = Reference< XDocumentHandler >( new OReadMenuBarHandler( getServiceFactory(), m_xMenuBarContainer, m_xContainerFactory ));

        m_xReader->startDocument();
    }
}


void SAL_CALL OReadMenuDocumentHandler::characters(const rtl::OUString& aChars)
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
            m_xReader = Reference< XDocumentHandler >();
            m_bMenuBarMode = sal_False;
            if ( !aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENUBAR )))
            {
                OUString aErrorMessage = getErrorLineString();
                aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "closing element menubar expected!" ));
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }
        }
    }
}


// -----------------------------------------------------------------------------


// #110897#
OReadMenuBarHandler::OReadMenuBarHandler(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    const Reference< XIndexContainer >& rMenuBarContainer,
    const Reference< XSingleComponentFactory >& rFactory          )
:   // #110897#
    mxServiceFactory( xServiceFactory ),
    m_xMenuBarContainer( rMenuBarContainer ),
    m_xContainerFactory( rFactory ),
    m_nElementDepth( 0 ),
    m_bMenuMode( sal_False )
{
}

// #110897#
const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& OReadMenuBarHandler::getServiceFactory()
{
    // #110897#
    return mxServiceFactory;
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
    const OUString& aName, const Reference< XAttributeList > &xAttrList )
throw( SAXException, RuntimeException )
{
    if ( m_bMenuMode )
    {
        ++m_nElementDepth;
        m_xReader->startElement( aName, xAttrList );
    }
    else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENU )))
    {
        ++m_nElementDepth;

        OUString aHelpId;
        OUString aCommandId;
        OUString aLabel;

        m_bMenuMode = sal_True;

        // Container must be factory to create sub container
        Reference< XComponentContext > xComponentContext;
        Reference< XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY );
        xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>=
            xComponentContext;

        Reference< XIndexContainer > xSubItemContainer;
        if ( m_xContainerFactory.is() )
            xSubItemContainer = Reference< XIndexContainer >( m_xContainerFactory->createInstanceWithContext( xComponentContext ), UNO_QUERY );

        if ( xSubItemContainer.is() )
        {
            // read attributes for menu
            for ( int i=0; i< xAttrList->getLength(); i++ )
            {
                OUString aName = xAttrList->getNameByIndex( i );
                OUString aValue = xAttrList->getValueByIndex( i );
                if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_ID )))
                    aCommandId = aValue;
                else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_LABEL )))
                    aLabel = aValue;
                else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_HELPID )))
                    aHelpId = aValue;
            }

            if ( aCommandId.getLength() > 0 )
            {
                Sequence< PropertyValue > aSubMenuProp( 5 );
                aSubMenuProp[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_COMMANDURL ));
                aSubMenuProp[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_HELPURL ));
                aSubMenuProp[2].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_CONTAINER ));
                aSubMenuProp[3].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_LABEL ));
                aSubMenuProp[4].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_TYPE ));

                aSubMenuProp[0].Value <<= aCommandId;
                aSubMenuProp[1].Value <<= aHelpId;
                aSubMenuProp[2].Value <<= xSubItemContainer;
                aSubMenuProp[3].Value <<= aLabel;
                aSubMenuProp[4].Value <<= drafts::com::sun::star::ui::ItemType::DEFAULT;

                m_xMenuBarContainer->insertByIndex( m_xMenuBarContainer->getCount(), makeAny( aSubMenuProp ) );
            }
            else
            {
                OUString aErrorMessage = getErrorLineString();
                aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "attribute id for element menu required!" ));
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_xReader = Reference< XDocumentHandler >( new OReadMenuHandler( xSubItemContainer, m_xContainerFactory ));
            m_xReader->startDocument();
        }
    }
    else
    {
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "element menu expected!" ));
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}


void SAL_CALL OReadMenuBarHandler::characters(const rtl::OUString& aChars)
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
            m_xReader = Reference< XDocumentHandler >();
            m_bMenuMode = sal_False;
            if ( !aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENU )))
            {
                OUString aErrorMessage = getErrorLineString();
                aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "closing element menu expected!" ));
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
    m_xMenuContainer( rMenuContainer ),
    m_xContainerFactory( rFactory ),
    m_nElementDepth( 0 ),
    m_bMenuPopupMode( sal_False )
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
    else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENUPOPUP )))
    {
        ++m_nElementDepth;
        m_bMenuPopupMode = sal_True;
        m_xReader = Reference< XDocumentHandler >( new OReadMenuPopupHandler( m_xMenuContainer, m_xContainerFactory ));
        m_xReader->startDocument();
    }
    else
    {
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "unknown element found!" ));
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}


void SAL_CALL OReadMenuHandler::characters(const rtl::OUString& aChars)
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
            m_xReader = Reference< XDocumentHandler >();
            m_bMenuPopupMode = sal_False;
            if ( !aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENUPOPUP )))
            {
                OUString aErrorMessage = getErrorLineString();
                aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "closing element menupopup expected!" ));
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
    m_xMenuContainer( rMenuContainer ),
    m_xContainerFactory( rFactory ),
    m_nElementDepth( 0 ),
    m_bMenuMode( sal_False ),
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
    const OUString& aName, const Reference< XAttributeList > &xAttrList )
throw( SAXException, RuntimeException )
{
    ++m_nElementDepth;

    if ( m_bMenuMode )
        m_xReader->startElement( aName, xAttrList );
    else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENU )))
    {
        OUString aHelpId;
        OUString aCommandId;
        OUString aLabel;

        m_bMenuMode = sal_True;

        // Container must be factory to create sub container
        Reference< XComponentContext > xComponentContext;
        Reference< XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY );
        xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>=
            xComponentContext;

        Reference< XIndexContainer > xSubItemContainer;
        if ( m_xContainerFactory.is() )
            xSubItemContainer = Reference< XIndexContainer >( m_xContainerFactory->createInstanceWithContext( xComponentContext ), UNO_QUERY );

        // read attributes for menu
        for ( int i=0; i< xAttrList->getLength(); i++ )
        {
            OUString aName = xAttrList->getNameByIndex( i );
            OUString aValue = xAttrList->getValueByIndex( i );
            if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_ID )))
                aCommandId = aValue;
            else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_LABEL )))
                aLabel = aValue;
            else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_HELPID )))
                aHelpId = aValue;
        }

        if ( aCommandId.getLength() > 0 )
        {
            Sequence< PropertyValue > aSubMenuProp( 5 );
            aSubMenuProp[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_COMMANDURL ));
            aSubMenuProp[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_HELPURL ));
            aSubMenuProp[2].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_CONTAINER ));
            aSubMenuProp[3].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_LABEL ));
            aSubMenuProp[4].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_TYPE ));

            aSubMenuProp[0].Value <<= aCommandId;
            aSubMenuProp[1].Value <<= aHelpId;
            aSubMenuProp[2].Value <<= xSubItemContainer;
            aSubMenuProp[3].Value <<= aLabel;
            aSubMenuProp[4].Value <<= drafts::com::sun::star::ui::ItemType::DEFAULT;

            m_xMenuContainer->insertByIndex( m_xMenuContainer->getCount(), makeAny( aSubMenuProp ) );
        }
        else
        {
            OUString aErrorMessage = getErrorLineString();
            aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "attribute id for element menu required!" ));
            throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
        }

        m_xReader = Reference< XDocumentHandler >( new OReadMenuHandler( xSubItemContainer, m_xContainerFactory ));
        m_xReader->startDocument();
    }
    else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENUITEM )))
    {
        OUString aHelpId;
        OUString aCommandId;
        OUString aLabel;

        // read attributes for menu item
        for ( int i=0; i< xAttrList->getLength(); i++ )
        {
            OUString aName = xAttrList->getNameByIndex( i );
            OUString aValue = xAttrList->getValueByIndex( i );
            if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_ID )))
                aCommandId = aValue;
            else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_LABEL )))
                aLabel = aValue;
            else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_HELPID )))
                aHelpId = aValue;
        }

        if ( aCommandId.getLength() > 0 )
        {
            Sequence< PropertyValue > aMenuItem( 5 );
            aMenuItem[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_COMMANDURL ));
            aMenuItem[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_HELPURL ));
            aMenuItem[2].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_CONTAINER ));
            aMenuItem[3].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_LABEL ));
            aMenuItem[4].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_TYPE ));

            aMenuItem[0].Value <<= aCommandId;
            aMenuItem[1].Value <<= aHelpId;
            aMenuItem[2].Value <<= Reference< XIndexContainer >();
            aMenuItem[3].Value <<= aLabel;
            aMenuItem[4].Value <<= drafts::com::sun::star::ui::ItemType::DEFAULT;

            m_xMenuContainer->insertByIndex( m_xMenuContainer->getCount(), makeAny( aMenuItem ) );
        }

        m_nNextElementExpected = ELEM_CLOSE_MENUITEM;
    }
    else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENUSEPARATOR )))
    {
        Sequence< PropertyValue > aMenuSeparator( 1 );
        aMenuSeparator[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_TYPE ));
        aMenuSeparator[0].Value <<= drafts::com::sun::star::ui::ItemType::SEPARATOR_LINE;

        m_xMenuContainer->insertByIndex( m_xMenuContainer->getCount(), makeAny( aMenuSeparator ) );

        m_nNextElementExpected = ELEM_CLOSE_MENUSEPARATOR;
    }
    else
    {
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "unknown element found!" ));
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}


void SAL_CALL OReadMenuPopupHandler::characters(const rtl::OUString& aChars)
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
            m_xReader = Reference< XDocumentHandler >();
            m_bMenuMode = sal_False;
            if ( !aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENU )))
            {
                OUString aErrorMessage = getErrorLineString();
                aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "closing element menu expected!" ));
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
                aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "closing element menuitem expected!" ));
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }
        }
        else if ( m_nNextElementExpected == ELEM_CLOSE_MENUSEPARATOR )
        {
            if ( !aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENUSEPARATOR )))
            {
                OUString aErrorMessage = getErrorLineString();
                aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "closing element menuseparator expected!" ));
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
    m_xEmptyList = Reference< XAttributeList >( (XAttributeList *)new AttributeListImpl, UNO_QUERY );
    m_aAttributeType =  OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_TYPE_CDATA ));
}


OWriteMenuDocumentHandler::~OWriteMenuDocumentHandler()
{
}


void OWriteMenuDocumentHandler::WriteMenuDocument()
throw ( SAXException, RuntimeException )
{
    AttributeListImpl* pList = new AttributeListImpl;
    Reference< XAttributeList > rList( (XAttributeList *) pList , UNO_QUERY );

    m_xWriteDocumentHandler->startDocument();

    // write DOCTYPE line!
    Reference< XExtendedDocumentHandler > xExtendedDocHandler( m_xWriteDocumentHandler, UNO_QUERY );
    if ( xExtendedDocHandler.is() )
    {
        xExtendedDocHandler->unknown( OUString( RTL_CONSTASCII_USTRINGPARAM( MENUBAR_DOCTYPE )) );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    }

    pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XMLNS_MENU )),
                         m_aAttributeType,
                         OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_MENU )) );

    pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NS_ID )),
                         m_aAttributeType,
                         OUString( RTL_CONSTASCII_USTRINGPARAM( "menubar" )) );

    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENUBAR )), pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    WriteMenu( m_xMenuBarContainer );

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENUBAR )) );
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
            sal_Int16   nType( drafts::com::sun::star::ui::ItemType::DEFAULT );
            Reference< XIndexAccess > xSubMenu;

            ExtractMenuParameters( aProps, aCommandURL, aLabel, aHelpURL, xSubMenu, nType );
            if ( xSubMenu.is() )
            {
                if ( aCommandURL.equalsAscii( ADDDIRECT_CMD ) ||
                    aCommandURL.equalsAscii( AUTOPILOTMENU_CMD ))
                {
                    WriteMenuItem( aCommandURL, aLabel, aHelpURL );
                    bSeparator = sal_False;
                }
                else if ( aCommandURL.equalsAscii( FORMATMENU_CMD ))
                {
                    // special popup menu - must be written as empty popup!
                    AttributeListImpl* pListMenu = new AttributeListImpl;
                    Reference< XAttributeList > xListMenu( (XAttributeList *)pListMenu , UNO_QUERY );

                    pListMenu->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NS_ID )),
                                            m_aAttributeType,
                                            aCommandURL );

                    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENU )), xListMenu );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENUPOPUP )), m_xEmptyList );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENUPOPUP )) );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENU )) );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    bSeparator = FALSE;
                }
                else if (( aCommandURL.getLength() > 0 ) && !AddonPopupMenu::IsCommandURLPrefix ( aCommandURL ))
                {
                    AttributeListImpl* pListMenu = new AttributeListImpl;
                    Reference< XAttributeList > xListMenu( (XAttributeList *)pListMenu , UNO_QUERY );

                    pListMenu->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NS_ID )),
                                            m_aAttributeType,
                                            aCommandURL );

                    if ( !( aCommandURL.copy( CMD_PROTOCOL_SIZE ).equalsAscii( CMD_PROTOCOL )))
                        pListMenu->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NS_LABEL )),
                                                 m_aAttributeType,
                                                 aLabel );

                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENU )), xListMenu );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENUPOPUP )), m_xEmptyList );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

                    WriteMenu( xSubMenu );

                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENUPOPUP )) );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENU )) );
                    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                    bSeparator = FALSE;
                }
            }
            else
            {
                if ( nType == drafts::com::sun::star::ui::ItemType::DEFAULT )
                {
                    if ( aCommandURL.getLength() > 0 )
                    {
                        bSeparator = FALSE;
                        WriteMenuItem( aCommandURL, aLabel, aHelpURL );
                    }
                }
                else if ( !bSeparator )
                {
                    // Don't write two separators together
                    WriteMenuSeparator();
                    bSeparator = TRUE;
                }
            }
        }
    }
}


void OWriteMenuDocumentHandler::WriteMenuItem( const OUString& aCommandURL, const OUString& aLabel, const OUString& aHelpURL)
{
    AttributeListImpl* pList = new AttributeListImpl;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NS_ID )),
                                m_aAttributeType,
                                aCommandURL );

    if ( aHelpURL.getLength() > 0 )
    {
        pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NS_HELPID )),
                             m_aAttributeType,
                             aHelpURL );
    }

    if (( aLabel.getLength() > 0 ) && !( aCommandURL.copy( CMD_PROTOCOL_SIZE ).equalsAscii( CMD_PROTOCOL )))
    {
        pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NS_LABEL )),
                                m_aAttributeType,
                                aLabel );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENUITEM )), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENUITEM )) );
}


void OWriteMenuDocumentHandler::WriteMenuSeparator()
{
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENUSEPARATOR )), m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENUSEPARATOR )) );
}

} // namespace framework
