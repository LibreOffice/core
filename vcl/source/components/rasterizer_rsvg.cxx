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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/alpha.hxx>
#include <com/sun/star/graphic/XGraphicRasterizer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <cppuhelper/implbase2.hxx>
#include <osl/module.h>

#include <vector>

#if defined MACOSX
    #define VCL_RSVG_GOBJECT_LIBNAME    "libgobject-2.0.0.dylib"
    #define VCL_RSVG_CAIRO_LIBNAME      "libcairo.2.dylib"
    #define VCL_RSVG_LIBRSVG_LIBNAME    "librsvg-2.2.dylib"
#elif defined UNX
    #define VCL_RSVG_GOBJECT_LIBNAME    "libgobject-2.0.so"
    #define VCL_RSVG_CAIRO_LIBNAME      "libcairo.so"
    #define VCL_RSVG_LIBRSVG_LIBNAME    "librsvg-2.so"
#elif defined WNT
    #define VCL_RSVG_GOBJECT_LIBNAME    "libgobject-2.0-0.dll"
    #define VCL_RSVG_CAIRO_LIBNAME      "cairo.dll"
    #define VCL_RSVG_LIBRSVG_LIBNAME    "librsvg-2-2.dll"
#else
    #define VCL_RSVG_GOBJECT_LIBNAME    "nogobjectlib"
    #define VCL_RSVG_CAIRO_LIBNAME      "nocairolib"
    #define VCL_RSVG_LIBRSVG_LIBNAME    "nolibrsvglib"
#endif

#define VCL_RSVG_DEFAULT_DPI 72

using namespace ::com::sun::star;

// -----------------------------------------------------
// - external stuff for dynamic library function calls -
// -----------------------------------------------------

typedef int gboolean;
typedef unsigned char guint8;
typedef sal_Size gsize;
typedef void* gpointer;

struct GError;

enum cairo_format_t { CAIRO_FORMAT_ARGB32 = 0 };
enum cairo_status_t { CAIRO_STATUS_SUCCESS = 0 };

struct cairo_surface_t;
struct cairo_t;
struct cairo_matrix_t
{
    double xx; double yx;
    double xy; double yy;
    double x0; double y0;
};

struct RsvgHandle;
struct RsvgDimensionData
{
    int width;
    int height;
    double em;
    double ex;
};

namespace vcl
{
namespace rsvg
{
// -----------------
// - Uno functions -
// ----------------

uno::Sequence< ::rtl::OUString > Rasterizer_getSupportedServiceNames()
{
    static ::rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.graphic.GraphicRasterizer_RSVG" ) );
    static uno::Sequence< ::rtl::OUString > aServiceNames( &aServiceName, 1 );

    return( aServiceNames );
}

// -----------------------------------------------------------------------------

::rtl::OUString Rasterizer_getImplementationName()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "vcl::rsvg::Rasterizer" ) );
}

// ------------------
// - LibraryWrapper -
// ------------------

class LibraryWrapper
{
public:

    static LibraryWrapper& get();

    bool isValid() const { return( ( mpGObjectLib != NULL ) && ( mpCairoLib != NULL ) && ( mpRSVGLib != NULL ) ); }

    // G-Object
    gpointer g_object_unref( gpointer pointer ) { return( (*mp_g_object_unref)( pointer ) ); };

    // LibRSVG

    // Cairo
    cairo_surface_t* image_surface_create( cairo_format_t format, int width, int height ) { return( (*mp_image_surface_create)( format, width, height ) ); }
    void surface_destroy( cairo_surface_t* surface ) { (*mp_surface_destroy)( surface ); }
    cairo_status_t surface_status( cairo_surface_t* surface ) { return( (*mp_surface_status)( surface ) ); }
    cairo_t* create( cairo_surface_t* surface ) { return( (*mp_create)( surface ) ); }
    void destroy( cairo_t* cairo ) { (*mp_destroy )( cairo ); }
    void matrix_init_identity( cairo_matrix_t* matrix ){ (*mp_matrix_init_identity)( matrix ); }
    void matrix_translate( cairo_matrix_t* matrix, double nx, double ny ) { (*mp_matrix_translate)( matrix, nx, ny ); }
    void matrix_scale( cairo_matrix_t* matrix, double sx, double sy ) {( *mp_matrix_scale )( matrix, sx, sy ); }
    void matrix_rotate( cairo_matrix_t* matrix, double radians ) { ( *mp_matrix_rotate )( matrix, radians ); }
    void transform( cairo_t* cairo, cairo_matrix_t *matrix ) { (*mp_transform)( cairo, matrix ); }
    unsigned char* image_surface_get_data(cairo_surface_t* surface) { return( (*mp_image_surface_get_data)( surface ) ); }
    int image_surface_get_width(cairo_surface_t* surface) { return( (*mp_image_surface_get_width)( surface ) ); }
    int image_surface_get_height(cairo_surface_t* surface) { return( (*mp_image_surface_get_height)( surface ) ); }
    int image_surface_get_stride(cairo_surface_t* surface) { return( (*mp_image_surface_get_stride)( surface ) ); }

