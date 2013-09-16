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

#include <tools/stream.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graphicfilter.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/tools/gradienttools.hxx>
#include <basegfx/tools/tools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
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
#define EmfPlusRecordTypeFillEllipse 16398
#define EmfPlusRecordTypeDrawEllipse 16399
#define EmfPlusRecordTypeFillPie 16400
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
#define EmfPlusRecordTypeRestore 16422
#define EmfPlusRecordTypeBeginContainerNoParams 16424
#define EmfPlusRecordTypeEndContainer 16425
#define EmfPlusRecordTypeSetWorldTransform 16426
#define EmfPlusRecordTypeResetWorldTransform 16427
#define EmfPlusRecordTypeMultiplyWorldTransform 16428
#define EmfPlusRecordTypeSetPageTransform 16432
#define EmfPlusRecordTypeSetClipRect 16434
#define EmfPlusRecordTypeSetClipPath 16435
#define EmfPlusRecordTypeSetClipRegion 16436
#define EmfPlusRecordTypeDrawDriverString 16438

#define EmfPlusObjectTypeBrush 0x100
#define EmfPlusObjectTypePen 0x200
#define EmfPlusObjectTypePath 0x300
#define EmfPlusObjectTypeRegion 0x400
#define EmfPlusObjectTypeImage 0x500
#define EmfPlusObjectTypeFont 0x600

#define EmfPlusRegionInitialStateInfinite 0x10000003

using namespace ::com::sun::star;
using namespace ::basegfx;

namespace cppcanvas
{
    namespace internal
    {

#if OSL_DEBUG_LEVEL > 1
        void dumpWords (SvStream& s, int i)
        {
            sal_uInt32 pos = s.Tell ();
            sal_Int16 data;
            SAL_INFO ("cppcanvas.emf", "EMF+ dumping words");
            for (; i > 0; i --) {
                s >> data;
                SAL_INFO ("cppcanvas.emf", "EMF+\tdata: " << std::hex << data << std::dec);
            }
            SAL_INFO ("cppcanvas.emf", "EMF+ end dumping words");
            s.Seek (pos);
        }
#endif

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
                        SAL_INFO ("cppcanvas.emf", "EMF+\tpoint [x,y]: " << x << "," << y);
                        pPoints [i*2] = x;
                        pPoints [i*2 + 1] = y;
                    } else {
                        // points are stored in Single (float) format
                        s >> pPoints [i*2] >> pPoints [i*2 + 1];
                        SAL_INFO ("cppcanvas.emf", "EMF+\tpoint [x,y]: " << pPoints [i*2] << "," << pPoints [i*2 + 1]);
                    }
                }

                if (pPointTypes)
                    for (int i = 0; i < nPoints; i ++) {
                        s >> pPointTypes [i];
                        SAL_INFO ("cppcanvas.emf", "EMF+\tpoint type: " << (int)pPointTypes [i]);
                    }

                aPolygon.clear ();

#if OSL_DEBUG_LEVEL > 1
                const ::basegfx::B2DRectangle aBounds (::basegfx::tools::getRange (GetPolygon (rR)));

                SAL_INFO ("cppcanvas.emf",
                          "EMF+\tpolygon bounding box: " << aBounds.getMinX () << "," << aBounds.getMinY () << aBounds.getWidth () << "x" << aBounds.getHeight () << " (mapped)");
#else
                (void) rR; // avoid warnings
