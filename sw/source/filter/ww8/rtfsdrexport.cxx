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

#include "rtfsdrexport.hxx"
#include <memory>
#include "rtfattributeoutput.hxx"
#include <svtools/rtfkeywd.hxx>
#include <svtools/unitconv.hxx>
#include <filter/msfilter/rtfutil.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editids.hrc>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/svdotext.hxx>
#include <svx/unoapi.hxx>
#include <vcl/cvtgrf.hxx>
#include <textboxhelper.hxx>
#include <dcontact.hxx>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>
#include <algorithm>
#include "rtfexport.hxx"

using namespace css;

RtfSdrExport::RtfSdrExport(RtfExport& rExport)
    : EscherEx(std::make_shared<EscherExGlobal>(), nullptr)
    , m_rExport(rExport)
    , m_rAttrOutput(static_cast<RtfAttributeOutput&>(m_rExport.AttrOutput()))
    , m_pSdrObject(nullptr)
    , m_nShapeType(ESCHER_ShpInst_Nil)
    , m_nShapeFlags(ShapeFlag::NONE)
    , m_aShapeStyle(200)
    , m_pShapeTypeWritten(new bool[ESCHER_ShpInst_COUNT])
{
    mnGroupLevel = 1;
    memset(m_pShapeTypeWritten.get(), 0, ESCHER_ShpInst_COUNT * sizeof(bool));
}

RtfSdrExport::~RtfSdrExport()
{
    delete mpOutStrm;
    mpOutStrm = nullptr;
}

void RtfSdrExport::OpenContainer(sal_uInt16 nEscherContainer, int nRecInstance)
{
    EscherEx::OpenContainer(nEscherContainer, nRecInstance);

    if (nEscherContainer == ESCHER_SpContainer)
    {
        m_nShapeType = ESCHER_ShpInst_Nil;
        m_aShapeStyle.setLength(0);
        m_aShapeStyle.ensureCapacity(200);
        m_aShapeProps.clear();
    }
}

void RtfSdrExport::CloseContainer()
{
    if (mRecTypes.back() == ESCHER_SpContainer)
    {
        // write the shape now when we have all the info
        sal_Int32 nShapeElement = StartShape();
        EndShape(nShapeElement);

        // cleanup
        m_nShapeType = ESCHER_ShpInst_Nil;
    }

    EscherEx::CloseContainer();
}

sal_uInt32 RtfSdrExport::EnterGroup(const OUString& /*rShapeName*/,
                                    const tools::Rectangle* /*pRect*/)
{
    m_bInGroup = true;
    return GenerateShapeId();
}

void RtfSdrExport::LeaveGroup() { m_bInGroup = false; }

void RtfSdrExport::AddShape(sal_uInt32 nShapeType, ShapeFlag nShapeFlags, sal_uInt32 /*nShapeId*/)
{
    m_nShapeType = nShapeType;
    m_nShapeFlags = nShapeFlags;
}

static sal_uInt16 impl_GetUInt16(const sal_uInt8*& pVal)
{
    sal_uInt16 nRet = *pVal++;
    nRet += (*pVal++) << 8;
    return nRet;
}

static sal_Int32 impl_GetPointComponent(const sal_uInt8*& pVal, std::size_t& rVerticesPos,
                                        sal_uInt16 nPointSize)
{
    sal_Int32 nRet = 0;
    if ((nPointSize == 0xfff0) || (nPointSize == 4))
    {
        sal_uInt16 nUnsigned = *pVal++;
        nUnsigned += (*pVal++) << 8;
        rVerticesPos += 2;

        nRet = sal_Int16(nUnsigned);
    }
    else if (nPointSize == 8)
    {
        sal_uInt32 nUnsigned = *pVal++;
        nUnsigned += (*pVal++) << 8;
        nUnsigned += (*pVal++) << 16;
        nUnsigned += (*pVal++) << 24;
        rVerticesPos += 4;

        nRet = nUnsigned;
    }

    return nRet;
}

