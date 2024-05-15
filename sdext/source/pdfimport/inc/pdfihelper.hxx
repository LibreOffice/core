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

#ifndef INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_PDFIHELPER_HXX
#define INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_PDFIHELPER_HXX

#include "contentsink.hxx"

#include <rtl/ustring.hxx>
#include <rtl/math.h>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>

#include <unordered_map>
#include <vector>
#include <o3tl/hash_combine.hxx>

// virtual resolution of the PDF OutputDev in dpi
#define PDFI_OUTDEV_RESOLUTION 7200

namespace com::sun::star::task { class XInteractionHandler; }

namespace pdfi
{
    typedef std::unordered_map< OUString, OUString > PropertyMap;
    typedef sal_Int32 ImageId;

    /// What to do with a polygon. values can be ORed together
    enum PolygonAction { PATH_STROKE=1, PATH_FILL=2, PATH_EOFILL=4 };

    OUString unitMMString( double fMM );
    OUString convertPixelToUnitString( double fPix );

    inline double convPx2mm( double fPix )
    {
        const double px2mm = 25.4/PDFI_OUTDEV_RESOLUTION;
        fPix *= px2mm;
        return fPix;
    }

    inline double convmm2Px( double fMM )
    {
        const double mm2px = PDFI_OUTDEV_RESOLUTION/25.4;
        fMM *= mm2px;
        return fMM;
    }

    /// round to 2 decimal places
    inline double convPx2mmPrec2( double fPix )
    {
        constexpr double px2mm = 25.4/PDFI_OUTDEV_RESOLUTION;
        double mm = fPix * ( px2mm * 100);
        return std::floor(mm) / 100;
    }

    /// Convert color to "#FEFEFE" color notation
    OUString getColorString( const css::rendering::ARGBColor& );
    OUString getPercentString(double value);

    double GetAverageTransformationScale(const basegfx::B2DHomMatrix& matrix);
    void FillDashStyleProps(PropertyMap& props, const std::vector<double>& dashArray, double scale);

    struct FontAttrHash
    {
        size_t operator()(const FontAttributes& rFont ) const
        {
            std::size_t seed = 0;
            o3tl::hash_combine(seed, rFont.familyName.hashCode());
            o3tl::hash_combine(seed, rFont.fontWeight);
            o3tl::hash_combine(seed, rFont.isItalic);
            o3tl::hash_combine(seed, rFont.isUnderline);
            o3tl::hash_combine(seed, rFont.isOutline);
            o3tl::hash_combine(seed, rFont.size);
            return seed;
        }
    };

    struct GraphicsContext
    {
        css::rendering::ARGBColor     LineColor;
        css::rendering::ARGBColor     FillColor;
        sal_Int8                                   LineJoin;
        sal_Int8                                   LineCap;
        sal_Int8                                   BlendMode;
        double                                     Flatness;
        double                                     LineWidth;
        double                                     MiterLimit;
        std::vector<double>                        DashArray;
        sal_Int32                                  FontId;
        sal_Int32                                  TextRenderMode;
        basegfx::B2DHomMatrix                      Transformation;
        basegfx::B2DPolyPolygon                    Clip;

        GraphicsContext() :
            LineColor(),
            FillColor(),
            LineJoin(0),
            LineCap(0),
            BlendMode(0),
            Flatness(0.0),
            LineWidth(1.0),
            MiterLimit(10.0),
            DashArray(),
            FontId(0),
            TextRenderMode(0),
            Transformation(),
            Clip()
        {}

