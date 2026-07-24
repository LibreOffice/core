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

#include <sal/config.h>

#include <utility>

#include <unx/font/GenericFontFace.hxx>
#include <unx/font/GenericFont.hxx>

#include <font/LogicalFontInstance.hxx>
#include <fontattributes.hxx>

#include <hb-ot.h>

GenericFontFace::GenericFontFace(const FontAttributes& rDevFontAttributes,
                                   OString aNativeFileName, int nFaceNum, int nFaceVariation,
                                   sal_IntPtr nFontId)
    : vcl::font::PhysicalFontFace(rDevFontAttributes)
    , maNativeFileName(std::move(aNativeFileName))
    , mnFaceNum(nFaceNum)
    , mnFaceVariation(nFaceVariation)
    , mnFontId(nFontId)
{
    // prefer font with low ID
    IncreaseQualityBy(10000 - nFontId);
}

rtl::Reference<LogicalFontInstance>
GenericFontFace::CreateFontInstance(const vcl::font::FontSelectPattern& rFSD) const
{
    return new GenericFont(*this, rFSD);
}

hb_face_t* GenericFontFace::GetHbFace() const
{
    if (!mpHbFace)
    {
        hb_blob_t* pBlob = hb_blob_create_from_file(maNativeFileName.getStr());
        mpHbFace = hb_face_create(pBlob, GetFontFaceIndex());
        hb_blob_destroy(pBlob);
    }
    return mpHbFace;
}

hb_blob_t* GenericFontFace::GetHbTable(hb_tag_t nTag) const
{
    return hb_face_reference_table(mpHbFace, nTag);
}

const std::vector<vcl::font::Variation>&
GenericFontFace::GetVariations(const LogicalFontInstance&) const
{
    if (!mxVariations)
    {
        mxVariations.emplace();
        sal_uInt32 nFaceVariation = GetFontFaceVariation();
        if (nFaceVariation)
        {
            hb_face_t* pHbFace = GetHbFace();
            unsigned int nAxes = hb_ot_var_get_axis_count(pHbFace);
            if (nAxes && nFaceVariation - 1 < hb_ot_var_get_named_instance_count(pHbFace))
            {
                std::vector<hb_ot_var_axis_info_t> aInfos(nAxes);
                hb_ot_var_get_axis_infos(pHbFace, 0, &nAxes, aInfos.data());
                std::vector<float> aCoords(nAxes);
                unsigned int nCoords = nAxes;
                if (hb_ot_var_named_instance_get_design_coords(pHbFace, nFaceVariation - 1,
                                                               &nCoords, aCoords.data()))
                {
                    mxVariations->reserve(nAxes);
                    for (unsigned int i = 0; i < nAxes; ++i)
                        mxVariations->push_back({ aInfos[i].tag, aCoords[i] });
                }
            }
        }
    }

    return *mxVariations;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
