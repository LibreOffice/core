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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
