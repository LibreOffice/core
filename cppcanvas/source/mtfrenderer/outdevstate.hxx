/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: outdevstate.hxx,v $
 * $Revision: 1.9 $
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

#ifndef _CPPCANVAS_OUTDEVSTATE_HXX
#define _CPPCANVAS_OUTDEVSTATE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#ifndef _COM_SUN_STAR_RENDERING_STRINGCONTEXT_HPP__
#include <com/sun/star/rendering/StringContext.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_RENDERSTATE_HPP__
#include <com/sun/star/rendering/RenderState.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XPOLYPOLYGON2D_HPP__
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XCANVASFONT_HPP__
#include <com/sun/star/rendering/XCanvasFont.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_TEXTDIRECTION_HPP__
#include <com/sun/star/rendering/TextDirection.hpp>
#endif
#include <basegfx/matrix/b2dhommatrix.hxx>
#ifndef _BGFX_POLYGON_B2DPOLYPOLGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif
#include <vcl/fntstyle.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/outdev.hxx>
#include <vcl/outdev.hxx>


namespace cppcanvas
{
    namespace internal
    {
        struct OutDevState
        {
            OutDevState() :
                clip(),
                clipRect(),
                xClipPoly(),

                lineColor(),
                fillColor(),
                textColor(),
                textFillColor(),
                textLineColor(),

                xFont(),
                transform(),
                mapModeTransform(),
                fontRotation(0.0),

                textEmphasisMarkStyle(EMPHASISMARK_NONE),
                pushFlags(PUSH_ALL),
                textDirection(::com::sun::star::rendering::TextDirection::WEAK_LEFT_TO_RIGHT),
                textAlignment(0), // TODO(Q2): Synchronize with implrenderer
                                  // and possibly new rendering::TextAlignment
                textReliefStyle(RELIEF_NONE),
                textUnderlineStyle(UNDERLINE_NONE),
                textStrikeoutStyle(STRIKEOUT_NONE),
                textReferencePoint(ALIGN_BASELINE),

                isTextOutlineModeSet( false ),
                isTextEffectShadowSet( false ),
                isTextWordUnderlineSet( false ),

                isLineColorSet( false ),
                isFillColorSet( false ),
                isTextFillColorSet( false ),
                isTextLineColorSet( false )
            {
            }

            ::basegfx::B2DPolyPolygon                                                           clip;
            ::Rectangle                                                                         clipRect;
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >     xClipPoly;

            ::com::sun::star::uno::Sequence< double >                                           lineColor;
            ::com::sun::star::uno::Sequence< double >                                           fillColor;
            ::com::sun::star::uno::Sequence< double >                                           textColor;
            ::com::sun::star::uno::Sequence< double >                                           textFillColor;
            ::com::sun::star::uno::Sequence< double >                                           textLineColor;

            /** Current font.

                @attention Beware, this member can be NULL, and
                nevertheless text output is generated.
             */
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvasFont >        xFont;
            ::basegfx::B2DHomMatrix                                                             transform;
            ::basegfx::B2DHomMatrix                                                             mapModeTransform;
            double                                                                              fontRotation;

            sal_uInt16                                                                          textEmphasisMarkStyle;
            sal_uInt16                                                                          pushFlags;
            sal_Int8                                                                            textDirection;
            sal_Int8                                                                            textAlignment;
            sal_Int8                                                                            textReliefStyle;
            sal_Int8                                                                            textUnderlineStyle;
            sal_Int8                                                                            textStrikeoutStyle;
            TextAlign                                                                           textReferencePoint;

            bool                                                                                isTextOutlineModeSet;
            bool                                                                                isTextEffectShadowSet;
            bool                                                                                isTextWordUnderlineSet;

            bool                                                                                isLineColorSet;
            bool                                                                                isFillColorSet;
            bool                                                                                isTextFillColorSet;
            bool                                                                                isTextLineColorSet;
        };
    }
}

#endif /* _CPPCANVAS_OUTDEVSTATE_HXX */
