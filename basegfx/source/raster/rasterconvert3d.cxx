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

#include <basegfx/raster/rasterconvert3d.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <basegfx/point/b3dpoint.hxx>




namespace basegfx
{
    void RasterConverter3D::addArea(const B3DPolygon& rFill, const B3DHomMatrix* pViewToEye)
    {
        const sal_uInt32 nPointCount(rFill.count());

        for(sal_uInt32 a(0); a < nPointCount; a++)
        {
            addEdge(rFill, a, (a + 1) % nPointCount, pViewToEye);
        }
    }

    void RasterConverter3D::addArea(const B3DPolyPolygon& rFill, const B3DHomMatrix* pViewToEye)
    {
        const sal_uInt32 nPolyCount(rFill.count());

        for(sal_uInt32 a(0); a < nPolyCount; a++)
        {
            addArea(rFill.getB3DPolygon(a), pViewToEye);
        }
    }

    RasterConverter3D::RasterConverter3D()
    :   InterpolatorProvider3D(),
        maLineEntries()
    {}

    RasterConverter3D::~RasterConverter3D()
    {}

    void RasterConverter3D::rasterconvertB3DArea(sal_Int32 nStartLine, sal_Int32 nStopLine)
    {
        if(!maLineEntries.empty())
        {
            OSL_ENSURE(nStopLine >= nStartLine, "nStopLine is bigger than nStartLine (!)");

            
            
            ::std::sort(maLineEntries.begin(), maLineEntries.end());

            
            ::std::vector< RasterConversionLineEntry3D >::iterator aCurrentEntry(maLineEntries.begin());
            ::std::vector< RasterConversionLineEntry3D* > aCurrentLine;
            ::std::vector< RasterConversionLineEntry3D* > aNextLine;
            ::std::vector< RasterConversionLineEntry3D* >::iterator aRasterConversionLineEntry3D;
            sal_uInt32 nPairCount(0);

            
            sal_Int32 nLineNumber(::std::max(aCurrentEntry->getY(), nStartLine));

            while((aCurrentLine.size() || aCurrentEntry != maLineEntries.end()) && (nLineNumber < nStopLine))
            {
                
                while(aCurrentEntry != maLineEntries.end())
                {
                    const sal_Int32 nCurrentLineNumber(aCurrentEntry->getY());

                    if(nCurrentLineNumber > nLineNumber)
                    {
                        
                        break;
                    }
                    else
                    {
                        
                        
                        const sal_uInt32 nStep(nLineNumber - nCurrentLineNumber);

                        if(!nStep || aCurrentEntry->decrementRasterConversionLineEntry3D(nStep))
                        {
                            
                            
                            if(nStep)
                            {
                                aCurrentEntry->incrementRasterConversionLineEntry3D(nStep, *this);
                            }

                            aCurrentLine.push_back(&(*(aCurrentEntry)));
                        }
                    }

                    ++aCurrentEntry;
                }

                
                
                
                
                ::std::sort(aCurrentLine.begin(), aCurrentLine.end(), lineComparator());

                
                aRasterConversionLineEntry3D = aCurrentLine.begin();
                aNextLine.clear();
                nPairCount = 0;

                while(aRasterConversionLineEntry3D != aCurrentLine.end())
                {
                    RasterConversionLineEntry3D& rPrevScanRasterConversionLineEntry3D(**aRasterConversionLineEntry3D++);

                    
                    if(aRasterConversionLineEntry3D != aCurrentLine.end())
                    {
                        
                        processLineSpan(rPrevScanRasterConversionLineEntry3D, **aRasterConversionLineEntry3D, nLineNumber, nPairCount++);
                    }

                    
                    if(rPrevScanRasterConversionLineEntry3D.decrementRasterConversionLineEntry3D(1))
                    {
                        rPrevScanRasterConversionLineEntry3D.incrementRasterConversionLineEntry3D(1, *this);
                        aNextLine.push_back(&rPrevScanRasterConversionLineEntry3D);
                    }
                }

                
                if(aNextLine.size() != aCurrentLine.size())
                {
                    aCurrentLine = aNextLine;
                }

                
                nLineNumber++;
            }
        }
    }