void RtfSdrExport::Commit(EscherPropertyContainer& rProps, const tools::Rectangle& rRect)
{
    if (m_nShapeType == ESCHER_ShpInst_Nil)
        return;

    if (m_nShapeType == ESCHER_ShpInst_Line)
        AddLineDimensions(rRect);
    else
        AddRectangleDimensions(m_aShapeStyle, rRect);

    // properties
    const EscherProperties& rOpts = rProps.GetOpts();
    for (const auto& rOpt : rOpts)
    {
        sal_uInt16 nId = (rOpt.nPropId & 0x0FFF);

        switch (nId)
        {
            case ESCHER_Prop_WrapText:
            {
                int nWrapType = 0;
                switch (rOpt.nPropValue)
                {
                    case ESCHER_WrapSquare:
                        nWrapType = 2;
                        break;
                    case ESCHER_WrapByPoints:
                        nWrapType = 4;
                        break;
                    case ESCHER_WrapNone:
                        nWrapType = 3;
                        break;
                    case ESCHER_WrapTopBottom:
                        nWrapType = 1;
                        break;
                    case ESCHER_WrapThrough:
                        nWrapType = 5;
                        break;
                }
                if (nWrapType)
                    m_aShapeStyle.append(OOO_STRING_SVTOOLS_RTF_SHPWR)
                        .append(static_cast<sal_Int32>(nWrapType));
            }
            break;
            case ESCHER_Prop_fillColor:
                m_aShapeProps.insert(
                    std::pair<OString, OString>("fillColor", OString::number(rOpt.nPropValue)));
                break;
            case ESCHER_Prop_fillBackColor:
                m_aShapeProps.insert(
                    std::pair<OString, OString>("fillBackColor", OString::number(rOpt.nPropValue)));
                break;
            case ESCHER_Prop_AnchorText:
                m_aShapeProps.insert(
                    std::pair<OString, OString>("anchorText", OString::number(rOpt.nPropValue)));
                break;
            case ESCHER_Prop_fNoFillHitTest:
                if (rOpt.nPropValue)
                    m_aShapeProps.insert(
                        std::pair<OString, OString>("fNoFillHitTest", OString::number(1)));
                break;
            case ESCHER_Prop_fNoLineDrawDash:
                // for some reason the value is set to 0x90000 if lines are switched off
                if (rOpt.nPropValue == 0x90000)
                    m_aShapeProps.insert(std::pair<OString, OString>("fLine", OString::number(0)));
                break;
            case ESCHER_Prop_lineColor:
                m_aShapeProps.insert(
                    std::pair<OString, OString>("lineColor", OString::number(rOpt.nPropValue)));
                break;
            case ESCHER_Prop_lineBackColor:
                m_aShapeProps.insert(
                    std::pair<OString, OString>("lineBackColor", OString::number(rOpt.nPropValue)));
                break;
            case ESCHER_Prop_lineJoinStyle:
                m_aShapeProps.insert(
                    std::pair<OString, OString>("lineJoinStyle", OString::number(rOpt.nPropValue)));
                break;
            case ESCHER_Prop_fshadowObscured:
                if (rOpt.nPropValue)
                    m_aShapeProps.insert(std::pair<OString, OString>("fshadowObscured", "1"));
                break;
            case ESCHER_Prop_geoLeft:
            case ESCHER_Prop_geoTop:
            {
                sal_uInt32 nLeft = 0;
                sal_uInt32 nTop = 0;

                if (nId == ESCHER_Prop_geoLeft)
                {
                    nLeft = rOpt.nPropValue;
                    rProps.GetOpt(ESCHER_Prop_geoTop, nTop);
                }
                else
                {
                    nTop = rOpt.nPropValue;
                    rProps.GetOpt(ESCHER_Prop_geoLeft, nLeft);
                }

                m_aShapeProps.insert(
                    std::pair<OString, OString>("geoLeft", OString::number(sal_Int32(nLeft))));
                m_aShapeProps.insert(
                    std::pair<OString, OString>("geoTop", OString::number(sal_Int32(nTop))));
            }
            break;

            case ESCHER_Prop_geoRight:
            case ESCHER_Prop_geoBottom:
            {
                sal_uInt32 nLeft = 0;
                sal_uInt32 nRight = 0;
                sal_uInt32 nTop = 0;
                sal_uInt32 nBottom = 0;
                rProps.GetOpt(ESCHER_Prop_geoLeft, nLeft);
                rProps.GetOpt(ESCHER_Prop_geoTop, nTop);

                if (nId == ESCHER_Prop_geoRight)
                {
                    nRight = rOpt.nPropValue;
                    rProps.GetOpt(ESCHER_Prop_geoBottom, nBottom);
                }
                else
                {
                    nBottom = rOpt.nPropValue;
                    rProps.GetOpt(ESCHER_Prop_geoRight, nRight);
                }

                m_aShapeProps.insert(std::pair<OString, OString>(
                    "geoRight", OString::number(sal_Int32(nRight) - sal_Int32(nLeft))));
                m_aShapeProps.insert(std::pair<OString, OString>(
                    "geoBottom", OString::number(sal_Int32(nBottom) - sal_Int32(nTop))));
            }
            break;
            case ESCHER_Prop_pVertices:
            case ESCHER_Prop_pSegmentInfo:
            {
                EscherPropSortStruct aVertices;
                EscherPropSortStruct aSegments;

                if (rProps.GetOpt(ESCHER_Prop_pVertices, aVertices)
                    && rProps.GetOpt(ESCHER_Prop_pSegmentInfo, aSegments)
                    && aVertices.nProp.size() >= 6 && aSegments.nProp.size() >= 6)
                {
                    const sal_uInt8* pVerticesIt = &aVertices.nProp[0] + 6;
                    std::size_t nVerticesPos = 6;
                    const sal_uInt8* pSegmentIt = &aSegments.nProp[0];

                    OStringBuffer aSegmentInfo(512);
                    OStringBuffer aVerticies(512);

                    sal_uInt16 nPointSize = aVertices.nProp[4] + (aVertices.nProp[5] << 8);

                    // number of segments
                    sal_uInt16 nSegments = impl_GetUInt16(pSegmentIt);
                    sal_Int32 nVertices = 0;
                    aSegmentInfo.append("2;").append(static_cast<sal_Int32>(nSegments));
                    pSegmentIt += 4;

                    for (; nSegments; --nSegments)
                    {
                        sal_uInt16 nSeg = impl_GetUInt16(pSegmentIt);

                        // The segment type is stored in the upper 3 bits
                        // and segment count is stored in the lower 13
                        // bits.
                        unsigned char nSegmentType = (nSeg & 0xE000) >> 13;
                        unsigned short nSegmentCount = nSeg & 0x03FF;

                        aSegmentInfo.append(';').append(static_cast<sal_Int32>(nSeg));
                        switch (nSegmentType)
                        {
                            case msopathLineTo:
                                for (unsigned short i = 0; i < nSegmentCount; ++i)
                                {
                                    sal_Int32 nX = impl_GetPointComponent(pVerticesIt, nVerticesPos,
                                                                          nPointSize);
                                    sal_Int32 nY = impl_GetPointComponent(pVerticesIt, nVerticesPos,
                                                                          nPointSize);
                                    aVerticies.append(";(")
                                        .append(nX)
                                        .append(",")
                                        .append(nY)
                                        .append(")");
                                    nVertices++;
                                }
                                break;
                            case msopathMoveTo:
                            {
                                sal_Int32 nX
                                    = impl_GetPointComponent(pVerticesIt, nVerticesPos, nPointSize);
                                sal_Int32 nY
                                    = impl_GetPointComponent(pVerticesIt, nVerticesPos, nPointSize);
                                aVerticies.append(";(").append(nX).append(",").append(nY).append(
                                    ")");
                                nVertices++;
                                break;
                            }
                            case msopathCurveTo:
                                for (unsigned short j = 0; j < nSegmentCount; ++j)
                                {
                                    for (int i = 0; i < 3; i++)
                                    {
                                        sal_Int32 nX = impl_GetPointComponent(
                                            pVerticesIt, nVerticesPos, nPointSize);
                                        sal_Int32 nY = impl_GetPointComponent(
                                            pVerticesIt, nVerticesPos, nPointSize);
                                        aVerticies.append(";(")
                                            .append(nX)
                                            .append(",")
                                            .append(nY)
                                            .append(")");
                                        nVertices++;
                                    }
                                }
                                break;
                            case msopathEscape:
                            {
                                // If the segment type is msopathEscape, the lower 13 bits are
                                // divided in a 5 bit escape code and 8 bit
                                // vertex count (not segment count!)
                                unsigned char nVertexCount = nSegmentCount & 0x00FF;
                                nVerticesPos += nVertexCount;
                                break;
                            }
                            case msopathClientEscape:
                            case msopathClose:
                            case msopathEnd:
                                break;
                            default:
                                SAL_WARN("sw.rtf", "Totally b0rked");
                                break;
                        }
                    }

                    if (!aVerticies.isEmpty())
                    {
                        // We know the number of vertices at the end only, so we have to prepend them here.
                        m_aShapeProps.insert(std::pair<OString, OString>(
                            "pVerticies",
                            "8;" + OString::number(nVertices) + aVerticies.makeStringAndClear()));
                    }
                    if (!aSegmentInfo.isEmpty())
                        m_aShapeProps.insert(std::pair<OString, OString>(
                            "pSegmentInfo", aSegmentInfo.makeStringAndClear()));
                }
                else
                    SAL_INFO(
                        "sw.rtf",
                        OSL_THIS_FUNC
                            << ": unhandled shape path, missing either pVertices or pSegmentInfo");
            }
            break;
            case ESCHER_Prop_shapePath:
                // noop, we use pSegmentInfo instead
                break;
            case ESCHER_Prop_fFillOK:
                if (!rOpt.nPropValue)
                    m_aShapeProps.insert(std::pair<OString, OString>("fFillOK", "0"));
                break;
            case ESCHER_Prop_dxTextLeft:
                m_aShapeProps.insert(
                    std::pair<OString, OString>("dxTextLeft", OString::number(rOpt.nPropValue)));
                break;
            case ESCHER_Prop_dyTextTop:
                m_aShapeProps.insert(
                    std::pair<OString, OString>("dyTextTop", OString::number(rOpt.nPropValue)));
                break;
            case ESCHER_Prop_dxTextRight:
                m_aShapeProps.insert(
                    std::pair<OString, OString>("dxTextRight", OString::number(rOpt.nPropValue)));
                break;
            case ESCHER_Prop_dyTextBottom:
                m_aShapeProps.insert(
                    std::pair<OString, OString>("dyTextBottom", OString::number(rOpt.nPropValue)));
                break;
            case ESCHER_Prop_FitTextToShape:
                // Size text to fit shape size: not supported by RTF
                break;
            case ESCHER_Prop_adjustValue:
                m_aShapeProps.insert(
                    std::pair<OString, OString>("adjustValue", OString::number(rOpt.nPropValue)));
                break;
            case ESCHER_Prop_txflTextFlow:
                m_aShapeProps.insert(
                    std::pair<OString, OString>("txflTextFlow", OString::number(rOpt.nPropValue)));
                break;
            case ESCHER_Prop_fillType:
                m_aShapeProps.insert(
                    std::pair<OString, OString>("fillType", OString::number(rOpt.nPropValue)));
                break;
            case ESCHER_Prop_fillOpacity:
                m_aShapeProps.insert(
                    std::pair<OString, OString>("fillOpacity", OString::number(rOpt.nPropValue)));
                break;
            case ESCHER_Prop_fillBlip:
            {
                OStringBuffer aBuf;
                aBuf.append('{')
                    .append(OOO_STRING_SVTOOLS_RTF_PICT)
                    .append(OOO_STRING_SVTOOLS_RTF_PNGBLIP)
                    .append(SAL_NEWLINE_STRING);
                int nHeaderSize
                    = 25; // The first bytes are WW8-specific, we're only interested in the PNG
                aBuf.append(msfilter::rtfutil::WriteHex(&rOpt.nProp[0] + nHeaderSize,
                                                        rOpt.nProp.size() - nHeaderSize));
                aBuf.append('}');
                m_aShapeProps.insert(
                    std::pair<OString, OString>("fillBlip", aBuf.makeStringAndClear()));
            }
            break;
            default:
                SAL_INFO("sw.rtf", OSL_THIS_FUNC << ": unhandled property: " << nId
                                                 << " (value: " << rOpt.nPropValue << ")");
                break;
        }
    }
}

