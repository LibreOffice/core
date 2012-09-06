/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2010 Novell, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <tools/stream.hxx>
#include <vcl/metaact.hxx>
#include <svtools/filter.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/tools/gradienttools.hxx>
#include <basegfx/tools/tools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <vcl/canvastools.hxx>
#include <rtl/ustring.hxx>
#include <sal/alloca.h>

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>

#include <bitmapaction.hxx>
#include <implrenderer.hxx>
#include <outdevstate.hxx>
#include <polypolyaction.hxx>
#include <textaction.hxx>
#include <stdio.h>

#define EmfPlusRecordTypeHeader 16385
#define EmfPlusRecordTypeEndOfFile 16386
#define EmfPlusRecordTypeGetDC 16388
#define EmfPlusRecordTypeObject 16392
#define EmfPlusRecordTypeFillRects 16394
#define EmfPlusRecordTypeFillPolygon 16396
#define EmfPlusRecordTypeDrawLines 16397
#define EmfPlusRecordTypeFillPath 16404
#define EmfPlusRecordTypeDrawPath 16405
#define EmfPlusRecordTypeDrawImage 16410
#define EmfPlusRecordTypeDrawImagePoints 16411
#define EmfPlusRecordTypeDrawString 16412
#define EmfPlusRecordTypeSetRenderingOrigin 16413
#define EmfPlusRecordTypeSetAntiAliasMode 16414
#define EmfPlusRecordTypeSetTextRenderingHint 16415
#define EmfPlusRecordTypeSetInterpolationMode 16417
#define EmfPlusRecordTypeSetPixelOffsetMode 16418
#define EmfPlusRecordTypeSetCompositingQuality 16420
#define EmfPlusRecordTypeSave 16421
#define EmfPlusRecordTypeSetWorldTransform 16426
#define EmfPlusRecordTypeResetWorldTransform 16427
#define EmfPlusRecordTypeMultiplyWorldTransform 16428
#define EmfPlusRecordTypeSetPageTransform 16432
#define EmfPlusRecordTypeSetClipPath 16435
#define EmfPlusRecordTypeSetClipRegion 16436
#define EmfPlusRecordTypeDrawDriverString 16438

#define EmfPlusObjectTypeBrush 0x100
#define EmfPlusObjectTypePen 0x200
#define EmfPlusObjectTypePath 0x300
#define EmfPlusObjectTypeRegion 0x400
#define EmfPlusObjectTypeImage 0x500
#define EmfPlusObjectTypeFont 0x600
#define EmfPlusObjectTypeStringFormat 0x700
#define EmfPlusObjectTypeImageAttributes 0x800

#define EmfPlusRegionInitialStateRectangle 0x10000000
#define EmfPlusRegionInitialStatePath 0x10000001
#define EmfPlusRegionInitialStateEmpty 0x10000002
#define EmfPlusRegionInitialStateInfinite 0x10000003

#if OSL_DEBUG_LEVEL > 1
#define EMFP_DEBUG(x) x
#else
#define EMFP_DEBUG(x)
#endif

