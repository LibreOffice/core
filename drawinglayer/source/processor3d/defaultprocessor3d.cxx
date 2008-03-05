/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaultprocessor3d.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:45 $
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

//////////////////////////////////////////////////////////////////////////////
// includes for namespace basegfx

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX
#include <drawinglayer/processor3d/defaultprocessor3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

#ifndef _BGFX_RASTER_BZPIXELRASTER_HXX
#include <basegfx/raster/bzpixelraster.hxx>
#endif

#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX
#include <drawinglayer/attribute/materialattribute3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// includes for namespace drawinglayer

#ifndef INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE_HXX
#include <drawinglayer/texture/texture.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE3D_HXX
#include <drawinglayer/attribute/sdrattribute3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX
#include <drawinglayer/attribute/fillattribute.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_HATCHTEXTUREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/hatchtextureprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_MODIFIEDCOLORPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/modifiedcolorprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TRANSFORMPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRLABELPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/sdrlabelprimitive3d.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#include <drawinglayer/geometry/viewinformation2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class BDInterpolator
    {
    protected:
        double                                      mfVal;
        double                                      mfInc;

    public:
        BDInterpolator() {}
        BDInterpolator(double fValA, double fValB, sal_uInt32 nCount) : mfVal(fValA), mfInc((fValB - fValA) / (double)nCount) {}

        double getVal() const { return mfVal; }
        double getInc() const { return mfInc; }
        void increment() { mfVal += mfInc; }
        void increment(double fStep) { mfVal += fStep * mfInc; }
    };

    class BColorInterpolator
    {
    protected:
        BColor                                      maVal;
        BColor                                      maInc;

    public:
        BColorInterpolator() {}
        BColorInterpolator(const BColor& rValA, const BColor& rValB, sal_uInt32 nCount) : maVal(rValA), maInc((rValB - rValA) / (double)nCount) {}

        const BColor& getVal() const { return maVal; }
        const BColor& getInc() const { return maInc; }
        void increment() { maVal += maInc; }
        void increment(double fStep) { maVal += fStep * maInc; }
    };

    class B3DVectorInterpolator
    {
    protected:
        B3DVector                                   maVal;
        B3DVector                                   maInc;

    public:
        B3DVectorInterpolator() {}
        B3DVectorInterpolator(const B3DVector& rValA, const B3DVector& rValB, sal_uInt32 nCount) : maVal(rValA), maInc((rValB - rValA) / (double)nCount) {}

        const B3DVector& getVal() const { return maVal; }
        const B3DVector& getInc() const { return maInc; }
        void increment() { maVal += maInc; }
        void increment(double fStep) { maVal += fStep * maInc; }
    };

    class B2DPointInterpolator
    {
    protected:
        B2DPoint                                    maVal;
        B2DPoint                                    maInc;
        BDInterpolator                              maZInv;

    public:
        B2DPointInterpolator() {}
        B2DPointInterpolator(const B2DPoint& rValA, const B2DPoint& rValB, double fInvZEyeA, double fInvZEyeB, sal_uInt32 nCount)
        :   maVal(rValA),
            maInc((rValB - rValA) / (double)nCount),
            maZInv(fInvZEyeA, fInvZEyeB, nCount)
        {}

        const B2DPoint& getVal() const { return maVal; }
        const B2DPoint& getInc() const { return maInc; }
        double getZVal() const { return maZInv.getVal(); }
        double getZInc() const { return maZInv.getInc(); }
        void increment() { maVal += maInc; maZInv.increment(); }
        void increment(double fStep) { maVal += fStep * maInc; maZInv.increment(fStep); }
    };
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

#define SCANLINE_EMPTY_INDEX (0xffffffff)

namespace basegfx
{
    class B3DScanlineEntry
    {
    protected:
        sal_Int32                                   mnLine;
        sal_uInt32                                  mnLineCount;
        BDInterpolator                              maX;
        BDInterpolator                              maZ;

        // values for normal and texture coordinate interpolation are optional,
        // held simply by an index. Empty is marked by SCANLINE_EMPTY_INDEX to which it needs
        // to be initialized
        sal_uInt32                                  mnBColorIndex;
        sal_uInt32                                  mnNormalIndex;
        sal_uInt32                                  mnTextureCoordinateIndex;

    public:
        B3DScanlineEntry() {}

        // data write access
        void setLine(sal_Int32 nLine) { mnLine = nLine; }
        void setLineCount(sal_uInt32 nLineCount) { mnLineCount = nLineCount; }
        void setXInterpolator(const BDInterpolator& rInt) { maX = rInt; }
        void setZInterpolator(const BDInterpolator& rInt) { maZ = rInt; }
        void setBColorIndex(sal_uInt32 nIndex) { mnBColorIndex = nIndex; }
        void setNormalIndex(sal_uInt32 nIndex) { mnNormalIndex = nIndex; }
        void setTextureCoordinateIndex(sal_uInt32 nIndex) { mnTextureCoordinateIndex = nIndex; }

        // data read access
        sal_Int32 getLine() const { return mnLine; }
        sal_uInt32 getLineCount() const { return mnLineCount; }
        const BDInterpolator& getXInterpolator() const { return maX; }
        const BDInterpolator& getZInterpolator() const { return maZ; }
        sal_uInt32 getBColorIndex() const {return mnBColorIndex; }
        sal_uInt32 getNormalIndex() const {return mnNormalIndex; }
        sal_uInt32 getTextureCoordinateIndex() const {return mnTextureCoordinateIndex; }

        bool simpleLineSortComparator(const B3DScanlineEntry& rComp) const
        {
            return (maX.getVal() < rComp.maX.getVal());
        }

        bool operator<(const B3DScanlineEntry& rComp) const
        {
            if(mnLine == rComp.mnLine)
            {
                return simpleLineSortComparator(rComp);
            }

            return (mnLine < rComp.mnLine);
        }

        bool isValid() const
        {
            return (0L != mnLineCount);
        }

        bool decrement(sal_uInt32 nCount = 1L)
        {
            if(mnLineCount > nCount)
            {
                mnLineCount -= nCount;
                return true;
            }
            else
            {
                mnLineCount = 0L;
                return false;
            }
        }

        void increment()
        {
            maX.increment();
            maZ.increment();
        }

