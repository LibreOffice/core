/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <font/FeatureCollector.hxx>
#include <font/OpenTypeFeatureDefinitionList.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <font/OpenTypeFeatureStrings.hrc>
#include <svdata.hxx>

#include <hb-aat.h>
#include <hb-ot.h>
#include <hb-graphite2.h>

namespace vcl::font
{
bool FeatureCollector::collectGraphite()
{
    gr_face* grFace = hb_graphite2_face_get_gr_face(m_pHbFace);

    if (grFace == nullptr)
        return false;

    gr_uint16 nUILanguage = gr_uint16(m_rLanguageTag.getLanguageType());

    gr_uint16 nNumberOfFeatures = gr_face_n_fref(grFace);
    gr_feature_val* pfeatureValues
        = gr_face_featureval_for_lang(grFace, 0); // shame we don't know which lang

    for (gr_uint16 i = 0; i < nNumberOfFeatures; ++i)
    {
        const gr_feature_ref* pFeatureRef = gr_face_fref(grFace, i);
        gr_uint32 nFeatureCode = gr_fref_id(pFeatureRef);

        if (nFeatureCode == 0) // illegal feature code - skip
            continue;

        gr_uint16 nValue = gr_fref_feature_value(pFeatureRef, pfeatureValues);
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
                gr_uint16 nParamValue = gr_fref_value(pFeatureRef, j);
                aParameters.emplace_back(sal_uInt32(nParamValue), sValueLabel);
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

            m_rFontFeatures.emplace_back(nFeatureCode, vcl::font::FeatureType::Graphite);
            vcl::font::Feature& rFeature = m_rFontFeatures.back();
            rFeature.m_aDefinition = vcl::font::FeatureDefinition(
                nFeatureCode, std::move(sLabel), eFeatureParameterType, std::move(aParameters),
                int32_t(nValue));
        }
    }
    gr_featureval_destroy(pfeatureValues);
    return true;
}

void FeatureCollector::collectForTable(hb_tag_t aTableTag)
{
    unsigned int nFeatureCount
        = hb_ot_layout_table_get_feature_tags(m_pHbFace, aTableTag, 0, nullptr, nullptr);
    std::vector<hb_tag_t> aFeatureTags(nFeatureCount);
    hb_ot_layout_table_get_feature_tags(m_pHbFace, aTableTag, 0, &nFeatureCount,
                                        aFeatureTags.data());
    aFeatureTags.resize(nFeatureCount);

    for (hb_tag_t aFeatureTag : aFeatureTags)
    {
        if (OpenTypeFeatureDefinitionList().isRequired(aFeatureTag))
            continue;

        m_rFontFeatures.emplace_back();
        vcl::font::Feature& rFeature = m_rFontFeatures.back();
        rFeature.m_nCode = aFeatureTag;

        FeatureDefinition aDefinition = OpenTypeFeatureDefinitionList().getDefinition(rFeature);
        std::vector<vcl::font::FeatureParameter> aParameters{
            { 0, VclResId(STR_FONT_FEATURE_PARAM_NONE) }
        };

        unsigned int nFeatureIdx;
        if (hb_ot_layout_language_find_feature(m_pHbFace, aTableTag, 0,
                                               HB_OT_LAYOUT_DEFAULT_LANGUAGE_INDEX, aFeatureTag,
                                               &nFeatureIdx))
        {
            // ssXX and cvXX can have name ID defined for them, check for
            // them and use as appropriate.
            hb_ot_name_id_t aLabelID;
            hb_ot_name_id_t aFirstParameterID;
            unsigned nNamedParameters;
            if (hb_ot_layout_feature_get_name_ids(m_pHbFace, aTableTag, nFeatureIdx, &aLabelID,
                                                  nullptr, nullptr, &nNamedParameters,
                                                  &aFirstParameterID))
            {
                OUString sLabel = m_pFace->GetName(NameID(aLabelID), m_rLanguageTag);
                if (!sLabel.isEmpty())
                    aDefinition = vcl::font::FeatureDefinition(aFeatureTag, sLabel);

                // cvXX features can have parameters name IDs, check for
                // them and populate feature parameters as appropriate.
                for (unsigned i = 0; i < nNamedParameters; i++)
                {
                    hb_ot_name_id_t aNameID = aFirstParameterID + i;
                    OUString sName = m_pFace->GetName(NameID(aNameID), m_rLanguageTag);
                    if (!sName.isEmpty())
                        aParameters.emplace_back(uint32_t(i + 1), sName);
                    else
                        aParameters.emplace_back(uint32_t(i + 1), OUString::number(i + 1));
                }
            }

            unsigned int nAlternates = 0;
            if (aTableTag == HB_OT_TAG_GSUB)
            {
                // Collect lookups in this feature, and input glyphs for each
                // lookup, and calculate the max number of alternates they have.
                unsigned int nLookups = hb_ot_layout_feature_get_lookups(
                    m_pHbFace, aTableTag, nFeatureIdx, 0, nullptr, nullptr);
                std::vector<unsigned int> aLookups(nLookups);
                hb_ot_layout_feature_get_lookups(m_pHbFace, aTableTag, nFeatureIdx, 0, &nLookups,
                                                 aLookups.data());

                hb_set_t* pGlyphs = hb_set_create();
                for (unsigned int nLookupIdx : aLookups)
                {
                    hb_set_clear(pGlyphs);
                    hb_ot_layout_lookup_collect_glyphs(m_pHbFace, aTableTag, nLookupIdx, nullptr,
                                                       pGlyphs, nullptr, nullptr);
                    hb_codepoint_t nGlyphIdx = HB_SET_VALUE_INVALID;
                    while (hb_set_next(pGlyphs, &nGlyphIdx))
                    {
                        nAlternates
                            = std::max(nAlternates,
                                       hb_ot_layout_lookup_get_glyph_alternates(
                                           m_pHbFace, nLookupIdx, nGlyphIdx, 0, nullptr, nullptr));
                    }
                }
                hb_set_destroy(pGlyphs);
            }

            // Append the alternates to the feature parameters, keeping any
            // existing ones calculated from cvXX features above.
            for (unsigned int i = aParameters.size() - 1; i < nAlternates; i++)
                aParameters.emplace_back(uint32_t(i + 1), OUString::number(i + 1));

            if (aParameters.size() > 1)
            {
                aDefinition = vcl::font::FeatureDefinition(
                    aFeatureTag, aDefinition.getDescription(),
                    vcl::font::FeatureParameterType::ENUM, std::move(aParameters), 0);
            }
        }

        if (aDefinition)
            rFeature.m_aDefinition = std::move(aDefinition);
    }
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
        // tdf#163213: Font Features dialog should not show OpenType features if the font has "morx" table
        if (!hb_aat_layout_has_substitution(m_pHbFace))
            collectForTable(HB_OT_TAG_GSUB); // substitution
        collectForTable(HB_OT_TAG_GPOS); // positioning
        return true;
    }
}

} // end namespace vcl::font

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
