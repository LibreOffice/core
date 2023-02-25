/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/export/ThemeExport.hxx>

#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <oox/export/utils.hxx>
#include <docmodel/theme/Theme.hxx>
#include <sax/fshelper.hxx>
#include <sax/fastattribs.hxx>
#include <unordered_map>

namespace oox
{
ThemeExport::ThemeExport(oox::core::XmlFilterBase* pFilterBase)
    : mpFilterBase(pFilterBase)

{
}

void ThemeExport::write(OUString const& rPath, model::Theme const& rTheme)
{
    sax_fastparser::FSHelperPtr pFS = mpFilterBase->openFragmentStreamWithSerializer(
        rPath, "application/vnd.openxmlformats-officedocument.theme+xml");

    OUString aThemeName = rTheme.GetName();

    pFS->startElementNS(XML_a, XML_theme, FSNS(XML_xmlns, XML_a),
                        mpFilterBase->getNamespaceURL(OOX_NS(dml)), XML_name, aThemeName);

    pFS->startElementNS(XML_a, XML_themeElements);

    const model::ColorSet* pColorSet = rTheme.GetColorSet();

    pFS->startElementNS(XML_a, XML_clrScheme, XML_name, pColorSet->getName());
    writeColorSet(pFS, rTheme);
    pFS->endElementNS(XML_a, XML_clrScheme);

    model::FontScheme const& rFontScheme = rTheme.getFontScheme();
    pFS->startElementNS(XML_a, XML_fontScheme, XML_name, rFontScheme.getName());
    writeFontScheme(pFS, rFontScheme);
    pFS->endElementNS(XML_a, XML_fontScheme);

    pFS->startElementNS(XML_a, XML_fmtScheme);
    writeFormatScheme(pFS);
    pFS->endElementNS(XML_a, XML_fmtScheme);

    pFS->endElementNS(XML_a, XML_themeElements);
    pFS->endElementNS(XML_a, XML_theme);

    pFS->endDocument();
}

namespace
{
void fillAttrList(rtl::Reference<sax_fastparser::FastAttributeList> const& pAttrList,
                  model::ThemeFont const& rThemeFont)
{
    pAttrList->add(XML_typeface, rThemeFont.maTypeface);
    pAttrList->add(XML_panose, rThemeFont.maPanose);
    pAttrList->add(XML_pitchFamily, OString::number(rThemeFont.getPitchFamily()));
    pAttrList->add(XML_charset, OString::number(rThemeFont.maCharset));
}

} // end anonymous ns

bool ThemeExport::writeFontScheme(sax_fastparser::FSHelperPtr pFS,
                                  model::FontScheme const& rFontScheme)
{
    pFS->startElementNS(XML_a, XML_majorFont);

    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMajorLatin());
        pFS->singleElementNS(XML_a, XML_latin, aAttrList);
    }
    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMajorAsian());
        pFS->singleElementNS(XML_a, XML_ea, aAttrList);
    }
    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMajorComplex());
        pFS->singleElementNS(XML_a, XML_cs, aAttrList);
    }

    pFS->endElementNS(XML_a, XML_majorFont);

    pFS->startElementNS(XML_a, XML_minorFont);

    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMinorLatin());
        pFS->singleElementNS(XML_a, XML_latin, aAttrList);
    }
    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMinorAsian());
        pFS->singleElementNS(XML_a, XML_ea, aAttrList);
    }
    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMinorComplex());
        pFS->singleElementNS(XML_a, XML_cs, aAttrList);
    }

    pFS->endElementNS(XML_a, XML_minorFont);

    return true;
}