    // LibRSVG
    void rsvg_init() { (*mp_rsvg_init)(); }
    RsvgHandle* rsvg_handle_new_from_data( const guint8* data, gsize size, GError** error) { return( (*mp_rsvg_handle_new_from_data)( data, size, error ) ); }
    gboolean rsvg_handle_close( RsvgHandle* handle, GError** error ) { return( (*mp_rsvg_handle_close)( handle, error ) ); }
    void rsvg_handle_set_dpi_x_y( RsvgHandle* handle, double dpix, double dpiy ) { (*mp_rsvg_handle_set_dpi_x_y)( handle, dpix, dpiy ); }
    void rsvg_handle_get_dimensions( RsvgHandle* handle, RsvgDimensionData* dimensions ) { (*mp_rsvg_handle_get_dimensions)( handle, dimensions ); }
    gboolean rsvg_handle_render_cairo( RsvgHandle* handle, cairo_t* cairo ) { return( (*mp_rsvg_handle_render_cairo)( handle, cairo ) ); }

private:

    LibraryWrapper();

private:

    oslModule mpGObjectLib;
    oslModule mpCairoLib;
    oslModule mpRSVGLib;

    // GObject
    gpointer (*mp_g_object_unref)( gpointer );

    // Cairo
    cairo_surface_t* (*mp_image_surface_create)(cairo_format_t,int,int);
    void (*mp_surface_destroy )(cairo_surface_t*);
    cairo_status_t (*mp_surface_status)(cairo_surface_t*);
    cairo_t* (*mp_create)(cairo_surface_t*);
    void (*mp_destroy)(cairo_t*);
    void (*mp_matrix_init_identity)(cairo_matrix_t*);
    void (*mp_matrix_translate)( cairo_matrix_t*, double, double);
    void (*mp_matrix_scale )( cairo_matrix_t*, double, double);
    void (*mp_matrix_rotate)( cairo_matrix_t*, double);
    void (*mp_transform)( cairo_t*, cairo_matrix_t*);
    unsigned char* (*mp_image_surface_get_data)( cairo_surface_t* );
    int (*mp_image_surface_get_width)(cairo_surface_t* surface);
    int (*mp_image_surface_get_height)(cairo_surface_t* surface);
    int (*mp_image_surface_get_stride)(cairo_surface_t* surface);

    // LibRSVG
    void (*mp_rsvg_init)( void );
    RsvgHandle* (*mp_rsvg_handle_new_from_data)( const guint8*, gsize, GError** );
    gboolean (*mp_rsvg_handle_close)( RsvgHandle*, GError** );
    void (*mp_rsvg_handle_set_dpi_x_y)( RsvgHandle*, double, double );
    void (*mp_rsvg_handle_get_dimensions)( RsvgHandle*, RsvgDimensionData* );
    gboolean (*mp_rsvg_handle_render_cairo)( RsvgHandle*, cairo_t* );
};

// -----------------------------------------------------------------------------

LibraryWrapper& LibraryWrapper::get()
{
    static LibraryWrapper* pLibraryInstance = NULL;

    if( !pLibraryInstance )
        pLibraryInstance = new LibraryWrapper;

    return( *pLibraryInstance );
}

// -----------------------------------------------------------------------------

