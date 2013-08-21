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
                                      OUString&                       rCommandURL,
                                      OUString&                       rLabel,
                                      OUString&                       rHelpURL,
                                      OUString&                       rTooltip,
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
    m_aType( ITEM_DESCRIPTOR_TYPE ),
    m_aLabel( ITEM_DESCRIPTOR_LABEL ),
    m_aStyle( ITEM_DESCRIPTOR_STYLE ),
    m_aHelpURL( ITEM_DESCRIPTOR_HELPURL ),
    m_aTooltip( ITEM_DESCRIPTOR_TOOLTIP ),
    m_aIsVisible( ITEM_DESCRIPTOR_VISIBLE ),
    m_aCommandURL( ITEM_DESCRIPTOR_COMMANDURL )
 {
    OUString aNamespaceToolBar( XMLNS_TOOLBAR );
    OUString aNamespaceXLink( XMLNS_XLINK );
    OUString aSeparator( XMLNS_FILTER_SEPARATOR );

    // create hash map
    for ( int i = 0; i < (int)TB_XML_ENTRY_COUNT; i++ )
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
    m_nHashCode_Style_Radio         = OUString( ATTRIBUTE_ITEMSTYLE_RADIO ).hashCode();
    m_nHashCode_Style_Auto          = OUString( ATTRIBUTE_ITEMSTYLE_AUTO ).hashCode();
    m_nHashCode_Style_Left          = OUString( ATTRIBUTE_ITEMSTYLE_LEFT ).hashCode();
    m_nHashCode_Style_AutoSize      = OUString( ATTRIBUTE_ITEMSTYLE_AUTOSIZE ).hashCode();
    m_nHashCode_Style_DropDown      = OUString( ATTRIBUTE_ITEMSTYLE_DROPDOWN ).hashCode();
    m_nHashCode_Style_Repeat        = OUString( ATTRIBUTE_ITEMSTYLE_REPEAT ).hashCode();
    m_nHashCode_Style_DropDownOnly  = OUString( ATTRIBUTE_ITEMSTYLE_DROPDOWNONLY ).hashCode();
    m_nHashCode_Style_Text  = OUString( ATTRIBUTE_ITEMSTYLE_TEXT ).hashCode();
    m_nHashCode_Style_Image  = OUString( ATTRIBUTE_ITEMSTYLE_IMAGE ).hashCode();

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
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += "No matching start or end element 'toolbar' found!";
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
                    aErrorMessage += "Element 'toolbar:toolbar' cannot be embeded into 'toolbar:toolbar'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }
                        else
                        {
                            // Check if we have a UI name set in our XML file
                            OUString aUIName;
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
                                        xPropSet->setPropertyValue("UIName", makeAny( aUIName ) );
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
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'toolbar:toolbaritem' must be embeded into element 'toolbar:toolbar'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_bToolBarSeparatorStartFound ||
                     m_bToolBarBreakStartFound ||
                     m_bToolBarSpaceStartFound ||
                     m_bToolBarItemStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element toolbar:toolbaritem is not a container!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                sal_Bool bAttributeURL  = sal_False;

                m_bToolBarItemStartFound = sal_True;
                OUString        aLabel;
                OUString        aCommandURL;
                OUString        aHelpURL;
                OUString        aTooltip;
                OUString        aBitmapName;
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
                                    OUString aErrorMessage = getErrorLineString();
                                    aErrorMessage += "Attribute toolbar:visible must have value 'true' or 'false'!";
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
                                // read space separated item style list
                                OUString aTemp = xAttribs->getValueByIndex( n );
                                sal_Int32 nIndex = 0;

                                do
                                {
                                    OUString aToken  = aTemp.getToken( 0, ' ', nIndex );
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
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Required attribute toolbar:url must have a value!";
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

                    //fix for fdo#39370
                    /// check whether RTL interface or not
                    if(Application::GetSettings().GetLayoutRTL()){
                        if (aCommandURL == ".uno:ParaLeftToRight")
                            aCommandURL = ".uno:ParaRightToLeft";
                        else if (aCommandURL == ".uno:ParaRightToLeft")
                            aCommandURL = ".uno:ParaLeftToRight";
                        else if (aCommandURL == ".uno:LeftPara")
                            aCommandURL = ".uno:RightPara";
                        else if (aCommandURL == ".uno:RightPara")
                            aCommandURL = ".uno:LeftPara";
                        else if (aCommandURL == ".uno:AlignLeft")
                            aCommandURL = ".uno:AlignRight";
                        else if (aCommandURL == ".uno:AlignRight")
                            aCommandURL = ".uno:AlignLeft";
                    }

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
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element toolbar:toolbarspace is not a container!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarSpaceStartFound = sal_True;

                Sequence< PropertyValue > aToolbarItemProp( 2 );
                aToolbarItemProp[0].Name = m_aCommandURL;
                aToolbarItemProp[1].Name = m_aType;

                aToolbarItemProp[0].Value <<= OUString();
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
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element toolbar:toolbarbreak is not a container!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarBreakStartFound = sal_True;

                Sequence< PropertyValue > aToolbarItemProp( 2 );
                aToolbarItemProp[0].Name = m_aCommandURL;
                aToolbarItemProp[1].Name = m_aType;

                aToolbarItemProp[0].Value <<= OUString();
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
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element toolbar:toolbarseparator is not a container!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bToolBarSeparatorStartFound = sal_True;

                Sequence< PropertyValue > aToolbarItemProp( 2 );
                aToolbarItemProp[0].Name = m_aCommandURL;
                aToolbarItemProp[1].Name = m_aType;

                aToolbarItemProp[0].Value <<= OUString();
                aToolbarItemProp[1].Value <<= ::com::sun::star::ui::ItemType::SEPARATOR_LINE;

                m_rItemContainer->insertByIndex( m_rItemContainer->getCount(), makeAny( aToolbarItemProp ) );
            }
            break;

                  default:
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
                    aErrorMessage += "End element 'toolbar' found, but no start element 'toolbar'";
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
                    aErrorMessage += "End element 'toolbar:toolbaritem' found, but no start element 'toolbar:toolbaritem'";
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
                    aErrorMessage += "End element 'toolbar:toolbarbreak' found, but no start element 'toolbar:toolbarbreak'";
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
                    aErrorMessage += "End element 'toolbar:toolbarspace' found, but no start element 'toolbar:toolbarspace'";
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
                    aErrorMessage += "End element 'toolbar:toolbarseparator' found, but no start element 'toolbar:toolbarseparator'";
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

void SAL_CALL OReadToolBoxDocumentHandler::characters(const OUString&)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadToolBoxDocumentHandler::ignorableWhitespace(const OUString&)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadToolBoxDocumentHandler::processingInstruction(
    const OUString& /*aTarget*/, const OUString& /*aData*/ )
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

OUString OReadToolBoxDocumentHandler::getErrorLineString()
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
    m_aAttributeType    = OUString( ATTRIBUTE_TYPE_CDATA );
    m_aXMLXlinkNS       = OUString( XMLNS_XLINK_PREFIX );
    m_aXMLToolbarNS     = OUString( XMLNS_TOOLBAR_PREFIX );
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
        xExtendedDocHandler->unknown( OUString( TOOLBAR_DOCTYPE ) );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    }

    OUString aUIName;
    Reference< XPropertySet > xPropSet( m_rItemAccess, UNO_QUERY );
    if ( xPropSet.is() )
    {
        try
        {
            xPropSet->getPropertyValue("UIName") >>= aUIName;
        }
        catch ( const UnknownPropertyException& )
        {
        }
    }

    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    pList->AddAttribute( OUString( ATTRIBUTE_XMLNS_TOOLBAR ),
                         m_aAttributeType,
                         OUString( XMLNS_TOOLBAR ) );

    pList->AddAttribute( OUString( ATTRIBUTE_XMLNS_XLINK ),
                         m_aAttributeType,
                         OUString( XMLNS_XLINK ) );

    if ( !aUIName.isEmpty() )
        pList->AddAttribute( m_aXMLToolbarNS + OUString( ATTRIBUTE_UINAME ),
                             m_aAttributeType,
                             aUIName );

    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_TOOLBAR ), pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    sal_Int32  nItemCount = m_rItemAccess->getCount();
    Any        aAny;

    for ( sal_Int32 nItemPos = 0; nItemPos < nItemCount; nItemPos++ )
    {
        Sequence< PropertyValue > aProps;
        aAny = m_rItemAccess->getByIndex( nItemPos );
        if ( aAny >>= aProps )
        {
            OUString    aCommandURL;
            OUString    aLabel;
            OUString    aHelpURL;
            OUString    aTooltip;
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

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_TOOLBAR ) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endDocument();
}

