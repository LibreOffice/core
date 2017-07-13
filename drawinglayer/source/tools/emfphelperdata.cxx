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

#include <emfphelperdata.hxx>
#include <emfpbrush.hxx>
#include <emfppen.hxx>
#include <emfppath.hxx>
#include <emfpregion.hxx>
#include <emfpimage.hxx>
#include <emfpfont.hxx>
#include <emfpstringformat.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <wmfemfhelper.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

namespace emfplushelper
{
    const char* emfTypeToName(sal_uInt16 type)
    {
        switch (type)
        {
            case EmfPlusRecordTypeHeader: return "EmfPlusRecordTypeHeader";
            case EmfPlusRecordTypeEndOfFile: return "EmfPlusRecordTypeEndOfFile";
            case EmfPlusRecordTypeGetDC: return "EmfPlusRecordTypeGetDC";
            case EmfPlusRecordTypeObject: return "EmfPlusRecordTypeObject";
            case EmfPlusRecordTypeFillRects: return "EmfPlusRecordTypeFillRects";
            case EmfPlusRecordTypeDrawRects: return "EmfPlusRecordTypeDrawRects";
            case EmfPlusRecordTypeFillPolygon: return "EmfPlusRecordTypeFillPolygon";
            case EmfPlusRecordTypeDrawLines: return "EmfPlusRecordTypeDrawLines";
            case EmfPlusRecordTypeFillEllipse: return "EmfPlusRecordTypeFillEllipse";
            case EmfPlusRecordTypeDrawEllipse: return "EmfPlusRecordTypeDrawEllipse";
            case EmfPlusRecordTypeFillPie: return "EmfPlusRecordTypeFillPie";
            case EmfPlusRecordTypeDrawPie: return "EmfPlusRecordTypeDrawPie";
            case EmfPlusRecordTypeDrawArc: return "EmfPlusRecordTypeDrawArc";
            case EmfPlusRecordTypeFillPath: return "EmfPlusRecordTypeFillPath";
            case EmfPlusRecordTypeDrawPath: return "EmfPlusRecordTypeDrawPath";
            case EmfPlusRecordTypeDrawBeziers: return "EmfPlusRecordTypeDrawBeziers";
            case EmfPlusRecordTypeDrawImage: return "EmfPlusRecordTypeDrawImage";
            case EmfPlusRecordTypeDrawImagePoints: return "EmfPlusRecordTypeDrawImagePoints";
            case EmfPlusRecordTypeDrawString: return "EmfPlusRecordTypeDrawString";
            case EmfPlusRecordTypeSetRenderingOrigin: return "EmfPlusRecordTypeSetRenderingOrigin";
            case EmfPlusRecordTypeSetAntiAliasMode: return "EmfPlusRecordTypeSetAntiAliasMode";
            case EmfPlusRecordTypeSetTextRenderingHint: return "EmfPlusRecordTypeSetTextRenderingHint";
            case EmfPlusRecordTypeSetInterpolationMode: return "EmfPlusRecordTypeSetInterpolationMode";
            case EmfPlusRecordTypeSetPixelOffsetMode: return "EmfPlusRecordTypeSetPixelOffsetMode";
            case EmfPlusRecordTypeSetCompositingQuality: return "EmfPlusRecordTypeSetCompositingQuality";
            case EmfPlusRecordTypeSave: return "EmfPlusRecordTypeSave";
            case EmfPlusRecordTypeRestore: return "EmfPlusRecordTypeRestore";
            case EmfPlusRecordTypeBeginContainerNoParams: return "EmfPlusRecordTypeBeginContainerNoParams";
            case EmfPlusRecordTypeEndContainer: return "EmfPlusRecordTypeEndContainer";
            case EmfPlusRecordTypeSetWorldTransform: return "EmfPlusRecordTypeSetWorldTransform";
            case EmfPlusRecordTypeResetWorldTransform: return "EmfPlusRecordTypeResetWorldTransform";
            case EmfPlusRecordTypeMultiplyWorldTransform: return "EmfPlusRecordTypeMultiplyWorldTransform";
            case EmfPlusRecordTypeTranslateWorldTransform: return "EmfPlusRecordTypeTranslateWorldTransform";
            case EmfPlusRecordTypeScaleWorldTransform: return "EmfPlusRecordTypeScaleWorldTransform";
            case EmfPlusRecordTypeSetPageTransform: return "EmfPlusRecordTypeSetPageTransform";
            case EmfPlusRecordTypeSetClipRect: return "EmfPlusRecordTypeSetClipRect";
            case EmfPlusRecordTypeSetClipPath: return "EmfPlusRecordTypeSetClipPath";
            case EmfPlusRecordTypeSetClipRegion: return "EmfPlusRecordTypeSetClipRegion";
            case EmfPlusRecordTypeDrawDriverString: return "EmfPlusRecordTypeDrawDriverString";
        }
        return "";
    }

    EMFPObject::~EMFPObject()
    {
    }

//    OutDevState::OutDevState() :
//        clip(),
//        clipRect(),
//        xClipPoly(),
//
//        lineColor(),
//        fillColor(),
//        textColor(),
//        textFillColor(),
//        textLineColor(),
//
//        xFont(),
//        transform(),
//        mapModeTransform(),
//        fontRotation(0.0),
//
//        textEmphasisMarkStyle(FontEmphasisMark::NONE),
//        pushFlags(PushFlags::ALL),
//        textDirection(css::rendering::TextDirection::WEAK_LEFT_TO_RIGHT),
//        textAlignment(0), // TODO(Q2): Synchronize with implrenderer
//                            // and possibly new rendering::TextAlignment
//        textReliefStyle(FontRelief::NONE),
//        textOverlineStyle(LINESTYLE_NONE),
//        textUnderlineStyle(LINESTYLE_NONE),
//        textStrikeoutStyle(STRIKEOUT_NONE),
//        textReferencePoint(ALIGN_BASELINE),
//
//        isTextOutlineModeSet(false),
//        isTextEffectShadowSet(false),
//        isTextWordUnderlineSet(false),
//
//        isLineColorSet(false),
//        isFillColorSet(false),
//        isTextFillColorSet(false),
//        isTextLineColorSet(false)
//    {
//    }

