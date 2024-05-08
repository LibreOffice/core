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

#include <xml/toolboxdocumenthandler.hxx>
#include <xml/toolboxconfigurationdefines.hxx>

#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>

#include <sal/config.h>
#include <sal/macros.h>
#include <vcl/settings.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>

#include <comphelper/attributelist.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::xml::sax;

constexpr OUStringLiteral TOOLBAR_DOCTYPE = u"<!DOCTYPE toolbar:toolbar PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"toolbar.dtd\">";

namespace framework
{

// Property names of a menu/menu item ItemDescriptor
constexpr OUString ITEM_DESCRIPTOR_COMMANDURL = u"CommandURL"_ustr;
constexpr OUString ITEM_DESCRIPTOR_LABEL = u"Label"_ustr;
constexpr OUString ITEM_DESCRIPTOR_TYPE = u"Type"_ustr;
constexpr OUString ITEM_DESCRIPTOR_STYLE = u"Style"_ustr;
constexpr OUString ITEM_DESCRIPTOR_VISIBLE = u"IsVisible"_ustr;

static void ExtractToolbarParameters( const Sequence< PropertyValue >& rProp,
                                      OUString&                        rCommandURL,
                                      OUString&                        rLabel,
                                      sal_Int16&                       rStyle,
                                      bool&                            rVisible,
                                      sal_Int16&                       rType )
{
    for ( const PropertyValue& rEntry : rProp )
    {
        if ( rEntry.Name == ITEM_DESCRIPTOR_COMMANDURL )
            rEntry.Value >>= rCommandURL;
        else if ( rEntry.Name == ITEM_DESCRIPTOR_LABEL )
            rEntry.Value >>= rLabel;
        else if ( rEntry.Name == ITEM_DESCRIPTOR_TYPE )
            rEntry.Value >>= rType;
        else if ( rEntry.Name == ITEM_DESCRIPTOR_VISIBLE )
            rEntry.Value >>= rVisible;
        else if ( rEntry.Name == ITEM_DESCRIPTOR_STYLE )
            rEntry.Value >>= rStyle;
    }
}

namespace {

struct ToolboxStyleItem
{
    sal_Int16 nBit;
    OUString attrName;
};

}

constexpr ToolboxStyleItem Styles[ ] = {
    { css::ui::ItemStyle::RADIO_CHECK,   ATTRIBUTE_ITEMSTYLE_RADIO },
    { css::ui::ItemStyle::ALIGN_LEFT,    ATTRIBUTE_ITEMSTYLE_LEFT },
    { css::ui::ItemStyle::AUTO_SIZE,     ATTRIBUTE_ITEMSTYLE_AUTO },
    { css::ui::ItemStyle::REPEAT,        ATTRIBUTE_ITEMSTYLE_REPEAT },
    { css::ui::ItemStyle::DROPDOWN_ONLY, ATTRIBUTE_ITEMSTYLE_DROPDOWNONLY },
    { css::ui::ItemStyle::DROP_DOWN,     ATTRIBUTE_ITEMSTYLE_DROPDOWN },
    { css::ui::ItemStyle::ICON,          ATTRIBUTE_ITEMSTYLE_IMAGE },
    { css::ui::ItemStyle::TEXT,          ATTRIBUTE_ITEMSTYLE_TEXT },
};

sal_Int32 const nStyleItemEntries = SAL_N_ELEMENTS(Styles);

namespace {

struct ToolBarEntryProperty
{
    OReadToolBoxDocumentHandler::ToolBox_XML_Namespace  nNamespace;
    OUString aEntryName;
};

}

ToolBarEntryProperty constexpr ToolBoxEntries[OReadToolBoxDocumentHandler::TB_XML_ENTRY_COUNT] =
{
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   u"toolbar"_ustr             },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   u"toolbaritem"_ustr         },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   u"toolbarspace"_ustr        },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   u"toolbarbreak"_ustr        },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   u"toolbarseparator"_ustr    },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ATTRIBUTE_TEXT              },
    { OReadToolBoxDocumentHandler::TB_NS_XLINK,     ATTRIBUTE_URL               },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ATTRIBUTE_VISIBLE           },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ATTRIBUTE_ITEMSTYLE         },
    { OReadToolBoxDocumentHandler::TB_NS_TOOLBAR,   ATTRIBUTE_UINAME            },
};