        bool operator==(const GraphicsContext& rRight ) const
        {
            return LineColor.Red == rRight.LineColor.Red &&
                LineColor.Green == rRight.LineColor.Green &&
                LineColor.Blue == rRight.LineColor.Blue &&
                LineColor.Alpha == rRight.LineColor.Alpha &&
                FillColor.Red == rRight.FillColor.Red &&
                FillColor.Green == rRight.FillColor.Green &&
                FillColor.Blue == rRight.FillColor.Blue &&
                FillColor.Alpha == rRight.FillColor.Alpha &&
                LineJoin  == rRight.LineJoin &&
                LineCap   == rRight.LineCap &&
                BlendMode == rRight.BlendMode &&
                LineWidth == rRight.LineWidth &&
                Flatness == rRight.Flatness &&
                MiterLimit == rRight.MiterLimit &&
                DashArray == rRight.DashArray &&
                FontId    == rRight.FontId &&
                TextRenderMode == rRight.TextRenderMode &&
                Transformation == rRight.Transformation &&
                Clip == rRight.Clip;
        }

        OUString GetLineJoinString() const
        {
            switch (LineJoin)
            {
            default:
            case css::rendering::PathJoinType::MITER:
                return u"miter"_ustr;
            case css::rendering::PathJoinType::ROUND:
                return u"round"_ustr;
            case css::rendering::PathJoinType::BEVEL:
                return u"bevel"_ustr;
            }
        }

        OUString GetLineCapString() const
        {
            switch (LineCap)
            {
            default:
            case css::rendering::PathCapType::BUTT:
                return u"butt"_ustr;
            case css::rendering::PathCapType::ROUND:
                return u"round"_ustr;
            case css::rendering::PathCapType::SQUARE:
                return u"square"_ustr;
            }
        }

        bool isRotatedOrSkewed() const
        { return Transformation.get( 0, 1 ) != 0.0 ||
                Transformation.get( 1, 0 ) != 0.0; }
    };

    struct GraphicsContextHash
    {
        size_t operator()(const GraphicsContext& rGC ) const
        {
            std::size_t seed = 0;
            o3tl::hash_combine(seed, rGC.LineColor.Red);
            o3tl::hash_combine(seed, rGC.LineColor.Green);
            o3tl::hash_combine(seed, rGC.LineColor.Blue);
            o3tl::hash_combine(seed, rGC.LineColor.Alpha);
            o3tl::hash_combine(seed, rGC.FillColor.Red);
            o3tl::hash_combine(seed, rGC.FillColor.Green);
            o3tl::hash_combine(seed, rGC.FillColor.Blue);
            o3tl::hash_combine(seed, rGC.FillColor.Alpha);
            o3tl::hash_combine(seed, rGC.LineJoin);
            o3tl::hash_combine(seed, rGC.LineCap);
            o3tl::hash_combine(seed, rGC.BlendMode);
            o3tl::hash_combine(seed, rGC.LineWidth);
            o3tl::hash_combine(seed, rGC.Flatness);
            o3tl::hash_combine(seed, rGC.MiterLimit);
            o3tl::hash_combine(seed, rGC.DashArray.size());
            o3tl::hash_combine(seed, rGC.FontId);
            o3tl::hash_combine(seed, rGC.TextRenderMode);
            o3tl::hash_combine(seed, rGC.Transformation.get( 0, 0 ));
            o3tl::hash_combine(seed, rGC.Transformation.get( 1, 0 ));
            o3tl::hash_combine(seed, rGC.Transformation.get( 0, 1 ));
            o3tl::hash_combine(seed, rGC.Transformation.get( 1, 1 ));
            o3tl::hash_combine(seed, rGC.Transformation.get( 0, 2 ));
            o3tl::hash_combine(seed, rGC.Transformation.get( 1, 2 ));
            o3tl::hash_combine(seed, rGC.Clip.count() ? rGC.Clip.getB2DPolygon(0).count() : 0);
            return seed;
        }
    };

    /** retrieve password from user
     */
    bool getPassword( const css::uno::Reference<
                            css::task::XInteractionHandler >& xHandler,
                      OUString&                                       rOutPwd,
                      bool                                                 bFirstTry,
                      const OUString&                                 rDocName
                      );

    void reportUnsupportedEncryptionFormat(
        css::uno::Reference<
            css::task::XInteractionHandler > const & handler);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