using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {

        EMFP_DEBUG (void dumpWords (SvStream& s, int i)
        {
            sal_uInt32 pos = s.Tell ();
            sal_Int16 data;
            for (; i > 0; i --) {
                s >> data;
                printf ("EMF+\tdata: %04hX\n", data);
            }
            s.Seek (pos);
        });

        struct EMFPObject
        {
            virtual ~EMFPObject() {}
        };

        struct EMFPPath : public EMFPObject
        {
            ::basegfx::B2DPolyPolygon    aPolygon;
            sal_Int32                    nPoints;
            float*                       pPoints;
            sal_uInt8*                   pPointTypes;

        public:
            EMFPPath (sal_Int32 _nPoints, bool bLines = false)
            {
                if( _nPoints<0 || sal_uInt32(_nPoints)>SAL_MAX_INT32/(2*sizeof(float)) )
                    _nPoints = SAL_MAX_INT32/(2*sizeof(float));
                nPoints = _nPoints;
                pPoints = new float [nPoints*2];
                if (!bLines)
                    pPointTypes = new sal_uInt8 [_nPoints];
                else
                    pPointTypes = NULL;
            }

            ~EMFPPath ()
            {
                delete [] pPoints;
                delete [] pPointTypes;
            }

            // TODO: remove rR argument when debug code is not longer needed
            void Read (SvStream& s, sal_uInt32 pathFlags, ImplRenderer& rR)
            {
                for (int i = 0; i < nPoints; i ++) {
                    if (pathFlags & 0x4000) {
                        // points are stored in short 16bit integer format
                        sal_uInt16 x, y;

                        s >> x >> y;
                        EMFP_DEBUG (printf ("EMF+\tpoint [x,y]: %hd,%hd\n", x, y));
                        pPoints [i*2] = x;
                        pPoints [i*2 + 1] = y;
                    } else {
                        // points are stored in Single (float) format
                        s >> pPoints [i*2] >> pPoints [i*2 + 1];
                        EMFP_DEBUG (printf ("EMF+\tpoint [x,y]: %f,%f\n", pPoints [i*2], pPoints [i*2 + 1]));
                    }
                }

                if (pPointTypes)
                    for (int i = 0; i < nPoints; i ++) {
                        s >> pPointTypes [i];
                        EMFP_DEBUG (printf ("EMF+\tpoint type: %x\n", pPointTypes [i]));
                    }

                aPolygon.clear ();

                (void) rR; // avoid warnings
                EMFP_DEBUG (
                    const ::basegfx::B2DRectangle aBounds (::basegfx::tools::getRange (GetPolygon (rR)));
                    printf ("EMF+\tpolygon bounding box: %f,%f %fx%f (mapped)\n", aBounds.getMinX (), aBounds.getMinY (), aBounds.getWidth (), aBounds.getHeight ()));
            }

            ::basegfx::B2DPolyPolygon& GetPolygon (ImplRenderer& rR, bool bMapIt = true)
            {
                ::basegfx::B2DPolygon polygon;

                aPolygon.clear ();

                int last_normal = 0, p = 0;
                ::basegfx::B2DPoint prev, mapped;
                bool hasPrev = false;
                for (int i = 0; i < nPoints; i ++) {
                    if (p && pPointTypes && (pPointTypes [i] == 0)) {
                        aPolygon.append (polygon);
                        last_normal = i;
                        p = 0;
                        polygon.clear ();
                    }

                    if (bMapIt)
                        mapped = rR.Map (pPoints [i*2], pPoints [i*2 + 1]);
                    else
                        mapped = ::basegfx::B2DPoint (pPoints [i*2], pPoints [i*2 + 1]);
                    //EMFP_DEBUG (printf ("polygon point: %f,%f mapped: %f,%f\n", pPoints [i*2], pPoints [i*2 + 1], mapped.getX (), mapped.getY ()));
                    if (pPointTypes) {
                        if ((pPointTypes [i] & 0x07) == 3) {
                            if (((i - last_normal )% 3) == 1) {
                                polygon.setNextControlPoint (p - 1, mapped);
                                EMFP_DEBUG (printf ("polygon append  next: %d mapped: %f,%f\n", p - 1, mapped.getX (), mapped.getY ()));
                                continue;
                            } else if (((i - last_normal) % 3) == 2) {
                                prev = mapped;
                                hasPrev = true;
                                continue;
                            }
                        } else
                            last_normal = i;
                    }
                    polygon.append (mapped);
                    EMFP_DEBUG (printf ("polygon append point: %f,%f mapped: %f,%f\n", pPoints [i*2], pPoints [i*2 + 1], mapped.getX (), mapped.getY ()));
                    if (hasPrev) {
                        polygon.setPrevControlPoint (p, prev);
                        EMFP_DEBUG (printf ("polygon append  prev: %d mapped: %f,%f\n", p, prev.getX (), prev.getY ()));
                        hasPrev = false;
                    }
                    p ++;
                    if (pPointTypes && (pPointTypes [i] & 0x80)) { // closed polygon
                        polygon.setClosed (true);
                        aPolygon.append (polygon);
                        EMFP_DEBUG (printf ("close polygon\n"));
                        last_normal = i + 1;
                        p = 0;
                        polygon.clear ();
                    }
                }

                if (polygon.count ())
                    aPolygon.append (polygon);

                EMFP_DEBUG (
                    for (unsigned int i=0; i<aPolygon.count(); i++) {
                        polygon = aPolygon.getB2DPolygon(i);
                        printf ("polygon: %d\n", i);
                        for (unsigned int j=0; j<polygon.count(); j++) {
                            ::basegfx::B2DPoint point = polygon.getB2DPoint(j);
                            printf ("point: %f,%f\n", point.getX(), point.getY());
                            if (polygon.isPrevControlPointUsed(j)) {
                                point = polygon.getPrevControlPoint(j);
                                printf ("prev: %f,%f\n", point.getX(), point.getY());
                            }
                            if (polygon.isNextControlPointUsed(j)) {
                                point = polygon.getNextControlPoint(j);
                                printf ("next: %f,%f\n", point.getX(), point.getY());
                            }
                        }
                    }
                );

                return aPolygon;
            }
        };

        struct EMFPRegion : public EMFPObject
        {
            sal_Int32 parts;
            sal_Int32 *combineMode;
            sal_Int32 initialState;
            EMFPPath *initialPath;
            float ix, iy, iw, ih;

            EMFPRegion ()
            {
                combineMode = NULL;
                initialPath = NULL;
            }

            ~EMFPRegion ()
            {
                if (combineMode) {
                    delete [] combineMode;
                    combineMode = NULL;
                }
                if (initialPath) {
                    delete initialPath;
                    initialPath = NULL;
                }
            }

            void Read (SvStream& s)
            {
                sal_uInt32 header;

                s >> header >> parts;

                EMFP_DEBUG (printf ("EMF+\tregion\n"));
                EMFP_DEBUG (printf ("EMF+\theader: 0x%08x parts: %d\n", (unsigned int)header, (int)parts));

                if (parts) {
                    if( parts<0 || sal_uInt32(parts)>SAL_MAX_INT32/sizeof(sal_Int32) )
                        parts = SAL_MAX_INT32/sizeof(sal_Int32);

                    combineMode = new sal_Int32 [parts];

                    for (int i = 0; i < parts; i ++) {
                        s >> combineMode [i];
                        EMFP_DEBUG (printf ("EMF+\tcombine mode [%d]: 0x%08x\n", i,(unsigned int)combineMode [i]));
                    }
                }

                s >> initialState;
                EMFP_DEBUG (printf ("EMF+\tinitial state: 0x%d\n",(int) initialState));
            }
        };

        struct EMFPBrush : public EMFPObject
        {
            ::Color solidColor;
            sal_uInt32 type;
            sal_uInt32 additionalFlags;

            /* linear gradient */
            sal_Int32 wrapMode;
            float areaX, areaY, areaWidth, areaHeight;
            ::Color secondColor; // first color is stored in solidColor;
            XForm transformation;
            bool hasTransformation;
            sal_Int32 blendPoints;
            float* blendPositions;
            float* blendFactors;
            sal_Int32 colorblendPoints;
            float* colorblendPositions;
            ::Color* colorblendColors;
            sal_Int32 surroundColorsNumber;
            ::Color* surroundColors;
            EMFPPath *path;

        public:
            EMFPBrush ()
            {
                blendPositions = NULL;
                colorblendPositions = NULL;
                colorblendColors = NULL;
                surroundColors = NULL;
                path = NULL;
                hasTransformation = false;
            }

            ~EMFPBrush ()
            {
                if (blendPositions != NULL) {
                    delete[] blendPositions;
                    blendPositions = NULL;
                }
                if (colorblendPositions != NULL) {
                    delete[] colorblendPositions;
                    colorblendPositions = NULL;
                }
                if (colorblendColors != NULL) {
                    delete[] colorblendColors;
                    colorblendColors = NULL;
                }
                if (surroundColors != NULL) {
                    delete[] surroundColors;
                    surroundColors = NULL;
                }
                if (path) {
                    delete path;
                    path = NULL;
                }
            }

            sal_uInt32 GetType() const { return type; }
            const ::Color& GetColor() const { return solidColor; }

            void Read (SvStream& s, ImplRenderer& rR)
            {
                sal_uInt32 header;

                s >> header >> type;

                EMFP_DEBUG (printf ("EMF+\tbrush\nEMF+\theader: 0x%08x type: %d\n",(unsigned int) header,(int) type));

                switch (type) {
                case 0:
                    {
                        sal_uInt32 color;

                        s >> color;
                        solidColor = ::Color (0xff - (color >> 24), (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
                        EMFP_DEBUG (printf ("EMF+\tsolid color: 0x%08x\n", (unsigned int)color));

                        break;
                    }
                // path gradient
                case 3:
                    {
                        s >> additionalFlags >> wrapMode;

                        EMFP_DEBUG (printf ("EMF+\tpath gradient, additional flags: 0x%02x\n",(unsigned int) additionalFlags));

                        sal_uInt32 color;

                        s >> color;
                        solidColor = ::Color (0xff - (color >> 24), (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
                        EMFP_DEBUG (printf ("EMF+\tcenter color: 0x%08x\n",(unsigned int) color));

                        s >> areaX >> areaY;
                        EMFP_DEBUG (printf ("EMF+\tcenter point: %f,%f\n", areaX, areaY));

                        s >> surroundColorsNumber;
                        EMFP_DEBUG (printf ("EMF+\tsurround colors: %d\n",(int) surroundColorsNumber));

                        if( surroundColorsNumber<0 || sal_uInt32(surroundColorsNumber)>SAL_MAX_INT32/sizeof(::Color) )
                            surroundColorsNumber = SAL_MAX_INT32/sizeof(::Color);

                        surroundColors = new ::Color [surroundColorsNumber];
                        for (int i = 0; i < surroundColorsNumber; i++) {
                            s >> color;
                            surroundColors[i] = ::Color (0xff - (color >> 24), (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
                            if (i == 0)
                                secondColor = surroundColors [0];
                            EMFP_DEBUG (printf ("EMF+\tsurround color[%d]: 0x%08x\n", i, (unsigned int)color));
                        }

                        if (additionalFlags & 0x01) {
                            sal_Int32 pathLength;

                            s >> pathLength;
                            EMFP_DEBUG (printf ("EMF+\tpath length: %d\n", (int)pathLength));

                            sal_uInt32 pos = s.Tell ();
                            EMFP_DEBUG (dumpWords (s, 32));

                            sal_uInt32 pathHeader;
                            sal_Int32 pathPoints, pathFlags;
                            s >> pathHeader >> pathPoints >> pathFlags;

                            EMFP_DEBUG (printf ("EMF+\tpath (brush path gradient)\n"));
                            EMFP_DEBUG (printf ("EMF+\theader: 0x%08x points: %d additional flags: 0x%d\n", (unsigned int)pathHeader, (int)pathPoints, (int)pathFlags));

                            path = new EMFPPath (pathPoints);
                            path->Read (s, pathFlags, rR);

                            s.Seek (pos + pathLength);

                            const ::basegfx::B2DRectangle aBounds (::basegfx::tools::getRange (path->GetPolygon (rR, false)));
                            areaWidth = aBounds.getWidth ();
                            areaHeight = aBounds.getHeight ();

                            EMFP_DEBUG (printf ("EMF+\tpolygon bounding box: %f,%f %fx%f\n", aBounds.getMinX (), aBounds.getMinY (), aBounds.getWidth (), aBounds.getHeight ()));


                        if (additionalFlags & 0x02) {
                            EMFP_DEBUG (printf ("EMF+\tuse transformation\n"));
                            s >> transformation;
                            hasTransformation = true;
                            EMFP_DEBUG (printf ("EMF+\tm11: %f m12: %f\nEMF+\tm21: %f m22: %f\nEMF+\tdx: %f dy: %f\n",
                                    transformation.eM11, transformation.eM12,
                                    transformation.eM21, transformation.eM22,
                                    transformation.eDx, transformation.eDy));
                        }
                        if (additionalFlags & 0x08) {
                            s >> blendPoints;
                            EMFP_DEBUG (printf ("EMF+\tuse blend, points: %d\n", (int)blendPoints));
                            if( blendPoints<0 || sal_uInt32(blendPoints)>SAL_MAX_INT32/(2*sizeof(float)) )
                                blendPoints = SAL_MAX_INT32/(2*sizeof(float));
                            blendPositions = new float [2*blendPoints];
                            blendFactors = blendPositions + blendPoints;
                            for (int i=0; i < blendPoints; i ++) {
                                s >> blendPositions [i];
                                EMFP_DEBUG (printf ("EMF+\tposition[%d]: %f\n", i, blendPositions [i]));
                            }
                            for (int i=0; i < blendPoints; i ++) {
                                s >> blendFactors [i];
                                EMFP_DEBUG (printf ("EMF+\tfactor[%d]: %f\n", i, blendFactors [i]));
                            }
                        }

                        if (additionalFlags & 0x04) {
                            s >> colorblendPoints;
                            EMFP_DEBUG (printf ("EMF+\tuse color blend, points: %d\n", (int)colorblendPoints));
                            if( colorblendPoints<0 || sal_uInt32(colorblendPoints)>SAL_MAX_INT32/sizeof(float) )
                                colorblendPoints = SAL_MAX_INT32/sizeof(float);
                            if( sal_uInt32(colorblendPoints)>SAL_MAX_INT32/sizeof(::Color) )
                                colorblendPoints = SAL_MAX_INT32/sizeof(::Color);
                            colorblendPositions = new float [colorblendPoints];
                            colorblendColors = new ::Color [colorblendPoints];
                            for (int i=0; i < colorblendPoints; i ++) {
                                s >> colorblendPositions [i];
                                EMFP_DEBUG (printf ("EMF+\tposition[%d]: %f\n", i, colorblendPositions [i]));
                            }
                            for (int i=0; i < colorblendPoints; i ++) {
                                s >> color;
                                colorblendColors [i] = ::Color (0xff - (color >> 24), (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
                                EMFP_DEBUG (printf ("EMF+\tcolor[%d]: 0x%08x\n", i,(unsigned int) color));
                            }
                        }
                        } else {
                            EMFP_DEBUG (dumpWords (s, 1024));
            }
                        break;
                    }
                // linear gradient
                case 4:
                    {
                        s >> additionalFlags >> wrapMode;

                        EMFP_DEBUG (printf ("EMF+\tlinear gradient, additional flags: 0x%02x\n", (unsigned int)additionalFlags));

                        s >> areaX >> areaY >> areaWidth >> areaHeight;

                        EMFP_DEBUG (printf ("EMF+\tarea: %f,%f - %fx%f\n", areaX, areaY, areaWidth, areaHeight));

                        sal_uInt32 color;

                        s >> color;
                        solidColor = ::Color (0xff - (color >> 24), (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
                        EMFP_DEBUG (printf ("EMF+\tfirst color: 0x%08x\n", color));

                        s >> color;
                        secondColor = ::Color (0xff - (color >> 24), (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
                        EMFP_DEBUG (printf ("EMF+\tsecond color: 0x%08x\n", color));

                        // repeated colors, unknown meaning, see http://www.aces.uiuc.edu/~jhtodd/Metafile/MetafileRecords/ObjectBrush.html
                        s >> color;
                        s >> color;

                        if (additionalFlags & 0x02) {
                            EMFP_DEBUG (printf ("EMF+\tuse transformation\n"));
                            s >> transformation;
                            hasTransformation = true;
                            EMFP_DEBUG (printf ("EMF+\tm11: %f m12: %f\nEMF+\tm21: %f m22: %f\nEMF+\tdx: %f dy: %f\n",
                                    transformation.eM11, transformation.eM12,
                                    transformation.eM21, transformation.eM22,
                                    transformation.eDx, transformation.eDy));
                        }
                        if (additionalFlags & 0x08) {
                            s >> blendPoints;
                            EMFP_DEBUG (printf ("EMF+\tuse blend, points: %d\n", (int)blendPoints));
                            if( blendPoints<0 || sal_uInt32(blendPoints)>SAL_MAX_INT32/(2*sizeof(float)) )
                                blendPoints = SAL_MAX_INT32/(2*sizeof(float));
                            blendPositions = new float [2*blendPoints];
                            blendFactors = blendPositions + blendPoints;
                            for (int i=0; i < blendPoints; i ++) {
                                s >> blendPositions [i];
                                EMFP_DEBUG (printf ("EMF+\tposition[%d]: %f\n", i, blendPositions [i]));
                            }
                            for (int i=0; i < blendPoints; i ++) {
                                s >> blendFactors [i];
                                EMFP_DEBUG (printf ("EMF+\tfactor[%d]: %f\n", i, blendFactors [i]));
                            }
                        }

                        if (additionalFlags & 0x04) {
                            s >> colorblendPoints;
                            EMFP_DEBUG (printf ("EMF+\tuse color blend, points: %d\n", (int)colorblendPoints));
                            if( colorblendPoints<0 || sal_uInt32(colorblendPoints)>SAL_MAX_INT32/sizeof(float) )
                                colorblendPoints = SAL_MAX_INT32/sizeof(float);
                            if( sal_uInt32(colorblendPoints)>SAL_MAX_INT32/sizeof(::Color) )
                                colorblendPoints = sal_uInt32(SAL_MAX_INT32)/sizeof(::Color);
                            colorblendPositions = new float [colorblendPoints];
                            colorblendColors = new ::Color [colorblendPoints];
                            for (int i=0; i < colorblendPoints; i ++) {
                                s >> colorblendPositions [i];
                                EMFP_DEBUG (printf ("EMF+\tposition[%d]: %f\n", i, colorblendPositions [i]));
                            }
                            for (int i=0; i < colorblendPoints; i ++) {
                                s >> color;
                                colorblendColors [i] = ::Color (0xff - (color >> 24), (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
                                EMFP_DEBUG (printf ("EMF+\tcolor[%d]: 0x%08x\n", i, (unsigned int)color));
                            }
                        }

                        break;
                    }
                default:
                    EMFP_DEBUG (printf ("EMF+\tunhandled brush type: %08x\n", (unsigned int)type));
                }
            }
        };

        struct EMFPPen : public EMFPBrush
        {
            XForm transformation;
            float width;
            sal_Int32 startCap;
            sal_Int32 endCap;
            sal_Int32 lineJoin;
            float mitterLimit;
            sal_Int32 dashStyle;
            sal_Int32 dashCap;
            float dashOffset;
            sal_Int32 dashPatternLen;
            float *dashPattern;
            sal_Int32 alignment;
            sal_Int32 compoundArrayLen;
            float *compoundArray;
            sal_Int32 customStartCapLen;
            sal_uInt8 *customStartCap;
            sal_Int32 customEndCapLen;
            sal_uInt8 *customEndCap;

        public:
            EMFPPen () : EMFPBrush ()
            {
            }

            void SetStrokeAttributes (rendering::StrokeAttributes& rStrokeAttributes, ImplRenderer& rR, const OutDevState& rState)
            {
                EMFP_DEBUG (if (width == 0.0) printf ("EMF+\tTODO: pen with zero width - using minimal which might not be correct\n"));
                rStrokeAttributes.StrokeWidth = (rState.mapModeTransform * rR.MapSize (width == 0.0 ? 0.05 : width, 0)).getX ();
            }

            void Read (SvStream& s, ImplRenderer& rR, sal_Int32, sal_Int32 )
            {
                sal_uInt32 header, unknown, penFlags, unknown2;
                int i;

                s >> header >> unknown >> penFlags >> unknown2 >> width;

                EMFP_DEBUG (printf ("EMF+\tpen\nEMF+\theader: 0x%08x unknown: 0x%08x additional flags: 0x%08x unknown: 0x%08x width: %f\n", (unsigned int)header, (unsigned int)unknown, (unsigned int)penFlags,(unsigned int) unknown2, width));

                if (penFlags & 1)
                    s >> transformation;

                if (penFlags & 2)
                    s >> startCap;
                else
                    startCap = 0;

                if (penFlags & 4)
                    s >> endCap;
                else
                    endCap = 0;

                if (penFlags & 8)
                    s >> lineJoin;
                else
                    lineJoin = 0;

                if (penFlags & 16)
                    s >> mitterLimit;
                else
                    mitterLimit = 0;

                if (penFlags & 32)
                    s >> dashStyle;
                else
                    dashStyle = 0;

                if (penFlags & 64)
                    s >> dashCap;
                else
                    dashCap = 0;

                if (penFlags & 128)
                    s >> dashOffset;
                else
                    dashOffset = 0;

                if (penFlags & 256) {
                    s >> dashPatternLen;
                    if( dashPatternLen<0 || sal_uInt32(dashPatternLen)>SAL_MAX_INT32/sizeof(float) )
                        dashPatternLen = SAL_MAX_INT32/sizeof(float);
                    dashPattern = new float [dashPatternLen];
                    for (i = 0; i < dashPatternLen; i++)
                        s >> dashPattern [i];
                } else
                    dashPatternLen = 0;

                if (penFlags & 512)
                    s >> alignment;
                else
                    alignment = 0;

                if (penFlags & 1024) {
                    s >> compoundArrayLen;
                    if( compoundArrayLen<0 || sal_uInt32(compoundArrayLen)>SAL_MAX_INT32/sizeof(float) )
                        compoundArrayLen = SAL_MAX_INT32/sizeof(float);
                    compoundArray = new float [compoundArrayLen];
                    for (i = 0; i < compoundArrayLen; i++)
                        s >> compoundArray [i];
                } else
                    compoundArrayLen = 0;

                if (penFlags & 2048) {
                    s >> customStartCapLen;
                    if( customStartCapLen<0 )
                        customStartCapLen=0;
                    customStartCap = new sal_uInt8 [customStartCapLen];
                    for (i = 0; i < customStartCapLen; i++)
                        s >> customStartCap [i];
                } else
                    customStartCapLen = 0;

                if (penFlags & 4096) {
                    s >> customEndCapLen;
                    if( customEndCapLen<0 )
                        customEndCapLen=0;
                    customEndCap = new sal_uInt8 [customEndCapLen];
                    for (i = 0; i < customEndCapLen; i++)
                        s >> customEndCap [i];
                } else
                    customEndCapLen = 0;

                EMFPBrush::Read (s, rR);
            }
        };

        struct EMFPImage : public EMFPObject
        {
            sal_uInt32 type;
            sal_Int32 width;
            sal_Int32 height;
            sal_Int32 stride;
            sal_Int32 pixelFormat;
            Graphic graphic;


            void Read (SvMemoryStream &s, sal_Bool bUseWholeStream)
            {
                sal_uInt32 header, unknown;

                s >> header >> type;

                EMFP_DEBUG (printf ("EMF+\timage\nEMF+\theader: 0x%08x type: 0x%08x\n", header, type));

                if (type == 1) { // bitmap
                    s >> width >> height >> stride >> pixelFormat >> unknown;
                    EMFP_DEBUG (printf ("EMF+\tbitmap width: %d height: %d stride: %d pixelFormat: 0x%08x\n", width, height, stride, pixelFormat));
                    if (width == 0) { // non native formats
                        GraphicFilter filter;

                        filter.ImportGraphic (graphic, String (), s);
                        EMFP_DEBUG (printf ("EMF+\tbitmap width: %ld height: %ld\n", graphic.GetBitmap ().GetSizePixel ().Width (), graphic.GetBitmap ().GetSizePixel ().Height ()));
                    }

                } else if (type == 2) {
                    sal_Int32 mfType, mfSize;

                    s >> mfType >> mfSize;
                    EMFP_DEBUG (printf ("EMF+\tmetafile type: %d dataSize: %d\n", mfType, mfSize));

                    GraphicFilter filter;
                    // workaround buggy metafiles, which have wrong mfSize set (n#705956 for example)
                    SvMemoryStream mfStream (((char *)s.GetData()) + s.Tell(), bUseWholeStream ? s.remainingSize() : mfSize, STREAM_READ);

                    filter.ImportGraphic (graphic, String (), mfStream);

                    // debug code - write the stream to debug file /tmp/emf-stream.emf
                    EMFP_DEBUG(
                        mfStream.Seek(0);
                        static int emfp_debug_stream_number = 0;
                        rtl::OUString emfp_debug_filename("/tmp/emf-embedded-stream");
                        emfp_debug_filename += rtl::OUString::valueOf(emfp_debug_stream_number++);
                        emfp_debug_filename += rtl::OUString(".emf");

                        SvFileStream file( emfp_debug_filename, STREAM_WRITE | STREAM_TRUNC );

                        mfStream >> file;
                        file.Flush();
                        file.Close()
                    );
                }
            }
        };

        struct EMFPFont : public EMFPObject
        {
            sal_uInt32 version;
            float emSize;
            sal_uInt32 sizeUnit;
            sal_Int32 fontFlags;
            rtl::OUString family;

            void Read (SvMemoryStream &s)
            {
                sal_uInt32 header;
                sal_uInt32 reserved;
                sal_uInt32 length;

                s >> header >> emSize >> sizeUnit >> fontFlags >> reserved >> length;

                OSL_ASSERT( ( header >> 12 ) == 0xdbc01 );

                EMFP_DEBUG (printf ("EMF+\tfont\nEMF+\theader: 0x%08x version: 0x%08x size: %f unit: 0x%08x\n",(unsigned int) header >> 12, (unsigned int)header & 0x1fff, emSize, (unsigned int)sizeUnit));
                EMFP_DEBUG (printf ("EMF+\tflags: 0x%08x reserved: 0x%08x length: 0x%08x\n", (unsigned int)fontFlags, (unsigned int)reserved, (unsigned int)length));

                if( length > 0 && length < 0x4000 ) {
                    sal_Unicode *chars = (sal_Unicode *) alloca( sizeof( sal_Unicode ) * length );

                    for( sal_uInt32 i = 0; i < length; i++ )
                    s >> chars[ i ];

                    family = ::rtl::OUString( chars, length );
                    EMFP_DEBUG (printf ("EMF+\tfamily: %s\n", rtl::OUStringToOString( family, RTL_TEXTENCODING_UTF8).getStr()));
                }
            }
        };

        void ImplRenderer::ReadRectangle (SvStream& s, float& x, float& y, float &width, float& height, bool bCompressed)
        {
            if (bCompressed) {
                sal_Int16 ix, iy, iw, ih;

                s >> ix >> iy >> iw >> ih;

                x = ix;
                y = iy;
                width = iw;
                height = ih;
            } else
                s >> x >> y >> width >> height;
        }

        void ImplRenderer::ReadPoint (SvStream& s, float& x, float& y, sal_uInt32 flags)
        {
            if (flags & 0x4000) {
                sal_Int16 ix, iy;

                s >> ix >> iy;

                x = ix;
                y = iy;
            } else
                s >> x >> y;
        }

        void ImplRenderer::MapToDevice (double& x, double& y)
        {
            // TODO: other untis
            x = 100*nMmX*x/nPixX;
            y = 100*nMmY*y/nPixY;
        }

        ::basegfx::B2DPoint ImplRenderer::Map (double ix, double iy)
        {
            double x, y;

            x = ix*aWorldTransform.eM11 + iy*aWorldTransform.eM21 + aWorldTransform.eDx;
            y = ix*aWorldTransform.eM12 + iy*aWorldTransform.eM22 + aWorldTransform.eDy;

            MapToDevice (x, y);

            x -= nFrameLeft;
            y -= nFrameTop;

            x *= aBaseTransform.eM11;
            y *= aBaseTransform.eM22;

            return ::basegfx::B2DPoint (x, y);
        }

        ::basegfx::B2DSize ImplRenderer::MapSize (double iwidth, double iheight)
        {
            double w, h;

            w = iwidth*aWorldTransform.eM11 + iheight*aWorldTransform.eM21;
            h = iwidth*aWorldTransform.eM12 + iheight*aWorldTransform.eM22;

            MapToDevice (w, h);

            w *= aBaseTransform.eM11;
            h *= aBaseTransform.eM22;

            return ::basegfx::B2DSize (w, h);
        }

#define COLOR(x) \
    ::vcl::unotools::colorToDoubleSequence( ::Color (0xff - (x >> 24), \
                             (x >> 16) & 0xff, \
                             (x >> 8) & 0xff, \
                             x & 0xff), \
                        rCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace());
#define SET_FILL_COLOR(x) \
    rState.fillColor = COLOR(x);
#define SET_TEXT_COLOR(x) \
    rState.textColor = COLOR(x);

        void ImplRenderer::EMFPPlusFillPolygon (::basegfx::B2DPolyPolygon& polygon, const ActionFactoryParameters& rParms,
                                                OutDevState& rState, const CanvasSharedPtr& rCanvas, bool isColor, sal_uInt32 brushIndexOrColor)
        {
            ::basegfx::B2DPolyPolygon localPolygon (polygon);

            EMFP_DEBUG (printf ("EMF+\tfill polygon\n"));

            localPolygon.transform( rState.mapModeTransform );

            ActionSharedPtr pPolyAction;

            if (isColor) {
                EMFP_DEBUG (printf ("EMF+\t\tcolor fill\n"));

                rState.isFillColorSet = true;
                rState.isLineColorSet = false;
                SET_FILL_COLOR(brushIndexOrColor);

                pPolyAction = ActionSharedPtr ( internal::PolyPolyActionFactory::createPolyPolyAction( localPolygon, rParms.mrCanvas, rState ) );

            } else {
                rState.isFillColorSet = true;
                // extract UseBrush
                EMFPBrush* brush = (EMFPBrush*) aObjects [brushIndexOrColor & 0xff];
                EMFP_DEBUG (printf ("EMF+\tbrush fill slot: %u (type: %u)\n", (unsigned int)brushIndexOrColor, (unsigned int)brush->GetType ()));

                // give up in case something wrong happened
                if( !brush )
                    return;

                rState.isFillColorSet = false;
                rState.isLineColorSet = false;

                if (brush->type == 3 || brush->type == 4) {

                    if (brush->type == 3 && !(brush->additionalFlags & 0x1))
                        return;  // we are unable to parse these brushes yet

                ::basegfx::B2DHomMatrix aTextureTransformation;
                ::basegfx::B2DHomMatrix aWorldTransformation;
                ::basegfx::B2DHomMatrix aBaseTransformation;
                rendering::Texture aTexture;

                aWorldTransformation.set (0, 0, aWorldTransform.eM11);
                aWorldTransformation.set (0, 1, aWorldTransform.eM21);
                aWorldTransformation.set (0, 2, aWorldTransform.eDx);
                aWorldTransformation.set (1, 0, aWorldTransform.eM12);
                aWorldTransformation.set (1, 1, aWorldTransform.eM22);
                aWorldTransformation.set (1, 2, aWorldTransform.eDy);

                aBaseTransformation.set (0, 0, aBaseTransform.eM11);
                aBaseTransformation.set (0, 1, aBaseTransform.eM21);
                aBaseTransformation.set (0, 2, aBaseTransform.eDx);
                aBaseTransformation.set (1, 0, aBaseTransform.eM12);
                aBaseTransformation.set (1, 1, aBaseTransform.eM22);
                aBaseTransformation.set (1, 2, aBaseTransform.eDy);

                if (brush->type == 4) {
                    aTextureTransformation.scale (brush->areaWidth, brush->areaHeight);
                    aTextureTransformation.translate (brush->areaX, brush->areaY);
                } else {
                    aTextureTransformation.translate (-0.5, -0.5);
                    aTextureTransformation.scale (brush->areaWidth, brush->areaHeight);
                    aTextureTransformation.translate (brush->areaX,brush->areaY);
                }

                if (brush->hasTransformation) {
                    ::basegfx::B2DHomMatrix aTransformation;

                    aTransformation.set (0, 0, brush->transformation.eM11);
                    aTransformation.set (0, 1, brush->transformation.eM21);
                    aTransformation.set (0, 2, brush->transformation.eDx);
                    aTransformation.set (1, 0, brush->transformation.eM12);
                    aTransformation.set (1, 1, brush->transformation.eM22);
                    aTransformation.set (1, 2, brush->transformation.eDy);

                    aTextureTransformation *= aTransformation;
                }

                aTextureTransformation *= aWorldTransformation;
                aTextureTransformation.scale (100.0*nMmX/nPixX, 100.0*nMmY/nPixY);
                aTextureTransformation.translate (-nFrameLeft, -nFrameTop);
                aTextureTransformation *= rState.mapModeTransform;
                aTextureTransformation *= aBaseTransformation;

                aTexture.RepeatModeX = rendering::TexturingMode::CLAMP;
                aTexture.RepeatModeY = rendering::TexturingMode::CLAMP;
                aTexture.Alpha = 1.0;

        basegfx::ODFGradientInfo aGradInfo;
        rtl::OUString aGradientService;

                    const uno::Sequence< double > aStartColor(
                        ::vcl::unotools::colorToDoubleSequence( brush->solidColor,
                                                                rParms.mrCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() ) );
                    const uno::Sequence< double > aEndColor(
                        ::vcl::unotools::colorToDoubleSequence( brush->secondColor,
                                                                rParms.mrCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() ) );
                    uno::Sequence< uno::Sequence < double > > aColors (2);
                    uno::Sequence< double > aStops (2);

                    if (brush->blendPositions) {
                        EMFP_DEBUG (printf ("EMF+\t\tuse blend\n"));
                        aColors.realloc (brush->blendPoints);
                        aStops.realloc (brush->blendPoints);
                        int length = aStartColor.getLength ();
                        uno::Sequence< double > aColor (length);

                        OSL_ASSERT (length == aEndColor.getLength());

                        for (int i = 0; i < brush->blendPoints; i++) {
                            aStops[i] = brush->blendPositions [i];

                            for (int j = 0; j < length; j++) {
                                if (brush->type == 4) {
//                                     // gamma correction
//                                     if (brush->additionalFlags & 0x80)
//                                         aColor [j] = pow (aStartColor [j]*(1 - brush->blendFactors[i]) + aEndColor [j]*brush->blendFactors[i], 1/2.2);
//                                     else
                                    aColor [j] = aStartColor [j]*(1 - brush->blendFactors[i]) + aEndColor [j]*brush->blendFactors[i];
                                } else
                                    aColor [j] = aStartColor [j]*brush->blendFactors[i] + aEndColor [j]*(1 - brush->blendFactors[i]);
                            }

                            aColors[i] = aColor;
                        }
                    } else if (brush->colorblendPositions) {
                        EMFP_DEBUG (printf ("EMF+\t\tuse color blend\n"));
                        aColors.realloc (brush->colorblendPoints);
                        aStops.realloc (brush->colorblendPoints);

                        for (int i = 0; i < brush->colorblendPoints; i++) {
                            aStops[i] = brush->colorblendPositions [i];
                            aColors[(brush->type == 4) ? i : brush->colorblendPoints - 1 - i] = ::vcl::unotools::colorToDoubleSequence( brush->colorblendColors [i],
                                                                                                                                        rParms.mrCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() );
                        }
                    } else {
                        aStops[0] = 0.0;
                        aStops[1] = 1.0;

                        if (brush->type == 4) {
                            aColors[0] = aStartColor;
                            aColors[1] = aEndColor;
                        } else {
                            aColors[1] = aStartColor;
                            aColors[0] = aEndColor;
                        }
                    }

                    EMFP_DEBUG (printf ("EMF+\t\tset gradient\n"));
           basegfx::B2DRange aBoundsRectangle (0, 0, 1, 1);
                    if (brush->type == 4) {
           aGradientService = "LinearGradient";
           basegfx::tools::createLinearODFGradientInfo( aGradInfo,
                                    aBoundsRectangle,
                                    aStops.getLength(),
                                    0,
                                    0 );

                    } else {
            aGradientService = "EllipticalGradient";
            basegfx::tools::createEllipticalODFGradientInfo( aGradInfo,
                                     aBoundsRectangle,
                                     ::basegfx::B2DVector( 0, 0 ),
                                     aStops.getLength(),
                                     0,
                                     0 );
                    }

            uno::Reference< lang::XMultiServiceFactory > xFactory(
            rParms.mrCanvas->getUNOCanvas()->getDevice()->getParametricPolyPolygonFactory() );

            if( xFactory.is() ) {
            uno::Sequence<uno::Any> args( 3 );
            beans::PropertyValue aProp;
            aProp.Name = "Colors";
            aProp.Value <<= aColors;
            args[0] <<= aProp;
            aProp.Name = "Stops";
            aProp.Value <<= aStops;
            args[1] <<= aProp;
            aProp.Name = "AspectRatio";
            aProp.Value <<= static_cast<sal_Int32>(1);
            args[2] <<= aProp;

            aTexture.Gradient.set(
                xFactory->createInstanceWithArguments( aGradientService,
                                   args ),
                uno::UNO_QUERY);
            }

            ::basegfx::unotools::affineMatrixFromHomMatrix( aTexture.AffineTransform,
                                    aTextureTransformation );

            if( aTexture.Gradient.is() )
            pPolyAction =
                ActionSharedPtr ( internal::PolyPolyActionFactory::createPolyPolyAction( localPolygon,
                                                     rParms.mrCanvas,
                                                     rState,
                                                     aTexture ) );
                }
            }

            if( pPolyAction )
            {
                EMFP_DEBUG (printf ("EMF+\t\tadd poly action\n"));

                maActions.push_back(
                    MtfAction(
                        pPolyAction,
                        rParms.mrCurrActionIndex ) );

                rParms.mrCurrActionIndex += pPolyAction->getActionCount()-1;
            }
        }

        void ImplRenderer::processObjectRecord(SvMemoryStream& rObjectStream, sal_uInt16 flags, sal_Bool bUseWholeStream)
        {
            sal_uInt32 index;

            EMFP_DEBUG (printf ("EMF+ Object slot: %hd flags: %hx\n", flags & 0xff, flags & 0xff00));

            index = flags & 0xff;
            if (aObjects [index] != NULL) {
                delete aObjects [index];
                aObjects [index] = NULL;
            }

            switch (flags & 0x7f00) {
            case EmfPlusObjectTypeBrush:
                {
                    EMFPBrush *brush;
                    aObjects [index] = brush = new EMFPBrush ();
                    brush->Read (rObjectStream, *this);

                    break;
                }
            case EmfPlusObjectTypePen:
                {
                    EMFPPen *pen;
                    aObjects [index] = pen = new EMFPPen ();
                    pen->Read (rObjectStream, *this, nHDPI, nVDPI);

                    break;
                }
            case EmfPlusObjectTypePath:
                sal_uInt32 header, pathFlags;
                sal_Int32 points;

                rObjectStream >> header >> points >> pathFlags;

                EMFP_DEBUG (printf ("EMF+\tpath\n"));
                EMFP_DEBUG (printf ("EMF+\theader: 0x%08x points: %d additional flags: 0x%08x\n", (unsigned int)header, (int)points, (unsigned int)pathFlags));

                EMFPPath *path;
                aObjects [index] = path = new EMFPPath (points);
                path->Read (rObjectStream, pathFlags, *this);

                break;
            case EmfPlusObjectTypeRegion: {
                EMFPRegion *region;

                aObjects [index] = region = new EMFPRegion ();
                region->Read (rObjectStream);

                break;
            }
            case EmfPlusObjectTypeImage:
                {
                    EMFPImage *image;
                    aObjects [index] = image = new EMFPImage ();
                    image->Read (rObjectStream, bUseWholeStream);

                    break;
                }
            case EmfPlusObjectTypeFont:
                {
                    EMFPFont *font;
                    aObjects [index] = font = new EMFPFont ();
                    font->Read (rObjectStream);

                    break;
                }
            default:
                EMFP_DEBUG (printf ("EMF+\tObject unhandled flags: 0x%04x\n", flags & 0xff00));
                break;
            }
        }

        double ImplRenderer::setFont (sal_uInt8 objectId, const ActionFactoryParameters& rParms, OutDevState& rState)
        {
            EMFPFont *font = (EMFPFont*) aObjects[ objectId ];

            rendering::FontRequest aFontRequest;
            aFontRequest.FontDescription.FamilyName = font->family;
            double cellSize = font->emSize;
            aFontRequest.CellSize = (rState.mapModeTransform*MapSize( cellSize, 0 )).getX();
            rState.xFont = rParms.mrCanvas->getUNOCanvas()->createFont( aFontRequest,
                                               uno::Sequence< beans::PropertyValue >(),
                                               geometry::Matrix2D() );

            return cellSize;
        }

        void ImplRenderer::processEMFPlus( MetaCommentAction* pAct, const ActionFactoryParameters& rFactoryParms,
                                           OutDevState& rState, const CanvasSharedPtr& rCanvas )
        {
            sal_uInt32 length = pAct->GetDataSize ();
            SvMemoryStream rMF ((void*) pAct->GetData (), length, STREAM_READ);

            length -= 4;

            while (length > 0) {
                sal_uInt16 type, flags;
                sal_uInt32 size, dataSize;
                sal_uInt32 next;

                rMF >> type >> flags >> size >> dataSize;

                next = rMF.Tell() + ( size - 12 );

                EMFP_DEBUG (printf ("EMF+ record size: %u type: %04hx flags: %04hx data size: %u\n", (unsigned int)size, type, flags, (unsigned int)dataSize));

                if (type == EmfPlusRecordTypeObject && ((mbMultipart && (flags & 0x7fff) == (mMFlags & 0x7fff)) || (flags & 0x8000))) {
                    if (!mbMultipart) {
                        mbMultipart = true;
                        mMFlags = flags;
                        mMStream.Seek(0);
                    }

                    // 1st 4 bytes are unknown
                    mMStream.Write (((const char *)rMF.GetData()) + rMF.Tell() + 4, dataSize - 4);
                    EMFP_DEBUG (printf ("EMF+ read next object part size: %u type: %04hx flags: %04hx data size: %u\n", (unsigned int)size, type, flags, (unsigned int)dataSize));
                } else {
                    if (mbMultipart) {
                        EMFP_DEBUG (printf ("EMF+ multipart record flags: %04hx\n", mMFlags));
                        mMStream.Seek (0);
                        processObjectRecord (mMStream, mMFlags, sal_True);
                    }
                    mbMultipart = false;
                }

                if (type != EmfPlusRecordTypeObject || !(flags & 0x8000))
                switch (type) {
                case EmfPlusRecordTypeHeader:
                    sal_uInt32 header, version;

                    rMF >> header >> version >> nHDPI >> nVDPI;

                    EMFP_DEBUG (printf ("EMF+ Header\n"));
                    EMFP_DEBUG (printf ("EMF+\theader: 0x%08x version: %u horizontal DPI: %d vertical DPI: %d dual: %d\n", (int)header, (unsigned int)version, (int)nHDPI, (int)nVDPI,(int)( flags & 1)));

                    break;
                case EmfPlusRecordTypeEndOfFile:
                    EMFP_DEBUG (printf ("EMF+ EndOfFile\n"));
                    break;
                case EmfPlusRecordTypeGetDC:
                    EMFP_DEBUG (printf ("EMF+ GetDC\n"));
                    EMFP_DEBUG (printf ("EMF+\talready used in svtools wmf/emf filter parser\n"));
                    break;
                case EmfPlusRecordTypeObject:
                    processObjectRecord (rMF, flags);
                    break;
                case EmfPlusRecordTypeFillPath:
                    {
                        sal_uInt32 index = flags & 0xff;
                        sal_uInt32 brushIndexOrColor;

                        rMF >> brushIndexOrColor;

                        EMFP_DEBUG (printf ("EMF+ FillPath slot: %u\n", (unsigned int)index));

                        EMFPPlusFillPolygon (((EMFPPath*) aObjects [index])->GetPolygon (*this), rFactoryParms, rState, rCanvas, flags & 0x8000, brushIndexOrColor);
                    }
                    break;
                case EmfPlusRecordTypeFillRects:
                    {
                        EMFP_DEBUG (printf ("EMF+ FillRects\n"));

                        sal_uInt32 brushIndexOrColor;
                        sal_Int32 rectangles;
                        ::basegfx::B2DPolygon polygon;

                        rMF >> brushIndexOrColor >> rectangles;

                        EMFP_DEBUG (printf ("EMF+\t%s: 0x%08x\n", (flags & 0x8000) ? "color" : "brush index", (unsigned int)brushIndexOrColor));

                        for (int i=0; i < rectangles; i++) {
                            if (flags & 0x4000) {
                                /* 16bit integers */
                                sal_Int16 x, y, width, height;

                                rMF >> x >> y >> width >> height;

                                polygon.append (Map (x, y));
                                polygon.append (Map (x + width, y));
                                polygon.append (Map (x + width, y + height));
                                polygon.append (Map (x, y + height));

                                EMFP_DEBUG (printf ("EMF+\trectangle: %d,%d %dx%d\n", x, y, width, height));
                            } else {
                                /* Single's */
                                float x, y, width, height;

                                rMF >> x >> y >> width >> height;

                                polygon.append (Map (x, y));
                                polygon.append (Map (x + width, y));
                                polygon.append (Map (x + width, y + height));
                                polygon.append (Map (x, y + height));

                                EMFP_DEBUG (printf ("EMF+\trectangle: %f,%f %fx%f\n", x, y, width, height));
                            }

                            ::basegfx::B2DPolyPolygon polyPolygon (polygon);

                            EMFPPlusFillPolygon (polyPolygon, rFactoryParms, rState, rCanvas, flags & 0x8000, brushIndexOrColor);
                        }
                        break;
                    }
                case EmfPlusRecordTypeFillPolygon:
                    {
                        EMFP_DEBUG (sal_uInt8 index = flags & 0xff);
                        sal_uInt32 brushIndexOrColor;
                        sal_Int32 points;

                        rMF >> brushIndexOrColor;
                        rMF >> points;

                        EMFP_DEBUG (printf ("EMF+ FillPolygon in slot: %d points: %d\n", index, points));
                        EMFP_DEBUG (printf ("EMF+\t%s: 0x%08x\n", (flags & 0x8000) ? "color" : "brush index", brushIndexOrColor));

                        EMFPPath path (points, true);
                        path.Read (rMF, flags, *this);


                        EMFPPlusFillPolygon (path.GetPolygon (*this), rFactoryParms, rState, rCanvas, flags & 0x8000, brushIndexOrColor);

                        break;
                    }
                case EmfPlusRecordTypeDrawLines:
                    {
                        sal_uInt32 index = flags & 0xff;
                        sal_uInt32 points;

                        rMF >> points;

                        EMFP_DEBUG (printf ("EMF+ DrawLines in slot: %u points: %u\n", (unsigned int)index, (unsigned int)points));

                        EMFPPath path (points, true);
                        path.Read (rMF, flags, *this);

                        EMFPPen* pen = (EMFPPen*) aObjects [index];

                        rState.isFillColorSet = false;
                        rState.isLineColorSet = true;
                        rState.lineColor = ::vcl::unotools::colorToDoubleSequence (pen->GetColor (),
                                                                                   rCanvas->getUNOCanvas ()->getDevice()->getDeviceColorSpace() );
                        ::basegfx::B2DPolyPolygon& polygon (path.GetPolygon (*this));

                        polygon.transform( rState.mapModeTransform );

                        rendering::StrokeAttributes aStrokeAttributes;

                        pen->SetStrokeAttributes (aStrokeAttributes, *this, rState);

            ActionSharedPtr pPolyAction(
                internal::PolyPolyActionFactory::createPolyPolyAction(
                    polygon, rFactoryParms.mrCanvas, rState, aStrokeAttributes ) );

            if( pPolyAction )
            {
                maActions.push_back(
                    MtfAction(
                        pPolyAction,
                        rFactoryParms.mrCurrActionIndex ) );

                rFactoryParms.mrCurrActionIndex += pPolyAction->getActionCount()-1;
            }

                        break;
                    }
                case EmfPlusRecordTypeDrawPath:
                    {
                        sal_uInt32 penIndex;

                        rMF >> penIndex;

                        EMFP_DEBUG (printf ("EMF+ DrawPath\n"));
                        EMFP_DEBUG (printf ("EMF+\tpen: %u\n", (unsigned int)penIndex));

                        EMFPPath* path = (EMFPPath*) aObjects [flags & 0xff];
                        EMFPPen* pen = (EMFPPen*) aObjects [penIndex & 0xff];

                        SAL_WARN_IF( !pen, "cppcanvas", "EmfPlusRecordTypeDrawPath missing pen" );
                        SAL_WARN_IF( !path, "cppcanvas", "EmfPlusRecordTypeDrawPath missing path" );

                        if (pen && path)
                        {
                            rState.isFillColorSet = false;
                            rState.isLineColorSet = true;
                            rState.lineColor = ::vcl::unotools::colorToDoubleSequence (pen->GetColor (),
                                                                                       rCanvas->getUNOCanvas ()->getDevice()->getDeviceColorSpace());
                            ::basegfx::B2DPolyPolygon& polygon (path->GetPolygon (*this));

                            polygon.transform( rState.mapModeTransform );
                            rendering::StrokeAttributes aStrokeAttributes;

                            pen->SetStrokeAttributes (aStrokeAttributes, *this, rState);

                            ActionSharedPtr pPolyAction(
                                internal::PolyPolyActionFactory::createPolyPolyAction(
                                    polygon, rFactoryParms.mrCanvas, rState, aStrokeAttributes ) );

                            if( pPolyAction )
                            {
                                maActions.push_back(
                                    MtfAction(
                                        pPolyAction,
                                        rFactoryParms.mrCurrActionIndex ) );

                                rFactoryParms.mrCurrActionIndex += pPolyAction->getActionCount()-1;
                            }
                        }
                        break;
                    }
                case EmfPlusRecordTypeDrawImage:
                case EmfPlusRecordTypeDrawImagePoints:
                    {
                        sal_uInt32 attrIndex;
                        sal_Int32 sourceUnit;

                        rMF >> attrIndex >> sourceUnit;

                        EMFP_DEBUG (printf ("EMF+ %s attributes index: %d source unit: %d\n", type == EmfPlusRecordTypeDrawImagePoints ? "DrawImagePoints" : "DrawImage", (int)attrIndex, (int)sourceUnit));
                        EMFP_DEBUG (printf ("EMF+\tTODO: use image attributes\n"));

                        if (sourceUnit == 2 && aObjects [flags & 0xff]) { // we handle only GraphicsUnit.Pixel now
                            EMFPImage& image = *(EMFPImage *) aObjects [flags & 0xff];
                            float sx, sy, sw, sh;
                            sal_Int32 aCount;

                            ReadRectangle (rMF, sx, sy, sw, sh);

                            EMFP_DEBUG (printf ("EMF+ %s source rectangle: %f,%f %fx%f\n", type == EmfPlusRecordTypeDrawImagePoints ? "DrawImagePoints" : "DrawImage", sx, sy, sw, sh));

                            ::basegfx::B2DPoint aDstPoint;
                            ::basegfx::B2DSize aDstSize;
                            bool bValid = false;

                            if (type == EmfPlusRecordTypeDrawImagePoints) {
                                rMF >> aCount;

                                if( aCount == 3) { // TODO: now that we now that this value is count we should support it better
                                    float x1, y1, x2, y2, x3, y3;

                                    ReadPoint (rMF, x1, y1, flags);
                                    ReadPoint (rMF, x2, y2, flags);
                                    ReadPoint (rMF, x3, y3, flags);

                                    aDstPoint = Map (x1, y1);
                                    aDstSize = MapSize(x2 - x1, y3 - y1);

                                    bValid = true;
                                }
                            } else if (type == EmfPlusRecordTypeDrawImage) {
                                float dx, dy, dw, dh;

                                ReadRectangle (rMF, dx, dy, dw, dh, flags & 0x4000);

                                aDstPoint = Map (dx, dy);
                                aDstSize = MapSize(dw, dh);

                                bValid = true;
                            }

                            if (bValid) {
                                BitmapEx aBmp( image.graphic.GetBitmapEx () );
                                const Rectangle aCropRect (::vcl::unotools::pointFromB2DPoint (basegfx::B2DPoint (sx, sy)),
                                                           ::vcl::unotools::sizeFromB2DSize (basegfx::B2DSize(sw, sh)));
                                aBmp.Crop( aCropRect );


                                Size aSize( aBmp.GetSizePixel() );
                                if( aSize.Width() > 0 && aSize.Height() > 0 ) {
                                    ActionSharedPtr pBmpAction (
                                        internal::BitmapActionFactory::createBitmapAction (
                                            aBmp,
                                            rState.mapModeTransform * aDstPoint,
                                            rState.mapModeTransform * aDstSize,
                                            rCanvas,
                                            rState));

                                    if( pBmpAction ) {
                                        maActions.push_back( MtfAction( pBmpAction,
                                                                        rFactoryParms.mrCurrActionIndex ) );

                                        rFactoryParms.mrCurrActionIndex += pBmpAction->getActionCount()-1;
                                    }
                                } else {
                                    EMFP_DEBUG (printf ("EMF+ warning: empty bitmap\n"));
                                }
                            } else {
                                EMFP_DEBUG (printf ("EMF+ DrawImage(Points) TODO (fixme)\n"));
                            }
                        } else {
                            EMFP_DEBUG (printf ("EMF+ DrawImage(Points) TODO (fixme) - possibly unsupported source units for crop rectangle\n"));
                        }
                        break;
                    }
                case EmfPlusRecordTypeDrawString:
                    {
                        EMFP_DEBUG (printf ("EMF+ DrawString\n"));

                        sal_uInt32 brushId;
                        sal_uInt32 formatId;
                        sal_uInt32 stringLength;

                        rMF >> brushId >> formatId >> stringLength;
                        EMFP_DEBUG (printf ("EMF+ DrawString brushId: %x formatId: %x length: %x\n", brushId, formatId, stringLength));

                        if (flags & 0x8000) {
                            float lx, ly, lw, lh;

                            rMF >> lx >> ly >> lw >> lh;

                            EMFP_DEBUG (printf ("EMF+ DrawString layoutRect: %f,%f - %fx%f\n", lx, ly, lw, lh));

                            rtl::OUString text = read_uInt16s_ToOUString(rMF, stringLength);

                            double cellSize = setFont (flags & 0xff, rFactoryParms, rState);
                            SET_TEXT_COLOR( brushId );

                            ActionSharedPtr pTextAction(
                                TextActionFactory::createTextAction(
                                                                    // position is just rough guess for now
                                                                    // we should calculate it exactly from layoutRect or font
                                    ::vcl::unotools::pointFromB2DPoint ( Map( lx + 0.15*cellSize, ly + cellSize ) ),
                                    ::Size(),
                                    ::Color(),
                                    ::Size(),
                                    ::Color(),
                                    text,
                                    0,
                                    stringLength,
                                    NULL,
                                    rFactoryParms.mrVDev,
                                    rFactoryParms.mrCanvas,
                                    rState,
                                    rFactoryParms.mrParms,
                                    false ) );
                            if( pTextAction )
                            {
                                EMFP_DEBUG (printf ("EMF+\t\tadd text action\n"));

                                maActions.push_back(
                                                    MtfAction(
                                                              pTextAction,
                                                              rFactoryParms.mrCurrActionIndex ) );

                                rFactoryParms.mrCurrActionIndex += pTextAction->getActionCount()-1;
                            }
                        } else {
                            EMFP_DEBUG (printf ("EMF+ DrawString TODO - drawing with brush not yet supported\n"));
                        }
                    }
                    break;
                case EmfPlusRecordTypeSetPageTransform:
                    rMF >> fPageScale;

                    EMFP_DEBUG (printf ("EMF+ SetPageTransform\n"));
                    EMFP_DEBUG (printf ("EMF+\tscale: %f unit: %d\n", fPageScale, flags));
                    EMFP_DEBUG (printf ("EMF+\tTODO\n"));
                    break;
                case EmfPlusRecordTypeSetRenderingOrigin:
                    rMF >> nOriginX >> nOriginY;
                    EMFP_DEBUG (printf ("EMF+ SetRenderingOrigin\n"));
                    EMFP_DEBUG (printf ("EMF+\torigin [x,y]: %d,%d\n", (int)nOriginX, (int)nOriginY));
                    break;
                case EmfPlusRecordTypeSetTextRenderingHint:
                    EMFP_DEBUG (printf ("EMF+ SetTextRenderingHint\n"));
                    EMFP_DEBUG (printf ("EMF+\tTODO\n"));
                    break;
                case EmfPlusRecordTypeSetAntiAliasMode:
                    EMFP_DEBUG (printf ("EMF+ SetAntiAliasMode\n"));
                    EMFP_DEBUG (printf ("EMF+\tTODO\n"));
                    break;
                case EmfPlusRecordTypeSetInterpolationMode:
                    EMFP_DEBUG (printf ("EMF+ InterpolationMode\n"));
                    EMFP_DEBUG (printf ("EMF+\tTODO\n"));
                    break;
                case EmfPlusRecordTypeSetPixelOffsetMode:
                    EMFP_DEBUG (printf ("EMF+ SetPixelOffsetMode\n"));
                    EMFP_DEBUG (printf ("EMF+\tTODO\n"));
                    break;
                case EmfPlusRecordTypeSetCompositingQuality:
                    EMFP_DEBUG (printf ("EMF+ SetCompositingQuality\n"));
                    EMFP_DEBUG (printf ("EMF+\tTODO\n"));
                    break;
                case EmfPlusRecordTypeSave:
                    EMFP_DEBUG (printf ("EMF+ Save\n"));
                    EMFP_DEBUG (printf ("EMF+\tTODO\n"));
                    break;
                case EmfPlusRecordTypeSetWorldTransform: {
                    EMFP_DEBUG (printf ("EMF+ SetWorldTransform\n"));
                    XForm transform;
                    rMF >> transform;
                    aWorldTransform.Set (transform);
                    EMFP_DEBUG (printf ("EMF+\tm11: %f m12: %f\nEMF+\tm21: %f m22: %f\nEMF+\tdx: %f dy: %f\n",
                            aWorldTransform.eM11, aWorldTransform.eM12,
                            aWorldTransform.eM21, aWorldTransform.eM22,
                            aWorldTransform.eDx, aWorldTransform.eDy));
                    break;
                }
                case EmfPlusRecordTypeResetWorldTransform:
                    EMFP_DEBUG (printf ("EMF+ ResetWorldTransform\n"));
                    aWorldTransform.SetIdentity ();
                    break;
                case EmfPlusRecordTypeMultiplyWorldTransform: {
                    EMFP_DEBUG (printf ("EMF+ MultiplyWorldTransform\n"));
                    XForm transform;
                    rMF >> transform;

                    EMFP_DEBUG (printf ("EMF+\tmatrix m11: %f m12: %f\nEMF+\tm21: %f m22: %f\nEMF+\tdx: %f dy: %f\n",
                            transform.eM11, transform.eM12,
                            transform.eM21, transform.eM22,
                            transform.eDx, transform.eDy));

                    if (flags & 0x2000)  // post multiply
                        aWorldTransform.Multiply (transform);
                    else {               // pre multiply
                        transform.Multiply (aWorldTransform);
                        aWorldTransform.Set (transform);
                    }
                    EMFP_DEBUG (printf ("EMF+\tresult world matrix m11: %f m12: %f\nEMF+\tm21: %f m22: %f\nEMF+\tdx: %f dy: %f\n",
                            aWorldTransform.eM11, aWorldTransform.eM12,
                            aWorldTransform.eM21, aWorldTransform.eM22,
                            aWorldTransform.eDx, aWorldTransform.eDy));
                    break;
                }
                case EmfPlusRecordTypeSetClipPath:
                    {
                        EMFP_DEBUG (printf ("EMF+ SetClipPath\n"));
                        EMFP_DEBUG (printf ("EMF+\tpath in slot: %d\n", flags & 0xff));

                        EMFPPath& path = *(EMFPPath*) aObjects [flags & 0xff];
                        ::basegfx::B2DPolyPolygon& clipPoly (path.GetPolygon (*this));

                        clipPoly.transform (rState.mapModeTransform);
                        updateClipping (clipPoly, rFactoryParms, false);

                        break;
                    }
                case EmfPlusRecordTypeSetClipRegion: {
                    EMFP_DEBUG (printf ("EMF+ SetClipRegion\n"));
                    EMFP_DEBUG (printf ("EMF+\tregion in slot: %d combine mode: %d\n", flags & 0xff, (flags & 0xff00) >> 8));
                    EMFPRegion *region = (EMFPRegion*)aObjects [flags & 0xff];

                    // reset clip
                    if (region && region->parts == 0 && region->initialState == EmfPlusRegionInitialStateInfinite) {
                        updateClipping (::basegfx::B2DPolyPolygon (), rFactoryParms, false);
                    } else {
                        EMFP_DEBUG (printf ("EMF+\tTODO\n"));
            }
                    break;
                }
            case EmfPlusRecordTypeDrawDriverString: {
                    EMFP_DEBUG (printf ("EMF+ DrawDriverString, flags: 0x%04x\n", flags));
            sal_uInt32 brushIndexOrColor;
            sal_uInt32 optionFlags;
            sal_uInt32 hasMatrix;
            sal_uInt32 glyphsCount;

            rMF >> brushIndexOrColor >> optionFlags >> hasMatrix >> glyphsCount;

            EMFP_DEBUG (printf ("EMF+\t%s: 0x%08x\n", (flags & 0x8000) ? "color" : "brush index", (unsigned int)brushIndexOrColor));
            EMFP_DEBUG (printf ("EMF+\toption flags: 0x%08x\n", (unsigned int)optionFlags));
            EMFP_DEBUG (printf ("EMF+\thas matrix: %u\n", (unsigned int)hasMatrix));
            EMFP_DEBUG (printf ("EMF+\tglyphs: %u\n", (unsigned int)glyphsCount));

            if( ( optionFlags & 1 ) && glyphsCount > 0 ) {
            float *charsPosX = new float[glyphsCount];
            float *charsPosY = new float[glyphsCount];

            rtl::OUString text = read_uInt16s_ToOUString(rMF, glyphsCount);

            for( sal_uInt32 i=0; i<glyphsCount; i++) {
                rMF >> charsPosX[i] >> charsPosY[i];
                EMFP_DEBUG (printf ("EMF+\tglyphPosition[%u]: %f, %f\n", (unsigned int)i, charsPosX[i], charsPosY[i]));
            }

            XForm transform;
            if( hasMatrix ) {
                rMF >> transform;
                EMFP_DEBUG (printf ("EMF+\tmatrix:: %f, %f, %f, %f, %f, %f\n", transform.eM11, transform.eM12, transform.eM21, transform.eM22, transform.eDx, transform.eDy));
            }

            // add the text action
            setFont (flags & 0xff, rFactoryParms, rState);

            if( flags & 0x8000 )
                SET_TEXT_COLOR(brushIndexOrColor);

            ActionSharedPtr pTextAction(
                TextActionFactory::createTextAction(
                ::vcl::unotools::pointFromB2DPoint ( Map( charsPosX[0], charsPosY[0] ) ),
                ::Size(),
                ::Color(),
                ::Size(),
                ::Color(),
                text,
                0,
                glyphsCount,
                NULL,
                rFactoryParms.mrVDev,
                rFactoryParms.mrCanvas,
                rState,
                rFactoryParms.mrParms,
                false ) );

            if( pTextAction )
            {
                EMFP_DEBUG (printf ("EMF+\t\tadd text action\n"));

                maActions.push_back(
                MtfAction(
                    pTextAction,
                    rFactoryParms.mrCurrActionIndex ) );

                rFactoryParms.mrCurrActionIndex += pTextAction->getActionCount()-1;
            }

            delete[] charsPosX;
            delete[] charsPosY;
            } else {
            EMFP_DEBUG (printf ("EMF+\tTODO: fonts (non-unicode glyphs chars)\n"));
            }

                    break;
        }
                default:
                    EMFP_DEBUG (printf ("EMF+ unhandled record type: %d\n", type));
                    EMFP_DEBUG (printf ("EMF+\tTODO\n"));
                }

                rMF.Seek (next);

                length -= size;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
