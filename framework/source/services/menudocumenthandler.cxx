/*************************************************************************
 *
 *  $RCSfile: menudocumenthandler.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:18:13 $
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

#ifndef __FRAMEWORK_SERVICES_MENUDOCUMENTHANDLER_HXX_
#include <services/menudocumenthandler.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_MENUCONFIGURATION_HXX_
#include <classes/menuconfiguration.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_ADDONMENU_HXX_
#include <classes/addonmenu.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_ATTRIBUTELIST_HXX_
#include <services/attributelist.hxx>
#endif

#ifndef __COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif


using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

const int ITEMID_START_VALUE = 1000;

#define XMLNS_MENU              "http://openoffice.org/2001/menu"
#define XMLNS_PREFIX            "menu:"

#define ELEMENT_MENUBAR         "http://openoffice.org/2001/menu^menubar"
#define ELEMENT_MENU            "http://openoffice.org/2001/menu^menu"
#define ELEMENT_MENUPOPUP       "http://openoffice.org/2001/menu^menupopup"
#define ELEMENT_MENUITEM        "http://openoffice.org/2001/menu^menuitem"
#define ELEMENT_MENUSEPARATOR   "http://openoffice.org/2001/menu^menuseparator"

#define ELEMENT_NS_MENUBAR          "menu:menubar"
#define ELEMENT_NS_MENU             "menu:menu"
#define ELEMENT_NS_MENUPOPUP        "menu:menupopup"
#define ELEMENT_NS_MENUITEM         "menu:menuitem"
#define ELEMENT_NS_MENUSEPARATOR    "menu:menuseparator"

#define ATTRIBUTE_ID            "http://openoffice.org/2001/menu^id"
#define ATTRIBUTE_LABEL         "http://openoffice.org/2001/menu^label"
#define ATTRIBUTE_HELPID        "http://openoffice.org/2001/menu^helpid"
#define ATTRIBUTE_LINEBREAK     "http://openoffice.org/2001/menu^linebreak"

#define ATTRIBUTE_NS_ID         "menu:id"
#define ATTRIBUTE_NS_LABEL      "menu:label"
#define ATTRIBUTE_NS_HELPID     "menu:helpid"
#define ATTRIBUTE_NS_LINEBREAK  "menu:linebreak"

#define ATTRIBUTE_XMLNS_MENU    "xmlns:menu"

#define ATTRIBUTE_TYPE_CDATA    "CDATA"

#define MENUBAR_DOCTYPE         "<!DOCTYPE menu:menubar PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"menubar.dtd\">"


// special popup menus (filled during runtime) must be saved as a menuitem!!!
// same as in menumanager.cxx - you have to change both files!!!
#define SID_SFX_START           5000
#define SID_NEWDOCDIRECT        (SID_SFX_START + 537)
#define SID_AUTOPILOTMENU       (SID_SFX_START + 1381)
#define SID_FORMATMENU          (SID_SFX_START + 780)

const ::rtl::OUString aSlotProtocol( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
const ::rtl::OUString aSlotNewDocDirect( RTL_CONSTASCII_USTRINGPARAM( "slot:5537" ));
const ::rtl::OUString aSlotAutoPilot( RTL_CONSTASCII_USTRINGPARAM( "slot:6381" ));

const ::rtl::OUString aSpecialFileMenu( RTL_CONSTASCII_USTRINGPARAM( "file" ));
const ::rtl::OUString aSpecialWindowMenu( RTL_CONSTASCII_USTRINGPARAM( "window" ));

const ULONG  MENU_SAVE_LABEL            = 0x00000001;

namespace framework
{

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

OReadMenuDocumentHandler::OReadMenuDocumentHandler( MenuBar* pMenuBar ) :
    m_pMenuBar( pMenuBar ),
    m_nElementDepth( 0 ),
    m_bMenuBarMode( sal_False ),
    m_nItemId( ITEMID_START_VALUE )
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
        m_xReader = Reference< XDocumentHandler >( new OReadMenuBarHandler( m_pMenuBar, &m_nItemId ));
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


OReadMenuBarHandler::OReadMenuBarHandler( MenuBar* pMenuBar, USHORT* pItemId ) :
    m_pMenuBar( pMenuBar ),
    m_nElementDepth( 0 ),
    m_bMenuMode( sal_False ),
    m_pItemId( pItemId )
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

        ULONG    nHelpId = 0;
        OUString aCommandId;
        OUString aLabel;

        m_bMenuMode = sal_True;
        PopupMenu* pMenu = new PopupMenu();

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
                nHelpId = aValue.toInt32();
        }

        if ( aCommandId.getLength() > 0 )
        {
            USHORT nItemId;
            if ( aCommandId.compareTo( aSlotProtocol, aSlotProtocol.getLength() ) == 0 )
                nItemId = (USHORT) aCommandId.copy( aSlotProtocol.getLength() ).toInt32();
            else
                nItemId = ++(*m_pItemId);

            m_pMenuBar->InsertItem( nItemId, String() );
            m_pMenuBar->SetPopupMenu( nItemId, pMenu );
            m_pMenuBar->SetItemCommand( nItemId, aCommandId );
            if ( nHelpId > 0 )
                m_pMenuBar->SetHelpId( nItemId, nHelpId );
            if ( aLabel.getLength() > 0 )
            {
                m_pMenuBar->SetItemText( nItemId, aLabel );
                m_pMenuBar->SetUserValue( nItemId, MENU_SAVE_LABEL );
            }
            else
            {
                m_pMenuBar->SetUserValue( nItemId, 0 );
            }
        }
        else
        {
            delete pMenu;
            OUString aErrorMessage = getErrorLineString();
            aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "attribute id for element menu required!" ));
            throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
        }

        m_xReader = Reference< XDocumentHandler >( new OReadMenuHandler( pMenu, m_pItemId ));
        m_xReader->startDocument();
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


OReadMenuHandler::OReadMenuHandler( Menu* pMenu, USHORT* pItemId ) :
    m_pMenu( pMenu ),
    m_nElementDepth( 0 ),
    m_bMenuPopupMode( sal_False ),
    m_pItemId( pItemId )
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
        m_xReader = Reference< XDocumentHandler >( new OReadMenuPopupHandler( m_pMenu, m_pItemId ));
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


OReadMenuPopupHandler::OReadMenuPopupHandler( Menu* pMenu, USHORT* pItemId ) :
    m_pMenu( pMenu ),
    m_nElementDepth( 0 ),
    m_bMenuMode( sal_False ),
    m_pItemId( pItemId ),
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
        ULONG    nHelpId = 0;
        OUString aCommandId;
        OUString aLabel;

        m_bMenuMode = sal_True;
        PopupMenu* pMenu = new PopupMenu();

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
                nHelpId = aValue.toInt32();
        }

        if ( aCommandId.getLength() > 0 )
        {
            USHORT nItemId;
            if ( aCommandId.compareTo( aSlotProtocol, aSlotProtocol.getLength() ) == 0 )
                nItemId = (USHORT) aCommandId.copy( aSlotProtocol.getLength() ).toInt32();
            else
                nItemId = ++(*m_pItemId);

            m_pMenu->InsertItem( nItemId, String() );
            m_pMenu->SetPopupMenu( nItemId, pMenu );
            m_pMenu->SetItemCommand( nItemId, aCommandId );
            if ( nHelpId > 0 )
                m_pMenu->SetHelpId( nItemId, nHelpId );
            if ( aLabel.getLength() > 0 )
            {
                m_pMenu->SetItemText( nItemId, aLabel );
                m_pMenu->SetUserValue( nItemId, MENU_SAVE_LABEL );
            }
            else
            {
                m_pMenu->SetUserValue( nItemId, 0 );
             }
        }
        else
        {
            delete pMenu;
            OUString aErrorMessage = getErrorLineString();
            aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "attribute id for element menu required!" ));
            throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
        }

        m_xReader = Reference< XDocumentHandler >( new OReadMenuHandler( pMenu, m_pItemId ));
        m_xReader->startDocument();
    }
    else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENUITEM )))
    {
        ULONG    nHelpId = 0;
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
                nHelpId = aValue.toInt32();
        }

        if ( aCommandId.getLength() > 0 )
        {
            USHORT nItemId;
            if ( aCommandId.compareTo( aSlotProtocol, aSlotProtocol.getLength() ) == 0 )
                nItemId = (USHORT) aCommandId.copy( aSlotProtocol.getLength() ).toInt32();
            else
                nItemId = ++(*m_pItemId);

            m_pMenu->InsertItem( nItemId, String() );
            m_pMenu->SetItemCommand( nItemId, aCommandId );
            if ( nHelpId > 0 )
                m_pMenu->SetHelpId( nItemId, nHelpId );
            if ( aLabel.getLength() > 0 )
            {
                m_pMenu->SetItemText( nItemId, aLabel );
                m_pMenu->SetUserValue( nItemId, MENU_SAVE_LABEL );
            }
            else
            {
                m_pMenu->SetUserValue( nItemId, 0 );
            }
        }

        m_nNextElementExpected = ELEM_CLOSE_MENUITEM;
    }
    else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_MENUSEPARATOR )))
    {
        m_pMenu->InsertSeparator();
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


OWriteMenuDocumentHandler::OWriteMenuDocumentHandler( MenuBar* pMenu, Reference< XDocumentHandler > rxWriteDocumentHandler ) :
    m_pMenuBar( pMenu ),
    m_xWriteDocumentHandler( rxWriteDocumentHandler )
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

    WriteMenu( m_pMenuBar );

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENUBAR )) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endDocument();
}


void OWriteMenuDocumentHandler::WriteMenu( Menu* pMenu )
throw ( SAXException, RuntimeException )
{
    USHORT  nItemCount = pMenu->GetItemCount();
    BOOL    bSeparator = FALSE;

    for ( USHORT nItemPos = 0; nItemPos < nItemCount; nItemPos++ )
    {
        USHORT nItemId = pMenu->GetItemId( nItemPos );

        PopupMenu* pPopupMenu = pMenu->GetPopupMenu( nItemId );
        if ( pPopupMenu )
        {
            OUString aItemCommand = pMenu->GetItemCommand( nItemId );

            if ( nItemId == SID_NEWDOCDIRECT ||
                 nItemId == SID_AUTOPILOTMENU )
            {
                // special popup menus (filled during runtime) must be saved as a menuitem!!!
                WriteMenuItem( pMenu, nItemId );
                bSeparator = FALSE;
            }
            else if ( nItemId == SID_FORMATMENU )
            {
                // special popup menu - must be written as empty popup!
                AttributeListImpl* pListMenu = new AttributeListImpl;
                Reference< XAttributeList > xListMenu( (XAttributeList *)pListMenu , UNO_QUERY );

                String aCommand( pMenu->GetItemCommand( nItemId ) );
                if ( !aCommand.Len() )
                {
                    aCommand = String::CreateFromAscii("slot:");
                    aCommand += String::CreateFromInt32( nItemId );
                }

                pListMenu->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NS_ID )),
                                         m_aAttributeType,
                                         aCommand );

//                if ( pMenu->GetUserValue( nItemId ) & MENU_SAVE_LABEL )
                pListMenu->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NS_LABEL )),
                                            m_aAttributeType,
                                            pMenu->GetItemText( nItemId ) );

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
            else if ( !AddonPopupMenu::IsCommandURLPrefix ( aItemCommand ))
            {
                AttributeListImpl* pListMenu = new AttributeListImpl;
                Reference< XAttributeList > xListMenu( (XAttributeList *)pListMenu , UNO_QUERY );

                String aCommand( pMenu->GetItemCommand( nItemId ) );
                if ( !aCommand.Len() )
                {
                    aCommand = String::CreateFromAscii("slot:");
                    aCommand += String::CreateFromInt32( nItemId );
                }

                pListMenu->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NS_ID )),
                                         m_aAttributeType,
                                         aCommand );

//                if ( pMenu->GetUserValue( nItemId ) & MENU_SAVE_LABEL )
                    pListMenu->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NS_LABEL )),
                                             m_aAttributeType,
                                             pMenu->GetItemText( nItemId ) );

                m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENU )), xListMenu );
                m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
                m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_MENUPOPUP )), m_xEmptyList );
                m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

                WriteMenu( pPopupMenu );

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
            if ( pMenu->GetItemType( nItemPos ) != MENUITEM_SEPARATOR )
            {
                // don't save special menu items for (window list and pickup list, add-ons )
                if ( !MenuConfiguration::IsPickListItemId( nItemId ) &&
                     !MenuConfiguration::IsWindowListItemId( nItemId ) &&
                     !AddonMenuManager::IsAddonMenuId( nItemId ))
                {
                    bSeparator = FALSE;
                    WriteMenuItem( pMenu, nItemId );
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


void OWriteMenuDocumentHandler::WriteMenuItem( Menu* pMenu, USHORT nItemId )
{
    AttributeListImpl* pList = new AttributeListImpl;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    String aCommand( pMenu->GetItemCommand( nItemId ) );
    if ( !aCommand.Len() )
    {
        aCommand = String::CreateFromAscii("slot:");
        aCommand += String::CreateFromInt32( nItemId );
    }

    pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NS_ID )),
                                m_aAttributeType,
                                aCommand );

    ULONG nHelpId = pMenu->GetHelpId( nItemId );
    if ( nHelpId > 0 )
    {
        pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NS_HELPID )),
                             m_aAttributeType,
                             OUString::valueOf( sal_Int64( nHelpId )) );
    }

//    if ( pMenu->GetUserValue( nItemId ) & MENU_SAVE_LABEL )
         pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_NS_LABEL )),
                             m_aAttributeType,
                             pMenu->GetItemText( nItemId ));

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
