/*************************************************************************
 *
 *  $RCSfile: toolboxdocumenthandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-02-25 17:55:57 $
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

#include <threadhelp/resetableguard.hxx>
#include <xml/toolboxdocumenthandler.hxx>
#include <macros/debug.hxx>
#include <xml/attributelist.hxx>
#include <xml/toolboxconfigurationdefines.hxx>

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

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;


#define TOOLBAR_DOCTYPE             "<!DOCTYPE toolbar:toolbar PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"toolbar.dtd\">"

namespace framework
{

struct ToolBarEntryProperty
{
    OReadToolBoxDocumentHandler::ToolBox_XML_Namespace  nNamespace;
    char                                                aEntryName[20];
};

ToolBarEntryProperty ToolBoxEntries[OReadToolBoxDocumentHandler::TB_XML_ENTRY_COUNT] =
{
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ELEMENT_TOOLBAR             },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ELEMENT_TOOLBARITEM         },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ELEMENT_TOOLBARSPACE        },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ELEMENT_TOOLBARBREAK        },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ELEMENT_TOOLBARSEPARATOR    },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ATTRIBUTE_TEXT              },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ATTRIBUTE_BITMAP            },
    { OReadToolBoxDocumentHandler::TB_NS_XLINK,     ATTRIBUTE_URL               },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ATTRIBUTE_ITEMBITS          },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ATTRIBUTE_VISIBLE           },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ATTRIBUTE_WIDTH             },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ATTRIBUTE_USER              },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ATTRIBUTE_HELPID            },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ATTRIBUTE_ITEMSTYLE         }
};


OReadToolBoxDocumentHandler::OReadToolBoxDocumentHandler( ToolBoxDescriptor& aToolBoxItems ) :
    ThreadHelpBase( &Application::GetSolarMutex() ), ::cppu::OWeakObject(),     m_aToolBoxItems( aToolBoxItems )
{
    OUString aNamespaceToolBar( RTL_CONSTASCII_USTRINGPARAM( XMLNS_TOOLBAR ));
    OUString aNamespaceXLink( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK ));
    OUString aSeparator( RTL_CONSTASCII_USTRINGPARAM( XMLNS_FILTER_SEPARATOR ));

    // create hash map
    for ( int i = 0; i <= (int)TB_XML_ENTRY_COUNT; i++ )
    {
        if ( ToolBoxEntries[i].nNamespace == TB_NS_TOOLBAR )
        {
            OUString temp( aNamespaceToolBar );
            temp += aSeparator;
            temp += OUString::createFromAscii( ToolBoxEntries[i].aEntryName );
            m_aToolBoxMap.insert( ToolBoxHashMap::value_type( temp, (ToolBox_XML_Entry)i ) );
        }
        else
        {
            OUString temp( aNamespaceXLink );
            temp += aSeparator;
            temp += OUString::createFromAscii( ToolBoxEntries[i].aEntryName );
            m_aToolBoxMap.insert( ToolBoxHashMap::value_type( temp, (ToolBox_XML_Entry)i ) );
        }
    }

    // pre-calculate a hash code for all style strings to speed up xml read process
    m_nHashCode_Style_Radio     = OUString::createFromAscii( ATTRIBUTE_ITEMSTYLE_RADIO ).hashCode();
    m_nHashCode_Style_Auto      = OUString::createFromAscii( ATTRIBUTE_ITEMSTYLE_AUTO ).hashCode();
    m_nHashCode_Style_Left      = OUString::createFromAscii( ATTRIBUTE_ITEMSTYLE_LEFT ).hashCode();
    m_nHashCode_Style_AutoSize  = OUString::createFromAscii( ATTRIBUTE_ITEMSTYLE_AUTOSIZE ).hashCode();
    m_nHashCode_Style_DropDown  = OUString::createFromAscii( ATTRIBUTE_ITEMSTYLE_DROPDOWN ).hashCode();
    m_nHashCode_Style_Repeat    = OUString::createFromAscii( ATTRIBUTE_ITEMSTYLE_REPEAT ).hashCode();

    m_bToolBarStartFound            = sal_False;
    m_bToolBarEndFound              = sal_False;
    m_bToolBarItemStartFound        = sal_False;
    m_bToolBarSpaceStartFound       = sal_False;
    m_bToolBarBreakStartFound       = sal_False;
    m_bToolBarSeparatorStartFound   = sal_False;
}

OReadToolBoxDocumentHandler::~OReadToolBoxDocumentHandler()
{
}

Any SAL_CALL OReadToolBoxDocumentHandler::queryInterface( const Type & rType )
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
void SAL_CALL OReadToolBoxDocumentHandler::startDocument(void)
throw ( SAXException, RuntimeException )
{
}

void SAL_CALL OReadToolBoxDocumentHandler::endDocument(void)
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if (( m_bToolBarStartFound && !m_bToolBarEndFound ) ||
        ( !m_bToolBarStartFound && m_bToolBarEndFound )     )
    {
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "No matching start or end element 'toolbar' found!" ));
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}

void SAL_CALL OReadToolBoxDocumentHandler::startElement(
    const OUString& aName, const Reference< XAttributeList > &xAttribs )
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    ToolBoxHashMap::const_iterator pToolBoxEntry = m_aToolBoxMap.find( aName ) ;
    if ( pToolBoxEntry != m_aToolBoxMap.end() )
    {
        switch ( pToolBoxEntry->second )
        {
            case TB_ELEMENT_TOOLBAR:
            {
                if ( m_bToolBarStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'toolbar:toolbar' cannot be embeded into 'toolbar:toolbar'!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarStartFound = sal_True;
            }
            break;

            case TB_ELEMENT_TOOLBARITEM:
            {
                if ( !m_bToolBarStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'toolbar:toolbaritem' must be embeded into element 'toolbar:toolbar'!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_bToolBarSeparatorStartFound ||
                     m_bToolBarBreakStartFound ||
                     m_bToolBarSpaceStartFound ||
                     m_bToolBarItemStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element toolbar:toolbaritem is not a container!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                OUString aAttribute;
                sal_Bool bAttributeURL  = sal_False;

                m_bToolBarItemStartFound = sal_True;

                ToolBoxItemDescriptor* pItem = new ToolBoxItemDescriptor;

                m_aToolBoxItems.Insert( pItem, m_aToolBoxItems.Count() );
                pItem->nItemType = TOOLBOXITEM_BUTTON;

                for ( int n = 0; n < xAttribs->getLength(); n++ )
                {
                    pToolBoxEntry = m_aToolBoxMap.find( xAttribs->getNameByIndex( n ) );
                    if ( pToolBoxEntry != m_aToolBoxMap.end() )
                    {
                        switch ( pToolBoxEntry->second )
                        {
                            case TB_ATTRIBUTE_TEXT:
                            {
                                pItem->aItemText = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case TB_ATTRIBUTE_BITMAP:
                            {
                                pItem->aBitmapName = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case TB_ATTRIBUTE_URL:
                            {
                                bAttributeURL   = sal_True;
                                pItem->aURL     = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case TB_ATTRIBUTE_ITEMBITS:
                            {
                                pItem->nItemBits = (USHORT)(xAttribs->getValueByIndex( n ).toInt32());
                            }
                            break;

                            case TB_ATTRIBUTE_VISIBLE:
                            {
                                pItem->nVisible = 0;
                                if ( xAttribs->getValueByIndex( n ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_BOOLEAN_TRUE )) )
                                    pItem->nVisible = 1;
                                else if ( xAttribs->getValueByIndex( n ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_BOOLEAN_FALSE )) )
                                    pItem->nVisible = 0;
                                else
                                {
                                    OUString aErrorMessage = getErrorLineString();
                                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Attribute toolbar:visible must have value 'true' or 'false'!" ));
                                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                                }
                            }
                            break;

                            case TB_ATTRIBUTE_WIDTH:
                            {
                                pItem->nWidth = (USHORT)(xAttribs->getValueByIndex( n ).toInt32());
                            }
                            break;

                            case TB_ATTRIBUTE_USER:
                            {
                                pItem->nUserDef = (USHORT)(xAttribs->getValueByIndex( n ).toInt32());
                            }
                            break;

                            case TB_ATTRIBUTE_HELPID:
                            {
                                pItem->aHelpId = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case TB_ATTRIBUTE_STYLE:
                            {
                                // read space seperated item style list
                                OUString aTemp = xAttribs->getValueByIndex( n );
                                sal_Int32 nIndex = 0;

                                pItem->nItemBits = 0;
                                do
                                {
                                    OUString aToken = aTemp.getToken( 0, ' ', nIndex );
                                    if ( aToken.getLength() > 0 )
                                    {
                                        sal_Int32 nHashCode = aToken.hashCode();
                                        if ( nHashCode == m_nHashCode_Style_Radio )
                                            pItem->nItemBits |= TIB_RADIOCHECK;
                                        else if ( nHashCode == m_nHashCode_Style_Auto )
                                            pItem->nItemBits |= TIB_AUTOCHECK;
                                        else if ( nHashCode == m_nHashCode_Style_Left )
                                            pItem->nItemBits |= TIB_LEFT;
                                        else if ( nHashCode == m_nHashCode_Style_AutoSize )
                                            pItem->nItemBits |= TIB_AUTOSIZE;
                                        else if ( nHashCode == m_nHashCode_Style_DropDown )
                                            pItem->nItemBits |= TIB_DROPDOWN;
                                        else if ( nHashCode == m_nHashCode_Style_Repeat )
                                            pItem->nItemBits |= TIB_REPEAT;
                                    }
                                }
                                while ( nIndex >= 0 );
                            }
                            break;
                        }
                    }
                } // for

                if ( !bAttributeURL )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Required attribute toolbar:url must have a value!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }
            }
            break;

            case TB_ELEMENT_TOOLBARSPACE:
            {
                if ( m_bToolBarSeparatorStartFound ||
                     m_bToolBarBreakStartFound ||
                     m_bToolBarSpaceStartFound ||
                     m_bToolBarItemStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element toolbar:toolbarspace is not a container!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarSpaceStartFound = sal_True;
                ToolBoxItemDescriptor* pItem = new ToolBoxItemDescriptor;
                pItem->nItemType = TOOLBOXITEM_SPACE;
                m_aToolBoxItems.Insert( pItem, m_aToolBoxItems.Count() );
            }
            break;

            case TB_ELEMENT_TOOLBARBREAK:
            {
                if ( m_bToolBarSeparatorStartFound ||
                     m_bToolBarBreakStartFound ||
                     m_bToolBarSpaceStartFound ||
                     m_bToolBarItemStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element toolbar:toolbarbreak is not a container!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarBreakStartFound = sal_True;
                ToolBoxItemDescriptor* pItem = new ToolBoxItemDescriptor;
                pItem->nItemType = TOOLBOXITEM_BREAK;
                m_aToolBoxItems.Insert( pItem, m_aToolBoxItems.Count() );
            }
            break;

            case TB_ELEMENT_TOOLBARSEPARATOR:
            {
                if ( m_bToolBarSeparatorStartFound ||
                     m_bToolBarBreakStartFound ||
                     m_bToolBarSpaceStartFound ||
                     m_bToolBarItemStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element toolbar:toolbarseparator is not a container!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarSeparatorStartFound = sal_True;
                ToolBoxItemDescriptor* pItem = new ToolBoxItemDescriptor;
                pItem->nItemType = TOOLBOXITEM_SEPARATOR;
                m_aToolBoxItems.Insert( pItem, m_aToolBoxItems.Count() );
            }
            break;
        }
    }
}

void SAL_CALL OReadToolBoxDocumentHandler::endElement(const OUString& aName)
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    ToolBoxHashMap::const_iterator pToolBoxEntry = m_aToolBoxMap.find( aName ) ;
    if ( pToolBoxEntry != m_aToolBoxMap.end() )
    {
        switch ( pToolBoxEntry->second )
        {
            case TB_ELEMENT_TOOLBAR:
            {
                if ( !m_bToolBarStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "End element 'toolbar' found, but no start element 'toolbar'" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarStartFound = sal_False;
            }
            break;

            case TB_ELEMENT_TOOLBARITEM:
            {
                if ( !m_bToolBarItemStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "End element 'toolbar:toolbaritem' found, but no start element 'toolbar:toolbaritem'" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarItemStartFound = sal_False;
            }
            break;

            case TB_ELEMENT_TOOLBARBREAK:
            {
                if ( !m_bToolBarBreakStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "End element 'toolbar:toolbarbreak' found, but no start element 'toolbar:toolbarbreak'" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarBreakStartFound = sal_False;
            }
            break;

            case TB_ELEMENT_TOOLBARSPACE:
            {
                if ( !m_bToolBarSpaceStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "End element 'toolbar:toolbarspace' found, but no start element 'toolbar:toolbarspace'" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarSpaceStartFound = sal_False;
            }
            break;

            case TB_ELEMENT_TOOLBARSEPARATOR:
            {
                if ( !m_bToolBarSeparatorStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "End element 'toolbar:toolbarseparator' found, but no start element 'toolbar:toolbarseparator'" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarSeparatorStartFound = sal_False;
            }
            break;
        }
    }
}

void SAL_CALL OReadToolBoxDocumentHandler::characters(const OUString& aChars)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadToolBoxDocumentHandler::ignorableWhitespace(const OUString& aWhitespaces)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadToolBoxDocumentHandler::processingInstruction(
    const OUString& aTarget, const OUString& aData )
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadToolBoxDocumentHandler::setDocumentLocator(
    const Reference< XLocator > &xLocator)
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    m_xLocator = xLocator;
}

::rtl::OUString OReadToolBoxDocumentHandler::getErrorLineString()
{
    ResetableGuard aGuard( m_aLock );

    char buffer[32];

    if ( m_xLocator.is() )
    {
        snprintf( buffer, sizeof(buffer), "Line: %ld - ", m_xLocator->getLineNumber() );
        return OUString::createFromAscii( buffer );
    }
    else
        return OUString();
}


//_________________________________________________________________________________________________________________
//  OWriteToolBoxDocumentHandler
//_________________________________________________________________________________________________________________

OWriteToolBoxDocumentHandler::OWriteToolBoxDocumentHandler(
    const ToolBoxDescriptor& aToolBoxItems,
    Reference< XDocumentHandler > rWriteDocumentHandler ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_aToolBoxItems( aToolBoxItems ),
    m_xWriteDocumentHandler( rWriteDocumentHandler )
{
    m_xEmptyList        = Reference< XAttributeList >( (XAttributeList *)new AttributeListImpl, UNO_QUERY );
    m_aAttributeType    = OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_TYPE_CDATA ));
    m_aXMLXlinkNS       = OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK_PREFIX ));
    m_aXMLToolbarNS     = OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_TOOLBAR_PREFIX ));
}

OWriteToolBoxDocumentHandler::~OWriteToolBoxDocumentHandler()
{
}

void OWriteToolBoxDocumentHandler::WriteToolBoxDocument() throw
( SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    m_xWriteDocumentHandler->startDocument();

    // write DOCTYPE line!
    Reference< XExtendedDocumentHandler > xExtendedDocHandler( m_xWriteDocumentHandler, UNO_QUERY );
    if ( xExtendedDocHandler.is() )
    {
        xExtendedDocHandler->unknown( OUString( RTL_CONSTASCII_USTRINGPARAM( TOOLBAR_DOCTYPE )) );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    }

    AttributeListImpl* pList = new AttributeListImpl;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XMLNS_TOOLBAR )),
                         m_aAttributeType,
                         OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_TOOLBAR )) );

    pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XMLNS_XLINK )),
                         m_aAttributeType,
                         OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK )) );

    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBAR )), pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    for ( int i = 0; i < m_aToolBoxItems.Count(); i++ )
    {
        ToolBoxItemDescriptor* pItem = m_aToolBoxItems[i];
        switch ( pItem->nItemType )
        {
            case TOOLBOXITEM_BUTTON:
            {
                WriteToolBoxItem( pItem );
            }
            break;

            case TOOLBOXITEM_SPACE:
            {
                WriteToolBoxSpace();
            }
            break;

            case TOOLBOXITEM_SEPARATOR:
            {
                WriteToolBoxSeparator();
            }
            break;

            case TOOLBOXITEM_BREAK:
            {
                WriteToolBoxBreak();
            }
            break;

            default:
            {
            }
        }
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBAR )) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endDocument();
}

//_________________________________________________________________________________________________________________
//  protected member functions
//_________________________________________________________________________________________________________________

void OWriteToolBoxDocumentHandler::WriteToolBoxItem( const ToolBoxItemDescriptor* pItem ) throw
( SAXException, RuntimeException )
{
    AttributeListImpl* pList = new AttributeListImpl;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    if ( m_aAttributeURL.getLength() == 0 )
    {
        m_aAttributeURL = m_aXMLXlinkNS;
        m_aAttributeURL += OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_URL ));
    }

    // save required attribute (URL)
    pList->addAttribute( m_aAttributeURL, m_aAttributeType, pItem->aURL );

    if ( pItem->aItemText.Len() > 0 )
    {
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_TEXT )),
                             m_aAttributeType,
                             pItem->aItemText );
    }

    if ( pItem->aBitmapName.Len() > 0 )
    {
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_BITMAP )),
                             m_aAttributeType,
                             pItem->aBitmapName );
    }

    if ( pItem->nItemBits != 0 )
    {
        sal_Bool        bBlankNeeded = sal_False;
        OUStringBuffer aItemStyleBuffer( 30 );

        if ( pItem->nItemBits & TIB_RADIOCHECK )
        {
            aItemStyleBuffer.appendAscii( ATTRIBUTE_ITEMSTYLE_RADIO );
            bBlankNeeded = sal_True;
        }

        if ( pItem->nItemBits & TIB_AUTOCHECK )
        {
            if ( bBlankNeeded )
                aItemStyleBuffer.appendAscii( " " );
            else
                bBlankNeeded = sal_True;
            aItemStyleBuffer.appendAscii( ATTRIBUTE_ITEMSTYLE_AUTO );
        }

        if ( pItem->nItemBits & TIB_LEFT )
        {
            if ( bBlankNeeded )
                aItemStyleBuffer.appendAscii( " " );
            else
                bBlankNeeded = sal_True;
            aItemStyleBuffer.appendAscii( ATTRIBUTE_ITEMSTYLE_LEFT );
        }

        if ( pItem->nItemBits & TIB_AUTOSIZE )
        {
            if ( bBlankNeeded )
                aItemStyleBuffer.appendAscii( " " );
            else
                bBlankNeeded = sal_True;
            aItemStyleBuffer.appendAscii( ATTRIBUTE_ITEMSTYLE_AUTOSIZE );
        }

        if ( pItem->nItemBits & TIB_DROPDOWN )
        {
            if ( bBlankNeeded )
                aItemStyleBuffer.appendAscii( " " );
            else
                bBlankNeeded = sal_True;
            aItemStyleBuffer.appendAscii( ATTRIBUTE_ITEMSTYLE_DROPDOWN );
        }

        if ( pItem->nItemBits & TIB_REPEAT )
        {
            if ( bBlankNeeded )
                aItemStyleBuffer.appendAscii( " " );
            else
                bBlankNeeded = sal_True;
            aItemStyleBuffer.appendAscii( ATTRIBUTE_ITEMSTYLE_REPEAT );
        }

        OUString aValue = aItemStyleBuffer.makeStringAndClear();
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ITEMSTYLE )),
                             m_aAttributeType,
                             aValue );
    }

    if ( pItem->nVisible == sal_False )
    {
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_VISIBLE )),
                             m_aAttributeType,
                             OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_BOOLEAN_FALSE )) );
    }

    if ( pItem->nWidth > 0 )
    {
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_WIDTH )),
                             m_aAttributeType,
                             OUString::valueOf( (sal_Int32)pItem->nWidth ) );
    }

    if ( pItem->nUserDef != 0 )
    {
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_USER )),
                             m_aAttributeType,
                             OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_BOOLEAN_TRUE )) );
    }

    if ( pItem->aHelpId.Len() > 0 )
    {
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_HELPID )),
                             m_aAttributeType,
                            pItem->aHelpId );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARITEM )), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARITEM )) );
}

void OWriteToolBoxDocumentHandler::WriteToolBoxSpace() throw
( SAXException, RuntimeException )
{
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARSPACE )), m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARSPACE )) );
}

void OWriteToolBoxDocumentHandler::WriteToolBoxBreak() throw
( SAXException, RuntimeException )
{
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARBREAK )), m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARBREAK )) );
}

void OWriteToolBoxDocumentHandler::WriteToolBoxSeparator() throw
( SAXException, RuntimeException )
{
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARSEPARATOR )), m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARSEPARATOR )) );
}

} // namespace framework
