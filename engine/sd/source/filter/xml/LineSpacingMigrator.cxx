/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <LineSpacingMigrator.hxx>

#include <sal/log.hxx>

#include <editeng/editdata.hxx>
#include <editeng/editobj.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/outlobj.hxx>

#include <svx/sdtfchim.hxx>
#include <svx/svddef.hxx>
#include <svx/svdotext.hxx>

#include <vcl/font.hxx>
#include <vcl/metric.hxx>

#include <Outliner.hxx>
#include <drawdoc.hxx>

namespace sd
{
namespace
{
constexpr double constFontIndependentBase = 1.2;

// Arbitrary font height for metric queries. Exact value doesn't matter.
constexpr sal_Int32 constReferenceHeight = 2048;
}

void LineSpacingMigrator::migrate(SdDrawDocument* pDocument)
{
    auto pMigrator = std::make_shared<LineSpacingMigrator>();
    pMigrator->mpOutliner = pDocument->GetInternalOutliner();

    ModelTraverser aTraverser(pDocument, { .mbMasterPages = true });
    aTraverser.addNodeHandler(pMigrator);
    aTraverser.traverse();
}

void LineSpacingMigrator::handleSdrObject(SdrObject* pObject)
{
    SdrTextObj* pTextObject = DynCastSdrTextObj(pObject);
    if (pTextObject)
        migrateShape(pTextObject);
}

double LineSpacingMigrator::getConversionFactor(const OUString& rFontName)
{
    auto it = maFactorCache.find(rFontName);
    if (it != maFactorCache.end())
        return it->second;

    vcl::Font aFont(rFontName, Size(0, constReferenceHeight));
    mpVirtualDevice->SetFont(aFont);

    FontMetric aMetric = mpVirtualDevice->GetFontMetric();

    double fRatio = 0.0;
    if (rFontName == u"Carlito")
    {
        // For backwards compatibility with different Carlito versions it's best to use
        // the hhea metrics. That way the opening the document will not use different line
        // spacing for existing shapes as long as it is opened with same font version.
        fRatio = mpVirtualDevice->GetHheaLineHeightRatio();
    }
    else
    {
        double fLineHeight = aMetric.GetAscent() + aMetric.GetDescent();
        if (fLineHeight > 0 && constReferenceHeight > 0)
            fRatio = fLineHeight / constReferenceHeight;
    }

    double fFactor = (fRatio > 0.0) ? fRatio / constFontIndependentBase : 0.0;
    maFactorCache[rFontName] = fFactor;

    SAL_INFO("sd.filter", "Computed line spacing conversion for '"
                              << rFontName << "': metric ratio=" << fRatio
                              << ", conversion factor=" << fFactor);
    return fFactor;
}

void LineSpacingMigrator::migrateShape(SdrTextObj* pTextObject)
{
    // Already using font independent line spacing
    if (pTextObject->GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT).GetValue())
        return;

    OutlinerParaObject* pOutlinerParaObject = pTextObject->GetOutlinerParaObject();
    if (!pOutlinerParaObject)
        return;

    const OUString& rShapeFont = pTextObject->GetMergedItem(EE_CHAR_FONTINFO).GetFamilyName();
    double fShapeFactor = getConversionFactor(rShapeFont);

    mpOutliner->Clear();
    mpOutliner->SetText(*pOutlinerParaObject);

    bool bModified = false;
    const sal_Int32 nParagraphCount = mpOutliner->GetParagraphCount();
    for (sal_Int32 nParagraph = 0; nParagraph < nParagraphCount; ++nParagraph)
    {
        if (migrateParagraph(pOutlinerParaObject, nParagraph, fShapeFactor))
            bModified = true;
    }

    if (bModified)
    {
        pTextObject->SetMergedItem(SdrTextFixedCellHeightItem(true));
        pTextObject->SetOutlinerParaObject(mpOutliner->CreateParaObject(0, nParagraphCount));
    }
    mpOutliner->Clear();
}

bool LineSpacingMigrator::migrateParagraph(OutlinerParaObject* pOutlinerParaObject,
                                           sal_Int32 nParagraph, double fShapeFactor)
{
    SfxItemSet aAttrs(mpOutliner->GetParaAttribs(nParagraph));

    const SvxLineSpacingItem& rLineSpacing = aAttrs.Get(EE_PARA_SBL);
    if (rLineSpacing.GetLineSpaceRule() != SvxLineSpaceRule::Auto)
        return false;

    sal_uInt16 nOldProportion = 100;
    if (rLineSpacing.GetInterLineSpaceRule() == SvxInterLineSpaceRule::Off)
        nOldProportion = 100;
    else if (rLineSpacing.GetInterLineSpaceRule() == SvxInterLineSpaceRule::Prop)
        nOldProportion = rLineSpacing.GetPropLineSpace();
    else
        return false;

    double fFactor = fShapeFactor;
    const EditTextObject& rEditText = pOutlinerParaObject->GetTextObject();
    std::vector<EECharAttrib> aCharAttribs;
    rEditText.GetCharAttribs(nParagraph, aCharAttribs);
    for (const auto& rAttrib : aCharAttribs)
    {
        if (rAttrib.pAttr->Which() == EE_CHAR_FONTINFO
            || rAttrib.pAttr->Which() == EE_CHAR_FONTINFO_CJK
            || rAttrib.pAttr->Which() == EE_CHAR_FONTINFO_CTL)
        {
            const auto* pFont = static_cast<const SvxFontItem*>(rAttrib.pAttr);
            double fCharFactor = getConversionFactor(pFont->GetFamilyName());
            if (fCharFactor > 0.0)
                fFactor = fCharFactor;
            break;
        }
    }

    if (fFactor <= 0.0)
        return false;

    sal_uInt16 nNewProportion = sal_uInt16(std::lround(nOldProportion * fFactor));
    if (nNewProportion == 0)
        nNewProportion = 1;

    SAL_INFO("sd.filter", "Migrating paragraph " << nParagraph << " line spacing from "
                                                 << nOldProportion << "% to font-independent "
                                                 << nNewProportion << "%");

    SvxLineSpacingItem aNewLineSpacing(rLineSpacing);
    aNewLineSpacing.SetPropLineSpace(nNewProportion);
    aAttrs.Put(aNewLineSpacing);
    mpOutliner->SetParaAttribs(nParagraph, aAttrs);
    return true;
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