    void EmfPlusHelperData::processObjectRecord(SvMemoryStream& rObjectStream, sal_uInt16 flags, sal_uInt32 dataSize, bool bUseWholeStream)
    {
        sal_uInt32 index;
        SAL_INFO("cppcanvas.emf", "EMF+ Object slot: " << (flags & 0xff) << " flags: " << (flags & 0xff00));
        index = flags & 0xff;

        switch (flags & 0x7f00)
        {
            case EmfPlusObjectTypeBrush:
            {
                EMFPBrush *brush;
                maEMFPObjects[index].reset(brush = new EMFPBrush());
                brush->Read(rObjectStream, *this);
                break;
            }
            case EmfPlusObjectTypePen:
            {
                EMFPPen *pen;
                maEMFPObjects[index].reset(pen = new EMFPPen());
                pen->Read(rObjectStream, *this);
                break;
            }
            case EmfPlusObjectTypePath:
            {
                sal_uInt32 header, pathFlags;
                sal_Int32 points;

                rObjectStream.ReadUInt32(header).ReadInt32(points).ReadUInt32(pathFlags);
                SAL_INFO("cppcanvas.emf", "EMF+\tpath");
                SAL_INFO("cppcanvas.emf", "EMF+\theader: 0x" << std::hex << header << " points: " << std::dec << points << " additional flags: 0x" << std::hex << pathFlags << std::dec);
                EMFPPath *path;
                maEMFPObjects[index].reset(path = new EMFPPath(points));
                path->Read(rObjectStream, pathFlags, *this);
                break;
            }
            case EmfPlusObjectTypeRegion:
            {
                EMFPRegion *region;
                maEMFPObjects[index].reset(region = new EMFPRegion());
                region->Read(rObjectStream);
                break;
            }
            case EmfPlusObjectTypeImage:
            {
                EMFPImage *image;
                maEMFPObjects[index].reset(image = new EMFPImage);
                image->type = 0;
                image->width = 0;
                image->height = 0;
                image->stride = 0;
                image->pixelFormat = 0;
                image->Read(rObjectStream, dataSize, bUseWholeStream);
                break;
            }
            case EmfPlusObjectTypeFont:
            {
                EMFPFont *font;
                maEMFPObjects[index].reset(font = new EMFPFont);
                font->emSize = 0;
                font->sizeUnit = 0;
                font->fontFlags = 0;
                font->Read(rObjectStream);
                break;
            }
            case EmfPlusObjectTypeStringFormat:
            {
                EMFPStringFormat *stringFormat;
                maEMFPObjects[index].reset(stringFormat = new EMFPStringFormat());
                stringFormat->Read(rObjectStream);
                break;
            }
            case EmfPlusObjectTypeImageAttributes:
            {
                SAL_INFO("cppcanvas.emf", "EMF+\t Object type 'image attributes' not yet implemented");
                break;
            }
            case EmfPlusObjectTypeCustomLineCap:
            {
                SAL_INFO("cppcanvas.emf", "EMF+\t Object type 'custom line cap' not yet implemented");
                break;
            }
            default:
            {
                SAL_INFO("cppcanvas.emf", "EMF+\tObject unhandled flags: 0x" << std::hex << (flags & 0xff00) << std::dec);
            }
        }
    }

    void EmfPlusHelperData::ReadPoint(SvStream& s, float& x, float& y, sal_uInt32 flags)
    {
        if (flags & 0x800)
        {
            // specifies a location in the coordinate space that is relative to
            // the location specified by the previous element in the array. In the case of the first element in
            // PointData, a previous location at coordinates (0,0) is assumed.
            SAL_WARN("cppcanvas.emf", "EMF+\t\t TODO Relative coordinates bit detected. Implement parse EMFPlusPointR");
        }

        if (flags & 0x4000)
        {
            sal_Int16 ix, iy;

            s.ReadInt16(ix).ReadInt16(iy);

            x = ix;
            y = iy;
        }
        else
        {
            s.ReadFloat(x).ReadFloat(y);
        }
    }

    void EmfPlusHelperData::ReadRectangle(SvStream& s, float& x, float& y, float &width, float& height, bool bCompressed)
    {
        if (bCompressed)
        {
            sal_Int16 ix, iy, iw, ih;

            s.ReadInt16(ix).ReadInt16(iy).ReadInt16(iw).ReadInt16(ih);

            x = ix;
            y = iy;
            width = iw;
            height = ih;
        }
        else
        {
            s.ReadFloat(x).ReadFloat(y).ReadFloat(width).ReadFloat(height);
        }
    }

    bool EmfPlusHelperData::readXForm(SvStream& rIn, basegfx::B2DHomMatrix& rTarget)
    {
        rTarget.identity();

        if (sizeof(float) != 4)
        {
            OSL_FAIL("EnhWMFReader::sizeof( float ) != 4");
            return false;
        }
        else
        {
            float eM11(0.0);
            float eM12(0.0);
            float eM21(0.0);
            float eM22(0.0);
            float eDx(0.0);
            float eDy(0.0);
#ifdef OSL_BIGENDIAN
            eM11 = GetSwapFloat(rIn);
            eM12 = GetSwapFloat(rIn);
            eM21 = GetSwapFloat(rIn);
            eM22 = GetSwapFloat(rIn);
            eDx = GetSwapFloat(rIn);
            eDy = GetSwapFloat(rIn);
#else
            rIn.ReadFloat(eM11).ReadFloat(eM12).ReadFloat(eM21).ReadFloat(eM22).ReadFloat(eDx).ReadFloat(eDy);
#endif
            rTarget = basegfx::B2DHomMatrix(
                eM11, eM21, eDx,
                eM12, eM22, eDy);
        }

        return true;
    }

    void EmfPlusHelperData::mappingChanged()
    {
        // Call when mnMmX/mnMmY/mnPixX/mnPixY/mnFrameLeft/mnFrameTop/maWorldTransform/ changes.
        // Currently not used are mnHDPI/mnVDPI/mnFrameRight/mnFrameBottom. *If* these should
        // be used in the future, this method will need to be called.
        //
        // Re-calculate maMapTransform to contain the complete former transformation so that
        // it can be applied by a single matrix multiplication or be added to an encapsulated
        // primitive later
        //
        // To evtl. correct and see where this came from, please compare with the implementations
        // of EmfPlusHelperData::MapToDevice and EmfPlusHelperData::Map* in prev versions
        maMapTransform = maWorldTransform;
        maMapTransform *= basegfx::tools::createScaleB2DHomMatrix(100.0 * mnMmX / mnPixX, 100.0 * mnMmY / mnPixY);
        maMapTransform *= basegfx::tools::createTranslateB2DHomMatrix(double(-mnFrameLeft), double(-mnFrameTop));
        maMapTransform *= basegfx::tools::createScaleB2DHomMatrix(maBaseTransform.get(0, 0), maBaseTransform.get(1, 1));
    }

    ::basegfx::B2DPoint EmfPlusHelperData::Map(double ix, double iy)
    {
        // map in one step using complete MapTransform (see mappingChanged)
        return maMapTransform * ::basegfx::B2DPoint(ix, iy);
    }

    ::basegfx::B2DSize EmfPlusHelperData::MapSize(double iwidth, double iheight)
    {
        // map in one step using complete MapTransform (see mappingChanged)
        return maMapTransform * ::basegfx::B2DSize(iwidth, iheight);
    }

    void EmfPlusHelperData::EMFPPlusDrawPolygon(const ::basegfx::B2DPolyPolygon& polygon, sal_uInt32 penIndex)
    {
        const EMFPPen* pen = static_cast<EMFPPen*>(maEMFPObjects[penIndex & 0xff].get());
        SAL_WARN_IF(!pen, "cppcanvas.emf", "emf+ missing pen");

        if (pen && polygon.count())
        {
            mrTargetHolders.Current().append(
                new drawinglayer::primitive2d::PolyPolygonHairlinePrimitive2D(
                    polygon,
                    pen->GetColor().getBColor()));
        }
    }

