/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outdevstate.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:20:31 $
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

#ifndef _CPPCANVAS_OUTDEVSTATE_HXX
#define _CPPCANVAS_OUTDEVSTATE_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

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

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef _VCL_FNTSTYLE_HXX
#include <vcl/fntstyle.hxx>
#endif
#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif


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
                transform.identity();
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
