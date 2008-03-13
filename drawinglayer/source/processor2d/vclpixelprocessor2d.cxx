/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclpixelprocessor2d.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-13 08:22:03 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPIXELPROCESSOR2D_HXX
#include <drawinglayer/processor2d/vclpixelprocessor2d.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE_TEXTPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BITMAPPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLBITMAPPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_METAFILEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MODIFIEDCOLORPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ALPHAPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/alphaprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSFORMPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MARKERARRAYPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POINTRARRAYPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_WRONGSPELLPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CONTROLPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW2_HPP_
#include <com/sun/star/awt/XWindow2.hpp>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_UNIFIEDALPHAPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/unifiedalphaprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PAGEPREVIEWPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CHARTPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/chartprimitive2d.hxx>
#endif

#include <cstdio>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        VclPixelProcessor2D::VclPixelProcessor2D(const geometry::ViewInformation2D& rViewInformation, OutputDevice& rOutDev)
        :   VclProcessor2D(rViewInformation, rOutDev),
            maOriginalMapMode(rOutDev.GetMapMode())
        {
            // prepare maCurrentTransformation matrix with viewTransformation to target directly to pixels
            maCurrentTransformation = rViewInformation.getViewTransformation();

            // prepare output directly to pixels
               mpOutputDevice->Push(PUSH_MAPMODE);
            mpOutputDevice->SetMapMode();

            // react on AntiAliasing settings
            if(getOptionsDrawinglayer().IsAntiAliasing())
            {
                mpOutputDevice->SetAntialiasing(mpOutputDevice->GetAntialiasing() & ~ANTIALIASING_DISABLE_POLYGONS);
            }
            else
            {
                mpOutputDevice->SetAntialiasing(mpOutputDevice->GetAntialiasing() | ANTIALIASING_DISABLE_POLYGONS);
            }
        }

        VclPixelProcessor2D::~VclPixelProcessor2D()
        {
            // restore MapMode
               mpOutputDevice->Pop();

            // restore AntiAliasing
            mpOutputDevice->SetAntialiasing(mpOutputDevice->GetAntialiasing() | ANTIALIASING_DISABLE_POLYGONS);
        }

        void VclPixelProcessor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            switch(rCandidate.getPrimitiveID())
            {
                case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D :
                {
                    // directdraw of wrong spell primitive; added test possibility to check wrong spell decompose
                    static bool bHandleWrongSpellDirectly(true);

                    if(bHandleWrongSpellDirectly)
                    {
                        RenderWrongSpellPrimitive2D(static_cast< const primitive2d::WrongSpellPrimitive2D& >(rCandidate));
                    }
                    else
                    {
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }
                    break;
                }
                case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D :
                {
                    // directdraw of text simple portion; added test possibility to check text decompose
                    static bool bHandleSimpleTextDirectly(true);

                    // Adapt evtl. used special DrawMode
                    const sal_uInt32 nOriginalDrawMode(mpOutputDevice->GetDrawMode());
                    adaptTextToFillDrawMode();

                    if(bHandleSimpleTextDirectly)
                    {
                        RenderTextSimpleOrDecoratedPortionPrimitive2D(static_cast< const primitive2d::TextSimplePortionPrimitive2D& >(rCandidate));
                    }
                    else
                    {
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }

                    // restore DrawMode
                    mpOutputDevice->SetDrawMode(nOriginalDrawMode);

                    break;
                }
                case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D :
                {
                    // directdraw of text simple portion; added test possibility to check text decompose
                    static bool bHandleComplexTextDirectly(false);

                    // Adapt evtl. used special DrawMode
                    const sal_uInt32 nOriginalDrawMode(mpOutputDevice->GetDrawMode());
                    adaptTextToFillDrawMode();

                    if(bHandleComplexTextDirectly)
                    {
                        RenderTextSimpleOrDecoratedPortionPrimitive2D(static_cast< const primitive2d::TextSimplePortionPrimitive2D& >(rCandidate));
                    }
                    else
                    {
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }

                    // restore DrawMode
                    mpOutputDevice->SetDrawMode(nOriginalDrawMode);

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                {
                    // direct draw of hairline
                    RenderPolygonHairlinePrimitive2D(static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                {
                    // direct draw of transformed BitmapEx primitive
                    RenderBitmapPrimitive2D(static_cast< const primitive2d::BitmapPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_FILLBITMAPPRIMITIVE2D :
                {
                    // direct draw of fillBitmapPrimitive
                    RenderFillBitmapPrimitive2D(static_cast< const primitive2d::FillBitmapPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D :
                {
                    if(getOptionsDrawinglayer().IsAntiAliasing())
                    {
                        // For AA, direct render has to be avoided since it uses XOR maskings which will not
                        // work with AA. Instead, the decompose which uses MaskPrimitive2D with fillings is
                        // used
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }
                    else
                    {
                        // direct draw of gradient
                        RenderPolyPolygonGradientPrimitive2D(static_cast< const primitive2d::PolyPolygonGradientPrimitive2D& >(rCandidate));
                    }
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                {
                    // direct draw of PolyPolygon with color
                    RenderPolyPolygonColorPrimitive2D(static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                {
                    // direct draw of MetaFile
                    RenderMetafilePrimitive2D(static_cast< const primitive2d::MetafilePrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
                {
                    // mask group.
                    RenderMaskPrimitive2DPixel(static_cast< const primitive2d::MaskPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D :
                {
                    // modified color group. Force output to unified color.
                    RenderModifiedColorPrimitive2D(static_cast< const primitive2d::ModifiedColorPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_UNIFIEDALPHAPRIMITIVE2D :
                {
                    // Detect if a single PolyPolygonColorPrimitive2D is contained; in that case,
                    // use the faster OutputDevice::DrawTransparent method
                    const primitive2d::UnifiedAlphaPrimitive2D& rUniAlphaCandidate = static_cast< const primitive2d::UnifiedAlphaPrimitive2D& >(rCandidate);
                    const primitive2d::Primitive2DSequence rContent = rUniAlphaCandidate.getChildren();
                    bool bDrawTransparentUsed(false);

                    // ATM need to disable this since OutputDevice::DrawTransparent uses the
                    // old tools::Polygon classes and may not be sufficient here. HDU is evaluating...
                    static bool bAllowUsingDrawTransparent(false);

                    if(bAllowUsingDrawTransparent && rContent.hasElements() && 1 == rContent.getLength())
                    {
                        const primitive2d::Primitive2DReference xReference(rContent[0]);
                        const primitive2d::PolyPolygonColorPrimitive2D* pPoPoColor = dynamic_cast< const primitive2d::PolyPolygonColorPrimitive2D* >(xReference.get());

                        if(pPoPoColor && PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D == pPoPoColor->getPrimitiveID())
                        {
                            // single transparent PolyPolygon identified, use directly
                            const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(pPoPoColor->getBColor()));
                            mpOutputDevice->SetFillColor(Color(aPolygonColor));
                            mpOutputDevice->SetLineColor();

                            basegfx::B2DPolyPolygon aLocalPolyPolygon(pPoPoColor->getB2DPolyPolygon());
                            aLocalPolyPolygon.transform(maCurrentTransformation);

                            mpOutputDevice->DrawTransparent(aLocalPolyPolygon, rUniAlphaCandidate.getAlpha());
                            bDrawTransparentUsed = true;
                        }
                    }

                    if(!bDrawTransparentUsed)
                    {
                        // use decomposition
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }

                    break;
                }
                case PRIMITIVE2D_ID_ALPHAPRIMITIVE2D :
                {
                    // sub-transparence group. Draw to VDev first.
                    RenderAlphaPrimitive2D(static_cast< const primitive2d::AlphaPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D :
                {
                    // transform group.
                    RenderTransformPrimitive2D(static_cast< const primitive2d::TransformPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D :
                {
                    // new XDrawPage for ViewInformation2D
                    RenderPagePreviewPrimitive2D(static_cast< const primitive2d::PagePreviewPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D :
                {
                    // marker array
                    RenderMarkerArrayPrimitive2D(static_cast< const primitive2d::MarkerArrayPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                {
                    // point array
                    RenderPointArrayPrimitive2D(static_cast< const primitive2d::PointArrayPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D :
                {
                    // control primitive
                    const primitive2d::ControlPrimitive2D& rControlPrimitive = static_cast< const primitive2d::ControlPrimitive2D& >(rCandidate);

                    // if control primitive is a xWindow2 and visible, it oes not need to be painted
                    bool bControlIsVisibleAsChildWindow(false);

                    if(rControlPrimitive.getXControl().is())
                    {
                        com::sun::star::uno::Reference< com::sun::star::awt::XWindow2 > xControlWindow(rControlPrimitive.getXControl(), com::sun::star::uno::UNO_QUERY_THROW);

                        if(xControlWindow.is())
                        {
                            if(xControlWindow->isVisible())
                            {
                                bControlIsVisibleAsChildWindow = true;
                            }
                        }
                    }

                    if(bControlIsVisibleAsChildWindow)
                    {
                        // update position and size as VCL Child Window
                        static bool bDoSizeAndPositionControlsB(false);

                        if(bDoSizeAndPositionControlsB)
                        {
                            PositionAndSizeControl(rControlPrimitive);
                        }
                    }
                    else
                    {
                        // process recursively and use the decomposition as Bitmap
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
                {
                    // the stroke primitive may be decomposed to filled polygons. To keep
                    // evtl. set DrawModes aka DRAWMODE_BLACKLINE, DRAWMODE_GRAYLINE,
                    // DRAWMODE_GHOSTEDLINE, DRAWMODE_WHITELINE or DRAWMODE_SETTINGSLINE
                    // working, these need to be copied to the corresponding fill modes
                    const sal_uInt32 nOriginalDrawMode(mpOutputDevice->GetDrawMode());
                    adaptLineToFillDrawMode();

                    // polygon stroke primitive
                    static bool bSuppressFatToHairlineCorrection(false);

                    if(getOptionsDrawinglayer().IsAntiAliasing() || bSuppressFatToHairlineCorrection)
                    {
                        // remeber that we enter a PolygonStrokePrimitive2D decomposition,
                        // used for AA thick line drawing
                        mnPolygonStrokePrimitive2D++;

                        // with AA there is no need to handle thin lines special
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));

                        // leave PolygonStrokePrimitive2D
                        mnPolygonStrokePrimitive2D--;
                    }
                    else
                    {
                        // Lines with 1 and 2 pixel width without AA need special treatment since their vsiualisation
                        // as filled polygons is geometrically corret but looks wrong since polygon filling avoids
                        // the right and bottom pixels. The used method evaluates that and takes the correct action,
                        // including calling recursively with decomposition if line is wide enough
                        const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokePrimitive = static_cast< const primitive2d::PolygonStrokePrimitive2D& >(rCandidate);

                        RenderPolygonStrokePrimitive2D(rPolygonStrokePrimitive);
                    }

                    // restore DrawMode
                    mpOutputDevice->SetDrawMode(nOriginalDrawMode);

                    break;
                }
                case PRIMITIVE2D_ID_CHARTPRIMITIVE2D :
                {
                    // chart primitive in pixel renderer; restore original DrawMode during call
                    // since the evtl. used ChartPrettyPainter will use the MapMode
                       mpOutputDevice->Push(PUSH_MAPMODE);
                    mpOutputDevice->SetMapMode(maOriginalMapMode);
                    RenderChartPrimitive2D(static_cast< const primitive2d::ChartPrimitive2D& >(rCandidate));
                       mpOutputDevice->Pop();
                    break;
                }
                default :
                {
                    // process recursively
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    break;
                }
            }
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
