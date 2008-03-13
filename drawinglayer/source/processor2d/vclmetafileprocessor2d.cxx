/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclmetafileprocessor2d.cxx,v $
 *
 *  $Revision: 1.21 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLMETAFILEPROCESSOR2D_HXX
#include <drawinglayer/processor2d/vclmetafileprocessor2d.hxx>
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _SV_GDIMTF_HXX
#include <vcl/gdimtf.hxx>
#endif

#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX
#include <drawinglayer/attribute/fillattribute.hxx>
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_METAFILEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONCLIPPER_HXX
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MODIFIEDCOLORPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_UNIFIEDALPHAPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/unifiedalphaprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ALPHAPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/alphaprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLGRADIENTPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPIXELPROCESSOR2D_HXX
#include <drawinglayer/processor2d/vclpixelprocessor2d.hxx>
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
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

#ifndef _VCL_GRAPHICTOOLS_HXX_
#include <vcl/graphictools.hxx>
#endif

#ifndef _SV_METAACT_HXX
#include <vcl/metaact.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTHIERARCHYPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE_TEXTDECORATEDPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_CHARACTERITERATORMODE_HDL_
#include <com/sun/star/i18n/CharacterIteratorMode.hdl>
#endif

#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CONTROLPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRAPHICPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PAGEPREVIEWPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// for PDFExtOutDevData Graphic support