OReadToolBoxDocumentHandler::OReadToolBoxDocumentHandler( const Reference< XIndexContainer >& rItemContainer ) :
    m_rItemContainer( rItemContainer ),
    m_aType( ITEM_DESCRIPTOR_TYPE ),
    m_aLabel( ITEM_DESCRIPTOR_LABEL ),
    m_aStyle( ITEM_DESCRIPTOR_STYLE ),
    m_aIsVisible( ITEM_DESCRIPTOR_VISIBLE ),
    m_aCommandURL( ITEM_DESCRIPTOR_COMMANDURL )
 {
    // create hash map
    for ( int i = 0; i < TB_XML_ENTRY_COUNT; i++ )
    {
        if ( ToolBoxEntries[i].nNamespace == TB_NS_TOOLBAR )
        {
            OUString temp = XMLNS_TOOLBAR XMLNS_FILTER_SEPARATOR +
                ToolBoxEntries[i].aEntryName;
            m_aToolBoxMap.emplace( temp, static_cast<ToolBox_XML_Entry>(i) );
        }
        else
        {
            OUString temp = XMLNS_XLINK XMLNS_FILTER_SEPARATOR +
                ToolBoxEntries[i].aEntryName;
            m_aToolBoxMap.emplace( temp, static_cast<ToolBox_XML_Entry>(i) );
        }
    }

    m_bToolBarStartFound            = false;
    m_bToolBarItemStartFound        = false;
    m_bToolBarSpaceStartFound       = false;
    m_bToolBarBreakStartFound       = false;
    m_bToolBarSeparatorStartFound   = false;
}

OReadToolBoxDocumentHandler::~OReadToolBoxDocumentHandler()
{
}

// XDocumentHandler
void SAL_CALL OReadToolBoxDocumentHandler::startDocument()
{
}

