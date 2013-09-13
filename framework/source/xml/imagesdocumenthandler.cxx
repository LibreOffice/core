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
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_aImageList( aItems ),
    m_pImages( 0 ),
    m_pExternalImages( 0 )
{
    m_aImageList.pImageList         = NULL;
    m_aImageList.pExternalImageList = NULL;

    m_nHashMaskModeBitmap   = OUString( ATTRIBUTE_MASKMODE_BITMAP ).hashCode();
    m_nHashMaskModeColor    = OUString( ATTRIBUTE_MASKMODE_COLOR ).hashCode();

    // create hash map to speed up lookup
    for ( int i = 0; i < (int)IMG_XML_ENTRY_COUNT; i++ )
    {
        OUStringBuffer temp( 20 );

        if ( ImagesEntries[i].nNamespace == IMG_NS_IMAGE )
            temp.appendAscii( XMLNS_IMAGE );
        else
            temp.appendAscii( XMLNS_XLINK );

        temp.appendAscii( XMLNS_FILTER_SEPARATOR );
        temp.appendAscii( ImagesEntries[i].aEntryName );
        m_aImageMap.insert( ImageHashMap::value_type( temp.makeStringAndClear(), (Image_XML_Entry)i ) );
    }

    // reset states
    m_bImageContainerStartFound     = sal_False;
    m_bImageContainerEndFound       = sal_False;
    m_bImagesStartFound             = sal_False;
    m_bImagesEndFound               = sal_False;
    m_bImageStartFound              = sal_False;
    m_bExternalImagesStartFound     = sal_False;
    m_bExternalImagesEndFound       = sal_False;
    m_bExternalImageStartFound      = sal_False;
}

OReadImagesDocumentHandler::~OReadImagesDocumentHandler()
{
}

// XDocumentHandler
void SAL_CALL OReadImagesDocumentHandler::startDocument(void)
throw ( SAXException, RuntimeException )
{
}