    void EmfPlusHelperData::EMFPPlusFillPolygon(const ::basegfx::B2DPolyPolygon& polygon, bool isColor, sal_uInt32 brushIndexOrColor)
    {
        if (polygon.count())
        {
            if (isColor)
            {
                mrTargetHolders.Current().append(
                    new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                        polygon,
                        ::Color(0xff - (brushIndexOrColor >> 24), (brushIndexOrColor >> 16) & 0xff, (brushIndexOrColor >> 8) & 0xff, brushIndexOrColor & 0xff).getBColor()));
            }
        }
    }

    EmfPlusHelperData::EmfPlusHelperData(
        SvMemoryStream& rMS,
        wmfemfhelper::TargetHolders& rTargetHolders,
        wmfemfhelper::PropertyHolders& rPropertyHolders)
    :   maBaseTransform(),
        maWorldTransform(),
        maMapTransform(),
        maEMFPObjects(),
        mfPageScale(0.0),
        mnOriginX(0),
        mnOriginY(0),
        mnHDPI(0),
        mnVDPI(0),
        mnFrameLeft(0),
        mnFrameTop(0),
        mnFrameRight(0),
        mnFrameBottom(0),
        mnPixX(0),
        mnPixY(0),
        mnMmX(0),
        mnMmY(0),
        mbMultipart(false),
        mMFlags(0),
        mMStream(),
