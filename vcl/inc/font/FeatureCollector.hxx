/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_FONT_FEATURECOLLECTOR_HXX
#define INCLUDED_VCL_INC_FONT_FEATURECOLLECTOR_HXX

#include <vcl/font/Feature.hxx>
#include <hb.h>
#include <i18nlangtag/lang.h>

namespace vcl
{
namespace font
{
class FeatureCollector
{
private:
    hb_face_t* const m_pHbFace;
    std::vector<vcl::font::Feature>& m_rFontFeatures;
    LanguageType const m_eLanguageType;

public:
    FeatureCollector(hb_face_t* pHbFace, std::vector<vcl::font::Feature>& rFontFeatures,
                     LanguageType eLanguageType)
        : m_pHbFace(pHbFace)
        , m_rFontFeatures(rFontFeatures)
        , m_eLanguageType(eLanguageType)
    {
    }

private:
    void collectForLanguage(hb_tag_t aTableTag, sal_uInt32 nScript, hb_tag_t aScriptTag,
                            sal_uInt32 nLanguage, hb_tag_t aLanguageTag);

    void collectForScript(hb_tag_t aTableTag, sal_uInt32 nScript, hb_tag_t aScriptTag);
    void collectForTable(hb_tag_t aTableTag);
    bool collectGraphite();

public:
    bool collect();
};

} // end namespace font
} // end namespace vcl

#endif // INCLUDED_VCL_INC_FONT_FEATURECOLLECTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
