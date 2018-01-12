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
#include <com/sun/star/rendering/XColorSpace.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>

#include <unordered_map>
#include <vector>
#include <boost/functional/hash.hpp>

// virtual resolution of the PDF OutputDev in dpi
#define PDFI_OUTDEV_RESOLUTION 7200

namespace com { namespace sun { namespace star { namespace task
{ class XInteractionHandler; }}}}

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

    inline double convPx2mmPrec2( double fPix )
    {
        return rtl_math_round( convPx2mm( fPix ), 2, rtl_math_RoundingMode_Floor );
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
            return static_cast<size_t>(rFont.familyName.hashCode())
                ^  size_t(rFont.isBold ? 0xd47be593 : 0)
                ^  size_t(rFont.isItalic ? 0x1efd51a1 : 0)
                ^  size_t(rFont.isUnderline ? 0xf6bd325a : 0)
                ^  size_t(rFont.isOutline ?  0x12345678 : 0)
                ^  size_t(rFont.size)
                ;
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
                return OUString("miter");
            case css::rendering::PathJoinType::ROUND:
                return OUString("round");
            case css::rendering::PathJoinType::BEVEL:
                return OUString("bevel");
            }
        }

        OUString GetLineCapString() const
        {
            switch (LineCap)
            {
            default:
            case css::rendering::PathCapType::BUTT:
                return OUString("butt");
            case css::rendering::PathCapType::ROUND:
                return OUString("round");
            case css::rendering::PathCapType::SQUARE:
                return OUString("square");
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
            return boost::hash_value(rGC.LineColor.Red)
                ^  boost::hash_value(rGC.LineColor.Green)
                ^  boost::hash_value(rGC.LineColor.Blue)
                ^  boost::hash_value(rGC.LineColor.Alpha)
                ^  boost::hash_value(rGC.FillColor.Red)
                ^  boost::hash_value(rGC.FillColor.Green)
                ^  boost::hash_value(rGC.FillColor.Blue)
                ^  boost::hash_value(rGC.FillColor.Alpha)
                ^  boost::hash_value(rGC.LineJoin)
                ^  boost::hash_value(rGC.LineCap)
                ^  boost::hash_value(rGC.BlendMode)
                ^  boost::hash_value(rGC.LineWidth)
                ^  boost::hash_value(rGC.Flatness)
                ^  boost::hash_value(rGC.MiterLimit)
                ^  rGC.DashArray.size()
                ^  boost::hash_value(rGC.FontId)
                ^  boost::hash_value(rGC.TextRenderMode)
                ^  boost::hash_value(rGC.Transformation.get( 0, 0 ))
                ^  boost::hash_value(rGC.Transformation.get( 1, 0 ))
                ^  boost::hash_value(rGC.Transformation.get( 0, 1 ))
                ^  boost::hash_value(rGC.Transformation.get( 1, 1 ))
                ^  boost::hash_value(rGC.Transformation.get( 0, 2 ))
                ^  boost::hash_value(rGC.Transformation.get( 1, 2 ))
                ^  boost::hash_value(rGC.Clip.count() ? rGC.Clip.getB2DPolygon(0).count() : 0)
                ;
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