//        mGSStack(),
//        mGSContainerStack(),
        mrTargetHolders(rTargetHolders),
        mrPropertyHolders(rPropertyHolders)
    {
        rMS.ReadInt32(mnFrameLeft).ReadInt32(mnFrameTop).ReadInt32(mnFrameRight).ReadInt32(mnFrameBottom);
        SAL_INFO("cppcanvas.emf", "EMF+ picture frame: " << mnFrameLeft << "," << mnFrameTop << " - " << mnFrameRight << "," << mnFrameBottom);
        rMS.ReadInt32(mnPixX).ReadInt32(mnPixY).ReadInt32(mnMmX).ReadInt32(mnMmY);
        SAL_INFO("cppcanvas.emf", "EMF+ ref device pixel size: " << mnPixX << "x" << mnPixY << " mm size: " << mnMmX << "x" << mnMmY);
        readXForm(rMS, maBaseTransform);
        mappingChanged();
    }

    EmfPlusHelperData::~EmfPlusHelperData()
    {
    }

    void EmfPlusHelperData::processEmfPlusData(
        SvMemoryStream& rMS,
        const drawinglayer::geometry::ViewInformation2D& rViewInformation)
    {
        sal_uInt64 length = rMS.GetSize();

        if (length < 12)
        {
            SAL_INFO("cppcanvas.emf", "length is less than required header size");
        }

        // 12 is minimal valid EMF+ record size; remaining bytes are padding
        while (length >= 12)
        {
            sal_uInt16 type, flags;
            sal_uInt32 size, dataSize;
            sal_uInt64 next;

            rMS.ReadUInt16(type).ReadUInt16(flags).ReadUInt32(size).ReadUInt32(dataSize);

            next = rMS.Tell() + (size - 12);

            if (size < 12)
            {
                SAL_INFO("cppcanvas.emf", "Size field is less than 12 bytes");
            }
            else if (size > length)
            {
                SAL_INFO("cppcanvas.emf", "Size field is greater than bytes left");
            }
            if (dataSize > (size - 12))
            {
                SAL_INFO("cppcanvas.emf", "DataSize field is greater than Size-12");
            }

            SAL_INFO("cppcanvas.emf", "EMF+ record size: " << size << " type: " << emfTypeToName(type) << " flags: " << flags << " data size: " << dataSize);

            if (type == EmfPlusRecordTypeObject && ((mbMultipart && (flags & 0x7fff) == (mMFlags & 0x7fff)) || (flags & 0x8000)))
            {
                if (!mbMultipart)
                {
                    mbMultipart = true;
                    mMFlags = flags;
                    mMStream.Seek(0);
                }

                OSL_ENSURE(dataSize >= 4, "No room for TotalObjectSize in EmfPlusContinuedObjectRecord");

                // 1st 4 bytes are TotalObjectSize
                mMStream.WriteBytes(static_cast<const char *>(rMS.GetData()) + rMS.Tell() + 4, dataSize - 4);
                SAL_INFO("cppcanvas.emf", "EMF+ read next object part size: " << size << " type: " << type << " flags: " << flags << " data size: " << dataSize);
            }
            else
            {
                if (mbMultipart)
                {
                    SAL_INFO("cppcanvas.emf", "EMF+ multipart record flags: " << mMFlags);
                    mMStream.Seek(0);
                    processObjectRecord(mMStream, mMFlags, 0, true);
                }

                mbMultipart = false;
            }

            if (type != EmfPlusRecordTypeObject || !(flags & 0x8000))
            {
                switch (type)
                {
                    case EmfPlusRecordTypeHeader:
                    {
                        sal_uInt32 header, version;

                        rMS.ReadUInt32(header).ReadUInt32(version).ReadInt32(mnHDPI).ReadInt32(mnVDPI);
                        SAL_INFO("cppcanvas.emf", "EMF+ Header");
                        SAL_INFO("cppcanvas.emf", "EMF+\theader: 0x" << std::hex << header << " version: " << std::dec << version << " horizontal DPI: " << mnHDPI << " vertical DPI: " << mnVDPI << " dual: " << (flags & 1));
                        break;
                    }
                    case EmfPlusRecordTypeEndOfFile:
                    {
                        SAL_INFO("cppcanvas.emf", "EMF+ EndOfFile");
                        break;
                    }
                    case EmfPlusRecordTypeGetDC:
                    {
                        SAL_INFO("cppcanvas.emf", "EMF+ GetDC");
                        SAL_INFO("cppcanvas.emf", "EMF+\talready used in svtools wmf/emf filter parser");
                        break;
                    }
                    case EmfPlusRecordTypeObject:
                    {
                        processObjectRecord(rMS, flags, dataSize);
                        break;
                    }
                    case EmfPlusRecordTypeFillPie:
                    case EmfPlusRecordTypeDrawPie:
                    case EmfPlusRecordTypeDrawArc:
                    {
                        float startAngle, sweepAngle;

                        // Silent MSVC warning C4701: potentially uninitialized local variable 'brushIndexOrColor' used
                        sal_uInt32 brushIndexOrColor = 999;

                        if (type == EmfPlusRecordTypeFillPie)
                        {
                            rMS.ReadUInt32(brushIndexOrColor);
                            SAL_INFO("cppcanvas.emf", "EMF+ FillPie colorOrIndex: " << brushIndexOrColor);
                        }
                        else if (type == EmfPlusRecordTypeDrawPie)
                        {
                            SAL_INFO("cppcanvas.emf", "EMF+ DrawPie");
                        }
                        else
                        {
                            SAL_INFO("cppcanvas.emf", "EMF+ DrawArc");
                        }

                        rMS.ReadFloat(startAngle).ReadFloat(sweepAngle);
                        float dx, dy, dw, dh;
                        ReadRectangle(rMS, dx, dy, dw, dh, bool(flags & 0x4000));
                        SAL_INFO("cppcanvas.emf", "EMF+\t RectData: " << dx << "," << dy << " " << dw << "x" << dh);
                        startAngle = 2 * M_PI*startAngle / 360;
                        sweepAngle = 2 * M_PI*sweepAngle / 360;
                        ::basegfx::B2DPoint mappedCenter(Map(dx + dw / 2, dy + dh / 2));
                        ::basegfx::B2DSize mappedSize(MapSize(dw / 2, dh / 2));
                        float endAngle = startAngle + sweepAngle;
                        startAngle = fmodf(startAngle, static_cast<float>(M_PI * 2));

                        if (startAngle < 0.0)
                        {
                            startAngle += static_cast<float>(M_PI * 2.0);
                        }

                        endAngle = fmodf(endAngle, static_cast<float>(M_PI * 2.0));

                        if (endAngle < 0.0)
                        {
                            endAngle += static_cast<float>(M_PI * 2.0);
                        }

                        if (sweepAngle < 0)
                        {
                            std::swap(endAngle, startAngle);
                        }

                        SAL_INFO("cppcanvas.emf", "EMF+\t adjusted angles: start " <<
                            (360.0*startAngle / M_PI) << ", end: " << (360.0*endAngle / M_PI) <<
                            " startAngle: " << startAngle << " sweepAngle: " << sweepAngle);

                        ::basegfx::B2DPolygon polygon = basegfx::tools::createPolygonFromEllipseSegment(
                            mappedCenter, mappedSize.getX(), mappedSize.getY(), startAngle, endAngle);

                        if (type != EmfPlusRecordTypeDrawArc)
                        {
                            polygon.append(mappedCenter);
                            polygon.setClosed(true);
                        }

                        ::basegfx::B2DPolyPolygon polyPolygon(polygon);
                        if (type == EmfPlusRecordTypeFillPie)
                        {
                            EMFPPlusFillPolygon(polyPolygon, flags & 0x8000, brushIndexOrColor);
    //                        EMFPPlusFillPolygon(polyPolygon,
    //                            rFactoryParms, rState, rCanvas, flags & 0x8000, brushIndexOrColor);
                        }
                        else
                        {
                            EMFPPlusDrawPolygon(polyPolygon, flags & 0xff);
    //                        EMFPPlusDrawPolygon(polyPolygon,
    //                            rFactoryParms, rState, rCanvas, flags & 0xff);
                        }
                    }
                    break;
                    case EmfPlusRecordTypeFillPath:
                    {
                        sal_uInt32 index = flags & 0xff;
                        sal_uInt32 brushIndexOrColor;
                        rMS.ReadUInt32(brushIndexOrColor);
                        SAL_INFO("cppcanvas.emf", "EMF+ FillPath slot: " << index);

                        EMFPPlusFillPolygon(static_cast<EMFPPath*>(maEMFPObjects[index].get())->GetPolygon(*this), flags & 0x8000, brushIndexOrColor);
    //                    EMFPPlusFillPolygon(static_cast<EMFPPath*>(maEMFPObjects[index])->GetPolygon(*this), rFactoryParms, rState, rCanvas, flags & 0x8000, brushIndexOrColor);
                    }
                    break;
                    case EmfPlusRecordTypeDrawEllipse:
                    case EmfPlusRecordTypeFillEllipse:
                    {
                        // Intentionally very bogus initial value to avoid MSVC complaining about potentially uninitialized local
                        // variable. As long as the code stays as intended, this variable will be assigned a (real) value in the case
                        // when it is later used.
                        sal_uInt32 brushIndexOrColor = 1234567;

                        if (type == EmfPlusRecordTypeFillEllipse)
                        {
                            rMS.ReadUInt32(brushIndexOrColor);
                        }

                        SAL_INFO("cppcanvas.emf", "EMF+ " << (type == EmfPlusRecordTypeFillEllipse ? "Fill" : "Draw") << "Ellipse slot: " << (flags & 0xff));
                        float dx, dy, dw, dh;
                        ReadRectangle(rMS, dx, dy, dw, dh, bool(flags & 0x4000));
                        SAL_INFO("cppcanvas.emf", "EMF+ RectData: " << dx << "," << dy << " " << dw << "x" << dh);
                        ::basegfx::B2DPoint mappedCenter(Map(dx + dw / 2, dy + dh / 2));
                        ::basegfx::B2DSize mappedSize(MapSize(dw / 2, dh / 2));
                        ::basegfx::B2DPolyPolygon polyPolygon(
                            ::basegfx::B2DPolygon(
                                ::basegfx::tools::createPolygonFromEllipse(mappedCenter, mappedSize.getX(), mappedSize.getY())));

                        if (type == EmfPlusRecordTypeFillEllipse)
                        {
                            EMFPPlusFillPolygon(polyPolygon, flags & 0x8000, brushIndexOrColor);
    //                        EMFPPlusFillPolygon(polyPolygon,
    //                            rFactoryParms, rState, rCanvas, flags & 0x8000, brushIndexOrColor);
                        }
                        else
                        {
                            EMFPPlusDrawPolygon(polyPolygon, flags & 0xff);
    //                        EMFPPlusDrawPolygon(polyPolygon,
    //                            rFactoryParms, rState, rCanvas, flags & 0xff);
                        }
                    }
                    break;
                    case EmfPlusRecordTypeFillRects:
                    case EmfPlusRecordTypeDrawRects:
                    {
                        // Silent MSVC warning C4701: potentially uninitialized local variable 'brushIndexOrColor' used
                        sal_uInt32 brushIndexOrColor = 999;
                        sal_Int32 rectangles;
                        bool isColor = (flags & 0x8000);
                        ::basegfx::B2DPolygon polygon;

                        if (EmfPlusRecordTypeFillRects == type)
                        {
                            SAL_INFO("cppcanvas.emf", "EMF+ FillRects");
                            rMS.ReadUInt32(brushIndexOrColor);
                            SAL_INFO("cppcanvas.emf", "EMF+\t" << (isColor ? "color" : "brush index") << ": 0x" << std::hex << brushIndexOrColor << std::dec);
                        }
                        else
                        {
                            SAL_INFO("cppcanvas.emf", "EMF+ DrawRects");
                        }

                        rMS.ReadInt32(rectangles);

                        for (int i = 0; i < rectangles; i++)
                        {
                            float x, y, width, height;
                            ReadRectangle(rMS, x, y, width, height, bool(flags & 0x4000));

                            polygon.append(Map(x, y));
                            polygon.append(Map(x + width, y));
                            polygon.append(Map(x + width, y + height));
                            polygon.append(Map(x, y + height));
                            polygon.append(Map(x, y));

                            SAL_INFO("cppcanvas.emf", "EMF+\trectangle: " << x << ", " << width << "x" << height);

                            ::basegfx::B2DPolyPolygon polyPolygon(polygon);
                            if (type == EmfPlusRecordTypeFillRects)
                            {
                                EMFPPlusFillPolygon(polyPolygon, isColor, brushIndexOrColor);
    //                            EMFPPlusFillPolygon(polyPolygon,
    //                                rFactoryParms, rState, rCanvas, isColor, brushIndexOrColor);
                            }
                            else
                            {
                                EMFPPlusDrawPolygon(polyPolygon, flags & 0xff);
    //                            EMFPPlusDrawPolygon(polyPolygon,
    //                                rFactoryParms, rState, rCanvas, flags & 0xff);
                            }
                        }
                        break;
                    }
                    case EmfPlusRecordTypeFillPolygon:
                    {
                        sal_uInt8 index = flags & 0xff;
                        sal_uInt32 brushIndexOrColor;
                        sal_Int32 points;

                        rMS.ReadUInt32(brushIndexOrColor);
                        rMS.ReadInt32(points);
                        SAL_INFO("cppcanvas.emf", "EMF+ FillPolygon in slot: " << +index << " points: " << points);
                        SAL_INFO("cppcanvas.emf", "EMF+\t: " << ((flags & 0x8000) ? "color" : "brush index") << " 0x" << std::hex << brushIndexOrColor << std::dec);

                        EMFPPath path(points, true);
                        path.Read(rMS, flags, *this);

                        EMFPPlusFillPolygon(path.GetPolygon(*this), flags & 0x8000, brushIndexOrColor);
    //                    EMFPPlusFillPolygon(path.GetPolygon(*this), rFactoryParms, rState, rCanvas, flags & 0x8000, brushIndexOrColor);

                        break;
                    }
                    case EmfPlusRecordTypeDrawLines:
                    {
                        sal_uInt32 points;
                        rMS.ReadUInt32(points);
                        SAL_INFO("cppcanvas.emf", "EMF+ DrawLines in slot: " << (flags & 0xff) << " points: " << points);
                        EMFPPath path(points, true);
                        path.Read(rMS, flags, *this);

                        // 0x2000 bit indicates whether to draw an extra line between the last point
                        // and the first point, to close the shape.
                        EMFPPlusDrawPolygon(path.GetPolygon(*this, true, (flags & 0x2000)), flags);
    //                    EMFPPlusDrawPolygon(path.GetPolygon(*this, true, (flags & 0x2000)), rFactoryParms, rState, rCanvas, flags);

                        break;
                    }
                    case EmfPlusRecordTypeDrawPath:
                    {
                        sal_uInt32 penIndex;
                        rMS.ReadUInt32(penIndex);
                        SAL_INFO("cppcanvas.emf", "EMF+ DrawPath");
                        SAL_INFO("cppcanvas.emf", "EMF+\tpen: " << penIndex);
                        EMFPPath* path = static_cast<EMFPPath*>(maEMFPObjects[flags & 0xff].get());
                        SAL_WARN_IF(!path, "cppcanvas.emf", "EmfPlusRecordTypeDrawPath missing path");

                        EMFPPlusDrawPolygon(path->GetPolygon(*this), penIndex);
    //                    EMFPPlusDrawPolygon(path->GetPolygon(*this), rFactoryParms, rState, rCanvas, penIndex);

                        break;
                    }
                    case EmfPlusRecordTypeDrawBeziers:
                    {
                        sal_uInt32 aCount;
                        float x1, y1, x2, y2, x3, y3, x4, y4;
                        ::basegfx::B2DPoint aStartPoint, aControlPointA, aControlPointB, aEndPoint;
                        ::basegfx::B2DPolygon aPolygon;
                        rMS.ReadUInt32(aCount);
                        SAL_INFO("cppcanvas.emf", "EMF+ DrawBeziers slot: " << (flags & 0xff) << "Number of points: " << aCount);
                        SAL_WARN_IF((aCount - 1) % 3 != 0, "cppcanvas.emf", "EMF+\t Bezier Draw not support number of points other than 4, 7, 10, 13, 16...");

                        if (aCount < 4)
                        {
                            SAL_WARN("cppcanvas.emf", "EMF+\t Bezier Draw does not support less than 4 points. Number of points: " << aCount);
                            break;
                        }

                        ReadPoint(rMS, x1, y1, flags);
                        // We need to add first starting point
                        aStartPoint = Map(x1, y1);
                        aPolygon.append(aStartPoint);

                        for (sal_uInt32 i = 4; i <= aCount; i += 3)
                        {
                            ReadPoint(rMS, x2, y2, flags);
                            ReadPoint(rMS, x3, y3, flags);
                            ReadPoint(rMS, x4, y4, flags);

                            SAL_INFO("cppcanvas.emf", "EMF+\t Bezier points: " << x1 << "," << y1 << " " << x2 << "," << y2 << " " << x3 << "," << y3 << " " << x4 << "," << y4);

                            aStartPoint = Map(x1, y1);
                            aControlPointA = Map(x2, y2);
                            aControlPointB = Map(x3, y3);
                            aEndPoint = Map(x4, y4);

                            ::basegfx::B2DCubicBezier cubicBezier(aStartPoint, aControlPointA, aControlPointB, aEndPoint);
                            cubicBezier.adaptiveSubdivideByDistance(aPolygon, 10.0);

                            EMFPPlusDrawPolygon(::basegfx::B2DPolyPolygon(aPolygon), flags & 0xff);
    //                        EMFPPlusDrawPolygon(::basegfx::B2DPolyPolygon(aPolygon), rFactoryParms,
    //                            rState, rCanvas, flags & 0xff);
                            // The ending coordinate of one Bezier curve is the starting coordinate of the next.
                            x1 = x4;
                            y1 = y4;
                        }
                        break;
                    }
                    case EmfPlusRecordTypeDrawImage:
                    case EmfPlusRecordTypeDrawImagePoints:
                    {
                        sal_uInt32 attrIndex;
                        sal_Int32 sourceUnit;
                        rMS.ReadUInt32(attrIndex).ReadInt32(sourceUnit);
                        SAL_INFO("cppcanvas.emf", "EMF+ " << (type == EmfPlusRecordTypeDrawImagePoints ? "DrawImagePoints" : "DrawImage") << "attributes index: " << attrIndex << "source unit: " << sourceUnit);
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO: use image attributes");

                        if (sourceUnit == 2 && maEMFPObjects[flags & 0xff].get())
                        {
                            // we handle only GraphicsUnit.Pixel now
                            EMFPImage& image = *static_cast<EMFPImage *>(maEMFPObjects[flags & 0xff].get());
                            float sx, sy, sw, sh;
                            sal_Int32 aCount;
                            ReadRectangle(rMS, sx, sy, sw, sh);
                            ::tools::Rectangle aSource(Point(sx, sy), Size(sw, sh));
                            SAL_INFO("cppcanvas.emf", "EMF+ " << (type == EmfPlusRecordTypeDrawImagePoints ? "DrawImagePoints" : "DrawImage") << " source rectangle: " << sx << "," << sy << " " << sw << "x" << sh);
                            ::basegfx::B2DPoint aDstPoint;
                            ::basegfx::B2DSize aDstSize;
                            bool bValid = false;

                            if (type == EmfPlusRecordTypeDrawImagePoints)
                            {
                                rMS.ReadInt32(aCount);

                                if (aCount == 3)
                                {
                                    // TODO: now that we now that this value is count we should support it better
                                    float x1, y1, x2, y2, x3, y3;

                                    ReadPoint(rMS, x1, y1, flags);
                                    ReadPoint(rMS, x2, y2, flags);
                                    ReadPoint(rMS, x3, y3, flags);

                                    SAL_INFO("cppcanvas.emf", "EMF+ destination points: " << x1 << "," << y1 << " " << x2 << "," << y2 << " " << x3 << "," << y3);
                                    SAL_INFO("cppcanvas.emf", "EMF+ destination rectangle: " << x1 << "," << y1 << " " << x2 - x1 << "x" << y3 - y1);

                                    aDstPoint = Map(x1, y1);
                                    aDstSize = MapSize(x2 - x1, y3 - y1);

                                    bValid = true;
                                }
                            }
                            else if (type == EmfPlusRecordTypeDrawImage)
                            {
                                float dx, dy, dw, dh;
                                ReadRectangle(rMS, dx, dy, dw, dh, bool(flags & 0x4000));
                                SAL_INFO("cppcanvas.emf", "EMF+ destination rectangle: " << dx << "," << dy << " " << dw << "x" << dh);
                                aDstPoint = Map(dx, dy);
                                aDstSize = MapSize(dw, dh);
                                bValid = true;
                            }

                            if (bValid)
                            {
                                BitmapEx aBmp(image.graphic.GetBitmapEx());
                                aBmp.Crop(aSource);
                                Size aSize(aBmp.GetSizePixel());
                                SAL_INFO("cppcanvas.emf", "EMF+ bitmap size: " << aSize.Width() << "x" << aSize.Height());
                                if (aSize.Width() > 0 && aSize.Height() > 0)
                                {
    //                                ActionSharedPtr pBmpAction(
    //                                    internal::BitmapActionFactory::createBitmapAction(
    //                                        aBmp,
    //                                        rState.mapModeTransform * aDstPoint,
    //                                        rState.mapModeTransform * aDstSize,
    //                                        rCanvas,
    //                                        rState));
    //
    //                                if (pBmpAction) {
    //                                    maActions.push_back(MtfAction(pBmpAction,
    //                                        rFactoryParms.mrCurrActionIndex));
    //
    //                                    rFactoryParms.mrCurrActionIndex += pBmpAction->getActionCount() - 1;
    //                                }
                                }
                                else
                                {
                                    SAL_INFO("cppcanvas.emf", "EMF+ warning: empty bitmap");
                                }
                            }
                            else
                            {
                                SAL_WARN("cppcanvas.emf", "EMF+ DrawImage(Points) TODO (fixme)");
                            }
                        }
                        else
                        {
                            SAL_WARN("cppcanvas.emf", "EMF+ DrawImage(Points) TODO (fixme) - possibly unsupported source units for crop rectangle");
                        }
                        break;
                    }
                    case EmfPlusRecordTypeDrawString:
                    {
                        SAL_INFO("cppcanvas.emf", "EMF+ DrawString");
                        sal_uInt32 brushId;
                        sal_uInt32 formatId;
                        sal_uInt32 stringLength;
                        rMS.ReadUInt32(brushId).ReadUInt32(formatId).ReadUInt32(stringLength);
                        SAL_INFO("cppcanvas.emf", "EMF+ DrawString brushId: " << brushId << " formatId: " << formatId << " length: " << stringLength);

                        if (flags & 0x8000)
                        {
                            float lx, ly, lw, lh;
                            rMS.ReadFloat(lx).ReadFloat(ly).ReadFloat(lw).ReadFloat(lh);
                            SAL_INFO("cppcanvas.emf", "EMF+ DrawString layoutRect: " << lx << "," << ly << " - " << lw << "x" << lh);
                            OUString text = read_uInt16s_ToOUString(rMS, stringLength);
                            EMFPStringFormat *stringFormat = static_cast< EMFPStringFormat* >(maEMFPObjects[formatId & 0xff].get());
    //                        css::rendering::FontRequest aFontRequest;
    //
    //                        if (stringFormat)
    //                        {
    //                            LanguageTag aLanguageTag(static_cast< LanguageType >(stringFormat->language));
    //                            aFontRequest.Locale = aLanguageTag.getLocale(false);
    //                            SAL_INFO("cppcanvas.emf", "EMF+\t\t Font locale, Country:" << aLanguageTag.getCountry() << " Language:" << aLanguageTag.getLanguage());
    //                        }
    //
    //                        SAL_INFO("cppcanvas.emf", "EMF+\t\t TODO Use all string formatting attributes during drawing");
    //
    //                        double cellSize = setFont(aFontRequest, flags & 0xff, rFactoryParms, rState);
    //                        rState.textColor = COLOR(brushId);
    //
    //                        ::basegfx::B2DPoint point(Map(lx + 0.15*cellSize, ly + cellSize));
    //
    //                        ActionSharedPtr pTextAction(
    //                            TextActionFactory::createTextAction(
    //                                // position is just rough guess for now
    //                                // we should calculate it exactly from layoutRect or font
    //                                vcl::unotools::pointFromB2DPoint(point),
    //                                ::Size(),
    //                                ::Color(),
    //                                ::Size(),
    //                                ::Color(),
    //                                text,
    //                                0,
    //                                stringLength,
    //                                nullptr,
    //                                rFactoryParms.mrVDev,
    //                                rFactoryParms.mrCanvas,
    //                                rState,
    //                                rFactoryParms.mrParms,
    //                                false));
    //                        if (pTextAction)
    //                        {
    //                            SAL_INFO("cppcanvas.emf", "EMF+\t\tadd text action");
    //
    //                            maActions.push_back(
    //                                MtfAction(
    //                                    pTextAction,
    //                                    rFactoryParms.mrCurrActionIndex));
    //
    //                            rFactoryParms.mrCurrActionIndex += pTextAction->getActionCount() - 1;
    //                        }
                        }
                        else
                        {
                            SAL_WARN("cppcanvas.emf", "EMF+ DrawString TODO - drawing with brush not yet supported");
                        }
                        break;
                    }
                    case EmfPlusRecordTypeSetPageTransform:
                    {
                        rMS.ReadFloat(mfPageScale);
                        SAL_INFO("cppcanvas.emf", "EMF+ SetPageTransform");
                        SAL_INFO("cppcanvas.emf", "EMF+\tscale: " << mfPageScale << " unit: " << flags);

                        if (flags != UnitTypePixel)
                        {
                            SAL_WARN("cppcanvas.emf", "EMF+\t TODO Only UnitTypePixel is supported. ");
                        }
                        else
                        {
                            mnMmX *= mfPageScale;
                            mnMmY *= mfPageScale;
                            mappingChanged();
                        }
                        break;
                    }
                    case EmfPlusRecordTypeSetRenderingOrigin:
                    {
                        rMS.ReadInt32(mnOriginX).ReadInt32(mnOriginY);
                        SAL_INFO("cppcanvas.emf", "EMF+ SetRenderingOrigin");
                        SAL_INFO("cppcanvas.emf", "EMF+\torigin [x,y]: " << mnOriginX << "," << mnOriginY);
                        break;
                    }
                    case EmfPlusRecordTypeSetTextRenderingHint:
                    {
                        SAL_INFO("cppcanvas.emf", "EMF+ SetTextRenderingHint");
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    }
                    case EmfPlusRecordTypeSetAntiAliasMode:
                    {
                        SAL_INFO("cppcanvas.emf", "EMF+ SetAntiAliasMode");
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    }
                    case EmfPlusRecordTypeSetInterpolationMode:
                    {
                        SAL_INFO("cppcanvas.emf", "EMF+ InterpolationMode");
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    }
                    case EmfPlusRecordTypeSetPixelOffsetMode:
                    {
                        SAL_INFO("cppcanvas.emf", "EMF+ SetPixelOffsetMode");
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    }
                    case EmfPlusRecordTypeSetCompositingQuality:
                    {
                        SAL_INFO("cppcanvas.emf", "EMF+ SetCompositingQuality");
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    }
                    case EmfPlusRecordTypeSave:
                    {
                        sal_uInt32 stackIndex;
                        rMS.ReadUInt32(stackIndex);
                        SAL_INFO("cppcanvas.emf", "EMF+ Save stack index: " << stackIndex);

    //                    GraphicStatePush(mGSStack, stackIndex, rState);

                        break;
                    }
                    case EmfPlusRecordTypeRestore:
                    {
                        sal_uInt32 stackIndex;
                        rMS.ReadUInt32(stackIndex);
                        SAL_INFO("cppcanvas.emf", "EMF+ Restore stack index: " << stackIndex);

    //                    GraphicStatePop(mGSStack, stackIndex, rState);

                        break;
                    }
                    case EmfPlusRecordTypeBeginContainerNoParams:
                    {
                        sal_uInt32 stackIndex;
                        rMS.ReadUInt32(stackIndex);
                        SAL_INFO("cppcanvas.emf", "EMF+ Begin Container No Params stack index: " << stackIndex);

    //                    GraphicStatePush(mGSContainerStack, stackIndex, rState);
                        break;
                    }
                    case EmfPlusRecordTypeEndContainer:
                    {
                        sal_uInt32 stackIndex;
                        rMS.ReadUInt32(stackIndex);
                        SAL_INFO("cppcanvas.emf", "EMF+ End Container stack index: " << stackIndex);

    //                    GraphicStatePop(mGSContainerStack, stackIndex, rState);
                        break;
                    }
                    case EmfPlusRecordTypeSetWorldTransform:
                    {
                        SAL_INFO("cppcanvas.emf", "EMF+ SetWorldTransform");
                        basegfx::B2DHomMatrix transform;
                        readXForm(rMS, transform);
                        maWorldTransform = transform;
                        mappingChanged();
                        SAL_INFO("cppcanvas.emf",
                            "EMF+\tm11: " << maWorldTransform.get(0,0) << "\tm12: " << maWorldTransform.get(1,0) <<
                            "\tm21: " << maWorldTransform.get(0,1) << "\tm22: " << maWorldTransform.get(1,1) <<
                            "\tdx: " << maWorldTransform.get(0,2) << "\tdy: " << maWorldTransform.get(1,2));
                        break;
                    }
                    case EmfPlusRecordTypeResetWorldTransform:
                    {
                        SAL_INFO("cppcanvas.emf", "EMF+ ResetWorldTransform");
                        maWorldTransform.identity();
                        mappingChanged();
                        break;
                    }
                    case EmfPlusRecordTypeMultiplyWorldTransform:
                    {
                        SAL_INFO("cppcanvas.emf", "EMF+ MultiplyWorldTransform");
                        basegfx::B2DHomMatrix transform;
                        readXForm(rMS, transform);

                        SAL_INFO("cppcanvas.emf",
                            "EMF+\tmatrix m11: " << transform.get(0,0) << "m12: " << transform.get(0,1) <<
                            "EMF+\tm21: " << transform.get(1,0) << "m22: " << transform.get(1,1) <<
                            "EMF+\tdx: " << transform.get(2,0) << "dy: " << transform.get(2,1));

                        if (flags & 0x2000)
                        {
                            // post multiply
                            maWorldTransform *= transform;
                        }
                        else
                        {
                            // pre multiply
                            transform *= maWorldTransform;
                            maWorldTransform = transform;
                        }

                        mappingChanged();

                        SAL_INFO("cppcanvas.emf",
                            "EMF+\tmatrix m11: " << maWorldTransform.get(0, 0) << "m12: " << maWorldTransform.get(0, 1) <<
                            "EMF+\tm21: " << maWorldTransform.get(1, 0) << "m22: " << maWorldTransform.get(1, 1) <<
                            "EMF+\tdx: " << maWorldTransform.get(2, 0) << "dy: " << maWorldTransform.get(2, 1));
                        break;
                    }
                    case EmfPlusRecordTypeTranslateWorldTransform:
                    {
                        SAL_INFO("cppcanvas.emf", "EMF+ TranslateWorldTransform");

                        basegfx::B2DHomMatrix transform;
                        float eDx, eDy;
                        rMS.ReadFloat(eDx).ReadFloat(eDy);
                        transform.set(0, 2, eDx);
                        transform.set(1, 2, eDy);

                        SAL_INFO("cppcanvas.emf",
                            "EMF+\tmatrix m11: " << transform.get(0, 0) << "m12: " << transform.get(0, 1) <<
                            "EMF+\tm21: " << transform.get(1, 0) << "m22: " << transform.get(1, 1) <<
                            "EMF+\tdx: " << transform.get(2, 0) << "dy: " << transform.get(2, 1));

                        if (flags & 0x2000)
                        {
                            // post multiply
                            maWorldTransform *= transform;
                        }
                        else
                        {
                            // pre multiply
                            transform *= maWorldTransform;
                            maWorldTransform = transform;
                        }

                        mappingChanged();

                        SAL_INFO("cppcanvas.emf",
                            "EMF+\tmatrix m11: " << maWorldTransform.get(0, 0) << "m12: " << maWorldTransform.get(0, 1) <<
                            "EMF+\tm21: " << maWorldTransform.get(1, 0) << "m22: " << maWorldTransform.get(1, 1) <<
                            "EMF+\tdx: " << maWorldTransform.get(2, 0) << "dy: " << maWorldTransform.get(2, 1));
                        break;
                    }
                    case EmfPlusRecordTypeScaleWorldTransform:
                    {
                        basegfx::B2DHomMatrix transform;
                        float eM11, eM22;
                        rMS.ReadFloat(eM11).ReadFloat(eM22);
                        transform.set(0, 0, eM11);
                        transform.set(1, 1, eM22);

                        SAL_INFO("cppcanvas.emf", "EMF+ ScaleWorldTransform Sx: " << transform.get(0,0) << " Sy: " << transform.get(1,1));
                        SAL_INFO("cppcanvas.emf",
                            "EMF+\t m11: " << maWorldTransform.get(0,0) << ", m12: " << maWorldTransform.get(0,1) <<
                            "EMF+\t m21: " << maWorldTransform.get(1,0) << ", m22: " << maWorldTransform.get(1,1) <<
                            "EMF+\t dx: " << maWorldTransform.get(2,0) << ", dy: " << maWorldTransform.get(2,1));

                        if (flags & 0x2000)
                        {
                            // post multiply
                            maWorldTransform *= transform;
                        }
                        else
                        {
                            // pre multiply
                            transform *= maWorldTransform;
                            maWorldTransform = transform;
                        }

                        mappingChanged();

                        SAL_INFO("cppcanvas.emf",
                            "EMF+\t m11: " << maWorldTransform.get(0, 0) << ", m12: " << maWorldTransform.get(0, 1) <<
                            "EMF+\t m21: " << maWorldTransform.get(1, 0) << ", m22: " << maWorldTransform.get(1, 1) <<
                            "EMF+\t dx: " << maWorldTransform.get(2, 0) << ", dy: " << maWorldTransform.get(2, 1));
                        break;
                    }
                    case EmfPlusRecordTypeSetClipRect:
                    {
                        int combineMode = (flags >> 8) & 0xf;

                        SAL_INFO("cppcanvas.emf", "EMF+ SetClipRect combine mode: " << combineMode);
    #if OSL_DEBUG_LEVEL > 1
                        if (combineMode > 1) {
                            SAL_INFO("cppcanvas.emf", "EMF+ TODO combine mode > 1");
                        }
    #endif

                        float dx, dy, dw, dh;
                        ReadRectangle(rMS, dx, dy, dw, dh);
                        SAL_INFO("cppcanvas.emf", "EMF+ RectData: " << dx << "," << dy << " " << dw << "x" << dh);
                        ::basegfx::B2DPoint mappedPoint(Map(dx, dy));
                        ::basegfx::B2DSize mappedSize(MapSize(dw, dh));

                        ::basegfx::B2DPolyPolygon polyPolygon(
                            ::basegfx::B2DPolygon(
                                ::basegfx::tools::createPolygonFromRect(::basegfx::B2DRectangle(
                                    mappedPoint.getX(),
                                    mappedPoint.getY(),
                                    mappedPoint.getX() + mappedSize.getX(),
                                    mappedPoint.getY() + mappedSize.getY()))));

                        // use existing tooling from wmfemfhelper
                        HandleNewClipRegion(polyPolygon, mrTargetHolders, mrPropertyHolders);
                        // polyPolygon.transform(rState.mapModeTransform);
                        // updateClipping(polyPolygon, rFactoryParms, combineMode == 1);
                        break;
                    }
                    case EmfPlusRecordTypeSetClipPath:
                    {
                        int combineMode = (flags >> 8) & 0xf;
                        SAL_INFO("cppcanvas.emf", "EMF+ SetClipPath combine mode: " << combineMode);
                        SAL_INFO("cppcanvas.emf", "EMF+\tpath in slot: " << (flags & 0xff));

                        EMFPPath& path = *static_cast<EMFPPath*>(maEMFPObjects[flags & 0xff].get());
                        ::basegfx::B2DPolyPolygon& clipPoly(path.GetPolygon(*this));
                        // clipPoly.transform(rState.mapModeTransform);

                        switch (combineMode)
                        {
                            case EmfPlusCombineModeReplace:
                            case EmfPlusCombineModeIntersect:
                            case EmfPlusCombineModeUnion: // Is this, EmfPlusCombineModeXOR and EmfPlusCombineModeComplement correct?
                            case EmfPlusCombineModeXOR:
                            case EmfPlusCombineModeComplement:
                            {
                                // use existing tooling from wmfemfhelper
                                HandleNewClipRegion(clipPoly, mrTargetHolders, mrPropertyHolders);
                                // updateClipping(clipPoly, rFactoryParms, combineMode == 1);
                                break;
                            }
                            case EmfPlusCombineModeExclude:
                            {
                                // Not doing anything is better then including exactly what we wanted to exclude.
                                break;
                            }
                        }

                        break;
                    }
                    case EmfPlusRecordTypeSetClipRegion:
                    {
                        int combineMode = (flags >> 8) & 0xf;
                        SAL_INFO("cppcanvas.emf", "EMF+ SetClipRegion");
                        SAL_INFO("cppcanvas.emf", "EMF+\tregion in slot: " << (flags & 0xff) << " combine mode: " << combineMode);
                        EMFPRegion *region = static_cast<EMFPRegion*>(maEMFPObjects[flags & 0xff].get());

                        // reset clip
                        if (region && region->parts == 0 && region->initialState == EmfPlusRegionInitialStateInfinite)
                        {
                            // use existing tooling from wmfemfhelper
                            HandleNewClipRegion(::basegfx::B2DPolyPolygon(), mrTargetHolders, mrPropertyHolders);
                            // updateClipping(::basegfx::B2DPolyPolygon(), rFactoryParms, combineMode == 1);
                        }
                        else
                        {
                            SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        }
                        break;
                    }
                    case EmfPlusRecordTypeDrawDriverString:
                    {
                        SAL_INFO("cppcanvas.emf", "EMF+ DrawDriverString, flags: 0x" << std::hex << flags << std::dec);
                        sal_uInt32 brushIndexOrColor;
                        sal_uInt32 optionFlags;
                        sal_uInt32 hasMatrix;
                        sal_uInt32 glyphsCount;
                        rMS.ReadUInt32(brushIndexOrColor).ReadUInt32(optionFlags).ReadUInt32(hasMatrix).ReadUInt32(glyphsCount);
                        SAL_INFO("cppcanvas.emf", "EMF+\t: " << ((flags & 0x8000) ? "color" : "brush index") << " 0x" << std::hex << brushIndexOrColor << std::dec);
                        SAL_INFO("cppcanvas.emf", "EMF+\toption flags: 0x" << std::hex << optionFlags << std::dec);
                        SAL_INFO("cppcanvas.emf", "EMF+\thas matrix: " << hasMatrix);
                        SAL_INFO("cppcanvas.emf", "EMF+\tglyphs: " << glyphsCount);

                        if ((optionFlags & 1) && glyphsCount > 0)
                        {
                            std::unique_ptr<float[]> charsPosX(new float[glyphsCount]);
                            std::unique_ptr<float[]> charsPosY(new float[glyphsCount]);
                            OUString text = read_uInt16s_ToOUString(rMS, glyphsCount);

                            for (sal_uInt32 i = 0; i<glyphsCount; i++)
                            {
                                rMS.ReadFloat(charsPosX[i]).ReadFloat(charsPosY[i]);
                                SAL_INFO("cppcanvas.emf", "EMF+\tglyphPosition[" << i << "]: " << charsPosX[i] << "," << charsPosY[i]);
                            }

                            basegfx::B2DHomMatrix transform;

                            if (hasMatrix)
                            {
                                readXForm(rMS, transform);
                                SAL_INFO("cppcanvas.emf", "EMF+\tmatrix: " << transform.get(0,0) << ", " << transform.get(1,0) <<
                                    ", " << transform.get(0,1) << ", " << transform.get(1,1) <<
                                    ", " << transform.get(0,2) << ", " << transform.get(1,2));
                            }

    //                        rendering::FontRequest aFontRequest;
    //                        // add the text action
    //                        setFont(aFontRequest, flags & 0xff, rFactoryParms, rState);
    //
    //                        if (flags & 0x8000)
    //                            rState.textColor = COLOR(brushIndexOrColor);
    //
    //                        ::basegfx::B2DPoint point(Map(charsPosX[0], charsPosY[0]));
    //
    //                        ActionSharedPtr pTextAction(
    //                            TextActionFactory::createTextAction(
    //                                vcl::unotools::pointFromB2DPoint(point),
    //                                ::Size(),
    //                                ::Color(),
    //                                ::Size(),
    //                                ::Color(),
    //                                text,
    //                                0,
    //                                glyphsCount,
    //                                nullptr,
    //                                rFactoryParms.mrVDev,
    //                                rFactoryParms.mrCanvas,
    //                                rState,
    //                                rFactoryParms.mrParms,
    //                                false));
    //
    //                        if (pTextAction)
    //                        {
    //                            SAL_INFO("cppcanvas.emf", "EMF+\t\tadd text action");
    //
    //                            maActions.push_back(
    //                                MtfAction(
    //                                    pTextAction,
    //                                    rFactoryParms.mrCurrActionIndex));
    //
    //                            rFactoryParms.mrCurrActionIndex += pTextAction->getActionCount() - 1;
    //                        }
                        }
                        else
                        {
                            SAL_WARN("cppcanvas.emf", "EMF+\tTODO: fonts (non-unicode glyphs chars)");
                        }
                        break;
                    }
                    default:
                    {
                        SAL_WARN("cppcanvas.emf", "EMF+ TODO unhandled record type: 0x" << std::hex << type << std::dec);
                    }
                }
            }

            rMS.Seek(next);

            if (size <= length)
            {
                length -= size;
            }
            else
            {
                SAL_WARN("cppcanvas.emf", "ImplRenderer::processEMFPlus: "
                    "size " << size << " > length " << length);
                length = 0;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
