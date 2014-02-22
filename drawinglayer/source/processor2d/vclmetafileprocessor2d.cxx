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

#include "vclmetafileprocessor2d.hxx"
#include <tools/gen.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/gradient.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include "vclpixelprocessor2d.hxx"
#include <tools/stream.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <vcl/graphictools.hxx>
#include <vcl/metaact.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#include <drawinglayer/primitive2d/epsprimitive2d.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <vcl/dibtools.hxx>

//////////////////////////////////////////////////////////////////////////////


#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/formpdfexport.hxx>

//////////////////////////////////////////////////////////////////////////////


#include <com/sun/star/beans/XPropertySet.hpp>

//////////////////////////////////////////////////////////////////////////////


#include <drawinglayer/primitive2d/structuretagprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////




#define MAX_POLYGON_POINT_COUNT_METAFILE    (0x0000fff0)

//////////////////////////////////////////////////////////////////////////////

namespace
{
    
    void splitLinePolygon(
        const basegfx::B2DPolygon& rBasePolygon,
        basegfx::B2DPolygon& o_aLeft,
        basegfx::B2DPolygon& o_aRight)
    {
        const sal_uInt32 nCount(rBasePolygon.count());

        if(nCount)
        {
            const sal_uInt32 nHalfCount((nCount - 1) >> 1);

            o_aLeft = basegfx::B2DPolygon(rBasePolygon, 0, nHalfCount + 1);
            o_aLeft.setClosed(false);

            o_aRight = basegfx::B2DPolygon(rBasePolygon, nHalfCount, nCount - nHalfCount);
            o_aRight.setClosed(false);

            if(rBasePolygon.isClosed())
            {
                o_aRight.append(rBasePolygon.getB2DPoint(0));

                if(rBasePolygon.areControlPointsUsed())
                {
                    o_aRight.setControlPoints(
                        o_aRight.count() - 1,
                        rBasePolygon.getPrevControlPoint(0),
                        rBasePolygon.getNextControlPoint(0));
                }
            }
        }
        else
        {
            o_aLeft.clear();
            o_aRight.clear();
        }
    }

    
    bool fillPolyPolygonNeededToBeSplit(basegfx::B2DPolyPolygon& rPolyPolygon)
    {
        bool bRetval(false);
        const sal_uInt32 nPolyCount(rPolyPolygon.count());

        if(nPolyCount)
        {
            basegfx::B2DPolyPolygon aSplitted;

            for(sal_uInt32 a(0); a < nPolyCount; a++)
            {
                const basegfx::B2DPolygon aCandidate(rPolyPolygon.getB2DPolygon(a));
                const sal_uInt32 nPointCount(aCandidate.count());
                bool bNeedToSplit(false);

                if(aCandidate.areControlPointsUsed())
                {
                    
                    bNeedToSplit = nPointCount > ((MAX_POLYGON_POINT_COUNT_METAFILE / 3L) - 1L);
                }
                else
                {
                    
                    bNeedToSplit = nPointCount > (MAX_POLYGON_POINT_COUNT_METAFILE - 1);
                }

                if(bNeedToSplit)
                {
                    
                    const basegfx::B2DRange aRange(aCandidate.getB2DRange());
                    const basegfx::B2DPoint aCenter(aRange.getCenter());

                    if(aRange.getWidth() > aRange.getHeight())
                    {
                        
                        const basegfx::B2DPolyPolygon aLeft(
                            basegfx::tools::clipPolygonOnParallelAxis(
                                aCandidate,
                                false,
                                true,
                                aCenter.getX(),
                                false));
                        const basegfx::B2DPolyPolygon aRight(
                            basegfx::tools::clipPolygonOnParallelAxis(
                                aCandidate,
                                false,
                                false,
                                aCenter.getX(),
                                false));

                        aSplitted.append(aLeft);
                        aSplitted.append(aRight);
                    }
                    else
                    {
                        
                        const basegfx::B2DPolyPolygon aTop(
                            basegfx::tools::clipPolygonOnParallelAxis(
                                aCandidate,
                                true,
                                true,
                                aCenter.getY(),
                                false));
                        const basegfx::B2DPolyPolygon aBottom(
                            basegfx::tools::clipPolygonOnParallelAxis(
                                aCandidate,
                                true,
                                false,
                                aCenter.getY(),
                                false));

                        aSplitted.append(aTop);
                        aSplitted.append(aBottom);
                    }
                }
                else
                {
                    aSplitted.append(aCandidate);
                }
            }

            if(aSplitted.count() != nPolyCount)
            {
                rPolyPolygon = aSplitted;
            }
        }

        return bRetval;
    }