    void RasterConverter3D::addEdge(const B3DPolygon& rFill, sal_uInt32 a, sal_uInt32 b, const B3DHomMatrix* pViewToEye)
    {
        B3DPoint aStart(rFill.getB3DPoint(a));
        B3DPoint aEnd(rFill.getB3DPoint(b));
        sal_Int32 nYStart(fround(aStart.getY()));
        sal_Int32 nYEnd(fround(aEnd.getY()));

        if(nYStart != nYEnd)
        {
            if(nYStart > nYEnd)
            {
                ::std::swap(aStart, aEnd);
                ::std::swap(nYStart, nYEnd);
                ::std::swap(a, b);
            }

            const sal_uInt32 nYDelta(nYEnd - nYStart);
            const double fInvYDelta(1.0 / nYDelta);
            maLineEntries.push_back(RasterConversionLineEntry3D(
                aStart.getX(), (aEnd.getX() - aStart.getX()) * fInvYDelta,
                aStart.getZ(), (aEnd.getZ() - aStart.getZ()) * fInvYDelta,
                nYStart, nYDelta));

            
            RasterConversionLineEntry3D& rEntry = maLineEntries[maLineEntries.size() - 1];

            if(rFill.areBColorsUsed())
            {
                rEntry.setColorIndex(addColorInterpolator(rFill.getBColor(a), rFill.getBColor(b), fInvYDelta));
            }

            if(rFill.areNormalsUsed())
            {
                rEntry.setNormalIndex(addNormalInterpolator(rFill.getNormal(a), rFill.getNormal(b), fInvYDelta));
            }

            if(rFill.areTextureCoordinatesUsed())
            {
                if(pViewToEye)
                {
                    const double fEyeA(((*pViewToEye) * aStart).getZ());
                    const double fEyeB(((*pViewToEye) * aEnd).getZ());

                    rEntry.setInverseTextureIndex(addInverseTextureInterpolator(
                        rFill.getTextureCoordinate(a),
                        rFill.getTextureCoordinate(b),
                        fEyeA, fEyeB, fInvYDelta));
                }
                else
                {
                    rEntry.setTextureIndex(addTextureInterpolator(
                        rFill.getTextureCoordinate(a),
                        rFill.getTextureCoordinate(b),
                        fInvYDelta));
                }
            }
        }
    }