LibraryWrapper::LibraryWrapper() :
    mpGObjectLib( NULL ),
    mpCairoLib( NULL ),
    mpRSVGLib( NULL )
{
    const ::rtl::OUString aGObjectLibName( RTL_CONSTASCII_USTRINGPARAM( VCL_RSVG_GOBJECT_LIBNAME ) );
    const ::rtl::OUString aCairoLibName( RTL_CONSTASCII_USTRINGPARAM( VCL_RSVG_CAIRO_LIBNAME ) );
    const ::rtl::OUString aRSVGLibName( RTL_CONSTASCII_USTRINGPARAM( VCL_RSVG_LIBRSVG_LIBNAME ) );
    bool bCont = true;

    // GObject
    if( bCont && ( NULL != ( mpGObjectLib = osl_loadModule( aGObjectLibName.pData, SAL_LOADMODULE_DEFAULT ) ) ||
                   NULL != ( mpGObjectLib = osl_loadModuleRelative( (oslGenericFunction)LibraryWrapper::get,
                                                                   aGObjectLibName.pData, SAL_LOADMODULE_DEFAULT ) )

                  ) )
    {
        mp_g_object_unref = ( gpointer (*)( gpointer ) ) osl_getAsciiFunctionSymbol( mpGObjectLib, "g_object_unref" );

        if( !( mp_g_object_unref ) )
        {
            OSL_TRACE( "not all needed symbols were found in g-object library" );
            bCont = false;
        }
    }

    // Cairo
    if( bCont && ( NULL != ( mpCairoLib = osl_loadModule( aCairoLibName.pData, SAL_LOADMODULE_DEFAULT ) ) ||
                   NULL != ( mpCairoLib = osl_loadModuleRelative( (oslGenericFunction)LibraryWrapper::get,
                                                                   aCairoLibName.pData, SAL_LOADMODULE_DEFAULT ) )
                  ) )
    {
        mp_image_surface_create = ( cairo_surface_t* (*)( cairo_format_t, int, int ) ) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_image_surface_create" );
        mp_surface_destroy = ( void (*)( cairo_surface_t* ) ) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_surface_destroy" );
        mp_surface_status = ( cairo_status_t (*)( cairo_surface_t* ) ) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_surface_status" );
        mp_create = ( cairo_t* (*)( cairo_surface_t* ) ) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_create" );
        mp_destroy = ( void (*)( cairo_t* ) ) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_destroy" );
        mp_matrix_init_identity = ( void (*)( cairo_matrix_t* ) ) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_matrix_init_identity" );
        mp_matrix_translate = ( void (*)( cairo_matrix_t*, double, double ) ) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_matrix_translate" );
        mp_matrix_scale = ( void (*)( cairo_matrix_t*, double, double ) ) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_matrix_scale" );
        mp_matrix_rotate = ( void (*)( cairo_matrix_t*, double ) ) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_matrix_rotate" );
        mp_transform = ( void (*)( cairo_t*, cairo_matrix_t* ) ) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_transform" );
        mp_image_surface_get_data = ( unsigned char* (*)( cairo_surface_t* ) ) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_image_surface_get_data" );
        mp_image_surface_get_width = ( int (*)( cairo_surface_t* ) ) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_image_surface_get_width" );
        mp_image_surface_get_height = ( int (*)( cairo_surface_t* ) ) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_image_surface_get_height" );
        mp_image_surface_get_stride = ( int (*)( cairo_surface_t* ) ) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_image_surface_get_stride" );

        if( !( mp_image_surface_create &&
               mp_surface_destroy &&
               mp_surface_status &&
               mp_create &&
               mp_destroy &&
               mp_matrix_init_identity &&
               mp_matrix_translate &&
               mp_matrix_scale &&
               mp_matrix_rotate &&
               mp_transform &&
               mp_image_surface_get_data &&
               mp_image_surface_get_width &&
               mp_image_surface_get_height &&
               mp_image_surface_get_stride ) )
        {
            OSL_TRACE( "not all needed symbols were found in cairo library" );
            bCont = false;
        }
    }

    // LibRSVG
    if( bCont && ( NULL != ( mpRSVGLib = osl_loadModule( aRSVGLibName.pData, SAL_LOADMODULE_DEFAULT ) ) ||
                   NULL != ( mpRSVGLib = osl_loadModuleRelative( (oslGenericFunction)LibraryWrapper::get,
                                                                   aRSVGLibName.pData, SAL_LOADMODULE_DEFAULT ) )
                  ) )
    {
        mp_rsvg_init = ( void (*)( void ) ) osl_getAsciiFunctionSymbol( mpRSVGLib, "rsvg_init" );
        mp_rsvg_handle_new_from_data = ( RsvgHandle* (*)( const guint8*, gsize, GError** ) ) osl_getAsciiFunctionSymbol( mpRSVGLib, "rsvg_handle_new_from_data" );
        mp_rsvg_handle_close = ( gboolean (*)( RsvgHandle*, GError** ) ) osl_getAsciiFunctionSymbol( mpRSVGLib, "rsvg_handle_close" );
        mp_rsvg_handle_set_dpi_x_y = ( void (*)( RsvgHandle*, double, double ) ) osl_getAsciiFunctionSymbol( mpRSVGLib, "rsvg_handle_set_dpi_x_y" );
        mp_rsvg_handle_get_dimensions = ( void (*)( RsvgHandle*, RsvgDimensionData* ) ) osl_getAsciiFunctionSymbol( mpRSVGLib, "rsvg_handle_get_dimensions" );
        mp_rsvg_handle_render_cairo = ( gboolean (*)( RsvgHandle*, cairo_t* ) ) osl_getAsciiFunctionSymbol( mpRSVGLib, "rsvg_handle_render_cairo" );

        if( !( mp_rsvg_init &&
               mp_rsvg_handle_new_from_data &&
               mp_rsvg_handle_close &&
               mp_rsvg_handle_set_dpi_x_y &&
               mp_rsvg_handle_get_dimensions &&
               mp_rsvg_handle_render_cairo ) )
        {
            OSL_TRACE( "not all needed symbols were found in librsvg library" );
            bCont = false;
        }
    }

    OSL_ENSURE( mpGObjectLib, "g-object library could not be loaded" );
    OSL_ENSURE( mpCairoLib, "cairo library could not be loaded" );
    OSL_ENSURE( mpRSVGLib, "librsvg library could not be loaded" );

    bCont = bCont && mpGObjectLib != NULL && mpCairoLib != NULL && mpRSVGLib != NULL;

    // unload all libraries in case of failure
    if( !bCont )
    {
        if( mpRSVGLib )
        {
            osl_unloadModule( mpRSVGLib );
            mpRSVGLib = NULL;
        }

        if( mpCairoLib )
        {
            osl_unloadModule( mpCairoLib );
            mpCairoLib = NULL;
        }

        if( mpGObjectLib )
        {
            osl_unloadModule( mpGObjectLib );
            mpGObjectLib = NULL;
        }
    }
    else
        rsvg_init();
}