        void increment(double fStep)
        {
            maX.increment(fStep);
            maZ.increment(fStep);
        }
    };
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class B3DPolyPolygonRasterConverter
    {
    protected:
        ::std::vector< B3DScanlineEntry >               maGlobalEntries;
        ::std::vector< BColorInterpolator >             maGlobalBColorInterpolators;
        ::std::vector< B3DVectorInterpolator >          maGlobalNormalInterpolators;
        ::std::vector< B2DPointInterpolator >           maGlobalTextureCoordinateInterpolators;

        // bitfield
        unsigned                                        mbAreaMode : 1;

        struct scanlineEntryComparator
        {
            bool operator()( const B3DScanlineEntry* pA, const B3DScanlineEntry* pB)
            {
                // here, Y is the same anyways (sorting a scanline), so simple compare is enough
                OSL_ENSURE(pA && pB, "scanlineEntryComparator: empty pointer (!)");
                return pA->simpleLineSortComparator(*pB);
            }
        };

        // add BColor interpolator, return index
        sal_uInt32 addBColor(const BColor& rA, const BColor& rB, sal_uInt32 nDelta)
        {
            maGlobalBColorInterpolators.push_back(BColorInterpolator(rA, rB, nDelta));
            return (maGlobalBColorInterpolators.size() - 1L);
        }

        // add normal interpolator, return index
        sal_uInt32 addNormal(const B3DVector& rA, const B3DVector& rB, sal_uInt32 nDelta)
        {
            maGlobalNormalInterpolators.push_back(B3DVectorInterpolator(rA, rB, nDelta));
            return (maGlobalNormalInterpolators.size() - 1L);
        }

        // add texture interpolator, return index
        sal_uInt32 addTextureCoordinate(const B2DPoint& rA, const B2DPoint& rB, double fZEyeA, double fZEyeB, sal_uInt32 nDelta)
        {
            const double fInvZEyeA(fTools::equalZero(fZEyeA) ? fZEyeA : 1.0 / fZEyeA);
            const double fInvZEyeB(fTools::equalZero(fZEyeB) ? fZEyeB : 1.0 / fZEyeB);
            maGlobalTextureCoordinateInterpolators.push_back(B2DPointInterpolator(rA * fInvZEyeA, rB * fInvZEyeB, fInvZEyeA, fInvZEyeB, nDelta));
            return (maGlobalTextureCoordinateInterpolators.size() - 1L);
        }

        void increment(B3DScanlineEntry& rScanEntry)
        {
            rScanEntry.increment();

            const sal_uInt32 nBColor(rScanEntry.getBColorIndex());

            if(SCANLINE_EMPTY_INDEX != nBColor)
            {
                maGlobalBColorInterpolators[nBColor].increment();
            }

            const sal_uInt32 nNormal(rScanEntry.getNormalIndex());

            if(SCANLINE_EMPTY_INDEX != nNormal)
            {
                maGlobalNormalInterpolators[nNormal].increment();
            }

            const sal_uInt32 nTextureCoordinate(rScanEntry.getTextureCoordinateIndex());

            if(SCANLINE_EMPTY_INDEX != nTextureCoordinate)
            {
                maGlobalTextureCoordinateInterpolators[nTextureCoordinate].increment();
            }
        }

        void increment(B3DScanlineEntry& rScanEntry, double fStep)
        {
            rScanEntry.increment(fStep);

            const sal_uInt32 nBColor(rScanEntry.getBColorIndex());

            if(SCANLINE_EMPTY_INDEX != nBColor)
            {
                maGlobalBColorInterpolators[nBColor].increment(fStep);
            }

            const sal_uInt32 nNormal(rScanEntry.getNormalIndex());

            if(SCANLINE_EMPTY_INDEX != nNormal)
            {
                maGlobalNormalInterpolators[nNormal].increment(fStep);
            }

            const sal_uInt32 nTextureCoordinate(rScanEntry.getTextureCoordinateIndex());

            if(SCANLINE_EMPTY_INDEX != nTextureCoordinate)
            {
                maGlobalTextureCoordinateInterpolators[nTextureCoordinate].increment(fStep);
            }
        }

        void addEdge(const B3DPolygon& rPolygon, const B3DHomMatrix& rInvEyeToView, sal_uInt32 nIndA, sal_uInt32 nIndB)
        {
            B3DPoint aPntA(rPolygon.getB3DPoint(nIndA));
            B3DPoint aPntB(rPolygon.getB3DPoint(nIndB));
            sal_Int32 nLineA((sal_Int32)(aPntA.getY()));
            sal_Int32 nLineB((sal_Int32)(aPntB.getY()));

            if(nLineA == nLineB)
            {
                if(!mbAreaMode)
                {
                    B3DScanlineEntry aNewEntry;

                    aNewEntry.setLine(nLineA);
                    aNewEntry.setLineCount(0L);
                    aNewEntry.setXInterpolator(BDInterpolator(aPntA.getX(), aPntB.getX(), 1L));
                    aNewEntry.setZInterpolator(BDInterpolator(aPntA.getZ(), aPntB.getZ(), 1L));
                    aNewEntry.setBColorIndex(SCANLINE_EMPTY_INDEX);
                    aNewEntry.setNormalIndex(SCANLINE_EMPTY_INDEX);
                    aNewEntry.setTextureCoordinateIndex(SCANLINE_EMPTY_INDEX);

                    maGlobalEntries.push_back(aNewEntry);
                }
            }
            else
            {
                if(nLineB < nLineA)
                {
                    ::std::swap(aPntA, aPntB);
                    ::std::swap(nLineA, nLineB);
                    ::std::swap(nIndA, nIndB);
                }

                const sal_uInt32 nLineDelta(nLineB - nLineA);
                B3DScanlineEntry aNewEntry;

                aNewEntry.setLine(nLineA);
                aNewEntry.setLineCount(nLineDelta);
                aNewEntry.setXInterpolator(BDInterpolator(aPntA.getX(), aPntB.getX(), nLineDelta));
                aNewEntry.setZInterpolator(BDInterpolator(aPntA.getZ(), aPntB.getZ(), nLineDelta));

                const bool bColUsed(mbAreaMode && rPolygon.areBColorsUsed());
                aNewEntry.setBColorIndex(bColUsed ? addBColor(rPolygon.getBColor(nIndA), rPolygon.getBColor(nIndB), nLineDelta) : SCANLINE_EMPTY_INDEX);

                const bool bNrmUsed(mbAreaMode && rPolygon.areNormalsUsed());
                aNewEntry.setNormalIndex(bNrmUsed ? addNormal(rPolygon.getNormal(nIndA), rPolygon.getNormal(nIndB), nLineDelta) : SCANLINE_EMPTY_INDEX);

                const bool bTexUsed(mbAreaMode && rPolygon.areTextureCoordinatesUsed());
                if(bTexUsed)
                {
                    const double fEyeA((rInvEyeToView * aPntA).getZ());
                    const double fEyeB((rInvEyeToView * aPntB).getZ());
                    aNewEntry.setTextureCoordinateIndex(addTextureCoordinate(
                        rPolygon.getTextureCoordinate(nIndA),
                        rPolygon.getTextureCoordinate(nIndB),
                        fEyeA, fEyeB, nLineDelta));
                }
                else
                {
                    aNewEntry.setTextureCoordinateIndex(SCANLINE_EMPTY_INDEX);
                }

                maGlobalEntries.push_back(aNewEntry);
            }
        }

        // virtual rasterconverter
        virtual void processSpan(const B3DScanlineEntry& rA, const B3DScanlineEntry& rB, sal_Int32 nLine, sal_uInt32 nSpanCount) = 0;
        virtual void processLine(const B3DScanlineEntry& rEntry, sal_Int32 nLine) = 0;

    public:
        B3DPolyPolygonRasterConverter(bool bArea)
        :   mbAreaMode(bArea)
        {
        }

        virtual ~B3DPolyPolygonRasterConverter()
        {
        }

        void addPolygon(const B3DPolygon& rPolygon, const B3DHomMatrix& rInvEyeToView)
        {
            const sal_uInt32 nPointCount(rPolygon.count());

            if(nPointCount)
            {
                const sal_uInt32 nLoopCount((mbAreaMode || rPolygon.isClosed()) ? nPointCount : nPointCount - 1L);
                const bool bEnoughEdges(mbAreaMode ? (nLoopCount > 2L) : (nLoopCount > 0L));

                if(bEnoughEdges)
                {
                    for(sal_uInt32 a(0L); a < nLoopCount; a++)
                    {
                        addEdge(rPolygon, rInvEyeToView, a, (a + 1L) % nPointCount);
                    }
                }
            }
        }

        void rasterconvert(sal_Int32 nStartLine, sal_Int32 nStopLine)
        {
            OSL_ENSURE(nStartLine <= nStopLine, "rasterconvert: wrong start/stop line (!)");

            if(maGlobalEntries.size())
            {
                // sort global entries by YStart, xPos once
                ::std::sort(maGlobalEntries.begin(), maGlobalEntries.end());

                // local parameters
                ::std::vector< B3DScanlineEntry >::iterator aCurrentGlobalEntry(maGlobalEntries.begin());
                ::std::vector< B3DScanlineEntry* > aCurrentScanLine;
                ::std::vector< B3DScanlineEntry* > aNextScanLine;
                ::std::vector< B3DScanlineEntry* >::iterator aScanLineEntry;
                sal_Int32 nLineNumber(nStartLine);
                sal_uInt32 nPairCount;

                while((nLineNumber < nStopLine) && (aCurrentScanLine.size() || aCurrentGlobalEntry != maGlobalEntries.end()))
                {
                    // add all global entries which start at current Y to current scanline
                    while(aCurrentGlobalEntry != maGlobalEntries.end() && aCurrentGlobalEntry->getLine() <= nLineNumber)
                    {
                        if(aCurrentGlobalEntry->getLine() < nLineNumber)
                        {
                            // adapt to current line in nLineNumber by decrementing count incrementing accordigly
                            const sal_uInt32 nLineDiff(nLineNumber - aCurrentGlobalEntry->getLine());

                            if(aCurrentGlobalEntry->decrement(nLineDiff))
                            {
                                increment(*aCurrentGlobalEntry, (double)nLineDiff);
                                aCurrentScanLine.push_back(&(*(aCurrentGlobalEntry)));
                            }
                        }
                        else
                        {
                            aCurrentScanLine.push_back(&(*(aCurrentGlobalEntry)));
                        }

                        aCurrentGlobalEntry++;
                    }

                    if(mbAreaMode)
                    {
                        // sort current scanline using simple comparator (only X is compared)
                        ::std::sort(aCurrentScanLine.begin(), aCurrentScanLine.end(), scanlineEntryComparator());
                    }

                    // process current scanline
                    aScanLineEntry = aCurrentScanLine.begin();
                    aNextScanLine.clear();
                    nPairCount = 0L;

                    while(aScanLineEntry != aCurrentScanLine.end())
                    {
                        B3DScanlineEntry& rPrevScanLineEntry(**aScanLineEntry++);

                        if(mbAreaMode)
                        {
                            // area mode, look for 2nd span
                            if(aScanLineEntry != aCurrentScanLine.end())
                            {
                                // work on span from rPrevScanLineEntry to aScanLineEntry, nLineNumber is valid
                                processSpan(rPrevScanLineEntry, **aScanLineEntry, nLineNumber, nPairCount++);
                            }
                        }
                        else
                        {
                            // work on line position rPrevScanLineEntry
                            processLine(rPrevScanLineEntry, nLineNumber);
                        }

                        // do decrement counter and (evtl) increment to next line
                        if(rPrevScanLineEntry.decrement())
                        {
                            increment(rPrevScanLineEntry);
                            aNextScanLine.push_back(&rPrevScanLineEntry);
                        }
                    }

                    // copy back next scanline if count has changed
                    if(aNextScanLine.size() != aCurrentScanLine.size())
                    {
                        aCurrentScanLine = aNextScanLine;
                    }

                    // increment nLineNumber
                    nLineNumber++;
                }
            }
        }
    };
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    BitmapEx getBitmapEx(const BPixelRaster& rRaster)
    {
        BitmapEx aRetval;
        const sal_uInt32 nWidth(rRaster.getWidth());
        const sal_uInt32 nHeight(rRaster.getHeight());

        if(nWidth && nHeight)
        {
            sal_uInt8 nInitAlpha(255);
            Bitmap aContent(Size(nWidth, nHeight), 24);
            AlphaMask aAlpha(Size(nWidth, nHeight), &nInitAlpha);
            BitmapWriteAccess* pContent = aContent.AcquireWriteAccess();
            BitmapWriteAccess* pAlpha = aAlpha.AcquireWriteAccess();

            if(pContent && pAlpha)
            {
                sal_uInt32 nIndex(0L);

                for(sal_uInt32 y(0L); y < nHeight; y++)
                {
                    for(sal_uInt32 x(0L); x < nWidth; x++)
                    {
                        const BPixel& rPixel(rRaster.getBPixel(nIndex++));

                        if(rPixel.getOpacity())
                        {
                            pContent->SetPixel(y, x, BitmapColor(rPixel.getRed(), rPixel.getGreen(), rPixel.getBlue()));
                            pAlpha->SetPixel(y, x, BitmapColor(255 - rPixel.getOpacity()));
                        }
                    }
                }

                delete pContent;
                delete pAlpha;
            }

            aRetval = BitmapEx(aContent, aAlpha);
        }

        return aRetval;
    }
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace
    {
        class BZPolyRaCon : public basegfx::B3DPolyPolygonRasterConverter
        {
        protected:
            basegfx::BZPixelRaster&                     mrBuffer;
            const attribute::MaterialAttribute3D&       mrMaterial;
            const processor3d::DefaultProcessor3D&      mrProcessor;

            // virtual rasterconverter
            virtual void processSpan(const basegfx::B3DScanlineEntry& rA, const basegfx::B3DScanlineEntry& rB, sal_Int32 nLine, sal_uInt32 nSpanCount);
            virtual void processLine(const basegfx::B3DScanlineEntry& rEntry, sal_Int32 nLine);

        public:
            BZPolyRaCon(
                bool bArea,
                basegfx::BZPixelRaster& rBuffer,
                const attribute::MaterialAttribute3D& rMaterial,
                const processor3d::DefaultProcessor3D& rProcessor)
            :   B3DPolyPolygonRasterConverter(bArea),
                mrBuffer(rBuffer),
                mrMaterial(rMaterial),
                mrProcessor(rProcessor)
            {}
        };

        void BZPolyRaCon::processSpan(const basegfx::B3DScanlineEntry& rA, const basegfx::B3DScanlineEntry& rB, sal_Int32 nLine, sal_uInt32 nSpanCount)
        {
            if(!(nSpanCount & 0x0001))
            {
                if(nLine >= 0L && nLine < (sal_Int32)mrBuffer.getHeight())
                {
                    sal_Int32 nXA((sal_Int32)(rA.getXInterpolator().getVal()));
                    sal_Int32 nXB((sal_Int32)(rB.getXInterpolator().getVal()));
                    OSL_ENSURE(nXB >= nXA,"processSpan: positive run expected (!)");

                    if(nXB > nXA)
                    {
                        // initialize Z interpolator
                        const sal_uInt32 nSpanLength(nXB - nXA);
                        basegfx::BDInterpolator aZ(rA.getZInterpolator().getVal(), rB.getZInterpolator().getVal(), nSpanLength);

                        // prepare some references to used variables
                        const basegfx::BColor& rColor(mrMaterial.getColor());
                        const basegfx::BColor& rSpecular(mrMaterial.getSpecular());
                        const basegfx::BColor& rEmission(mrMaterial.getEmission());
                        const sal_uInt16 nSpecularIntensity(mrMaterial.getSpecularIntensity());

                        // get bools and init other interpolators on demand accordingly
                        const bool bUseTex((mrProcessor.getGeoTexSvx() || mrProcessor.getTransparenceGeoTexSvx()) && rA.getTextureCoordinateIndex() != SCANLINE_EMPTY_INDEX && rB.getTextureCoordinateIndex() != SCANLINE_EMPTY_INDEX);
                        const bool bUseColorTex(bUseTex && mrProcessor.getGeoTexSvx());
                        const bool bNeedOthers(!bUseColorTex || (bUseColorTex && mrProcessor.getModulate()));
                        const bool bUseNrm(bNeedOthers && rA.getNormalIndex() != SCANLINE_EMPTY_INDEX && rB.getNormalIndex() != SCANLINE_EMPTY_INDEX);
                        const bool bUseCol(!bUseNrm && bNeedOthers && rA.getBColorIndex() != SCANLINE_EMPTY_INDEX && rB.getBColorIndex() != SCANLINE_EMPTY_INDEX);
                        const bool bModifyColor(mrProcessor.getBColorModifierStack().count());
                        basegfx::B2DPointInterpolator aTex;
                        basegfx::B3DVectorInterpolator aNrm;
                        basegfx::BColorInterpolator aCol;

                        if(bUseTex)
                        {
                            const basegfx::B2DPointInterpolator& rLA(maGlobalTextureCoordinateInterpolators[rA.getTextureCoordinateIndex()]);
                            const basegfx::B2DPointInterpolator& rLB(maGlobalTextureCoordinateInterpolators[rB.getTextureCoordinateIndex()]);
                            aTex = basegfx::B2DPointInterpolator(rLA.getVal(), rLB.getVal(), rLA.getZVal(), rLB.getZVal(), nSpanLength);
                        }

                        if(bUseNrm)
                        {
                            aNrm = basegfx::B3DVectorInterpolator(
                                maGlobalNormalInterpolators[rA.getNormalIndex()].getVal(),
                                maGlobalNormalInterpolators[rB.getNormalIndex()].getVal(),
                                nSpanLength);
                        }

                        if(bUseCol)
                        {
                            aCol = basegfx::BColorInterpolator(
                                maGlobalBColorInterpolators[rA.getBColorIndex()].getVal(),
                                maGlobalBColorInterpolators[rB.getBColorIndex()].getVal(),
                                nSpanLength);
                        }

                        if(nXA < 0L)
                        {
                            const double fIncrement(-nXA);
                            nXA = 0L;
                            aZ.increment(fIncrement);

                            if(bUseTex)
                            {
                                aTex.increment(fIncrement);
                            }

                            if(bUseNrm)
                            {
                                aNrm.increment(fIncrement);
                            }

                            if(bUseCol)
                            {
                                aCol.increment(fIncrement);
                            }
                        }

                        if(nXB > (sal_Int32)mrBuffer.getWidth())
                        {
                            nXB = mrBuffer.getWidth();
                        }

                        if(nXA < nXB)
                        {
                            sal_uInt32 nScanlineIndex(mrBuffer.getIndexFromXY((sal_uInt32)nXA, (sal_uInt32)nLine));

                            while(nXA < nXB)
                            {
                                // get old and new Z to see if we need to do somethng at all
                                sal_uInt16& rOldZ(mrBuffer.getZ(nScanlineIndex));
                                const sal_uInt16 nNewZ((sal_uInt16)(aZ.getVal()));

                                if(nNewZ > rOldZ)
                                {
                                    // prepare color
                                    basegfx::BColor aNewColor(rColor);
                                    double fOpacity(1.0);
                                    bool bOpacity(true);

                                    if(bUseTex)
                                    {
                                        // get texture coor
                                        const basegfx::B2DPoint aTexCoor(aTex.getVal() / aTex.getZVal());

                                        if(mrProcessor.getGeoTexSvx())
                                        {
                                            // calc color in spot
                                            mrProcessor.getGeoTexSvx()->modifyBColor(aTexCoor, aNewColor, fOpacity);
                                            bOpacity = basegfx::fTools::more(fOpacity, 0.0);
                                        }

                                        if(bOpacity && mrProcessor.getTransparenceGeoTexSvx())
                                        {
                                            // calc opacity
                                            mrProcessor.getTransparenceGeoTexSvx()->modifyOpacity(aTexCoor, fOpacity);
                                            bOpacity = basegfx::fTools::more(fOpacity, 0.0);
                                        }
                                    }

                                    if(bOpacity)
                                    {
                                        if(mrProcessor.getGeoTexSvx())
                                        {
                                            if(bUseNrm)
                                            {
                                                // blend texture with phong
                                                aNewColor = mrProcessor.getSdrLightingAttribute().solveColorModel(aNrm.getVal(), aNewColor, rSpecular, rEmission, nSpecularIntensity);
                                            }
                                            else if(bUseCol)
                                            {
                                                // blend texture with gouraud
                                                aNewColor *= aCol.getVal();
                                            }
                                            else if(mrProcessor.getModulate())
                                            {
                                                // blend texture with single material color
                                                aNewColor *= rColor;
                                            }
                                        }
                                        else
                                        {
                                            if(bUseNrm)
                                            {
                                                // modify color with phong
                                                aNewColor = mrProcessor.getSdrLightingAttribute().solveColorModel(aNrm.getVal(), rColor, rSpecular, rEmission, nSpecularIntensity);
                                            }
                                            else if(bUseCol)
                                            {
                                                // modify color with gouraud
                                                aNewColor = aCol.getVal();
                                            }
                                        }

                                        if(bModifyColor)
                                        {
                                            aNewColor = mrProcessor.getBColorModifierStack().getModifiedColor(aNewColor);
                                        }

                                        if(basegfx::fTools::moreOrEqual(fOpacity, 1.0))
                                        {
                                            // full opacity, set z and color
                                            rOldZ = nNewZ;
                                            mrBuffer.getBPixel(nScanlineIndex) = basegfx::BPixel(aNewColor, 0xff);
                                        }
                                        else
                                        {
                                            basegfx::BPixel& rDest = mrBuffer.getBPixel(nScanlineIndex);

                                            if(rDest.getOpacity())
                                            {
                                                // mix color and existing color
                                                const double fOld(1.0 - fOpacity);
                                                fOpacity *= 255.0;
                                                rDest.setRed((sal_uInt8)(((double)rDest.getRed() * fOld) + (aNewColor.getRed() * fOpacity)));
                                                rDest.setGreen((sal_uInt8)(((double)rDest.getGreen() * fOld) + (aNewColor.getGreen() * fOpacity)));
                                                rDest.setBlue((sal_uInt8)(((double)rDest.getBlue() * fOld) + (aNewColor.getBlue() * fOpacity)));

                                                if((sal_uInt8)255 != rDest.getOpacity())
                                                {
                                                    // mix opacities by adding
                                                    double fNewOpacity(rDest.getOpacity() + fOpacity);

                                                    if(fNewOpacity > 255.0)
                                                    {
                                                        // set full opacity
                                                        rDest.setOpacity(0xff);
                                                    }
                                                    else
                                                    {
                                                        // set new opacity which is still transparent, so set no z
                                                        rDest.setOpacity((sal_uInt8)(fNewOpacity));
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                // set color and opacity
                                                rDest = basegfx::BPixel(aNewColor, (sal_uInt8)(fOpacity * 255.0));
                                            }
                                        }
                                    }
                                }

                                // increments
                                {
                                    nScanlineIndex++;
                                    nXA++;
                                    aZ.increment();

                                    if(bUseTex)
                                    {
                                        aTex.increment();
                                    }

                                    if(bUseNrm)
                                    {
                                        aNrm.increment();
                                    }

                                    if(bUseCol)
                                    {
                                        aCol.increment();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        void BZPolyRaCon::processLine(const basegfx::B3DScanlineEntry& rEntry, sal_Int32 nLine)
        {
            if(nLine >= 0L && nLine < (sal_Int32)mrBuffer.getHeight())
            {
                sal_Int32 nXA((sal_uInt32)(rEntry.getXInterpolator().getVal()));
                sal_Int32 nXB((sal_uInt32)(rEntry.getXInterpolator().getVal() + rEntry.getXInterpolator().getInc()));

                if(nXA == nXB)
                {
                    // only one position, get values and set direct
                    if(nXA >= 0L && nXA < (sal_Int32)mrBuffer.getWidth())
                    {
                        const sal_uInt32 nScanlineIndex(mrBuffer.getIndexFromXY((sal_uInt32)nXA, (sal_uInt32)nLine));
                        sal_uInt16& rOldZ(mrBuffer.getZ(nScanlineIndex));
                        const sal_uInt16 nNewZ((sal_uInt16)(rEntry.getZInterpolator().getVal()) + 0x00ff);

                        if(nNewZ > rOldZ)
                        {
                            rOldZ = nNewZ;
                            mrBuffer.getBPixel(nScanlineIndex) = basegfx::BPixel(mrMaterial.getColor(), 0xff);
                        }
                    }
                }
                else
                {
                    double fZStart(rEntry.getZInterpolator().getVal());
                    double fZStop(fZStart + rEntry.getZInterpolator().getInc());

                    if(nXB < nXA)
                    {
                        ::std::swap(nXB, nXA);
                        ::std::swap(fZStart, fZStop);
                    }

                    const basegfx::BPixel aPixel(mrMaterial.getColor(), 0xff);
                    const sal_uInt32 nSpanLength(nXB - nXA);
                    basegfx::BDInterpolator aZ(fZStart, fZStop, nSpanLength);

                    if(nXA < 0L)
                    {
                        const double fIncrement(-nXA);
                        nXA = 0L;
                        aZ.increment(fIncrement);
                    }

                    if(nXB > (sal_Int32)mrBuffer.getWidth())
                    {
                        nXB = mrBuffer.getWidth();
                    }

                    if(nXA < nXB)
                    {
                        sal_uInt32 nScanlineIndex(mrBuffer.getIndexFromXY((sal_uInt32)nXA, (sal_uInt32)nLine));

                        while(nXA < nXB)
                        {
                            sal_uInt16& rOldZ(mrBuffer.getZ(nScanlineIndex));
                            const sal_uInt16 nNewZ((sal_uInt16)(aZ.getVal()) + 0x00ff);

                            if(nNewZ > rOldZ)
                            {
                                rOldZ = nNewZ;
                                mrBuffer.getBPixel(nScanlineIndex) = aPixel;
                            }

                            nScanlineIndex++;
                            nXA++;
                            aZ.increment();
                        }
                    }
                }
            }
        }

        const ::rtl::OUString& getNamePropertyTime()
        {
            static ::rtl::OUString s_sNamePropertyTime(RTL_CONSTASCII_USTRINGPARAM("Time"));
            return s_sNamePropertyTime;
        }
    } // end of anonymous namespace
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        class GeoTexSvxMono : public GeoTexSvx
        {
        protected:
            basegfx::BColor                             maSingleColor;
            double                                      mfOpacity;

        public:
            GeoTexSvxMono(const basegfx::BColor& rSingleColor, double fOpacity);

            // compare operator
            virtual bool operator==(const GeoTexSvx& rGeoTexSvx) const;
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const;
        };

        GeoTexSvxMono::GeoTexSvxMono(const basegfx::BColor& rSingleColor, double fOpacity)
        :   maSingleColor(rSingleColor),
            mfOpacity(fOpacity)
        {
        }

        bool GeoTexSvxMono::operator==(const GeoTexSvx& rGeoTexSvx) const
        {
            const GeoTexSvxMono* pCompare = dynamic_cast< const GeoTexSvxMono* >(&rGeoTexSvx);
            return (pCompare
                && maSingleColor == pCompare->maSingleColor
                && mfOpacity == pCompare->mfOpacity);
        }

        void GeoTexSvxMono::modifyBColor(const basegfx::B2DPoint& /*rUV*/, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            rBColor = maSingleColor;
        }

        void GeoTexSvxMono::modifyOpacity(const basegfx::B2DPoint& /*rUV*/, double& rfOpacity) const
        {
            rfOpacity = mfOpacity;
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        class GeoTexSvxBitmap : public GeoTexSvx
        {
        protected:
            Bitmap                                      maBitmap;
            BitmapReadAccess*                           mpRead;
            basegfx::B2DPoint                           maTopLeft;
            basegfx::B2DVector                          maSize;
            double                                      mfMulX;
            double                                      mfMulY;

            // helpers
            bool impIsValid(const basegfx::B2DPoint& rUV, sal_Int32& rX, sal_Int32& rY) const;

        public:
            GeoTexSvxBitmap(const Bitmap& rBitmap, const basegfx::B2DPoint& rTopLeft, const basegfx::B2DVector& rSize);
            virtual ~GeoTexSvxBitmap();
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const;
        };

        GeoTexSvxBitmap::GeoTexSvxBitmap(const Bitmap& rBitmap, const basegfx::B2DPoint& rTopLeft, const basegfx::B2DVector& rSize)
        :   maBitmap(rBitmap),
            mpRead(0L),
            maTopLeft(rTopLeft),
            maSize(rSize),
            mfMulX(0.0),
            mfMulY(0.0)
        {
            mpRead = maBitmap.AcquireReadAccess();
            OSL_ENSURE(mpRead, "GeoTexSvxBitmap: Got no read access to Bitmap (!)");
            mfMulX = (double)mpRead->Width() / maSize.getX();
            mfMulY = (double)mpRead->Height() / maSize.getY();
        }

        GeoTexSvxBitmap::~GeoTexSvxBitmap()
        {
            delete mpRead;
        }

        bool GeoTexSvxBitmap::impIsValid(const basegfx::B2DPoint& rUV, sal_Int32& rX, sal_Int32& rY) const
        {
            if(mpRead)
            {
                rX = (sal_Int32)((rUV.getX() - maTopLeft.getX()) * mfMulX);

                if(rX >= 0L && rX < mpRead->Width())
                {
                    rY = (sal_Int32)((rUV.getY() - maTopLeft.getY()) * mfMulY);

                    return (rY >= 0L && rY < mpRead->Height());
                }
            }

            return false;
        }

        void GeoTexSvxBitmap::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const
        {
            sal_Int32 nX, nY;

            if(impIsValid(rUV, nX, nY))
            {
                const double fConvertColor(1.0 / 255.0);
                const BitmapColor aBMCol(mpRead->GetColor(nY, nX));
                const basegfx::BColor aBSource(
                    (double)aBMCol.GetRed() * fConvertColor,
                    (double)aBMCol.GetGreen() * fConvertColor,
                    (double)aBMCol.GetBlue() * fConvertColor);

                rBColor = aBSource;
            }
            else
            {
                rfOpacity = 0.0;
            }
        }

        void GeoTexSvxBitmap::modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const
        {
            sal_Int32 nX, nY;

            if(impIsValid(rUV, nX, nY))
            {
                const BitmapColor aBMCol(mpRead->GetColor(nY, nX));
                const Color aColor(aBMCol.GetRed(), aBMCol.GetGreen(), aBMCol.GetBlue());

                rfOpacity = ((double)(0xff - aColor.GetLuminance()) * (1.0 / 255.0));
            }
            else
            {
                rfOpacity = 0.0;
            }
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        class GeoTexSvxBitmapTiled : public GeoTexSvxBitmap
        {
        protected:
            // helpers
            basegfx::B2DPoint impGetCorrected(const basegfx::B2DPoint& rUV) const
            {
                double fX(fmod(rUV.getX() - maTopLeft.getX(), maSize.getX()));
                double fY(fmod(rUV.getY() - maTopLeft.getY(), maSize.getY()));

                if(fX < 0.0)
                {
                    fX += maSize.getX();
                }

                if(fY < 0.0)
                {
                    fY += maSize.getY();
                }

                return basegfx::B2DPoint(fX + maTopLeft.getX(), fY + maTopLeft.getY());
            }

        public:
            GeoTexSvxBitmapTiled(const Bitmap& rBitmap, const basegfx::B2DPoint& rTopLeft, const basegfx::B2DVector& rSize);
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const;
        };

        GeoTexSvxBitmapTiled::GeoTexSvxBitmapTiled(const Bitmap& rBitmap, const basegfx::B2DPoint& rTopLeft, const basegfx::B2DVector& rSize)
        :   GeoTexSvxBitmap(rBitmap, rTopLeft, rSize)
        {
        }

        void GeoTexSvxBitmapTiled::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const
        {
            if(mpRead)
            {
                GeoTexSvxBitmap::modifyBColor(impGetCorrected(rUV), rBColor, rfOpacity);
            }
        }

        void GeoTexSvxBitmapTiled::modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const
        {
            if(mpRead)
            {
                GeoTexSvxBitmap::modifyOpacity(impGetCorrected(rUV), rfOpacity);
            }
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        class GeoTexSvxMultiHatch : public GeoTexSvx
        {
        protected:
            basegfx::BColor                 maColor;
            double                          mfLogicPixelSize;
            GeoTexSvxHatch*                 mp0;
            GeoTexSvxHatch*                 mp1;
            GeoTexSvxHatch*                 mp2;

            // bitfield
            unsigned                        mbFillBackground : 1;

            // helpers
            bool impIsOnHatch(const basegfx::B2DPoint& rUV) const;

        public:
            GeoTexSvxMultiHatch(const primitive3d::HatchTexturePrimitive3D& rPrimitive, double fLogicPixelSize);
            virtual ~GeoTexSvxMultiHatch();
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const;

            // dada access
            bool getFillBackground() const { return mbFillBackground; }
        };

        GeoTexSvxMultiHatch::GeoTexSvxMultiHatch(const primitive3d::HatchTexturePrimitive3D& rPrimitive, double fLogicPixelSize)
        :   mfLogicPixelSize(fLogicPixelSize),
            mp0(0L),
            mp1(0L),
            mp2(0L)
        {
            const attribute::FillHatchAttribute& rHatch(rPrimitive.getHatch());
            const basegfx::B2DRange aOutlineRange(0.0, 0.0, rPrimitive.getTextureSize().getX(), rPrimitive.getTextureSize().getY());
            const double fAngleA(-rHatch.getAngle());
            maColor = rHatch.getColor();
            mbFillBackground = rHatch.isFillBackground();
            mp0 = new GeoTexSvxHatch(aOutlineRange, rHatch.getDistance(), fAngleA);

            if(attribute::HATCHSTYLE_DOUBLE == rHatch.getStyle() || attribute::HATCHSTYLE_TRIPLE == rHatch.getStyle())
            {
                mp1 = new GeoTexSvxHatch(aOutlineRange, rHatch.getDistance(), fAngleA + F_PI2);
            }

            if(attribute::HATCHSTYLE_TRIPLE == rHatch.getStyle())
            {
                mp2 = new GeoTexSvxHatch(aOutlineRange, rHatch.getDistance(), fAngleA + F_PI4);
            }
        }

        GeoTexSvxMultiHatch::~GeoTexSvxMultiHatch()
        {
            delete mp0;
            delete mp1;
            delete mp2;
        }

        bool GeoTexSvxMultiHatch::impIsOnHatch(const basegfx::B2DPoint& rUV) const
        {
            double fSmallestDistance();

            if(mp0->getDistanceToHatch(rUV) < mfLogicPixelSize)
            {
                return true;
            }

            if(mp1 && mp1->getDistanceToHatch(rUV) < mfLogicPixelSize)
            {
                return true;
            }

            if(mp2 && mp2->getDistanceToHatch(rUV) < mfLogicPixelSize)
            {
                return true;
            }

            return false;
        }

        void GeoTexSvxMultiHatch::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const
        {
            if(impIsOnHatch(rUV))
            {
                rBColor = maColor;
            }
            else if(!mbFillBackground)
            {
                rfOpacity = 0.0;
            }
        }

        void GeoTexSvxMultiHatch::modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const
        {
            if(mbFillBackground || impIsOnHatch(rUV))
            {
                rfOpacity = 1.0;
            }
            else
            {
                rfOpacity = 0.0;
            }
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        void DefaultProcessor3D::impRender_GRX3(const primitive3d::GradientTexturePrimitive3D& rPrimitive, bool bTransparence)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rPrimitive.getChildren();

            if(rSubSequence.hasElements())
            {
                // rescue values
                const bool bOldModulate(mbModulate); mbModulate = rPrimitive.getModulate();
                const bool bOldFilter(mbFilter); mbFilter = rPrimitive.getFilter();
                texture::GeoTexSvx* pOldTex = (bTransparence) ? mpTransparenceGeoTexSvx : mpGeoTexSvx;

                // create texture
                const attribute::FillGradientAttribute& rFillGradient = rPrimitive.getGradient();
                const basegfx::B2DRange aOutlineRange(0.0, 0.0, rPrimitive.getTextureSize().getX(), rPrimitive.getTextureSize().getY());
                const attribute::GradientStyle aGradientStyle(rFillGradient.getStyle());
                sal_uInt32 nSteps(rFillGradient.getSteps());
                const basegfx::BColor aStart(rFillGradient.getStartColor());
                const basegfx::BColor aEnd(rFillGradient.getEndColor());
                const sal_uInt32 nMaxSteps(sal_uInt32((aStart.getMaximumDistance(aEnd) * 127.5) + 0.5));
                texture::GeoTexSvx* pNewTex = 0L;

                if(nMaxSteps)
                {
                    // there IS a color distance
                    if(nSteps == 0L)
                    {
                        nSteps = nMaxSteps;
                    }

                    if(nSteps < 2L)
                    {
                        nSteps = 2L;
                    }

                    if(nSteps > nMaxSteps)
                    {
                        nSteps = nMaxSteps;
                    }

                    switch(aGradientStyle)
                    {
                        case attribute::GRADIENTSTYLE_LINEAR:
                        {
                            pNewTex = new texture::GeoTexSvxGradientLinear(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), -rFillGradient.getAngle());
                            break;
                        }
                        case attribute::GRADIENTSTYLE_AXIAL:
                        {
                            pNewTex = new texture::GeoTexSvxGradientAxial(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), -rFillGradient.getAngle());
                            break;
                        }
                        case attribute::GRADIENTSTYLE_RADIAL:
                        {
                            pNewTex = new texture::GeoTexSvxGradientRadial(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), rFillGradient.getOffsetX(), rFillGradient.getOffsetY());
                            break;
                        }
                        case attribute::GRADIENTSTYLE_ELLIPTICAL:
                        {
                            pNewTex = new texture::GeoTexSvxGradientElliptical(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), rFillGradient.getOffsetX(), rFillGradient.getOffsetY(), -rFillGradient.getAngle());
                            break;
                        }
                        case attribute::GRADIENTSTYLE_SQUARE:
                        {
                            pNewTex = new texture::GeoTexSvxGradientSquare(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), rFillGradient.getOffsetX(), rFillGradient.getOffsetY(), -rFillGradient.getAngle());
                            break;
                        }
                        case attribute::GRADIENTSTYLE_RECT:
                        {
                            pNewTex = new texture::GeoTexSvxGradientRect(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), rFillGradient.getOffsetX(), rFillGradient.getOffsetY(), -rFillGradient.getAngle());
                            break;
                        }
                    }
                }
                else
                {
                    // no color distance -> same color, use simple texture
                    pNewTex = new texture::GeoTexSvxMono(aStart, 1.0 - aStart.luminance());
                }

                // set created texture
                if(bTransparence)
                {
                    mpTransparenceGeoTexSvx = pNewTex;
                }
                else
                {
                    mpGeoTexSvx = pNewTex;
                }

                // process sub-list
                process(rSubSequence);

                // delete texture
                delete pNewTex;

                // restore values
                mbModulate = bOldModulate;
                mbFilter = bOldFilter;

                if(bTransparence)
                {
                    mpTransparenceGeoTexSvx = pOldTex;
                }
                else
                {
                    mpGeoTexSvx = pOldTex;
                }
            }
        }

        void DefaultProcessor3D::impRender_HAX3(const primitive3d::HatchTexturePrimitive3D& rPrimitive)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rPrimitive.getChildren();

            if(rSubSequence.hasElements())
            {
                // rescue values
                const bool bOldModulate(mbModulate); mbModulate = rPrimitive.getModulate();
                const bool bOldFilter(mbFilter); mbFilter = rPrimitive.getFilter();
                texture::GeoTexSvx* pOldTex = mpGeoTexSvx;

                // calculate logic pixel size in world coordinates
                const basegfx::B3DPoint aZero(maInvWorldToView * basegfx::B3DPoint(0.0, 0.0, 0.0));
                const basegfx::B3DPoint aOne(maInvWorldToView * basegfx::B3DPoint(1.0, 1.0, 1.0));
                const basegfx::B3DVector aLogicPixelSizeWorld(aOne - aZero);
                double fLogicPixelSizeWorld(fabs(aLogicPixelSizeWorld.getX()));

                if(fabs(aLogicPixelSizeWorld.getY()) > fLogicPixelSizeWorld)
                {
                    fLogicPixelSizeWorld = fabs(aLogicPixelSizeWorld.getY());
                }

                if(fabs(aLogicPixelSizeWorld.getZ()) > fLogicPixelSizeWorld)
                {
                    fLogicPixelSizeWorld = fabs(aLogicPixelSizeWorld.getZ());
                }

                // calculate logic pixel size in texture coordinates
                const double fLogicTexSizeX(fLogicPixelSizeWorld / rPrimitive.getTextureSize().getX());
                const double fLogicTexSizeY(fLogicPixelSizeWorld / rPrimitive.getTextureSize().getY());
                const double fLogicTexSize(fLogicTexSizeX > fLogicTexSizeY ? fLogicTexSizeX : fLogicTexSizeY);

                // create texture and set
                texture::GeoTexSvxMultiHatch* pNewTex = new texture::GeoTexSvxMultiHatch(rPrimitive, fLogicTexSize);
                mpGeoTexSvx = pNewTex;

                // process sub-list
                process(rSubSequence);

                // delete texture
                delete mpGeoTexSvx;

                // restore values
                mbModulate = bOldModulate;
                mbFilter = bOldFilter;
                mpGeoTexSvx = pOldTex;
            }
        }

        void DefaultProcessor3D::impRender_BMX3(const primitive3d::BitmapTexturePrimitive3D& rPrimitive)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rPrimitive.getChildren();

            if(rSubSequence.hasElements())
            {
                // rescue values
                const bool bOldModulate(mbModulate); mbModulate = rPrimitive.getModulate();
                const bool bOldFilter(mbFilter); mbFilter = rPrimitive.getFilter();
                texture::GeoTexSvx* pOldTex = mpGeoTexSvx;

                // create texture
                const attribute::FillBitmapAttribute& rFillBitmapAttribute = rPrimitive.getBitmap();

                if(rFillBitmapAttribute.getTiling())
                {
                    mpGeoTexSvx = new texture::GeoTexSvxBitmapTiled(
                        rFillBitmapAttribute.getBitmap(),
                        rFillBitmapAttribute.getTopLeft() * rPrimitive.getTextureSize(),
                        rFillBitmapAttribute.getSize() * rPrimitive.getTextureSize());
                }
                else
                {
                    mpGeoTexSvx = new texture::GeoTexSvxBitmap(
                        rFillBitmapAttribute.getBitmap(),
                        rFillBitmapAttribute.getTopLeft() * rPrimitive.getTextureSize(),
                        rFillBitmapAttribute.getSize() * rPrimitive.getTextureSize());
                }

                // process sub-list
                process(rSubSequence);

                // delete texture
                delete mpGeoTexSvx;

                // restore values
                mbModulate = bOldModulate;
                mbFilter = bOldFilter;
                mpGeoTexSvx = pOldTex;
            }
        }

        void DefaultProcessor3D::impRender_MCOL(const primitive3d::ModifiedColorPrimitive3D& rModifiedCandidate)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rModifiedCandidate.getChildren();

            if(rSubSequence.hasElements())
            {
                maBColorModifierStack.push(rModifiedCandidate.getColorModifier());
                process(rModifiedCandidate.getChildren());
                maBColorModifierStack.pop();
            }
        }

        void DefaultProcessor3D::impRender_POH3(const primitive3d::PolygonHairlinePrimitive3D& rPrimitive)
        {
            basegfx::B3DPolygon aHairline(rPrimitive.getB3DPolygon());

            if(aHairline.count() && mpBZPixelRaster)
            {
                // hairlines need no extra data, clear it
                aHairline.clearTextureCoordinates();
                aHairline.clearNormals();
                aHairline.clearBColors();

                // transform to device coordinates (-1.0 .. 1.0) and check for visibility
                aHairline.transform(maWorldToView);
                const basegfx::B3DRange a3DRange(basegfx::tools::getRange(aHairline));
                const basegfx::B2DRange a2DRange(a3DRange.getMinX(), a3DRange.getMinY(), a3DRange.getMaxX(), a3DRange.getMaxY());

                if(a2DRange.overlaps(maRasterRange))
                {
                    const attribute::MaterialAttribute3D aMaterial(rPrimitive.getBColor());
                    BZPolyRaCon aNewRaCon(false, *mpBZPixelRaster, aMaterial, *this);
                    aNewRaCon.addPolygon(aHairline, maInvEyeToView);
                    aNewRaCon.rasterconvert(0L, mpBZPixelRaster->getHeight());
                }
            }
        }

        void DefaultProcessor3D::impRender_POM3(const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive)
        {
            basegfx::B3DPolyPolygon aFill(rPrimitive.getB3DPolyPolygon());
            basegfx::BColor aObjectColor(rPrimitive.getMaterial().getColor());
            bool bPaintIt(aFill.count() && mpBZPixelRaster);

            if(bPaintIt)
            {
                // get rid of texture coordinates if there is no texture
                if(aFill.areTextureCoordinatesUsed() && !mpGeoTexSvx && !mpTransparenceGeoTexSvx)
                {
                    aFill.clearTextureCoordinates();
                }

                // transform to device coordinates (-1.0 .. 1.0) and check for visibility
                aFill.transform(maWorldToView);
                const basegfx::B3DRange a3DRange(basegfx::tools::getRange(aFill));
                const basegfx::B2DRange a2DRange(a3DRange.getMinX(), a3DRange.getMinY(), a3DRange.getMaxX(), a3DRange.getMaxY());

                bPaintIt = a2DRange.overlaps(maRasterRange);
            }

            // check if it shall be painted regarding hiding of normals (backface culling)
            if(bPaintIt && !rPrimitive.getDoubleSided())
            {
                // get plane normal of polygon in view coordinates (with ZBuffer values),
                // left-handed coordinate system
                const basegfx::B3DVector aPlaneNormal(aFill.getB3DPolygon(0L).getNormal());

                if(aPlaneNormal.getZ() > 0.0)
                {
                    bPaintIt = false;
                }
            }

            if(bPaintIt)
            {
                ::com::sun::star::drawing::ShadeMode aShadeMode(mrSdrSceneAttribute.getShadeMode());
                basegfx::B3DHomMatrix aNormalTransform(maWorldToEye);

                if(mrSdrSceneAttribute.getTwoSidedLighting())
                {
                    // get plane normal of polygon in view coordinates (with ZBuffer values),
                    // left-handed coordinate system
                    const basegfx::B3DVector aPlaneNormal(aFill.getB3DPolygon(0L).getNormal());

                    if(aPlaneNormal.getZ() > 0.0)
                    {
                        // mirror normals
                        aNormalTransform.scale(-1.0, -1.0, -1.0);
                    }
                }

                if(::com::sun::star::drawing::ShadeMode_PHONG == aShadeMode)
                {
                    // phong shading
                    if(aFill.areNormalsUsed())
                    {
                        // transform normals to eye coor
                        aFill.transformNormals(aNormalTransform);
                    }
                    else
                    {
                        // fallback to gouraud when no normals available
                        aShadeMode = ::com::sun::star::drawing::ShadeMode_SMOOTH;
                    }
                }

                if(::com::sun::star::drawing::ShadeMode_SMOOTH == aShadeMode)
                {
                    // gouraud shading
                    if(aFill.areNormalsUsed())
                    {
                        // transform normals to eye coor
                        aFill.transformNormals(aNormalTransform);

                        // prepare color model parameters, evtl. use blend color
                        const basegfx::BColor aColor(mbModulate ? basegfx::BColor(1.0, 1.0, 1.0) : rPrimitive.getMaterial().getColor());
                        const basegfx::BColor& rSpecular(rPrimitive.getMaterial().getSpecular());
                        const basegfx::BColor& rEmission(rPrimitive.getMaterial().getEmission());
                        const sal_uInt16 nSpecularIntensity(rPrimitive.getMaterial().getSpecularIntensity());

                        // solve color model for each normal vector, set colors at points. Clear normals.
                        for(sal_uInt32 a(0L); a < aFill.count(); a++)
                        {
                            basegfx::B3DPolygon aPartFill(aFill.getB3DPolygon(a));

                            for(sal_uInt32 b(0L); b < aPartFill.count(); b++)
                            {
                                // solve color model. Transform normal to eye coor
                                const basegfx::B3DVector aNormal(aPartFill.getNormal(b));
                                const basegfx::BColor aSolvedColor(mrSdrLightingAttribute.solveColorModel(aNormal, aColor, rSpecular, rEmission, nSpecularIntensity));
                                aPartFill.setBColor(b, aSolvedColor);
                            }

                            // clear normals on this part polygon and write it back
                            aPartFill.clearNormals();
                            aFill.setB3DPolygon(a, aPartFill);
                        }
                    }
                    else
                    {
                        // fallback to flat when no normals available
                        aShadeMode = ::com::sun::star::drawing::ShadeMode_FLAT;
                    }
                }

                if(::com::sun::star::drawing::ShadeMode_FLAT == aShadeMode)
                {
                    // flat shading. Clear normals and colors
                    aFill.clearNormals();
                    aFill.clearBColors();

                    // get plane vector in eye coordinates
                    const basegfx::B3DVector aPlaneEyeNormal(aNormalTransform * rPrimitive.getB3DPolyPolygon().getB3DPolygon(0L).getNormal());

                    // prepare color model parameters, evtl. use blend color
                    const basegfx::BColor aColor(mbModulate ? basegfx::BColor(1.0, 1.0, 1.0) : rPrimitive.getMaterial().getColor());
                    const basegfx::BColor& rSpecular(rPrimitive.getMaterial().getSpecular());
                    const basegfx::BColor& rEmission(rPrimitive.getMaterial().getEmission());
                    const sal_uInt16 nSpecularIntensity(rPrimitive.getMaterial().getSpecularIntensity());

                    // solve color model for plane vector and use that color for whole plane
                    aObjectColor = mrSdrLightingAttribute.solveColorModel(aPlaneEyeNormal, aColor, rSpecular, rEmission, nSpecularIntensity);
                }

                if(::com::sun::star::drawing::ShadeMode_DRAFT == aShadeMode)
                {
                    // draft, just use object color which is already set. Delete all other infos
                    aFill.clearNormals();
                    aFill.clearBColors();
                }
            }

            if(bPaintIt)
            {
                // draw it to ZBuffer
                const attribute::MaterialAttribute3D aMaterial(
                    aObjectColor, rPrimitive.getMaterial().getSpecular(),
                    rPrimitive.getMaterial().getEmission(),
                    rPrimitive.getMaterial().getSpecularIntensity());
                BZPolyRaCon aNewRaCon(true, *mpBZPixelRaster, aMaterial, *this);

                for(sal_uInt32 a(0L); a < aFill.count(); a++)
                {
                    aNewRaCon.addPolygon(aFill.getB3DPolygon(a), maInvEyeToView);
                }

                aNewRaCon.rasterconvert(0L, mpBZPixelRaster->getHeight());
            }
        }

        void DefaultProcessor3D::impRender_TRN3(const primitive3d::TransformPrimitive3D& rTransformCandidate)
        {
            // remember current transformations
            basegfx::B3DHomMatrix aLastWorldToView(maWorldToView);
            basegfx::B3DHomMatrix aLastWorldToEye(maWorldToEye);
            basegfx::B3DHomMatrix aLastInvWorldToView(maInvWorldToView);

            // create new transformations
            maWorldToView = maWorldToView * rTransformCandidate.getTransformation();
            maWorldToEye = maWorldToEye * rTransformCandidate.getTransformation();
            maInvWorldToView = maWorldToView;
            maInvWorldToView.invert();

            // let break down
            process(rTransformCandidate.getChildren());

            // restore transformations
            maWorldToView = aLastWorldToView;
            maWorldToEye = aLastWorldToEye;
            maInvWorldToView = aLastInvWorldToView;
        }

        void DefaultProcessor3D::process(const primitive3d::Primitive3DSequence& rSource)
        {
            if(rSource.hasElements())
            {
                const sal_Int32 nCount(rSource.getLength());

                for(sal_Int32 a(0L); a < nCount; a++)
                {
                    // get reference
                    const primitive3d::Primitive3DReference xReference(rSource[a]);

                    if(xReference.is())
                    {
                        // try to cast to BasePrimitive3D implementation
                        const primitive3d::BasePrimitive3D* pBasePrimitive = dynamic_cast< const primitive3d::BasePrimitive3D* >(xReference.get());

                        if(pBasePrimitive)
                        {
                            // it is a BasePrimitive3D implementation, use getPrimitiveID() call for switch
                            switch(pBasePrimitive->getPrimitiveID())
                            {
                                case PRIMITIVE3D_ID_GRADIENTTEXTUREPRIMITIVE3D :
                                {
                                    // GradientTexturePrimitive3D
                                    const primitive3d::GradientTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::GradientTexturePrimitive3D& >(*pBasePrimitive);
                                    impRender_GRX3(rPrimitive, false);
                                    break;
                                }
                                case PRIMITIVE3D_ID_HATCHTEXTUREPRIMITIVE3D :
                                {
                                    // HatchTexturePrimitive3D
                                    static bool bDoHatchDecomposition(true);

                                    if(bDoHatchDecomposition)
                                    {
                                        // let break down
                                        process(pBasePrimitive->get3DDecomposition(getTime()));
                                    }
                                    else
                                    {
                                        // hatchTexturePrimitive3D
                                        const primitive3d::HatchTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::HatchTexturePrimitive3D& >(*pBasePrimitive);
                                        impRender_HAX3(rPrimitive);
                                    }
                                    break;
                                }
                                case PRIMITIVE3D_ID_BITMAPTEXTUREPRIMITIVE3D :
                                {
                                    // BitmapTexturePrimitive3D
                                    const primitive3d::BitmapTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::BitmapTexturePrimitive3D& >(*pBasePrimitive);
                                    impRender_BMX3(rPrimitive);
                                    break;
                                }
                                case PRIMITIVE3D_ID_ALPHATEXTUREPRIMITIVE3D :
                                {
                                    // AlphaTexturePrimitive3D
                                    const primitive3d::AlphaTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::AlphaTexturePrimitive3D& >(*pBasePrimitive);

                                    if(mbProcessTransparent)
                                    {
                                        impRender_GRX3(rPrimitive, true);
                                    }
                                    else
                                    {
                                        mbContainsTransparent = true;
                                    }
                                    break;
                                }
                                case PRIMITIVE3D_ID_MODIFIEDCOLORPRIMITIVE3D :
                                {
                                    // ModifiedColorPrimitive3D
                                    // Force output to unified color.
                                    const primitive3d::ModifiedColorPrimitive3D& rPrimitive = static_cast< const primitive3d::ModifiedColorPrimitive3D& >(*pBasePrimitive);
                                    impRender_MCOL(rPrimitive);
                                    break;
                                }
                                case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D :
                                {
                                    // directdraw of PolygonHairlinePrimitive3D
                                    const primitive3d::PolygonHairlinePrimitive3D& rPrimitive = static_cast< const primitive3d::PolygonHairlinePrimitive3D& >(*pBasePrimitive);

                                    if((bool)mbProcessTransparent == (0L != mpTransparenceGeoTexSvx))
                                    {
                                        impRender_POH3(rPrimitive);
                                    }
                                    break;
                                }
                                case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D :
                                {
                                    // directdraw of PolyPolygonMaterialPrimitive3D
                                    const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const primitive3d::PolyPolygonMaterialPrimitive3D& >(*pBasePrimitive);

                                    if((bool)mbProcessTransparent == (0L != mpTransparenceGeoTexSvx))
                                    {
                                        impRender_POM3(rPrimitive);
                                    }
                                    break;
                                }
                                case PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D :
                                {
                                    // transform group (TransformPrimitive3D)
                                    impRender_TRN3(static_cast< const primitive3d::TransformPrimitive3D& >(*pBasePrimitive));
                                    break;
                                }
                                case PRIMITIVE3D_ID_SDRLABELPRIMITIVE3D :
                                {
                                    // SdrLabelPrimitive3D. Accept, but ignore. Is handled by the scenePrimitive decompose
                                    // method which creates 2d text objects at the 3d-projection-dependent positions.
                                    break;
                                }
                                default:
                                {
                                    // process recursively
                                    process(pBasePrimitive->get3DDecomposition(getTime()));
                                    break;
                                }
                            }
                        }
                        else
                        {
                            // unknown implementation, use UNO API call instead and process recursively
                            const uno::Sequence< beans::PropertyValue > xViewParameters(primitive3d::TimeToViewParameters(getTime()));
                            process(xReference->getDecomposition(xViewParameters));
                        }
                    }
                }
            }
        }

        void DefaultProcessor3D::processNonTransparent(const primitive3d::Primitive3DSequence& rSource)
        {
            mbProcessTransparent = false;
            mbContainsTransparent = false;
            process(rSource);
        }

        void DefaultProcessor3D::processTransparent(const primitive3d::Primitive3DSequence& rSource)
        {
            if(mbContainsTransparent)
            {
                mbProcessTransparent = true;
                process(rSource);
            }
        }

        DefaultProcessor3D::DefaultProcessor3D(
            const geometry::ViewInformation2D& rViewInformation,
            const geometry::Transformation3D& rTransformation3D,
            const attribute::SdrSceneAttribute& rSdrSceneAttribute,
            const attribute::SdrLightingAttribute& rSdrLightingAttribute,
            double fSizeX,
            double fSizeY,
            const basegfx::B2DRange& rVisiblePart)
        :   BaseProcessor3D(rViewInformation.getViewTime()),
            mrSdrSceneAttribute(rSdrSceneAttribute),
            mrSdrLightingAttribute(rSdrLightingAttribute),
            maDeviceToView(),
            maWorldToEye(),
            maWorldToView(),
            maInvEyeToView(),
            maInvWorldToView(),
            maRasterRange(),
            mpBZPixelRaster(0),
            maBColorModifierStack(),
            mpGeoTexSvx(0),
            mpTransparenceGeoTexSvx(0),
            mbModulate(false),
            mbFilter(false),
            mbProcessTransparent(false),
            mbContainsTransparent(false)
        {
            // generate ViewSizes
            const double fFullViewSizeX((rViewInformation.getViewTransformation() * basegfx::B2DVector(fSizeX, 0.0)).getLength());
            const double fFullViewSizeY((rViewInformation.getViewTransformation() * basegfx::B2DVector(0.0, fSizeY)).getLength());
            const double fViewSizeX(fFullViewSizeX * rVisiblePart.getWidth());
            const double fViewSizeY(fFullViewSizeY * rVisiblePart.getHeight());
            const sal_uInt32 nViewSizeX((sal_uInt32)floor(fViewSizeX));
            const sal_uInt32 nViewSizeY((sal_uInt32)floor(fViewSizeY));

            if(nViewSizeX && nViewSizeY)
            {
                // create view unit buffer
                mpBZPixelRaster = new basegfx::BZPixelRaster(nViewSizeX + 1L, nViewSizeY + 1L);
                OSL_ENSURE(mpBZPixelRaster, "DefaultProcessor3D: Could not allocate basegfx::BZPixelRaster (!)");

                // create DeviceToView
                // outcome is [-1.0 .. 1.0] in X,Y and Z.

                {
                    // step one:
                    //
                    // bring from [-1.0 .. 1.0] in X,Y and Z to [0.0 .. 1.0]. Also
                    // necessary to
                    // - flip Y due to screen orientation
                    // - flip Z due to Z-Buffer orientation from back to front

                    maDeviceToView.scale(0.5, -0.5, -0.5);
                    maDeviceToView.translate(0.5, 0.5, 0.5);
                }

                {
                    // step two:
                    //
                    // bring from [0.0 .. 1.0] in X,Y and Z to view cordinates. also:
                    // - scale Z to [0.0 .. fMaxZDepth]
                    const double fMaxZDepth(double(0x0000ff00));
                    maDeviceToView.translate(-rVisiblePart.getMinX(), -rVisiblePart.getMinY(), 0.0);
                    maDeviceToView.scale(fFullViewSizeX, fFullViewSizeY, fMaxZDepth);
                }

                // create world to eye transformation
                maWorldToEye = rTransformation3D.getOrientation() * rTransformation3D.getTransformation();

                // create EyeToView transformation
                maWorldToView = maDeviceToView * rTransformation3D.getProjection() * maWorldToEye;

                // create inverse EyeToView transformation
                maInvEyeToView = maDeviceToView * rTransformation3D.getProjection();
                maInvEyeToView.invert();

                // create inverse WorldToView transformation
                maInvWorldToView = maWorldToView;
                maInvWorldToView.invert();

                // prepare maRasterRange
                maRasterRange.expand(basegfx::B2DPoint(0.0, 0.0));
                maRasterRange.expand(basegfx::B2DPoint(mpBZPixelRaster->getWidth(), mpBZPixelRaster->getHeight()));
            }
        }

        DefaultProcessor3D::~DefaultProcessor3D()
        {
            if(mpBZPixelRaster)
            {
                delete mpBZPixelRaster;
            }
        }

        BitmapEx DefaultProcessor3D::getBitmapEx() const
        {
            if(mpBZPixelRaster)
            {
                return basegfx::getBitmapEx(*mpBZPixelRaster);
            }

            return BitmapEx();
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
