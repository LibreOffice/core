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

#include <xml/imagesdocumenthandler.hxx>

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>

#include <rtl/ref.hxx>

#include <o3tl/enumarray.hxx>
#include <o3tl/enumrange.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

constexpr OUString ELEMENT_NS_IMAGESCONTAINER = u"image:imagescontainer"_ustr;
constexpr OUString ELEMENT_NS_IMAGES = u"image:images"_ustr;
constexpr OUString ELEMENT_NS_ENTRY = u"image:entry"_ustr;

constexpr OUString ATTRIBUTE_XMLNS_IMAGE = u"xmlns:image"_ustr;
constexpr OUString ATTRIBUTE_XMLNS_XLINK = u"xmlns:xlink"_ustr;

constexpr OUString ATTRIBUTE_XLINK_HREF = u"xlink:href"_ustr;
constexpr OUString ATTRIBUTE_XLINK_TYPE = u"xlink:type"_ustr;
constexpr OUString ATTRIBUTE_XLINK_TYPE_VALUE = u"simple"_ustr;

constexpr OUString XMLNS_IMAGE = u"http://openoffice.org/2001/image"_ustr;
constexpr OUString XMLNS_XLINK = u"http://www.w3.org/1999/xlink"_ustr;
constexpr OUString XMLNS_IMAGE_PREFIX = u"image:"_ustr;

constexpr OUString XMLNS_FILTER_SEPARATOR = u"^"_ustr;

constexpr OUString IMAGES_DOCTYPE
    = u"<!DOCTYPE image:imagecontainer PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"image.dtd\">"_ustr;