// ---------------------------
// - ::vcl::rsvg::Rasterizer -
// ---------------------------

class Rasterizer : public ::cppu::WeakAggImplHelper2< graphic::XGraphicRasterizer, lang::XServiceInfo >
{
public:
                Rasterizer();
    virtual     ~Rasterizer();

    // XGraphicRasterizer
    virtual ::sal_Bool SAL_CALL initializeData( const uno::Reference< io::XInputStream >& DataStream,
                                                ::sal_uInt32 DPI_X, ::sal_uInt32 DPI_Y,
                                                awt::Size& DefaultSizePixel )
        throw ( uno::RuntimeException );

    virtual uno::Reference< graphic::XGraphic > SAL_CALL rasterize( ::sal_uInt32 Width,
                                                                    ::sal_uInt32 Height,
                                                                    double RotateAngle,
                                                                    double ShearAngle_X,
                                                                    double ShearAngle_Y,
                                                                    const uno::Sequence< beans::PropertyValue >& RasterizeProperties )
        throw (uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& )
        throw( uno::RuntimeException );

    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( uno::RuntimeException );

protected:

    void        implFreeRsvgHandle();

    uno::Reference< graphic::XGraphic > implGetXGraphicFromSurface( cairo_surface_t* pSurface ) const;

private:

                Rasterizer( const Rasterizer& );
    Rasterizer& operator=( const Rasterizer& );

private:

    RsvgHandle* mpRsvgHandle;
    sal_Int32   mnDefaultWidth;
    sal_Int32   mnDefaultHeight;
    bool        mbLibInit;
};

// -----------------------------------------------------------------------------

Rasterizer::Rasterizer() :
    mpRsvgHandle( NULL ),
    mnDefaultWidth( 0 ),
    mnDefaultHeight( 0 ),
    mbLibInit( true )
{
    try
    {
        LibraryWrapper& rLib = LibraryWrapper::get();
        mbLibInit = rLib.isValid();
    }
    catch( ... )
    {
        mbLibInit = false;
    }
}

// -----------------------------------------------------------------------------

Rasterizer::~Rasterizer()
{
    implFreeRsvgHandle();
}

// -----------------------------------------------------------------------------

