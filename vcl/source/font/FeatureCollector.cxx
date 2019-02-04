/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <font/FeatureCollector.hxx>
#include <font/OpenTypeFeatureDefinitonList.hxx>

#include <hb-ot.h>
#include <hb-graphite2.h>

namespace vcl
{
namespace font
{
bool FeatureCollector::collectGraphite()
{
    gr_face* grFace = hb_graphite2_face_get_gr_face(m_pHbFace);

    if (grFace == nullptr)
        return false;

    gr_uint16 nUILanguage = gr_uint16(m_eLanguageType);

    gr_uint16 nNumberOfFeatures = gr_face_n_fref(grFace);

    for (gr_uint16 i = 0; i < nNumberOfFeatures; ++i)
    {
        const gr_feature_ref* pFeatureRef = gr_face_fref(grFace, i);
        gr_uint32 nFeatureCode = gr_fref_id(pFeatureRef);

        if (nFeatureCode == 0) // illegal feature code - skip
            continue;

        gr_uint32 nLabelLength = 0;
        void* pLabel = gr_fref_label(pFeatureRef, &nUILanguage, gr_utf8, &nLabelLength);
        OUString sLabel(OUString::createFromAscii(static_cast<char*>(pLabel)));
        gr_label_destroy(pLabel);

        std::vector<vcl::font::FeatureParameter> aParameters;
        gr_uint16 nNumberOfValues = gr_fref_n_values(pFeatureRef);

        if (nNumberOfValues > 0)
        {
            for (gr_uint16 j = 0; j < nNumberOfValues; ++j)
            {
                gr_uint32 nValueLabelLength = 0;
                void* pValueLabel = gr_fref_value_label(pFeatureRef, j, &nUILanguage, gr_utf8,
                                                        &nValueLabelLength);
                OUString sValueLabel(OUString::createFromAscii(static_cast<char*>(pValueLabel)));
                aParameters.emplace_back(sal_uInt32(j), sValueLabel);
                gr_label_destroy(pValueLabel);
            }

            auto eFeatureParameterType = vcl::font::FeatureParameterType::ENUM;

            // Check if the parameters are boolean
            if (aParameters.size() == 2
                && (aParameters[0].getDescription() == "True"
                    || aParameters[0].getDescription() == "False"))
            {
                eFeatureParameterType = vcl::font::FeatureParameterType::BOOL;
                aParameters.clear();
            }

            m_rFontFeatures.emplace_back(
                FeatureID{ nFeatureCode, HB_OT_TAG_DEFAULT_SCRIPT, HB_OT_TAG_DEFAULT_LANGUAGE },
                vcl::font::FeatureType::Graphite);
            vcl::font::Feature& rFeature = m_rFontFeatures.back();
            rFeature.m_aDefinition = vcl::font::FeatureDefinition(
                nFeatureCode, sLabel, eFeatureParameterType, aParameters);
        }
    }
    return true;
}

void FeatureCollector::collectForLanguage(hb_tag_t aTableTag, sal_uInt32 nScript,
                                          hb_tag_t aScriptTag, sal_uInt32 nLanguage,
                                          hb_tag_t aLanguageTag)
{
    unsigned int nFeatureCount = hb_ot_layout_language_get_feature_tags(
        m_pHbFace, aTableTag, nScript, nLanguage, 0, nullptr, nullptr);
    std::vector<hb_tag_t> aFeatureTags(nFeatureCount);
    hb_ot_layout_language_get_feature_tags(m_pHbFace, aTableTag, nScript, nLanguage, 0,
                                           &nFeatureCount, aFeatureTags.data());
    aFeatureTags.resize(nFeatureCount);

    for (hb_tag_t aFeatureTag : aFeatureTags)
    {
        if (OpenTypeFeatureDefinitonList::get().isRequired(aFeatureTag))
            continue;

        m_rFontFeatures.emplace_back();
        vcl::font::Feature& rFeature = m_rFontFeatures.back();
        rFeature.m_aID = { aFeatureTag, aScriptTag, aLanguageTag };

        FeatureDefinition aDefinition
            = OpenTypeFeatureDefinitonList::get().getDefinition(aFeatureTag);
        if (aDefinition)
        {
            rFeature.m_aDefinition = aDefinition;
        }
    }
}

void FeatureCollector::collectForScript(hb_tag_t aTableTag, sal_uInt32 nScript, hb_tag_t aScriptTag)
{
    collectForLanguage(aTableTag, nScript, aScriptTag, HB_OT_LAYOUT_DEFAULT_LANGUAGE_INDEX,
                       HB_OT_TAG_DEFAULT_LANGUAGE);

    unsigned int nLanguageCount
        = hb_ot_layout_script_get_language_tags(m_pHbFace, aTableTag, nScript, 0, nullptr, nullptr);
    std::vector<hb_tag_t> aLanguageTags(nLanguageCount);
    hb_ot_layout_script_get_language_tags(m_pHbFace, aTableTag, nScript, 0, &nLanguageCount,
                                          aLanguageTags.data());
    aLanguageTags.resize(nLanguageCount);
    for (sal_uInt32 nLanguage = 0; nLanguage < sal_uInt32(nLanguageCount); ++nLanguage)
        collectForLanguage(aTableTag, nScript, aScriptTag, nLanguage, aLanguageTags[nLanguage]);
}

void FeatureCollector::collectForTable(hb_tag_t aTableTag)
{
    unsigned int nScriptCount
        = hb_ot_layout_table_get_script_tags(m_pHbFace, aTableTag, 0, nullptr, nullptr);
    std::vector<hb_tag_t> aScriptTags(nScriptCount);
    hb_ot_layout_table_get_script_tags(m_pHbFace, aTableTag, 0, &nScriptCount, aScriptTags.data());
    aScriptTags.resize(nScriptCount);

    for (sal_uInt32 nScript = 0; nScript < sal_uInt32(nScriptCount); ++nScript)
        collectForScript(aTableTag, nScript, aScriptTags[nScript]);
}

bool FeatureCollector::collect()
{
    gr_face* grFace = hb_graphite2_face_get_gr_face(m_pHbFace);

    if (grFace)
    {
        return collectGraphite();
    }
    else
    {
        collectForTable(HB_OT_TAG_GSUB); // substitution
        collectForTable(HB_OT_TAG_GPOS); // positioning
        return true;
    }
}

} // end namespace font
} // end namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