void RtfSdrExport::AddLineDimensions(const tools::Rectangle& rRectangle)
{
    // We get the position relative to (the current?) character
    m_aShapeProps.insert(std::pair<OString, OString>("posrelh", "3"));

    if (m_nShapeFlags & ShapeFlag::FlipV)
        m_aShapeProps.insert(std::pair<OString, OString>("fFlipV", "1"));

    if (m_nShapeFlags & ShapeFlag::FlipH)
        m_aShapeProps.insert(std::pair<OString, OString>("fFlipH", "1"));

    // the actual dimensions
    m_aShapeStyle.append(OOO_STRING_SVTOOLS_RTF_SHPLEFT).append(rRectangle.Left());
    m_aShapeStyle.append(OOO_STRING_SVTOOLS_RTF_SHPTOP).append(rRectangle.Top());
    m_aShapeStyle.append(OOO_STRING_SVTOOLS_RTF_SHPRIGHT).append(rRectangle.Right());
    m_aShapeStyle.append(OOO_STRING_SVTOOLS_RTF_SHPBOTTOM).append(rRectangle.Bottom());
}

void RtfSdrExport::AddRectangleDimensions(OStringBuffer& rBuffer,
                                          const tools::Rectangle& rRectangle)
{
    // We get the position relative to (the current?) character
    m_aShapeProps.insert(std::pair<OString, OString>("posrelh", "3"));

    rBuffer.append(OOO_STRING_SVTOOLS_RTF_SHPLEFT).append(rRectangle.Left());
    rBuffer.append(OOO_STRING_SVTOOLS_RTF_SHPTOP).append(rRectangle.Top());
    rBuffer.append(OOO_STRING_SVTOOLS_RTF_SHPRIGHT).append(rRectangle.Right());
    rBuffer.append(OOO_STRING_SVTOOLS_RTF_SHPBOTTOM).append(rRectangle.Bottom());
}