#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _TOOLKIT_HELPER_FORM_FORMPDFEXPORT_HXX
#include <toolkit/helper/formpdfexport.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// for Control printing

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//////////////////////////////////////////////////////////////////////////////
// for current chart PrettyPrinting support

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CHARTPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/chartprimitive2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// for StructureTagPrimitive support in sd's unomodel.cxx

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_STRUCTURETAGPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/structuretagprimitive2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        //////////////////////////////////////////////////////////////////////////////
        // UNO usings
        using ::com::sun::star::uno::Reference;
        using ::com::sun::star::uno::UNO_QUERY;
        using ::com::sun::star::uno::UNO_QUERY_THROW;
        using ::com::sun::star::uno::Exception;
        using ::com::sun::star::beans::XPropertySet;
        using ::com::sun::star::beans::XPropertySetInfo;
        using ::com::sun::star::awt::XView;
        //using ::com::sun::star::awt::XGraphics;
        //using ::com::sun::star::awt::XWindow;
        //using ::com::sun::star::awt::PosSize::POSSIZE;

        Rectangle VclMetafileProcessor2D::impDumpToMetaFile(
            const primitive2d::Primitive2DSequence& rContent,
            GDIMetaFile& o_rContentMetafile)
        {
            // Prepare VDev, MetaFile and connections
            OutputDevice* pLastOutputDevice = mpOutputDevice;
            basegfx::B2DRange aPrimitiveRange(primitive2d::getB2DRangeFromPrimitive2DSequence(rContent, getViewInformation2D()));

            // transform primitive range with current transformation (e.g shadow offset)
            aPrimitiveRange.transform(maCurrentTransformation);

            const Rectangle aPrimitiveRectangle(
                basegfx::fround(aPrimitiveRange.getMinX()), basegfx::fround(aPrimitiveRange.getMinY()),
                basegfx::fround(aPrimitiveRange.getMaxX()), basegfx::fround(aPrimitiveRange.getMaxY()));
            VirtualDevice aContentVDev;
            MapMode aNewMapMode(pLastOutputDevice->GetMapMode());

            mpOutputDevice = &aContentVDev;
            aContentVDev.EnableOutput(false);
            aContentVDev.SetMapMode(pLastOutputDevice->GetMapMode());
            o_rContentMetafile.Record(&aContentVDev);
            aContentVDev.SetLineColor(pLastOutputDevice->GetLineColor());
            aContentVDev.SetFillColor(pLastOutputDevice->GetFillColor());
            aContentVDev.SetFont(pLastOutputDevice->GetFont());
            aContentVDev.SetDrawMode(pLastOutputDevice->GetDrawMode());
            aContentVDev.SetSettings(pLastOutputDevice->GetSettings());
            aContentVDev.SetRefPoint(pLastOutputDevice->GetRefPoint());

            // dump to MetaFile
            process(rContent);

            // cleanups
            o_rContentMetafile.Stop();
            o_rContentMetafile.WindStart();
            aNewMapMode.SetOrigin(aPrimitiveRectangle.TopLeft());
            o_rContentMetafile.SetPrefMapMode(aNewMapMode);
            o_rContentMetafile.SetPrefSize(aPrimitiveRectangle.GetSize());
            mpOutputDevice = pLastOutputDevice;

            return aPrimitiveRectangle;
        }

        void VclMetafileProcessor2D::impConvertFillGradientAttributeToVCLGradient(
            Gradient& o_rVCLGradient,
            const attribute::FillGradientAttribute& rFiGrAtt,
            bool bIsTransparenceGradient)
        {
            if(bIsTransparenceGradient)
            {
                // it's about alpha channel intensities (black/white), do not use color modifier
                o_rVCLGradient.SetStartColor(Color(rFiGrAtt.getStartColor()));
                o_rVCLGradient.SetEndColor(Color(rFiGrAtt.getEndColor()));
            }
            else
            {
                // use color modifier to influence start/end color of gradient
                o_rVCLGradient.SetStartColor(Color(maBColorModifierStack.getModifiedColor(rFiGrAtt.getStartColor())));
                o_rVCLGradient.SetEndColor(Color(maBColorModifierStack.getModifiedColor(rFiGrAtt.getEndColor())));
            }

            o_rVCLGradient.SetAngle(static_cast< sal_uInt16 >(rFiGrAtt.getAngle() * (1.0 / F_PI1800)));
            o_rVCLGradient.SetBorder(static_cast< sal_uInt16 >(rFiGrAtt.getBorder() * 100.0));
            o_rVCLGradient.SetOfsX(static_cast< sal_uInt16 >(rFiGrAtt.getOffsetX() * 100.0));
            o_rVCLGradient.SetOfsY(static_cast< sal_uInt16 >(rFiGrAtt.getOffsetY() * 100.0));
            o_rVCLGradient.SetSteps(rFiGrAtt.getSteps());

            // defaults for intensity; those were computed into the start/end colors already
            o_rVCLGradient.SetStartIntensity(100);
            o_rVCLGradient.SetEndIntensity(100);

            switch(rFiGrAtt.getStyle())
            {
                default : // attribute::GRADIENTSTYLE_LINEAR :
                {
                    o_rVCLGradient.SetStyle(GRADIENT_LINEAR);
                    break;
                }
                case attribute::GRADIENTSTYLE_AXIAL :
                {
                    o_rVCLGradient.SetStyle(GRADIENT_AXIAL);
                    break;
                }
                case attribute::GRADIENTSTYLE_RADIAL :
                {
                    o_rVCLGradient.SetStyle(GRADIENT_RADIAL);
                    break;
                }
                case attribute::GRADIENTSTYLE_ELLIPTICAL :
                {
                    o_rVCLGradient.SetStyle(GRADIENT_ELLIPTICAL);
                    break;
                }
                case attribute::GRADIENTSTYLE_SQUARE :
                {
                    o_rVCLGradient.SetStyle(GRADIENT_SQUARE);
                    break;
                }
                case attribute::GRADIENTSTYLE_RECT :
                {
                    o_rVCLGradient.SetStyle(GRADIENT_RECT);
                    break;
                }
            }
        }

        void VclMetafileProcessor2D::impStartSvtGraphicFill(SvtGraphicFill* pSvtGraphicFill)
        {
            if(pSvtGraphicFill && !mnSvtGraphicFillCount)
            {
                SvMemoryStream aMemStm;

                aMemStm << *pSvtGraphicFill;
                mrMetaFile.AddAction(new MetaCommentAction("XPATHFILL_SEQ_BEGIN", 0, static_cast< const BYTE* >(aMemStm.GetData()), aMemStm.Seek(STREAM_SEEK_TO_END)));
                mnSvtGraphicFillCount++;
            }
        }

        void VclMetafileProcessor2D::impEndSvtGraphicFill(SvtGraphicFill* pSvtGraphicFill)
        {
            if(pSvtGraphicFill && mnSvtGraphicFillCount)
            {
                mnSvtGraphicFillCount--;
                mrMetaFile.AddAction(new MetaCommentAction("XPATHFILL_SEQ_END"));
                delete pSvtGraphicFill;
            }
        }

        SvtGraphicStroke* VclMetafileProcessor2D::impTryToCreateSvtGraphicStroke(
            const basegfx::B2DPolygon& rB2DPolygon,
            const basegfx::BColor* pColor,
            const attribute::LineAttribute* pLineAttribute,
            const attribute::StrokeAttribute* pStrokeAttribute,
            const attribute::LineStartEndAttribute* pStart,
            const attribute::LineStartEndAttribute* pEnd)
        {
            SvtGraphicStroke* pRetval = 0;

            if(rB2DPolygon.count() && !mnSvtGraphicStrokeCount)
            {
                basegfx::BColor aStrokeColor;
                PolyPolygon aStartPolyPolygon;
                PolyPolygon aEndPolyPolygon;

                if(pColor)
                {
                    aStrokeColor = *pColor;
                }
                else if(pLineAttribute)
                {
                    aStrokeColor = maBColorModifierStack.getModifiedColor(pLineAttribute->getColor());
                }

                // It IS needed to record the stroke color at all in the metafile,
                // SvtGraphicStroke has NO entry for stroke color(!)
                mpOutputDevice->SetLineColor(Color(aStrokeColor));

                if(!rB2DPolygon.isClosed())
                {
                    double fPolyLength(0.0);

                    if(pStart && pStart->isActive())
                    {
                        fPolyLength = basegfx::tools::getLength(rB2DPolygon);

                        const basegfx::B2DPolyPolygon aStartArrow(basegfx::tools::createAreaGeometryForLineStartEnd(
                            rB2DPolygon, pStart->getB2DPolyPolygon(), true, pStart->getWidth(),
                            fPolyLength, pStart->isCentered() ? 0.5 : 0.0, 0));

                        aStartPolyPolygon = PolyPolygon(aStartArrow);
                    }

                    if(pEnd && pEnd->isActive())
                    {
                        if(basegfx::fTools::equalZero(fPolyLength))
                        {
                            fPolyLength = basegfx::tools::getLength(rB2DPolygon);
                        }

                        const basegfx::B2DPolyPolygon aEndArrow(basegfx::tools::createAreaGeometryForLineStartEnd(
                            rB2DPolygon, pEnd->getB2DPolyPolygon(), false, pEnd->getWidth(),
                            fPolyLength, pEnd->isCentered() ? 0.5 : 0.0, 0));

                        aEndPolyPolygon = PolyPolygon(aEndArrow);
                    }
                }

                SvtGraphicStroke::JoinType eJoin(SvtGraphicStroke::joinNone);
                double fLineWidth(0.0);
                double fMiterLength(0.0);
                SvtGraphicStroke::DashArray aDashArray;

                if(pLineAttribute)
                {
                    // pre-fill fLineWidth
                    fLineWidth = pLineAttribute->getWidth();

                    // pre-fill fMiterLength
                    fMiterLength = fLineWidth;

                    // get Join
                    switch(pLineAttribute->getLineJoin())
                    {
                        default : // basegfx::B2DLINEJOIN_NONE :
                        {
                            eJoin = SvtGraphicStroke::joinNone;
                            break;
                        }
                        case basegfx::B2DLINEJOIN_BEVEL :
                        {
                            eJoin = SvtGraphicStroke::joinBevel;
                            break;
                        }
                        case basegfx::B2DLINEJOIN_MIDDLE :
                        case basegfx::B2DLINEJOIN_MITER :
                        {
                            eJoin = SvtGraphicStroke::joinMiter;
                            // ATM 15 degrees is assumed
                            fMiterLength /= rtl::math::sin(M_PI * (15.0 / 360.0));
                            break;
                        }
                        case basegfx::B2DLINEJOIN_ROUND :
                        {
                            eJoin = SvtGraphicStroke::joinRound;
                            break;
                        }
                    }
                }

                if(pStrokeAttribute)
                {
                    // copy dash array
                    aDashArray = pStrokeAttribute->getDotDashArray();
                }

                pRetval = new SvtGraphicStroke(
                    Polygon(rB2DPolygon),
                    aStartPolyPolygon,
                    aEndPolyPolygon,
                    mfCurrentUnifiedTransparence,
                    fLineWidth,
                    SvtGraphicStroke::capButt,
                    eJoin,
                    fMiterLength,
                    aDashArray);
            }

            return pRetval;
        }

        void VclMetafileProcessor2D::impStartSvtGraphicStroke(SvtGraphicStroke* pSvtGraphicStroke)
        {
            if(pSvtGraphicStroke && !mnSvtGraphicStrokeCount)
            {
                SvMemoryStream aMemStm;

                aMemStm << *pSvtGraphicStroke;
                mrMetaFile.AddAction(new MetaCommentAction("XPATHSTROKE_SEQ_BEGIN", 0, static_cast< const BYTE* >(aMemStm.GetData()), aMemStm.Seek(STREAM_SEEK_TO_END)));
                mnSvtGraphicStrokeCount++;
            }
        }

        void VclMetafileProcessor2D::impEndSvtGraphicStroke(SvtGraphicStroke* pSvtGraphicStroke)
        {
            if(pSvtGraphicStroke && mnSvtGraphicStrokeCount)
            {
                mnSvtGraphicStrokeCount--;
                mrMetaFile.AddAction(new MetaCommentAction("XPATHSTROKE_SEQ_END"));
                delete pSvtGraphicStroke;
            }
        }

        // init static break iterator
        Reference< ::com::sun::star::i18n::XBreakIterator > VclMetafileProcessor2D::mxBreakIterator;

        VclMetafileProcessor2D::VclMetafileProcessor2D(const geometry::ViewInformation2D& rViewInformation, OutputDevice& rOutDev)
        :   VclProcessor2D(rViewInformation, rOutDev),
            mrMetaFile(*rOutDev.GetConnectMetaFile()),
            mnSvtGraphicFillCount(0),
            mnSvtGraphicStrokeCount(0),
            mfCurrentUnifiedTransparence(0.0),
            mpPDFExtOutDevData(dynamic_cast< vcl::PDFExtOutDevData* >(rOutDev.GetExtOutDevData()))
        {
            OSL_ENSURE(rOutDev.GetConnectMetaFile(), "VclMetafileProcessor2D: Used on OutDev which has no MetaFile Target (!)");
            // draw to logic coordinates, do not initialize maCurrentTransformation to viewTransformation,
            // do not change MapMode of destination
        }

        VclMetafileProcessor2D::~VclMetafileProcessor2D()
        {
            // MapMode was not changed, no restore necessary
        }

        /***********************************************************************************************

            Support of MetaCommentActions in the VclMetafileProcessor2D
            Found MetaCommentActions and how they are supported:

            XGRAD_SEQ_BEGIN, XGRAD_SEQ_END:

            Used inside OutputDevice::DrawGradient to mark the start and end of a MetaGradientEx action.
            It is used in various exporters/importers to have direct access to the gradient before it
            is rendered by VCL (and thus fragmented to polygon color actions and others). On that base, e.g.
            the Metafile to SdrObject import creates it's gradient objects.
            Best (and safest) way to support it here is to use PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D,
            map it back to the corresponding tools PolyPolygon and the Gradient and just call
            OutputDevice::DrawGradient which creates the necessary compatible actions.

            XPATHFILL_SEQ_BEGIN, XPATHFILL_SEQ_END:

            Two producers, one is vcl/source/gdi/gdimtf.cxx, line 1273. There, it is transformed
            inside GDIMetaFile::Rotate, nothing to take care of here.
            The second producer is in graphics/svx/source/svdraw/impgrfll.cxx, line 374. This is used
            with each incarnation of Imp_GraphicFill when a metafile is recorded, fillstyle is not
            XFILL_NONE and not completely transparent. It creates a SvtGraphicFill and streams it
            to the comment action. A closing end token is created in the destructor.
            Usages of Imp_GraphicFill are in Do_Paint_Object-methods of SdrCircObj, SdrPathObj and
            SdrRectObj.
            The token users pick various actions from SvtGraphicFill, so it may need to be added for all kind
            of filled objects, even simple colored polygons. It is added as extra information; the
            Metafile actions between the two tokens are interpreted as output generated from those
            fills. Thus, users have the choice to use the SvtGraphicFill info or the created output
            actions.
            Even for XFillTransparenceItem it is used, thus it may need to be supported in
            UnifiedAlphaPrimitive2D, too, when interpreted as normally filled PolyPolygon.
            Implemented for:
                PRIMITIVE2D_ID_POLYPOLYGONBITMAPPRIMITIVE2D,
                PRIMITIVE2D_ID_POLYPOLYGONHATCHPRIMITIVE2D,
                PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D,
                PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D,
                and for PRIMITIVE2D_ID_UNIFIEDALPHAPRIMITIVE2D when detected unified alpha

            XPATHSTROKE_SEQ_BEGIN, XPATHSTROKE_SEQ_END:

            Similar to pathfill, but using SvtGraphicStroke instead. It also has two producers where one
            is also the GDIMetaFile::Rotate. Another user is MetaCommentAction::Move which modifies the
            contained path accordingly.
            The other one is SdrObject::Imp_DrawLineGeometry. It's done when MetaFile is set at OutDev and
            only when geometry is a single polygon (!). I see no reason for that; in the PS exporter this
            would hinder to make use of PolyPolygon strokes. I will need to add support at:
                PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D
                PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D
                PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D
            This can be done hierarchical, too.
            Okay, base implementation done based on those three primitives.

            FIELD_SEQ_BEGIN, FIELD_SEQ_END

            Used from slideshow for URLs, created from diverse SvxField implementations inside
            createBeginComment()/createEndComment(). createBeginComment() is used from editeng\impedit3.cxx
            inside ImpEditEngine::Paint.
            Created TextHierarchyFieldPrimitive2D and added needed infos there; it is an group primitive and wraps
            text primitives (but is not limited to that). It contains the field type if special actions for the
            support of FIELD_SEQ_BEGIN/END are needed; this is the case for Page and URL fields. If more is
            needed, it may be supported there.
            FIELD_SEQ_BEGIN;PageField
            FIELD_SEQ_END
            Okay, these are now completely supported by TextHierarchyFieldPrimitive2D. URL works, too.

            XTEXT

            XTEXT_EOC(i) end of character
            XTEXT_EOW(i) end of word
            XTEXT_EOS(i) end of sentence

            this three are with index and are created with the help of a i18n::XBreakIterator in
            ImplDrawWithComments. Simplifying, moving out text painting, reworking to create some
            data structure for holding those TEXT infos.
            Supported directly by TextSimplePortionPrimitive2D with adding a Locale to the basic text
            primitive. In the MetaFileRenderer, the creation is now done (see below). This has the advantage
            that this creations do not need to be done for all paints all the time. This would be
            expensive since the BreakIterator and it's usage is expensive and for each paint also the
            whole character stops would need to be created.
            Created only for TextDecoratedPortionPrimitive2D due to XTEXT_EOL and XTEXT_EOP (see below)

            XTEXT_EOL() end of line
            XTEXT_EOP() end of paragraph

            First try with boolean marks at TextDecoratedPortionPrimitive2D did not work too well,
            i decided to solve it with structure. I added the TextHierarchyPrimitives for this,
            namely:
            - TextHierarchyLinePrimitive2D: Encapsulates single line
            - TextHierarchyParagraphPrimitive2D: Encapsulates single paragraph
            - TextHierarchyBlockPrimitive2D: encapsulates object texts (only one ATM)
            Those are now supported in hierarchy. This means the MetaFile renderer will support them
            by using them, reculrively using their content and adding MetaFile comments as needed.
            This also means that when another text layouter will be used it will be necessary to
            create/support the same HierarchyPrimitives to support users.
            To transport the information using this hierarchy is best suited to all future needs;
            the slideshow will be able to profit from it directly when using primitives; all other
            renderers not interested in the text structure will just ignore the encapsulations.

            XTEXT_PAINTSHAPE_BEGIN, XTEXT_PAINTSHAPE_END
            Supported now by the TextHierarchyBlockPrimitive2D.

            EPSReplacementGraphic:
            Only used in goodies\source\filter.vcl\ieps\ieps.cxx and svx\source\xml\xmlgrhlp.cxx to
            hold the original EPS which was imported in the same MetaFile as first 2 entries. Only
            used to export the original again (if exists).
            Not necessary to support with MetaFuleRenderer.

            XTEXT_SCROLLRECT, XTEXT_PAINTRECT
            Currently used to get extra MetaFile infos using GraphicExporter which again uses
            SdrTextObj::GetTextScrollMetaFileAndRectangle(). ATM works with primitives since
            the rectangle data is added directly by the GraphicsExporter as comment. Does not need
            to be adapted at once.
            When adapting later, the only user - the diashow - should directly use the provided
            Anination infos in the appropriate primitives (e.g. AnimatedSwitchPrimitive2D)

            PRNSPOOL_TRANSPARENTBITMAP_BEGIN, PRNSPOOL_TRANSPARENTBITMAP_END
            VCL usage when printing PL -> THB. Okay, THB confirms that it is only used as
            a fix (hack) while VCL printing. It is needed to not downscale a bitmap which
            was explicitely created for the printer already again to some default maximum
            bitmap sizes.
            Nothing to do here for the primitive renderer.

            Support for vcl::PDFExtOutDevData:
            PL knows that SJ did that stuff, it's used to hold a pointer to PDFExtOutDevData at
            the OutDev. When set, some extra data is written there. Trying simple PDF export and
            watching if i get those infos.
            Well, a PDF export does not use e.g. ImpEditEngine::Paint since the PdfFilter uses
            the SdXImpressDocument::render and thus uses the VclMetafileProcessor2D. I will check
            if i get a PDFExtOutDevData at the target output device.
            Indeed, i get one. Checking what all may be done when that extra-device-info is there.

            All in all i have to talk to SJ. I will need to emulate some of those actions, but
            i need to discuss which ones.
            In the future, all those infos would be taken from the primitive sequence anyways,
            thus these extensions would potentially be temporary, too.
            Discussed with SJ, added the necessary support and tested it. Details follow.

            - In ImpEditEngine::Paint, paragraph infos and URL stuff is added.
              Added in primitive MetaFile renderer.
              Checking URL: Indeed, current version exports it, but it is missing in primitive
              CWS version. Adding support.
              Okay, URLs work. Checked, Done.

            - UnoControlPDFExportContact is only created when PDFExtOutDevData is used at the
              target and uno control data is created in UnoControlPDFExportContact::do_PaintObject.
              This may be added in primitive MetaFile renderer.
              Adding support...
              OOps, the necessary helper stuff is in svx/source/form/formpdxexport.cxx in namespace
              svxform. Have to talk to FS if this has to be like that. Especially since
              ::vcl::PDFWriter::AnyWidget is filled out, which is already part of vcl.
              Wrote an eMail to FS, he is on vacation currently. I see no reason why not to move
              that stuff to somewhere else, maybe tools or svtools ?!? We will see...
              Moved to toolkit, so i have to link against it. I tried VCL first, but it did
              not work since VCLUnoHelper::CreateFont is unresolved in VCL (!). Other then the name
              may imply, it is defined in toolkit (!). Since toolkit is linked against VCL itself,
              the lowest move,ment plave is toolkit.
              Checked form control export, it works well. Done.

            - In goodies, in GraphicObject::Draw, when the used Graphic is linked, infos are
              generated. I will need to check what happens here with primitives.
              To support, use of GraphicPrimitive2D (PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D) may be needed.
              Added support, but feature is broken in main version, so i cannot test at all.
              Writing a bug to CL (or SJ) and seeing what happens (#i80380#).
              SJ took a look and we got it working. Tested VCL MetaFile Renderer based export,
              as intended, the original file is exported. Works, Done.




            To be done:

            - Maybe there are more places to take care of for vcl::PDFExtOutDevData!



        ****************************************************************************************************/

        void VclMetafileProcessor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            switch(rCandidate.getPrimitiveID())
            {
                case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D :
                {
                    // directdraw of wrong spell primitive
                    // Ignore for VclMetafileProcessor2D, this is for printing and MetaFile recording only
                    break;
                }
                case PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D :
                {
                    const primitive2d::GraphicPrimitive2D& rGraphicPrimitive = static_cast< const primitive2d::GraphicPrimitive2D& >(rCandidate);
                    bool bUsingPDFExtOutDevData(false);
                    basegfx::B2DVector aTranslate, aScale;
                    static bool bSuppressPDFExtOutDevDataSupport(false);

                    if(mpPDFExtOutDevData && !bSuppressPDFExtOutDevDataSupport)
                    {
                        // emulate data handling from UnoControlPDFExportContact, original see
                        // goodies/source/graphic/grfmgr.cxx
                        const Graphic& rGraphic = rGraphicPrimitive.getGraphicObject().GetGraphic();

                        if(rGraphic.IsLink())
                        {
                            const GraphicAttr& rAttr = rGraphicPrimitive.getGraphicAttr();

                            if(!rAttr.IsSpecialDrawMode() && !rAttr.IsMirrored() && !rAttr.IsRotated() && !rAttr.IsAdjusted())
                            {
                                const basegfx::B2DHomMatrix& rTransform = rGraphicPrimitive.getTransform();
                                double fRotate, fShearX;
                                rTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                                if(aScale.getX() > 0.0 && aScale.getY() > 0.0)
                                {
                                    bUsingPDFExtOutDevData = true;
                                    mpPDFExtOutDevData->BeginGroup();
                                }
                            }
                        }
                    }

                    // process recursively and add MetaFile comment
                    process(rGraphicPrimitive.get2DDecomposition(getViewInformation2D()));

                    if(bUsingPDFExtOutDevData)
                    {
                        // emulate data handling from UnoControlPDFExportContact, original see
                        // goodies/source/graphic/grfmgr.cxx
                        const basegfx::B2DRange aCurrentRange(
                            aTranslate.getX(), aTranslate.getY(),
                            aTranslate.getX() + aScale.getX(), aTranslate.getY() + aScale.getY());
                        const Rectangle aCurrentRect(
                            sal_Int32(floor(aCurrentRange.getMinX())), sal_Int32(floor(aCurrentRange.getMinY())),
                            sal_Int32(ceil(aCurrentRange.getMaxX())), sal_Int32(ceil(aCurrentRange.getMaxY())));
                        const GraphicAttr& rAttr = rGraphicPrimitive.getGraphicAttr();
                        Rectangle aCropRect;

                        if(rAttr.IsCropped())
                        {
                            // calculate scalings between real image size and logic object size. This
                            // is necessary since the crop values are relative to original bitmap size
                            double fFactorX(1.0);
                            double fFactorY(1.0);

                            {
                                const MapMode aMapMode100thmm(MAP_100TH_MM);
                                const Size aBitmapSize(Application::GetDefaultDevice()->LogicToLogic(
                                    rGraphicPrimitive.getGraphicObject().GetPrefSize(),
                                    rGraphicPrimitive.getGraphicObject().GetPrefMapMode(), aMapMode100thmm));
                                const double fDivX(aBitmapSize.Width() - rAttr.GetLeftCrop() - rAttr.GetRightCrop());
                                const double fDivY(aBitmapSize.Height() - rAttr.GetTopCrop() - rAttr.GetBottomCrop());

                                if(!basegfx::fTools::equalZero(fDivX))
                                {
                                    fFactorX = aScale.getX() / fDivX;
                                }

                                if(!basegfx::fTools::equalZero(fDivY))
                                {
                                    fFactorY = aScale.getY() / fDivY;
                                }
                            }

                            // calculate crop range and rect
                            basegfx::B2DRange aCropRange;
                            aCropRange.expand(aCurrentRange.getMinimum() - basegfx::B2DPoint(rAttr.GetLeftCrop() * fFactorX, rAttr.GetTopCrop() * fFactorY));
                            aCropRange.expand(aCurrentRange.getMaximum() + basegfx::B2DPoint(rAttr.GetRightCrop() * fFactorX, rAttr.GetBottomCrop() * fFactorY));

                            aCropRect = Rectangle(
                                sal_Int32(floor(aCropRange.getMinX())), sal_Int32(floor(aCropRange.getMinY())),
                                sal_Int32(ceil(aCropRange.getMaxX())), sal_Int32(ceil(aCropRange.getMaxY())));
                        }

                        mpPDFExtOutDevData->EndGroup(rGraphicPrimitive.getGraphicObject().GetGraphic(),
                            rAttr.GetTransparency(),
                            aCurrentRect,
                            aCropRect);
                    }

                    break;
                }
                case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D :
                {
                    const primitive2d::ControlPrimitive2D& rControlPrimitive = static_cast< const primitive2d::ControlPrimitive2D& >(rCandidate);
                    bool bDoProcessRecursively(true);
                    static bool bSuppressPDFExtOutDevDataSupport(false);
                    static bool bSuppressPrinterOutput(false);

                    if(mpPDFExtOutDevData && !bSuppressPDFExtOutDevDataSupport && mpPDFExtOutDevData->GetIsExportFormFields())
                    {
                        // emulate data handling from UnoControlPDFExportContact
                        // I have now moved describePDFControl to toolkit, thus i can implement the PDF
                        // form control support now as follows
                        ::std::auto_ptr< ::vcl::PDFWriter::AnyWidget > pPDFControl;
                        ::toolkitform::describePDFControl( rControlPrimitive.getXControl(), pPDFControl );

                        if(pPDFControl.get())
                        {
                            // still need to fill in the location (is a class Rectangle)
                            const basegfx::B2DRange aRangeLogic(rControlPrimitive.getB2DRange(getViewInformation2D()));
                            const Rectangle aRectLogic(
                                (sal_Int32)floor(aRangeLogic.getMinX()), (sal_Int32)floor(aRangeLogic.getMinY()),
                                (sal_Int32)ceil(aRangeLogic.getMaxX()), (sal_Int32)ceil(aRangeLogic.getMaxY()));
                            pPDFControl->Location = aRectLogic;

                            Size aFontSize(pPDFControl->TextFont.GetSize());
                            aFontSize = mpOutputDevice->LogicToLogic(aFontSize, MapMode(MAP_POINT), mpOutputDevice->GetMapMode());
                            pPDFControl->TextFont.SetSize(aFontSize);

                            mpPDFExtOutDevData->BeginStructureElement(vcl::PDFWriter::Form);
                            mpPDFExtOutDevData->CreateControl(*pPDFControl.get());
                            mpPDFExtOutDevData->EndStructureElement();

                            // no normal paint needed (see original UnoControlPDFExportContact::do_PaintObject);
                            // do not process recursively
                            bDoProcessRecursively = false;
                        }
                    }

                    // printer output preparation
                    if(bDoProcessRecursively && !bSuppressPrinterOutput)
                    {
                        // this needs to do the same as UnoControlPrintOrPreviewContact::do_PaintObject
                        // does ATM. This means prepare_PrintOrPrintPreview and paint_Control
                        bool bIsPrintableControl(false);

                        if(rControlPrimitive.getXControl().is())
                        {
                            try
                            {
                                // find out if control is printable
                                Reference< XPropertySet > xModelProperties(rControlPrimitive.getXControl()->getModel(), UNO_QUERY);
                                Reference< XPropertySetInfo > xPropertyInfo(xModelProperties.is()
                                    ? xModelProperties->getPropertySetInfo()
                                    : Reference< XPropertySetInfo >());
                                const ::rtl::OUString sPrintablePropertyName(RTL_CONSTASCII_USTRINGPARAM("Printable"));

                                if(xPropertyInfo.is() && xPropertyInfo->hasPropertyByName(sPrintablePropertyName))
                                {
                                    OSL_VERIFY(xModelProperties->getPropertyValue(sPrintablePropertyName) >>= bIsPrintableControl);
                                }
                            }
                            catch(const Exception&)
                            {
                                OSL_ENSURE(false, "VclMetafileProcessor2D: No access to printable flag of Control, caught an exception!");
                            }
                        }

                        if(bIsPrintableControl)
                        {
                            try
                            {
                                // update position and size
                                const basegfx::B2DPoint aTopLeftPixel(PositionAndSizeControl(rControlPrimitive));

                                // output to given device
                                Reference< XView > xControlView(rControlPrimitive.getXControl(), UNO_QUERY_THROW);
                                xControlView->draw(basegfx::fround(aTopLeftPixel.getX()), basegfx::fround(aTopLeftPixel.getY()));
                                bDoProcessRecursively = false;
                            }
                            catch( const Exception& )
                            {
                                OSL_ENSURE(false, "VclMetafileProcessor2D: Printing of Control failed, caught an exception!");
                            }
                        }
                    }

                    // process recursively and add MetaFile comment
                    if(bDoProcessRecursively)
                    {
                        process(rControlPrimitive.get2DDecomposition(getViewInformation2D()));
                    }

                    break;
                }
                case PRIMITIVE2D_ID_TEXTHIERARCHYFIELDPRIMITIVE2D :
                {
                    // support for FIELD_SEQ_BEGIN, FIELD_SEQ_END and URL. It wraps text primitives (but is not limited to)
                    // thus do the MetafileAction embedding stuff but just handle recursively.
                    const primitive2d::TextHierarchyFieldPrimitive2D& rFieldPrimitive = static_cast< const primitive2d::TextHierarchyFieldPrimitive2D& >(rCandidate);
                    static const ByteString aCommentStringCommon("FIELD_SEQ_BEGIN");
                    static const ByteString aCommentStringPage("FIELD_SEQ_BEGIN;PageField");
                    static const ByteString aCommentStringEnd("FIELD_SEQ_END");

                    switch(rFieldPrimitive.getType())
                    {
                        default : // case drawinglayer::primitive2d::FIELD_TYPE_COMMON :
                        {
                            mrMetaFile.AddAction(new MetaCommentAction(aCommentStringCommon));
                            break;
                        }
                        case drawinglayer::primitive2d::FIELD_TYPE_PAGE :
                        {
                            mrMetaFile.AddAction(new MetaCommentAction(aCommentStringPage));
                            break;
                        }
                        case drawinglayer::primitive2d::FIELD_TYPE_URL :
                        {
                            const rtl::OUString& rURL = rFieldPrimitive.getString();
                            const String aOldString(rURL);
                            mrMetaFile.AddAction(new MetaCommentAction(aCommentStringCommon, 0, reinterpret_cast< const BYTE* >(aOldString.GetBuffer()), 2 * aOldString.Len()));
                            break;
                        }
                    }

                    // process recursively
                    const primitive2d::Primitive2DSequence rContent = rFieldPrimitive.get2DDecomposition(getViewInformation2D());
                    process(rContent);

                    // for the end comment the type is not relevant yet, they are all the same. Just add.
                    mrMetaFile.AddAction(new MetaCommentAction(aCommentStringEnd));

                    if(mpPDFExtOutDevData && drawinglayer::primitive2d::FIELD_TYPE_URL == rFieldPrimitive.getType())
                    {
                        // emulate data handling from ImpEditEngine::Paint
                        const basegfx::B2DRange aViewRange(primitive2d::getB2DRangeFromPrimitive2DSequence(rContent, getViewInformation2D()));
                        const Rectangle aRectLogic(
                            (sal_Int32)floor(aViewRange.getMinX()), (sal_Int32)floor(aViewRange.getMinY()),
                            (sal_Int32)ceil(aViewRange.getMaxX()), (sal_Int32)ceil(aViewRange.getMaxY()));
                        vcl::PDFExtOutDevBookmarkEntry aBookmark;
                        aBookmark.nLinkId = mpPDFExtOutDevData->CreateLink(aRectLogic);
                        aBookmark.aBookmark = rFieldPrimitive.getString();
                        std::vector< vcl::PDFExtOutDevBookmarkEntry >& rBookmarks = mpPDFExtOutDevData->GetBookmarks();
                        rBookmarks.push_back( aBookmark );
                    }

                    break;
                }
                case PRIMITIVE2D_ID_TEXTHIERARCHYLINEPRIMITIVE2D :
                {
                    const primitive2d::TextHierarchyLinePrimitive2D& rLinePrimitive = static_cast< const primitive2d::TextHierarchyLinePrimitive2D& >(rCandidate);
                    static const ByteString aCommentString("XTEXT_EOL");

                    // process recursively and add MetaFile comment
                    process(rLinePrimitive.get2DDecomposition(getViewInformation2D()));
                    mrMetaFile.AddAction(new MetaCommentAction(aCommentString));

                    break;
                }
                case PRIMITIVE2D_ID_TEXTHIERARCHYBULLETPRIMITIVE2D :
                {
                    // in Outliner::PaintBullet(), a MetafileComment for bullets is added, too. The
                    // "XTEXT_EOC" is used, use here, too.
                    const primitive2d::TextHierarchyBulletPrimitive2D& rBulletPrimitive = static_cast< const primitive2d::TextHierarchyBulletPrimitive2D& >(rCandidate);
                    static const ByteString aCommentString("XTEXT_EOC");

                    // process recursively and add MetaFile comment
                    process(rBulletPrimitive.get2DDecomposition(getViewInformation2D()));
                    mrMetaFile.AddAction(new MetaCommentAction(aCommentString));

                    break;
                }
                case PRIMITIVE2D_ID_TEXTHIERARCHYPARAGRAPHPRIMITIVE2D :
                {
                    const primitive2d::TextHierarchyParagraphPrimitive2D& rParagraphPrimitive = static_cast< const primitive2d::TextHierarchyParagraphPrimitive2D& >(rCandidate);
                    static const ByteString aCommentString("XTEXT_EOP");

                    if(mpPDFExtOutDevData)
                    {
                        // emulate data handling from ImpEditEngine::Paint
                        mpPDFExtOutDevData->BeginStructureElement( vcl::PDFWriter::Paragraph );
                    }

                    // process recursively and add MetaFile comment
                    process(rParagraphPrimitive.get2DDecomposition(getViewInformation2D()));
                    mrMetaFile.AddAction(new MetaCommentAction(aCommentString));

                    if(mpPDFExtOutDevData)
                    {
                        // emulate data handling from ImpEditEngine::Paint
                        mpPDFExtOutDevData->EndStructureElement();
                    }

                    break;
                }
                case PRIMITIVE2D_ID_TEXTHIERARCHYBLOCKPRIMITIVE2D :
                {
                    const primitive2d::TextHierarchyBlockPrimitive2D& rBlockPrimitive = static_cast< const primitive2d::TextHierarchyBlockPrimitive2D& >(rCandidate);
                    static const ByteString aCommentStringA("XTEXT_PAINTSHAPE_BEGIN");
                    static const ByteString aCommentStringB("XTEXT_PAINTSHAPE_END");

                    // add MetaFile comment, process recursively and add MetaFile comment
                    mrMetaFile.AddAction(new MetaCommentAction(aCommentStringA));
                    process(rBlockPrimitive.get2DDecomposition(getViewInformation2D()));
                    mrMetaFile.AddAction(new MetaCommentAction(aCommentStringB));

                    break;
                }
                case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D :
                case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D :
                {
                    // for supporting TEXT_ MetaFile actions there is more to do here; get the candidate
                    const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate = static_cast< const primitive2d::TextSimplePortionPrimitive2D& >(rCandidate);
                    const primitive2d::TextDecoratedPortionPrimitive2D* pTextDecoratedCandidate = dynamic_cast< const primitive2d::TextDecoratedPortionPrimitive2D* >(&rCandidate);

                    // Adapt evtl. used special DrawMode
                    const sal_uInt32 nOriginalDrawMode(mpOutputDevice->GetDrawMode());
                    adaptTextToFillDrawMode();

                    // directdraw of text simple portion; use default processing
                    RenderTextSimpleOrDecoratedPortionPrimitive2D(rTextCandidate);

                    // restore DrawMode
                    mpOutputDevice->SetDrawMode(nOriginalDrawMode);

                    if(pTextDecoratedCandidate)
                    {
                        // support for TEXT_ MetaFile actions only for decorated texts
                        if(!mxBreakIterator.is())
                        {
                            Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF(::comphelper::getProcessServiceFactory());
                            mxBreakIterator.set(xMSF->createInstance(rtl::OUString::createFromAscii("com.sun.star.i18n.BreakIterator")), UNO_QUERY);
                        }

                        if(mxBreakIterator.is())
                        {
                            const rtl::OUString& rTxt = rTextCandidate.getText();
                            const sal_Int32 nTextLength(rTxt.getLength());

                            if(nTextLength)
                            {
                                const ::com::sun::star::lang::Locale& rLocale = rTextCandidate.getLocale();

                                sal_Int32 nDone;
                                sal_Int32 nNextCellBreak(mxBreakIterator->nextCharacters(rTxt, 0, rLocale, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL, 0, nDone));
                                ::com::sun::star::i18n::Boundary nNextWordBoundary(mxBreakIterator->getWordBoundary(rTxt, 0, rLocale, ::com::sun::star::i18n::WordType::ANY_WORD, sal_True));
                                sal_Int32 nNextSentenceBreak(mxBreakIterator->endOfSentence(rTxt, 0, rLocale));
                                static const ByteString aCommentStringA("XTEXT_EOC");
                                static const ByteString aCommentStringB("XTEXT_EOW");
                                static const ByteString aCommentStringC("XTEXT_EOS");

                                for(sal_Int32 i(0); i < nTextLength; i++)
                                {
                                    // create the entries for the respective break positions
                                    if(i == nNextCellBreak)
                                    {
                                        mrMetaFile.AddAction(new MetaCommentAction(aCommentStringA, i));
                                        nNextCellBreak = mxBreakIterator->nextCharacters(rTxt, i, rLocale, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
                                    }
                                    if(i == nNextWordBoundary.endPos)
                                    {
                                        mrMetaFile.AddAction(new MetaCommentAction(aCommentStringB, i));
                                        nNextWordBoundary = mxBreakIterator->getWordBoundary(rTxt, i + 1, rLocale, ::com::sun::star::i18n::WordType::ANY_WORD, sal_True);
                                    }
                                    if(i == nNextSentenceBreak)
                                    {
                                        mrMetaFile.AddAction(new MetaCommentAction(aCommentStringC, i));
                                        nNextSentenceBreak = mxBreakIterator->endOfSentence(rTxt, i + 1, rLocale);
                                    }
                                }
                            }
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                {
                    // direct draw of hairline; use default processing
                    // also support SvtGraphicStroke MetaCommentAction
                    const primitive2d::PolygonHairlinePrimitive2D& rHairlinePrimitive = static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate);
                    const basegfx::BColor aLineColor(maBColorModifierStack.getModifiedColor(rHairlinePrimitive.getBColor()));
                    SvtGraphicStroke* pSvtGraphicStroke = impTryToCreateSvtGraphicStroke(rHairlinePrimitive.getB2DPolygon(), &aLineColor, 0, 0, 0, 0);

                    impStartSvtGraphicStroke(pSvtGraphicStroke);
                    RenderPolygonHairlinePrimitive2D(static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate));
                    impEndSvtGraphicStroke(pSvtGraphicStroke);
                    break;
                }
                case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D :
                {
                    // support SvtGraphicStroke MetaCommentAction
                    const primitive2d::PolygonStrokePrimitive2D& rStrokePrimitive = static_cast< const primitive2d::PolygonStrokePrimitive2D& >(rCandidate);
                    SvtGraphicStroke* pSvtGraphicStroke = impTryToCreateSvtGraphicStroke(rStrokePrimitive.getB2DPolygon(), 0, &rStrokePrimitive.getLineAttribute(),
                        &rStrokePrimitive.getStrokeAttribute(), 0, 0);

                    // Adapt OutDev's DrawMode if special ones were used
                    const sal_uInt32 nOriginalDrawMode(mpOutputDevice->GetDrawMode());
                    adaptLineToFillDrawMode();

                    impStartSvtGraphicStroke(pSvtGraphicStroke);
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    impEndSvtGraphicStroke(pSvtGraphicStroke);

                    // restore DrawMode
                    mpOutputDevice->SetDrawMode(nOriginalDrawMode);

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D :
                {
                    // support SvtGraphicStroke MetaCommentAction
                    const primitive2d::PolygonStrokeArrowPrimitive2D& rStrokeArrowPrimitive = static_cast< const primitive2d::PolygonStrokeArrowPrimitive2D& >(rCandidate);
                    SvtGraphicStroke* pSvtGraphicStroke = impTryToCreateSvtGraphicStroke(rStrokeArrowPrimitive.getB2DPolygon(), 0, &rStrokeArrowPrimitive.getLineAttribute(),
                        &rStrokeArrowPrimitive.getStrokeAttribute(), &rStrokeArrowPrimitive.getStart(), &rStrokeArrowPrimitive.getEnd());

                    impStartSvtGraphicStroke(pSvtGraphicStroke);
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    impEndSvtGraphicStroke(pSvtGraphicStroke);
                    break;
                }
                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                {
                    // direct draw of transformed BitmapEx primitive; use default processing
                    RenderBitmapPrimitive2D(static_cast< const primitive2d::BitmapPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONBITMAPPRIMITIVE2D :
                {
                    // need to handle PolyPolygonBitmapPrimitive2D here to support XPATHFILL_SEQ_BEGIN/XPATHFILL_SEQ_END
                    SvtGraphicFill* pSvtGraphicFill = 0;

                    if(!mnSvtGraphicFillCount)
                    {
                        const primitive2d::PolyPolygonBitmapPrimitive2D& rBitmapCandidate = static_cast< const primitive2d::PolyPolygonBitmapPrimitive2D& >(rCandidate);
                        basegfx::B2DPolyPolygon aLocalPolyPolygon(rBitmapCandidate.getB2DPolyPolygon());
                        aLocalPolyPolygon.transform(maCurrentTransformation);

                        if(aLocalPolyPolygon.count())
                        {
                            // calculate transformation. Get real object size, all values in FillBitmapAttribute
                            // are relative to the unified object
                            const attribute::FillBitmapAttribute& rFillBitmapAttribute = rBitmapCandidate .getFillBitmap();
                            const basegfx::B2DRange aOutlineRange(basegfx::tools::getRange(aLocalPolyPolygon));
                            const basegfx::B2DVector aOutlineSize(aOutlineRange.getRange());

                            // get absolute values
                            const basegfx::B2DVector aFillBitmapSize(rFillBitmapAttribute.getSize() * aOutlineSize);
                            const basegfx::B2DPoint aFillBitmapTopLeft(rFillBitmapAttribute.getTopLeft() * aOutlineSize);

                            // the scaling needs scale from pixel to logic coordinate system
                            const Bitmap& rBitmap = rFillBitmapAttribute.getBitmap();
                            Size aBmpSizePixel(rBitmap.GetSizePixel());

                            if(!aBmpSizePixel.Width())
                            {
                                aBmpSizePixel.Width() = 1;
                            }

                            if(!aBmpSizePixel.Height())
                            {
                                aBmpSizePixel.Height() = 1;
                            }

                            // setup transformation like in impgrfll
                            SvtGraphicFill::Transform aTransform;

                            // scale values are divided by bitmap pixel sizes
                            aTransform.matrix[0] = aFillBitmapSize.getX() / aBmpSizePixel.Width();
                            aTransform.matrix[4] = aFillBitmapSize.getY() / aBmpSizePixel.Height();

                            // translates are absolute
                            aTransform.matrix[2] = aFillBitmapTopLeft.getX();
                            aTransform.matrix[5] = aFillBitmapTopLeft.getY();

                            // setup fill graphic like in impgrfll
                            Graphic aFillGraphic = Graphic(rBitmap);
                            aFillGraphic.SetPrefMapMode(MapMode(MAP_PIXEL));
                            aFillGraphic.SetPrefSize(aBmpSizePixel);

                            pSvtGraphicFill = new SvtGraphicFill(
                                PolyPolygon(aLocalPolyPolygon),
                                Color(),
                                0.0,
                                SvtGraphicFill::fillEvenOdd,
                                SvtGraphicFill::fillTexture,
                                aTransform,
                                rFillBitmapAttribute.getTiling(),
                                SvtGraphicFill::hatchSingle,
                                Color(),
                                SvtGraphicFill::gradientLinear,
                                Color(),
                                Color(),
                                0,
                                aFillGraphic);
                        }
                    }

                    // Do use decomposition; encapsulate with SvtGraphicFill
                    impStartSvtGraphicFill(pSvtGraphicFill);
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    impEndSvtGraphicFill(pSvtGraphicFill);

                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONHATCHPRIMITIVE2D :
                {
                    // need to handle PolyPolygonHatchPrimitive2D here to support XPATHFILL_SEQ_BEGIN/XPATHFILL_SEQ_END
                    SvtGraphicFill* pSvtGraphicFill = 0;

                    if(!mnSvtGraphicFillCount)
                    {
                        const primitive2d::PolyPolygonHatchPrimitive2D& rHatchCandidate = static_cast< const primitive2d::PolyPolygonHatchPrimitive2D& >(rCandidate);
                        basegfx::B2DPolyPolygon aLocalPolyPolygon(rHatchCandidate.getB2DPolyPolygon());
                        aLocalPolyPolygon.transform(maCurrentTransformation);

                        if(aLocalPolyPolygon.count())
                        {
                            // re-create a VCL hatch as base data
                            const attribute::FillHatchAttribute& rFillHatchAttribute = rHatchCandidate.getFillHatch();
                            SvtGraphicFill::HatchType eHatch(SvtGraphicFill::hatchSingle);

                            switch(rFillHatchAttribute.getStyle())
                            {
                                default: // attribute::HATCHSTYLE_SINGLE :
                                {
                                    eHatch = SvtGraphicFill::hatchSingle;
                                    break;
                                }
                                case attribute::HATCHSTYLE_DOUBLE :
                                {
                                    eHatch = SvtGraphicFill::hatchDouble;
                                    break;
                                }
                                case attribute::HATCHSTYLE_TRIPLE :
                                {
                                    eHatch = SvtGraphicFill::hatchTriple;
                                    break;
                                }
                            }

                            SvtGraphicFill::Transform aTransform;

                            // scale
                            aTransform.matrix[0] *= rFillHatchAttribute.getDistance();
                            aTransform.matrix[4] *= rFillHatchAttribute.getDistance();

                            // rotate (was never correct in impgrfll anyways, use correct angle now)
                            aTransform.matrix[0] *= cos(rFillHatchAttribute.getAngle());
                            aTransform.matrix[1] *= -sin(rFillHatchAttribute.getAngle());
                            aTransform.matrix[3] *= sin(rFillHatchAttribute.getAngle());
                            aTransform.matrix[4] *= cos(rFillHatchAttribute.getAngle());

                            pSvtGraphicFill = new SvtGraphicFill(
                                PolyPolygon(aLocalPolyPolygon),
                                Color(),
                                0.0,
                                SvtGraphicFill::fillEvenOdd,
                                SvtGraphicFill::fillHatch,
                                aTransform,
                                false,
                                eHatch,
                                Color(rFillHatchAttribute.getColor()),
                                SvtGraphicFill::gradientLinear,
                                Color(),
                                Color(),
                                0,
                                Graphic());
                        }
                    }

                    // Do use decomposition; encapsulate with SvtGraphicFill
                    impStartSvtGraphicFill(pSvtGraphicFill);
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    impEndSvtGraphicFill(pSvtGraphicFill);

                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D :
                {
                    const primitive2d::PolyPolygonGradientPrimitive2D& rGradientCandidate = static_cast< const primitive2d::PolyPolygonGradientPrimitive2D& >(rCandidate);

                    // for support of MetaCommentActions of the form XGRAD_SEQ_BEGIN, XGRAD_SEQ_END
                    // it is safest to use the VCL OutputDevice::DrawGradient method which creates those.
                    // re-create a VCL-gradient from FillGradientPrimitive2D and the needed tools PolyPolygon
                    Gradient aVCLGradient;
                    impConvertFillGradientAttributeToVCLGradient(aVCLGradient, rGradientCandidate.getFillGradient(), false);
                    basegfx::B2DPolyPolygon aLocalPolyPolygon(rGradientCandidate.getB2DPolyPolygon());
                    aLocalPolyPolygon.transform(maCurrentTransformation);

                    // #i82145# ATM VCL printing of gradients using curved shapes does not work,
                    // i submitted the bug with the given ID to THB. When that task is fixed it is
                    // necessary to again remove this subdivision since it decreases possible
                    // printing quality (not even resolution-dependent for now). THB will tell
                    // me when that task is fixed in the master
                    const PolyPolygon aToolsPolyPolygon(basegfx::tools::adaptiveSubdivideByAngle(aLocalPolyPolygon));

                    // XPATHFILL_SEQ_BEGIN/XPATHFILL_SEQ_END support
                    SvtGraphicFill* pSvtGraphicFill = 0;

                    if(!mnSvtGraphicFillCount && aLocalPolyPolygon.count())
                    {
                        // setup gradient stuff like in like in impgrfll
                        SvtGraphicFill::GradientType eGrad(SvtGraphicFill::gradientLinear);

                        switch(aVCLGradient.GetStyle())
                        {
                            default : // GRADIENT_LINEAR:
                            case GRADIENT_AXIAL:
                                eGrad = SvtGraphicFill::gradientLinear;
                                break;
                            case GRADIENT_RADIAL:
                            case GRADIENT_ELLIPTICAL:
                                eGrad = SvtGraphicFill::gradientRadial;
                                break;
                            case GRADIENT_SQUARE:
                            case GRADIENT_RECT:
                                eGrad = SvtGraphicFill::gradientRectangular;
                                break;
                        }

                        pSvtGraphicFill = new SvtGraphicFill(
                            aToolsPolyPolygon,
                            Color(),
                            0.0,
                            SvtGraphicFill::fillEvenOdd,
                            SvtGraphicFill::fillGradient,
                            SvtGraphicFill::Transform(),
                            false,
                            SvtGraphicFill::hatchSingle,
                            Color(),
                            eGrad,
                            aVCLGradient.GetStartColor(),
                            aVCLGradient.GetEndColor(),
                            aVCLGradient.GetSteps(),
                            Graphic());
                    }

                    // call VCL directly; encapsulate with SvtGraphicFill
                    impStartSvtGraphicFill(pSvtGraphicFill);
                    mpOutputDevice->DrawGradient(aToolsPolyPolygon, aVCLGradient);
                    impEndSvtGraphicFill(pSvtGraphicFill);

                    // NO usage of common own gradient randerer, not used ATM for VCL MetaFile, see text above
                    // RenderPolyPolygonGradientPrimitive2D(static_cast< const primitive2d::PolyPolygonGradientPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                {
                    const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(rCandidate));
                    const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(rPolygonCandidate.getBColor()));
                    basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());
                    aLocalPolyPolygon.transform(maCurrentTransformation);

                    // XPATHFILL_SEQ_BEGIN/XPATHFILL_SEQ_END support
                    SvtGraphicFill* pSvtGraphicFill = 0;

                    if(!mnSvtGraphicFillCount && aLocalPolyPolygon.count())
                    {
                        // setup simple color fill stuff like in impgrfll
                        pSvtGraphicFill = new SvtGraphicFill(
                            PolyPolygon(aLocalPolyPolygon),
                            Color(aPolygonColor),
                            0.0,
                            SvtGraphicFill::fillEvenOdd,
                            SvtGraphicFill::fillSolid,
                            SvtGraphicFill::Transform(),
                            false,
                            SvtGraphicFill::hatchSingle,
                            Color(),
                            SvtGraphicFill::gradientLinear,
                            Color(),
                            Color(),
                            0,
                            Graphic());
                    }

                    // set line and fill color
                    mpOutputDevice->SetFillColor(Color(aPolygonColor));
                    mpOutputDevice->SetLineColor();

                    // call VCL directly; encapsulate with SvtGraphicFill
                    impStartSvtGraphicFill(pSvtGraphicFill);
                    mpOutputDevice->DrawPolyPolygon(aLocalPolyPolygon);
                    impEndSvtGraphicFill(pSvtGraphicFill);

                    break;
                }
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                {
                    // direct draw of MetaFile, use default pocessing
                    RenderMetafilePrimitive2D(static_cast< const primitive2d::MetafilePrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
                {
                    // mask group. Special handling for MetaFiles.
                    const primitive2d::MaskPrimitive2D& rMaskCandidate = static_cast< const primitive2d::MaskPrimitive2D& >(rCandidate);

                    if(rMaskCandidate.getChildren().hasElements())
                    {
                        basegfx::B2DPolyPolygon aMask(rMaskCandidate.getMask());

                        if(aMask.count())
                        {
                            // prepare new mask polygon and rescue current one
                            aMask.transform(maCurrentTransformation);
                            const basegfx::B2DPolyPolygon aLastClipPolyPolygon(maClipPolyPolygon);

                            if(maClipPolyPolygon.count())
                            {
                                // there is already a clip polygon set; build clipped union of
                                // current mask polygon and new one
                                maClipPolyPolygon = basegfx::tools::clipPolyPolygonOnPolyPolygon(aMask, maClipPolyPolygon, false, false);
                            }
                            else
                            {
                                // use mask directly
                                maClipPolyPolygon = aMask;
                            }

                            if(maClipPolyPolygon.count())
                            {
                                // set VCL clip region; subdivide before conversion to tools polygon. Subdivision necessary (!)
                                mpOutputDevice->Push(PUSH_CLIPREGION);
                                mpOutputDevice->SetClipRegion(Region(PolyPolygon(maClipPolyPolygon)));
                                mpOutputDevice->SetClipRegion(Region(PolyPolygon(basegfx::tools::adaptiveSubdivideByAngle(maClipPolyPolygon))));
                            }

                            // recursively paint content
                            process(rMaskCandidate.getChildren());

                            if(maClipPolyPolygon.count())
                            {
                                // restore VCL clip region
                                mpOutputDevice->Pop();
                            }

                            // restore to rescued clip polygon
                            maClipPolyPolygon = aLastClipPolyPolygon;
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D :
                {
                    // modified color group. Force output to unified color. Use default pocessing.
                    RenderModifiedColorPrimitive2D(static_cast< const primitive2d::ModifiedColorPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_UNIFIEDALPHAPRIMITIVE2D :
                {
                    // for metafile: Need to examine what the pure vcl version is doing here actually
                    // - uses DrawTransparent with metafile for content and a gradient
                    // - uses DrawTransparent for single PolyPoylgons directly. Can be detected by
                    //   checking the content for single PolyPolygonColorPrimitive2D
                    const primitive2d::UnifiedAlphaPrimitive2D& rUniAlphaCandidate = static_cast< const primitive2d::UnifiedAlphaPrimitive2D& >(rCandidate);
                    const primitive2d::Primitive2DSequence rContent = rUniAlphaCandidate.getChildren();

                    if(rContent.hasElements())
                    {
                        // try to identify a single PolyPolygonColorPrimitive2D in the
                        // content part of the alpha primitive
                        const primitive2d::PolyPolygonColorPrimitive2D* pPoPoColor = 0;
                        static bool bForceToMetafile(false);

                        if(!bForceToMetafile && 1 == rContent.getLength())
                        {
                            const primitive2d::Primitive2DReference xReference(rContent[0]);
                            pPoPoColor = dynamic_cast< const primitive2d::PolyPolygonColorPrimitive2D* >(xReference.get());
                        }

                        // PolyPolygonGradientPrimitive2D, PolyPolygonHatchPrimitive2D and
                        // PolyPolygonBitmapPrimitive2D are derived from PolyPolygonColorPrimitive2D.
                        // Check also for correct ID to exclude derived implementations
                        if(pPoPoColor && PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D == pPoPoColor->getPrimitiveID())
                        {
                            // single transparent PolyPolygon identified, use directly
                            const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(pPoPoColor->getBColor()));
                            basegfx::B2DPolyPolygon aLocalPolyPolygon(pPoPoColor->getB2DPolyPolygon());
                            aLocalPolyPolygon.transform(maCurrentTransformation);

                            // XPATHFILL_SEQ_BEGIN/XPATHFILL_SEQ_END support
                            SvtGraphicFill* pSvtGraphicFill = 0;

                            if(!mnSvtGraphicFillCount && aLocalPolyPolygon.count())
                            {
                                // setup simple color with transparence fill stuff like in impgrfll
                                pSvtGraphicFill = new SvtGraphicFill(
                                    PolyPolygon(aLocalPolyPolygon),
                                    Color(aPolygonColor),
                                    rUniAlphaCandidate.getAlpha(),
                                    SvtGraphicFill::fillEvenOdd,
                                    SvtGraphicFill::fillSolid,
                                    SvtGraphicFill::Transform(),
                                    false,
                                    SvtGraphicFill::hatchSingle,
                                    Color(),
                                    SvtGraphicFill::gradientLinear,
                                    Color(),
                                    Color(),
                                    0,
                                    Graphic());
                            }

                            // set line and fill color
                            const sal_uInt16 nTransPercentVcl((sal_uInt16)basegfx::fround(rUniAlphaCandidate.getAlpha() * 100.0));
                            mpOutputDevice->SetFillColor(Color(aPolygonColor));
                            mpOutputDevice->SetLineColor();

                            // call VCL directly; encapsulate with SvtGraphicFill
                            impStartSvtGraphicFill(pSvtGraphicFill);
                            mpOutputDevice->DrawTransparent(
                                PolyPolygon(aLocalPolyPolygon),
                                nTransPercentVcl);
                            impEndSvtGraphicFill(pSvtGraphicFill);
                        }
                        else
                        {
                            // svae old mfCurrentUnifiedTransparence and set new one
                            // so that contained SvtGraphicStroke may use the current one
                            const double fLastCurrentUnifiedTransparence(mfCurrentUnifiedTransparence);
                            mfCurrentUnifiedTransparence = rUniAlphaCandidate.getAlpha();

                            // various content, create content-metafile
                            GDIMetaFile aContentMetafile;
                            const Rectangle aPrimitiveRectangle(impDumpToMetaFile(rContent, aContentMetafile));

                            // restore mfCurrentUnifiedTransparence; it may have been used
                            // while processing the sub-content in impDumpToMetaFile
                            mfCurrentUnifiedTransparence = fLastCurrentUnifiedTransparence;

                            // create uniform VCL gradient for uniform transparency
                            Gradient aVCLGradient;
                            const sal_uInt8 nTransPercentVcl((sal_uInt8)basegfx::fround(rUniAlphaCandidate.getAlpha() * 255.0));
                            const Color aTransColor(nTransPercentVcl, nTransPercentVcl, nTransPercentVcl);

                            aVCLGradient.SetStyle(GRADIENT_LINEAR);
                            aVCLGradient.SetStartColor(aTransColor);
                            aVCLGradient.SetEndColor(aTransColor);
                            aVCLGradient.SetAngle(0);
                            aVCLGradient.SetBorder(0);
                            aVCLGradient.SetOfsX(0);
                            aVCLGradient.SetOfsY(0);
                            aVCLGradient.SetStartIntensity(100);
                            aVCLGradient.SetEndIntensity(100);
                            aVCLGradient.SetSteps(2);

                            // render it to VCL
                            mpOutputDevice->DrawTransparent(
                                aContentMetafile, aPrimitiveRectangle.TopLeft(),
                                aPrimitiveRectangle.GetSize(), aVCLGradient);
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_ALPHAPRIMITIVE2D :
                {
                    // for metafile: Need to examine what the pure vcl version is doing here actually
                    // - uses DrawTransparent with metafile for content and a gradient
                    // i can detect this here with checking the gradient part for a single
                    // FillGradientPrimitive2D and reconstruct the gradient.
                    // If that detection goes wrong, i have to create an alpha-blended bitmap. Eventually
                    // do that in stripes, else RenderAlphaPrimitive2D may just be used
                    const primitive2d::AlphaPrimitive2D& rAlphaCandidate = static_cast< const primitive2d::AlphaPrimitive2D& >(rCandidate);
                    const primitive2d::Primitive2DSequence rContent = rAlphaCandidate.getChildren();
                    const primitive2d::Primitive2DSequence rAlpha = rAlphaCandidate.getAlpha();

                    if(rContent.hasElements() && rAlpha.hasElements())
                    {
                        // try to identify a single FillGradientPrimitive2D in the
                        // alpha part of the primitive
                        const primitive2d::FillGradientPrimitive2D* pFiGradient = 0;
                        static bool bForceToBigTransparentVDev(false);

                        if(!bForceToBigTransparentVDev && 1 == rAlpha.getLength())
                        {
                            const primitive2d::Primitive2DReference xReference(rAlpha[0]);
                            pFiGradient = dynamic_cast< const primitive2d::FillGradientPrimitive2D* >(xReference.get());
                        }

                        // Check also for correct ID to exclude derived implementations
                        if(pFiGradient && PRIMITIVE2D_ID_FILLGRADIENTPRIMITIVE2D == pFiGradient->getPrimitiveID())
                        {
                            // various content, create content-metafile
                            GDIMetaFile aContentMetafile;
                            const Rectangle aPrimitiveRectangle(impDumpToMetaFile(rContent, aContentMetafile));

                            // re-create a VCL-gradient from FillGradientPrimitive2D
                            Gradient aVCLGradient;
                            impConvertFillGradientAttributeToVCLGradient(aVCLGradient, pFiGradient->getFillGradient(), true);

                            // render it to VCL
                            mpOutputDevice->DrawTransparent(
                                aContentMetafile, aPrimitiveRectangle.TopLeft(),
                                aPrimitiveRectangle.GetSize(), aVCLGradient);
                        }
                        else
                        {
                            // sub-transparence group. Draw to VDev first.
                            // this may get refined to tiling when resolution is too big here

                            // need to avoid switching off MapMode stuff here; maybe need another
                            // tooling class, cannot just do the same as with the pixel renderer.
                            // Need to experiment...

                            // Okay, basic implementation finished and tested. The DPI stuff was hard
                            // and not easy to find out that it's needed.
                            // Since this will not yet happen normally (as long as noone constructs
                            // alpha primitives with non-trivial alpha content) i will for now not
                            // refine to tiling here.

                            basegfx::B2DRange aViewRange(primitive2d::getB2DRangeFromPrimitive2DSequence(rContent, getViewInformation2D()));
                            aViewRange.transform(maCurrentTransformation);
                            const Rectangle aRectLogic(
                                (sal_Int32)floor(aViewRange.getMinX()), (sal_Int32)floor(aViewRange.getMinY()),
                                (sal_Int32)ceil(aViewRange.getMaxX()), (sal_Int32)ceil(aViewRange.getMaxY()));
                            const Rectangle aRectPixel(mpOutputDevice->LogicToPixel(aRectLogic));
                            const Size aSizePixel(aRectPixel.GetSize());
                            const Point aEmptyPoint;
                            VirtualDevice aBufferDevice;

                            if(aBufferDevice.SetOutputSizePixel(aSizePixel))
                            {
                                // create and set MapModes for target devices
                                MapMode aNewMapMode(mpOutputDevice->GetMapMode());
                                aNewMapMode.SetOrigin(Point(-aRectLogic.Left(), -aRectLogic.Top()));
                                aBufferDevice.SetMapMode(aNewMapMode);

                                // prepare view transformation for target renderers
                                // ATTENTION! Need to apply another scaling because of the potential DPI differences
                                // between Printer and VDev (mpOutputDevice and aBufferDevice here).
                                // To get the DPI, LogicToPixel from (1,1) from MAP_INCH needs to be used.
                                basegfx::B2DHomMatrix aViewTransform(aBufferDevice.GetViewTransformation());
                                const Size aDPIOld(mpOutputDevice->LogicToPixel(Size(1, 1), MAP_INCH));
                                const Size aDPINew(aBufferDevice.LogicToPixel(Size(1, 1), MAP_INCH));
                                const double fDPIXChange((double)aDPIOld.getWidth() / (double)aDPINew.getWidth());
                                const double fDPIYChange((double)aDPIOld.getHeight() / (double)aDPINew.getHeight());

                                if(!basegfx::fTools::equal(fDPIXChange, 1.0) || !basegfx::fTools::equal(fDPIYChange, 1.0))
                                {
                                    aViewTransform.scale(fDPIXChange, fDPIYChange);
                                }

                                // create view information and pixel renderer. Reuse known ViewInformation
                                // except new transformation and range
                                const geometry::ViewInformation2D aViewInfo(
                                    aViewTransform,
                                    aViewRange,
                                    getViewInformation2D().getVisualizedPage(),
                                    getViewInformation2D().getViewTime(),
                                    getViewInformation2D().getExtendedInformationSequence());

                                VclPixelProcessor2D aBufferProcessor(aViewInfo, aBufferDevice);

                                // draw content using pixel renderer
                                aBufferProcessor.process(rContent);
                                const Bitmap aBmContent(aBufferDevice.GetBitmap(aEmptyPoint, aSizePixel));

                                // draw alpha using pixel renderer
                                aBufferDevice.Erase();
                                aBufferProcessor.process(rAlpha);
                                const AlphaMask aBmAlpha(aBufferDevice.GetBitmap(aEmptyPoint, aSizePixel));

#ifdef DBG_UTIL
                                static bool bDoSaveForVisualControl(false);
                                if(bDoSaveForVisualControl)
                                {
                                    SvFileStream aNew(String(ByteString( "c:\\test.bmp" ), RTL_TEXTENCODING_UTF8), STREAM_WRITE|STREAM_TRUNC);
                                    aNew << aBmContent;
                                }
#endif

                                // paint
                                mpOutputDevice->DrawBitmapEx(
                                    aRectLogic.TopLeft(),
                                    aRectLogic.GetSize(),
                                    BitmapEx(aBmContent, aBmAlpha));
                            }
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D :
                {
                    // use default transform group pocessing
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
                    // use default marker array pocessing
                    RenderMarkerArrayPrimitive2D(static_cast< const primitive2d::MarkerArrayPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                {
                    // use default point array pocessing
                    RenderPointArrayPrimitive2D(static_cast< const primitive2d::PointArrayPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_CHARTPRIMITIVE2D :
                {
                    // point array
                    RenderChartPrimitive2D(static_cast< const primitive2d::ChartPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_STRUCTURETAGRIMITIVE2D :
                {
                    // structured tag primitive
                    const primitive2d::StructureTagPrimitive2D& rStructureTagCandidate = static_cast< const primitive2d::StructureTagPrimitive2D& >(rCandidate);
                    const vcl::PDFWriter::StructElement& rTagElement(rStructureTagCandidate.getStructureElement());
                    const bool bTagUsed(vcl::PDFWriter::NonStructElement != rTagElement);

                    if(mpPDFExtOutDevData &&  bTagUsed)
                    {
                        // write start tag
                        mpPDFExtOutDevData->BeginStructureElement(rTagElement);
                    }

                    // proccess childs normally
                    process(rStructureTagCandidate.getChildren());

                    if(mpPDFExtOutDevData &&  bTagUsed)
                    {
                        // write end tag
                        mpPDFExtOutDevData->EndStructureElement();
                    }

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
