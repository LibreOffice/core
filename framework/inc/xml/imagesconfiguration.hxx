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

#ifndef FRAMEWORK_XML_IMAGESCONFIGURATION_HXX_
#define FRAMEWORK_XML_IMAGESCONFIGURATION_HXX_

#include <framework/fwedllapi.h>
#include <svl/svarray.hxx>
#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <tools/color.hxx>

// #110897#
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <vector>

namespace framework
{

enum ImageMaskMode
{
    ImageMaskMode_Color,
    ImageMaskMode_Bitmap
};

struct ImageItemDescriptor
{
    ImageItemDescriptor() : nIndex( -1 ) {}

    String  aCommandURL;                // URL command to dispatch
    long    nIndex;                     // index of the bitmap inside the bitmaplist
};

struct ExternalImageItemDescriptor
{
    String  aCommandURL;                // URL command to dispatch
    String  aURL;                       // a URL to an external bitmap
};

typedef ImageItemDescriptor* ImageItemDescriptorPtr;
SV_DECL_PTRARR_DEL( ImageItemListDescriptor, ImageItemDescriptorPtr, 10, 2)

typedef ExternalImageItemDescriptor* ExternalImageItemDescriptorPtr;
SV_DECL_PTRARR_DEL( ExternalImageItemListDescriptor, ExternalImageItemDescriptorPtr, 10, 2)

struct ImageListItemDescriptor
{
    ImageListItemDescriptor() : nMaskMode( ImageMaskMode_Color ),
                                pImageItemList( 0 ) {}

    ~ImageListItemDescriptor() { delete pImageItemList; }

    String                      aURL;               // an URL to a bitmap with several images inside
    Color                       aMaskColor;         // a color used as transparent
    String                      aMaskURL;           // an URL to an optional bitmap used as a mask
    ImageMaskMode                     nMaskMode;            // an enum to describe the current mask mode
    ImageItemListDescriptor*              pImageItemList;       // an array of ImageItemDescriptors that describes every image
    String                      aHighContrastURL;       // an URL to an optional high contrast bitmap with serveral images inside
    String                      aHighContrastMaskURL;   // an URL to an optional high contrast bitmap as a mask
};

typedef ImageListItemDescriptor* ImageListItemDescriptorPtr;
SV_DECL_PTRARR_DEL( ImageListDescriptor, ImageListItemDescriptorPtr, 10, 2)

struct ImageListsDescriptor
{
    ImageListsDescriptor() : pImageList( 0 ),
                     pExternalImageList( 0 ) {}
    ~ImageListsDescriptor() { delete pImageList; delete pExternalImageList; }

    ImageListDescriptor*            pImageList;
    ExternalImageItemListDescriptor*    pExternalImageList;
};

class ImagesConfiguration
{
    public:
        // #110897#
        static sal_Bool LoadImages(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            SvStream& rInStream, ImageListsDescriptor& aItems );

        // #110897#
        static sal_Bool StoreImages(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            SvStream& rOutStream, const ImageListsDescriptor& aItems );

        static sal_Bool LoadImages(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rInputStream,
            ImageListsDescriptor& rItems );

        static sal_Bool StoreImages(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rOutputStream,
            const ImageListsDescriptor& rItems );
};

} // namespace framework

#endif // __FRAMEWORK_CLASSES_IMAGES
