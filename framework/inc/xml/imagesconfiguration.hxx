/*************************************************************************
 *
 *  $RCSfile: imagesconfiguration.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 16:54:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_CLASSES_IMAGESCONFIGURATION_HXX_
#define __FRAMEWORK_CLASSES_IMAGESCONFIGURATION_HXX_

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

// #110897#
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

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
    ImageListItemDescriptor() : pImageItemList( 0 )
                                , nMaskMode( ImageMaskMode_Color ) {}
    ~ImageListItemDescriptor() { delete pImageItemList; }

    String                      aURL;                   // an URL to a bitmap with several images inside
    Color                       aMaskColor;             // a color used as transparent
    String                      aMaskURL;               // an URL to an optional bitmap used as a mask
    ImageMaskMode               nMaskMode;              // an enum to describe the current mask mode
    ImageItemListDescriptor*    pImageItemList;         // an array of ImageItemDescriptors that describes every image
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

    ImageListDescriptor*                pImageList;
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
