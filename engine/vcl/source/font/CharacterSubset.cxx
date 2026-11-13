/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/font/CharacterSubset.hxx>

#include <comphelper/scopeguard.hxx>
#include <tools/stream.hxx>

#include <hb-subset.h>
#include <hb.h>

namespace font
{
BinaryDataContainer subsetToCharacters(const BinaryDataContainer& rFontData,
                                       const std::set<sal_UCS4>& rCharacters)
{
    BinaryDataContainer aSubset;
    if (rFontData.isEmpty() || rCharacters.empty())
        return aSubset;

    // The blob reads the bytes in place. They outlive every face made from it here.
    hb_blob_t* pBlob = hb_blob_create(reinterpret_cast<const char*>(rFontData.getData()),
                                      static_cast<unsigned int>(rFontData.getSize()),
                                      HB_MEMORY_MODE_READONLY, nullptr, nullptr);
    comphelper::ScopeGuard aBlobGuard([&]() { hb_blob_destroy(pBlob); });

    // A font collection holds several faces. The face drawn is the first one.
    hb_face_t* pFace = hb_face_create(pBlob, 0);
    comphelper::ScopeGuard aFaceGuard([&]() { hb_face_destroy(pFace); });
    if (hb_face_get_glyph_count(pFace) == 0)
        return aSubset;

    hb_subset_input_t* pInput = hb_subset_input_create_or_fail();
    comphelper::ScopeGuard aInputGuard([&]() { hb_subset_input_destroy(pInput); });
    if (!pInput)
        return aSubset;

    // The subset keeps the glyphs the characters map to, and every glyph the layout tables reach
    // from them, such as ligatures and contextual forms.
    hb_set_t* pUnicodes = hb_subset_input_unicode_set(pInput);
    for (const sal_UCS4 nCharacter : rCharacters)
        hb_set_add(pUnicodes, nCharacter);

    hb_face_t* pSubsetFace = hb_subset_or_fail(pFace, pInput);
    comphelper::ScopeGuard aSubsetFaceGuard([&]() { hb_face_destroy(pSubsetFace); });
    if (!pSubsetFace)
        return aSubset;

    hb_blob_t* pSubsetBlob = hb_face_reference_blob(pSubsetFace);
    comphelper::ScopeGuard aSubsetBlobGuard([&]() { hb_blob_destroy(pSubsetBlob); });
    unsigned int nLength = 0;
    const char* pData = hb_blob_get_data(pSubsetBlob, &nLength);
    if (!pData || nLength == 0)
        return aSubset;

    // The container copies the bytes, so they outlive the blob.
    SvMemoryStream aStream(const_cast<char*>(pData), nLength, StreamMode::READ);
    return BinaryDataContainer(aStream, nLength);
}

std::vector<std::pair<sal_UCS4, sal_UCS4>> characterRanges(const std::set<sal_UCS4>& rCharacters)
{
    std::vector<std::pair<sal_UCS4, sal_UCS4>> aRanges;
    for (const sal_UCS4 nCharacter : rCharacters)
    {
        if (!aRanges.empty() && aRanges.back().second + 1 == nCharacter)
            aRanges.back().second = nCharacter;
        else
            aRanges.emplace_back(nCharacter, nCharacter);
    }
    return aRanges;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
