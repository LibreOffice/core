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

#ifndef _JPEG_HXX
#define _JPEG_HXX

#ifndef _GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _FLTCALL_HXX
#include <bf_svtools/fltcall.hxx>
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

namespace binfilter
{

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
// - JPEGReader	-
// --------------

class JPEGReader : public GraphicReader
{
    SvStream&			rIStm;
    Bitmap				aBmp;
    Bitmap				aBmp1;
    BitmapWriteAccess*	pAcc;
    BitmapWriteAccess*	pAcc1;
    void*				pBuffer;
    long				nLastPos;
    long				nFormerPos;
    long				nLastLines;
    sal_Bool            bSetLogSize;

    Graphic				CreateIntermediateGraphic( const Bitmap& rBitmap, long nLines );
    void				FillBitmap();

public:

    void*				CreateBitmap( void* JPEGCreateBitmapParam );

public:



                        JPEGReader( SvStream& rStm, void* pCallData, sal_Bool bSetLogSize );
    virtual				~JPEGReader();


    ReadState			Read( Graphic& rGraphic );
};

// --------------
// - JPEGWriter	-
// --------------

class JPEGWriter
{
    SvStream&			rOStm;
    Bitmap				aBmp;
    BitmapReadAccess*	pAcc;
    BYTE*				pBuffer;
    BOOL				bNative;

    sal_Bool			bGreys;
    sal_Int32			nQuality;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

public:

    void*				GetScanline( long nY );

                        JPEGWriter( SvStream& rOStm, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData );
                        ~JPEGWriter() {};

    BOOL				Write( const Graphic& rGraphic );
};

#endif // _JPEGPRIVATE

// ---------------------
// - Import/ExportJPEG -
// ---------------------

BOOL ImportJPEG( SvStream& rStream, Graphic& rGraphic, void* pCallerData, sal_Int32 nImportFlags );

BOOL ExportJPEG( SvStream& rStream, const Graphic& rGraphic, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData );

}

#endif // _JPEG_HXX
