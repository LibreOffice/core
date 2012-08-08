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
#include <xml/imagesdocumenthandler.hxx>
#include <macros/debug.hxx>

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

    m_nHashMaskModeBitmap   = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKMODE_BITMAP )).hashCode();
    m_nHashMaskModeColor    = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKMODE_COLOR )).hashCode();

    // create hash map to speed up lookup
    for ( int i = 0; i < (int)IMG_XML_ENTRY_COUNT; i++ )
    {
        ::rtl::OUStringBuffer temp( 20 );

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
        ::rtl::OUString aErrorMessage = getErrorLineString();
        aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "No matching start or end element 'image:imagecontainer' found!" ));
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}

void SAL_CALL OReadImagesDocumentHandler::startElement(
    const ::rtl::OUString& aName, const Reference< XAttributeList > &xAttribs )
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
                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:imagecontainer' cannot be embeded into 'image:imagecontainer'!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                m_bImageContainerStartFound = sal_True;
            }
            break;

            case IMG_ELEMENT_IMAGES:
            {
                if ( !m_bImageContainerStartFound )
                {
                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:images' must be embeded into element 'image:imagecontainer'!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_bImagesStartFound )
                {
                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:images' cannot be embeded into 'image:images'!" ));
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
                                ::rtl::OUString aColor = xAttribs->getValueByIndex( n );

                                if ( !aColor.isEmpty() )
                                {
                                    if ( aColor.getStr()[0] == '#' )
                                    {
                                        // the color value is given as #rrggbb and used the hexadecimal system!!
                                        sal_uInt32 nColor = aColor.copy( 1 ).toInt32( 16 );

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

                                    ::rtl::OUString aErrorMessage = getErrorLineString();
                                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Attribute image:maskmode must be 'maskcolor' or 'maskbitmap'!" ));
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

                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Required attribute xlink:href must have a value!" ));
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

                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:entry' must be embeded into element 'image:images'!" ));
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

                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Required attribute 'image:bitmap-index' must have a value >= 0!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                // Check required attribute "command"
                if ( pItem->aCommandURL.Len() == 0 )
                {
                    delete pItem;
                    delete m_pImages;
                    m_pImages = NULL;

                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Required attribute 'image:command' must have a value!" ));
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

                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:externalimages' must be embeded into element 'image:imagecontainer'!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                // Check that image:externalentry is NOT embeded into image:externalentry
                if ( m_bExternalImagesStartFound )
                {
                    delete m_pImages;
                    m_pImages = NULL;

                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:externalimages' cannot be embeded into 'image:externalimages'!" ));
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

                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:externalentry' must be embeded into 'image:externalimages'!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_bExternalImageStartFound )
                {
                    delete m_pImages;
                    delete m_pExternalImages;
                    m_pImages = NULL;
                    m_pExternalImages = NULL;

                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:externalentry' cannot be embeded into 'image:externalentry'!" ));
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

                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Required attribute 'image:command' must have a value!" ));
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

                    ::rtl::OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Required attribute 'xlink:href' must have a value!" ));
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

void SAL_CALL OReadImagesDocumentHandler::endElement(const ::rtl::OUString& aName)
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

void SAL_CALL OReadImagesDocumentHandler::characters(const ::rtl::OUString&)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadImagesDocumentHandler::ignorableWhitespace(const ::rtl::OUString&)
throw(  SAXException, RuntimeException )
{
}

void SAL_CALL OReadImagesDocumentHandler::processingInstruction(
    const ::rtl::OUString& /*aTarget*/, const ::rtl::OUString& /*aData*/ )
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

::rtl::OUString OReadImagesDocumentHandler::getErrorLineString()
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
    m_aAttributeType        = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_TYPE_CDATA ));
    m_aXMLImageNS           = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_IMAGE_PREFIX ));
    m_aXMLXlinkNS           = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK_PREFIX ));
    m_aAttributeXlinkType   = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XLINK_TYPE ));
    m_aAttributeValueSimple = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XLINK_TYPE_VALUE ));
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
        xExtendedDocHandler->unknown( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMAGES_DOCTYPE )) );
        m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
    }

    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    pList->AddAttribute( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XMLNS_IMAGE )),
                         m_aAttributeType,
                         ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_IMAGE )) );

    pList->AddAttribute( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XMLNS_XLINK )),
                         m_aAttributeType,
                         ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK )) );

    m_xWriteDocumentHandler->startElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_IMAGESCONTAINER )), pList );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );

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

    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
    m_xWriteDocumentHandler->endElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_IMAGESCONTAINER )) );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
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

    pList->AddAttribute( m_aXMLXlinkNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_HREF )),
                         m_aAttributeType,
                         pImageList->aURL );

    if ( pImageList->nMaskMode == ImageMaskMode_Bitmap )
    {
        pList->AddAttribute( m_aXMLImageNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKMODE )),
                             m_aAttributeType,
                             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKMODE_BITMAP )) );

        pList->AddAttribute( m_aXMLImageNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKURL )),
                             m_aAttributeType,
                             pImageList->aMaskURL );

        if ( pImageList->aHighContrastMaskURL.Len() > 0 )
        {
            pList->AddAttribute( m_aXMLImageNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_HIGHCONTRASTMASKURL )),
                                 m_aAttributeType,
                                 pImageList->aHighContrastMaskURL );
        }
    }
    else
    {
        ::rtl::OUStringBuffer   aColorStrBuffer( 8 );
        sal_Int64       nValue = pImageList->aMaskColor.GetRGBColor();

        aColorStrBuffer.appendAscii( "#" );
        aColorStrBuffer.append( ::rtl::OUString::valueOf( nValue, 16 ));

        pList->AddAttribute( m_aXMLImageNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKCOLOR )),
                             m_aAttributeType,
                             aColorStrBuffer.makeStringAndClear() );

        pList->AddAttribute( m_aXMLImageNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKMODE )),
                             m_aAttributeType,
                             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKMODE_COLOR )) );
    }

    if ( pImageList->aHighContrastURL.Len() > 0 )
    {
        pList->AddAttribute( m_aXMLImageNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_HIGHCONTRASTURL )),
                             m_aAttributeType,
                             pImageList->aHighContrastURL );
    }

    m_xWriteDocumentHandler->startElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_IMAGES )), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );

    ImageItemListDescriptor* pImageItemList = pImageList->pImageItemList;
    if ( pImageItemList )
    {
        for ( sal_uInt16 i = 0; i < pImageItemList->size(); i++ )
            WriteImage( &(*pImageItemList)[i] );
    }

    m_xWriteDocumentHandler->endElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_IMAGES )) );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
}

