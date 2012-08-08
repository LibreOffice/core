/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <stdio.h>

#include <threadhelp/resetableguard.hxx>
#include <xml/toolboxdocumenthandler.hxx>
#include <macros/debug.hxx>
#include <xml/toolboxconfigurationdefines.hxx>

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <sal/config.h>
#include <sal/macros.h>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <rtl/ustrbuf.hxx>

#include <comphelper/attributelist.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::xml::sax;


#define TOOLBAR_DOCTYPE             "<!DOCTYPE toolbar:toolbar PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"toolbar.dtd\">"

namespace framework
{

// Property names of a menu/menu item ItemDescriptor
static const char ITEM_DESCRIPTOR_COMMANDURL[]  = "CommandURL";
static const char ITEM_DESCRIPTOR_HELPURL[]     = "HelpURL";
static const char ITEM_DESCRIPTOR_TOOLTIP[]     = "Tooltip";
static const char ITEM_DESCRIPTOR_LABEL[]       = "Label";
static const char ITEM_DESCRIPTOR_TYPE[]        = "Type";
static const char ITEM_DESCRIPTOR_STYLE[]       = "Style";
static const char ITEM_DESCRIPTOR_VISIBLE[]     = "IsVisible";
static const char ITEM_DESCRIPTOR_WIDTH[]       = "Width";

static void ExtractToolbarParameters( const Sequence< PropertyValue > rProp,
                                      ::rtl::OUString&                       rCommandURL,
                                      ::rtl::OUString&                       rLabel,
                                      ::rtl::OUString&                       rHelpURL,
                                      ::rtl::OUString&                       rTooltip,
                                      sal_Int16&                      rStyle,
                                      sal_Int16&                      rWidth,
                                      sal_Bool&                       rVisible,
                                      sal_Int16&                      rType )
{
    for ( sal_Int32 i = 0; i < rProp.getLength(); i++ )
    {
        if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_COMMANDURL ))
        {
            rProp[i].Value >>= rCommandURL;
            rCommandURL = rCommandURL.intern();
        }
        else if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_HELPURL ))
            rProp[i].Value >>= rHelpURL;
        else if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_TOOLTIP ))
            rProp[i].Value >>= rTooltip;
        else if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_LABEL ))
            rProp[i].Value >>= rLabel;
        else if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_TYPE ))
            rProp[i].Value >>= rType;
        else if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_VISIBLE ))
            rProp[i].Value >>= rVisible;
        else if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_WIDTH ))
            rProp[i].Value >>= rWidth;
        else if ( rProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_STYLE ))
            rProp[i].Value >>= rStyle;
    }
}

struct ToolboxStyleItem
{
    sal_Int16 nBit;
    const char* attrName;
};

ToolboxStyleItem Styles[ ] = {
    { ::com::sun::star::ui::ItemStyle::RADIO_CHECK, ATTRIBUTE_ITEMSTYLE_RADIO },
    { ::com::sun::star::ui::ItemStyle::ALIGN_LEFT, ATTRIBUTE_ITEMSTYLE_LEFT },
    { ::com::sun::star::ui::ItemStyle::AUTO_SIZE, ATTRIBUTE_ITEMSTYLE_AUTO },
    { ::com::sun::star::ui::ItemStyle::REPEAT, ATTRIBUTE_ITEMSTYLE_REPEAT },
    { ::com::sun::star::ui::ItemStyle::DROPDOWN_ONLY, ATTRIBUTE_ITEMSTYLE_DROPDOWNONLY },
    { ::com::sun::star::ui::ItemStyle::DROP_DOWN, ATTRIBUTE_ITEMSTYLE_DROPDOWN },
    { ::com::sun::star::ui::ItemStyle::ICON, ATTRIBUTE_ITEMSTYLE_IMAGE },
    { ::com::sun::star::ui::ItemStyle::TEXT, ATTRIBUTE_ITEMSTYLE_TEXT },
};

sal_Int32 nStyleItemEntries = sizeof (Styles) / sizeof (Styles[0]);

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
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ATTRIBUTE_ITEMSTYLE         },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ATTRIBUTE_UINAME            },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ATTRIBUTE_TOOLTIP           },
};