void Rasterizer::implFreeRsvgHandle()
{
    if( mpRsvgHandle )
    {
        LibraryWrapper::get().g_object_unref( static_cast< gpointer >( mpRsvgHandle ) );
        mpRsvgHandle = NULL;
    }
}

// -----------------------------------------------------------------------------

uno::Reference< graphic::XGraphic > Rasterizer::implGetXGraphicFromSurface( cairo_surface_t* pSurface ) const
{
    LibraryWrapper& rLib = LibraryWrapper::get();
    unsigned char*  pData = rLib.image_surface_get_data( pSurface );
    const sal_Int32 nWidth = rLib.image_surface_get_width( pSurface );
    const sal_Int32 nHeight =rLib.image_surface_get_height( pSurface );
    const sal_Int32 nStride = rLib.image_surface_get_stride( pSurface );

    uno::Reference< graphic::XGraphic > xRet;

    if( pData && nWidth && nHeight && nStride )
    {
        Size                aSize( nWidth, nHeight );
        Bitmap              aBmp( aSize, 24 );
        AlphaMask           aAlphaMask( aSize );
        BitmapWriteAccess*  pBmpAcc = aBmp.AcquireWriteAccess();
        BitmapWriteAccess*  pAlpAcc = aAlphaMask.AcquireWriteAccess();

        if( pBmpAcc && pAlpAcc )
        {
            BitmapColor aPixel, aWhitePixel( 255, 255, 255 ), aAlpha( 0 ), aFullAlpha( 255 );
            sal_uInt32* pRow = reinterpret_cast< sal_uInt32* >( pData );

            for( sal_Int32 nY = 0; nY < nHeight; ++nY, pRow = reinterpret_cast< sal_uInt32* >( pData += nStride )  )
            {
                for( sal_Int32 nX = 0; nX < nWidth; ++nX )
                {
                    const register sal_uInt32 nPixel = *pRow++;
                    const register sal_uInt32 nAlpha = nPixel >> 24;

                    if( !nAlpha )
                    {
                        pBmpAcc->SetPixel( nY, nX, aWhitePixel );
                        pAlpAcc->SetPixel( nY, nX, aFullAlpha );
                    }
                    else
                    {
                        aPixel.SetRed( static_cast< sal_uInt8 >( ( ( ( nPixel & 0x00ff0000 ) >> 16 ) * 255 ) / nAlpha ) );
                        aPixel.SetGreen( static_cast< sal_uInt8 >( ( ( ( nPixel & 0x0000ff00 ) >> 8 ) * 255 ) / nAlpha ) );
                        aPixel.SetBlue( static_cast< sal_uInt8 >( ( ( nPixel & 0x000000ff ) * 255 ) / nAlpha ) );
                        pBmpAcc->SetPixel( nY, nX, aPixel );

                        aAlpha.SetIndex( static_cast< sal_uInt8 >( 255 - nAlpha ) );
                        pAlpAcc->SetPixel( nY, nX, aAlpha );
                    }
                }
            }

            aBmp.ReleaseAccess( pBmpAcc );
            aAlphaMask.ReleaseAccess( pAlpAcc );

            const Graphic aGraphic( BitmapEx( aBmp, aAlphaMask ) );
            xRet.set( aGraphic.GetXGraphic(), uno::UNO_QUERY );
        }
        else
        {
            aBmp.ReleaseAccess( pBmpAcc );
            aAlphaMask.ReleaseAccess( pAlpAcc );
        }
    }

    return( xRet );
}

// -----------------------------------------------------------------------------

