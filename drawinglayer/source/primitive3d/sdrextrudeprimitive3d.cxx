/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <drawinglayer/primitive3d/sdrextrudeprimitive3d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <drawinglayer/primitive3d/sdrdecompositiontools3d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrshadowattribute.hxx>



using namespace com::sun::star;



namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence SdrExtrudePrimitive3D::create3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const
        {
            Primitive3DSequence aRetval;

            
            const Slice3DVector& rSliceVector = getSlices();

            if(!rSliceVector.empty())
            {
                sal_uInt32 a;

                
                const ::com::sun::star::drawing::NormalsKind eNormalsKind(getSdr3DObjectAttribute().getNormalsKind());
                const bool bCreateNormals(::com::sun::star::drawing::NormalsKind_SPECIFIC == eNormalsKind);
                const bool bCreateTextureCoordiantesX(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionX());
                const bool bCreateTextureCoordiantesY(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionY());
                basegfx::B2DHomMatrix aTexTransform;

                if(!getSdrLFSAttribute().getFill().isDefault() && (bCreateTextureCoordiantesX || bCreateTextureCoordiantesY))
                {
                    const basegfx::B2DPolygon aFirstPolygon(maCorrectedPolyPolygon.getB2DPolygon(0L));
                    const double fFrontLength(basegfx::tools::getLength(aFirstPolygon));
                    const double fFrontArea(basegfx::tools::getArea(aFirstPolygon));
                    const double fSqrtFrontArea(sqrt(fFrontArea));
                    double fRelativeTextureWidth = basegfx::fTools::equalZero(fSqrtFrontArea) ? 1.0 : fFrontLength / fSqrtFrontArea;
                    fRelativeTextureWidth = (double)((sal_uInt32)(fRelativeTextureWidth - 0.5));

                    if(fRelativeTextureWidth < 1.0)
                    {
                        fRelativeTextureWidth = 1.0;
                    }

                    aTexTransform.translate(-0.5, -0.5);
                    aTexTransform.scale(-1.0, -1.0);
                    aTexTransform.translate(0.5, 0.5);
                    aTexTransform.scale(fRelativeTextureWidth, 1.0);
                }

                
                ::std::vector< basegfx::B3DPolyPolygon > aFill;
                extractPlanesFromSlice(aFill, rSliceVector,
                    bCreateNormals, getSmoothHorizontalNormals(), getSmoothNormals(), getSmoothLids(), false,
                    0.5, 0.6, bCreateTextureCoordiantesX || bCreateTextureCoordiantesY, aTexTransform);

                
                const basegfx::B3DRange aRange(getRangeFrom3DGeometry(aFill));

                
                if(!getSdrLFSAttribute().getFill().isDefault())
                {
                    if(::com::sun::star::drawing::NormalsKind_SPHERE == eNormalsKind)
                    {
                        applyNormalsKindSphereTo3DGeometry(aFill, aRange);
                    }
                    else if(::com::sun::star::drawing::NormalsKind_FLAT == eNormalsKind)
                    {
                        applyNormalsKindFlatTo3DGeometry(aFill);
                    }

                    if(getSdr3DObjectAttribute().getNormalsInvert())
                    {
                        applyNormalsInvertTo3DGeometry(aFill);
                    }
                }

                
                if(!getSdrLFSAttribute().getFill().isDefault())
                {
                    applyTextureTo3DGeometry(
                        getSdr3DObjectAttribute().getTextureProjectionX(),
                        getSdr3DObjectAttribute().getTextureProjectionY(),
                        aFill,
                        aRange,
                        getTextureSize());
                }

                if(!getSdrLFSAttribute().getFill().isDefault())
                {
                    
                    aRetval = create3DPolyPolygonFillPrimitives(
                        aFill,
                        getTransform(),
                        getTextureSize(),
                        getSdr3DObjectAttribute(),
                        getSdrLFSAttribute().getFill(),
                        getSdrLFSAttribute().getFillFloatTransGradient());
                }
                else
                {
                    
                    aRetval = createHiddenGeometryPrimitives3D(
                        aFill,
                        getTransform(),
                        getTextureSize(),
                        getSdr3DObjectAttribute());
                }

                
                if(!getSdrLFSAttribute().getLine().isDefault())
                {
                    if(getSdr3DObjectAttribute().getReducedLineGeometry())
                    {
                        
                        const basegfx::B3DPolyPolygon aVerLine(extractVerticalLinesFromSlice(rSliceVector));
                        const sal_uInt32 nCount(aVerLine.count());
                        basegfx::B3DPolyPolygon aReducedLoops;
                        basegfx::B3DPolyPolygon aNewLineGeometry;

                        
                        
                        for(a = 0; a < nCount; a++)
                        {
                            const sal_uInt32 nReducedCount(aReducedLoops.count());
                            const basegfx::B3DPolygon aCandidate(aVerLine.getB3DPolygon(a));
                            bool bAdd(true);

                            if(nReducedCount)
                            {
                                for(sal_uInt32 b(0); bAdd && b < nReducedCount; b++)
                                {
                                    if(aCandidate == aReducedLoops.getB3DPolygon(b))
                                    {
                                        bAdd = false;
                                    }
                                }
                            }

                            if(bAdd)
                            {
                                aReducedLoops.append(aCandidate);
                            }
                        }

                        
                        const sal_uInt32 nReducedCount(aReducedLoops.count());

                        if(nReducedCount > 1)
                        {
                            for(sal_uInt32 b(1); b < nReducedCount; b++)
                            {
                                
                                const basegfx::B3DPolygon aCandA(aReducedLoops.getB3DPolygon(b - 1));
                                const basegfx::B3DPolygon aCandB(aReducedLoops.getB3DPolygon(b));

                                
                                createReducedOutlines(
                                    rViewInformation,
                                    getTransform(),
                                    aCandA,
                                    aCandB,
                                    aNewLineGeometry);
                            }
                        }

                        
                        aNewLineGeometry.append(aReducedLoops);

                        
                        
                        //
                        
                        
                        
                        
                        
                        
                        //
                        
                        
                        
                        
                        
                        const sal_uInt32 nPolyCount(maCorrectedPolyPolygon.count());

                        for(sal_uInt32 c(0); c < nPolyCount; c++)
                        {
                            const basegfx::B2DPolygon aCandidate(maCorrectedPolyPolygon.getB2DPolygon(c));
                            const sal_uInt32 nPointCount(aCandidate.count());

                            if(nPointCount > 2)
                            {
                                sal_uInt32 nIndexA(nPointCount);
                                sal_uInt32 nIndexB(nPointCount);
                                sal_uInt32 nIndexC(nPointCount);

                                for(sal_uInt32 d(0); d < nPointCount; d++)
                                {
                                    const sal_uInt32 nPrevInd((d + nPointCount - 1) % nPointCount);
                                    const sal_uInt32 nNextInd((d + 1) % nPointCount);
                                    const basegfx::B2DPoint aPoint(aCandidate.getB2DPoint(d));
                                    const basegfx::B2DVector aPrev(aCandidate.getB2DPoint(nPrevInd) - aPoint);
                                    const basegfx::B2DVector aNext(aCandidate.getB2DPoint(nNextInd) - aPoint);
                                    const double fAngle(aPrev.angle(aNext));

                                    
                                    
                                    
                                    if(F_PI - fabs(fAngle) > F_PI * 0.1)
                                    {
                                        if(nPointCount == nIndexA)
                                        {
                                            nIndexA = d;
                                        }
                                        else if(nPointCount == nIndexB)
                                        {
                                            nIndexB = d;
                                        }
                                        else if(nPointCount == nIndexC)
                                        {
                                            nIndexC = d;
                                            d = nPointCount;
                                        }
                                    }
                                }

                                const bool bIndexAUsed(nIndexA != nPointCount);
                                const bool bIndexBUsed(nIndexB != nPointCount);
                                bool bIndexCUsed(nIndexC != nPointCount);

                                if(bIndexCUsed)
                                {
                                    
                                    
                                }
                                else if(bIndexAUsed && bIndexBUsed)
                                {
                                    
                                    
                                    if(((nIndexA + 2) % nPointCount) == nIndexB)
                                    {
                                        nIndexC = (nIndexA + 1) % nPointCount;
                                    }
                                    else if(((nIndexA + nPointCount - 2) % nPointCount) == nIndexB)
                                    {
                                        nIndexC = (nIndexA + nPointCount - 1) % nPointCount;
                                    }

                                    bIndexCUsed = (nIndexC != nPointCount);
                                }

                                if(bIndexAUsed)
                                {
                                    const basegfx::B2DPoint aPoint(aCandidate.getB2DPoint(nIndexA));
                                    const basegfx::B3DPoint aStart(aPoint.getX(), aPoint.getY(), 0.0);
                                    const basegfx::B3DPoint aEnd(aPoint.getX(), aPoint.getY(), getDepth());
                                    basegfx::B3DPolygon aToBeAdded;

                                    aToBeAdded.append(aStart);
                                    aToBeAdded.append(aEnd);
                                    aNewLineGeometry.append(aToBeAdded);
                                }

                                if(bIndexBUsed)
                                {
                                    const basegfx::B2DPoint aPoint(aCandidate.getB2DPoint(nIndexB));
                                    const basegfx::B3DPoint aStart(aPoint.getX(), aPoint.getY(), 0.0);
                                    const basegfx::B3DPoint aEnd(aPoint.getX(), aPoint.getY(), getDepth());
                                    basegfx::B3DPolygon aToBeAdded;

                                    aToBeAdded.append(aStart);
                                    aToBeAdded.append(aEnd);
                                    aNewLineGeometry.append(aToBeAdded);
                                }

                                if(bIndexCUsed)
                                {
                                    const basegfx::B2DPoint aPoint(aCandidate.getB2DPoint(nIndexC));
                                    const basegfx::B3DPoint aStart(aPoint.getX(), aPoint.getY(), 0.0);
                                    const basegfx::B3DPoint aEnd(aPoint.getX(), aPoint.getY(), getDepth());
                                    basegfx::B3DPolygon aToBeAdded;

                                    aToBeAdded.append(aStart);
                                    aToBeAdded.append(aEnd);
                                    aNewLineGeometry.append(aToBeAdded);
                                }
                            }
                        }

                        
                        aNewLineGeometry.append(aReducedLoops);

                        if(aNewLineGeometry.count())
                        {
                            const Primitive3DSequence aLines(create3DPolyPolygonLinePrimitives(
                                aNewLineGeometry, getTransform(), getSdrLFSAttribute().getLine()));
                            appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aLines);
                        }
                    }
                    else
                    {
                        
                        const basegfx::B3DPolyPolygon aHorLine(extractHorizontalLinesFromSlice(rSliceVector, false));
                        const basegfx::B3DPolyPolygon aVerLine(extractVerticalLinesFromSlice(rSliceVector));

                        
                        const Primitive3DSequence aHorLines(create3DPolyPolygonLinePrimitives(
                            aHorLine, getTransform(), getSdrLFSAttribute().getLine()));
                        appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aHorLines);

                        
                        const Primitive3DSequence aVerLines(create3DPolyPolygonLinePrimitives(
                            aVerLine, getTransform(), getSdrLFSAttribute().getLine()));
                        appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aVerLines);
                    }
                }

                
                if(!getSdrLFSAttribute().getShadow().isDefault() && aRetval.hasElements())
                {
                    const Primitive3DSequence aShadow(createShadowPrimitive3D(
                        aRetval, getSdrLFSAttribute().getShadow(), getSdr3DObjectAttribute().getShadow3D()));
                    appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aShadow);
                }
            }

            return aRetval;
        }

        void SdrExtrudePrimitive3D::impCreateSlices()
        {
            
            
            
            maCorrectedPolyPolygon = basegfx::tools::adaptiveSubdivideByAngle(getPolyPolygon());
            maCorrectedPolyPolygon.removeDoublePoints();
            maCorrectedPolyPolygon = basegfx::tools::correctOrientations(maCorrectedPolyPolygon);
            maCorrectedPolyPolygon = basegfx::tools::correctOutmostPolygon(maCorrectedPolyPolygon);

            
            createExtrudeSlices(maSlices, maCorrectedPolyPolygon, getBackScale(), getDiagonal(), getDepth(), getCharacterMode(), getCloseFront(), getCloseBack());
        }

        const Slice3DVector& SdrExtrudePrimitive3D::getSlices() const
        {
            
            
            if(getPolyPolygon().count() && !maSlices.size())
            {
                const_cast< SdrExtrudePrimitive3D& >(*this).impCreateSlices();
            }

            return maSlices;
        }

        SdrExtrudePrimitive3D::SdrExtrudePrimitive3D(
            const basegfx::B3DHomMatrix& rTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::SdrLineFillShadowAttribute3D& rSdrLFSAttribute,
            const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute,
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            double fDepth,
            double fDiagonal,
            double fBackScale,
            bool bSmoothNormals,
            bool bSmoothHorizontalNormals,
            bool bSmoothLids,
            bool bCharacterMode,
            bool bCloseFront,
            bool bCloseBack)
        :   SdrPrimitive3D(rTransform, rTextureSize, rSdrLFSAttribute, rSdr3DObjectAttribute),
            maCorrectedPolyPolygon(),
            maSlices(),
            maPolyPolygon(rPolyPolygon),
            mfDepth(fDepth),
            mfDiagonal(fDiagonal),
            mfBackScale(fBackScale),
            mpLastRLGViewInformation(0),
            mbSmoothNormals(bSmoothNormals),
            mbSmoothHorizontalNormals(bSmoothHorizontalNormals),
            mbSmoothLids(bSmoothLids),
            mbCharacterMode(bCharacterMode),
            mbCloseFront(bCloseFront),
            mbCloseBack(bCloseBack)
        {
            
            if(basegfx::fTools::lessOrEqual(getDepth(), 0.0))
            {
                mfDepth = 0.0;
            }

            
            if(basegfx::fTools::lessOrEqual(getDiagonal(), 0.0))
            {
                mfDiagonal = 0.0;
            }
            else if(basegfx::fTools::moreOrEqual(getDiagonal(), 1.0))
            {
                mfDiagonal = 1.0;
            }

            
            if(getPolyPolygon().count() && !getPolyPolygon().getB2DPolygon(0L).isClosed())
            {
                mbCloseFront = mbCloseBack = false;
            }

            
            if(!getCloseFront() && !getCloseBack())
            {
                mfDiagonal = 0.0;
            }
        }

        SdrExtrudePrimitive3D::~SdrExtrudePrimitive3D()
        {
            if(mpLastRLGViewInformation)
            {
                delete mpLastRLGViewInformation;
            }
        }

        bool SdrExtrudePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(SdrPrimitive3D::operator==(rPrimitive))
            {
                const SdrExtrudePrimitive3D& rCompare = static_cast< const SdrExtrudePrimitive3D& >(rPrimitive);

                return (getPolyPolygon() == rCompare.getPolyPolygon()
                    && getDepth() == rCompare.getDepth()
                    && getDiagonal() == rCompare.getDiagonal()
                    && getBackScale() == rCompare.getBackScale()
                    && getSmoothNormals() == rCompare.getSmoothNormals()
                    && getSmoothHorizontalNormals() == rCompare.getSmoothHorizontalNormals()
                    && getSmoothLids() == rCompare.getSmoothLids()
                    && getCharacterMode() == rCompare.getCharacterMode()
                    && getCloseFront() == rCompare.getCloseFront()
                    && getCloseBack() == rCompare.getCloseBack());
            }

            return false;
        }

        basegfx::B3DRange SdrExtrudePrimitive3D::getB3DRange(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            
            
            
            
            
            
            return get3DRangeFromSlices(getSlices());
        }

        Primitive3DSequence SdrExtrudePrimitive3D::get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const
        {
            if(getSdr3DObjectAttribute().getReducedLineGeometry())
            {
                if(!mpLastRLGViewInformation ||
                    (getBuffered3DDecomposition().hasElements()
                        && *mpLastRLGViewInformation != rViewInformation))
                {
                    
                    
                    SdrExtrudePrimitive3D* pThat = const_cast< SdrExtrudePrimitive3D* >(this);
                    pThat->setBuffered3DDecomposition(Primitive3DSequence());
                    delete pThat->mpLastRLGViewInformation;
                    pThat->mpLastRLGViewInformation = new geometry::ViewInformation3D(rViewInformation);
                }
            }

            
            return SdrPrimitive3D::get3DDecomposition(rViewInformation);
        }

        
        ImplPrimitive3DIDBlock(SdrExtrudePrimitive3D, PRIMITIVE3D_ID_SDREXTRUDEPRIMITIVE3D)

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