OReadToolBoxDocumentHandler::OReadToolBoxDocumentHandler( const Reference< XIndexContainer >& rItemContainer ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_rItemContainer( rItemContainer ),
    m_aType( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_TYPE )),
    m_aLabel( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_LABEL )),
    m_aStyle( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_STYLE )),
    m_aHelpURL( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_HELPURL )),
    m_aTooltip( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_TOOLTIP )),
    m_aIsVisible( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_VISIBLE )),
    m_aCommandURL( RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_COMMANDURL ))
 {
    ::rtl::OUString aNamespaceToolBar( RTL_CONSTASCII_USTRINGPARAM( XMLNS_TOOLBAR ));
    ::rtl::OUString aNamespaceXLink( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK ));
    ::rtl::OUString aSeparator( RTL_CONSTASCII_USTRINGPARAM( XMLNS_FILTER_SEPARATOR ));

    // create hash map
    for ( int i = 0; i < (int)TB_XML_ENTRY_COUNT; i++ )
    {
        if ( ToolBoxEntries[i].nNamespace == TB_NS_TOOLBAR )
        {
            ::rtl::OUString temp( aNamespaceToolBar );
            temp += aSeparator;
            temp += ::rtl::OUString::createFromAscii( ToolBoxEntries[i].aEntryName );
            m_aToolBoxMap.insert( ToolBoxHashMap::value_type( temp, (ToolBox_XML_Entry)i ) );
        }
        else
        {
            ::rtl::OUString temp( aNamespaceXLink );
            temp += aSeparator;
            temp += ::rtl::OUString::createFromAscii( ToolBoxEntries[i].aEntryName );
            m_aToolBoxMap.insert( ToolBoxHashMap::value_type( temp, (ToolBox_XML_Entry)i ) );
        }
    }

    // pre-calculate a hash code for all style strings to speed up xml read process
    m_nHashCode_Style_Radio         = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ITEMSTYLE_RADIO )).hashCode();
    m_nHashCode_Style_Auto          = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ITEMSTYLE_AUTO )).hashCode();
    m_nHashCode_Style_Left          = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ITEMSTYLE_LEFT )).hashCode();
    m_nHashCode_Style_AutoSize      = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ITEMSTYLE_AUTOSIZE )).hashCode();
    m_nHashCode_Style_DropDown      = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ITEMSTYLE_DROPDOWN )).hashCode();
    m_nHashCode_Style_Repeat        = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ITEMSTYLE_REPEAT )).hashCode();
    m_nHashCode_Style_DropDownOnly  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ITEMSTYLE_DROPDOWNONLY )).hashCode();
    m_nHashCode_Style_Text  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ITEMSTYLE_TEXT )).hashCode();
    m_nHashCode_Style_Image  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ITEMSTYLE_IMAGE )).hashCode();

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
        ::rtl::OUString aErrorMessage = getErrorLineString();
        aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "No matching start or end element 'toolbar' found!" ));
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}

