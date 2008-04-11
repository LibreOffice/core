/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: imagesdocumenthandler.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <stdio.h>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/resetableguard.hxx>
#include <xml/imagesdocumenthandler.hxx>
#include <macros/debug.hxx>
#include <xml/attributelist.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef __COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <rtl/ustrbuf.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl;
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
    ::cppu::OWeakObject(),
    m_aImageList( aItems ),
    m_pImages( 0 ),
    m_pExternalImages( 0 )
{
    m_aImageList.pImageList         = NULL;
    m_aImageList.pExternalImageList = NULL;

    m_nHashMaskModeBitmap   = OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKMODE_BITMAP )).hashCode();
    m_nHashMaskModeColor    = OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKMODE_COLOR )).hashCode();

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

Any SAL_CALL OReadImagesDocumentHandler::queryInterface( const Type & rType )
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
        aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "No matching start or end element 'image:imagecontainer' found!" ));
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
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:imagecontainer' cannot be embeded into 'image:imagecontainer'!" ));
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
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:images' must be embeded into element 'image:imagecontainer'!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_bImagesStartFound )
                {
                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:images' cannot be embeded into 'image:images'!" ));
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

                                if ( aColor.getLength() > 0 )
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

                                    OUString aErrorMessage = getErrorLineString();
                                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Attribute image:maskmode must be 'maskcolor' or 'maskbitmap'!" ));
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
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Required attribute xlink:href must have a value!" ));
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
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:entry' must be embeded into element 'image:images'!" ));
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
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Required attribute 'image:bitmap-index' must have a value >= 0!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                // Check required attribute "command"
                if ( pItem->aCommandURL.Len() == 0 )
                {
                    delete pItem;
                    delete m_pImages;
                    m_pImages = NULL;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Required attribute 'image:command' must have a value!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_pImages )
                    m_pImages->pImageItemList->Insert( pItem, m_pImages->pImageItemList->Count() );
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
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:externalimages' must be embeded into element 'image:imagecontainer'!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                // Check that image:externalentry is NOT embeded into image:externalentry
                if ( m_bExternalImagesStartFound )
                {
                    delete m_pImages;
                    m_pImages = NULL;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:externalimages' cannot be embeded into 'image:externalimages'!" ));
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
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:externalentry' must be embeded into 'image:externalimages'!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_bExternalImageStartFound )
                {
                    delete m_pImages;
                    delete m_pExternalImages;
                    m_pImages = NULL;
                    m_pExternalImages = NULL;

                    OUString aErrorMessage = getErrorLineString();
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Element 'image:externalentry' cannot be embeded into 'image:externalentry'!" ));
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
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Required attribute 'image:command' must have a value!" ));
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
                    aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Required attribute 'xlink:href' must have a value!" ));
                    throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
                }

                if ( m_pExternalImages )
                    m_pExternalImages->Insert( pItem, m_pExternalImages->Count() );
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
                        m_aImageList.pImageList->Insert( m_pImages, m_aImageList.pImageList->Count() );
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

::rtl::OUString OReadImagesDocumentHandler::getErrorLineString()
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
//  OWriteImagesDocumentHandler
//_________________________________________________________________________________________________________________

OWriteImagesDocumentHandler::OWriteImagesDocumentHandler(
    const ImageListsDescriptor& aItems,
    Reference< XDocumentHandler > rWriteDocumentHandler ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_aImageListsItems( aItems ),
    m_xWriteDocumentHandler( rWriteDocumentHandler )
{
    m_xEmptyList            = Reference< XAttributeList >( (XAttributeList *)new AttributeListImpl, UNO_QUERY );
    m_aAttributeType        = OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_TYPE_CDATA ));
    m_aXMLImageNS           = OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_IMAGE_PREFIX ));
    m_aXMLXlinkNS           = OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK_PREFIX ));
    m_aAttributeXlinkType   = OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XLINK_TYPE ));
    m_aAttributeValueSimple = OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XLINK_TYPE_VALUE ));
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
        xExtendedDocHandler->unknown( OUString( RTL_CONSTASCII_USTRINGPARAM( IMAGES_DOCTYPE )) );
        m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    }

    AttributeListImpl* pList = new AttributeListImpl;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XMLNS_IMAGE )),
                         m_aAttributeType,
                         OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_IMAGE )) );

    pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_XMLNS_XLINK )),
                         m_aAttributeType,
                         OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK )) );

    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_IMAGESCONTAINER )), pList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    if ( m_aImageListsItems.pImageList )
    {
        ImageListDescriptor* pImageList = m_aImageListsItems.pImageList;

        for ( USHORT i = 0; i < m_aImageListsItems.pImageList->Count(); i++ )
        {
            const ImageListItemDescriptor* pImageItems = (*pImageList)[i];
            WriteImageList( pImageItems );
        }
    }

    if ( m_aImageListsItems.pExternalImageList )
    {
        WriteExternalImageList( m_aImageListsItems.pExternalImageList );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_IMAGESCONTAINER )) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endDocument();
}

