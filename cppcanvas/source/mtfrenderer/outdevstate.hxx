/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
                textOverlineStyle(UNDERLINE_NONE),
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
            sal_Int8                                                                            textOverlineStyle;
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