::sal_Bool SAL_CALL Rasterizer::initializeData( const uno::Reference< io::XInputStream >& rDataStream,
                                                ::sal_uInt32 nDPI_X, ::sal_uInt32 nDPI_Y,
                                                awt::Size& rDefaultSizePixel )
        throw ( uno::RuntimeException )
{
    LibraryWrapper& rLib = LibraryWrapper::get();

    implFreeRsvgHandle();

    if( mbLibInit && rDataStream.is() )
    {
        ::std::vector< sal_Int8 >       aDataBuffer;
        uno::Reference< io::XSeekable > xSeekable( rDataStream, uno::UNO_QUERY );
        sal_Int32                       nReadSize, nBlockSize = ( xSeekable.is() ? xSeekable->getLength() : 65536 );
        uno::Sequence< sal_Int8 >       aStmBuffer( nBlockSize );

        do
        {
            nReadSize = rDataStream->readBytes( aStmBuffer, nBlockSize );

            if( nReadSize > 0 )
            {
                const sal_Int8* pArray = aStmBuffer.getArray();
                aDataBuffer.insert( aDataBuffer.end(), pArray, pArray + nReadSize );
            }
        }
        while( nReadSize == nBlockSize );

        if( aDataBuffer.size() &&
            ( NULL != ( mpRsvgHandle = rLib.rsvg_handle_new_from_data( reinterpret_cast< sal_uInt8* >( &aDataBuffer[ 0 ] ),
                                                                       aDataBuffer.size(), NULL ) ) ) &&
            !rLib.rsvg_handle_close( mpRsvgHandle, NULL ) )
        {
            implFreeRsvgHandle();
        }
    }

    // get default dimensions of image
    mnDefaultWidth = mnDefaultHeight = 0;

    if( mpRsvgHandle )
    {
        RsvgDimensionData aDefaultDimension = { 0, 0, 0.0, 0.0 };

        rLib.rsvg_handle_set_dpi_x_y( mpRsvgHandle, nDPI_X ? nDPI_X: 72, nDPI_Y ? nDPI_Y : 72 );
        rLib.rsvg_handle_get_dimensions( mpRsvgHandle, &aDefaultDimension );

        mnDefaultWidth = aDefaultDimension.width;
        mnDefaultHeight = aDefaultDimension.height;
    }

    rDefaultSizePixel.Width = mnDefaultWidth;
    rDefaultSizePixel.Height = mnDefaultHeight;

    return( mpRsvgHandle != NULL );
}

// -----------------------------------------------------------------------------

uno::Reference< graphic::XGraphic > SAL_CALL Rasterizer::rasterize( ::sal_uInt32 nWidth,
                                                                    ::sal_uInt32 nHeight,
                                                                    double /*fRotateAngle*/,
                                                                    double /*fShearAngle_X*/,
                                                                    double /*ShearAngle_Y*/,
                                                                    const uno::Sequence< beans::PropertyValue >&
                                                                        /*rRasterizeProperties*/ )
        throw ( uno::RuntimeException )
{
    LibraryWrapper& rLib = LibraryWrapper::get();
    uno::Reference< graphic::XGraphic > xRet;

    if( mpRsvgHandle && rLib.isValid() && nWidth && nHeight && mnDefaultWidth && mnDefaultHeight )
    {
        cairo_surface_t* pSurface = rLib.image_surface_create( CAIRO_FORMAT_ARGB32, nWidth, nHeight );

        if( pSurface && ( CAIRO_STATUS_SUCCESS == rLib.surface_status( pSurface ) ) )
        {
            cairo_t* pCr = rLib.create( pSurface );

            if( pCr )
            {
                cairo_matrix_t aMatrix;

                rLib.matrix_init_identity( &aMatrix );
                rLib.matrix_scale( &aMatrix,
                                     static_cast< double >( nWidth ) / mnDefaultWidth,
                                     static_cast< double >( nHeight ) / mnDefaultHeight );
                rLib.transform( pCr, &aMatrix );

                if( rLib.rsvg_handle_render_cairo( mpRsvgHandle, pCr ) )
                {
                    xRet = implGetXGraphicFromSurface( pSurface );
                }

                rLib.destroy( pCr );
            }

            rLib.surface_destroy( pSurface );
            OSL_ENSURE( xRet.is(), "SVG *not* rendered successfully" );
        }
    }

    return( xRet );
}

// -----------------------------------------------------------------------------

::rtl::OUString SAL_CALL Rasterizer::getImplementationName()
    throw( uno::RuntimeException )
{
    return( Rasterizer_getImplementationName() );
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL Rasterizer::supportsService( const ::rtl::OUString& rServiceName )
        throw( uno::RuntimeException )
{
    const uno::Sequence< ::rtl::OUString > aServices( Rasterizer_getSupportedServiceNames() );

    for( sal_Int32 nService = 0; nService < aServices.getLength(); ++nService )
    {
        if( rServiceName == aServices[ nService ] )
            return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL Rasterizer::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    return( Rasterizer_getSupportedServiceNames() );
}

// ------------------------------
// - Uno instantiation function -
// ------------------------------

uno::Reference< uno::XInterface > SAL_CALL Rasterizer_createInstance( const uno::Reference< lang::XMultiServiceFactory >&  )
{
    return static_cast< ::cppu::OWeakObject* >( new rsvg::Rasterizer );
}

} // namespace rsvg
} // namespace vcl
