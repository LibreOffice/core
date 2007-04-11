/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: image.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:56:38 $
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

#ifndef _SV_IMAGE_HXX
#define _SV_IMAGE_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#include <vector>

struct ImplImage;
struct ImplImageList;
namespace com { namespace sun { namespace star { namespace graphic { class XGraphic;} } } }

// -----------
// - Defines -
// -----------

#define IMAGE_STDBTN_COLOR          Color( 0xC0, 0xC0, 0xC0 )
#define IMAGELIST_IMAGE_NOTFOUND    ((USHORT)0xFFFF)

// -----------------------
// - ImageColorTransform -
// -----------------------

enum ImageColorTransform
{
    IMAGECOLORTRANSFORM_NONE = 0,
    IMAGECOLORTRANSFORM_HIGHCONTRAST = 1,
    IMAGECOLORTRANSFORM_MONOCHROME_BLACK = 2,
    IMAGECOLORTRANSFORM_MONOCHROME_WHITE = 3
};

// ---------
// - Image -
// ---------

class VCL_DLLPUBLIC Image
{
    friend class ImageList;
    friend class OutputDevice;

public:
                    Image();
                    Image( const ResId& rResId );
                    Image( const Image& rImage );
                    Image( const BitmapEx& rBitmapEx );
                    Image( const Bitmap& rBitmap );
                    Image( const Bitmap& rBitmap, const Bitmap& rMaskBitmap );
                    Image( const Bitmap& rBitmap, const Color& rColor );
                    Image( const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& rxGraphic );
                    ~Image();

    Size            GetSizePixel() const;

    BitmapEx        GetBitmapEx() const;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > GetXGraphic() const;

    Image           GetColorTransformedImage( ImageColorTransform eColorTransform ) const;
    static void     GetColorTransformArrays( ImageColorTransform eColorTransform, Color*& rpSrcColor, Color*& rpDstColor, ULONG& rColorCount );

    void            Invert();

    BOOL            operator!() const { return( !mpImplData ? true : false ); }
    Image&          operator=( const Image& rImage );
    BOOL            operator==( const Image& rImage ) const;
    BOOL            operator!=( const Image& rImage ) const { return !(Image::operator==( rImage )); }

private:

    ImplImage*             mpImplData;

    SAL_DLLPRIVATE void    ImplInit( const BitmapEx& rBmpEx );
};

// -------------
// - ImageList -
// -------------

class VCL_DLLPUBLIC ImageList
{
public:
                    ImageList( USHORT nInit = 8, USHORT nGrow = 4 );
                    ImageList( const ResId& rResId );
                    ImageList( const ::std::vector< ::rtl::OUString >& rNameVector,
                               const ::rtl::OUString& rPrefix,
                               const Color* pMaskColor = NULL );
                    ImageList( const ImageList& rImageList );
                    ImageList( const BitmapEx& rBitmapEx,
                               USHORT nInitSize,
                               USHORT* pIdAry = NULL,
                               USHORT nGrow = 4 );
                    ImageList( const BitmapEx& rBitmapEx,
                               const ::std::vector< ::rtl::OUString >& rNameVector,
                               USHORT nGrow = 4 );

                    ImageList( const Bitmap& rBitmap,
                               USHORT nInitSize, USHORT* pIdAry = NULL,
                               USHORT nGrow = 4 );
                    ImageList( const Bitmap& rBitmap, const Bitmap& rMaskBmp,
                               USHORT nInitSize, USHORT* pIdAry = NULL,
                               USHORT nGrow = 4 );
                    ImageList( const Bitmap& rBitmap, const Color& rColor,
                               USHORT nInitSize, USHORT* pIdAry = NULL,
                               USHORT nGrow = 4 );
                    ~ImageList();

    void            Clear();

    USHORT          GetImageCount() const;
    Size            GetImageSize() const;
    BitmapEx        GetBitmapEx() const;
    ImageList       GetColorTransformedImageList( ImageColorTransform eColorTransform ) const;
       void            Invert();

    void            AddImage( USHORT nNewId, const Image& rImage );
    void            AddImage( const ::rtl::OUString& rImageName, const Image& rImage );

    void            CopyImage( USHORT nNewId, USHORT nCopyId );
    void            CopyImage( const ::rtl::OUString& rImageName, const ::rtl::OUString& rCopyName );

    void            ReplaceImage( USHORT nId, const Image& rImage );
    void            ReplaceImage( const ::rtl::OUString& rImageName, const Image& rImage );

    void            ReplaceImage( USHORT nId, USHORT nReplaceId );
    void            ReplaceImage( const ::rtl::OUString& rImageName, const ::rtl::OUString& rReplaceName );

    void            RemoveImage( USHORT nId );
    void            RemoveImage( const ::rtl::OUString& rImageName );

    Image           GetImage( USHORT nId ) const;
    Image           GetImage( const ::rtl::OUString& rImageName ) const;

    USHORT          GetImagePos( USHORT nId ) const;
    USHORT          GetImagePos( const ::rtl::OUString& rImageName ) const;

    USHORT          GetImageId( USHORT nPos ) const;
    void            GetImageIds( ::std::vector< USHORT >& rIds ) const;

    ::rtl::OUString GetImageName( USHORT nPos ) const;
    void            GetImageNames( ::std::vector< ::rtl::OUString >& rNames ) const;

    ImageList&      operator=( const ImageList& rImageList );
    BOOL            operator==( const ImageList& rImageList ) const;
    BOOL            operator!=( const ImageList& rImageList ) const { return !(ImageList::operator==( rImageList )); }

    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStream, ImageList& rImageList );
    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStream, const ImageList& rImageList );

private:

    ImplImageList*  mpImplData;
    USHORT          mnInitSize;
    USHORT          mnGrowSize;

    SAL_DLLPRIVATE void    ImplInitBitmapEx( const ::rtl::OUString& rUserImageName,
                                             const ::std::vector< ::rtl::OUString >& rImageNames,
                                             const ::rtl::OUString& rSymbolsStyle,
                                             BitmapEx& rBmpEx,
                                             const Color* pMaskColor ) const;
    SAL_DLLPRIVATE void    ImplInit( const BitmapEx& rBitmapEx, USHORT nInit, const USHORT* pIdAry, const ::std::vector< ::rtl::OUString >* pNames, USHORT nGrow );
    SAL_DLLPRIVATE USHORT  ImplGetImageId( const ::rtl::OUString& rImageName ) const;
    SAL_DLLPRIVATE void    ImplMakeUnique();
};

#endif  // _SV_IMAGE_HXX
