/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <font/OpenTypeFeatureDefinitonList.hxx>
#include <font/OpenTypeFeatureStrings.hrc>

#include <rtl/character.hxx>

#include <algorithm>

namespace vcl
{
namespace font
{
OpenTypeFeatureDefinitonListPrivate::OpenTypeFeatureDefinitonListPrivate() { init(); }

void OpenTypeFeatureDefinitonListPrivate::init()
{
    m_aFeatureDefinition.assign({
        { featureCode("aalt"), STR_FONT_FEATURE_ID_AALT },
        { featureCode("afrc"), STR_FONT_FEATURE_ID_AFRC },
        { featureCode("alig"), STR_FONT_FEATURE_ID_ALIG },
        { featureCode("c2pc"), STR_FONT_FEATURE_ID_C2PC },
        { featureCode("c2sc"), STR_FONT_FEATURE_ID_C2SC },
        { featureCode("calt"), STR_FONT_FEATURE_ID_CALT },
        { featureCode("case"), STR_FONT_FEATURE_ID_CASE },
        { featureCode("clig"), STR_FONT_FEATURE_ID_CLIG },
        { featureCode("cpct"), STR_FONT_FEATURE_ID_CPCT },
        { featureCode("cpsp"), STR_FONT_FEATURE_ID_CPSP },
        { featureCode("cswh"), STR_FONT_FEATURE_ID_CSWH },
        { featureCode("dcap"), STR_FONT_FEATURE_ID_DCAP },
        { featureCode("dlig"), STR_FONT_FEATURE_ID_DLIG },
        { featureCode("dnom"), STR_FONT_FEATURE_ID_DNOM },
        { featureCode("dpng"), STR_FONT_FEATURE_ID_DPNG },
        { featureCode("expt"), STR_FONT_FEATURE_ID_EXPT },
        { featureCode("falt"), STR_FONT_FEATURE_ID_FALT },
        { featureCode("frac"), STR_FONT_FEATURE_ID_FRAC,
          std::vector<FeatureParameter>{ { 1, STR_FONT_FEATURE_ID_FRAC_PARAM_1 },
                                         { 2, STR_FONT_FEATURE_ID_FRAC_PARAM_2 } } },
        { featureCode("fwid"), STR_FONT_FEATURE_ID_FWID },
        { featureCode("halt"), STR_FONT_FEATURE_ID_HALT },
        { featureCode("hist"), STR_FONT_FEATURE_ID_HIST },
        { featureCode("hkna"), STR_FONT_FEATURE_ID_HKNA },
        { featureCode("hlig"), STR_FONT_FEATURE_ID_HLIG },
        { featureCode("hngl"), STR_FONT_FEATURE_ID_HNGL },
        { featureCode("hojo"), STR_FONT_FEATURE_ID_HOJO },
        { featureCode("hwid"), STR_FONT_FEATURE_ID_HWID },
        { featureCode("ital"), STR_FONT_FEATURE_ID_ITAL },
        { featureCode("jalt"), STR_FONT_FEATURE_ID_JALT },
        { featureCode("jp78"), STR_FONT_FEATURE_ID_JP78 },
        { featureCode("jp83"), STR_FONT_FEATURE_ID_JP83 },
        { featureCode("jp90"), STR_FONT_FEATURE_ID_JP90 },
        { featureCode("jp04"), STR_FONT_FEATURE_ID_JP04 },
        { featureCode("kern"), STR_FONT_FEATURE_ID_KERN },
        { featureCode("lfbd"), STR_FONT_FEATURE_ID_LFBD },
        { featureCode("liga"), STR_FONT_FEATURE_ID_LIGA },
        { featureCode("lnum"), STR_FONT_FEATURE_ID_LNUM },
        { featureCode("mgrk"), STR_FONT_FEATURE_ID_MGRK },
        { featureCode("nalt"), STR_FONT_FEATURE_ID_NALT },
        { featureCode("nlck"), STR_FONT_FEATURE_ID_NLCK },
        { featureCode("numr"), STR_FONT_FEATURE_ID_NUMR },
        { featureCode("onum"), STR_FONT_FEATURE_ID_ONUM },
        { featureCode("opbd"), STR_FONT_FEATURE_ID_OPBD },
        { featureCode("ordn"), STR_FONT_FEATURE_ID_ORDN },
        { featureCode("ornm"), STR_FONT_FEATURE_ID_ORNM },
        { featureCode("palt"), STR_FONT_FEATURE_ID_PALT },
        { featureCode("pcap"), STR_FONT_FEATURE_ID_PCAP },
        { featureCode("pkna"), STR_FONT_FEATURE_ID_PKNA },
        { featureCode("pnum"), STR_FONT_FEATURE_ID_PNUM },
        { featureCode("pwid"), STR_FONT_FEATURE_ID_PWID },
        { featureCode("qwid"), STR_FONT_FEATURE_ID_QWID },
        { featureCode("rtbd"), STR_FONT_FEATURE_ID_RTBD },
        { featureCode("ruby"), STR_FONT_FEATURE_ID_RUBY },
        { featureCode("salt"), STR_FONT_FEATURE_ID_SALT },
        { featureCode("sinf"), STR_FONT_FEATURE_ID_SINF },
        { featureCode("smcp"), STR_FONT_FEATURE_ID_SMCP },
        { featureCode("smpl"), STR_FONT_FEATURE_ID_SMPL },
        { featureCode("subs"), STR_FONT_FEATURE_ID_SUBS },
        { featureCode("sups"), STR_FONT_FEATURE_ID_SUPS },
        { featureCode("swsh"), STR_FONT_FEATURE_ID_SWSH },
        { featureCode("titl"), STR_FONT_FEATURE_ID_TITL },
        { featureCode("tnam"), STR_FONT_FEATURE_ID_TNAM },
        { featureCode("tnum"), STR_FONT_FEATURE_ID_TNUM },
        { featureCode("trad"), STR_FONT_FEATURE_ID_TRAD },
        { featureCode("twid"), STR_FONT_FEATURE_ID_TWID },
        { featureCode("unic"), STR_FONT_FEATURE_ID_UNIC },
        { featureCode("valt"), STR_FONT_FEATURE_ID_VALT },
        { featureCode("vhal"), STR_FONT_FEATURE_ID_VHAL },
        { featureCode("vkna"), STR_FONT_FEATURE_ID_VKNA },
        { featureCode("vkrn"), STR_FONT_FEATURE_ID_VKRN },
        { featureCode("vpal"), STR_FONT_FEATURE_ID_VPAL },
        { featureCode("vrt2"), STR_FONT_FEATURE_ID_VRT2 },
        { featureCode("vrtr"), STR_FONT_FEATURE_ID_VRTR },
        { featureCode("zero"), STR_FONT_FEATURE_ID_ZERO },
    });

    for (size_t i = 0; i < m_aFeatureDefinition.size(); ++i)
    {
        m_aCodeToIndex.emplace(m_aFeatureDefinition[i].getCode(), i);
    }

    m_aRequiredFeatures.assign({
        featureCode("abvf"), featureCode("abvm"), featureCode("abvs"), featureCode("akhn"),
        featureCode("blwf"), featureCode("blwm"), featureCode("blws"), featureCode("ccmp"),
        featureCode("cfar"), featureCode("cjct"), featureCode("curs"), featureCode("dist"),
        featureCode("dtls"), featureCode("fin2"), featureCode("fin3"), featureCode("fina"),
        featureCode("flac"), featureCode("half"), featureCode("haln"), featureCode("init"),
        featureCode("isol"), featureCode("ljmo"), featureCode("locl"), featureCode("ltra"),
        featureCode("ltrm"), featureCode("mark"), featureCode("med2"), featureCode("medi"),
        featureCode("mkmk"), featureCode("mset"), featureCode("nukt"), featureCode("pref"),
        featureCode("pres"), featureCode("pstf"), featureCode("psts"), featureCode("rand"),
        featureCode("rclt"), featureCode("rkrf"), featureCode("rlig"), featureCode("rphf"),
        featureCode("rtla"), featureCode("rtlm"), featureCode("rvrn"), featureCode("size"),
        featureCode("ssty"), featureCode("stch"), featureCode("tjmo"), featureCode("vatu"),
        featureCode("vert"), featureCode("vjmo"),
    });
}

namespace
{
bool isCharacterVariantCode(sal_uInt32 nFeatureCode)
{
    return sal_Char((sal_uInt32(nFeatureCode) >> 24) & 0xFF) == 'c'
           && sal_Char((sal_uInt32(nFeatureCode) >> 16) & 0xFF) == 'v';
}

bool isStylisticSetCode(sal_uInt32 nFeatureCode)
{
    return sal_Char((sal_uInt32(nFeatureCode) >> 24) & 0xFF) == 's'
           && sal_Char((sal_uInt32(nFeatureCode) >> 16) & 0xFF) == 's';
}

OUString getNumericLowerPart(sal_uInt32 nFeatureCode)
{
    sal_Char cChar1((sal_uInt32(nFeatureCode) >> 8) & 0xFF);
    sal_Char cChar2((sal_uInt32(nFeatureCode) >> 0) & 0xFF);

    if (rtl::isAsciiDigit(static_cast<unsigned char>(cChar1))
        && rtl::isAsciiDigit(static_cast<unsigned char>(cChar2)))
    {
        return OUString(cChar1) + OUString(cChar2);
    }
    return OUString();
}

} // end anonymous namespace

bool OpenTypeFeatureDefinitonListPrivate::isSpecialFeatureCode(sal_uInt32 nFeatureCode)
{
    return isCharacterVariantCode(nFeatureCode) || isStylisticSetCode(nFeatureCode);
}

FeatureDefinition
OpenTypeFeatureDefinitonListPrivate::handleSpecialFeatureCode(sal_uInt32 nFeatureCode)
{
    FeatureDefinition aFeatureDefinition;
    OUString sNumericPart = getNumericLowerPart(nFeatureCode);
    if (!sNumericPart.isEmpty())
    {
        if (isCharacterVariantCode(nFeatureCode))
            aFeatureDefinition = { nFeatureCode, STR_FONT_FEATURE_ID_CVXX, sNumericPart };
        else if (isStylisticSetCode(nFeatureCode))
            aFeatureDefinition = { nFeatureCode, STR_FONT_FEATURE_ID_SSXX, sNumericPart };
    }
    return aFeatureDefinition;
}

FeatureDefinition OpenTypeFeatureDefinitonListPrivate::getDefinition(sal_uInt32 nFeatureCode)
{
    if (isSpecialFeatureCode(nFeatureCode))
    {
        return handleSpecialFeatureCode(nFeatureCode);
    }

    if (m_aCodeToIndex.find(nFeatureCode) != m_aCodeToIndex.end())
    {
        size_t nIndex = m_aCodeToIndex.at(nFeatureCode);
        return m_aFeatureDefinition[nIndex];
    }
    return FeatureDefinition();
}

bool OpenTypeFeatureDefinitonListPrivate::isRequired(sal_uInt32 nFeatureCode)
{
    return std::find(m_aRequiredFeatures.begin(), m_aRequiredFeatures.end(), nFeatureCode)
           != m_aRequiredFeatures.end();
}

} // end font namespace
} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