void SAL_CALL OReadToolBoxDocumentHandler::startElement(
    const ::rtl::OUString& aName, const Reference< XAttributeList > &xAttribs )
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
                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'toolbar:toolbar' cannot be embeded into 'toolbar:toolbar'!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }
                        else
                        {
                            // Check if we have a UI name set in our XML file
                            ::rtl::OUString aUIName;
                            for ( sal_Int16 n = 0; n < xAttribs->getLength(); n++ )
                      {
                        pToolBoxEntry = m_aToolBoxMap.find( xAttribs->getNameByIndex( n ) );
                        if ( pToolBoxEntry != m_aToolBoxMap.end() )
                        {
                                    switch ( pToolBoxEntry->second )
                                    {
                                        case TB_ATTRIBUTE_UINAME:
                                    aUIName = xAttribs->getValueByIndex( n );
                                            break;
                                        default:
                                            break;
                                    }
                                }
                            }

                            if ( !aUIName.isEmpty() )
                            {
                                // Try to set UI name as a container property
                                Reference< XPropertySet > xPropSet( m_rItemContainer, UNO_QUERY );
                                if ( xPropSet.is() )
                                {
                                    try
                                    {
                                        xPropSet->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UIName" )), makeAny( aUIName ) );
                                    }
                                    catch ( const UnknownPropertyException& )
                                    {
                                    }
                                }
                            }
                        }

                m_bToolBarStartFound = sal_True;
            }
            break;

            case TB_ELEMENT_TOOLBARITEM:
            {
                if ( !m_bToolBarStartFound )
                {
                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'toolbar:toolbaritem' must be embeded into element 'toolbar:toolbar'!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_bToolBarSeparatorStartFound ||
                     m_bToolBarBreakStartFound ||
                     m_bToolBarSpaceStartFound ||
                     m_bToolBarItemStartFound )
                {
                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Element toolbar:toolbaritem is not a container!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                sal_Bool bAttributeURL  = sal_False;

                m_bToolBarItemStartFound = sal_True;
                ::rtl::OUString        aLabel;
                ::rtl::OUString        aCommandURL;
                ::rtl::OUString        aHelpURL;
                ::rtl::OUString        aTooltip;
                ::rtl::OUString        aBitmapName;
                sal_uInt16      nItemBits( 0 );
                sal_Bool        bVisible( sal_True );

                for ( sal_Int16 n = 0; n < xAttribs->getLength(); n++ )
                {
                    pToolBoxEntry = m_aToolBoxMap.find( xAttribs->getNameByIndex( n ) );
                    if ( pToolBoxEntry != m_aToolBoxMap.end() )
                    {
                        switch ( pToolBoxEntry->second )
                        {
                            case TB_ATTRIBUTE_TEXT:
                            {
                                aLabel = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case TB_ATTRIBUTE_BITMAP:
                            {
                                aBitmapName = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case TB_ATTRIBUTE_URL:
                            {
                                bAttributeURL   = sal_True;
                                aCommandURL     = xAttribs->getValueByIndex( n ).intern();
                            }
                            break;

                            case TB_ATTRIBUTE_ITEMBITS:
                            {
                                nItemBits = (sal_uInt16)(xAttribs->getValueByIndex( n ).toInt32());
                            }
                            break;

                            case TB_ATTRIBUTE_VISIBLE:
                            {
                                if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_BOOLEAN_TRUE )
                                    bVisible = sal_True;
                                else if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_BOOLEAN_FALSE )
                                    bVisible = sal_False;
                                else
                                {
                                    ::rtl::OUString aErrorMessage = getErrorLineString();
                                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Attribute toolbar:visible must have value 'true' or 'false'!" ));
                                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                                }
                            }
                            break;

                            case TB_ATTRIBUTE_HELPID:
                            {
                                aHelpURL = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case TB_ATTRIBUTE_TOOLTIP:
                            {
                                aTooltip = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case TB_ATTRIBUTE_STYLE:
                            {
                                // read space seperated item style list
                                ::rtl::OUString aTemp = xAttribs->getValueByIndex( n );
                                sal_Int32 nIndex = 0;

                                do
                                {
                                    ::rtl::OUString aToken  = aTemp.getToken( 0, ' ', nIndex );
                                    if ( !aToken.isEmpty() )
                                    {
                                        sal_Int32 nHashCode = aToken.hashCode();
                                        if ( nHashCode == m_nHashCode_Style_Radio )
                                            nItemBits |= ::com::sun::star::ui::ItemStyle::RADIO_CHECK;
                                        else if ( nHashCode == m_nHashCode_Style_Left )
                                            nItemBits |= ::com::sun::star::ui::ItemStyle::ALIGN_LEFT;
                                        else if ( nHashCode == m_nHashCode_Style_AutoSize )
                                            nItemBits |= ::com::sun::star::ui::ItemStyle::AUTO_SIZE;
                                        else if ( nHashCode == m_nHashCode_Style_Repeat )
                                            nItemBits |= ::com::sun::star::ui::ItemStyle::REPEAT;
                                        else if ( nHashCode == m_nHashCode_Style_DropDownOnly )
                                            nItemBits |= ::com::sun::star::ui::ItemStyle::DROPDOWN_ONLY;
                                        else if ( nHashCode == m_nHashCode_Style_DropDown )
                                            nItemBits |= ::com::sun::star::ui::ItemStyle::DROP_DOWN;
                                        else if ( nHashCode == m_nHashCode_Style_Text )
                                            nItemBits |= ::com::sun::star::ui::ItemStyle::TEXT;
                                        else if ( nHashCode == m_nHashCode_Style_Image )
                                            nItemBits |= ::com::sun::star::ui::ItemStyle::ICON;
                                    }
                                }
                                while ( nIndex >= 0 );
                            }
                            break;
                            case TB_ATTRIBUTE_USER:
                            case TB_ATTRIBUTE_WIDTH:
                            default:
                            break;
                        }
                    }
                } // for

                if ( !bAttributeURL )
                {
                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Required attribute toolbar:url must have a value!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( !aCommandURL.isEmpty() )
                {
                    Sequence< PropertyValue > aToolbarItemProp( 7 );
                    aToolbarItemProp[0].Name = m_aCommandURL;
                    aToolbarItemProp[1].Name = m_aHelpURL;
                    aToolbarItemProp[2].Name = m_aLabel;
                    aToolbarItemProp[3].Name = m_aType;
                    aToolbarItemProp[4].Name = m_aStyle;
                    aToolbarItemProp[5].Name = m_aIsVisible;
                    aToolbarItemProp[6].Name = m_aTooltip;

                    aToolbarItemProp[0].Value <<= aCommandURL;
                    aToolbarItemProp[1].Value <<= aHelpURL;
                    aToolbarItemProp[2].Value <<= aLabel;
                    aToolbarItemProp[3].Value = makeAny( ::com::sun::star::ui::ItemType::DEFAULT );
                    aToolbarItemProp[4].Value <<= nItemBits;
                    aToolbarItemProp[5].Value <<= bVisible;
                    aToolbarItemProp[6].Value <<= aTooltip;

                    m_rItemContainer->insertByIndex( m_rItemContainer->getCount(), makeAny( aToolbarItemProp ) );
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
                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Element toolbar:toolbarspace is not a container!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarSpaceStartFound = sal_True;

                Sequence< PropertyValue > aToolbarItemProp( 2 );
                aToolbarItemProp[0].Name = m_aCommandURL;
                aToolbarItemProp[1].Name = m_aType;

                aToolbarItemProp[0].Value <<= rtl::OUString();
                aToolbarItemProp[1].Value <<= ::com::sun::star::ui::ItemType::SEPARATOR_SPACE;

                m_rItemContainer->insertByIndex( m_rItemContainer->getCount(), makeAny( aToolbarItemProp ) );
            }
            break;

            case TB_ELEMENT_TOOLBARBREAK:
            {
                if ( m_bToolBarSeparatorStartFound ||
                     m_bToolBarBreakStartFound ||
                     m_bToolBarSpaceStartFound ||
                     m_bToolBarItemStartFound )
                {
                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Element toolbar:toolbarbreak is not a container!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarBreakStartFound = sal_True;

                Sequence< PropertyValue > aToolbarItemProp( 2 );
                aToolbarItemProp[0].Name = m_aCommandURL;
                aToolbarItemProp[1].Name = m_aType;

                aToolbarItemProp[0].Value <<= rtl::OUString();
                aToolbarItemProp[1].Value <<= ::com::sun::star::ui::ItemType::SEPARATOR_LINEBREAK;

                m_rItemContainer->insertByIndex( m_rItemContainer->getCount(), makeAny( aToolbarItemProp ) );
            }
            break;

            case TB_ELEMENT_TOOLBARSEPARATOR:
            {
                if ( m_bToolBarSeparatorStartFound ||
                     m_bToolBarBreakStartFound ||
                     m_bToolBarSpaceStartFound ||
                     m_bToolBarItemStartFound )
                {
                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Element toolbar:toolbarseparator is not a container!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarSeparatorStartFound = sal_True;

                Sequence< PropertyValue > aToolbarItemProp( 2 );
                aToolbarItemProp[0].Name = m_aCommandURL;
                aToolbarItemProp[1].Name = m_aType;

                aToolbarItemProp[0].Value <<= rtl::OUString();
                aToolbarItemProp[1].Value <<= ::com::sun::star::ui::ItemType::SEPARATOR_LINE;

                m_rItemContainer->insertByIndex( m_rItemContainer->getCount(), makeAny( aToolbarItemProp ) );
            }
            break;

                  default:
                      break;
        }
    }
}

void SAL_CALL OReadToolBoxDocumentHandler::endElement(const ::rtl::OUString& aName)
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
                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "End element 'toolbar' found, but no start element 'toolbar'" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarStartFound = sal_False;
            }
            break;

            case TB_ELEMENT_TOOLBARITEM:
            {
                if ( !m_bToolBarItemStartFound )
                {
                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "End element 'toolbar:toolbaritem' found, but no start element 'toolbar:toolbaritem'" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarItemStartFound = sal_False;
            }
            break;

            case TB_ELEMENT_TOOLBARBREAK:
            {
                if ( !m_bToolBarBreakStartFound )
                {
                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "End element 'toolbar:toolbarbreak' found, but no start element 'toolbar:toolbarbreak'" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarBreakStartFound = sal_False;
            }
            break;

            case TB_ELEMENT_TOOLBARSPACE:
            {
                if ( !m_bToolBarSpaceStartFound )
                {
                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "End element 'toolbar:toolbarspace' found, but no start element 'toolbar:toolbarspace'" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarSpaceStartFound = sal_False;
            }
            break;

            case TB_ELEMENT_TOOLBARSEPARATOR:
            {
                if ( !m_bToolBarSeparatorStartFound )
                {
                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "End element 'toolbar:toolbarseparator' found, but no start element 'toolbar:toolbarseparator'" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarSeparatorStartFound = sal_False;
            }
            break;

                  default:
                      break;
        }
    }
}

void SAL_CALL OReadToolBoxDocumentHandler::characters(const ::rtl::OUString&)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadToolBoxDocumentHandler::ignorableWhitespace(const ::rtl::OUString&)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadToolBoxDocumentHandler::processingInstruction(
    const ::rtl::OUString& /*aTarget*/, const ::rtl::OUString& /*aData*/ )
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
        snprintf( buffer, sizeof(buffer), "Line: %ld - ", static_cast<long>( m_xLocator->getLineNumber() ));
        return ::rtl::OUString::createFromAscii( buffer );
    }
    else
        return ::rtl::OUString();
}


//_________________________________________________________________________________________________________________
//  OWriteToolBoxDocumentHandler
//_________________________________________________________________________________________________________________

OWriteToolBoxDocumentHandler::OWriteToolBoxDocumentHandler(
    const Reference< XIndexAccess >& rItemAccess,
    Reference< XDocumentHandler >& rWriteDocumentHandler ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_xWriteDocumentHandler( rWriteDocumentHandler ),
    m_rItemAccess( rItemAccess )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    m_xEmptyList        = Reference< XAttributeList >( (XAttributeList *) pList, UNO_QUERY );
    m_aAttributeType    = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_TYPE_CDATA ));
    m_aXMLXlinkNS       = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK_PREFIX ));
    m_aXMLToolbarNS     = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_TOOLBAR_PREFIX ));
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
        xExtendedDocHandler->unknown( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( TOOLBAR_DOCTYPE )) );
        m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
    }

    ::rtl::OUString aUIName;
    Reference< XPropertySet > xPropSet( m_rItemAccess, UNO_QUERY );
    if ( xPropSet.is() )
    {
        try
        {
            xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UIName" ))) >>= aUIName;
        }
        catch ( const UnknownPropertyException& )
        {
        }
    }

    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    pList->AddAttribute( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XMLNS_TOOLBAR )),
                         m_aAttributeType,
                         ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_TOOLBAR )) );

    pList->AddAttribute( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XMLNS_XLINK )),
                         m_aAttributeType,
                         ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK )) );

    if ( !aUIName.isEmpty() )
        pList->AddAttribute( m_aXMLToolbarNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_UINAME )),
                             m_aAttributeType,
                             aUIName );

    m_xWriteDocumentHandler->startElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBAR )), pList );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );

    sal_Int32  nItemCount = m_rItemAccess->getCount();
    Any        aAny;

    for ( sal_Int32 nItemPos = 0; nItemPos < nItemCount; nItemPos++ )
    {
        Sequence< PropertyValue > aProps;
        aAny = m_rItemAccess->getByIndex( nItemPos );
        if ( aAny >>= aProps )
        {
            ::rtl::OUString    aCommandURL;
            ::rtl::OUString    aLabel;
            ::rtl::OUString    aHelpURL;
            ::rtl::OUString    aTooltip;
            sal_Bool    bVisible( sal_True );
            sal_Int16   nType( ::com::sun::star::ui::ItemType::DEFAULT );
            sal_Int16   nWidth( 0 );
            sal_Int16   nStyle( 0 );

            ExtractToolbarParameters( aProps, aCommandURL, aLabel, aHelpURL, aTooltip, nStyle, nWidth, bVisible, nType );
            if ( nType == ::com::sun::star::ui::ItemType::DEFAULT )
                WriteToolBoxItem( aCommandURL, aLabel, aHelpURL, aTooltip, nStyle, nWidth, bVisible );
            else if ( nType == ::com::sun::star::ui::ItemType::SEPARATOR_SPACE )
                WriteToolBoxSpace();
            else if ( nType == ::com::sun::star::ui::ItemType::SEPARATOR_LINE )
                WriteToolBoxSeparator();
            else if ( nType == ::com::sun::star::ui::ItemType::SEPARATOR_LINEBREAK )
                WriteToolBoxBreak();
        }
    }

    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
    m_xWriteDocumentHandler->endElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBAR )) );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
    m_xWriteDocumentHandler->endDocument();
}