    /** Filter input polypolygon for effectively empty sub-fills

        Needed to fix fdo#37559

        @param rPoly
        PolyPolygon to filter

        @return converted tools PolyPolygon, w/o one-point fills
     */
    PolyPolygon getFillPolyPolygon( const ::basegfx::B2DPolyPolygon& rPoly )
    {
        
        basegfx::B2DPolyPolygon aPoly;
        sal_uInt32 nCount(rPoly.count());
        for( sal_uInt32 i=0; i<nCount; ++i )
        {
            basegfx::B2DPolygon aCandidate(rPoly.getB2DPolygon(i));
            if( !aCandidate.isClosed() || aCandidate.count() > 1 )
                aPoly.append(aCandidate);
        }
        return PolyPolygon(aPoly);
    }

} 

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        Rectangle VclMetafileProcessor2D::impDumpToMetaFile(
            const primitive2d::Primitive2DSequence& rContent,
            GDIMetaFile& o_rContentMetafile)
        {
            
            OutputDevice* pLastOutputDevice = mpOutputDevice;
            GDIMetaFile* pLastMetafile = mpMetaFile;
            basegfx::B2DRange aPrimitiveRange(primitive2d::getB2DRangeFromPrimitive2DSequence(rContent, getViewInformation2D()));

            
            aPrimitiveRange.transform(maCurrentTransformation);

            const Rectangle aPrimitiveRectangle(
                basegfx::fround(aPrimitiveRange.getMinX()), basegfx::fround(aPrimitiveRange.getMinY()),
                basegfx::fround(aPrimitiveRange.getMaxX()), basegfx::fround(aPrimitiveRange.getMaxY()));
            VirtualDevice aContentVDev;
            MapMode aNewMapMode(pLastOutputDevice->GetMapMode());

            mpOutputDevice = &aContentVDev;
            mpMetaFile = &o_rContentMetafile;
            aContentVDev.EnableOutput(false);
            aContentVDev.SetMapMode(pLastOutputDevice->GetMapMode());
            o_rContentMetafile.Record(&aContentVDev);
            aContentVDev.SetLineColor(pLastOutputDevice->GetLineColor());
            aContentVDev.SetFillColor(pLastOutputDevice->GetFillColor());
            aContentVDev.SetFont(pLastOutputDevice->GetFont());
            aContentVDev.SetDrawMode(pLastOutputDevice->GetDrawMode());
            aContentVDev.SetSettings(pLastOutputDevice->GetSettings());
            aContentVDev.SetRefPoint(pLastOutputDevice->GetRefPoint());

            
            process(rContent);

            
            o_rContentMetafile.Stop();
            o_rContentMetafile.WindStart();
            aNewMapMode.SetOrigin(aPrimitiveRectangle.TopLeft());
            o_rContentMetafile.SetPrefMapMode(aNewMapMode);
            o_rContentMetafile.SetPrefSize(aPrimitiveRectangle.GetSize());
            mpOutputDevice = pLastOutputDevice;
            mpMetaFile = pLastMetafile;

            return aPrimitiveRectangle;
        }

        void VclMetafileProcessor2D::impConvertFillGradientAttributeToVCLGradient(
            Gradient& o_rVCLGradient,
            const attribute::FillGradientAttribute& rFiGrAtt,
            bool bIsTransparenceGradient)
        {
            if(bIsTransparenceGradient)
            {
                
                o_rVCLGradient.SetStartColor(Color(rFiGrAtt.getStartColor()));
                o_rVCLGradient.SetEndColor(Color(rFiGrAtt.getEndColor()));
            }
            else
            {
                
                o_rVCLGradient.SetStartColor(Color(maBColorModifierStack.getModifiedColor(rFiGrAtt.getStartColor())));
                o_rVCLGradient.SetEndColor(Color(maBColorModifierStack.getModifiedColor(rFiGrAtt.getEndColor())));
            }

            o_rVCLGradient.SetAngle(static_cast< sal_uInt16 >(rFiGrAtt.getAngle() * (1.0 / F_PI1800)));
            o_rVCLGradient.SetBorder(static_cast< sal_uInt16 >(rFiGrAtt.getBorder() * 100.0));
            o_rVCLGradient.SetOfsX(static_cast< sal_uInt16 >(rFiGrAtt.getOffsetX() * 100.0));
            o_rVCLGradient.SetOfsY(static_cast< sal_uInt16 >(rFiGrAtt.getOffsetY() * 100.0));
            o_rVCLGradient.SetSteps(rFiGrAtt.getSteps());

            
            o_rVCLGradient.SetStartIntensity(100);
            o_rVCLGradient.SetEndIntensity(100);

            switch(rFiGrAtt.getStyle())
            {
                default : 
                {
                    o_rVCLGradient.SetStyle(GradientStyle_LINEAR);
                    break;
                }
                case attribute::GRADIENTSTYLE_AXIAL :
                {
                    o_rVCLGradient.SetStyle(GradientStyle_AXIAL);
                    break;
                }
                case attribute::GRADIENTSTYLE_RADIAL :
                {
                    o_rVCLGradient.SetStyle(GradientStyle_RADIAL);
                    break;
                }
                case attribute::GRADIENTSTYLE_ELLIPTICAL :
                {
                    o_rVCLGradient.SetStyle(GradientStyle_ELLIPTICAL);
                    break;
                }
                case attribute::GRADIENTSTYLE_SQUARE :
                {
                    o_rVCLGradient.SetStyle(GradientStyle_SQUARE);
                    break;
                }
                case attribute::GRADIENTSTYLE_RECT :
                {
                    o_rVCLGradient.SetStyle(GradientStyle_RECT);
                    break;
                }
            }
        }

        void VclMetafileProcessor2D::impStartSvtGraphicFill(SvtGraphicFill* pSvtGraphicFill)
        {
            if(pSvtGraphicFill && !mnSvtGraphicFillCount)
            {
                SvMemoryStream aMemStm;

                WriteSvtGraphicFill( aMemStm, *pSvtGraphicFill );
                mpMetaFile->AddAction(new MetaCommentAction("XPATHFILL_SEQ_BEGIN", 0, static_cast< const sal_uInt8* >(aMemStm.GetData()), aMemStm.Seek(STREAM_SEEK_TO_END)));
                mnSvtGraphicFillCount++;
            }
        }

        void VclMetafileProcessor2D::impEndSvtGraphicFill(SvtGraphicFill* pSvtGraphicFill)
        {
            if(pSvtGraphicFill && mnSvtGraphicFillCount)
            {
                mnSvtGraphicFillCount--;
                mpMetaFile->AddAction(new MetaCommentAction("XPATHFILL_SEQ_END"));
                delete pSvtGraphicFill;
            }
        }

        double VclMetafileProcessor2D::getTransformedLineWidth( double fWidth ) const
        {
            
            
            
            
            
            const basegfx::B2DVector aDiscreteUnit( maCurrentTransformation * basegfx::B2DVector( fWidth, 0.0 ) );

            return aDiscreteUnit.getLength();
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
                basegfx::B2DPolygon aLocalPolygon(rB2DPolygon);
                basegfx::BColor aStrokeColor;
                basegfx::B2DPolyPolygon aStartArrow;
                basegfx::B2DPolyPolygon aEndArrow;

                if(pColor)
                {
                    aStrokeColor = *pColor;
                }
                else if(pLineAttribute)
                {
                    aStrokeColor = maBColorModifierStack.getModifiedColor(pLineAttribute->getColor());
                }

                
                
                mpOutputDevice->SetLineColor(Color(aStrokeColor));

                if(!aLocalPolygon.isClosed())
                {
                    double fPolyLength(0.0);
                    double fStart(0.0);
                    double fEnd(0.0);

                    if(pStart && pStart->isActive())
                    {
                        fPolyLength = basegfx::tools::getLength(aLocalPolygon);

                        aStartArrow = basegfx::tools::createAreaGeometryForLineStartEnd(
                            aLocalPolygon, pStart->getB2DPolyPolygon(), true, pStart->getWidth(),
                            fPolyLength, pStart->isCentered() ? 0.5 : 0.0, &fStart);
                    }

                    if(pEnd && pEnd->isActive())
                    {
                        if(basegfx::fTools::equalZero(fPolyLength))
                        {
                            fPolyLength = basegfx::tools::getLength(aLocalPolygon);
                        }

                        aEndArrow = basegfx::tools::createAreaGeometryForLineStartEnd(
                            aLocalPolygon, pEnd->getB2DPolyPolygon(), false, pEnd->getWidth(),
                            fPolyLength, pEnd->isCentered() ? 0.5 : 0.0, &fEnd);
                    }

                    if(0.0 != fStart || 0.0 != fEnd)
                    {
                        
                        aLocalPolygon = basegfx::tools::getSnippetAbsolute(aLocalPolygon, fStart, fPolyLength - fEnd, fPolyLength);
                    }
                }

                SvtGraphicStroke::JoinType eJoin(SvtGraphicStroke::joinNone);
                SvtGraphicStroke::CapType eCap(SvtGraphicStroke::capButt);
                double fLineWidth(0.0);
                double fMiterLength(0.0);
                SvtGraphicStroke::DashArray aDashArray;

                if(pLineAttribute)
                {
                    fLineWidth = fMiterLength = getTransformedLineWidth( pLineAttribute->getWidth() );

                    
                    switch(pLineAttribute->getLineJoin())
                    {
                        default : 
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
                            
                            fMiterLength /= rtl::math::sin(M_PI * (15.0 / 360.0));
                            break;
                        }
                        case basegfx::B2DLINEJOIN_ROUND :
                        {
                            eJoin = SvtGraphicStroke::joinRound;
                            break;
                        }
                    }

                    
                    switch(pLineAttribute->getLineCap())
                    {
                        default: /* com::sun::star::drawing::LineCap_BUTT */
                        {
                            eCap = SvtGraphicStroke::capButt;
                            break;
                        }
                        case com::sun::star::drawing::LineCap_ROUND:
                        {
                            eCap = SvtGraphicStroke::capRound;
                            break;
                        }
                        case com::sun::star::drawing::LineCap_SQUARE:
                        {
                            eCap = SvtGraphicStroke::capSquare;
                            break;
                        }
                    }
                }

                if(pStrokeAttribute)
                {
                    
                    aDashArray = pStrokeAttribute->getDotDashArray();
                }

                
                
                
                
                
                
                
                aLocalPolygon.transform(maCurrentTransformation);
                aStartArrow.transform(maCurrentTransformation);
                aEndArrow.transform(maCurrentTransformation);

                pRetval = new SvtGraphicStroke(
                    Polygon(aLocalPolygon),
                    PolyPolygon(aStartArrow),
                    PolyPolygon(aEndArrow),
                    mfCurrentUnifiedTransparence,
                    fLineWidth,
                    eCap,
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

                WriteSvtGraphicStroke( aMemStm, *pSvtGraphicStroke );
                mpMetaFile->AddAction(new MetaCommentAction("XPATHSTROKE_SEQ_BEGIN", 0, static_cast< const sal_uInt8* >(aMemStm.GetData()), aMemStm.Seek(STREAM_SEEK_TO_END)));
                mnSvtGraphicStrokeCount++;
            }
        }

        void VclMetafileProcessor2D::impEndSvtGraphicStroke(SvtGraphicStroke* pSvtGraphicStroke)
        {
            if(pSvtGraphicStroke && mnSvtGraphicStrokeCount)
            {
                mnSvtGraphicStrokeCount--;
                mpMetaFile->AddAction(new MetaCommentAction("XPATHSTROKE_SEQ_END"));
                delete pSvtGraphicStroke;
            }
        }

        
        uno::Reference< ::com::sun::star::i18n::XBreakIterator > VclMetafileProcessor2D::mxBreakIterator;

        VclMetafileProcessor2D::VclMetafileProcessor2D(const geometry::ViewInformation2D& rViewInformation, OutputDevice& rOutDev)
        :   VclProcessor2D(rViewInformation, rOutDev),
            mpMetaFile(rOutDev.GetConnectMetaFile()),
            mnSvtGraphicFillCount(0),
            mnSvtGraphicStrokeCount(0),
            mfCurrentUnifiedTransparence(0.0),
            mpPDFExtOutDevData(dynamic_cast< vcl::PDFExtOutDevData* >(rOutDev.GetExtOutDevData()))
        {
            OSL_ENSURE(rOutDev.GetConnectMetaFile(), "VclMetafileProcessor2D: Used on OutDev which has no MetaFile Target (!)");
            
            
            maCurrentTransformation = rViewInformation.getObjectTransformation();
        }

        VclMetafileProcessor2D::~VclMetafileProcessor2D()
        {
            
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
            UnifiedTransparencePrimitive2D, too, when interpreted as normally filled PolyPolygon.
            Implemented for:
                PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D,
                PRIMITIVE2D_ID_POLYPOLYGONHATCHPRIMITIVE2D,
                PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D,
                PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D,
                and for PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D when detected unified transparence

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
            was explicitly created for the printer already again to some default maximum
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
            switch(rCandidate.getPrimitive2DID())
            {
                case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D :
                {
                    
                    
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
                        
                        
                        const Graphic& rGraphic = rGraphicPrimitive.getGraphicObject().GetGraphic();

                        if(rGraphic.IsLink())
                        {
                            const GraphicAttr& rAttr = rGraphicPrimitive.getGraphicAttr();

                            if(!rAttr.IsSpecialDrawMode() && !rAttr.IsAdjusted())
                            {
                                const basegfx::B2DHomMatrix& rTransform = rGraphicPrimitive.getTransform();
                                double fRotate, fShearX;
                                rTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                                if( basegfx::fTools::equalZero( fRotate ) && ( aScale.getX() > 0.0 ) && ( aScale.getY() > 0.0 ) )
                                {
                                    bUsingPDFExtOutDevData = true;
                                    mpPDFExtOutDevData->BeginGroup();
                                }
                            }
                        }
                    }

                    
                    process(rGraphicPrimitive.get2DDecomposition(getViewInformation2D()));

                    if(bUsingPDFExtOutDevData)
                    {
                        
                        
                        const basegfx::B2DRange aCurrentRange(
                            aTranslate.getX(), aTranslate.getY(),
                            aTranslate.getX() + aScale.getX(), aTranslate.getY() + aScale.getY());
                        const Rectangle aCurrentRect(
                            sal_Int32(floor(aCurrentRange.getMinX())), sal_Int32(floor(aCurrentRange.getMinY())),
                            sal_Int32(ceil(aCurrentRange.getMaxX())), sal_Int32(ceil(aCurrentRange.getMaxY())));
                        const GraphicAttr& rAttr = rGraphicPrimitive.getGraphicAttr();
                        
                        Rectangle aCropRect(aCurrentRect);

                        if(rAttr.IsCropped())
                        {
                            
                            
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

                            
                            basegfx::B2DRange aCropRange;
                            aCropRange.expand(aCurrentRange.getMinimum() - basegfx::B2DPoint(rAttr.GetLeftCrop() * fFactorX, rAttr.GetTopCrop() * fFactorY));
                            aCropRange.expand(aCurrentRange.getMaximum() + basegfx::B2DPoint(rAttr.GetRightCrop() * fFactorX, rAttr.GetBottomCrop() * fFactorY));

                            aCropRect = Rectangle(
                                sal_Int32(floor(aCropRange.getMinX())), sal_Int32(floor(aCropRange.getMinY())),
                                sal_Int32(ceil(aCropRange.getMaxX())), sal_Int32(ceil(aCropRange.getMaxY())));
                        }

                        
                        
                        
                        mpPDFExtOutDevData->EndGroup(rGraphicPrimitive.getGraphicObject().GetGraphic(),
                            rAttr.GetTransparency(),
                            aCropRect,
                            aCurrentRect);
                    }

                    break;
                }
                case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D :
                {
                    const primitive2d::ControlPrimitive2D& rControlPrimitive = static_cast< const primitive2d::ControlPrimitive2D& >(rCandidate);
                    const uno::Reference< awt::XControl >& rXControl(rControlPrimitive.getXControl());
                    bool bIsPrintableControl(false);

                    
                    if(rXControl.is())
                    {
                        try
                        {
                            uno::Reference< beans::XPropertySet > xModelProperties(rXControl->getModel(), uno::UNO_QUERY);
                            uno::Reference< beans::XPropertySetInfo > xPropertyInfo(xModelProperties.is()
                                ? xModelProperties->getPropertySetInfo()
                                : uno::Reference< beans::XPropertySetInfo >());
                            const OUString sPrintablePropertyName("Printable");

                            if(xPropertyInfo.is() && xPropertyInfo->hasPropertyByName(sPrintablePropertyName))
                            {
                                OSL_VERIFY(xModelProperties->getPropertyValue(sPrintablePropertyName) >>= bIsPrintableControl);
                            }
                        }
                        catch(const uno::Exception&)
                        {
                            OSL_FAIL("VclMetafileProcessor2D: No access to printable flag of Control, caught an exception!");
                        }
                    }

                    
                    if(bIsPrintableControl)
                    {
                        const bool bPDFExport(mpPDFExtOutDevData && mpPDFExtOutDevData->GetIsExportFormFields());
                        bool bDoProcessRecursively(true);

                        if(bPDFExport)
                        {
                            
                            
                            
                            ::std::auto_ptr< ::vcl::PDFWriter::AnyWidget > pPDFControl;
                            ::toolkitform::describePDFControl( rXControl, pPDFControl, *mpPDFExtOutDevData );

                            if(pPDFControl.get())
                            {
                                
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

                                
                                
                                bDoProcessRecursively = false;
                            }
                            else
                            {
                                
                                
                                
                            }
                        }

                        
                        if(bDoProcessRecursively)
                        {
                            
                            try
                            {
                                
                                uno::Reference< awt::XView > xControlView(rXControl, uno::UNO_QUERY_THROW);
                                const uno::Reference< awt::XGraphics > xOriginalGraphics(xControlView->getGraphics());
                                const uno::Reference< awt::XGraphics > xNewGraphics(mpOutputDevice->CreateUnoGraphics());

                                if(xNewGraphics.is())
                                {
                                    
                                    xControlView->setGraphics(xNewGraphics);

                                    
                                    const basegfx::B2DHomMatrix aObjectToDiscrete(getViewInformation2D().getObjectToViewTransformation() * rControlPrimitive.getTransform());
                                    const basegfx::B2DPoint aTopLeftDiscrete(aObjectToDiscrete * basegfx::B2DPoint(0.0, 0.0));

                                    
                                    xControlView->draw(basegfx::fround(aTopLeftDiscrete.getX()), basegfx::fround(aTopLeftDiscrete.getY()));
                                    bDoProcessRecursively = false;

                                    
                                    xControlView->setGraphics(xOriginalGraphics);
                                }
                            }
                            catch( const uno::Exception& )
                            {
                                OSL_FAIL("VclMetafileProcessor2D: Printing of Control failed, caught an exception!");
                            }
                        }

                        
                        if(bDoProcessRecursively)
                        {
                            process(rControlPrimitive.get2DDecomposition(getViewInformation2D()));
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_TEXTHIERARCHYFIELDPRIMITIVE2D :
                {
                    
                    
                    const primitive2d::TextHierarchyFieldPrimitive2D& rFieldPrimitive = static_cast< const primitive2d::TextHierarchyFieldPrimitive2D& >(rCandidate);
                    const OString aCommentStringCommon("FIELD_SEQ_BEGIN");
                    const OString aCommentStringPage("FIELD_SEQ_BEGIN;PageField");
                    const OString aCommentStringEnd("FIELD_SEQ_END");

                    switch(rFieldPrimitive.getType())
                    {
                        default : 
                        {
                            mpMetaFile->AddAction(new MetaCommentAction(aCommentStringCommon));
                            break;
                        }
                        case drawinglayer::primitive2d::FIELD_TYPE_PAGE :
                        {
                            mpMetaFile->AddAction(new MetaCommentAction(aCommentStringPage));
                            break;
                        }
                        case drawinglayer::primitive2d::FIELD_TYPE_URL :
                        {
                            const OUString& rURL = rFieldPrimitive.getString();
                            const OUString aOldString(rURL);
                            mpMetaFile->AddAction(new MetaCommentAction(aCommentStringCommon, 0, reinterpret_cast< const sal_uInt8* >(aOldString.getStr()), 2 * aOldString.getLength()));
                            break;
                        }
                    }

                    
                    const primitive2d::Primitive2DSequence rContent = rFieldPrimitive.get2DDecomposition(getViewInformation2D());
                    process(rContent);

                    
                    mpMetaFile->AddAction(new MetaCommentAction(aCommentStringEnd));

                    if(mpPDFExtOutDevData && drawinglayer::primitive2d::FIELD_TYPE_URL == rFieldPrimitive.getType())
                    {
                        
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
                    const OString aCommentString("XTEXT_EOL");

                    
                    process(rLinePrimitive.get2DDecomposition(getViewInformation2D()));
                    mpMetaFile->AddAction(new MetaCommentAction(aCommentString));

                    break;
                }
                case PRIMITIVE2D_ID_TEXTHIERARCHYBULLETPRIMITIVE2D :
                {
                    
                    
                    const primitive2d::TextHierarchyBulletPrimitive2D& rBulletPrimitive = static_cast< const primitive2d::TextHierarchyBulletPrimitive2D& >(rCandidate);
                    const OString aCommentString("XTEXT_EOC");

                    
                    process(rBulletPrimitive.get2DDecomposition(getViewInformation2D()));
                    mpMetaFile->AddAction(new MetaCommentAction(aCommentString));

                    break;
                }
                case PRIMITIVE2D_ID_TEXTHIERARCHYPARAGRAPHPRIMITIVE2D :
                {
                    const primitive2d::TextHierarchyParagraphPrimitive2D& rParagraphPrimitive = static_cast< const primitive2d::TextHierarchyParagraphPrimitive2D& >(rCandidate);
                    const OString aCommentString("XTEXT_EOP");

                    if(mpPDFExtOutDevData)
                    {
                        
                        mpPDFExtOutDevData->BeginStructureElement( vcl::PDFWriter::Paragraph );
                    }

                    
                    process(rParagraphPrimitive.get2DDecomposition(getViewInformation2D()));
                    mpMetaFile->AddAction(new MetaCommentAction(aCommentString));

                    if(mpPDFExtOutDevData)
                    {
                        
                        mpPDFExtOutDevData->EndStructureElement();
                    }

                    break;
                }
                case PRIMITIVE2D_ID_TEXTHIERARCHYBLOCKPRIMITIVE2D :
                {
                    const primitive2d::TextHierarchyBlockPrimitive2D& rBlockPrimitive = static_cast< const primitive2d::TextHierarchyBlockPrimitive2D& >(rCandidate);
                    const OString aCommentStringA("XTEXT_PAINTSHAPE_BEGIN");
                    const OString aCommentStringB("XTEXT_PAINTSHAPE_END");

                    
                    mpMetaFile->AddAction(new MetaCommentAction(aCommentStringA));
                    process(rBlockPrimitive.get2DDecomposition(getViewInformation2D()));
                    mpMetaFile->AddAction(new MetaCommentAction(aCommentStringB));

                    break;
                }
                case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D :
                case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D :
                {
                    
                    const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate = static_cast< const primitive2d::TextSimplePortionPrimitive2D& >(rCandidate);
                    

                    
                    const sal_uInt32 nOriginalDrawMode(mpOutputDevice->GetDrawMode());
                    adaptTextToFillDrawMode();

                    
                    RenderTextSimpleOrDecoratedPortionPrimitive2D(rTextCandidate);

                    
                    mpOutputDevice->SetDrawMode(nOriginalDrawMode);

                    
                    {
                        
                        if(!mxBreakIterator.is())
                        {
                            uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                            mxBreakIterator = i18n::BreakIterator::create(xContext);
                        }

                        const OUString& rTxt = rTextCandidate.getText();
                        const sal_Int32 nTextLength(rTextCandidate.getTextLength()); 

                        if(nTextLength)
                        {
                            const ::com::sun::star::lang::Locale& rLocale = rTextCandidate.getLocale();
                            const sal_Int32 nTextPosition(rTextCandidate.getTextPosition());

                            sal_Int32 nDone;
                            sal_Int32 nNextCellBreak(mxBreakIterator->nextCharacters(rTxt, nTextPosition, rLocale, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL, 0, nDone));
                            ::com::sun::star::i18n::Boundary nNextWordBoundary(mxBreakIterator->getWordBoundary(rTxt, nTextPosition, rLocale, ::com::sun::star::i18n::WordType::ANY_WORD, sal_True));
                            sal_Int32 nNextSentenceBreak(mxBreakIterator->endOfSentence(rTxt, nTextPosition, rLocale));
                            const OString aCommentStringA("XTEXT_EOC");
                            const OString aCommentStringB("XTEXT_EOW");
                            const OString aCommentStringC("XTEXT_EOS");

                            for(sal_Int32 i(nTextPosition); i < nTextPosition + nTextLength; i++)
                            {
                                
                                if(i == nNextCellBreak)
                                {
                                    mpMetaFile->AddAction(new MetaCommentAction(aCommentStringA, i - nTextPosition));
                                    nNextCellBreak = mxBreakIterator->nextCharacters(rTxt, i, rLocale, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
                                }
                                if(i == nNextWordBoundary.endPos)
                                {
                                    mpMetaFile->AddAction(new MetaCommentAction(aCommentStringB, i - nTextPosition));
                                    nNextWordBoundary = mxBreakIterator->getWordBoundary(rTxt, i + 1, rLocale, ::com::sun::star::i18n::WordType::ANY_WORD, sal_True);
                                }
                                if(i == nNextSentenceBreak)
                                {
                                    mpMetaFile->AddAction(new MetaCommentAction(aCommentStringC, i - nTextPosition));
                                    nNextSentenceBreak = mxBreakIterator->endOfSentence(rTxt, i + 1, rLocale);
                                }
                            }
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                {
                    const primitive2d::PolygonHairlinePrimitive2D& rHairlinePrimitive = static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate);
                    const basegfx::B2DPolygon& rBasePolygon = rHairlinePrimitive.getB2DPolygon();

                    if(rBasePolygon.count() > (MAX_POLYGON_POINT_COUNT_METAFILE - 1))
                    {
                        
                        
                        basegfx::B2DPolygon aLeft, aRight;
                        splitLinePolygon(rBasePolygon, aLeft, aRight);
                        const primitive2d::PolygonHairlinePrimitive2D aPLeft(aLeft, rHairlinePrimitive.getBColor());
                        const primitive2d::PolygonHairlinePrimitive2D aPRight(aRight, rHairlinePrimitive.getBColor());

                        processBasePrimitive2D(aPLeft);
                        processBasePrimitive2D(aPRight);
                    }
                    else
                    {
                        
                        
                        const basegfx::BColor aLineColor(maBColorModifierStack.getModifiedColor(rHairlinePrimitive.getBColor()));
                        SvtGraphicStroke* pSvtGraphicStroke = 0;

                        
                        
                        
                        bool bSupportSvtGraphicStroke(false);

                        if(bSupportSvtGraphicStroke)
                        {
                            pSvtGraphicStroke = impTryToCreateSvtGraphicStroke(
                                rHairlinePrimitive.getB2DPolygon(),
                                &aLineColor,
                                0, 0, 0, 0);

                            impStartSvtGraphicStroke(pSvtGraphicStroke);
                        }

                        RenderPolygonHairlinePrimitive2D(static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate), false);

                        if(bSupportSvtGraphicStroke)
                        {
                            impEndSvtGraphicStroke(pSvtGraphicStroke);
                        }
                    }
                    break;
                }
                case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D :
                {
                    const primitive2d::PolygonStrokePrimitive2D& rStrokePrimitive = static_cast< const primitive2d::PolygonStrokePrimitive2D& >(rCandidate);
                    const basegfx::B2DPolygon& rBasePolygon = rStrokePrimitive.getB2DPolygon();

                    if(rBasePolygon.count() > (MAX_POLYGON_POINT_COUNT_METAFILE - 1))
                    {
                        
                        
                        basegfx::B2DPolygon aLeft, aRight;
                        splitLinePolygon(rBasePolygon, aLeft, aRight);
                        const primitive2d::PolygonStrokePrimitive2D aPLeft(
                            aLeft, rStrokePrimitive.getLineAttribute(), rStrokePrimitive.getStrokeAttribute());
                        const primitive2d::PolygonStrokePrimitive2D aPRight(
                            aRight, rStrokePrimitive.getLineAttribute(), rStrokePrimitive.getStrokeAttribute());

                        processBasePrimitive2D(aPLeft);
                        processBasePrimitive2D(aPRight);
                    }
                    else
                    {
                        
                        SvtGraphicStroke* pSvtGraphicStroke = impTryToCreateSvtGraphicStroke(
                            rBasePolygon, 0,
                            &rStrokePrimitive.getLineAttribute(),
                            &rStrokePrimitive.getStrokeAttribute(),
                            0, 0);

                        impStartSvtGraphicStroke(pSvtGraphicStroke);
                        const attribute::LineAttribute& rLine = rStrokePrimitive.getLineAttribute();

                        
                        if(basegfx::fTools::more(rLine.getWidth(), 0.0))
                        {
                            const attribute::StrokeAttribute& rStroke = rStrokePrimitive.getStrokeAttribute();
                            basegfx::B2DPolyPolygon aHairLinePolyPolygon;

                            if(0.0 == rStroke.getFullDotDashLen())
                            {
                                aHairLinePolyPolygon.append(rBasePolygon);
                            }
                            else
                            {
                                basegfx::tools::applyLineDashing(
                                    rBasePolygon, rStroke.getDotDashArray(),
                                    &aHairLinePolyPolygon, 0, rStroke.getFullDotDashLen());
                            }

                            const basegfx::BColor aHairlineColor(maBColorModifierStack.getModifiedColor(rLine.getColor()));
                            mpOutputDevice->SetLineColor(Color(aHairlineColor));
                            mpOutputDevice->SetFillColor();
                            aHairLinePolyPolygon.transform(maCurrentTransformation);

                            
                            LineInfo aLineInfo(LINE_SOLID, basegfx::fround(getTransformedLineWidth(rLine.getWidth())));
                            aLineInfo.SetLineJoin(rLine.getLineJoin());
                            aLineInfo.SetLineCap(rLine.getLineCap());

                            for(sal_uInt32 a(0); a < aHairLinePolyPolygon.count(); a++)
                            {
                                const basegfx::B2DPolygon aCandidate(aHairLinePolyPolygon.getB2DPolygon(a));

                                if(aCandidate.count() > 1)
                                {
                                    const Polygon aToolsPolygon(aCandidate);

                                    mpMetaFile->AddAction(new MetaPolyLineAction(aToolsPolygon, aLineInfo));
                                }
                            }
                        }
                        else
                        {
                            process(rCandidate.get2DDecomposition(getViewInformation2D()));
                        }

                        impEndSvtGraphicStroke(pSvtGraphicStroke);
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D :
                {
                    const primitive2d::PolygonStrokeArrowPrimitive2D& rStrokeArrowPrimitive = static_cast< const primitive2d::PolygonStrokeArrowPrimitive2D& >(rCandidate);
                    const basegfx::B2DPolygon& rBasePolygon = rStrokeArrowPrimitive.getB2DPolygon();

                    if(rBasePolygon.count() > (MAX_POLYGON_POINT_COUNT_METAFILE - 1))
                    {
                        
                        
                        basegfx::B2DPolygon aLeft, aRight;
                        splitLinePolygon(rBasePolygon, aLeft, aRight);
                        const attribute::LineStartEndAttribute aEmpty;
                        const primitive2d::PolygonStrokeArrowPrimitive2D aPLeft(
                            aLeft,
                            rStrokeArrowPrimitive.getLineAttribute(),
                            rStrokeArrowPrimitive.getStrokeAttribute(),
                            rStrokeArrowPrimitive.getStart(),
                            aEmpty);
                        const primitive2d::PolygonStrokeArrowPrimitive2D aPRight(
                            aRight,
                            rStrokeArrowPrimitive.getLineAttribute(),
                            rStrokeArrowPrimitive.getStrokeAttribute(),
                            aEmpty,
                            rStrokeArrowPrimitive.getEnd());

                        processBasePrimitive2D(aPLeft);
                        processBasePrimitive2D(aPRight);
                    }
                    else
                    {
                        
                        SvtGraphicStroke* pSvtGraphicStroke = impTryToCreateSvtGraphicStroke(
                            rBasePolygon, 0,
                            &rStrokeArrowPrimitive.getLineAttribute(),
                            &rStrokeArrowPrimitive.getStrokeAttribute(),
                            &rStrokeArrowPrimitive.getStart(),
                            &rStrokeArrowPrimitive.getEnd());

                        
                        impStartSvtGraphicStroke(pSvtGraphicStroke);

                        
                        
                        
                        
                        
                        
                        
                        
                        const sal_uLong nOldDrawMode(mpOutputDevice->GetDrawMode());
                        const bool bDrawmodeChange(nOldDrawMode & DRAWMODE_WHITEFILL && mnSvtGraphicStrokeCount);

                        if(bDrawmodeChange)
                        {
                            mpOutputDevice->SetDrawMode((nOldDrawMode & ~DRAWMODE_WHITEFILL) | DRAWMODE_BLACKFILL);
                        }

                        
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));

                        if(bDrawmodeChange)
                        {
                            mpOutputDevice->SetDrawMode(nOldDrawMode);
                        }

                        
                        impEndSvtGraphicStroke(pSvtGraphicStroke);
                    }

                    break;
                }
                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                {
                    
                    
                    
                    
                    RenderBitmapPrimitive2D(static_cast< const primitive2d::BitmapPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D :
                {
                    
                    const primitive2d::PolyPolygonGraphicPrimitive2D& rBitmapCandidate = static_cast< const primitive2d::PolyPolygonGraphicPrimitive2D& >(rCandidate);
                    basegfx::B2DPolyPolygon aLocalPolyPolygon(rBitmapCandidate.getB2DPolyPolygon());

                    if(fillPolyPolygonNeededToBeSplit(aLocalPolyPolygon))
                    {
                        
                        
                        const primitive2d::PolyPolygonGraphicPrimitive2D aSplitted(
                            aLocalPolyPolygon,
                            rBitmapCandidate.getFillGraphic());

                        processBasePrimitive2D(aSplitted);
                    }
                    else
                    {
                        SvtGraphicFill* pSvtGraphicFill = 0;

                        if(!mnSvtGraphicFillCount && aLocalPolyPolygon.count())
                        {
                            
                            

                            
                            
                            aLocalPolyPolygon.transform(maCurrentTransformation);
                            const basegfx::B2DVector aOutlineSize(aLocalPolyPolygon.getB2DRange().getRange());

                            
                            const attribute::FillGraphicAttribute& rFillGraphicAttribute = rBitmapCandidate.getFillGraphic();
                            const Size aBmpSizePixel(rFillGraphicAttribute.getGraphic().GetSizePixel());

                            
                            
                            
                            
                            const basegfx::B2DVector aTransformScale(
                                rFillGraphicAttribute.getGraphicRange().getRange() /
                                basegfx::B2DVector(
                                    std::max(1.0, double(aBmpSizePixel.Width())),
                                    std::max(1.0, double(aBmpSizePixel.Height()))) *
                                aOutlineSize);
                            const basegfx::B2DPoint aTransformPosition(
                                rFillGraphicAttribute.getGraphicRange().getMinimum() * aOutlineSize);

                            
                            SvtGraphicFill::Transform aTransform;

                            
                            aTransform.matrix[0] = aTransformScale.getX();
                            aTransform.matrix[4] = aTransformScale.getY();

                            
                            aTransform.matrix[2] = aTransformPosition.getX();
                            aTransform.matrix[5] = aTransformPosition.getY();

                            pSvtGraphicFill = new SvtGraphicFill(
                                getFillPolyPolygon(aLocalPolyPolygon),
                                Color(),
                                0.0,
                                SvtGraphicFill::fillEvenOdd,
                                SvtGraphicFill::fillTexture,
                                aTransform,
                                rFillGraphicAttribute.getTiling(),
                                SvtGraphicFill::hatchSingle,
                                Color(),
                                SvtGraphicFill::gradientLinear,
                                Color(),
                                Color(),
                                0,
                                rFillGraphicAttribute.getGraphic());
                        }

                        
                        impStartSvtGraphicFill(pSvtGraphicFill);
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                        impEndSvtGraphicFill(pSvtGraphicFill);
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONHATCHPRIMITIVE2D :
                {
                    
                    const primitive2d::PolyPolygonHatchPrimitive2D& rHatchCandidate = static_cast< const primitive2d::PolyPolygonHatchPrimitive2D& >(rCandidate);
                    const attribute::FillHatchAttribute& rFillHatchAttribute = rHatchCandidate.getFillHatch();
                    basegfx::B2DPolyPolygon aLocalPolyPolygon(rHatchCandidate.getB2DPolyPolygon());

                    
                    
                    while(fillPolyPolygonNeededToBeSplit(aLocalPolyPolygon))
                        ;

                    if(rFillHatchAttribute.isFillBackground())
                    {
                        
                        
                        
                        const primitive2d::Primitive2DReference xBackground(
                            new primitive2d::PolyPolygonColorPrimitive2D(
                                aLocalPolyPolygon,
                                rHatchCandidate.getBackgroundColor()));

                        process(primitive2d::Primitive2DSequence(&xBackground, 1));
                    }

                    SvtGraphicFill* pSvtGraphicFill = 0;
                    aLocalPolyPolygon.transform(maCurrentTransformation);

                    if(!mnSvtGraphicFillCount && aLocalPolyPolygon.count())
                    {
                        
                        SvtGraphicFill::HatchType eHatch(SvtGraphicFill::hatchSingle);

                        switch(rFillHatchAttribute.getStyle())
                        {
                            default: 
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

                        
                        aTransform.matrix[0] *= rFillHatchAttribute.getDistance();
                        aTransform.matrix[4] *= rFillHatchAttribute.getDistance();

                        
                        aTransform.matrix[0] *= cos(rFillHatchAttribute.getAngle());
                        aTransform.matrix[1] *= -sin(rFillHatchAttribute.getAngle());
                        aTransform.matrix[3] *= sin(rFillHatchAttribute.getAngle());
                        aTransform.matrix[4] *= cos(rFillHatchAttribute.getAngle());

                        pSvtGraphicFill = new SvtGraphicFill(
                            getFillPolyPolygon(aLocalPolyPolygon),
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

                    
                    impStartSvtGraphicFill(pSvtGraphicFill);

                    
                    
                    const PolyPolygon aToolsPolyPolygon(basegfx::tools::adaptiveSubdivideByAngle(aLocalPolyPolygon));
                    const HatchStyle aHatchStyle(
                        attribute::HATCHSTYLE_SINGLE == rFillHatchAttribute.getStyle() ? HATCH_SINGLE :
                        attribute::HATCHSTYLE_DOUBLE == rFillHatchAttribute.getStyle() ? HATCH_DOUBLE :
                        HATCH_TRIPLE);

                    mpOutputDevice->DrawHatch(aToolsPolyPolygon,
                        Hatch(aHatchStyle,
                            Color(rFillHatchAttribute.getColor()),
                            basegfx::fround(rFillHatchAttribute.getDistance()),
                            basegfx::fround(rFillHatchAttribute.getAngle() / F_PI1800)));

                    impEndSvtGraphicFill(pSvtGraphicFill);

                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D :
                {
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate, fShearX;

                    maCurrentTransformation.decompose(aScale, aTranslate, fRotate, fShearX);

                    if(!basegfx::fTools::equalZero(fRotate) || !basegfx::fTools::equalZero(fShearX))
                    {
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }
                    else
                    {
                        const primitive2d::PolyPolygonGradientPrimitive2D& rGradientCandidate = static_cast< const primitive2d::PolyPolygonGradientPrimitive2D& >(rCandidate);
                        basegfx::B2DPolyPolygon aLocalPolyPolygon(rGradientCandidate.getB2DPolyPolygon());

                        
                        
                        while(fillPolyPolygonNeededToBeSplit(aLocalPolyPolygon))
                            ;

                        
                        
                        
                        Gradient aVCLGradient;
                        impConvertFillGradientAttributeToVCLGradient(aVCLGradient, rGradientCandidate.getFillGradient(), false);
                        aLocalPolyPolygon.transform(maCurrentTransformation);

                        
                        
                        
                        
                        
                        const PolyPolygon aToolsPolyPolygon(
                            getFillPolyPolygon(
                                basegfx::tools::adaptiveSubdivideByAngle(aLocalPolyPolygon)));


                        
                        SvtGraphicFill* pSvtGraphicFill = 0;

                        if(!mnSvtGraphicFillCount && aLocalPolyPolygon.count())
                        {
                            
                            SvtGraphicFill::GradientType eGrad(SvtGraphicFill::gradientLinear);

                            switch(aVCLGradient.GetStyle())
                            {
                                default : 
                                case GradientStyle_AXIAL:
                                    eGrad = SvtGraphicFill::gradientLinear;
                                    break;
                                case GradientStyle_RADIAL:
                                case GradientStyle_ELLIPTICAL:
                                    eGrad = SvtGraphicFill::gradientRadial;
                                    break;
                                case GradientStyle_SQUARE:
                                case GradientStyle_RECT:
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

                        
                        impStartSvtGraphicFill(pSvtGraphicFill);
                        mpOutputDevice->DrawGradient(aToolsPolyPolygon, aVCLGradient);
                        impEndSvtGraphicFill(pSvtGraphicFill);
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                {
                    const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(rCandidate));
                    basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());

                    
                    
                    while(fillPolyPolygonNeededToBeSplit(aLocalPolyPolygon))
                        ;

                    const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(rPolygonCandidate.getBColor()));
                    aLocalPolyPolygon.transform(maCurrentTransformation);

                    
                    SvtGraphicFill* pSvtGraphicFill = 0;

                    
                    
                    
                    bool bSupportSvtGraphicFill(false);

                    if(bSupportSvtGraphicFill && !mnSvtGraphicFillCount && aLocalPolyPolygon.count())
                    {
                        
                        pSvtGraphicFill = new SvtGraphicFill(
                            getFillPolyPolygon(aLocalPolyPolygon),
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

                    
                    mpOutputDevice->SetFillColor(Color(aPolygonColor));
                    mpOutputDevice->SetLineColor();

                    
                    if(bSupportSvtGraphicFill)
                    {
                            impStartSvtGraphicFill(pSvtGraphicFill);
                    }

                    mpOutputDevice->DrawPolyPolygon(aLocalPolyPolygon);

                    if(bSupportSvtGraphicFill)
                    {
                        impEndSvtGraphicFill(pSvtGraphicFill);
                    }

                    break;
                }
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                {
                    static bool bUseMetaFilePrimitiveDecomposition(true);
                    const primitive2d::MetafilePrimitive2D& aMetafile = static_cast< const primitive2d::MetafilePrimitive2D& >(rCandidate);

                    if(bUseMetaFilePrimitiveDecomposition && !aMetafile.getMetaFile().GetUseCanvas())
                    {
                        
                        
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }
                    else
                    {
                        
                        RenderMetafilePrimitive2D(aMetafile);
                    }

                    break;
                }
                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
                {
                    
                    const primitive2d::MaskPrimitive2D& rMaskCandidate = static_cast< const primitive2d::MaskPrimitive2D& >(rCandidate);

                    if(rMaskCandidate.getChildren().hasElements())
                    {
                        basegfx::B2DPolyPolygon aMask(rMaskCandidate.getMask());

                        if(aMask.count())
                        {
                            
                            aMask.transform(maCurrentTransformation);
                            const basegfx::B2DPolyPolygon aLastClipPolyPolygon(maClipPolyPolygon);

                            if(maClipPolyPolygon.count())
                            {
                                
                                
                                maClipPolyPolygon = basegfx::tools::clipPolyPolygonOnPolyPolygon(
                                    aMask,
                                    maClipPolyPolygon,
                                    true, 
                                    false);
                            }
                            else
                            {
                                
                                maClipPolyPolygon = aMask;
                            }

                            if(maClipPolyPolygon.count())
                            {
                                
                                
                                
                                mpOutputDevice->Push(PUSH_CLIPREGION);
                                mpOutputDevice->SetClipRegion(Region(maClipPolyPolygon));

                                
                                
                                
                                process(rMaskCandidate.getChildren());

                                
                                mpOutputDevice->Pop();
                            }

                            
                            maClipPolyPolygon = aLastClipPolyPolygon;
                        }
                        else
                        {
                            
                            process(rMaskCandidate.getChildren());
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D :
                {
                    
                    RenderModifiedColorPrimitive2D(static_cast< const primitive2d::ModifiedColorPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D :
                {
                    
                    
                    
                    
                    const primitive2d::UnifiedTransparencePrimitive2D& rUniTransparenceCandidate = static_cast< const primitive2d::UnifiedTransparencePrimitive2D& >(rCandidate);
                    const primitive2d::Primitive2DSequence rContent = rUniTransparenceCandidate.getChildren();

                    if(rContent.hasElements())
                    {
                        if(0.0 == rUniTransparenceCandidate.getTransparence())
                        {
                            
                            process(rUniTransparenceCandidate.getChildren());
                        }
                        else if(rUniTransparenceCandidate.getTransparence() > 0.0 && rUniTransparenceCandidate.getTransparence() < 1.0)
                        {
                            
                            
                            const primitive2d::PolyPolygonColorPrimitive2D* pPoPoColor = 0;
                            static bool bForceToMetafile(false);

                            if(!bForceToMetafile && 1 == rContent.getLength())
                            {
                                const primitive2d::Primitive2DReference xReference(rContent[0]);
                                pPoPoColor = dynamic_cast< const primitive2d::PolyPolygonColorPrimitive2D* >(xReference.get());
                            }

                            
                            
                            
                            if(pPoPoColor && PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D == pPoPoColor->getPrimitive2DID())
                            {
                                
                                const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(pPoPoColor->getBColor()));
                                basegfx::B2DPolyPolygon aLocalPolyPolygon(pPoPoColor->getB2DPolyPolygon());

                                
                                
                                while(fillPolyPolygonNeededToBeSplit(aLocalPolyPolygon))
                                    ;

                                
                                aLocalPolyPolygon.transform(maCurrentTransformation);

                                
                                SvtGraphicFill* pSvtGraphicFill = 0;

                                
                                
                                
                                bool bSupportSvtGraphicFill(false);

                                if(bSupportSvtGraphicFill && !mnSvtGraphicFillCount && aLocalPolyPolygon.count())
                                {
                                    
                                    pSvtGraphicFill = new SvtGraphicFill(
                                        getFillPolyPolygon(aLocalPolyPolygon),
                                        Color(aPolygonColor),
                                        rUniTransparenceCandidate.getTransparence(),
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

                                
                                const sal_uInt16 nTransPercentVcl((sal_uInt16)basegfx::fround(rUniTransparenceCandidate.getTransparence() * 100.0));
                                mpOutputDevice->SetFillColor(Color(aPolygonColor));
                                mpOutputDevice->SetLineColor();

                                
                                if(bSupportSvtGraphicFill)
                                {
                                    impStartSvtGraphicFill(pSvtGraphicFill);
                                }

                                mpOutputDevice->DrawTransparent(
                                    PolyPolygon(aLocalPolyPolygon),
                                    nTransPercentVcl);

                                if(bSupportSvtGraphicFill)
                                {
                                    impEndSvtGraphicFill(pSvtGraphicFill);
                                }
                            }
                            else
                            {
                                
                                
                                const double fLastCurrentUnifiedTransparence(mfCurrentUnifiedTransparence);
                                
                                
                                
                                mfCurrentUnifiedTransparence = 0;

                                
                                GDIMetaFile aContentMetafile;
                                const Rectangle aPrimitiveRectangle(impDumpToMetaFile(rContent, aContentMetafile));

                                
                                
                                mfCurrentUnifiedTransparence = fLastCurrentUnifiedTransparence;

                                
                                Gradient aVCLGradient;
                                const sal_uInt8 nTransPercentVcl((sal_uInt8)basegfx::fround(rUniTransparenceCandidate.getTransparence() * 255.0));
                                const Color aTransColor(nTransPercentVcl, nTransPercentVcl, nTransPercentVcl);

                                aVCLGradient.SetStyle(GradientStyle_LINEAR);
                                aVCLGradient.SetStartColor(aTransColor);
                                aVCLGradient.SetEndColor(aTransColor);
                                aVCLGradient.SetAngle(0);
                                aVCLGradient.SetBorder(0);
                                aVCLGradient.SetOfsX(0);
                                aVCLGradient.SetOfsY(0);
                                aVCLGradient.SetStartIntensity(100);
                                aVCLGradient.SetEndIntensity(100);
                                aVCLGradient.SetSteps(2);

                                
                                mpOutputDevice->DrawTransparent(
                                    aContentMetafile, aPrimitiveRectangle.TopLeft(),
                                    aPrimitiveRectangle.GetSize(), aVCLGradient);
                            }
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D :
                {
                    
                    
                    
                    
                    
                    
                    const primitive2d::TransparencePrimitive2D& rTransparenceCandidate = static_cast< const primitive2d::TransparencePrimitive2D& >(rCandidate);
                    const primitive2d::Primitive2DSequence rContent = rTransparenceCandidate.getChildren();
                    const primitive2d::Primitive2DSequence rTransparence = rTransparenceCandidate.getTransparence();

                    if(rContent.hasElements() && rTransparence.hasElements())
                    {
                        
                        
                        const primitive2d::FillGradientPrimitive2D* pFiGradient = 0;
                        static bool bForceToBigTransparentVDev(false);

                        if(!bForceToBigTransparentVDev && 1 == rTransparence.getLength())
                        {
                            const primitive2d::Primitive2DReference xReference(rTransparence[0]);
                            pFiGradient = dynamic_cast< const primitive2d::FillGradientPrimitive2D* >(xReference.get());
                        }

                        
                        if(pFiGradient && PRIMITIVE2D_ID_FILLGRADIENTPRIMITIVE2D == pFiGradient->getPrimitive2DID())
                        {
                            
                            GDIMetaFile aContentMetafile;
                            const Rectangle aPrimitiveRectangle(impDumpToMetaFile(rContent, aContentMetafile));

                            
                            Gradient aVCLGradient;
                            impConvertFillGradientAttributeToVCLGradient(aVCLGradient, pFiGradient->getFillGradient(), true);

                            
                            mpOutputDevice->DrawTransparent(
                                aContentMetafile, aPrimitiveRectangle.TopLeft(),
                                aPrimitiveRectangle.GetSize(), aVCLGradient);
                        }
                        else
                        {
                            
                            

                            
                            
                            

                            
                            
                            
                            
                            

                            basegfx::B2DRange aViewRange(primitive2d::getB2DRangeFromPrimitive2DSequence(rContent, getViewInformation2D()));
                            aViewRange.transform(maCurrentTransformation);
                            const Rectangle aRectLogic(
                                (sal_Int32)floor(aViewRange.getMinX()), (sal_Int32)floor(aViewRange.getMinY()),
                                (sal_Int32)ceil(aViewRange.getMaxX()), (sal_Int32)ceil(aViewRange.getMaxY()));
                            const Rectangle aRectPixel(mpOutputDevice->LogicToPixel(aRectLogic));
                            Size aSizePixel(aRectPixel.GetSize());
                            const Point aEmptyPoint;
                            VirtualDevice aBufferDevice;
                            const sal_uInt32 nMaxQuadratPixels(500000);
                            const sal_uInt32 nViewVisibleArea(aSizePixel.getWidth() * aSizePixel.getHeight());
                            double fReduceFactor(1.0);

                            if(nViewVisibleArea > nMaxQuadratPixels)
                            {
                                
                                fReduceFactor = sqrt((double)nMaxQuadratPixels / (double)nViewVisibleArea);
                                aSizePixel = Size(basegfx::fround((double)aSizePixel.getWidth() * fReduceFactor),
                                    basegfx::fround((double)aSizePixel.getHeight() * fReduceFactor));
                            }

                            if(aBufferDevice.SetOutputSizePixel(aSizePixel))
                            {
                                
                                MapMode aNewMapMode(mpOutputDevice->GetMapMode());
                                aNewMapMode.SetOrigin(Point(-aRectLogic.Left(), -aRectLogic.Top()));
                                aBufferDevice.SetMapMode(aNewMapMode);

                                
                                
                                
                                
                                basegfx::B2DHomMatrix aViewTransform(aBufferDevice.GetViewTransformation());
                                const Size aDPIOld(mpOutputDevice->LogicToPixel(Size(1, 1), MAP_INCH));
                                const Size aDPINew(aBufferDevice.LogicToPixel(Size(1, 1), MAP_INCH));
                                const double fDPIXChange((double)aDPIOld.getWidth() / (double)aDPINew.getWidth());
                                const double fDPIYChange((double)aDPIOld.getHeight() / (double)aDPINew.getHeight());

                                if(!basegfx::fTools::equal(fDPIXChange, 1.0) || !basegfx::fTools::equal(fDPIYChange, 1.0))
                                {
                                    aViewTransform.scale(fDPIXChange, fDPIYChange);
                                }

                                
                                if(!basegfx::fTools::equal(fReduceFactor, 1.0))
                                {
                                    aViewTransform.scale(fReduceFactor, fReduceFactor);
                                }

                                
                                
                                const geometry::ViewInformation2D aViewInfo(
                                    getViewInformation2D().getObjectTransformation(),
                                    aViewTransform,
                                    aViewRange,
                                    getViewInformation2D().getVisualizedPage(),
                                    getViewInformation2D().getViewTime(),
                                    getViewInformation2D().getExtendedInformationSequence());

                                VclPixelProcessor2D aBufferProcessor(aViewInfo, aBufferDevice);

                                
                                aBufferProcessor.process(rContent);
                                const Bitmap aBmContent(aBufferDevice.GetBitmap(aEmptyPoint, aSizePixel));

                                
                                aBufferDevice.Erase();
                                aBufferProcessor.process(rTransparence);
                                const AlphaMask aBmAlpha(aBufferDevice.GetBitmap(aEmptyPoint, aSizePixel));

                                
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
                    
                    RenderTransformPrimitive2D(static_cast< const primitive2d::TransformPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D :
                {
                    
                    RenderPagePreviewPrimitive2D(static_cast< const primitive2d::PagePreviewPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D :
                {
                    
                    RenderMarkerArrayPrimitive2D(static_cast< const primitive2d::MarkerArrayPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                {
                    
                    RenderPointArrayPrimitive2D(static_cast< const primitive2d::PointArrayPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_STRUCTURETAGPRIMITIVE2D :
                {
                    
                    const primitive2d::StructureTagPrimitive2D& rStructureTagCandidate = static_cast< const primitive2d::StructureTagPrimitive2D& >(rCandidate);
                    const vcl::PDFWriter::StructElement& rTagElement(rStructureTagCandidate.getStructureElement());
                    const bool bTagUsed(vcl::PDFWriter::NonStructElement != rTagElement);

                    if(mpPDFExtOutDevData &&  bTagUsed)
                    {
                        
                        mpPDFExtOutDevData->BeginStructureElement(rTagElement);
                    }

                    
                    process(rStructureTagCandidate.getChildren());

                    if(mpPDFExtOutDevData &&  bTagUsed)
                    {
                        
                        mpPDFExtOutDevData->EndStructureElement();
                    }

                    break;
                }
                case PRIMITIVE2D_ID_EPSPRIMITIVE2D :
                {
                    RenderEpsPrimitive2D(static_cast< const primitive2d::EpsPrimitive2D& >(rCandidate));
                    break;
                }
                default :
                {
                    
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    break;
                }
            }
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
