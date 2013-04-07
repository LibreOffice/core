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

    static sal_uInt16 getThumbnailReplacementIDByFactoryName_Impl( const OUString& aFactoryShortName,
                                                                    sal_Bool bIsTemplate );

    static sal_Bool getThumbnailReplacement_Impl(
            sal_Int32 nResID,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xStream );

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