//_________________________________________________________________________________________________________________
//  protected member functions
//_________________________________________________________________________________________________________________

void OWriteToolBoxDocumentHandler::WriteToolBoxItem(
    const ::rtl::OUString& rCommandURL,
    const ::rtl::OUString& rLabel,
    const ::rtl::OUString& rHelpURL,
    const ::rtl::OUString& rTooltip,
    sal_Int16       nStyle,
    sal_Int16       nWidth,
    sal_Bool        bVisible )
throw ( SAXException, RuntimeException )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    if ( m_aAttributeURL.isEmpty() )
    {
        m_aAttributeURL = m_aXMLXlinkNS;
        m_aAttributeURL += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_URL ));
    }

    // save required attribute (URL)
    pList->AddAttribute( m_aAttributeURL, m_aAttributeType, rCommandURL );

    if ( !rLabel.isEmpty() )
    {
        pList->AddAttribute( m_aXMLToolbarNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_TEXT )),
                             m_aAttributeType,
                             rLabel );
    }

    if ( bVisible == sal_False )
    {
        pList->AddAttribute( m_aXMLToolbarNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_VISIBLE )),
                             m_aAttributeType,
                             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_BOOLEAN_FALSE )) );
    }

    if ( !rHelpURL.isEmpty() )
    {
        pList->AddAttribute( m_aXMLToolbarNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_HELPID )),
                             m_aAttributeType,
                             rHelpURL );
    }

    if ( !rTooltip.isEmpty() )
    {
        pList->AddAttribute( m_aXMLToolbarNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_TOOLTIP )),
                             m_aAttributeType,
                             rTooltip );
    }

    if ( nStyle > 0 )
    {
        rtl::OUString aValue;
        ToolboxStyleItem* pStyle = Styles;

        for ( sal_Int32 nIndex = 0; nIndex < nStyleItemEntries; ++nIndex, ++pStyle )
        {
            if ( nStyle & pStyle->nBit )
            {
                if ( !aValue.isEmpty() )
                    aValue = aValue.concat( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(" ") ) );
                aValue += rtl::OUString::createFromAscii( pStyle->attrName );
            }
        }
        pList->AddAttribute( m_aXMLToolbarNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_ITEMSTYLE )),
                             m_aAttributeType,
                             aValue );
    }

    if ( nWidth > 0 )
    {
        pList->AddAttribute( m_aXMLToolbarNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_WIDTH )),
                             m_aAttributeType,
                             ::rtl::OUString::valueOf( sal_Int32( nWidth )) );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
    m_xWriteDocumentHandler->startElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARITEM )), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
    m_xWriteDocumentHandler->endElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARITEM )) );
}

void OWriteToolBoxDocumentHandler::WriteToolBoxSpace() throw
( SAXException, RuntimeException )
{
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
    m_xWriteDocumentHandler->startElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARSPACE )), m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
    m_xWriteDocumentHandler->endElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARSPACE )) );
}

void OWriteToolBoxDocumentHandler::WriteToolBoxBreak() throw
( SAXException, RuntimeException )
{
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
    m_xWriteDocumentHandler->startElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARBREAK )), m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
    m_xWriteDocumentHandler->endElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARBREAK )) );
}

void OWriteToolBoxDocumentHandler::WriteToolBoxSeparator() throw
( SAXException, RuntimeException )
{
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
    m_xWriteDocumentHandler->startElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARSEPARATOR )), m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
    m_xWriteDocumentHandler->endElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_TOOLBARSEPARATOR )) );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