#endif
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
                    if (pPointTypes) {
                        if ((pPointTypes [i] & 0x07) == 3) {
                            if (((i - last_normal )% 3) == 1) {
                                polygon.setNextControlPoint (p - 1, mapped);
                                SAL_INFO ("cppcanvas.emf", "polygon append  next: " << p - 1 << " mapped: " << mapped.getX () << "," << mapped.getY ());
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
                    SAL_INFO ("cppcanvas.emf", "polygon append point: " << pPoints [i*2] << "," << pPoints [i*2 + 1] << " mapped: " << mapped.getX () << ":" << mapped.getY ());
                    if (hasPrev) {
                        polygon.setPrevControlPoint (p, prev);
                        SAL_INFO ("cppcanvas.emf", "polygon append  prev: " << p << " mapped: " << prev.getX () << "," << prev.getY ());
                        hasPrev = false;
                    }
                    p ++;
                    if (pPointTypes && (pPointTypes [i] & 0x80)) { // closed polygon
                        polygon.setClosed (true);
                        aPolygon.append (polygon);
                        SAL_INFO ("cppcanvas.emf", "close polygon");
                        last_normal = i + 1;
                        p = 0;
                        polygon.clear ();
                    }
                }

                if (polygon.count ()) {
                    aPolygon.append (polygon);

#if OSL_DEBUG_LEVEL > 1
                    for (unsigned int i=0; i<aPolygon.count(); i++) {
                        polygon = aPolygon.getB2DPolygon(i);
                        SAL_INFO ("cppcanvas.emf", "polygon: " << i);
                        for (unsigned int j=0; j<polygon.count(); j++) {
                            ::basegfx::B2DPoint point = polygon.getB2DPoint(j);
                            SAL_INFO ("cppcanvas.emf", "point: " << point.getX() << "," << point.getY());
                            if (polygon.isPrevControlPointUsed(j)) {
                                point = polygon.getPrevControlPoint(j);
                                SAL_INFO ("cppcanvas.emf", "prev: " << point.getX() << "," << point.getY());
                            }
                            if (polygon.isNextControlPointUsed(j)) {
                                point = polygon.getNextControlPoint(j);
                                SAL_INFO ("cppcanvas.emf", "next: " << point.getX() << "," << point.getY());
                            }
                        }
                    }
#endif
                }

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

                SAL_INFO ("cppcanvas.emf", "EMF+\tregion");
                SAL_INFO ("cppcanvas.emf", "EMF+\theader: 0x" << std::hex << header << " parts: " << parts << std::dec );

                if (parts) {
                    if( parts<0 || sal_uInt32(parts)>SAL_MAX_INT32/sizeof(sal_Int32) )
                        parts = SAL_MAX_INT32/sizeof(sal_Int32);

                    combineMode = new sal_Int32 [parts];

                    for (int i = 0; i < parts; i ++) {
                        s >> combineMode [i];
                        SAL_INFO ("cppcanvas.emf", "EMF+\tcombine mode [" << i << "]: 0x" << std::hex << combineMode [i] << std::dec);
                    }
                }

                s >> initialState;
                SAL_INFO ("cppcanvas.emf", "EMF+\tinitial state: 0x" << std::hex << initialState << std::dec);
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

                SAL_INFO ("cppcanvas.emf", "EMF+\tbrush");
                SAL_INFO ("cppcanvas.emf", "EMF+\theader: 0x" << std::hex << header << " type: " << type << std::dec);

                switch (type) {
                case 0:
                    {
                        sal_uInt32 color;

                        s >> color;
                        solidColor = ::Color (0xff - (color >> 24), (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
                        SAL_INFO ("cppcanvas.emf", "EMF+\tsolid color: 0x" << std::hex << color << std::dec);

                        break;
                    }
                // path gradient
                case 3:
                    {
                        s >> additionalFlags >> wrapMode;

                        SAL_INFO ("cppcanvas.emf", "EMF+\tpath gradient, additional flags: 0x" << std::hex << additionalFlags << std::dec);

                        sal_uInt32 color;

                        s >> color;
                        solidColor = ::Color (0xff - (color >> 24), (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
                        SAL_INFO("cppcanvas.emf", "EMF+\tcenter color: 0x" << std::hex << color << std::dec);

                        s >> areaX >> areaY;
                        SAL_INFO("cppcanvas.emf", "EMF+\tcenter point: " << areaX << "," << areaY);

                        s >> surroundColorsNumber;
                        SAL_INFO("cppcanvas.emf", "EMF+\tsurround colors: " << surroundColorsNumber);

                        if( surroundColorsNumber<0 || sal_uInt32(surroundColorsNumber)>SAL_MAX_INT32/sizeof(::Color) )
                            surroundColorsNumber = SAL_MAX_INT32/sizeof(::Color);

                        surroundColors = new ::Color [surroundColorsNumber];
                        for (int i = 0; i < surroundColorsNumber; i++) {
                            s >> color;
                            surroundColors[i] = ::Color (0xff - (color >> 24), (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
                            if (i == 0)
                                secondColor = surroundColors [0];
                            SAL_INFO("cppcanvas.emf", "EMF+\tsurround color[" << i << "]: 0x" << std::hex << color << std::dec);
                        }

                        if (additionalFlags & 0x01) {
                            sal_Int32 pathLength;

                            s >> pathLength;
                            SAL_INFO("cppcanvas.emf", "EMF+\tpath length: " << pathLength);

                            sal_uInt32 pos = s.Tell ();
#if OSL_DEBUG_LEVEL > 1
                            dumpWords (s, 32);
#endif

                            sal_uInt32 pathHeader;
                            sal_Int32 pathPoints, pathFlags;
                            s >> pathHeader >> pathPoints >> pathFlags;

                            SAL_INFO("cppcanvas.emf", "EMF+\tpath (brush path gradient)");
                            SAL_INFO("cppcanvas.emf", "EMF+\theader: 0x" << std::hex << pathHeader << " points: " << std::dec << pathPoints << " additional flags: 0x" << std::hex << pathFlags << std::dec );

                            path = new EMFPPath (pathPoints);
                            path->Read (s, pathFlags, rR);

                            s.Seek (pos + pathLength);

                            const ::basegfx::B2DRectangle aBounds (::basegfx::tools::getRange (path->GetPolygon (rR, false)));
                            areaWidth = aBounds.getWidth ();
                            areaHeight = aBounds.getHeight ();

                            SAL_INFO("cppcanvas.emf", "EMF+\tpolygon bounding box: " << aBounds.getMinX () << "," << aBounds.getMinY () << " " << aBounds.getWidth () << "x" << aBounds.getHeight ());


                        if (additionalFlags & 0x02) {
                            SAL_INFO("cppcanvas.emf", "EMF+\tuse transformation");
                            s >> transformation;
                            hasTransformation = true;
                            SAL_INFO("cppcanvas.emf",
                                    "EMF+\tm11: "   << transformation.eM11 << " m12: " << transformation.eM12 <<
                                    "\nEMF+\tm21: " << transformation.eM21 << " m22: " << transformation.eM22 <<
                                    "\nEMF+\tdx: "  << transformation.eDx  << " dy: "  << transformation.eDy);

                        }
                        if (additionalFlags & 0x08) {
                            s >> blendPoints;
                            SAL_INFO("cppcanvas.emf", "EMF+\tuse blend, points: " << blendPoints);
                            if( blendPoints<0 || sal_uInt32(blendPoints)>SAL_MAX_INT32/(2*sizeof(float)) )
                                blendPoints = SAL_MAX_INT32/(2*sizeof(float));
                            blendPositions = new float [2*blendPoints];
                            blendFactors = blendPositions + blendPoints;
                            for (int i=0; i < blendPoints; i ++) {
                                s >> blendPositions [i];
                                SAL_INFO("cppcanvas.emf", "EMF+\tposition[" << i << "]: " << blendPositions [i]);
                            }
                            for (int i=0; i < blendPoints; i ++) {
                                s >> blendFactors [i];
                                SAL_INFO("cppcanvas.emf", "EMF+\tfactor[" << i << "]: " << blendFactors [i]);
                            }
                        }

                        if (additionalFlags & 0x04) {
                            s >> colorblendPoints;
                            SAL_INFO("cppcanvas.emf", "EMF+\tuse color blend, points: " << colorblendPoints);
                            if( colorblendPoints<0 || sal_uInt32(colorblendPoints)>SAL_MAX_INT32/sizeof(float) )
                                colorblendPoints = SAL_MAX_INT32/sizeof(float);
                            if( sal_uInt32(colorblendPoints)>SAL_MAX_INT32/sizeof(::Color) )
                                colorblendPoints = SAL_MAX_INT32/sizeof(::Color);
                            colorblendPositions = new float [colorblendPoints];
                            colorblendColors = new ::Color [colorblendPoints];
                            for (int i=0; i < colorblendPoints; i ++) {
                                s >> colorblendPositions [i];
                                SAL_INFO("cppcanvas.emf", "EMF+\tposition[" << i << "]: " << colorblendPositions [i]);
                            }
                            for (int i=0; i < colorblendPoints; i ++) {
                                s >> color;
                                colorblendColors [i] = ::Color (0xff - (color >> 24), (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
                                SAL_INFO("cppcanvas.emf", "EMF+\tcolor[" << i << "]: 0x" << std::hex << color << std::dec);
                            }
                        }
                        } else {
#if OSL_DEBUG_LEVEL > 1
                            dumpWords (s, 1024);
#endif
                        }
                        break;
                    }
                // linear gradient
                case 4:
                    {
                        s >> additionalFlags >> wrapMode;

                        SAL_INFO("cppcanvas.emf", "EMF+\tlinear gradient, additional flags: 0x" << std::hex << additionalFlags << std::dec);

                        s >> areaX >> areaY >> areaWidth >> areaHeight;

                        SAL_INFO("cppcanvas.emf", "EMF+\tarea: " << areaX << "," << areaY << " - " << areaWidth << "x" << areaHeight);

                        sal_uInt32 color;

                        s >> color;
                        solidColor = ::Color (0xff - (color >> 24), (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
                        SAL_INFO("cppcanvas.emf", "EMF+\tfirst color: 0x" << std::hex << color << std::dec);

                        s >> color;
                        secondColor = ::Color (0xff - (color >> 24), (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
                        SAL_INFO("cppcanvas.emf", "EMF+\tsecond color: 0x" << std::hex << color << std::dec);

                        // repeated colors, unknown meaning, see http://www.aces.uiuc.edu/~jhtodd/Metafile/MetafileRecords/ObjectBrush.html
                        s >> color;
                        s >> color;

                        if (additionalFlags & 0x02) {
                            SAL_INFO("cppcanvas.emf", "EMF+\tuse transformation");
                            s >> transformation;
                            hasTransformation = true;
                            SAL_INFO("cppcanvas.emf",
                                    "EMF+\tm11: "   << transformation.eM11 << " m12: " << transformation.eM12 <<
                                    "\nEMF+\tm21: " << transformation.eM21 << " m22: " << transformation.eM22 <<
                                    "\nEMF+\tdx: "  << transformation.eDx  << " dy: "  << transformation.eDy);
                        }
                        if (additionalFlags & 0x08) {
                            s >> blendPoints;
                            SAL_INFO("cppcanvas.emf", "EMF+\tuse blend, points: " << blendPoints);
                            if( blendPoints<0 || sal_uInt32(blendPoints)>SAL_MAX_INT32/(2*sizeof(float)) )
                                blendPoints = SAL_MAX_INT32/(2*sizeof(float));
                            blendPositions = new float [2*blendPoints];
                            blendFactors = blendPositions + blendPoints;
                            for (int i=0; i < blendPoints; i ++) {
                                s >> blendPositions [i];
                                SAL_INFO("cppcanvas.emf", "EMF+\tposition[" << i << "]: " << blendPositions [i]);
                            }
                            for (int i=0; i < blendPoints; i ++) {
                                s >> blendFactors [i];
                                SAL_INFO("cppcanvas.emf", "EMF+\tfactor[" << i << "]: " << blendFactors [i]);
                            }
                        }

                        if (additionalFlags & 0x04) {
                            s >> colorblendPoints;
                            SAL_INFO("cppcanvas.emf", "EMF+\tuse color blend, points: " << colorblendPoints);
                            if( colorblendPoints<0 || sal_uInt32(colorblendPoints)>SAL_MAX_INT32/sizeof(float) )
                                colorblendPoints = SAL_MAX_INT32/sizeof(float);
                            if( sal_uInt32(colorblendPoints)>SAL_MAX_INT32/sizeof(::Color) )
                                colorblendPoints = sal_uInt32(SAL_MAX_INT32)/sizeof(::Color);
                            colorblendPositions = new float [colorblendPoints];
                            colorblendColors = new ::Color [colorblendPoints];
                            for (int i=0; i < colorblendPoints; i ++) {
                                s >> colorblendPositions [i];
                                SAL_INFO("cppcanvas.emf", "EMF+\tposition[" << i << "]: " << colorblendPositions [i]);
                            }
                            for (int i=0; i < colorblendPoints; i ++) {
                                s >> color;
                                colorblendColors [i] = ::Color (0xff - (color >> 24), (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
                                SAL_INFO("cppcanvas.emf", "EMF+\tcolor[" << i << "]: 0x" << std::hex << color << std::dec);
                            }
                        }

                        break;
                    }
                default:
                    SAL_INFO("cppcanvas.emf", "EMF+\tunhandled brush type: " << std::hex << type << std::dec);
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
                dashPattern = NULL;
                compoundArray = NULL;
                customStartCap = NULL;
                customEndCap = NULL;
            }

            ~EMFPPen ()
            {
                delete[] dashPattern;
                delete[] compoundArray;
                delete[] customStartCap;
                delete[] customEndCap;
            }

            void SetStrokeAttributes (rendering::StrokeAttributes& rStrokeAttributes, ImplRenderer& rR, const OutDevState& rState)
            {
#if OSL_DEBUG_LEVEL > 1
                if (width == 0.0) {
                    SAL_INFO ("cppcanvas.emf", "TODO: pen with zero width - using minimal which might not be correct\n");
                }
#endif
                rStrokeAttributes.StrokeWidth = (rState.mapModeTransform * rR.MapSize (width == 0.0 ? 0.05 : width, 0)).getX ();
            }

            void Read (SvStream& s, ImplRenderer& rR, sal_Int32, sal_Int32 )
            {
                sal_uInt32 header, unknown, penFlags, unknown2;
                int i;

                s >> header >> unknown >> penFlags >> unknown2 >> width;

                SAL_INFO("cppcanvas.emf", "EMF+\tpen");
                SAL_INFO("cppcanvas.emf", "EMF+\theader: 0x" << std::hex << header << " unknown: 0x" << unknown <<
                            " additional flags: 0x" << penFlags << " unknown: 0x" << unknown2 << " width: " << std::dec << width );

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


            void Read (SvMemoryStream &s, sal_uInt32 dataSize, sal_Bool bUseWholeStream)
            {
                sal_uInt32 header, unknown;

                s >> header >> type;

                SAL_INFO("cppcanvas.emf", "EMF+\timage\nEMF+\theader: 0x" << std::hex << header << " type: " << type << std::dec );

                if (type == 1) { // bitmap
                    s >> width >> height >> stride >> pixelFormat >> unknown;
                    SAL_INFO("cppcanvas.emf", "EMF+\tbitmap width: " << width << " height: " << height << " stride: " << "pixelFormat: 0x" << std::hex << pixelFormat << std::dec);
                    if (width == 0) { // non native formats
                        GraphicFilter filter;

                        filter.ImportGraphic (graphic, OUString(), s);
                        SAL_INFO("cppcanvas.emf", "EMF+\tbitmap width: "  << graphic.GetBitmap().GetSizePixel().Width() << " height: " << graphic.GetBitmap().GetSizePixel().Height());
                    }

                } else if (type == 2) {
                    sal_Int32 mfType, mfSize;

                    s >> mfType >> mfSize;
                    SAL_INFO("cppcanvas.emf", "EMF+\tmetafile type: " << mfType << " dataSize: " << mfSize << " real size calculated from record dataSize: " << dataSize - 16);

                    GraphicFilter filter;
                    // workaround buggy metafiles, which have wrong mfSize set (n#705956 for example)
                    SvMemoryStream mfStream (((char *)s.GetData()) + s.Tell(), bUseWholeStream ? s.remainingSize() : dataSize - 16, STREAM_READ);

                    filter.ImportGraphic (graphic, OUString(), mfStream);

                    // debug code - write the stream to debug file /tmp/emf-stream.emf
#if OSL_DEBUG_LEVEL > 1
                        mfStream.Seek(0);
                        static sal_Int32 emfp_debug_stream_number = 0;
                        OUString emfp_debug_filename("/tmp/emf-embedded-stream");
                        emfp_debug_filename += OUString::number(emfp_debug_stream_number++);
                        emfp_debug_filename += OUString(".emf");

                        SvFileStream file( emfp_debug_filename, STREAM_WRITE | STREAM_TRUNC );

                        mfStream >> file;
                        file.Flush();
                        file.Close();
#endif
                }
            }
        };

        struct EMFPFont : public EMFPObject
        {
            sal_uInt32 version;
            float emSize;
            sal_uInt32 sizeUnit;
            sal_Int32 fontFlags;
            OUString family;

            void Read (SvMemoryStream &s)
            {
                sal_uInt32 header;
                sal_uInt32 reserved;
                sal_uInt32 length;

                s >> header >> emSize >> sizeUnit >> fontFlags >> reserved >> length;

                OSL_ASSERT( ( header >> 12 ) == 0xdbc01 );

                SAL_INFO("cppcanvas.emf", "EMF+\tfont\n"
                           << "EMF+\theader: 0x" << std::hex << (header >> 12) << " version: 0x" << (header & 0x1fff) << " size: " << std::dec << emSize << " unit: 0x" << std::hex << sizeUnit << std::dec);
                SAL_INFO("cppcanvas.emf", "EMF+\tflags: 0x" << std::hex << fontFlags << " reserved: 0x" << reserved << " length: 0x" << std::hex << length << std::dec);

                if( length > 0 && length < 0x4000 ) {
                    sal_Unicode *chars = (sal_Unicode *) alloca( sizeof( sal_Unicode ) * length );

                    for( sal_uInt32 i = 0; i < length; i++ )
                        s >> chars[ i ];

                    family = OUString( chars, length );
                    SAL_INFO("cppcanvas.emf", "EMF+\tfamily: " << OUStringToOString( family, RTL_TEXTENCODING_UTF8).getStr()); // TODO: can we just use family?
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
            // TODO: other units
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

        void ImplRenderer::EMFPPlusFillPolygon (::basegfx::B2DPolyPolygon& polygon, const ActionFactoryParameters& rParms,
                                                OutDevState& rState, const CanvasSharedPtr& rCanvas, bool isColor, sal_uInt32 brushIndexOrColor)
        {
            ::basegfx::B2DPolyPolygon localPolygon (polygon);

            SAL_INFO("cppcanvas.emf", "EMF+\tfill polygon");

            localPolygon.transform( rState.mapModeTransform );

            ActionSharedPtr pPolyAction;

            if (isColor) {
                SAL_INFO("cppcanvas.emf", "EMF+\t\tcolor fill:0x" << std::hex << brushIndexOrColor << std::dec);
                rState.isFillColorSet = true;
                rState.isLineColorSet = false;

                rState.fillColor = COLOR(brushIndexOrColor);

                pPolyAction = ActionSharedPtr ( internal::PolyPolyActionFactory::createPolyPolyAction( localPolygon, rParms.mrCanvas, rState ) );

            } else {
                rState.isFillColorSet = true;
                // extract UseBrush
                EMFPBrush* brush = (EMFPBrush*) aObjects [brushIndexOrColor & 0xff];
                SAL_INFO("cppcanvas.emf", "EMF+\tbrush fill slot: " << brushIndexOrColor << " (type: " << brush->GetType () << ")");

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
                    OUString aGradientService;

                    const uno::Sequence< double > aStartColor(
                            ::vcl::unotools::colorToDoubleSequence( brush->solidColor,
                                rParms.mrCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() ) );
                    const uno::Sequence< double > aEndColor(
                            ::vcl::unotools::colorToDoubleSequence( brush->secondColor,
                                rParms.mrCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() ) );
                    uno::Sequence< uno::Sequence < double > > aColors (2);
                    uno::Sequence< double > aStops (2);

                    if (brush->blendPositions) {
                        SAL_INFO("cppcanvas.emf", "EMF+\t\tuse blend");
                        aColors.realloc (brush->blendPoints);
                        aStops.realloc (brush->blendPoints);
                        int length = aStartColor.getLength ();
                        uno::Sequence< double > aColor (length);

                        OSL_ASSERT (length == aEndColor.getLength());

                        for (int i = 0; i < brush->blendPoints; i++) {
                            aStops[i] = brush->blendPositions [i];

                            for (int j = 0; j < length; j++) {
                                if (brush->type == 4) {
                                    aColor [j] = aStartColor [j]*(1 - brush->blendFactors[i]) + aEndColor [j]*brush->blendFactors[i];
                                } else
                                    aColor [j] = aStartColor [j]*brush->blendFactors[i] + aEndColor [j]*(1 - brush->blendFactors[i]);
                            }

                            aColors[i] = aColor;
                        }
                    } else if (brush->colorblendPositions) {
                        SAL_INFO("cppcanvas.emf", "EMF+\t\tuse color blend");
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

                    SAL_INFO("cppcanvas.emf", "EMF+\t\tset gradient");
                    basegfx::B2DRange aBoundsRectangle (0, 0, 1, 1);
                    if (brush->type == 4) {
                        aGradientService = "LinearGradient";
                        aGradInfo = basegfx::tools::createLinearODFGradientInfo(
                                aBoundsRectangle,
                                aStops.getLength(),
                                0,
                                0);

                    } else {
                        aGradientService = "EllipticalGradient";
                        aGradInfo = basegfx::tools::createEllipticalODFGradientInfo(
                                aBoundsRectangle,
                                ::basegfx::B2DVector( 0, 0 ),
                                aStops.getLength(),
                                0,
                                0);
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
                SAL_INFO("cppcanvas.emf", "EMF+\t\tadd poly action");

                maActions.push_back(
                    MtfAction(
                        pPolyAction,
                        rParms.mrCurrActionIndex ) );

                rParms.mrCurrActionIndex += pPolyAction->getActionCount()-1;
            }
        }

        void ImplRenderer::EMFPPlusDrawPolygon (::basegfx::B2DPolyPolygon& polygon, const ActionFactoryParameters& rParms,
                                                OutDevState& rState, const CanvasSharedPtr& rCanvas, sal_uInt32 penIndex)
        {
            EMFPPen* pen = (EMFPPen*) aObjects [penIndex & 0xff];

            SAL_WARN_IF( !pen, "cppcanvas.emf", "emf+ missing pen" );

            if (pen)
            {
                rState.isFillColorSet = false;
                rState.isLineColorSet = true;
                rState.lineColor = ::vcl::unotools::colorToDoubleSequence (pen->GetColor (),
                                                                           rCanvas->getUNOCanvas ()->getDevice()->getDeviceColorSpace());

                polygon.transform( rState.mapModeTransform );
                rendering::StrokeAttributes aStrokeAttributes;

                pen->SetStrokeAttributes (aStrokeAttributes, *this, rState);

                ActionSharedPtr pPolyAction(
                                            internal::PolyPolyActionFactory::createPolyPolyAction(
                                                                                                  polygon, rParms.mrCanvas, rState, aStrokeAttributes ) );

                if( pPolyAction )
                {
                    maActions.push_back(
                                        MtfAction(
                                                  pPolyAction,
                                                  rParms.mrCurrActionIndex ) );

                    rParms.mrCurrActionIndex += pPolyAction->getActionCount()-1;
                }
            }
        }

        void ImplRenderer::processObjectRecord(SvMemoryStream& rObjectStream, sal_uInt16 flags, sal_uInt32 dataSize, sal_Bool bUseWholeStream)
        {
            sal_uInt32 index;

            SAL_INFO("cppcanvas.emf", "EMF+ Object slot: " << (flags & 0xff) << " flags: " << (flags & 0xff00));

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

                SAL_INFO("cppcanvas.emf", "EMF+\tpath");
                SAL_INFO("cppcanvas.emf", "EMF+\theader: 0x" << std::hex << header << " points: " << std::dec << points << " additional flags: 0x" << std::hex << pathFlags << std::dec);

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
                    image->Read (rObjectStream, dataSize, bUseWholeStream);

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
                SAL_INFO("cppcanvas.emf", "EMF+\tObject unhandled flags: 0x" << std::hex << (flags & 0xff00) << std::dec);
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

        void ImplRenderer::GraphicStatePush(GraphicStateMap& map, sal_Int32 index, OutDevState& rState)
        {
            GraphicStateMap::iterator iter = map.find( index );

            if ( iter != map.end() )
            {
                EmfPlusGraphicState state = iter->second;
                map.erase( iter );

                SAL_INFO("cppcanvas.emf", "stack index: " << index << " found and erased");
            }

            EmfPlusGraphicState state;

            state.aWorldTransform = aWorldTransform;
            state.aDevState = rState;

            map[ index ] = state;
        }

        void ImplRenderer::GraphicStatePop(GraphicStateMap& map, sal_Int32 index, OutDevState& rState)
        {
            GraphicStateMap::iterator iter = map.find( index );

            if ( iter != map.end() )
            {
                SAL_INFO("cppcanvas.emf", "stack index: " << index << " found");

                EmfPlusGraphicState state = iter->second;

                aWorldTransform = state.aWorldTransform;
                rState.clip = state.aDevState.clip;
                rState.clipRect = state.aDevState.clipRect;
                rState.xClipPoly = state.aDevState.xClipPoly;
            }
        }

        void ImplRenderer::processEMFPlus( MetaCommentAction* pAct, const ActionFactoryParameters& rFactoryParms,
                                           OutDevState& rState, const CanvasSharedPtr& rCanvas )
        {
            sal_uInt32 length = pAct->GetDataSize ();
            SvMemoryStream rMF ((void*) pAct->GetData (), length, STREAM_READ);

#if OSL_DEBUG_LEVEL > 2
            SAL_INFO("cppcanvas.emf", "EMF+\tDump of EMF+ record");
            dumpWords(rMF, length);
#endif
            length -= 4;

            while (length > 0) {
                sal_uInt16 type, flags;
                sal_uInt32 size, dataSize;
                sal_uInt32 next;

                rMF >> type >> flags >> size >> dataSize;

                next = rMF.Tell() + ( size - 12 );

                if (size < 12) {
                    SAL_INFO("cppcanvas.emf", "Size field is less than 12 bytes");
                }

                SAL_INFO("cppcanvas.emf", "EMF+ record size: " << size << " type: " << type << " flags: " << flags << " data size: " << dataSize);

                if (type == EmfPlusRecordTypeObject && ((mbMultipart && (flags & 0x7fff) == (mMFlags & 0x7fff)) || (flags & 0x8000))) {
                    if (!mbMultipart) {
                        mbMultipart = true;
                        mMFlags = flags;
                        mMStream.Seek(0);
                    }

                    // 1st 4 bytes are unknown
                    mMStream.Write (((const char *)rMF.GetData()) + rMF.Tell() + 4, dataSize - 4);
                    SAL_INFO("cppcanvas.emf", "EMF+ read next object part size: " << size << " type: " << type << " flags: " << flags << " data size: " << dataSize);
                } else {
                    if (mbMultipart) {
                        SAL_INFO("cppcanvas.emf", "EMF+ multipart record flags: " << mMFlags);
                        mMStream.Seek (0);
                        processObjectRecord (mMStream, mMFlags, dataSize, sal_True);
                    }
                    mbMultipart = false;
                }

                if (type != EmfPlusRecordTypeObject || !(flags & 0x8000))
                {
                    switch (type) {
                    case EmfPlusRecordTypeHeader:
                        sal_uInt32 header, version;

                        rMF >> header >> version >> nHDPI >> nVDPI;

                        SAL_INFO("cppcanvas.emf", "EMF+ Header");
                        SAL_INFO("cppcanvas.emf", "EMF+\theader: 0x" << std::hex << header << " version: " << std::dec << version << " horizontal DPI: " << nHDPI << " vertical DPI: " << nVDPI << " dual: " << (flags & 1));

                        break;
                    case EmfPlusRecordTypeEndOfFile:
                        SAL_INFO("cppcanvas.emf", "EMF+ EndOfFile");
                        break;
                    case EmfPlusRecordTypeGetDC:
                        SAL_INFO("cppcanvas.emf", "EMF+ GetDC");
                        SAL_INFO("cppcanvas.emf", "EMF+\talready used in svtools wmf/emf filter parser");
                        break;
                    case EmfPlusRecordTypeObject:
                        processObjectRecord (rMF, flags, dataSize);
                        break;
                    case EmfPlusRecordTypeFillPie:
                        {
                            sal_uInt32 brushIndexOrColor;
                            float startAngle, sweepAngle;

                            rMF >> brushIndexOrColor >> startAngle >> sweepAngle;

                            SAL_INFO("cppcanvas.emf", "EMF+ FillPie colorOrIndex: " << brushIndexOrColor << " startAngle: " << startAngle << " sweepAngle: " << sweepAngle);

                            float dx, dy, dw, dh;

                            ReadRectangle (rMF, dx, dy, dw, dh, flags & 0x4000);

                            SAL_INFO("cppcanvas.emf", "EMF+ RectData: " << dx << "," << dy << " " << dw << "x" << dh);

                            startAngle = 2*M_PI*startAngle/360;
                            sweepAngle = 2*M_PI*sweepAngle/360;

                            B2DPoint mappedCenter (Map (dx + dw/2, dy + dh/2));
                            B2DSize mappedSize( MapSize (dw/2, dh/2));

                            float endAngle = startAngle + sweepAngle;
                            startAngle = fmodf(startAngle, static_cast<float>(M_PI*2));
                            if (startAngle < 0)
                                startAngle += static_cast<float>(M_PI*2);
                            endAngle = fmodf(endAngle, static_cast<float>(M_PI*2));
                            if (endAngle < 0)
                                endAngle += static_cast<float>(M_PI*2);

                            if (sweepAngle < 0)
                                std::swap (endAngle, startAngle);

                            SAL_INFO("cppcanvas.emf", "EMF+ adjusted angles: start " <<
                                     (360.0*startAngle/M_PI) << ", end: " << (360.0*endAngle/M_PI));

                            B2DPolygon polygon = tools::createPolygonFromEllipseSegment (mappedCenter, mappedSize.getX (), mappedSize.getY (), startAngle, endAngle);
                            polygon.append (mappedCenter);
                            polygon.setClosed (true);

                            B2DPolyPolygon polyPolygon (polygon);
                            EMFPPlusFillPolygon (polyPolygon, rFactoryParms, rState, rCanvas, flags & 0x8000, brushIndexOrColor);
                        }
                        break;
                    case EmfPlusRecordTypeFillPath:
                        {
                            sal_uInt32 index = flags & 0xff;
                            sal_uInt32 brushIndexOrColor;

                            rMF >> brushIndexOrColor;

                            SAL_INFO("cppcanvas.emf", "EMF+ FillPath slot: " << index);

                            EMFPPlusFillPolygon (((EMFPPath*) aObjects [index])->GetPolygon (*this), rFactoryParms, rState, rCanvas, flags & 0x8000, brushIndexOrColor);
                        }
                        break;
                    case EmfPlusRecordTypeDrawEllipse:
                    case EmfPlusRecordTypeFillEllipse:
                        {
                            // Intentionally very bogus initial value to avoid MSVC complaining about potentially uninitialized local
                            // variable. As long as the code stays as intended, this variable will be assigned a (real) value in the case
                            // when it is later used.
                            sal_uInt32 brushIndexOrColor = 1234567;

                            if ( type == EmfPlusRecordTypeFillEllipse )
                                rMF >> brushIndexOrColor;

                            SAL_INFO("cppcanvas.emf", "EMF+ " << (type == EmfPlusRecordTypeFillEllipse ? "Fill" : "Draw") << "Ellipse slot: " << (flags & 0xff));

                            float dx, dy, dw, dh;

                            ReadRectangle (rMF, dx, dy, dw, dh, flags & 0x4000);

                            SAL_INFO("cppcanvas.emf", "EMF+ RectData: " << dx << "," << dy << " " << dw << "x" << dh);

                            B2DPoint mappedCenter (Map (dx + dw/2, dy + dh/2));
                            B2DSize mappedSize( MapSize (dw/2, dh/2));

                            ::basegfx::B2DPolyPolygon polyPolygon( ::basegfx::B2DPolygon( ::basegfx::tools::createPolygonFromEllipse( mappedCenter, mappedSize.getX (), mappedSize.getY () ) ) );

                            if ( type == EmfPlusRecordTypeFillEllipse )
                                EMFPPlusFillPolygon( polyPolygon,
                                                     rFactoryParms, rState, rCanvas, flags & 0x8000, brushIndexOrColor );
                            else
                                EMFPPlusDrawPolygon( polyPolygon,
                                                     rFactoryParms, rState, rCanvas, flags & 0xff );
                        }
                        break;
                    case EmfPlusRecordTypeFillRects:
                        {
                            SAL_INFO("cppcanvas.emf", "EMF+ FillRects");

                            sal_uInt32 brushIndexOrColor;
                            sal_Int32 rectangles;
                            bool isColor = (flags & 0x8000);
                            ::basegfx::B2DPolygon polygon;

                            rMF >> brushIndexOrColor >> rectangles;

                            SAL_INFO("cppcanvas.emf", "EMF+\t" << ((flags & 0x8000) ? "color" : "brush index") << ": 0x" << std::hex << brushIndexOrColor << std::dec);

                            for (int i=0; i < rectangles; i++) {
                                if (flags & 0x4000) {
                                    /* 16bit integers */
                                    sal_Int16 x, y, width, height;

                                    rMF >> x >> y >> width >> height;

                                    polygon.append (Map (x, y));
                                    polygon.append (Map (x + width, y));
                                    polygon.append (Map (x + width, y + height));
                                    polygon.append (Map (x, y + height));

                                    SAL_INFO("cppcanvas.emf", "EMF+\trectangle: " << x << "," << " " << width << "x" << height);
                                } else {
                                    /* Single's */
                                    float x, y, width, height;

                                    rMF >> x >> y >> width >> height;

                                    polygon.append (Map (x, y));
                                    polygon.append (Map (x + width, y));
                                    polygon.append (Map (x + width, y + height));
                                    polygon.append (Map (x, y + height));

                                    SAL_INFO("cppcanvas.emf", "EMF+\trectangle: " << x << "," << " " << width << "x" << height);
                                }

                                ::basegfx::B2DPolyPolygon polyPolygon (polygon);

                                // n#812793: EMF+ Seems to specify transparent background with Alpha=0xFF !
                                // Workaround for the problem.
                                if( isColor && brushIndexOrColor == 0xFFFFFFFF && rectangles == 1 )
                                    brushIndexOrColor = 0xFFFFFF;

                                EMFPPlusFillPolygon (polyPolygon, rFactoryParms, rState, rCanvas, isColor, brushIndexOrColor);
                            }
                            break;
                        }
                    case EmfPlusRecordTypeFillPolygon:
                        {
                            sal_uInt8 index = flags & 0xff;
                            sal_uInt32 brushIndexOrColor;
                            sal_Int32 points;

                            rMF >> brushIndexOrColor;
                            rMF >> points;

                            SAL_INFO("cppcanvas.emf", "EMF+ FillPolygon in slot: " << +index << " points: " << points);
                            SAL_INFO("cppcanvas.emf", "EMF+\t: " << ((flags & 0x8000) ? "color" : "brush index") << " 0x" << std::hex << brushIndexOrColor << std::dec);

                            EMFPPath path (points, true);
                            path.Read (rMF, flags, *this);

                            EMFPPlusFillPolygon (path.GetPolygon (*this), rFactoryParms, rState, rCanvas, flags & 0x8000, brushIndexOrColor);

                            break;
                        }
                    case EmfPlusRecordTypeDrawLines:
                        {
                            sal_uInt32 points;

                            rMF >> points;

                            SAL_INFO("cppcanvas.emf", "EMF+ DrawLines in slot: " << (flags && 0xff) << " points: " << points);

                            EMFPPath path (points, true);
                            path.Read (rMF, flags, *this);

                            EMFPPlusDrawPolygon (path.GetPolygon (*this), rFactoryParms, rState, rCanvas, flags);

                            break;
                        }
                    case EmfPlusRecordTypeDrawPath:
                        {
                            sal_uInt32 penIndex;

                            rMF >> penIndex;

                            SAL_INFO("cppcanvas.emf", "EMF+ DrawPath");
                            SAL_INFO("cppcanvas.emf", "EMF+\tpen: " << penIndex);

                            EMFPPath* path = (EMFPPath*) aObjects [flags & 0xff];
                            SAL_WARN_IF( !path, "cppcanvas.emf", "EmfPlusRecordTypeDrawPath missing path" );

                            EMFPPlusDrawPolygon (path->GetPolygon (*this), rFactoryParms, rState, rCanvas, penIndex);

                            break;
                        }
                    case EmfPlusRecordTypeDrawImage:
                    case EmfPlusRecordTypeDrawImagePoints:
                        {
                            sal_uInt32 attrIndex;
                            sal_Int32 sourceUnit;

                            rMF >> attrIndex >> sourceUnit;

                            SAL_INFO("cppcanvas.emf", "EMF+ " << (type == EmfPlusRecordTypeDrawImagePoints ? "DrawImagePoints" : "DrawImage") << "attributes index: " << attrIndex << "source unit: " << sourceUnit);
                            SAL_INFO("cppcanvas.emf", "EMF+\tTODO: use image attributes");

                            if (sourceUnit == 2 && aObjects [flags & 0xff]) { // we handle only GraphicsUnit.Pixel now
                                EMFPImage& image = *(EMFPImage *) aObjects [flags & 0xff];
                                float sx, sy, sw, sh;
                                sal_Int32 aCount;

                                ReadRectangle (rMF, sx, sy, sw, sh);

                                SAL_INFO("cppcanvas.emf", "EMF+ " << (type == EmfPlusRecordTypeDrawImagePoints ? "DrawImagePoints" : "DrawImage") << " source rectangle: " << sx << "," << sy << " " << sw << "x" << sh);

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

                                        SAL_INFO("cppcanvas.emf", "EMF+ destination points: " << x1 << "," << y1 << " " << x2 << "," << y2 << " " << x3 << "," << y3);
                                        SAL_INFO("cppcanvas.emf", "EMF+ destination rectangle: " << x1 << "," << y1 << " " << x2 - x1 << "x" << y3 - y1);

                                        aDstPoint = Map (x1, y1);
                                        aDstSize = MapSize(x2 - x1, y3 - y1);

                                        bValid = true;
                                    }
                                } else if (type == EmfPlusRecordTypeDrawImage) {
                                    float dx, dy, dw, dh;

                                    ReadRectangle (rMF, dx, dy, dw, dh, flags & 0x4000);

                                    SAL_INFO("cppcanvas.emf", "EMF+ destination rectangle: " << dx << "," << dy << " " << dw << "x" << dh);

                                    aDstPoint = Map (dx, dy);
                                    aDstSize = MapSize(dw, dh);

                                    bValid = true;
                                }

                                if (bValid) {
                                    BitmapEx aBmp( image.graphic.GetBitmapEx () );

                                    Size aSize( aBmp.GetSizePixel() );
                                    SAL_INFO("cppcanvas.emf", "EMF+ bitmap size: " << aSize.Width() << "x" << aSize.Height());
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
                                        SAL_INFO("cppcanvas.emf", "EMF+ warning: empty bitmap");
                                    }
                                } else {
                                    SAL_INFO("cppcanvas.emf", "EMF+ DrawImage(Points) TODO (fixme)");
                                }
                            } else {
                                SAL_INFO("cppcanvas.emf", "EMF+ DrawImage(Points) TODO (fixme) - possibly unsupported source units for crop rectangle");
                            }
                            break;
                        }
                    case EmfPlusRecordTypeDrawString:
                        {
                            SAL_INFO("cppcanvas.emf", "EMF+ DrawString");

                            sal_uInt32 brushId;
                            sal_uInt32 formatId;
                            sal_uInt32 stringLength;

                            rMF >> brushId >> formatId >> stringLength;
                            SAL_INFO("cppcanvas.emf", "EMF+ DrawString brushId: " << brushId << " formatId: " << formatId << " length: " << stringLength);

                            if (flags & 0x8000) {
                                float lx, ly, lw, lh;

                                rMF >> lx >> ly >> lw >> lh;

                                SAL_INFO("cppcanvas.emf", "EMF+ DrawString layoutRect: " << lx << "," << ly << " - " << lw << "x" << lh);

                                OUString text = read_uInt16s_ToOUString(rMF, stringLength);

                                double cellSize = setFont (flags & 0xff, rFactoryParms, rState);
                                rState.textColor = COLOR( brushId );

                                ::basegfx::B2DPoint point( Map( lx + 0.15*cellSize, ly + cellSize ) );

                                ActionSharedPtr pTextAction(
                                    TextActionFactory::createTextAction(
                                                                        // position is just rough guess for now
                                                                        // we should calculate it exactly from layoutRect or font
                                        ::vcl::unotools::pointFromB2DPoint ( point ),
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
                                    SAL_INFO("cppcanvas.emf", "EMF+\t\tadd text action");

                                    maActions.push_back(
                                                        MtfAction(
                                                                  pTextAction,
                                                                  rFactoryParms.mrCurrActionIndex ) );

                                    rFactoryParms.mrCurrActionIndex += pTextAction->getActionCount()-1;
                                }
                            } else {
                                SAL_INFO("cppcanvas.emf", "EMF+ DrawString TODO - drawing with brush not yet supported");
                            }
                        }
                        break;
                    case EmfPlusRecordTypeSetPageTransform:
                        rMF >> fPageScale;

                        SAL_INFO("cppcanvas.emf", "EMF+ SetPageTransform");
                        SAL_INFO("cppcanvas.emf", "EMF+\tscale: " << fPageScale << " unit: " << flags);
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    case EmfPlusRecordTypeSetRenderingOrigin:
                        rMF >> nOriginX >> nOriginY;
                        SAL_INFO("cppcanvas.emf", "EMF+ SetRenderingOrigin");
                        SAL_INFO("cppcanvas.emf", "EMF+\torigin [x,y]: " << nOriginX << "," << nOriginY);
                        break;
                    case EmfPlusRecordTypeSetTextRenderingHint:
                        SAL_INFO("cppcanvas.emf", "EMF+ SetTextRenderingHint");
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    case EmfPlusRecordTypeSetAntiAliasMode:
                        SAL_INFO("cppcanvas.emf", "EMF+ SetAntiAliasMode");
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    case EmfPlusRecordTypeSetInterpolationMode:
                        SAL_INFO("cppcanvas.emf", "EMF+ InterpolationMode");
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    case EmfPlusRecordTypeSetPixelOffsetMode:
                        SAL_INFO("cppcanvas.emf", "EMF+ SetPixelOffsetMode");
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    case EmfPlusRecordTypeSetCompositingQuality:
                        SAL_INFO("cppcanvas.emf", "EMF+ SetCompositingQuality");
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    case EmfPlusRecordTypeSave:
                    {
                        sal_uInt32 stackIndex;

                        rMF >> stackIndex;

                        SAL_INFO("cppcanvas.emf", "EMF+ Save stack index: " << stackIndex);

                        GraphicStatePush( mGSStack, stackIndex, rState );

                        break;
                    }
                    case EmfPlusRecordTypeRestore:
                    {
                        sal_uInt32 stackIndex;

                        rMF >> stackIndex;

                        SAL_INFO("cppcanvas.emf", "EMF+ Restore stack index: " << stackIndex);

                        GraphicStatePop( mGSStack, stackIndex, rState );

                        break;
                    }
                    case EmfPlusRecordTypeBeginContainerNoParams:
                    {
                        sal_uInt32 stackIndex;

                        rMF >> stackIndex;

                        SAL_INFO("cppcanvas.emf", "EMF+ Begin Container No Params stack index: " << stackIndex);

                        GraphicStatePush( mGSContainerStack, stackIndex, rState );
                    }
                    break;
                    case EmfPlusRecordTypeEndContainer:
                    {
                        sal_uInt32 stackIndex;

                        rMF >> stackIndex;

                        SAL_INFO("cppcanvas.emf", "EMF+ End Container stack index: " << stackIndex);

                        GraphicStatePop( mGSContainerStack, stackIndex, rState );
                    }
                    break;
                    case EmfPlusRecordTypeSetWorldTransform: {
                        SAL_INFO("cppcanvas.emf", "EMF+ SetWorldTransform");
                        XForm transform;
                        rMF >> transform;
                        aWorldTransform.Set (transform);
                        SAL_INFO("cppcanvas.emf",
                                "EMF+\tm11: " << aWorldTransform.eM11 << "\tm12: " << aWorldTransform.eM12 <<
                                "\tm21: " << aWorldTransform.eM21 << "\tm22: " << aWorldTransform.eM22 <<
                                "\tdx: "  << aWorldTransform.eDx  << "\tdy: "  << aWorldTransform.eDy);
                        break;
                    }
                    case EmfPlusRecordTypeResetWorldTransform:
                        SAL_INFO("cppcanvas.emf", "EMF+ ResetWorldTransform");
                        aWorldTransform.SetIdentity ();
                        break;
                    case EmfPlusRecordTypeMultiplyWorldTransform: {
                        SAL_INFO("cppcanvas.emf", "EMF+ MultiplyWorldTransform");
                        XForm transform;
                        rMF >> transform;

                        SAL_INFO("cppcanvas.emf",
                                "EMF+\tmatrix m11: " << transform.eM11 << "m12: " << transform.eM12 <<
                                "EMF+\tm21: "        << transform.eM21 << "m22: " << transform.eM22 <<
                                "EMF+\tdx: "         << transform.eDx  << "dy: "  << transform.eDy);

                        if (flags & 0x2000)  // post multiply
                            aWorldTransform.Multiply (transform);
                        else {               // pre multiply
                            transform.Multiply (aWorldTransform);
                            aWorldTransform.Set (transform);
                        }
                        SAL_INFO("cppcanvas.emf",
                                "EMF+\tm11: " << aWorldTransform.eM11 << "m12: " << aWorldTransform.eM12 <<
                                "EMF+\tm21: " << aWorldTransform.eM21 << "m22: " << aWorldTransform.eM22 <<
                                "EMF+\tdx: "  << aWorldTransform.eDx  << "dy: "  << aWorldTransform.eDy);
                        break;
                    }
                    case EmfPlusRecordTypeSetClipRect:
                        {
                            int combineMode = (flags >> 8) & 0xf;

                            SAL_INFO("cppcanvas.emf", "EMF+ SetClipRect combine mode: " << combineMode);
#if OSL_DEBUG_LEVEL > 1
                            if (combineMode > 1) {
                                SAL_INFO ("cppcanvas.emf", "EMF+ TODO combine mode > 1");
                            }
#endif

                            float dx, dy, dw, dh;

                            ReadRectangle (rMF, dx, dy, dw, dh, false);

                            SAL_INFO("cppcanvas.emf", "EMF+ RectData: " << dx << "," << dy << " " << dw << "x" << dh);

                            B2DPoint mappedPoint (Map (dx, dy));
                            B2DSize mappedSize( MapSize (dw, dh));

                            ::basegfx::B2DPolyPolygon polyPolygon( ::basegfx::B2DPolygon( ::basegfx::tools::createPolygonFromRect( ::basegfx::B2DRectangle( mappedPoint.getX(), mappedPoint.getY(),
                                                                                                                                                            mappedPoint.getX() + mappedSize.getX(),
                                                                                                                                                            mappedPoint.getY() + mappedSize.getY() ) ) ) );
                            polyPolygon.transform(rState.mapModeTransform);

                            updateClipping (polyPolygon, rFactoryParms, combineMode == 1);

                            break;
                        }
                    case EmfPlusRecordTypeSetClipPath:
                        {
                            int combineMode = (flags >> 8) & 0xf;

                            SAL_INFO("cppcanvas.emf", "EMF+ SetClipPath combine mode: " << combineMode);
                            SAL_INFO("cppcanvas.emf", "EMF+\tpath in slot: " << (flags & 0xff));

                            EMFPPath& path = *(EMFPPath*) aObjects [flags & 0xff];
                            ::basegfx::B2DPolyPolygon& clipPoly (path.GetPolygon (*this));

                            clipPoly.transform (rState.mapModeTransform);
                            updateClipping (clipPoly, rFactoryParms, combineMode == 1);

                            break;
                        }
                    case EmfPlusRecordTypeSetClipRegion: {
                        int combineMode = (flags >> 8) & 0xf;

                        SAL_INFO("cppcanvas.emf", "EMF+ SetClipRegion");
                        SAL_INFO("cppcanvas.emf", "EMF+\tregion in slot: " << (flags & 0xff) << " combine mode: " << combineMode);
                        EMFPRegion *region = (EMFPRegion*)aObjects [flags & 0xff];

                        // reset clip
                        if (region && region->parts == 0 && region->initialState == EmfPlusRegionInitialStateInfinite) {
                            updateClipping (::basegfx::B2DPolyPolygon (), rFactoryParms, combineMode == 1);
                        } else {
                            SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        }
                        break;
                    }
                    case EmfPlusRecordTypeDrawDriverString: {
                        SAL_INFO("cppcanvas.emf", "EMF+ DrawDriverString, flags: 0x" << std::hex << flags << std::dec);
                        sal_uInt32 brushIndexOrColor;
                        sal_uInt32 optionFlags;
                        sal_uInt32 hasMatrix;
                        sal_uInt32 glyphsCount;

                        rMF >> brushIndexOrColor >> optionFlags >> hasMatrix >> glyphsCount;

                        SAL_INFO("cppcanvas.emf", "EMF+\t: " << ((flags & 0x8000) ? "color" : "brush index") << " 0x" << std::hex << brushIndexOrColor << std::dec);
                        SAL_INFO("cppcanvas.emf", "EMF+\toption flags: 0x" << std::hex << optionFlags << std::dec);
                        SAL_INFO("cppcanvas.emf", "EMF+\thas matrix: " << hasMatrix);
                        SAL_INFO("cppcanvas.emf", "EMF+\tglyphs: " << glyphsCount);

                        if( ( optionFlags & 1 ) && glyphsCount > 0 ) {
                            float *charsPosX = new float[glyphsCount];
                            float *charsPosY = new float[glyphsCount];

                            OUString text = read_uInt16s_ToOUString(rMF, glyphsCount);

                            for( sal_uInt32 i=0; i<glyphsCount; i++) {
                                rMF >> charsPosX[i] >> charsPosY[i];
                                SAL_INFO("cppcanvas.emf", "EMF+\tglyphPosition[" << i << "]: " << charsPosX[i] << "," << charsPosY[i]);
                            }

                            XForm transform;
                            if( hasMatrix ) {
                                rMF >> transform;
                                SAL_INFO("cppcanvas.emf", "EMF+\tmatrix: " << transform.eM11 << ", " << transform.eM12 << ", " << transform.eM21 << ", " << transform.eM22 << ", " << transform.eDx << ", " << transform.eDy);
                            }

                            // add the text action
                            setFont (flags & 0xff, rFactoryParms, rState);

                            if( flags & 0x8000 )
                                rState.textColor = COLOR( brushIndexOrColor );

                            ::basegfx::B2DPoint point( Map( charsPosX[0], charsPosY[0] ) );

                            ActionSharedPtr pTextAction(
                                    TextActionFactory::createTextAction(
                                        ::vcl::unotools::pointFromB2DPoint ( point ),
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
                                SAL_INFO("cppcanvas.emf", "EMF+\t\tadd text action");

                                maActions.push_back(
                                        MtfAction(
                                            pTextAction,
                                            rFactoryParms.mrCurrActionIndex ) );

                                rFactoryParms.mrCurrActionIndex += pTextAction->getActionCount()-1;
                            }

                            delete[] charsPosX;
                            delete[] charsPosY;
                        } else {
                            SAL_INFO("cppcanvas.emf", "EMF+\tTODO: fonts (non-unicode glyphs chars)");
                        }

                        break;
                    }
                    default:
                        SAL_INFO("cppcanvas.emf", "EMF+ unhandled record type: " << type);
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                    }
                }

                rMF.Seek (next);

                if (size <= length)
                {
                    length -= size;
                }
                else
                {
                    SAL_WARN("cppcanvas.emf", "ImplRenderer::processEMFPlus: "
                            "size " << size << " > length " << length);
#if OSL_DEBUG_LEVEL > 1
                    dumpWords(rMF, length);
#endif
                    length = 0;
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
