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

#ifndef _SV_RENDERGRAPHICRASTERIZER_HXX
#define _SV_RENDERGRAPHICRASTERIZER_HXX

#include <vcl/rendergraphic.hxx>
#include <vcl/bitmapex.hxx>

#include <com/sun/star/graphic/XGraphicRasterizer.hpp>

#include <memory>
#include <deque>

#define VCL_RASTERIZER_UNLIMITED_EXTENT 0x00000000
#define VCL_RASTERIZER_DEFAULT_EXTENT   0xffffffff

namespace vcl
{
    // ---------------------------
    // - RenderGraphicRasterizer -
    // ---------------------------

    class VCL_DLLPUBLIC RenderGraphicRasterizer
    {
    public:

        RenderGraphicRasterizer( const RenderGraphic& rData );


        RenderGraphicRasterizer( const RenderGraphicRasterizer& rRenderGraphicRasterizer );

        virtual ~RenderGraphicRasterizer();

        RenderGraphicRasterizer& operator=( const RenderGraphicRasterizer& rRenderGraphicRasterizer );

        inline const RenderGraphic& GetRenderGraphic() const
        {
            return( maRenderGraphic );
        }

        const Size& GetDefaultSizePixel() const;

        BitmapEx GetReplacement() const;

        virtual Size GetPrefSize() const;

        virtual MapMode GetPrefMapMode() const;

        virtual const BitmapEx& Rasterize( const Size& rSizePixel_UnrotatedUnsheared,
                                           double fRotateAngle = 0.0,
                                           double fShearAngleX = 0.0,
                                           double fShearAngleY = 0.0,
                                           sal_uInt32 nMaxExtent = VCL_RASTERIZER_DEFAULT_EXTENT ) const;

    protected:

        RenderGraphic                                                                           maRenderGraphic;
        mutable com::sun::star::uno::Reference< com::sun::star::graphic::XGraphicRasterizer >   mxRasterizer;

        virtual void InitializeRasterizer();

    private:

        RenderGraphicRasterizer();

        mutable BitmapEx    maBitmapEx;
        mutable Size        maDefaultSizePixel;
        mutable double      mfRotateAngle;
        mutable double      mfShearAngleX;
        mutable double      mfShearAngleY;

    private:

        typedef ::std::deque< RenderGraphicRasterizer > RenderGraphicRasterizerCache;

        static RenderGraphicRasterizerCache& ImplGetCache();

        static bool ImplInitializeFromCache( RenderGraphicRasterizer& rRasterizer );
        static bool ImplRasterizeFromCache( RenderGraphicRasterizer& rRasterizer,
                                            const Size& rSizePixel, double fRotateAngle,
                                            double fShearAngleX, double fShearAngleY );
        static void ImplUpdateCache( const RenderGraphicRasterizer& rRasterizer );
};
}

#endif // _SV_RENDERGRAPHICRASTERIZER_HXX
