/*************************************************************************
 *
 *  $RCSfile: outdevstate.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 20:55:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_STRINGCONTEXT_HPP__
#include <drafts/com/sun/star/rendering/StringContext.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_RENDERSTATE_HPP__
#include <drafts/com/sun/star/rendering/RenderState.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XPOLYPOLYGON2D_HPP__
#include <drafts/com/sun/star/rendering/XPolyPolygon2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XCANVASFONT_HPP__
#include <drafts/com/sun/star/rendering/XCanvasFont.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_TEXTDIRECTION_HPP__
#include <drafts/com/sun/star/rendering/TextDirection.hpp>
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


namespace cppcanvas
{
    namespace internal
    {
        struct OutDevState
        {
            OutDevState() :
                clip(),
                xClipPoly(),

                lineColor(),
                fillColor(),
                textColor(),
                textFillColor(),
                textLineColor(),

                xFont(),
                transform(),
                fontTransform(),

                textEmphasisMarkStyle(EMPHASISMARK_NONE),
                textDirection(::drafts::com::sun::star::rendering::TextDirection::WEAK_LEFT_TO_RIGHT),
                textAlignment(0), // TODO(Q2): Synchronize with implrenderer
                                  // and possibly new rendering::TextAlignment
                textReliefStyle(RELIEF_NONE),
                textUnderlineStyle(UNDERLINE_NONE),
                textStrikeoutStyle(STRIKEOUT_NONE),

                isTextOutlineModeSet( false ),
                isTextEffectShadowSet( false ),
                isTextWordUnderlineSet( false ),

                isLineColorSet( false ),
                isFillColorSet( false ),
                isTextFillColorSet( false ),
                isTextLineColorSet( false )
            {
                transform.identity();
                fontTransform.identity();
            }

            ::basegfx::B2DPolyPolygon                                                               clip;
            ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D > xClipPoly;

            ::com::sun::star::uno::Sequence< double >                                               lineColor;
            ::com::sun::star::uno::Sequence< double >                                               fillColor;
            ::com::sun::star::uno::Sequence< double >                                               textColor;
            ::com::sun::star::uno::Sequence< double >                                               textFillColor;
            ::com::sun::star::uno::Sequence< double >                                               textLineColor;

            /** Current font.

                @attention Beware, this member can be NULL, and
                nevertheless text output is generated.
             */
            ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCanvasFont >    xFont;
            ::basegfx::B2DHomMatrix                                                                 transform;
            ::basegfx::B2DHomMatrix                                                                 fontTransform;

            sal_uInt16                                                                              textEmphasisMarkStyle;
            sal_Int8                                                                                textDirection;
            sal_Int8                                                                                textAlignment;
            sal_Int8                                                                                textReliefStyle;
            sal_Int8                                                                                textUnderlineStyle;
            sal_Int8                                                                                textStrikeoutStyle;

            bool                                                                                    isTextOutlineModeSet;
            bool                                                                                    isTextEffectShadowSet;
            bool                                                                                    isTextWordUnderlineSet;

            bool                                                                                    isLineColorSet;
            bool                                                                                    isFillColorSet;
            bool                                                                                    isTextFillColorSet;
            bool                                                                                    isTextLineColorSet;
        };
    }
}

#endif /* _CPPCANVAS_OUTDEVSTATE_HXX */
