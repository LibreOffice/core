/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <unordered_map>
#include <cstring>
#include <drawingml/presetgeometrynames.hxx>
#include <memory>

namespace
{
typedef std::unordered_map<const char*, const char*, rtl::CStringHash, rtl::CStringEqual>
    PresetGeometryHashMap;

struct PresetGeometryName
{
    const char* pMsoName;
    const char* pFontworkType;
};

const PresetGeometryName pPresetGeometryNameArray[]
    = { { "textNoShape", "" },
        { "textPlain", "fontwork-plain-text" },
        { "textStop", "fontwork-stop" },
        { "textTriangle", "fontwork-triangle-up" },
        { "textTriangleInverted", "fontwork-triangle-down" },
        { "textChevron", "fontwork-chevron-up" },
        { "textChevronInverted", "fontwork-chevron-down" },
        { "textRingInside", "mso-spt142" },
        { "textRingOutside", "mso-spt143" },
        { "textArchUp", "fontwork-arch-up-curve" },
        { "textArchDown", "fontwork-arch-down-curve" },
        { "textCircle", "fontwork-circle-curve" },
        { "textButton", "fontwork-open-circle-curve" },
        { "textArchUpPour", "fontwork-arch-up-pour" },
        { "textArchDownPour", "fontwork-arch-down-pour" },
        { "textCirclePour", "fontwork-circle-pour" },
        { "textButtonPour", "fontwork-open-circle-pour" },
        { "textCurveUp", "fontwork-curve-up" },
        { "textCurveDown", "fontwork-curve-down" },
        { "textCanUp", "mso-spt174" },
        { "textCanDown", "mso-spt175" },
        { "textWave1", "fontwork-wave" },
        { "textWave2", "mso-spt157" },
        { "textDoubleWave1", "mso-spt158" },
        { "textWave4", "mso-spt159" },
        { "textInflate", "fontwork-inflate" },
        { "textDeflate", "mso-spt161" },
        { "textInflateBottom", "mso-spt162" },
        { "textDeflateBottom", "mso-spt163" },
        { "textInflateTop", "mso-spt164" },
        { "textDeflateTop", "mso-spt165" },
        { "textDeflateInflate", "mso-spt166" },
        { "textDeflateInflateDeflate", "mso-spt167" },
        { "textFadeRight", "fontwork-fade-right" },
        { "textFadeLeft", "fontwork-fade-left" },
        { "textFadeUp", "fontwork-fade-up" },
        { "textFadeDown", "fontwork-fade-down" },
        { "textSlantUp", "fontwork-slant-up" },
        { "textSlantDown", "fontwork-slant-down" },
        { "textCascadeUp", "fontwork-fade-up-and-right" },
        { "textCascadeDown", "fontwork-fade-up-and-left" } };
}

OUString PresetGeometryTypeNames::GetFontworkType(const OUString& rMsoType)
{
    static const PresetGeometryHashMap s_HashMap = []() {
        PresetGeometryHashMap aH;
        for (const auto& item : pPresetGeometryNameArray)
            aH[item.pMsoName] = item.pFontworkType;
        return aH;
    }();
    const char* pRetValue = "";
    int i, nLen = rMsoType.getLength();
    std::unique_ptr<char[]> pBuf(new char[nLen + 1]);
    for (i = 0; i < nLen; i++)
        pBuf[i] = static_cast<char>(rMsoType[i]);
    pBuf[i] = 0;
    PresetGeometryHashMap::const_iterator aHashIter(s_HashMap.find(pBuf.get()));
    if (aHashIter != s_HashMap.end())
        pRetValue = (*aHashIter).second;

    return OUString(pRetValue, strlen(pRetValue), RTL_TEXTENCODING_ASCII_US);
}

OUString PresetGeometryTypeNames::GetMsoName(const OUString& rFontworkType)
{
    static const PresetGeometryHashMap s_HashMapInv = []() {
        PresetGeometryHashMap aHInv;
        for (const auto& item : pPresetGeometryNameArray)
            aHInv[item.pFontworkType] = item.pMsoName;
        return aHInv;
    }();
    const char* pRetValue = "";
    int i, nLen = rFontworkType.getLength();
    std::unique_ptr<char[]> pBuf(new char[nLen + 1]);
    for (i = 0; i < nLen; i++)
        pBuf[i] = static_cast<char>(rFontworkType[i]);
    pBuf[i] = 0;
    PresetGeometryHashMap::const_iterator aHashIter(s_HashMapInv.find(pBuf.get()));
    if (aHashIter != s_HashMapInv.end())
        pRetValue = (*aHashIter).second;

    return OUString(pRetValue, strlen(pRetValue), RTL_TEXTENCODING_ASCII_US);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