bool ThemeExport::writeFormatScheme(sax_fastparser::FSHelperPtr pFS)
{
    // Format Scheme: 3 or more per list but only 3 will be used currently
    pFS->startElementNS(XML_a, XML_fillStyleLst);
    {
        pFS->startElementNS(XML_a, XML_solidFill);
        pFS->singleElementNS(XML_a, XML_schemeClr, XML_val, "phClr");
        pFS->endElementNS(XML_a, XML_solidFill);

        pFS->startElementNS(XML_a, XML_solidFill);
        pFS->singleElementNS(XML_a, XML_schemeClr, XML_val, "phClr");
        pFS->endElementNS(XML_a, XML_solidFill);

        pFS->startElementNS(XML_a, XML_solidFill);
        pFS->singleElementNS(XML_a, XML_schemeClr, XML_val, "phClr");
        pFS->endElementNS(XML_a, XML_solidFill);
    }
    pFS->endElementNS(XML_a, XML_fillStyleLst);

    pFS->startElementNS(XML_a, XML_lnStyleLst);
    {
        pFS->startElementNS(XML_a, XML_ln, XML_w, "6350", XML_cap, "flat", XML_cmpd, "sng",
                            XML_algn, "ctr");
        {
            pFS->startElementNS(XML_a, XML_solidFill);
            pFS->startElementNS(XML_a, XML_schemeClr, XML_val, "phClr");
            pFS->singleElementNS(XML_a, XML_shade, XML_val, "95000");
            pFS->endElementNS(XML_a, XML_schemeClr);
            pFS->endElementNS(XML_a, XML_solidFill);

            pFS->singleElementNS(XML_a, XML_prstDash, XML_val, "solid");

            pFS->singleElementNS(XML_a, XML_miter);
        }
        pFS->endElementNS(XML_a, XML_ln);
    }
    {
        pFS->startElementNS(XML_a, XML_ln, XML_w, "6350", XML_cap, "flat", XML_cmpd, "sng",
                            XML_algn, "ctr");
        {
            pFS->startElementNS(XML_a, XML_solidFill);
            pFS->startElementNS(XML_a, XML_schemeClr, XML_val, "phClr");
            pFS->singleElementNS(XML_a, XML_shade, XML_val, "95000");
            pFS->endElementNS(XML_a, XML_schemeClr);
            pFS->endElementNS(XML_a, XML_solidFill);

            pFS->singleElementNS(XML_a, XML_prstDash, XML_val, "solid");

            pFS->singleElementNS(XML_a, XML_miter);
        }
        pFS->endElementNS(XML_a, XML_ln);
    }
    {
        pFS->startElementNS(XML_a, XML_ln, XML_w, "6350", XML_cap, "flat", XML_cmpd, "sng",
                            XML_algn, "ctr");
        {
            pFS->startElementNS(XML_a, XML_solidFill);
            pFS->startElementNS(XML_a, XML_schemeClr, XML_val, "phClr");
            pFS->singleElementNS(XML_a, XML_shade, XML_val, "95000");
            pFS->endElementNS(XML_a, XML_schemeClr);
            pFS->endElementNS(XML_a, XML_solidFill);

            pFS->singleElementNS(XML_a, XML_prstDash, XML_val, "solid");

            pFS->singleElementNS(XML_a, XML_miter);
        }
        pFS->endElementNS(XML_a, XML_ln);
    }
    pFS->endElementNS(XML_a, XML_lnStyleLst);

    pFS->startElementNS(XML_a, XML_effectStyleLst);
    {
        pFS->startElementNS(XML_a, XML_effectStyle);
        pFS->singleElementNS(XML_a, XML_effectLst);
        pFS->endElementNS(XML_a, XML_effectStyle);

        pFS->startElementNS(XML_a, XML_effectStyle);
        pFS->singleElementNS(XML_a, XML_effectLst);
        pFS->endElementNS(XML_a, XML_effectStyle);

        pFS->startElementNS(XML_a, XML_effectStyle);
        pFS->singleElementNS(XML_a, XML_effectLst);
        pFS->endElementNS(XML_a, XML_effectStyle);
    }
    pFS->endElementNS(XML_a, XML_effectStyleLst);

    pFS->startElementNS(XML_a, XML_bgFillStyleLst);
    {
        pFS->startElementNS(XML_a, XML_solidFill);
        pFS->singleElementNS(XML_a, XML_schemeClr, XML_val, "phClr");
        pFS->endElementNS(XML_a, XML_solidFill);

        pFS->startElementNS(XML_a, XML_solidFill);
        pFS->singleElementNS(XML_a, XML_schemeClr, XML_val, "phClr");
        pFS->endElementNS(XML_a, XML_solidFill);

        pFS->startElementNS(XML_a, XML_solidFill);
        pFS->singleElementNS(XML_a, XML_schemeClr, XML_val, "phClr");
        pFS->endElementNS(XML_a, XML_solidFill);
    }
    pFS->endElementNS(XML_a, XML_bgFillStyleLst);

    return true;
}

bool ThemeExport::writeColorSet(sax_fastparser::FSHelperPtr pFS, model::Theme const& rTheme)
{
    static std::unordered_map<sal_Int32, model::ThemeColorType> constTokenMap
        = { { XML_dk1, model::ThemeColorType::Dark1 },
            { XML_lt1, model::ThemeColorType::Light1 },
            { XML_dk2, model::ThemeColorType::Dark2 },
            { XML_lt2, model::ThemeColorType::Light2 },
            { XML_accent1, model::ThemeColorType::Accent1 },
            { XML_accent2, model::ThemeColorType::Accent2 },
            { XML_accent3, model::ThemeColorType::Accent3 },
            { XML_accent4, model::ThemeColorType::Accent4 },
            { XML_accent5, model::ThemeColorType::Accent5 },
            { XML_accent6, model::ThemeColorType::Accent6 },
            { XML_hlink, model::ThemeColorType::Hyperlink },
            { XML_folHlink, model::ThemeColorType::FollowedHyperlink } };

    static std::array<sal_Int32, 12> constTokenArray
        = { XML_dk1,     XML_lt1,     XML_dk2,     XML_lt2,     XML_accent1, XML_accent2,
            XML_accent3, XML_accent4, XML_accent5, XML_accent6, XML_hlink,   XML_folHlink };

    const model::ColorSet* pColorSet = rTheme.GetColorSet();
    if (!pColorSet)
        return false;

    for (auto nToken : constTokenArray)
    {
        model::ThemeColorType eColorType = constTokenMap[nToken];
        Color aColor = pColorSet->getColor(eColorType);
        pFS->startElementNS(XML_a, nToken);
        pFS->singleElementNS(XML_a, XML_srgbClr, XML_val, I32SHEX(sal_Int32(aColor)));
        pFS->endElementNS(XML_a, nToken);
    }

    return true;
}

} // end namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