void SAL_CALL OReadToolBoxDocumentHandler::endDocument()
{
    if ( m_bToolBarStartFound )
    {
        OUString aErrorMessage = getErrorLineString() + "No matching start or end element 'toolbar' found!";
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}

void SAL_CALL OReadToolBoxDocumentHandler::startElement(
    const OUString& aName, const Reference< XAttributeList > &xAttribs )
{
    ToolBoxHashMap::const_iterator pToolBoxEntry = m_aToolBoxMap.find( aName );
    if ( pToolBoxEntry == m_aToolBoxMap.end() )
        return;

    switch ( pToolBoxEntry->second )
    {
        case TB_ELEMENT_TOOLBAR:
        {
            if ( m_bToolBarStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "Element 'toolbar:toolbar' cannot be embedded into 'toolbar:toolbar'!";
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
                            xPropSet->setPropertyValue(u"UIName"_ustr, Any( aUIName ) );
                        }
                        catch ( const UnknownPropertyException& )
                        {
                        }
                    }

                }
            }
            m_bToolBarStartFound = true;
        }
        break;

        case TB_ELEMENT_TOOLBARITEM:
        {
            if ( !m_bToolBarStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "Element 'toolbar:toolbaritem' must be embedded into element 'toolbar:toolbar'!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            if ( m_bToolBarSeparatorStartFound ||
                 m_bToolBarBreakStartFound ||
                 m_bToolBarSpaceStartFound ||
                 m_bToolBarItemStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "Element toolbar:toolbaritem is not a container!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            bool bAttributeURL  = false;

            m_bToolBarItemStartFound = true;
            OUString        aLabel;
            OUString        aCommandURL;
            sal_uInt16      nItemBits( 0 );
            bool            bVisible( true );

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

                        case TB_ATTRIBUTE_URL:
                        {
                            bAttributeURL   = true;
                            aCommandURL     = xAttribs->getValueByIndex( n );
                        }
                        break;

                        case TB_ATTRIBUTE_VISIBLE:
                        {
                            if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_BOOLEAN_TRUE )
                                bVisible = true;
                            else if ( xAttribs->getValueByIndex( n ) == ATTRIBUTE_BOOLEAN_FALSE )
                                bVisible = false;
                            else
                            {
                                OUString aErrorMessage = getErrorLineString() + "Attribute toolbar:visible must have value 'true' or 'false'!";
                                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                            }
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
                                    if ( aToken == ATTRIBUTE_ITEMSTYLE_RADIO )
                                        nItemBits |= css::ui::ItemStyle::RADIO_CHECK;
                                    else if ( aToken == ATTRIBUTE_ITEMSTYLE_LEFT )
                                        nItemBits |= css::ui::ItemStyle::ALIGN_LEFT;
                                    else if ( aToken == ATTRIBUTE_ITEMSTYLE_AUTOSIZE )
                                        nItemBits |= css::ui::ItemStyle::AUTO_SIZE;
                                    else if ( aToken == ATTRIBUTE_ITEMSTYLE_REPEAT )
                                        nItemBits |= css::ui::ItemStyle::REPEAT;
                                    else if ( aToken == ATTRIBUTE_ITEMSTYLE_DROPDOWNONLY )
                                        nItemBits |= css::ui::ItemStyle::DROPDOWN_ONLY;
                                    else if ( aToken == ATTRIBUTE_ITEMSTYLE_DROPDOWN )
                                        nItemBits |= css::ui::ItemStyle::DROP_DOWN;
                                    else if ( aToken == ATTRIBUTE_ITEMSTYLE_TEXT )
                                        nItemBits |= css::ui::ItemStyle::TEXT;
                                    else if ( aToken == ATTRIBUTE_ITEMSTYLE_IMAGE )
                                        nItemBits |= css::ui::ItemStyle::ICON;
                                }
                            }
                            while ( nIndex >= 0 );
                        }
                        break;

                        default:
                        break;
                    }
                }
            } // for

            if ( !bAttributeURL )
            {
                OUString aErrorMessage = getErrorLineString() + "Required attribute toolbar:url must have a value!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            if ( !aCommandURL.isEmpty() )
            {
                //fix for fdo#39370
                /// check whether RTL interface or not
                if(AllSettings::GetLayoutRTL()){
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
                    else if (aCommandURL == ".uno:WrapLeft")
                        aCommandURL = ".uno:WrapRight";
                    else if (aCommandURL == ".uno:WrapRight")
                        aCommandURL = ".uno:WrapLeft";
                }

                auto aToolbarItemProp( comphelper::InitPropertySequence( {
                    { m_aCommandURL, css::uno::Any( aCommandURL ) },
                    { m_aLabel, css::uno::Any( aLabel ) },
                    { m_aType, css::uno::Any( css::ui::ItemType::DEFAULT ) },
                    { m_aStyle, css::uno::Any( nItemBits ) },
                    { m_aIsVisible, css::uno::Any( bVisible ) },
                } ) );

                m_rItemContainer->insertByIndex( m_rItemContainer->getCount(), Any( aToolbarItemProp ) );
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
                OUString aErrorMessage = getErrorLineString() + "Element toolbar:toolbarspace is not a container!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_bToolBarSpaceStartFound = true;

            Sequence< PropertyValue > aToolbarItemProp{
                comphelper::makePropertyValue(m_aCommandURL, OUString()),
                comphelper::makePropertyValue(m_aType, css::ui::ItemType::SEPARATOR_SPACE)
            };

            m_rItemContainer->insertByIndex( m_rItemContainer->getCount(), Any( aToolbarItemProp ) );
        }
        break;

        case TB_ELEMENT_TOOLBARBREAK:
        {
            if ( m_bToolBarSeparatorStartFound ||
                 m_bToolBarBreakStartFound ||
                 m_bToolBarSpaceStartFound ||
                 m_bToolBarItemStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "Element toolbar:toolbarbreak is not a container!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_bToolBarBreakStartFound = true;

            Sequence< PropertyValue > aToolbarItemProp{
                comphelper::makePropertyValue(m_aCommandURL, OUString()),
                comphelper::makePropertyValue(m_aType, css::ui::ItemType::SEPARATOR_LINEBREAK)
            };

            m_rItemContainer->insertByIndex( m_rItemContainer->getCount(), Any( aToolbarItemProp ) );
        }
        break;

        case TB_ELEMENT_TOOLBARSEPARATOR:
        {
            if ( m_bToolBarSeparatorStartFound ||
                 m_bToolBarBreakStartFound ||
                 m_bToolBarSpaceStartFound ||
                 m_bToolBarItemStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "Element toolbar:toolbarseparator is not a container!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_bToolBarSeparatorStartFound = true;

            Sequence< PropertyValue > aToolbarItemProp{
                comphelper::makePropertyValue(m_aCommandURL, OUString()),
                comphelper::makePropertyValue(m_aType, css::ui::ItemType::SEPARATOR_LINE)
            };

            m_rItemContainer->insertByIndex( m_rItemContainer->getCount(), Any( aToolbarItemProp ) );
        }
        break;

              default:
                  break;
    }
}

void SAL_CALL OReadToolBoxDocumentHandler::endElement(const OUString& aName)
{
    ToolBoxHashMap::const_iterator pToolBoxEntry = m_aToolBoxMap.find( aName );
    if ( pToolBoxEntry == m_aToolBoxMap.end() )
        return;

    switch ( pToolBoxEntry->second )
    {
        case TB_ELEMENT_TOOLBAR:
        {
            if ( !m_bToolBarStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "End element 'toolbar' found, but no start element 'toolbar'";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_bToolBarStartFound = false;
        }
        break;

        case TB_ELEMENT_TOOLBARITEM:
        {
            if ( !m_bToolBarItemStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "End element 'toolbar:toolbaritem' found, but no start element 'toolbar:toolbaritem'";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_bToolBarItemStartFound = false;
        }
        break;

        case TB_ELEMENT_TOOLBARBREAK:
        {
            if ( !m_bToolBarBreakStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "End element 'toolbar:toolbarbreak' found, but no start element 'toolbar:toolbarbreak'";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_bToolBarBreakStartFound = false;
        }
        break;

        case TB_ELEMENT_TOOLBARSPACE:
        {
            if ( !m_bToolBarSpaceStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "End element 'toolbar:toolbarspace' found, but no start element 'toolbar:toolbarspace'";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_bToolBarSpaceStartFound = false;
        }
        break;

        case TB_ELEMENT_TOOLBARSEPARATOR:
        {
            if ( !m_bToolBarSeparatorStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "End element 'toolbar:toolbarseparator' found, but no start element 'toolbar:toolbarseparator'";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_bToolBarSeparatorStartFound = false;
        }
        break;

        default: break;
    }
}

void SAL_CALL OReadToolBoxDocumentHandler::characters(const OUString&)
{
}

void SAL_CALL OReadToolBoxDocumentHandler::ignorableWhitespace(const OUString&)
{
}

void SAL_CALL OReadToolBoxDocumentHandler::processingInstruction(
    const OUString& /*aTarget*/, const OUString& /*aData*/ )
{
}

void SAL_CALL OReadToolBoxDocumentHandler::setDocumentLocator(
    const Reference< XLocator > &xLocator)
{
    m_xLocator = xLocator;
}

OUString OReadToolBoxDocumentHandler::getErrorLineString()
{
    if ( m_xLocator.is() )
        return "Line: " + OUString::number( m_xLocator->getLineNumber() ) + " - ";
    else
        return OUString();
}

//  OWriteToolBoxDocumentHandler

OWriteToolBoxDocumentHandler::OWriteToolBoxDocumentHandler(
    const Reference< XIndexAccess >& rItemAccess,
    Reference< XDocumentHandler > const & rWriteDocumentHandler ) :
    m_xWriteDocumentHandler( rWriteDocumentHandler ),
    m_rItemAccess( rItemAccess )
{
    m_xEmptyList = new ::comphelper::AttributeList;
    m_aXMLXlinkNS       = XMLNS_XLINK_PREFIX;
    m_aXMLToolbarNS     = XMLNS_TOOLBAR_PREFIX;
}

OWriteToolBoxDocumentHandler::~OWriteToolBoxDocumentHandler()
{
}

void OWriteToolBoxDocumentHandler::WriteToolBoxDocument()
{
    m_xWriteDocumentHandler->startDocument();

    // write DOCTYPE line!
    Reference< XExtendedDocumentHandler > xExtendedDocHandler( m_xWriteDocumentHandler, UNO_QUERY );
    if ( xExtendedDocHandler.is() )
    {
        xExtendedDocHandler->unknown( TOOLBAR_DOCTYPE );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    }

    OUString aUIName;
    Reference< XPropertySet > xPropSet( m_rItemAccess, UNO_QUERY );
    if ( xPropSet.is() )
    {
        try
        {
            xPropSet->getPropertyValue(u"UIName"_ustr) >>= aUIName;
        }
        catch ( const UnknownPropertyException& )
        {
        }
    }

    rtl::Reference<::comphelper::AttributeList> pList = new ::comphelper::AttributeList;

    pList->AddAttribute( ATTRIBUTE_XMLNS_TOOLBAR,
                         u"" XMLNS_TOOLBAR ""_ustr );

    pList->AddAttribute( ATTRIBUTE_XMLNS_XLINK,
                         u"" XMLNS_XLINK ""_ustr );

    if ( !aUIName.isEmpty() )
        pList->AddAttribute( m_aXMLToolbarNS + ATTRIBUTE_UINAME,
                             aUIName );

    m_xWriteDocumentHandler->startElement( ELEMENT_NS_TOOLBAR, pList );
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
            bool    bVisible( true );
            sal_Int16   nType( css::ui::ItemType::DEFAULT );
            sal_Int16   nStyle( 0 );

            ExtractToolbarParameters( aProps, aCommandURL, aLabel, nStyle, bVisible, nType );
            if ( nType == css::ui::ItemType::DEFAULT )
                WriteToolBoxItem( aCommandURL, aLabel, nStyle, bVisible );
            else if ( nType == css::ui::ItemType::SEPARATOR_SPACE )
                WriteToolBoxSpace();
            else if ( nType == css::ui::ItemType::SEPARATOR_LINE )
                WriteToolBoxSeparator();
            else if ( nType == css::ui::ItemType::SEPARATOR_LINEBREAK )
                WriteToolBoxBreak();
        }
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( ELEMENT_NS_TOOLBAR );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endDocument();
}

//  protected member functions

void OWriteToolBoxDocumentHandler::WriteToolBoxItem(
    const OUString& rCommandURL,
    const OUString& rLabel,
    sal_Int16       nStyle,
    bool        bVisible )
{
    rtl::Reference<::comphelper::AttributeList> pList = new ::comphelper::AttributeList;

    if ( m_aAttributeURL.isEmpty() )
    {
        m_aAttributeURL = m_aXMLXlinkNS + ATTRIBUTE_URL;
    }

    // save required attribute (URL)
    pList->AddAttribute( m_aAttributeURL, rCommandURL );

    if ( !rLabel.isEmpty() )
    {
        pList->AddAttribute( m_aXMLToolbarNS + ATTRIBUTE_TEXT,
                             rLabel );
    }

    if ( !bVisible )
    {
        pList->AddAttribute( m_aXMLToolbarNS + ATTRIBUTE_VISIBLE,
                             ATTRIBUTE_BOOLEAN_FALSE );
    }

    if ( nStyle > 0 )
    {
        OUStringBuffer aValue;
        const ToolboxStyleItem* pStyle = Styles;

        for ( sal_Int32 nIndex = 0; nIndex < nStyleItemEntries; ++nIndex, ++pStyle )
        {
            if ( nStyle & pStyle->nBit )
            {
                if ( !aValue.isEmpty() )
                    aValue.append(" ");
                aValue.append( pStyle->attrName );
            }
        }
        pList->AddAttribute( m_aXMLToolbarNS + ATTRIBUTE_ITEMSTYLE,
                             aValue.makeStringAndClear() );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( ELEMENT_NS_TOOLBARITEM, pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( ELEMENT_NS_TOOLBARITEM );
}

void OWriteToolBoxDocumentHandler::WriteToolBoxSpace()
{
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( ELEMENT_NS_TOOLBARSPACE, m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( ELEMENT_NS_TOOLBARSPACE );
}

void OWriteToolBoxDocumentHandler::WriteToolBoxBreak()
{
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( ELEMENT_NS_TOOLBARBREAK, m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( ELEMENT_NS_TOOLBARBREAK );
}

void OWriteToolBoxDocumentHandler::WriteToolBoxSeparator()
{
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->startElement( ELEMENT_NS_TOOLBARSEPARATOR, m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( ELEMENT_NS_TOOLBARSEPARATOR );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
