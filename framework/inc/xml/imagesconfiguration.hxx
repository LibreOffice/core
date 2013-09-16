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

#ifndef FRAMEWORK_XML_IMAGESCONFIGURATION_HXX_
#define FRAMEWORK_XML_IMAGESCONFIGURATION_HXX_

#include <framework/fwedllapi.h>
#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <tools/color.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

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

    OUString  aCommandURL;                // URL command to dispatch
    long      nIndex;                     // index of the bitmap inside the bitmaplist
};

struct ExternalImageItemDescriptor
{
    OUString  aCommandURL;                // URL command to dispatch
    OUString  aURL;                       // a URL to an external bitmap
};

typedef boost::ptr_vector<ImageItemDescriptor> ImageItemListDescriptor;

typedef boost::ptr_vector<ExternalImageItemDescriptor> ExternalImageItemListDescriptor;

struct ImageListItemDescriptor
{
    ImageListItemDescriptor() : nMaskMode( ImageMaskMode_Color ),
                                pImageItemList( 0 ) {}

    ~ImageListItemDescriptor() { delete pImageItemList; }

    OUString                    aURL;               // an URL to a bitmap with several images inside
    Color                       aMaskColor;         // a color used as transparent
    OUString                    aMaskURL;           // an URL to an optional bitmap used as a mask
    ImageMaskMode               nMaskMode;            // an enum to describe the current mask mode
    ImageItemListDescriptor*    pImageItemList;       // an array of ImageItemDescriptors that describes every image
    OUString                    aHighContrastURL;       // an URL to an optional high contrast bitmap with serveral images inside
    OUString                    aHighContrastMaskURL;   // an URL to an optional high contrast bitmap as a mask
};

typedef boost::ptr_vector<ImageListItemDescriptor> ImageListDescriptor;

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
        static sal_Bool LoadImages(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rInputStream,
            ImageListsDescriptor& rItems );

        static sal_Bool StoreImages(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rOutputStream,
            const ImageListsDescriptor& rItems );
};

} // namespace framework

#endif // __FRAMEWORK_CLASSES_IMAGES

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