    void RasterConverter3D::rasterconvertB3DEdge(const B3DPolygon& rLine, sal_uInt32 nA, sal_uInt32 nB, sal_Int32 nStartLine, sal_Int32 nStopLine, sal_uInt16 nLineWidth)
    {
        B3DPoint aStart(rLine.getB3DPoint(nA));
        B3DPoint aEnd(rLine.getB3DPoint(nB));
        const double fZBufferLineAdd(0x00ff);
        static bool bForceToPolygon(false);

        if(nLineWidth > 1 || bForceToPolygon)
        {
            
            
            if(!aStart.equal(aEnd))
            {
                reset();
                maLineEntries.clear();

                B2DVector aVector(aEnd.getX() - aStart.getX(), aEnd.getY() - aStart.getY());
                aVector.normalize();
                const B2DVector aPerpend(getPerpendicular(aVector) * ((static_cast<double>(nLineWidth) + 0.5) * 0.5));
                const double fZStartWithAdd(aStart.getZ() + fZBufferLineAdd);
                const double fZEndWithAdd(aEnd.getZ() + fZBufferLineAdd);

                B3DPolygon aPolygon;
                aPolygon.append(B3DPoint(aStart.getX() + aPerpend.getX(), aStart.getY() + aPerpend.getY(), fZStartWithAdd));
                aPolygon.append(B3DPoint(aEnd.getX() + aPerpend.getX(), aEnd.getY() + aPerpend.getY(), fZEndWithAdd));
                aPolygon.append(B3DPoint(aEnd.getX() - aPerpend.getX(), aEnd.getY() - aPerpend.getY(), fZEndWithAdd));
                aPolygon.append(B3DPoint(aStart.getX() - aPerpend.getX(), aStart.getY() - aPerpend.getY(), fZStartWithAdd));
                aPolygon.setClosed(true);

                addArea(aPolygon, 0);
            }
        }
        else
        {
            
            
            sal_Int32 nYStart(fround(aStart.getY()));
            sal_Int32 nYEnd(fround(aEnd.getY()));

            if(nYStart == nYEnd)
            {
                
                const sal_Int32 nXStart(static_cast<sal_Int32>(aStart.getX()));
                const sal_Int32 nXEnd(static_cast<sal_Int32>(aEnd.getX()));

                if(nXStart != nXEnd)
                {
                    reset();
                    maLineEntries.clear();

                    
                    
                    maLineEntries.push_back(RasterConversionLineEntry3D(
                        aStart.getX(), 0.0,
                        aStart.getZ() + fZBufferLineAdd, 0.0,
                        nYStart, 1));
                    maLineEntries.push_back(RasterConversionLineEntry3D(
                        aEnd.getX(), 0.0,
                        aEnd.getZ() + fZBufferLineAdd, 0.0,
                        nYStart, 1));
                }
            }
            else
            {
                reset();
                maLineEntries.clear();

                if(nYStart > nYEnd)
                {
                    ::std::swap(aStart, aEnd);
                    ::std::swap(nYStart, nYEnd);
                }

                const sal_uInt32 nYDelta(static_cast<sal_uInt32>(nYEnd - nYStart));
                const double fInvYDelta(1.0 / nYDelta);

                
                
                maLineEntries.push_back(RasterConversionLineEntry3D(
                    aStart.getX(), (aEnd.getX() - aStart.getX()) * fInvYDelta,
                    aStart.getZ() + fZBufferLineAdd, (aEnd.getZ() - aStart.getZ()) * fInvYDelta,
                    nYStart, nYDelta));

                RasterConversionLineEntry3D& rEntry = maLineEntries[maLineEntries.size() - 1];

                
                
                
                
                const double fDistanceX(fabs(rEntry.getX().getInc()) >= 1.0 ? rEntry.getX().getInc() : 1.0);

                maLineEntries.push_back(RasterConversionLineEntry3D(
                    rEntry.getX().getVal() + fDistanceX, rEntry.getX().getInc(),
                    rEntry.getZ().getVal() + rEntry.getZ().getInc(), rEntry.getZ().getInc(),
                    nYStart, nYDelta));
            }
        }

        if(!maLineEntries.empty())
        {
            rasterconvertB3DArea(nStartLine, nStopLine);
        }
    }

    void RasterConverter3D::rasterconvertB3DPolyPolygon(const B3DPolyPolygon& rFill, const B3DHomMatrix* pViewToEye, sal_Int32 nStartLine, sal_Int32 nStopLine)
    {
        reset();
        maLineEntries.clear();
        addArea(rFill, pViewToEye);
        rasterconvertB3DArea(nStartLine, nStopLine);
    }

    void RasterConverter3D::rasterconvertB3DPolygon(const B3DPolygon& rLine, sal_Int32 nStartLine, sal_Int32 nStopLine, sal_uInt16 nLineWidth)
    {
        const sal_uInt32 nPointCount(rLine.count());

        if(nPointCount)
        {
            const sal_uInt32 nEdgeCount(rLine.isClosed() ? nPointCount : nPointCount - 1);

            for(sal_uInt32 a(0); a < nEdgeCount; a++)
            {
                rasterconvertB3DEdge(rLine, a, (a + 1) % nPointCount, nStartLine, nStopLine, nLineWidth);
            }
        }
    }
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
