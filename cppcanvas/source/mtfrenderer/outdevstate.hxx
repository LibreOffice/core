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

#ifndef _CPPCANVAS_OUTDEVSTATE_HXX
#define _CPPCANVAS_OUTDEVSTATE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/rendering/StringContext.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <vcl/fntstyle.hxx>
#include <vcl/vclenum.hxx>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