//_________________________________________________________________________________________________________________
//  protected member functions
//_________________________________________________________________________________________________________________

void OWriteImagesDocumentHandler::WriteImageList( const ImageListItemDescriptor* pImageList ) throw
( SAXException, RuntimeException )
{
    AttributeListImpl*          pList = new AttributeListImpl;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    // save required attributes
    pList->addAttribute( m_aAttributeXlinkType,
                         m_aAttributeType,
                         m_aAttributeValueSimple );

    pList->addAttribute( m_aXMLXlinkNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_HREF )),
                         m_aAttributeType,
                         pImageList->aURL );

    if ( pImageList->nMaskMode == ImageMaskMode_Bitmap )
    {
        pList->addAttribute( m_aXMLImageNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKMODE )),
                             m_aAttributeType,
                             OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKMODE_BITMAP )) );

        pList->addAttribute( m_aXMLImageNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKURL )),
                             m_aAttributeType,
                             pImageList->aMaskURL );

        if ( pImageList->aHighContrastMaskURL.Len() > 0 )
        {
            pList->addAttribute( m_aXMLImageNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_HIGHCONTRASTMASKURL )),
                                 m_aAttributeType,
                                 pImageList->aHighContrastMaskURL );
        }
    }
    else
    {
        OUStringBuffer  aColorStrBuffer( 8 );
        sal_Int64       nValue = pImageList->aMaskColor.GetRGBColor();

        aColorStrBuffer.appendAscii( "#" );
        aColorStrBuffer.append( OUString::valueOf( nValue, 16 ));

        pList->addAttribute( m_aXMLImageNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKCOLOR )),
                             m_aAttributeType,
                             aColorStrBuffer.makeStringAndClear() );

        pList->addAttribute( m_aXMLImageNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKMODE )),
                             m_aAttributeType,
                             OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MASKMODE_COLOR )) );
    }

    if ( pImageList->aHighContrastURL.Len() > 0 )
    {
        pList->addAttribute( m_aXMLImageNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_HIGHCONTRASTURL )),
                             m_aAttributeType,
                             pImageList->aHighContrastURL );
    }

    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_IMAGES )), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    ImageItemListDescriptor* pImageItemList = pImageList->pImageItemList;
    if ( pImageItemList )
    {
        for ( USHORT i = 0; i < pImageItemList->Count(); i++ )
            WriteImage( (*pImageItemList)[i] );
    }

    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_IMAGES )) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
}

void OWriteImagesDocumentHandler::WriteImage( const ImageItemDescriptor* pImage ) throw
( SAXException, RuntimeException )
{
    AttributeListImpl*          pList = new AttributeListImpl;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    pList->addAttribute( m_aXMLImageNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_BITMAPINDEX )),
                         m_aAttributeType,
                         OUString::valueOf( (sal_Int32)pImage->nIndex ) );

    pList->addAttribute( m_aXMLImageNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_COMMAND )),
                         m_aAttributeType,
                         pImage->aCommandURL );

    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_ENTRY )), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_ENTRY )) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
}

void OWriteImagesDocumentHandler::WriteExternalImageList( const ExternalImageItemListDescriptor* pExternalImageList ) throw
( SAXException, RuntimeException )
{
    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_EXTERNALIMAGES )), m_xEmptyList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    for ( USHORT i = 0; i < pExternalImageList->Count(); i++ )
    {
        ExternalImageItemDescriptor* pItem = (*pExternalImageList)[i];
        WriteExternalImage( pItem );
    }

    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_EXTERNALIMAGES )) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
}

void OWriteImagesDocumentHandler::WriteExternalImage( const ExternalImageItemDescriptor* pExternalImage ) throw
( SAXException, RuntimeException )
{
    AttributeListImpl*          pList = new AttributeListImpl;
    Reference< XAttributeList > xList( (XAttributeList *) pList , UNO_QUERY );

    // save required attributes
    pList->addAttribute( m_aAttributeXlinkType,
                         m_aAttributeType,
                         m_aAttributeValueSimple );

    if ( pExternalImage->aURL.Len() > 0 )
    {
        pList->addAttribute( m_aXMLXlinkNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_HREF )),
                             m_aAttributeType,
                             pExternalImage->aURL );
    }

    if ( pExternalImage->aCommandURL.Len() > 0 )
    {
        pList->addAttribute( m_aXMLImageNS + OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_COMMAND )),
                             m_aAttributeType,
                             pExternalImage->aCommandURL );
    }

    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_EXTERNALENTRY )), xList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_NS_EXTERNALENTRY )) );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
}

} // namespace framework
