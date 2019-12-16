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

#ifndef INCLUDED_DRAWINGLAYER_SOURCE_TOOLS_EMFPHELPERDATA_HXX
#define INCLUDED_DRAWINGLAYER_SOURCE_TOOLS_EMFPHELPERDATA_HXX

#include "emfpenums.hxx"
#include <wmfemfhelper.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <tools/stream.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <map>

class SvStream;
namespace basegfx
{
    class B2DRange;
    typedef B2DRange B2DRectangle;
    class B2DPolyPolygon;
}

namespace emfplushelper
{
    struct EMFPImage;

    const char* emfTypeToName(sal_uInt16 type);
    struct EMFPObject
    {
        virtual ~EMFPObject();
    };

    typedef std::map<int, wmfemfhelper::PropertyHolder> GraphicStateMap;

    struct EmfPlusHelperData
    {
    private:
        /* EMF+ */
        basegfx::B2DHomMatrix       maBaseTransform;
        basegfx::B2DHomMatrix       maWorldTransform;
        basegfx::B2DHomMatrix       maMapTransform;

        std::unique_ptr<EMFPObject> maEMFPObjects[256];
        float                       mfPageScale;
        sal_Int32                   mnOriginX;
        sal_Int32                   mnOriginY;
        sal_uInt32                  mnHDPI;
        sal_uInt32                  mnVDPI;
        bool                        mbSetTextContrast;
        sal_uInt16                  mnTextContrast;

        /* EMF+ emf header info */
        sal_Int32                   mnFrameLeft;
        sal_Int32                   mnFrameTop;
        sal_Int32                   mnFrameRight;
        sal_Int32                   mnFrameBottom;
        sal_Int32                   mnPixX;
        sal_Int32                   mnPixY;
        sal_Int32                   mnMmX;
        sal_Int32                   mnMmY;

        /* multipart object data */
        bool                        mbMultipart;
        sal_uInt16                  mMFlags;
        SvMemoryStream              mMStream;

        /* emf+ graphic state stack */
        GraphicStateMap             mGSStack;
        GraphicStateMap             mGSContainerStack;

        /// data holders
        wmfemfhelper::TargetHolders&    mrTargetHolders;
        wmfemfhelper::PropertyHolders&  mrPropertyHolders;
        bool                            bIsGetDCProcessing;

        // readers
        void processObjectRecord(SvMemoryStream& rObjectStream, sal_uInt16 flags, sal_uInt32 dataSize, bool bUseWholeStream = false);
        static void ReadPoint(SvStream& s, float& x, float& y, sal_uInt32 flags);

        // internal mapper
        void mappingChanged();

        // stack actions
        void GraphicStatePush(GraphicStateMap& map, sal_Int32 index);
        void GraphicStatePop (GraphicStateMap& map, sal_Int32 index, wmfemfhelper::PropertyHolder& rState);

        // primitive creators
        void EMFPPlusDrawPolygon(const ::basegfx::B2DPolyPolygon& polygon, sal_uInt32 penIndex);
        void EMFPPlusFillPolygon(const ::basegfx::B2DPolyPolygon& polygon, const bool isColor, const sal_uInt32 brushIndexOrColor);
        void EMFPPlusDrawImage(const ::basegfx::B2DRectangle& rect, EMFPImage const& image, basegfx::B2DHomMatrix const& rTransformationMatrix);

        // helper functions
        Color EMFPGetBrushColorOrARGBColor(const sal_uInt16 flags, const sal_uInt32 brushIndexOrColor) const;

    public:
        EmfPlusHelperData(
            SvMemoryStream& rMS,
            wmfemfhelper::TargetHolders& rTargetHolders,
            wmfemfhelper::PropertyHolders& rPropertyHolders);
        ~EmfPlusHelperData();

        void processEmfPlusData(
            SvMemoryStream& rMS,
            const drawinglayer::geometry::ViewInformation2D& rViewInformation);

        // mappers
        ::basegfx::B2DPoint Map(double ix, double iy) const;

        // readers
        static void ReadRectangle(SvStream& s, float& x, float& y, float &width, float& height, bool bCompressed = false);
        static bool readXForm(SvStream& rIn, basegfx::B2DHomMatrix& rTarget);
        static ::basegfx::B2DPolyPolygon combineClip(::basegfx::B2DPolyPolygon const & leftPolygon, int combineMode, ::basegfx::B2DPolyPolygon const & rightPolygon);

        static float getUnitToPixelMultiplier(const UnitType aUnitType);
    };
}

#endif // INCLUDED_DRAWINGLAYER_SOURCE_TOOLS_EMFPHELPERDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