extern const char* pShapeTypes[];

static void lcl_AppendSP(OStringBuffer& rRunText, const char cName[], const OString& rValue)
{
    rRunText.append('{')
        .append(OOO_STRING_SVTOOLS_RTF_SP)
        .append('{')
        .append(OOO_STRING_SVTOOLS_RTF_SN " ")
        .append(cName)
        .append('}')
        .append('{')
        .append(OOO_STRING_SVTOOLS_RTF_SV " ")
        .append(rValue)
        .append('}')
        .append('}');
}

void RtfSdrExport::impl_writeGraphic()
{
    // Get the Graphic object from the Sdr one.
    uno::Reference<drawing::XShape> xShape
        = GetXShapeForSdrObject(const_cast<SdrObject*>(m_pSdrObject));
    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);

    uno::Reference<graphic::XGraphic> xGraphic;

    Graphic aGraphic;

    try
    {
        xPropertySet->getPropertyValue("Graphic") >>= xGraphic;
    }
    catch (beans::UnknownPropertyException const&)
    {
        DBG_UNHANDLED_EXCEPTION("sw.rtf");
    }

    if (xGraphic.is())
    {
        aGraphic = Graphic(xGraphic);
    }

    // Export it to a stream.
    SvMemoryStream aStream;
    (void)GraphicConverter::Export(aStream, aGraphic, ConvertDataFormat::PNG);
    sal_uInt32 nSize = aStream.TellEnd();
    auto pGraphicAry = static_cast<sal_uInt8 const*>(aStream.GetData());

    Size aMapped(aGraphic.GetPrefSize());

    // Add it to the properties.
    RtfStringBuffer aBuf;
    aBuf->append('{').append(OOO_STRING_SVTOOLS_RTF_PICT).append(OOO_STRING_SVTOOLS_RTF_PNGBLIP);
    aBuf->append(OOO_STRING_SVTOOLS_RTF_PICW).append(sal_Int32(aMapped.Width()));
    aBuf->append(OOO_STRING_SVTOOLS_RTF_PICH)
        .append(sal_Int32(aMapped.Height()))
        .append(SAL_NEWLINE_STRING);
    aBuf->append(msfilter::rtfutil::WriteHex(pGraphicAry, nSize));
    aBuf->append('}');
    m_aShapeProps.insert(std::pair<OString, OString>("pib", aBuf.makeStringAndClear()));
}

