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

#include <xml/imagesdocumenthandler.hxx>

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <rtl/ustrbuf.hxx>

#include <comphelper/attributelist.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

#define ELEMENT_IMAGECONTAINER      "imagescontainer"
#define ELEMENT_IMAGES              "images"
#define ELEMENT_ENTRY               "entry"
#define ELEMENT_EXTERNALIMAGES      "externalimages"
#define ELEMENT_EXTERNALENTRY       "externalentry"

#define ELEMENT_NS_IMAGESCONTAINER  "image:imagescontainer"
#define ELEMENT_NS_IMAGES           "image:images"
#define ELEMENT_NS_ENTRY            "image:entry"
#define ELEMENT_NS_EXTERNALIMAGES   "image:externalimages"
#define ELEMENT_NS_EXTERNALENTRY    "image:externalentry"

#define ATTRIBUTE_HREF                  "href"
#define ATTRIBUTE_MASKCOLOR             "maskcolor"
#define ATTRIBUTE_COMMAND               "command"
#define ATTRIBUTE_BITMAPINDEX           "bitmap-index"
#define ATTRIBUTE_MASKURL               "maskurl"
#define ATTRIBUTE_MASKMODE              "maskmode"
#define ATTRIBUTE_HIGHCONTRASTURL       "highcontrasturl"
#define ATTRIBUTE_HIGHCONTRASTMASKURL   "highcontrastmaskurl"
#define ATTRIBUTE_TYPE_CDATA            "CDATA"

#define ATTRIBUTE_MASKMODE_BITMAP   "maskbitmap"
#define ATTRIBUTE_MASKMODE_COLOR    "maskcolor"

#define ATTRIBUTE_XMLNS_IMAGE       "xmlns:image"
#define ATTRIBUTE_XMLNS_XLINK       "xmlns:xlink"

#define ATTRIBUTE_XLINK_TYPE        "xlink:type"
#define ATTRIBUTE_XLINK_TYPE_VALUE  "simple"

#define XMLNS_IMAGE                 "http://openoffice.org/2001/image"
#define XMLNS_XLINK                 "http://www.w3.org/1999/xlink"
#define XMLNS_IMAGE_PREFIX          "image:"
#define XMLNS_XLINK_PREFIX          "xlink:"

#define XMLNS_FILTER_SEPARATOR      "^"

#define IMAGES_DOCTYPE  "<!DOCTYPE image:imagecontainer PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"image.dtd\">"