void SAL_CALL OReadImagesDocumentHandler::endDocument(void)
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

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
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    ImageHashMap::const_iterator pImageEntry = m_aImageMap.find( aName ) ;
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
                    aErrorMessage += "Element 'image:imagecontainer' cannot be embeded into 'image:imagecontainer'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bImageContainerStartFound = sal_True;
            }
            break;

            case IMG_ELEMENT_IMAGES:
            {
                if ( !m_bImageContainerStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'image:images' must be embeded into element 'image:imagecontainer'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_bImagesStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'image:images' cannot be embeded into 'image:images'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( !m_aImageList.pImageList )
                    m_aImageList.pImageList = new ImageListDescriptor;

                m_bImagesStartFound = sal_True;
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

                                if ( !aColor.isEmpty() )
                                {
                                    if ( aColor.getStr()[0] == '#' )
                                    {
                                        // the color value is given as #rrggbb and used the hexadecimal system!!
                                        sal_uInt32 nColor = aColor.copy( 1 ).toUInt32( 16 );

                                        m_pImages->aMaskColor = Color( COLORDATA_RGB( nColor ) );
                                    }
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
                                    m_pImages = NULL;

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

                if ( m_pImages->aURL.Len() == 0 )
                {
                    delete m_pImages;
                    m_pImages = NULL;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Required attribute xlink:href must have a value!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }
            }
            break;

            case IMG_ELEMENT_ENTRY:
            {
                // Check that image:entry is embeded into image:images!
                if ( !m_bImagesStartFound )
                {
                    delete m_pImages;
                    m_pImages = NULL;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'image:entry' must be embeded into element 'image:images'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( !m_pImages->pImageItemList )
                    m_pImages->pImageItemList = new ImageItemListDescriptor;

                m_bImageStartFound = sal_True;

                // Create new image item descriptor
                ImageItemDescriptor* pItem = new ImageItemDescriptor;
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
                    delete pItem;
                    delete m_pImages;
                    m_pImages = NULL;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Required attribute 'image:bitmap-index' must have a value >= 0!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                // Check required attribute "command"
                if ( pItem->aCommandURL.Len() == 0 )
                {
                    delete pItem;
                    delete m_pImages;
                    m_pImages = NULL;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Required attribute 'image:command' must have a value!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_pImages )
                    m_pImages->pImageItemList->push_back( pItem );
            }
            break;

            case IMG_ELEMENT_EXTERNALIMAGES:
            {
                // Check that image:externalimages is embeded into image:imagecontainer
                if ( !m_bImageContainerStartFound )
                {
                    delete m_pImages;
                    m_pImages = NULL;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'image:externalimages' must be embeded into element 'image:imagecontainer'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                // Check that image:externalentry is NOT embeded into image:externalentry
                if ( m_bExternalImagesStartFound )
                {
                    delete m_pImages;
                    m_pImages = NULL;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'image:externalimages' cannot be embeded into 'image:externalimages'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                // Create unique external image container
                m_bExternalImagesStartFound = sal_True;
                m_pExternalImages = new ExternalImageItemListDescriptor;
            }
            break;

            case IMG_ELEMENT_EXTERNALENTRY:
            {
                if ( !m_bExternalImagesStartFound )
                {
                    delete m_pImages;
                    delete m_pExternalImages;
                    m_pImages = NULL;
                    m_pExternalImages = NULL;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'image:externalentry' must be embeded into 'image:externalimages'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_bExternalImageStartFound )
                {
                    delete m_pImages;
                    delete m_pExternalImages;
                    m_pImages = NULL;
                    m_pExternalImages = NULL;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Element 'image:externalentry' cannot be embeded into 'image:externalentry'!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bExternalImageStartFound = sal_True;

                ExternalImageItemDescriptor* pItem = new ExternalImageItemDescriptor;

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
                if ( pItem->aCommandURL.Len() == 0 )
                {
                    delete pItem;
                    delete m_pImages;
                    delete m_pExternalImages;
                    m_pImages = NULL;
                    m_pExternalImages = NULL;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Required attribute 'image:command' must have a value!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                // Check required attribute "href"
                if ( pItem->aURL.Len() == 0 )
                {
                    delete pItem;
                    delete m_pImages;
                    delete m_pExternalImages;
                    m_pImages = NULL;
                    m_pExternalImages = NULL;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += "Required attribute 'xlink:href' must have a value!";
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_pExternalImages )
                    m_pExternalImages->push_back( pItem );
                else
                    delete pItem;
            }
            break;

                  default:
                      break;
        }
    }
}

void SAL_CALL OReadImagesDocumentHandler::endElement(const OUString& aName)
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    ImageHashMap::const_iterator pImageEntry = m_aImageMap.find( aName ) ;
    if ( pImageEntry != m_aImageMap.end() )
    {
        switch ( pImageEntry->second )
        {
            case IMG_ELEMENT_IMAGECONTAINER:
            {
                m_bImageContainerEndFound = sal_True;
            }
            break;

            case IMG_ELEMENT_IMAGES:
            {
                if ( m_pImages )
                {
                    if ( m_aImageList.pImageList )
                        m_aImageList.pImageList->push_back( m_pImages );
                    m_pImages = NULL;
                }
                m_bImagesStartFound = sal_False;
            }
            break;

            case IMG_ELEMENT_ENTRY:
            {
                m_bImageStartFound = sal_False;
            }
            break;

            case IMG_ELEMENT_EXTERNALIMAGES:
            {
                if ( m_pExternalImages && !m_aImageList.pExternalImageList )
                {
                    if ( !m_aImageList.pExternalImageList )
                        m_aImageList.pExternalImageList = m_pExternalImages;
                }

                m_bExternalImagesStartFound = sal_False;
                m_pExternalImages = NULL;
            }
            break;

            case IMG_ELEMENT_EXTERNALENTRY:
            {
                m_bExternalImageStartFound = sal_False;
            }
            break;

                  default:
                      break;
        }
    }
}

void SAL_CALL OReadImagesDocumentHandler::characters(const OUString&)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadImagesDocumentHandler::ignorableWhitespace(const OUString&)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadImagesDocumentHandler::processingInstruction(
    const OUString& /*aTarget*/, const OUString& /*aData*/ )
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadImagesDocumentHandler::setDocumentLocator(
    const Reference< XLocator > &xLocator)
throw(  SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    m_xLocator = xLocator;
}

OUString OReadImagesDocumentHandler::getErrorLineString()
{
    ResetableGuard aGuard( m_aLock );


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


//_________________________________________________________________________________________________________________
//  OWriteImagesDocumentHandler
//_________________________________________________________________________________________________________________

OWriteImagesDocumentHandler::OWriteImagesDocumentHandler(
    const ImageListsDescriptor& aItems,
    Reference< XDocumentHandler > rWriteDocumentHandler ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_aImageListsItems( aItems ),
    m_xWriteDocumentHandler( rWriteDocumentHandler )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    m_xEmptyList            = Reference< XAttributeList >( (XAttributeList *) pList, UNO_QUERY );
    m_aAttributeType        = OUString( ATTRIBUTE_TYPE_CDATA );
    m_aXMLImageNS           = OUString( XMLNS_IMAGE_PREFIX );
    m_aXMLXlinkNS           = OUString( XMLNS_XLINK_PREFIX );
    m_aAttributeXlinkType   = OUString( ATTRIBUTE_XLINK_TYPE );
    m_aAttributeValueSimple = OUString( ATTRIBUTE_XLINK_TYPE_VALUE );
}

OWriteImagesDocumentHandler::~OWriteImagesDocumentHandler()
{
}

void OWriteImagesDocumentHandler::WriteImagesDocument() throw
( SAXException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    m_xWriteDocumentHandler->startDocument();

    // write DOCTYPE line!
    Reference< XExtendedDocumentHandler > xExtendedDocHandler( m_xWriteDocumentHandler, UNO_QUERY );
    if ( xExtendedDocHandler.is() )
    {
        xExtendedDocHandler->unknown( OUString( IMAGES_DOCTYPE ) );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    }

    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    pList->AddAttribute( OUString( ATTRIBUTE_XMLNS_IMAGE),
                         m_aAttributeType,
                         OUString( XMLNS_IMAGE ) );

    pList->AddAttribute( OUString( ATTRIBUTE_XMLNS_XLINK ),
                         m_aAttributeType,
                         OUString( XMLNS_XLINK ) );

    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_IMAGESCONTAINER ), pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    if ( m_aImageListsItems.pImageList )
    {
        ImageListDescriptor* pImageList = m_aImageListsItems.pImageList;

        for ( sal_uInt16 i = 0; i < m_aImageListsItems.pImageList->size(); i++ )
        {
            const ImageListItemDescriptor* pImageItems = &(*pImageList)[i];
            WriteImageList( pImageItems );
        }
    }

    if ( m_aImageListsItems.pExternalImageList )
    {
        WriteExternalImageList( m_aImageListsItems.pExternalImageList );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_IMAGESCONTAINER ) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endDocument();
}

//_________________________________________________________________________________________________________________
//  protected member functions
//_________________________________________________________________________________________________________________

void OWriteImagesDocumentHandler::WriteImageList( const ImageListItemDescriptor* pImageList ) throw
( SAXException, RuntimeException )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    // save required attributes
    pList->AddAttribute( m_aAttributeXlinkType,
                         m_aAttributeType,
                         m_aAttributeValueSimple );

    pList->AddAttribute( m_aXMLXlinkNS + OUString( ATTRIBUTE_HREF ),
                         m_aAttributeType,
                         pImageList->aURL );

    if ( pImageList->nMaskMode == ImageMaskMode_Bitmap )
    {
        pList->AddAttribute( m_aXMLImageNS + OUString( ATTRIBUTE_MASKMODE ),
                             m_aAttributeType,
                             OUString( ATTRIBUTE_MASKMODE_BITMAP ) );

        pList->AddAttribute( m_aXMLImageNS + OUString( ATTRIBUTE_MASKURL ),
                             m_aAttributeType,
                             pImageList->aMaskURL );

        if ( pImageList->aHighContrastMaskURL.Len() > 0 )
        {
            pList->AddAttribute( m_aXMLImageNS + OUString( ATTRIBUTE_HIGHCONTRASTMASKURL ),
                                 m_aAttributeType,
                                 pImageList->aHighContrastMaskURL );
        }
    }
    else
    {
        OUStringBuffer   aColorStrBuffer( 8 );
        sal_Int64       nValue = pImageList->aMaskColor.GetRGBColor();

        aColorStrBuffer.appendAscii( "#" );
        aColorStrBuffer.append( OUString::number( nValue, 16 ));

        pList->AddAttribute( m_aXMLImageNS + OUString( ATTRIBUTE_MASKCOLOR ),
                             m_aAttributeType,
                             aColorStrBuffer.makeStringAndClear() );

        pList->AddAttribute( m_aXMLImageNS + OUString( ATTRIBUTE_MASKMODE ),
                             m_aAttributeType,
                             OUString( ATTRIBUTE_MASKMODE_COLOR ) );
    }

    if ( pImageList->aHighContrastURL.Len() > 0 )
    {
        pList->AddAttribute( m_aXMLImageNS + OUString( ATTRIBUTE_HIGHCONTRASTURL ),
                             m_aAttributeType,
                             pImageList->aHighContrastURL );
    }

    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_IMAGES ), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    ImageItemListDescriptor* pImageItemList = pImageList->pImageItemList;
    if ( pImageItemList )
    {
        for ( sal_uInt16 i = 0; i < pImageItemList->size(); i++ )
            WriteImage( &(*pImageItemList)[i] );
    }

    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_IMAGES ) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
}

