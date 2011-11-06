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



#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/XStream.hpp>

#include <rtl/ustring.hxx>
class SvMemoryStream;
class GDIMetaFile;
class BitmapEx;

class GraphicHelper
{

    static sal_Bool mergeBitmaps_Impl( const BitmapEx& rBmpEx, const BitmapEx& rOverlay,
                   const Rectangle& rOverlayRect, BitmapEx& rReturn );

    static sal_Bool createThumb_Impl( const GDIMetaFile& rMtf,
                       sal_uInt32 nMaximumExtent,
                       BitmapEx& rBmpEx,
                       const BitmapEx* pOverlay = NULL,
                       const Rectangle* pOverlayRect = NULL );

public:

    static SvMemoryStream* getFormatStrFromGDI_Impl( const GDIMetaFile* pGDIMeta, sal_uInt32 nFormat );

    static void* getEnhMetaFileFromGDI_Impl( const GDIMetaFile* pGDIMeta );

    static void* getWinMetaFileFromGDI_Impl( const GDIMetaFile* pGDIMeta, const Size& aMetaSize );

    static sal_Bool supportsMetaFileHandle_Impl();

    static sal_Bool getThumbnailFormatFromGDI_Impl(
            GDIMetaFile* pMetaFile,
            sal_Bool bSigned,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xStream );

    static sal_Bool getSignedThumbnailFormatFromBitmap_Impl(
            const BitmapEx& aBitmap,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xStream );

    static sal_uInt16 getThumbnailReplacementIDByFactoryName_Impl( const ::rtl::OUString& aFactoryShortName,
                                                                    sal_Bool bIsTemplate );

    static sal_Bool getThumbnailReplacement_Impl(
            sal_Int32 nResID,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xStream );

};

