/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
