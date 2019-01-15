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

#include <emfplushelper.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <tools/stream.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <map>

// predefines
class SvStream;
namespace basegfx { class B2DPolyPolygon; }

namespace emfplushelper
{
    // EMF+ commands
    #define EmfPlusRecordTypeHeader 0x4001
    #define EmfPlusRecordTypeEndOfFile 0x4002
    #define EmfPlusRecordTypeComment 0x4003
    #define EmfPlusRecordTypeGetDC 0x4004
    //TODO EmfPlusRecordTypeMultiFormatStart 0x4005
    //TODO EmfPlusRecordTypeMultiFormatSection 0x4006
    //TODO EmfPlusRecordTypeMultiFormatEnd 0x4007
    #define EmfPlusRecordTypeObject 0x4008
    //TODO EmfPlusRecordTypeClear 0x4009
    #define EmfPlusRecordTypeFillRects 0x400A
    #define EmfPlusRecordTypeDrawRects 0x400B
    #define EmfPlusRecordTypeFillPolygon 0x400C
    #define EmfPlusRecordTypeDrawLines 0x400D
    #define EmfPlusRecordTypeFillEllipse 0x400E
    #define EmfPlusRecordTypeDrawEllipse 0x400F
    #define EmfPlusRecordTypeFillPie 0x4010
    #define EmfPlusRecordTypeDrawPie 0x4011
    #define EmfPlusRecordTypeDrawArc 0x4012
    #define EmfPlusRecordTypeFillRegion 0x4013
    #define EmfPlusRecordTypeFillPath 0x4014
    #define EmfPlusRecordTypeDrawPath 0x4015
    //TODO EmfPlusRecordTypeFillClosedCurve 0x4016
    //TODO EmfPlusRecordTypeDrawClosedCurve 0x4017
    //TODO EmfPlusRecordTypeDrawCurve 0x4018
    #define EmfPlusRecordTypeDrawBeziers 0x4019
    #define EmfPlusRecordTypeDrawImage 0x401A
    #define EmfPlusRecordTypeDrawImagePoints 0x401B
    #define EmfPlusRecordTypeDrawString 0x401C
    #define EmfPlusRecordTypeSetRenderingOrigin 0x401D
    #define EmfPlusRecordTypeSetAntiAliasMode 0x401E
    #define EmfPlusRecordTypeSetTextRenderingHint 0x401F
    //TODO EmfPlusSetTextContrast 0x4020
    #define EmfPlusRecordTypeSetInterpolationMode 0x4021
    #define EmfPlusRecordTypeSetPixelOffsetMode 0x4022
    //TODO EmfPlusRecordTypeSetCompositingMode 0x4023
    #define EmfPlusRecordTypeSetCompositingQuality 0x4024
    #define EmfPlusRecordTypeSave 0x4025
    #define EmfPlusRecordTypeRestore 0x4026
    //TODO EmfPlusRecordTypeBeginContainer 0x4027
    #define EmfPlusRecordTypeBeginContainerNoParams 0x4028
    #define EmfPlusRecordTypeEndContainer 0x4029
    #define EmfPlusRecordTypeSetWorldTransform 0x402A
    #define EmfPlusRecordTypeResetWorldTransform 0x402B
    #define EmfPlusRecordTypeMultiplyWorldTransform 0x402C
    #define EmfPlusRecordTypeTranslateWorldTransform 0x402D
    #define EmfPlusRecordTypeScaleWorldTransform 0x402E
    #define EmfPlusRecordTypeRotateWorldTransform 0x402F
    #define EmfPlusRecordTypeSetPageTransform 0x4030
    #define EmfPlusRecordTypeResetClip 0x4031
    #define EmfPlusRecordTypeSetClipRect 0x4032
    #define EmfPlusRecordTypeSetClipPath 0x4033
    #define EmfPlusRecordTypeSetClipRegion 0x4034
    #define EmfPlusRecordTypeOffsetClip 0x4035
    #define EmfPlusRecordTypeDrawDriverString 0x4036
    //TODO EmfPlusRecordTypeStrokeFillPath 0x4037
    //TODO EmfPlusRecordTypeSerializableObject 0x4038
    //TODO EmfPlusRecordTypeSetTSGraphics 0x4039
    //TODO EmfPlusRecordTypeSetTSClip 0x403A

    // EMF+object types
    #define EmfPlusObjectTypeBrush 0x100
    #define EmfPlusObjectTypePen 0x200
    #define EmfPlusObjectTypePath 0x300
    #define EmfPlusObjectTypeRegion 0x400
    #define EmfPlusObjectTypeImage 0x500
    #define EmfPlusObjectTypeFont 0x600
    #define EmfPlusObjectTypeStringFormat 0x700
    #define EmfPlusObjectTypeImageAttributes 0x800
    #define EmfPlusObjectTypeCustomLineCap 0x900

    enum UnitType
    {
        UnitTypeWorld = 0x00,
        UnitTypeDisplay = 0x01,
        UnitTypePixel = 0x02,
        UnitTypePoint = 0x03,
        UnitTypeInch = 0x04,
        UnitTypeDocument = 0x05,
        UnitTypeMillimeter = 0x06
    };

    enum EmfPlusCombineMode
    {
        EmfPlusCombineModeReplace = 0x00000000,
        EmfPlusCombineModeIntersect = 0x00000001,
        EmfPlusCombineModeUnion = 0x00000002,
        EmfPlusCombineModeXOR = 0x00000003,
        EmfPlusCombineModeExclude = 0x00000004,
        EmfPlusCombineModeComplement = 0x00000005
    };

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
        static ::basegfx::B2DPolyPolygon const combineClip(::basegfx::B2DPolyPolygon const & leftPolygon, int combineMode, ::basegfx::B2DPolyPolygon const & rightPolygon);

        static float getUnitToPixelMultiplier(const UnitType aUnitType);
    };
}

#endif // INCLUDED_DRAWINGLAYER_SOURCE_TOOLS_EMFPHELPERDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
