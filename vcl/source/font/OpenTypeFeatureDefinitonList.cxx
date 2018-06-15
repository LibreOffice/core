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
#include <svdata.hxx>

namespace vcl
{
namespace font
{
OpenTypeFeatureDefinitonListPrivate::OpenTypeFeatureDefinitonListPrivate() { init(); }

void OpenTypeFeatureDefinitonListPrivate::init()
{
    m_aFeatureDefinition.assign({
        { featureCode("aalt"), STR_FONT_FEATURE_ID_AALT },
        { featureCode("abvf"), STR_FONT_FEATURE_ID_ABVF },
        { featureCode("abvm"), STR_FONT_FEATURE_ID_ABVM },
        { featureCode("abvs"), STR_FONT_FEATURE_ID_ABVS },
        { featureCode("afrc"), STR_FONT_FEATURE_ID_AFRC },
        { featureCode("akhn"), STR_FONT_FEATURE_ID_AKHN },
        { featureCode("alig"), STR_FONT_FEATURE_ID_ALIG },
        { featureCode("blwf"), STR_FONT_FEATURE_ID_BLWF },
        { featureCode("blwm"), STR_FONT_FEATURE_ID_BLWM },
        { featureCode("blws"), STR_FONT_FEATURE_ID_BLWS },
        { featureCode("c2pc"), STR_FONT_FEATURE_ID_C2PC },
        { featureCode("c2sc"), STR_FONT_FEATURE_ID_C2SC },
        { featureCode("calt"), STR_FONT_FEATURE_ID_CALT },
        { featureCode("case"), STR_FONT_FEATURE_ID_CASE },
        { featureCode("ccmp"), STR_FONT_FEATURE_ID_CCMP },
        { featureCode("cfar"), STR_FONT_FEATURE_ID_CFAR },
        { featureCode("cjct"), STR_FONT_FEATURE_ID_CJCT },
        { featureCode("clig"), STR_FONT_FEATURE_ID_CLIG },
        { featureCode("cpct"), STR_FONT_FEATURE_ID_CPCT },
        { featureCode("cpsp"), STR_FONT_FEATURE_ID_CPSP },
        { featureCode("cswh"), STR_FONT_FEATURE_ID_CSWH },
        { featureCode("curs"), STR_FONT_FEATURE_ID_CURS },
        { featureCode("cv01"), STR_FONT_FEATURE_ID_CV01 },
        { featureCode("cv02"), STR_FONT_FEATURE_ID_CV02 },
        { featureCode("cv03"), STR_FONT_FEATURE_ID_CV03 },
        { featureCode("cv04"), STR_FONT_FEATURE_ID_CV04 },
        { featureCode("cv05"), STR_FONT_FEATURE_ID_CV05 },
        { featureCode("cv06"), STR_FONT_FEATURE_ID_CV06 },
        { featureCode("cv07"), STR_FONT_FEATURE_ID_CV07 },
        { featureCode("cv08"), STR_FONT_FEATURE_ID_CV08 },
        { featureCode("cv09"), STR_FONT_FEATURE_ID_CV09 },
        { featureCode("dcap"), STR_FONT_FEATURE_ID_DCAP },
        { featureCode("dist"), STR_FONT_FEATURE_ID_DIST },
        { featureCode("dlig"), STR_FONT_FEATURE_ID_DLIG },
        { featureCode("dnom"), STR_FONT_FEATURE_ID_DNOM },
        { featureCode("dpng"), STR_FONT_FEATURE_ID_DPNG },
        { featureCode("dtls"), STR_FONT_FEATURE_ID_DTLS },
        { featureCode("expt"), STR_FONT_FEATURE_ID_EXPT },
        { featureCode("falt"), STR_FONT_FEATURE_ID_FALT },
        { featureCode("fin2"), STR_FONT_FEATURE_ID_FIN2 },
        { featureCode("fin3"), STR_FONT_FEATURE_ID_FIN3 },
        { featureCode("fina"), STR_FONT_FEATURE_ID_FINA },
        { featureCode("flac"), STR_FONT_FEATURE_ID_FLAC },
        { featureCode("frac"), STR_FONT_FEATURE_ID_FRAC,
          std::vector<FeatureParameter>{ { 1, STR_FONT_FEATURE_ID_FRAC_PARAM_1 },
                                         { 2, STR_FONT_FEATURE_ID_FRAC_PARAM_2 } } },
        { featureCode("fwid"), STR_FONT_FEATURE_ID_FWID },
        { featureCode("half"), STR_FONT_FEATURE_ID_HALF },
        { featureCode("haln"), STR_FONT_FEATURE_ID_HALN },
        { featureCode("halt"), STR_FONT_FEATURE_ID_HALT },
        { featureCode("hist"), STR_FONT_FEATURE_ID_HIST },
        { featureCode("hkna"), STR_FONT_FEATURE_ID_HKNA },
        { featureCode("hlig"), STR_FONT_FEATURE_ID_HLIG },
        { featureCode("hngl"), STR_FONT_FEATURE_ID_HNGL },
        { featureCode("hojo"), STR_FONT_FEATURE_ID_HOJO },
        { featureCode("hwid"), STR_FONT_FEATURE_ID_HWID },
        { featureCode("init"), STR_FONT_FEATURE_ID_INIT },
        { featureCode("isol"), STR_FONT_FEATURE_ID_ISOL },
        { featureCode("ital"), STR_FONT_FEATURE_ID_ITAL },
        { featureCode("jalt"), STR_FONT_FEATURE_ID_JALT },
        { featureCode("jp78"), STR_FONT_FEATURE_ID_JP78 },
        { featureCode("jp83"), STR_FONT_FEATURE_ID_JP83 },
        { featureCode("jp90"), STR_FONT_FEATURE_ID_JP90 },
        { featureCode("jp04"), STR_FONT_FEATURE_ID_JP04 },
        { featureCode("kern"), STR_FONT_FEATURE_ID_KERN },
        { featureCode("lfbd"), STR_FONT_FEATURE_ID_LFBD },
        { featureCode("liga"), STR_FONT_FEATURE_ID_LIGA },
        { featureCode("ljmo"), STR_FONT_FEATURE_ID_LJMO },
        { featureCode("lnum"), STR_FONT_FEATURE_ID_LNUM },
        { featureCode("locl"), STR_FONT_FEATURE_ID_LOCL },
        { featureCode("ltra"), STR_FONT_FEATURE_ID_LTRA },
        { featureCode("ltrm"), STR_FONT_FEATURE_ID_LTRM },
        { featureCode("mark"), STR_FONT_FEATURE_ID_MARK },
        { featureCode("med2"), STR_FONT_FEATURE_ID_MED2 },
        { featureCode("medi"), STR_FONT_FEATURE_ID_MEDI },
        { featureCode("mgrk"), STR_FONT_FEATURE_ID_MGRK },
        { featureCode("mkmk"), STR_FONT_FEATURE_ID_MKMK },
        { featureCode("mset"), STR_FONT_FEATURE_ID_MSET },
        { featureCode("nalt"), STR_FONT_FEATURE_ID_NALT },
        { featureCode("nlck"), STR_FONT_FEATURE_ID_NLCK },
        { featureCode("nukt"), STR_FONT_FEATURE_ID_NUKT },
        { featureCode("numr"), STR_FONT_FEATURE_ID_NUMR },
        { featureCode("onum"), STR_FONT_FEATURE_ID_ONUM },
        { featureCode("opbd"), STR_FONT_FEATURE_ID_OPBD },
        { featureCode("ordn"), STR_FONT_FEATURE_ID_ORDN },
        { featureCode("ornm"), STR_FONT_FEATURE_ID_ORNM },
        { featureCode("palt"), STR_FONT_FEATURE_ID_PALT },
        { featureCode("pcap"), STR_FONT_FEATURE_ID_PCAP },
        { featureCode("pkna"), STR_FONT_FEATURE_ID_PKNA },
        { featureCode("pnum"), STR_FONT_FEATURE_ID_PNUM },
        { featureCode("pref"), STR_FONT_FEATURE_ID_PREF },
        { featureCode("pres"), STR_FONT_FEATURE_ID_PRES },
        { featureCode("pstf"), STR_FONT_FEATURE_ID_PSTF },
        { featureCode("psts"), STR_FONT_FEATURE_ID_PSTS },
        { featureCode("pwid"), STR_FONT_FEATURE_ID_PWID },
        { featureCode("qwid"), STR_FONT_FEATURE_ID_QWID },
        { featureCode("rand"), STR_FONT_FEATURE_ID_RAND },
        { featureCode("rclt"), STR_FONT_FEATURE_ID_RCLT },
        { featureCode("rkrf"), STR_FONT_FEATURE_ID_RKRF },
        { featureCode("rlig"), STR_FONT_FEATURE_ID_RLIG },
        { featureCode("rphf"), STR_FONT_FEATURE_ID_RPHF },
        { featureCode("rtbd"), STR_FONT_FEATURE_ID_RTBD },
        { featureCode("rtla"), STR_FONT_FEATURE_ID_RTLA },
        { featureCode("rtlm"), STR_FONT_FEATURE_ID_RTLM },
        { featureCode("ruby"), STR_FONT_FEATURE_ID_RUBY },
        { featureCode("rvrn"), STR_FONT_FEATURE_ID_RVRN },
        { featureCode("salt"), STR_FONT_FEATURE_ID_SALT },
        { featureCode("sinf"), STR_FONT_FEATURE_ID_SINF },
        { featureCode("size"), STR_FONT_FEATURE_ID_SIZE },
        { featureCode("smcp"), STR_FONT_FEATURE_ID_SMCP },
        { featureCode("smpl"), STR_FONT_FEATURE_ID_SMPL },
        { featureCode("ss01"), STR_FONT_FEATURE_ID_SS01 },
        { featureCode("ss02"), STR_FONT_FEATURE_ID_SS02 },
        { featureCode("ss03"), STR_FONT_FEATURE_ID_SS03 },
        { featureCode("ss04"), STR_FONT_FEATURE_ID_SS04 },
        { featureCode("ss05"), STR_FONT_FEATURE_ID_SS05 },
        { featureCode("ss06"), STR_FONT_FEATURE_ID_SS06 },
        { featureCode("ss07"), STR_FONT_FEATURE_ID_SS07 },
        { featureCode("ss08"), STR_FONT_FEATURE_ID_SS08 },
        { featureCode("ss09"), STR_FONT_FEATURE_ID_SS09 },
        { featureCode("ss10"), STR_FONT_FEATURE_ID_SS10 },
        { featureCode("ss11"), STR_FONT_FEATURE_ID_SS11 },
        { featureCode("ss12"), STR_FONT_FEATURE_ID_SS12 },
        { featureCode("ss13"), STR_FONT_FEATURE_ID_SS13 },
        { featureCode("ss14"), STR_FONT_FEATURE_ID_SS14 },
        { featureCode("ss15"), STR_FONT_FEATURE_ID_SS15 },
        { featureCode("ss16"), STR_FONT_FEATURE_ID_SS16 },
        { featureCode("ss17"), STR_FONT_FEATURE_ID_SS17 },
        { featureCode("ss18"), STR_FONT_FEATURE_ID_SS18 },
        { featureCode("ss19"), STR_FONT_FEATURE_ID_SS19 },
        { featureCode("ss20"), STR_FONT_FEATURE_ID_SS20 },
        { featureCode("ssty"), STR_FONT_FEATURE_ID_SSTY },
        { featureCode("stch"), STR_FONT_FEATURE_ID_STCH },
        { featureCode("subs"), STR_FONT_FEATURE_ID_SUBS },
        { featureCode("sups"), STR_FONT_FEATURE_ID_SUPS },
        { featureCode("swsh"), STR_FONT_FEATURE_ID_SWSH },
        { featureCode("titl"), STR_FONT_FEATURE_ID_TITL },
        { featureCode("tjmo"), STR_FONT_FEATURE_ID_TJMO },
        { featureCode("tnam"), STR_FONT_FEATURE_ID_TNAM },
        { featureCode("tnum"), STR_FONT_FEATURE_ID_TNUM },
        { featureCode("trad"), STR_FONT_FEATURE_ID_TRAD },
        { featureCode("twid"), STR_FONT_FEATURE_ID_TWID },
        { featureCode("unic"), STR_FONT_FEATURE_ID_UNIC },
        { featureCode("valt"), STR_FONT_FEATURE_ID_VALT },
        { featureCode("vatu"), STR_FONT_FEATURE_ID_VATU },
        { featureCode("vert"), STR_FONT_FEATURE_ID_VERT },
        { featureCode("vhal"), STR_FONT_FEATURE_ID_VHAL },
        { featureCode("vjmo"), STR_FONT_FEATURE_ID_VJMO },
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
}

FeatureDefinition OpenTypeFeatureDefinitonListPrivate::getDefinition(sal_uInt32 nFeatureCode)
{
    if (m_aCodeToIndex.find(nFeatureCode) != m_aCodeToIndex.end())
    {
        size_t nIndex = m_aCodeToIndex.at(nFeatureCode);
        return m_aFeatureDefinition[nIndex];
    }
    return FeatureDefinition();
}

} // end font namespace
} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