sal_Int32 RtfSdrExport::StartShape()
{
    if (m_nShapeType == ESCHER_ShpInst_Nil)
        return -1;

    m_aShapeProps.insert(std::pair<OString, OString>("shapeType", OString::number(m_nShapeType)));
    if (ESCHER_ShpInst_PictureFrame == m_nShapeType)
        impl_writeGraphic();

    m_rAttrOutput.RunText().append('{').append(OOO_STRING_SVTOOLS_RTF_SHP);
    m_rAttrOutput.RunText()
        .append('{')
        .append(OOO_STRING_SVTOOLS_RTF_IGNORE)
        .append(OOO_STRING_SVTOOLS_RTF_SHPINST);

    m_rAttrOutput.RunText().append(m_aShapeStyle.makeStringAndClear());
    // Ignore \shpbxpage, \shpbxmargin, and \shpbxcolumn, in favor of the posrelh property.
    m_rAttrOutput.RunText().append(OOO_STRING_SVTOOLS_RTF_SHPBXIGNORE);
    // Ignore \shpbypage, \shpbymargin, and \shpbycolumn, in favor of the posrelh property.
    m_rAttrOutput.RunText().append(OOO_STRING_SVTOOLS_RTF_SHPBYIGNORE);

    // Write ZOrder.
    if (!m_bInGroup)
    {
        // Order inside the group shape is not relevant for the flat shape list
        // we write.
        m_rAttrOutput.RunText().append(OOO_STRING_SVTOOLS_RTF_SHPZ);
        m_rAttrOutput.RunText().append(OString::number(m_pSdrObject->GetOrdNum()));
    }

    for (auto it = m_aShapeProps.rbegin(); it != m_aShapeProps.rend(); ++it)
        lcl_AppendSP(m_rAttrOutput.RunText(), (*it).first.getStr(), (*it).second);

    lcl_AppendSP(m_rAttrOutput.RunText(), "wzDescription",
                 msfilter::rtfutil::OutString(m_pSdrObject->GetDescription(),
                                              m_rExport.GetCurrentEncoding()));
    lcl_AppendSP(
        m_rAttrOutput.RunText(), "wzName",
        msfilter::rtfutil::OutString(m_pSdrObject->GetName(), m_rExport.GetCurrentEncoding()));

    // now check if we have some text
    const SwFrameFormat* pShape = FindFrameFormat(m_pSdrObject);
    if (pShape)
    {
        if (SwFrameFormat* pTextBox
            = SwTextBoxHelper::getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT))
        {
            ww8::Frame* pFrame = nullptr;
            for (auto& rFrame : m_rExport.m_aFrames)
            {
                if (pTextBox == &rFrame.GetFrameFormat())
                {
                    pFrame = &rFrame;
                    break;
                }
            }

            if (pFrame)
                m_rAttrOutput.writeTextFrame(*pFrame, /*bTextBox=*/true);
            return m_nShapeType;
        }
    }

    auto pTextObj = dynamic_cast<const SdrTextObj*>(m_pSdrObject);
    if (pTextObj)
    {
        const OutlinerParaObject* pParaObj = nullptr;
        std::unique_ptr<const OutlinerParaObject> pOwnedParaObj;

        /*
        #i13885#
        When the object is actively being edited, that text is not set into
        the objects normal text object, but lives in a separate object.
        */
        if (pTextObj->IsTextEditActive())
        {
            pOwnedParaObj = pTextObj->GetEditOutlinerParaObject();
            pParaObj = pOwnedParaObj.get();
        }
        else
        {
            pParaObj = pTextObj->GetOutlinerParaObject();
        }

        if (pParaObj)
        {
            // this is reached only in case some text is attached to the shape
            // Watermark or TextBox?
            if (pTextObj->TakeObjNameSingul().match("Text Frame"))
                WriteOutliner(*pParaObj, TXT_HFTXTBOX);
            else
            {
                const EditTextObject& rEditObj = pParaObj->GetTextObject();
                const SfxItemSet& rItemSet = rEditObj.GetParaAttribs(0);

                lcl_AppendSP(m_rAttrOutput.RunText(), "gtextUNICODE",
                             msfilter::rtfutil::OutString(rEditObj.GetText(0),
                                                          m_rExport.GetCurrentEncoding()));

                auto pFontFamily
                    = static_cast<const SvxFontItem*>(rItemSet.GetItem(SID_ATTR_CHAR_FONT));
                if (pFontFamily)
                {
                    lcl_AppendSP(m_rAttrOutput.RunText(), "gtextFont",
                                 msfilter::rtfutil::OutString(pFontFamily->GetFamilyName(),
                                                              m_rExport.GetCurrentEncoding()));
                }

                auto pFontHeight = static_cast<const SvxFontHeightItem*>(
                    rItemSet.GetItem(SID_ATTR_CHAR_FONTHEIGHT));
                if (pFontHeight)
                {
                    long nFontHeight = TransformMetric(pFontHeight->GetHeight(), FieldUnit::TWIP,
                                                       FieldUnit::POINT);
                    lcl_AppendSP(
                        m_rAttrOutput.RunText(), "gtextSize",
                        msfilter::rtfutil::OutString(OUString::number(nFontHeight * RTF_MULTIPLIER),
                                                     m_rExport.GetCurrentEncoding()));
                }

                // RTF angle: 0-360 * 2^16  clockwise
                // LO  angle: 0-360 * 100   counter-clockwise
                sal_Int32 nRotation
                    = -1 * pTextObj->GetGeoStat().nRotationAngle * RTF_MULTIPLIER / 100;
                lcl_AppendSP(m_rAttrOutput.RunText(), "rotation",
                             msfilter::rtfutil::OutString(OUString::number(nRotation),
                                                          m_rExport.GetCurrentEncoding()));
            }
        }
    }

    return m_nShapeType;
}

void RtfSdrExport::WriteOutliner(const OutlinerParaObject& rParaObj, TextTypes eType)
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " start");

    const EditTextObject& rEditObj = rParaObj.GetTextObject();
    MSWord_SdrAttrIter aAttrIter(m_rExport, rEditObj, eType);

    sal_Int32 nPara = rEditObj.GetParagraphCount();

    bool bShape = eType == TXT_HFTXTBOX;
    if (bShape)
        m_rAttrOutput.RunText().append('{').append(OOO_STRING_SVTOOLS_RTF_SHPTXT).append(' ');
    for (sal_Int32 n = 0; n < nPara; ++n)
    {
        if (n)
            aAttrIter.NextPara(n);

        rtl_TextEncoding eChrSet = aAttrIter.GetNodeCharSet();

        OUString aStr(rEditObj.GetText(n));
        sal_Int32 nCurrentPos = 0;
        const sal_Int32 nEnd = aStr.getLength();

        aAttrIter.OutParaAttr(false);
        m_rAttrOutput.RunText().append(m_rAttrOutput.Styles().makeStringAndClear());
        m_rAttrOutput.RunText().append(m_rAttrOutput.StylesEnd().makeStringAndClear());

        do
        {
            const sal_Int32 nNextAttr = std::min(aAttrIter.WhereNext(), nEnd);
            rtl_TextEncoding eNextChrSet = aAttrIter.GetNextCharSet();

            aAttrIter.OutAttr(nCurrentPos);
            m_rAttrOutput.RunText().append('{');
            m_rAttrOutput.RunText().append(m_rAttrOutput.Styles().makeStringAndClear());
            m_rAttrOutput.RunText().append(m_rAttrOutput.StylesEnd().makeStringAndClear());
            m_rAttrOutput.RunText().append(SAL_NEWLINE_STRING);
            bool bTextAtr = aAttrIter.IsTextAttr(nCurrentPos);
            if (!bTextAtr)
            {
                OUString aOut(aStr.copy(nCurrentPos, nNextAttr - nCurrentPos));
                m_rAttrOutput.RunText().append(msfilter::rtfutil::OutString(aOut, eChrSet));
            }

            m_rAttrOutput.RunText().append('}');

            nCurrentPos = nNextAttr;
            eChrSet = eNextChrSet;
            aAttrIter.NextPos();
        } while (nCurrentPos < nEnd);
        if (bShape || n + 1 < nPara)
            m_rAttrOutput.RunText().append(OOO_STRING_SVTOOLS_RTF_PAR);
    }
    if (bShape)
        m_rAttrOutput.RunText().append('}');

    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " end");
}

void RtfSdrExport::EndShape(sal_Int32 nShapeElement)
{
    if (nShapeElement >= 0)
    {
        // end of the shape
        m_rAttrOutput.RunText().append('}').append('}');
    }
}

void RtfSdrExport::AddSdrObject(const SdrObject& rObj)
{
    m_pSdrObject = &rObj;
    EscherEx::AddSdrObject(rObj);
}

bool RtfSdrExport::isTextBox(const SwFrameFormat& rFrameFormat)
{
    return SwTextBoxHelper::isTextBox(&rFrameFormat, RES_FLYFRMFMT);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
