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


#include "dx_bitmap.hxx"
#include "dx_graphicsprovider.hxx"
#include "dx_impltools.hxx"

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2irange.hxx>

#if defined(DX_DEBUG_IMAGES)
# if OSL_DEBUG_LEVEL > 0
#  include <imdebug.h>
#  undef min
#  undef max
# endif
#endif

using namespace ::com::sun::star;

namespace dxcanvas
{

    // DXBitmap::DXBitmap


    DXBitmap::DXBitmap( const BitmapSharedPtr& rBitmap,
                        bool                   bWithAlpha ) :
        mpGdiPlusUser( GDIPlusUser::createInstance() ),
        maSize(rBitmap->GetWidth(),rBitmap->GetHeight()),
        mpBitmap(rBitmap),
        mpGraphics(tools::createGraphicsFromBitmap(mpBitmap)),
        mbAlpha(bWithAlpha)
    {
    }

    DXBitmap::DXBitmap( const ::basegfx::B2IVector& rSize,
                        bool                        bWithAlpha ) :
        mpGdiPlusUser( GDIPlusUser::createInstance() ),
        maSize(rSize),
        mpBitmap(),
        mpGraphics(),
        mbAlpha(bWithAlpha)
    {
        // create container for pixel data
        if(mbAlpha)
        {
            mpBitmap.reset(
                new Gdiplus::Bitmap(
                    maSize.getX(),
                    maSize.getY(),
                    PixelFormat32bppARGB));
        }
        else
        {
            mpBitmap.reset(
                new Gdiplus::Bitmap(
                    maSize.getX(),
                    maSize.getY(),
                    PixelFormat24bppRGB));
        }

        mpGraphics.reset( tools::createGraphicsFromBitmap(mpBitmap) );
    }

    BitmapSharedPtr DXBitmap::getBitmap() const
    {
        return mpBitmap;
    }

    GraphicsSharedPtr DXBitmap::getGraphics()
    {
        return mpGraphics;
    }

    ::basegfx::B2IVector DXBitmap::getSize() const
    {
        return maSize;
    }

    bool DXBitmap::hasAlpha() const
    {
        return mbAlpha;
    }

    uno::Sequence< sal_Int8 > DXBitmap::getData( rendering::IntegerBitmapLayout&     /*bitmapLayout*/,
                                                 const geometry::IntegerRectangle2D& rect )
    {
        uno::Sequence< sal_Int8 > aRes( (rect.X2-rect.X1)*(rect.Y2-rect.Y1)*4 ); // TODO(F1): Be format-agnostic here

        const Gdiplus::Rect aRect( tools::gdiPlusRectFromIntegerRectangle2D( rect ) );

        Gdiplus::BitmapData aBmpData;
        aBmpData.Width       = rect.X2-rect.X1;
        aBmpData.Height      = rect.Y2-rect.Y1;
        aBmpData.Stride      = 4*aBmpData.Width;
        aBmpData.PixelFormat = PixelFormat32bppARGB;
        aBmpData.Scan0       = aRes.getArray();

        // TODO(F1): Support more pixel formats natively

        // read data from bitmap
        if( Gdiplus::Ok != mpBitmap->LockBits( &aRect,
                                                      Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeUserInputBuf,
                                                      PixelFormat32bppARGB, // TODO(F1): Adapt to
                                                      // Graphics native
                                                      // format/change
                                                      // getMemoryLayout
                                                      &aBmpData ) )
        {
            // failed to lock, bail out
            return uno::Sequence< sal_Int8 >();
        }

        mpBitmap->UnlockBits( &aBmpData );

        return aRes;
    }

    void DXBitmap::setData( const uno::Sequence< sal_Int8 >&        data,
                            const rendering::IntegerBitmapLayout&   /*bitmapLayout*/,
                            const geometry::IntegerRectangle2D&     rect )
    {
        const Gdiplus::Rect aRect( tools::gdiPlusRectFromIntegerRectangle2D( rect ) );

        Gdiplus::BitmapData aBmpData;
        aBmpData.Width       = rect.X2-rect.X1;
        aBmpData.Height      = rect.Y2-rect.Y1;
        aBmpData.Stride      = 4*aBmpData.Width;
        aBmpData.PixelFormat = PixelFormat32bppARGB;
        aBmpData.Scan0       = (void*)data.getConstArray();

        // TODO(F1): Support more pixel formats natively

        if( Gdiplus::Ok != mpBitmap->LockBits( &aRect,
                                                      Gdiplus::ImageLockModeWrite | Gdiplus::ImageLockModeUserInputBuf,
                                                      PixelFormat32bppARGB, // TODO: Adapt to
                                                      // Graphics native
                                                      // format/change
                                                      // getMemoryLayout
                                                      &aBmpData ) )
        {
            throw uno::RuntimeException();
        }

        // commit data to bitmap
        mpBitmap->UnlockBits( &aBmpData );
    }

    void DXBitmap::setPixel( const uno::Sequence< sal_Int8 >&       color,
                             const rendering::IntegerBitmapLayout&  /*bitmapLayout*/,
                             const geometry::IntegerPoint2D&        pos )
    {
        const geometry::IntegerSize2D aSize( maSize.getX(),maSize.getY() );

        ENSURE_ARG_OR_THROW( pos.X >= 0 && pos.X < aSize.Width,
                             "CanvasHelper::setPixel: X coordinate out of bounds" );
        ENSURE_ARG_OR_THROW( pos.Y >= 0 && pos.Y < aSize.Height,
                             "CanvasHelper::setPixel: Y coordinate out of bounds" );
        ENSURE_ARG_OR_THROW( color.getLength() > 3,
                             "CanvasHelper::setPixel: not enough color components" );

        if( Gdiplus::Ok != mpBitmap->SetPixel( pos.X, pos.Y,
                                                      Gdiplus::Color( tools::sequenceToArgb( color ))))
        {
            throw uno::RuntimeException();
        }
    }

    uno::Sequence< sal_Int8 > DXBitmap::getPixel( rendering::IntegerBitmapLayout&   /*bitmapLayout*/,
                                                  const geometry::IntegerPoint2D&   pos )
    {
        const geometry::IntegerSize2D aSize( maSize.getX(),maSize.getY() );

        ENSURE_ARG_OR_THROW( pos.X >= 0 && pos.X < aSize.Width,
                             "CanvasHelper::getPixel: X coordinate out of bounds" );
        ENSURE_ARG_OR_THROW( pos.Y >= 0 && pos.Y < aSize.Height,
                             "CanvasHelper::getPixel: Y coordinate out of bounds" );

        Gdiplus::Color aColor;

        if( Gdiplus::Ok != mpBitmap->GetPixel( pos.X, pos.Y, &aColor ) )
            return uno::Sequence< sal_Int8 >();

        return tools::argbToIntSequence(aColor.GetValue());
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
