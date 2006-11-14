/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: jpeg.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:38:19 $
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

#ifndef _JPEG_HXX
#define _JPEG_HXX

#ifndef _GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _FLTCALL_HXX
#include "fltcall.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif

#ifdef _JPEGPRIVATE

// --------
// - Enum -
// --------

enum ReadState
{
    JPEGREAD_OK,
    JPEGREAD_ERROR,
    JPEGREAD_NEED_MORE
};

// --------------
// - JPEGReader -
// --------------

class JPEGReader : public GraphicReader
{
    SvStream&           rIStm;
    Bitmap              aBmp;
    Bitmap              aBmp1;
    BitmapWriteAccess*  pAcc;
    BitmapWriteAccess*  pAcc1;
    void*               pBuffer;
    long                nLastPos;
    long                nFormerPos;
    long                nLastLines;
    sal_Bool            bSetLogSize;

    Graphic             CreateIntermediateGraphic( const Bitmap& rBitmap, long nLines );
    void                FillBitmap();

public:

    void*               CreateBitmap( void* JPEGCreateBitmapParam );

public:



                        JPEGReader( SvStream& rStm, void* pCallData, sal_Bool bSetLogSize );
    virtual             ~JPEGReader();


    ReadState           Read( Graphic& rGraphic );
};

// --------------
// - JPEGWriter -
// --------------

class JPEGWriter
{
    SvStream&           rOStm;
    Bitmap              aBmp;
    BitmapReadAccess*   pAcc;
    BYTE*               pBuffer;
    BOOL                bNative;

    sal_Bool            bGreys;
    sal_Int32           nQuality;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

public:

    void*               GetScanline( long nY );

                        JPEGWriter( SvStream& rOStm, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData );
                        ~JPEGWriter() {};

    BOOL                Write( const Graphic& rGraphic );
};

#endif // _JPEGPRIVATE

// ---------------------
// - Import/ExportJPEG -
// ---------------------

BOOL ImportJPEG( SvStream& rStream, Graphic& rGraphic, void* pCallerData, sal_Int32 nImportFlags );

BOOL ExportJPEG( SvStream& rStream, const Graphic& rGraphic, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData );

#endif // _JPEG_HXX
