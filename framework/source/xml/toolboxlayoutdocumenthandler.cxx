/*************************************************************************
 *
 *  $RCSfile: toolboxlayoutdocumenthandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-02-25 17:56:07 $
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
#include <xml/toolboxlayoutdocumenthandler.hxx>
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

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;


#define TOOLBARLAYOUTS_DOCTYPE  "<!DOCTYPE toolbar:toolbarlayouts PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"toolbar.dtd\">"

namespace framework
{

struct ToolBarLayoutEntryProperty
{
    OReadToolBoxLayoutDocumentHandler::ToolBox_XML_Namespace    nNamespace;
    char                                                        aEntryName[20];
};

ToolBarLayoutEntryProperty ToolBoxLayoutEntries[OReadToolBoxLayoutDocumentHandler::TBL_XML_ENTRY_COUNT] =
{
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ELEMENT_TOOLBARLAYOUTS      },
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ELEMENT_TOOLBARLAYOUT       },
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ELEMENT_TOOLBARCONFIGITEMS  },
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ELEMENT_TOOLBARCONFIGITEM   },
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ATTRIBUTE_ID                },
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ATTRIBUTE_TOOLBARNAME       },
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ATTRIBUTE_CONTEXT           },
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ATTRIBUTE_FLOATINGLINES     },
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ATTRIBUTE_DOCKINGLINES      },
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ATTRIBUTE_ALIGN             },
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ATTRIBUTE_FLOATING          },
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ATTRIBUTE_FLOATINGPOSLEFT   },
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ATTRIBUTE_FLOATINGPOSTOP    },
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ATTRIBUTE_VISIBLE           },
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ATTRIBUTE_BUTTONTYPE        },
    { OReadToolBoxLayoutDocumentHandler::TBL_NS_TOOLBAR,    ATTRIBUTE_USERDEFNAME       }
};


OReadToolBoxLayoutDocumentHandler::OReadToolBoxLayoutDocumentHandler( ToolBoxLayoutDescriptor& aToolBoxItems ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    ::cppu::OWeakObject(),
    m_aToolBoxItems( aToolBoxItems )
{
    OUString aNamespaceToolBar( RTL_CONSTASCII_USTRINGPARAM( XMLNS_TOOLBAR ));
    OUString aSeparator( RTL_CONSTASCII_USTRINGPARAM( XMLNS_FILTER_SEPARATOR ));

    // create hash map
    for ( int i = 0; i <= (int)TBL_XML_ENTRY_COUNT; i++ )
    {
        OUString temp( aNamespaceToolBar );
        temp += aSeparator;
        temp += OUString::createFromAscii( ToolBoxLayoutEntries[i].aEntryName );
        m_aToolBoxMap.insert( ToolBoxLayoutHashMap::value_type( temp, (ToolBoxLayout_XML_Entry)i ) );
    }

    m_bToolBarLayoutsStartFound     = sal_False;
    m_bToolBarLayoutsEndFound       = sal_False;
    m_bToolBarLayoutStartFound      = sal_False;
}

OReadToolBoxLayoutDocumentHandler::~OReadToolBoxLayoutDocumentHandler()
{
}

Any SAL_CALL OReadToolBoxLayoutDocumentHandler::queryInterface( const Type & rType )
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
void SAL_CALL OReadToolBoxLayoutDocumentHandler::startDocument(void)
throw ( SAXException, RuntimeException )
{
}

void SAL_CALL OReadToolBoxLayoutDocumentHandler::endDocument(void)
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if (( m_bToolBarLayoutsEndFound && !m_bToolBarLayoutsEndFound ) ||
        ( !m_bToolBarLayoutsEndFound && m_bToolBarLayoutsEndFound )     )
    {
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "No matching start or end element 'toolbar:toolbarlayouts' found!" ));
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}

void SAL_CALL OReadToolBoxLayoutDocumentHandler::startElement(
    const OUString& aName, const Reference< XAttributeList > &xAttribs )
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    ToolBoxLayoutHashMap::const_iterator pToolBoxEntry = m_aToolBoxMap.find( aName ) ;
    if ( pToolBoxEntry != m_aToolBoxMap.end() )
    {
        switch ( pToolBoxEntry->second )
        {
            case TBL_ELEMENT_TOOLBARLAYOUTS:
            {
                // toolbar:toolbarlayouts element (container) for toolbar:toolbarlayout
                if ( m_bToolBarLayoutsStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'toolbar:toolbarlayouts' cannot be embeded into 'toolbar:toolbarlayouts'!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarLayoutsStartFound = sal_True;
            }
            break;

            case TBL_ELEMENT_TOOLBARLAYOUT:
            {
                if ( !m_bToolBarLayoutsStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'toolbar:toolbarlayout' must be embeded into element 'toolbar:toolbarlayouts'!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarLayoutStartFound = sal_True;
                ToolBoxLayoutItemDescriptor* pItem = new ToolBoxLayoutItemDescriptor;
                m_aToolBoxItems.Insert( pItem, m_aToolBoxItems.Count() );

                for ( int n = 0; n < xAttribs->getLength(); n++ )
                {
                    pToolBoxEntry = m_aToolBoxMap.find( xAttribs->getNameByIndex( n ) );
                    if ( pToolBoxEntry != m_aToolBoxMap.end() )
                    {
                        switch ( pToolBoxEntry->second )
                        {
                            case TBL_ATTRIBUTE_ID:
                            {
                                pItem->aName = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case TBL_ATTRIBUTE_FLOATINGPOSLEFT:
                            {
                                pItem->aFloatingPos.X() = xAttribs->getValueByIndex( n ).toInt32();
                            }
                            break;

                            case TBL_ATTRIBUTE_FLOATINGPOSTOP:
                            {
                                pItem->aFloatingPos.Y() = xAttribs->getValueByIndex( n ).toInt32();
                            }
                            break;

                            case TBL_ATTRIBUTE_FLOATINGLINES:
                            {
                                pItem->nFloatingLines = (USHORT)(xAttribs->getValueByIndex( n ).toInt32());
                            }
                            break;

                            case TBL_ATTRIBUTE_VISIBLE:
                            {
                                pItem->bVisible = sal_True;
                                if ( xAttribs->getValueByIndex( n ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_BOOLEAN_TRUE )) )
                                    pItem->bVisible = sal_True;
                                else if ( xAttribs->getValueByIndex( n ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_BOOLEAN_FALSE )) )
                                    pItem->bVisible = sal_False;
                                else
                                {
                                    OUString aErrorMessage = getErrorLineString();
                                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Attribute toolbar:visible must have value 'true' or 'false'!" ));
                                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                                }
                            }
                            break;

                            case TBL_ATTRIBUTE_DOCKINGLINES:
                            {
                                pItem->nLines = (USHORT)(xAttribs->getValueByIndex( n ).toInt32());
                            }
                            break;

                            case TBL_ATTRIBUTE_ALIGN:
                            {
                                if ( xAttribs->getValueByIndex( n ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_ALIGN_LEFT )) )
                                    pItem->eAlign = WINDOWALIGN_LEFT;
                                else if ( xAttribs->getValueByIndex( n ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_ALIGN_RIGHT )) )
                                    pItem->eAlign = WINDOWALIGN_RIGHT;
                                else if ( xAttribs->getValueByIndex( n ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_ALIGN_TOP )) )
                                    pItem->eAlign = WINDOWALIGN_TOP;
                                else if ( xAttribs->getValueByIndex( n ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_ALIGN_BOTTOM )) )
                                    pItem->eAlign = WINDOWALIGN_BOTTOM;
                                else
                                {
                                    OUString aErrorMessage = getErrorLineString();
                                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Attribute toolbar:align must have one value of 'left','right','top','bottom'" ));
                                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                                }
                            }
                            break;

                            case TBL_ATTRIBUTE_FLOATING:
                            {
                                if ( xAttribs->getValueByIndex( n ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_BOOLEAN_TRUE )) )
                                    pItem->bFloating = sal_True;
                                else if ( xAttribs->getValueByIndex( n ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_BOOLEAN_FALSE )) )
                                    pItem->bFloating = sal_False;
                                else
                                {
                                    OUString aErrorMessage = getErrorLineString();
                                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Attribute toolbar:floating must have value 'true' or 'false'!" ));
                                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                                }
                            }
                            break;

                            case TBL_ATTRIBUTE_STYLE:
                            {
                                if ( xAttribs->getValueByIndex( n ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_STYLE_TEXT )) )
                                    pItem->eType = BUTTON_TEXT;
                                else if ( xAttribs->getValueByIndex( n ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_STYLE_SYMBOLTEXT )) )
                                    pItem->eType = BUTTON_SYMBOLTEXT;
                                else if ( xAttribs->getValueByIndex( n ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_STYLE_SYMBOL )) )
                                    pItem->eType = BUTTON_SYMBOL;
                                else if ( xAttribs->getValueByIndex( n ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_STYLE_SYMBOLPLUSTEXT )) )
                                    pItem->eType = BUTTON_SYMBOL;
                                else
                                {
                                    OUString aErrorMessage = getErrorLineString();
                                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Attribute toolbar:style must have one of the following values 'text', 'symbol' or 'symbol+text'!" ));
                                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                                }
                            }
                            break;

                            case TBL_ATTRIBUTE_USERDEFNAME:
                            {
                                pItem->aUserName = xAttribs->getValueByIndex( n );
                            }
                            break;
                        }
                    }
                } // for

                if ( pItem->aName.Len() == 0 )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Required attribute toolbar:id must have a value!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }
            }
            break;
        }
    }
}

void SAL_CALL OReadToolBoxLayoutDocumentHandler::endElement(const OUString& aName)
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    ToolBoxLayoutHashMap::const_iterator pToolBoxEntry = m_aToolBoxMap.find( aName ) ;
    if ( pToolBoxEntry != m_aToolBoxMap.end() )
    {
        switch ( pToolBoxEntry->second )
        {
            case TBL_ELEMENT_TOOLBARLAYOUTS:
            {
                if ( !m_bToolBarLayoutsStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "End element 'toolbar:toolbarlayouts' found, but not start element" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarLayoutsEndFound = sal_True;
            }
            break;

            case TBL_ELEMENT_TOOLBARLAYOUT:
            {
                if ( !m_bToolBarLayoutStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "End element 'toolbar:toolbarlayout' found, but not start element" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarLayoutStartFound = sal_False;
            }
            break;
        }
    }
}

void SAL_CALL OReadToolBoxLayoutDocumentHandler::characters(const OUString& aChars)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadToolBoxLayoutDocumentHandler::ignorableWhitespace(const OUString& aWhitespaces)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadToolBoxLayoutDocumentHandler::processingInstruction(
    const OUString& aTarget, const OUString& aData )
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadToolBoxLayoutDocumentHandler::setDocumentLocator(
    const Reference< XLocator > &xLocator)
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    m_xLocator = xLocator;
}

::rtl::OUString OReadToolBoxLayoutDocumentHandler::getErrorLineString()
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
//  OWriteToolBoxLayoutDocumentHandler
//_________________________________________________________________________________________________________________

OWriteToolBoxLayoutDocumentHandler::OWriteToolBoxLayoutDocumentHandler(
    const ToolBoxLayoutDescriptor& aToolBoxLayoutItems,
    Reference< XDocumentHandler > rWriteDocumentHandler ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_aToolBoxLayoutItems( aToolBoxLayoutItems ),
    m_xWriteDocumentHandler( rWriteDocumentHandler )
{
    m_xEmptyList        = Reference< XAttributeList >( (XAttributeList *)new AttributeListImpl, UNO_QUERY );
    m_aAttributeType    = OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_TYPE_CDATA ));
    m_aXMLToolbarNS     = OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_TOOLBAR_PREFIX ));
    m_aTrueValue        = OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_BOOLEAN_TRUE ));
    m_aFalseValue       = OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_BOOLEAN_FALSE ));
}

OWriteToolBoxLayoutDocumentHandler::~OWriteToolBoxLayoutDocumentHandler()
{
}

void OWriteToolBoxLayoutDocumentHandler::WriteToolBoxLayoutDocument() throw
( SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    m_xWriteDocumentHandler->startDocument();

    // write DOCTYPE line!
    Reference< XExtendedDocumentHandler > xExtendedDocHandler( m_xWriteDocumentHandler, UNO_QUERY );
    if ( xExtendedDocHandler.is() )
    {
        xExtendedDocHandler->unknown( OUString( RTL_CONSTASCII_USTRINGPARAM( TOOLBARLAYOUTS_DOCTYPE )) );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    }

    AttributeListImpl* pList = new AttributeListImpl;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XMLNS_TOOLBAR )),
                         m_aAttributeType,
                         OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_TOOLBAR )) );

    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARLAYOUTS )), pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    for ( int i = 0; i < m_aToolBoxLayoutItems.Count(); i++ )
    {
        ToolBoxLayoutItemDescriptor* pItem = m_aToolBoxLayoutItems[i];
        WriteToolBoxLayoutItem( pItem );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARLAYOUTS )) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endDocument();
}

//_________________________________________________________________________________________________________________
//  protected member functions
//_________________________________________________________________________________________________________________

void OWriteToolBoxLayoutDocumentHandler::WriteToolBoxLayoutItem( const ToolBoxLayoutItemDescriptor* pItem ) throw
( SAXException, RuntimeException )
{
    Point                       aPoint;
    AttributeListImpl*          pList = new AttributeListImpl;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    // save required attribute ()
    pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ID )),
                         m_aAttributeType,
                         pItem->aName );

    if ( pItem->aFloatingPos != aPoint )
    {
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_FLOATINGPOSLEFT )),
                             m_aAttributeType,
                             OUString::valueOf( (sal_Int32)pItem->aFloatingPos.X() ));

        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_FLOATINGPOSTOP )),
                             m_aAttributeType,
                             OUString::valueOf( (sal_Int32)pItem->aFloatingPos.Y() ));
    }

    if ( pItem->nFloatingLines > 0 )
    {
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_FLOATINGLINES )),
                             m_aAttributeType,
                             OUString::valueOf( (sal_Int32)pItem->nFloatingLines ));
    }

    if ( pItem->nLines > 1 )
    {
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_DOCKINGLINES )),
                             m_aAttributeType,
                             OUString::valueOf( (sal_Int32)pItem->nLines ) );
    }

    if ( pItem->eAlign == WINDOWALIGN_TOP )
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ALIGN )),
                                m_aAttributeType,
                                OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ALIGN_TOP )) );
    else if ( pItem->eAlign == WINDOWALIGN_BOTTOM )
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ALIGN )),
                                m_aAttributeType,
                                OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ALIGN_BOTTOM )) );
    else if ( pItem->eAlign == WINDOWALIGN_LEFT )
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ALIGN )),
                                m_aAttributeType,
                                OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ALIGN_LEFT )) );
    else if ( pItem->eAlign == WINDOWALIGN_RIGHT )
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ALIGN )),
                                m_aAttributeType,
                                OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ALIGN_RIGHT )) );

    if ( pItem->bVisible )
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_VISIBLE )),
                             m_aAttributeType,
                             m_aTrueValue );
    else
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_VISIBLE )),
                             m_aAttributeType,
                             m_aFalseValue );

    if ( pItem->bFloating )
    {
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_FLOATING )),
                             m_aAttributeType,
                             m_aTrueValue );
    }

    if ( pItem->eType == BUTTON_TEXT )
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_BUTTONTYPE )),
                             m_aAttributeType,
                             OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_STYLE_TEXT )) );
    else if ( pItem->eType == BUTTON_SYMBOLTEXT )
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_BUTTONTYPE )),
                             m_aAttributeType,
                             OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_STYLE_SYMBOLTEXT )) );
    else
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_BUTTONTYPE )),
                             m_aAttributeType,
                             OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_STYLE_SYMBOL )) );

    if ( pItem->aUserName.Len() )
    {
        pList->addAttribute( m_aXMLToolbarNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_USERDEFNAME )),
                             m_aAttributeType,
                             pItem->aUserName );
    }

    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARLAYOUT )), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARLAYOUT )) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
}

} // namespace framework