namespace framework
{

namespace {
using ImageXMLNamespace = OReadImagesDocumentHandler::ImageXMLNamespace;
using ImageXMLEntry = OReadImagesDocumentHandler::ImageXMLEntry;

struct ImageXMLEntryProperty
{
    ImageXMLNamespace nNamespace;
    OUString aEntryName;
};

}

constexpr o3tl::enumarray<ImageXMLEntry, ImageXMLEntryProperty> ImagesEntries
    = { ImageXMLEntryProperty{ ImageXMLNamespace::Image, u"imagescontainer"_ustr },
        ImageXMLEntryProperty{ ImageXMLNamespace::Image, u"images"_ustr },
        ImageXMLEntryProperty{ ImageXMLNamespace::Image, u"entry"_ustr },
        ImageXMLEntryProperty{ ImageXMLNamespace::Image, u"externalimages"_ustr },
        ImageXMLEntryProperty{ ImageXMLNamespace::Image, u"externalentry"_ustr },
        ImageXMLEntryProperty{ ImageXMLNamespace::XLink, u"href"_ustr },
        ImageXMLEntryProperty{ ImageXMLNamespace::Image, u"maskcolor"_ustr },
        ImageXMLEntryProperty{ ImageXMLNamespace::Image, u"command"_ustr },
        ImageXMLEntryProperty{ ImageXMLNamespace::Image, u"bitmap-index"_ustr },
        ImageXMLEntryProperty{ ImageXMLNamespace::Image, u"maskurl"_ustr },
        ImageXMLEntryProperty{ ImageXMLNamespace::Image, u"maskmode"_ustr },
        ImageXMLEntryProperty{ ImageXMLNamespace::Image, u"highcontrasturl"_ustr },
        ImageXMLEntryProperty{ ImageXMLNamespace::Image, u"highcontrastmaskurl"_ustr } };

OReadImagesDocumentHandler::OReadImagesDocumentHandler( ImageItemDescriptorList& rItems ) :
    m_rImageList( rItems )
{
    // create hash map to speed up lookup
    for (ImageXMLEntry e : o3tl::enumrange<ImageXMLEntry>())
    {
        const auto& rProp = ImagesEntries[e];

        const OUString& rNsURL
            = rProp.nNamespace == ImageXMLNamespace::Image ? XMLNS_IMAGE : XMLNS_XLINK;

        m_aImageMap.emplace(rNsURL + XMLNS_FILTER_SEPARATOR + rProp.aEntryName, e);
    }

    // reset states
    m_bImageContainerStartFound     = false;
    m_bImageContainerEndFound       = false;
    m_bImagesStartFound             = false;
}

OReadImagesDocumentHandler::~OReadImagesDocumentHandler()
{
}

// XDocumentHandler
void SAL_CALL OReadImagesDocumentHandler::startDocument()
{
}

void SAL_CALL OReadImagesDocumentHandler::endDocument()
{
    if (m_bImageContainerStartFound != m_bImageContainerEndFound)
    {
        OUString aErrorMessage = getErrorLineString() + "No matching start or end element 'image:imagecontainer' found!";
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}

void SAL_CALL OReadImagesDocumentHandler::startElement(
    const OUString& aName, const Reference< XAttributeList > &xAttribs )
{
    auto pImageEntry = m_aImageMap.find( aName );
    if ( pImageEntry == m_aImageMap.end() )
        return;

    switch ( pImageEntry->second )
    {
        case ImageXMLEntry::ElementImageContainer:
        {
            // image:imagecontainer element (container element for all further image elements)
            if ( m_bImageContainerStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "Element 'image:imagecontainer' cannot be embedded into 'image:imagecontainer'!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_bImageContainerStartFound = true;
        }
        break;

        case ImageXMLEntry::ElementImages:
        {
            if ( !m_bImageContainerStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "Element 'image:images' must be embedded into element 'image:imagecontainer'!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            if ( m_bImagesStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "Element 'image:images' cannot be embedded into 'image:images'!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_bImagesStartFound = true;
        }
        break;

        case ImageXMLEntry::ElementEntry:
        {
            // Check that image:entry is embedded into image:images!
            if ( !m_bImagesStartFound )
            {
                OUString aErrorMessage = getErrorLineString() + "Element 'image:entry' must be embedded into element 'image:images'!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            // Create new image item descriptor
            ImageItemDescriptor aItem;

            // Read attributes for this image definition
            for ( sal_Int16 n = 0; n < xAttribs->getLength(); n++ )
            {
                pImageEntry = m_aImageMap.find( xAttribs->getNameByIndex( n ) );
                if ( pImageEntry != m_aImageMap.end() )
                {
                    switch ( pImageEntry->second )
                    {
                        case ImageXMLEntry::AttributeCommand:
                        {
                            aItem.aCommandURL  = xAttribs->getValueByIndex( n );
                        }
                        break;

                        default:
                            break;
                    }
                }
            }

            // Check required attribute "command"
            if ( aItem.aCommandURL.isEmpty() )
            {
                OUString aErrorMessage = getErrorLineString() + "Required attribute 'image:command' must have a value!";
                throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
            }

            m_rImageList.aImageItemDescriptors.push_back(aItem);
        }
        break;

        default:
        break;
    }
}

void SAL_CALL OReadImagesDocumentHandler::endElement(const OUString& aName)
{
    auto pImageEntry = m_aImageMap.find( aName );
    if ( pImageEntry == m_aImageMap.end() )
        return;

    switch ( pImageEntry->second )
    {
        case ImageXMLEntry::ElementImageContainer:
        {
            m_bImageContainerEndFound = true;
        }
        break;

        case ImageXMLEntry::ElementImages:
        {
            m_bImagesStartFound = false;
        }
        break;

        default: break;
    }
}

void SAL_CALL OReadImagesDocumentHandler::characters(const OUString&)
{
}

void SAL_CALL OReadImagesDocumentHandler::ignorableWhitespace(const OUString&)
{
}

void SAL_CALL OReadImagesDocumentHandler::processingInstruction(
    const OUString& /*aTarget*/, const OUString& /*aData*/ )
{
}

void SAL_CALL OReadImagesDocumentHandler::setDocumentLocator(
    const Reference< XLocator > &xLocator)
{
    m_xLocator = xLocator;
}

OUString OReadImagesDocumentHandler::getErrorLineString()
{
    if ( m_xLocator.is() )
    {
        return "Line: " +
            OUString::number(m_xLocator->getLineNumber()) +
            " - ";
    }
    else
        return OUString();
}

//  OWriteImagesDocumentHandler

OWriteImagesDocumentHandler::OWriteImagesDocumentHandler(
    const ImageItemDescriptorList& rItems,
    Reference< XDocumentHandler > const & rWriteDocumentHandler ) :
    m_rImageItemList( rItems ),
    m_xWriteDocumentHandler( rWriteDocumentHandler )
{
    m_aXMLImageNS           = XMLNS_IMAGE_PREFIX;
    m_aAttributeXlinkType   = ATTRIBUTE_XLINK_TYPE;
    m_aAttributeValueSimple = ATTRIBUTE_XLINK_TYPE_VALUE;
}

OWriteImagesDocumentHandler::~OWriteImagesDocumentHandler()
{
}

void OWriteImagesDocumentHandler::WriteImagesDocument()
{
    m_xWriteDocumentHandler->startDocument();

    // write DOCTYPE line!
    Reference< XExtendedDocumentHandler > xExtendedDocHandler( m_xWriteDocumentHandler, UNO_QUERY );
    if ( xExtendedDocHandler.is() )
    {
        xExtendedDocHandler->unknown( IMAGES_DOCTYPE );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    }

    rtl::Reference<::comphelper::AttributeList> pList = new ::comphelper::AttributeList;

    pList->AddAttribute( ATTRIBUTE_XMLNS_IMAGE,
                         XMLNS_IMAGE );

    pList->AddAttribute( ATTRIBUTE_XMLNS_XLINK,
                         XMLNS_XLINK );

    m_xWriteDocumentHandler->startElement( ELEMENT_NS_IMAGESCONTAINER, pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    WriteImageList( &m_rImageItemList );

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( ELEMENT_NS_IMAGESCONTAINER );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endDocument();
}

//  protected member functions

void OWriteImagesDocumentHandler::WriteImageList( const ImageItemDescriptorList* pImageList )
{
    rtl::Reference<::comphelper::AttributeList> pList = new ::comphelper::AttributeList;

    // save required attributes
    pList->AddAttribute( m_aAttributeXlinkType,
                         m_aAttributeValueSimple );

    pList->AddAttribute(ATTRIBUTE_XLINK_HREF, pImageList->aURL);

    m_xWriteDocumentHandler->startElement( ELEMENT_NS_IMAGES, pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    for (const ImageItemDescriptor & i : pImageList->aImageItemDescriptors)
        WriteImage( &i );

    m_xWriteDocumentHandler->endElement( ELEMENT_NS_IMAGES );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
}

void OWriteImagesDocumentHandler::WriteImage( const ImageItemDescriptor* pImage )
{
    rtl::Reference<::comphelper::AttributeList> pList = new ::comphelper::AttributeList;

    pList->AddAttribute(m_aXMLImageNS
                            + ImagesEntries[ImageXMLEntry::AttributeBitmapIndex].aEntryName,
                        OUString::number(pImage->nIndex));

    pList->AddAttribute(m_aXMLImageNS + ImagesEntries[ImageXMLEntry::AttributeCommand].aEntryName,
                        pImage->aCommandURL);

    m_xWriteDocumentHandler->startElement( ELEMENT_NS_ENTRY, pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    m_xWriteDocumentHandler->endElement( ELEMENT_NS_ENTRY );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