void OWriteImagesDocumentHandler::WriteImage( const ImageItemDescriptor* pImage ) throw
( SAXException, RuntimeException )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    pList->AddAttribute( m_aXMLImageNS + OUString( ATTRIBUTE_BITMAPINDEX ),
                         m_aAttributeType,
                         OUString::number( pImage->nIndex ) );

    pList->AddAttribute( m_aXMLImageNS + OUString( ATTRIBUTE_COMMAND ),
                         m_aAttributeType,
                         pImage->aCommandURL );

    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_ENTRY ), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_ENTRY ) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
}

void OWriteImagesDocumentHandler::WriteExternalImageList( const ExternalImageItemListDescriptor* pExternalImageList ) throw
( SAXException, RuntimeException )
{
    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_EXTERNALIMAGES ), m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    for ( sal_uInt16 i = 0; i < pExternalImageList->size(); i++ )
    {
        const ExternalImageItemDescriptor* pItem = &(*pExternalImageList)[i];
        WriteExternalImage( pItem );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_EXTERNALIMAGES ) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
}

void OWriteImagesDocumentHandler::WriteExternalImage( const ExternalImageItemDescriptor* pExternalImage ) throw
( SAXException, RuntimeException )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    // save required attributes
    pList->AddAttribute( m_aAttributeXlinkType,
                         m_aAttributeType,
                         m_aAttributeValueSimple );

    if ( pExternalImage->aURL.Len() > 0 )
    {
        pList->AddAttribute( m_aXMLXlinkNS + OUString( ATTRIBUTE_HREF ),
                             m_aAttributeType,
                             pExternalImage->aURL );
    }

    if ( pExternalImage->aCommandURL.Len() > 0 )
    {
        pList->AddAttribute( m_aXMLImageNS + OUString( ATTRIBUTE_COMMAND ),
                             m_aAttributeType,
                             pExternalImage->aCommandURL );
    }

    m_xWriteDocumentHandler->startElement( OUString( ELEMENT_NS_EXTERNALENTRY ), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    m_xWriteDocumentHandler->endElement( OUString( ELEMENT_NS_EXTERNALENTRY ) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
}

} // namespace framework




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