void OWriteImagesDocumentHandler::WriteImage( const ImageItemDescriptor* pImage ) throw
( SAXException, RuntimeException )
{
    ::comphelper::AttributeList* pList = new ::comphelper::AttributeList;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    pList->AddAttribute( m_aXMLImageNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_BITMAPINDEX )),
                         m_aAttributeType,
                         ::rtl::OUString::valueOf( (sal_Int32)pImage->nIndex ) );

    pList->AddAttribute( m_aXMLImageNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_COMMAND )),
                         m_aAttributeType,
                         pImage->aCommandURL );

    m_xWriteDocumentHandler->startElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_ENTRY )), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );

    m_xWriteDocumentHandler->endElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_ENTRY )) );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
}

void OWriteImagesDocumentHandler::WriteExternalImageList( const ExternalImageItemListDescriptor* pExternalImageList ) throw
( SAXException, RuntimeException )
{
    m_xWriteDocumentHandler->startElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_EXTERNALIMAGES )), m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );

    for ( sal_uInt16 i = 0; i < pExternalImageList->size(); i++ )
    {
        const ExternalImageItemDescriptor* pItem = &(*pExternalImageList)[i];
        WriteExternalImage( pItem );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
    m_xWriteDocumentHandler->endElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_EXTERNALIMAGES )) );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
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
        pList->AddAttribute( m_aXMLXlinkNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_HREF )),
                             m_aAttributeType,
                             pExternalImage->aURL );
    }

    if ( pExternalImage->aCommandURL.Len() > 0 )
    {
        pList->AddAttribute( m_aXMLImageNS + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_COMMAND )),
                             m_aAttributeType,
                             pExternalImage->aCommandURL );
    }

    m_xWriteDocumentHandler->startElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_EXTERNALENTRY )), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );

    m_xWriteDocumentHandler->endElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_EXTERNALENTRY )) );
    m_xWriteDocumentHandler->ignorableWhitespace( ::rtl::OUString() );
}

} // namespace framework




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
