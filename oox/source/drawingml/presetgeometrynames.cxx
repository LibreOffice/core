/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OOX_DRAWINGML_PRESETGEOMETRYNAMES_HXX
#define INCLUDED_OOX_DRAWINGML_PRESETGEOMETRYNAMES_HXX

#include <osl/mutex.hxx>
#include <rtl/string.hxx>
#include <unordered_map>
#include <cstring>
#include <drawingml/presetgeometrynames.hxx>

namespace
{
typedef std::unordered_map<const char*, const char*, rtl::CStringHash, rtl::CStringEqual>
    PresetGeometryHashMap;

static PresetGeometryHashMap* pHashMap = nullptr;
static ::osl::Mutex& getHashMapMutex()
{
    static osl::Mutex s_aHashMapProtection;
    return s_aHashMapProtection;
}

struct PresetGeometryName
{
    const char* pMsoName;
    const char* pFontworkType;
};

static const PresetGeometryName pPresetGeometryNameArray[]
    = { { "textArchUp", "fontwork-arch-up-curve" },
        { "textArchDown", "fontwork-arch-down-curve" },
        { "textCircle", "fontwork-open-circle-curve" },
        { "textButton", "fontwork-open-circle-curve" },
        { "textButtonPour", "fontwork-open-circle-pour" },
        { "textPlain", "fontwork-plain-text" },
        { "textStop", "fontwork-stop" },
        { "textTriangle", "fontwork-triangle-up" },
        { "textTriangleInverted", "fontwork-triangle-down" },
        { "textChevron", "fontwork-chevron-up" },
        { "textChevronInverted", "fontwork-chevron-down" },
        { "textRingOutside", "mso-spt143" },
        { "textRingInside", "mso-spt142" },
        { "textArchUpPour", "fontwork-arch-up-pour" },
        { "textArchDownPour", "fontwork-arch-down-pour" },
        { "textCanDown", "mso-spt175" },
        { "textWave2", "mso-spt157" },
        { "textDoubleWave1", "fontwork-wave" },
        { "textWave1", "fontwork-wave" },
        { "textCirclePour", "fontwork-circle-pour" },
        { "textCurveUp", "fontwork-curve-up" },
        { "textCurveDown", "fontwork-curve-down" },
        { "textCanUp", "mso-spt174" },
        { "textFadeUp", "fontwork-fade-up" } };
}

namespace PresetGeometryTypeNames
{
OUString GetFontworkType(const OUString& rMsoType)
{
    if (!pHashMap)
    { // init hash map
        ::osl::MutexGuard aGuard(getHashMapMutex());
        if (!pHashMap)
        {
            PresetGeometryHashMap* pH = new PresetGeometryHashMap;
            const PresetGeometryName* pPtr = pPresetGeometryNameArray;
            const PresetGeometryName* pEnd = pPtr + SAL_N_ELEMENTS(pPresetGeometryNameArray);
            for (; pPtr < pEnd; pPtr++)
                (*pH)[pPtr->pMsoName] = pPtr->pFontworkType;
            pHashMap = pH;
        }
    }
    const char* pRetValue = "";
    int i, nLen = rMsoType.getLength();
    std::unique_ptr<char[]> pBuf(new char[nLen + 1]);
    for (i = 0; i < nLen; i++)
        pBuf[i] = static_cast<char>(rMsoType[i]);
    pBuf[i] = 0;
    PresetGeometryHashMap::const_iterator aHashIter(pHashMap->find(pBuf.get()));
    if (aHashIter != pHashMap->end())
        pRetValue = (*aHashIter).second;

    return OUString(pRetValue, strlen(pRetValue), RTL_TEXTENCODING_ASCII_US);
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */