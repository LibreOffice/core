/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: graphhelp.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:43:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/uno/XStream.hpp>
#endif

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