//_________________________________________________________________________________________________________________
//  protected member functions
//_________________________________________________________________________________________________________________

void OWriteToolBoxDocumentHandler::WriteToolBoxItem(
    const OUString& rCommandURL,
    const OUString& rLabel,
    const OUString& rHelpURL,
    const OUString& rTooltip,
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
        m_aAttributeURL += OUString( ATTRIBUTE_URL );
    }

    // save required attribute (URL)
    pList->AddAttribute( m_aAttributeURL, m_aAttributeType, rCommandURL );

    if ( !rLabel.isEmpty() )
    {
        pList->AddAttribute( m_aXMLToolbarNS + OUString( ATTRIBUTE_TEXT ),
                             m_aAttributeType,
                             rLabel );
    }

    if ( bVisible == sal_False )
    {
        pList->AddAttribute( m_aXMLToolbarNS + OUString( ATTRIBUTE_VISIBLE ),
                             m_aAttributeType,
                             OUString( ATTRIBUTE_BOOLEAN_FALSE ) );
    }

    if ( !rHelpURL.isEmpty() )
    {
        pList->AddAttribute( m_aXMLToolbarNS + OUString( ATTRIBUTE_HELPID ),
                             m_aAttributeType,
                             rHelpURL );
    }

    if ( !rTooltip.isEmpty() )
    {
        pList->AddAttribute( m_aXMLToolbarNS + OUString( ATTRIBUTE_TOOLTIP ),
                             m_aAttributeType,
                             rTooltip );
    }

    if ( nStyle > 0 )
    {
        OUString aValue;
        ToolboxStyleItem* pStyle = Styles;

        for ( sal_Int32 nIndex = 0; nIndex < nStyleItemEntries; ++nIndex, ++pStyle )
        {
            if ( nStyle & pStyle->nBit )
            {
                if ( !aValue.isEmpty() )
                    aValue = aValue.concat( OUString( " " ) );
                aValue += OUString::createFromAscii( pStyle->attrName );
            }
        }
        pList->AddAttribute( m_aXMLToolbarNS + OUString( ATTRIBUTE_ITEMSTYLE ),
                             m_aAttributeType,
                             aValue );
    }

    if ( nWidth > 0 )
    {
        pList->AddAttribute( m_aXMLToolbarNS + OUString( ATTRIBUTE_WIDTH ),
                             m_aAttributeType,
                             OUString::number( nWidth) );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_TOOLBARITEM ), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_TOOLBARITEM ) );
}

void OWriteToolBoxDocumentHandler::WriteToolBoxSpace() throw
( SAXException, RuntimeException )
{
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_TOOLBARSPACE ), m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_TOOLBARSPACE ) );
}

void OWriteToolBoxDocumentHandler::WriteToolBoxBreak() throw
( SAXException, RuntimeException )
{
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_TOOLBARBREAK ), m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_TOOLBARBREAK ) );
}

void OWriteToolBoxDocumentHandler::WriteToolBoxSeparator() throw
( SAXException, RuntimeException )
{
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_TOOLBARSEPARATOR ), m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_TOOLBARSEPARATOR ) );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