namespace framework
{

struct ImageXMLEntryProperty
{
    OReadImagesDocumentHandler::Image_XML_Namespace nNamespace;
    char                                            aEntryName[20];
};

ImageXMLEntryProperty ImagesEntries[OReadImagesDocumentHandler::IMG_XML_ENTRY_COUNT] =
{
    { OReadImagesDocumentHandler::IMG_NS_IMAGE, ELEMENT_IMAGECONTAINER          },
    { OReadImagesDocumentHandler::IMG_NS_IMAGE, ELEMENT_IMAGES                  },
    { OReadImagesDocumentHandler::IMG_NS_IMAGE, ELEMENT_ENTRY                   },
    { OReadImagesDocumentHandler::IMG_NS_IMAGE, ELEMENT_EXTERNALIMAGES          },
    { OReadImagesDocumentHandler::IMG_NS_IMAGE, ELEMENT_EXTERNALENTRY           },
    { OReadImagesDocumentHandler::IMG_NS_XLINK, ATTRIBUTE_HREF                  },
    { OReadImagesDocumentHandler::IMG_NS_IMAGE, ATTRIBUTE_MASKCOLOR             },
    { OReadImagesDocumentHandler::IMG_NS_IMAGE, ATTRIBUTE_COMMAND               },
    { OReadImagesDocumentHandler::IMG_NS_IMAGE, ATTRIBUTE_BITMAPINDEX           },
    { OReadImagesDocumentHandler::IMG_NS_IMAGE, ATTRIBUTE_MASKURL               },
    { OReadImagesDocumentHandler::IMG_NS_IMAGE, ATTRIBUTE_MASKMODE              },
    { OReadImagesDocumentHandler::IMG_NS_IMAGE, ATTRIBUTE_HIGHCONTRASTURL       },
    { OReadImagesDocumentHandler::IMG_NS_IMAGE, ATTRIBUTE_HIGHCONTRASTMASKURL   }
};

OReadImagesDocumentHandler::OReadImagesDocumentHandler( ImageListsDescriptor& aItems ) :
    m_aImageList( aItems ),
    m_pImages( nullptr ),
    m_pExternalImages( nullptr )
{
    m_aImageList.pImageList         = nullptr;
    m_aImageList.pExternalImageList = nullptr;

    m_nHashMaskModeBitmap   = OUString( ATTRIBUTE_MASKMODE_BITMAP ).hashCode();
    m_nHashMaskModeColor    = OUString( ATTRIBUTE_MASKMODE_COLOR ).hashCode();

    // create hash map to speed up lookup
    for ( int i = 0; i < (int)IMG_XML_ENTRY_COUNT; i++ )
    {
        OUStringBuffer temp( 20 );

        if ( ImagesEntries[i].nNamespace == IMG_NS_IMAGE )
            temp.append( XMLNS_IMAGE );
        else
            temp.append( XMLNS_XLINK );

        temp.append( XMLNS_FILTER_SEPARATOR );
        temp.appendAscii( ImagesEntries[i].aEntryName );
        m_aImageMap.insert( ImageHashMap::value_type( temp.makeStringAndClear(), (Image_XML_Entry)i ) );
    }

    // reset states
    m_bImageContainerStartFound     = false;
    m_bImageContainerEndFound       = false;
    m_bImagesStartFound             = false;
    m_bImagesEndFound               = false;
    m_bImageStartFound              = false;
    m_bExternalImagesStartFound     = false;
    m_bExternalImagesEndFound       = false;
    m_bExternalImageStartFound      = false;
}

OReadImagesDocumentHandler::~OReadImagesDocumentHandler()
{
}

// XDocumentHandler
void SAL_CALL OReadImagesDocumentHandler::startDocument()
throw ( SAXException, RuntimeException, std::exception )
{
}

void SAL_CALL OReadImagesDocumentHandler::endDocument()
throw(  SAXException, RuntimeException, std::exception )
{
    SolarMutexGuard g;

    if (( m_bImageContainerStartFound && !m_bImageContainerEndFound ) ||
        ( !m_bImageContainerStartFound && m_bImageContainerEndFound )    )
    {
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += "No matching start or end element 'image:imagecontainer' found!";
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}

void SAL_CALL OReadImagesDocumentHandler::startElement(
    const OUString& aName, const Reference< XAttributeList > &xAttribs )
        throw(SAXException,
              RuntimeException,
              std::exception)
{
    SolarMutexGuard g;

    ImageHashMap::const_iterator pImageEntry = m_aImageMap.find( aName );
    if ( pImageEntry != m_aImageMap.end() )
    {
        switch ( pImageEntry->second )
        {
            case IMG_ELEMENT_IMAGECONTAINER:
            {
                // image:imagecontainer element (container element for all further image elements)
                if ( m_bImageContainerStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'image:imagecontainer' cannot be embedded into 'image:imagecontainer'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bImageContainerStartFound = true;
            }
            break;

            case IMG_ELEMENT_IMAGES:
            {
                if ( !m_bImageContainerStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'image:images' must be embedded into element 'image:imagecontainer'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_bImagesStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'image:images' cannot be embedded into 'image:images'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( !m_aImageList.pImageList )
                    m_aImageList.pImageList = new ImageListDescriptor;

                m_bImagesStartFound = true;
                m_pImages = new ImageListItemDescriptor;

                for ( sal_Int16 n = 0; n < xAttribs->getLength(); n++ )
                {
                    pImageEntry = m_aImageMap.find( xAttribs->getNameByIndex( n ) );
                    if ( pImageEntry != m_aImageMap.end() )
                    {
                        switch ( pImageEntry->second )
                        {
                            case IMG_ATTRIBUTE_HREF:
                            {
                                m_pImages->aURL = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case IMG_ATTRIBUTE_MASKCOLOR:
                            {
                                OUString aColor = xAttribs->getValueByIndex( n );

                                if ( aColor.startsWith("#") )
                                {
                                    // the color value is given as #rrggbb and used the hexadecimal system!!
                                    sal_uInt32 nColor = aColor.copy( 1 ).toUInt32( 16 );

                                    m_pImages->aMaskColor = Color( COLORDATA_RGB( nColor ) );
                                }
                            }
                            break;

                            case IMG_ATTRIBUTE_MASKURL:
                            {
                                m_pImages->aMaskURL = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case IMG_ATTRIBUTE_MASKMODE:
                            {
                                sal_Int32 nHashCode = xAttribs->getValueByIndex( n ).hashCode();
                                if ( nHashCode == m_nHashMaskModeBitmap )
                                    m_pImages->nMaskMode = ImageMaskMode_Bitmap;
                                else if ( nHashCode == m_nHashMaskModeColor )
                                    m_pImages->nMaskMode = ImageMaskMode_Color;
                                else
                                {
                                    delete m_pImages;
                                    m_pImages = nullptr;

                                    OUString aErrorMessage = getErrorLineString();
                                    aErrorMessage += "Attribute image:maskmode must be 'maskcolor' or 'maskbitmap'!";
                                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                                }
                            }
                            break;

                            case IMG_ATTRIBUTE_HIGHCONTRASTURL:
                            {
                                m_pImages->aHighContrastURL = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case IMG_ATTRIBUTE_HIGHCONTRASTMASKURL:
                            {
                                m_pImages->aHighContrastMaskURL = xAttribs->getValueByIndex( n );
                            }
                            break;

                                          default:
                                              break;
                        }
                    }
                } // for

                if ( m_pImages->aURL.isEmpty() )
                {
                    delete m_pImages;
                    m_pImages = nullptr;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Required attribute xlink:href must have a value!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }
            }
            break;

            case IMG_ELEMENT_ENTRY:
            {
                // Check that image:entry is embedded into image:images!
                if ( !m_bImagesStartFound )
                {
                    delete m_pImages;
                    m_pImages = nullptr;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'image:entry' must be embedded into element 'image:images'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( !m_pImages->pImageItemList )
                    m_pImages->pImageItemList = new ImageItemListDescriptor;

                m_bImageStartFound = true;

                // Create new image item descriptor
                std::unique_ptr<ImageItemDescriptor> pItem(new ImageItemDescriptor);
                pItem->nIndex = -1;

                // Read attributes for this image definition
                for ( sal_Int16 n = 0; n < xAttribs->getLength(); n++ )
                {
                    pImageEntry = m_aImageMap.find( xAttribs->getNameByIndex( n ) );
                    if ( pImageEntry != m_aImageMap.end() )
                    {
                        switch ( pImageEntry->second )
                        {
                            case IMG_ATTRIBUTE_COMMAND:
                            {
                                pItem->aCommandURL  = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case IMG_ATTRIBUTE_BITMAPINDEX:
                            {
                                pItem->nIndex       = xAttribs->getValueByIndex( n ).toInt32();
                            }
                            break;

                                          default:
                                              break;
                        }
                    }
                }

                // Check required attribute "bitmap-index"
                if ( pItem->nIndex < 0 )
                {
                    delete m_pImages;
                    m_pImages = nullptr;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Required attribute 'image:bitmap-index' must have a value >= 0!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                // Check required attribute "command"
                if ( pItem->aCommandURL.isEmpty() )
                {
                    delete m_pImages;
                    m_pImages = nullptr;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Required attribute 'image:command' must have a value!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_pImages->pImageItemList->push_back( std::move(pItem) );
            }
            break;

            case IMG_ELEMENT_EXTERNALIMAGES:
            {
                // Check that image:externalimages is embedded into image:imagecontainer
                if ( !m_bImageContainerStartFound )
                {
                    delete m_pImages;
                    m_pImages = nullptr;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'image:externalimages' must be embedded into element 'image:imagecontainer'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                // Check that image:externalentry is NOT embedded into image:externalentry
                if ( m_bExternalImagesStartFound )
                {
                    delete m_pImages;
                    m_pImages = nullptr;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'image:externalimages' cannot be embedded into 'image:externalimages'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                // Create unique external image container
                m_bExternalImagesStartFound = true;
                m_pExternalImages = new ExternalImageItemListDescriptor;
            }
            break;

            case IMG_ELEMENT_EXTERNALENTRY:
            {
                if ( !m_bExternalImagesStartFound )
                {
                    delete m_pImages;
                    delete m_pExternalImages;
                    m_pImages = nullptr;
                    m_pExternalImages = nullptr;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'image:externalentry' must be embedded into 'image:externalimages'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_bExternalImageStartFound )
                {
                    delete m_pImages;
                    delete m_pExternalImages;
                    m_pImages = nullptr;
                    m_pExternalImages = nullptr;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'image:externalentry' cannot be embedded into 'image:externalentry'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bExternalImageStartFound = true;

                std::unique_ptr<ExternalImageItemDescriptor> pItem(new ExternalImageItemDescriptor);

                // Read attributes for this external image definition
                for ( sal_Int16 n = 0; n < xAttribs->getLength(); n++ )
                {
                    pImageEntry = m_aImageMap.find( xAttribs->getNameByIndex( n ) );
                    if ( pImageEntry != m_aImageMap.end() )
                    {
                        switch ( pImageEntry->second )
                        {
                            case IMG_ATTRIBUTE_COMMAND:
                            {
                                pItem->aCommandURL  = xAttribs->getValueByIndex( n );
                            }
                            break;

                            case IMG_ATTRIBUTE_HREF:
                            {
                                pItem->aURL         = xAttribs->getValueByIndex( n );
                            }
                            break;

                            default:
                            break;
                        }
                    }
                }

                // Check required attribute "command"
                if ( pItem->aCommandURL.isEmpty() )
                {
                    delete m_pImages;
                    delete m_pExternalImages;
                    m_pImages = nullptr;
                    m_pExternalImages = nullptr;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Required attribute 'image:command' must have a value!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                // Check required attribute "href"
                if ( pItem->aURL.isEmpty() )
                {
                    delete m_pImages;
                    delete m_pExternalImages;
                    m_pImages = nullptr;
                    m_pExternalImages = nullptr;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Required attribute 'xlink:href' must have a value!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_pExternalImages )
                    m_pExternalImages->push_back( std::move(pItem) );
            }
            break;

            default:
            break;
        }
    }
}

void SAL_CALL OReadImagesDocumentHandler::endElement(const OUString& aName)
    throw(SAXException,
          RuntimeException,
          std::exception)
{
    SolarMutexGuard g;

    ImageHashMap::const_iterator pImageEntry = m_aImageMap.find( aName );
    if ( pImageEntry != m_aImageMap.end() )
    {
        switch ( pImageEntry->second )
        {
            case IMG_ELEMENT_IMAGECONTAINER:
            {
                m_bImageContainerEndFound = true;
            }
            break;

            case IMG_ELEMENT_IMAGES:
            {
                if ( m_pImages )
                {
                    if ( m_aImageList.pImageList )
                        m_aImageList.pImageList->push_back( std::unique_ptr<ImageListItemDescriptor>(m_pImages) );
                    m_pImages = nullptr;
                }
                m_bImagesStartFound = false;
            }
            break;

            case IMG_ELEMENT_ENTRY:
            {
                m_bImageStartFound = false;
            }
            break;

            case IMG_ELEMENT_EXTERNALIMAGES:
            {
                if ( m_pExternalImages && !m_aImageList.pExternalImageList )
                {
                    if ( !m_aImageList.pExternalImageList )
                        m_aImageList.pExternalImageList = m_pExternalImages;
                }

                m_bExternalImagesStartFound = false;
                m_pExternalImages = nullptr;
            }
            break;

            case IMG_ELEMENT_EXTERNALENTRY:
            {
                m_bExternalImageStartFound = false;
            }
            break;

                  default:
                      break;
        }
    }
}

void SAL_CALL OReadImagesDocumentHandler::characters(const OUString&)
throw(  SAXException, RuntimeException, std::exception )
{
}

void SAL_CALL OReadImagesDocumentHandler::ignorableWhitespace(const OUString&)
throw(  SAXException, RuntimeException, std::exception )
{
}

void SAL_CALL OReadImagesDocumentHandler::processingInstruction(
    const OUString& /*aTarget*/, const OUString& /*aData*/ )
throw(  SAXException, RuntimeException, std::exception )
{
}

void SAL_CALL OReadImagesDocumentHandler::setDocumentLocator(
    const Reference< XLocator > &xLocator)
throw(  SAXException, RuntimeException, std::exception )
{
    SolarMutexGuard g;
    m_xLocator = xLocator;
}

OUString OReadImagesDocumentHandler::getErrorLineString()
{
    SolarMutexGuard g;
    if ( m_xLocator.is() )
    {
        OUStringBuffer buffer("Line: ");
        buffer.append(m_xLocator->getLineNumber());
        buffer.append(" - ");
        return buffer.makeStringAndClear();
    }
    else
        return OUString();
}

//  OWriteImagesDocumentHandler

OWriteImagesDocumentHandler::OWriteImagesDocumentHandler(
    const ImageListsDescriptor& aItems,
    Reference< XDocumentHandler > rWriteDocumentHandler ) :
    m_aImageListsItems( aItems ),
    m_xWriteDocumentHandler( rWriteDocumentHandler )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    m_xEmptyList.set( static_cast<XAttributeList *>(pList), UNO_QUERY );
    m_aAttributeType        = ATTRIBUTE_TYPE_CDATA;
    m_aXMLImageNS           = XMLNS_IMAGE_PREFIX;
    m_aXMLXlinkNS           = XMLNS_XLINK_PREFIX;
    m_aAttributeXlinkType   = ATTRIBUTE_XLINK_TYPE;
    m_aAttributeValueSimple = ATTRIBUTE_XLINK_TYPE_VALUE;
}

OWriteImagesDocumentHandler::~OWriteImagesDocumentHandler()
{
}

void OWriteImagesDocumentHandler::WriteImagesDocument() throw
( SAXException, RuntimeException )
{
    SolarMutexGuard g;

    m_xWriteDocumentHandler->startDocument();

    // write DOCTYPE line!
    Reference< XExtendedDocumentHandler > xExtendedDocHandler( m_xWriteDocumentHandler, UNO_QUERY );
    if ( xExtendedDocHandler.is() )
    {
        xExtendedDocHandler->unknown( IMAGES_DOCTYPE );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    }

    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( static_cast<XAttributeList *>(pList) , UNO_QUERY );

    pList->AddAttribute( ATTRIBUTE_XMLNS_IMAGE,
                         m_aAttributeType,
                         XMLNS_IMAGE );

    pList->AddAttribute( ATTRIBUTE_XMLNS_XLINK,
                         m_aAttributeType,
                         XMLNS_XLINK );

    m_xWriteDocumentHandler->startElement( ELEMENT_NS_IMAGESCONTAINER, pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    if ( m_aImageListsItems.pImageList )
    {
        ImageListDescriptor* pImageList = m_aImageListsItems.pImageList;

        for ( size_t i = 0; i < m_aImageListsItems.pImageList->size(); i++ )
        {
            const ImageListItemDescriptor* pImageItems = (*pImageList)[i].get();
            WriteImageList( pImageItems );
        }
    }

    if ( m_aImageListsItems.pExternalImageList )
    {
        WriteExternalImageList( m_aImageListsItems.pExternalImageList );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( ELEMENT_NS_IMAGESCONTAINER );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endDocument();
}

//  protected member functions

void OWriteImagesDocumentHandler::WriteImageList( const ImageListItemDescriptor* pImageList ) throw
( SAXException, RuntimeException )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( static_cast<XAttributeList *>(pList) , UNO_QUERY );

    // save required attributes
    pList->AddAttribute( m_aAttributeXlinkType,
                         m_aAttributeType,
                         m_aAttributeValueSimple );

    pList->AddAttribute( m_aXMLXlinkNS + ATTRIBUTE_HREF,
                         m_aAttributeType,
                         pImageList->aURL );

    if ( pImageList->nMaskMode == ImageMaskMode_Bitmap )
    {
        pList->AddAttribute( m_aXMLImageNS + ATTRIBUTE_MASKMODE,
                             m_aAttributeType,
                             ATTRIBUTE_MASKMODE_BITMAP );

        pList->AddAttribute( m_aXMLImageNS + ATTRIBUTE_MASKURL,
                             m_aAttributeType,
                             pImageList->aMaskURL );

        if ( !pImageList->aHighContrastMaskURL.isEmpty() )
        {
            pList->AddAttribute( m_aXMLImageNS + ATTRIBUTE_HIGHCONTRASTMASKURL,
                                 m_aAttributeType,
                                 pImageList->aHighContrastMaskURL );
        }
    }
    else
    {
        OUStringBuffer   aColorStrBuffer( 8 );
        sal_Int64       nValue = pImageList->aMaskColor.GetRGBColor();

        aColorStrBuffer.append( "#" );
        aColorStrBuffer.append( OUString::number( nValue, 16 ));

        pList->AddAttribute( m_aXMLImageNS + ATTRIBUTE_MASKCOLOR,
                             m_aAttributeType,
                             aColorStrBuffer.makeStringAndClear() );

        pList->AddAttribute( m_aXMLImageNS + ATTRIBUTE_MASKMODE,
                             m_aAttributeType,
                             ATTRIBUTE_MASKMODE_COLOR );
    }

    if ( !pImageList->aHighContrastURL.isEmpty() )
    {
        pList->AddAttribute( m_aXMLImageNS + ATTRIBUTE_HIGHCONTRASTURL,
                             m_aAttributeType,
                             pImageList->aHighContrastURL );
    }

    m_xWriteDocumentHandler->startElement( ELEMENT_NS_IMAGES, xList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    ImageItemListDescriptor* pImageItemList = pImageList->pImageItemList;
    if ( pImageItemList )
    {
        for ( size_t i = 0; i < pImageItemList->size(); i++ )
            WriteImage( (*pImageItemList)[i].get() );
    }

    m_xWriteDocumentHandler->endElement( ELEMENT_NS_IMAGES );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
}

void OWriteImagesDocumentHandler::WriteImage( const ImageItemDescriptor* pImage ) throw
( SAXException, RuntimeException )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( static_cast<XAttributeList *>(pList) , UNO_QUERY );

    pList->AddAttribute( m_aXMLImageNS + ATTRIBUTE_BITMAPINDEX,
                         m_aAttributeType,
                         OUString::number( pImage->nIndex ) );

    pList->AddAttribute( m_aXMLImageNS + ATTRIBUTE_COMMAND,
                         m_aAttributeType,
                         pImage->aCommandURL );

    m_xWriteDocumentHandler->startElement( ELEMENT_NS_ENTRY, xList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    m_xWriteDocumentHandler->endElement( ELEMENT_NS_ENTRY );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
}

void OWriteImagesDocumentHandler::WriteExternalImageList( const ExternalImageItemListDescriptor* pExternalImageList ) throw
( SAXException, RuntimeException )
{
    m_xWriteDocumentHandler->startElement( ELEMENT_NS_EXTERNALIMAGES, m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    for ( size_t i = 0; i < pExternalImageList->size(); i++ )
    {
        const ExternalImageItemDescriptor* pItem = (*pExternalImageList)[i].get();
        WriteExternalImage( pItem );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( ELEMENT_NS_EXTERNALIMAGES );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
}

void OWriteImagesDocumentHandler::WriteExternalImage( const ExternalImageItemDescriptor* pExternalImage ) throw
( SAXException, RuntimeException )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( static_cast<XAttributeList *>(pList) , UNO_QUERY );

    // save required attributes
    pList->AddAttribute( m_aAttributeXlinkType,
                         m_aAttributeType,
                         m_aAttributeValueSimple );

    if ( !pExternalImage->aURL.isEmpty() )
    {
        pList->AddAttribute( m_aXMLXlinkNS + ATTRIBUTE_HREF,
                             m_aAttributeType,
                             pExternalImage->aURL );
    }

    if ( !pExternalImage->aCommandURL.isEmpty() )
    {
        pList->AddAttribute( m_aXMLImageNS + ATTRIBUTE_COMMAND,
                             m_aAttributeType,
                             pExternalImage->aCommandURL );
    }

    m_xWriteDocumentHandler->startElement( ELEMENT_NS_EXTERNALENTRY, xList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    m_xWriteDocumentHandler->endElement( ELEMENT_NS_EXTERNALENTRY );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
