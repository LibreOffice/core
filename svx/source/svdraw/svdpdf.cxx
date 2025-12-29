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

#include <svdpdf.hxx>

#include <tools/stream.hxx>
#include <tools/UnitConversion.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/embeddedfontsmanager.hxx>
#include <vcl/graph.hxx>
#include <vcl/pdf/PDFBitmapType.hxx>
#include <vcl/pdf/PDFFillMode.hxx>
#include <vcl/pdf/PDFPageObjectType.hxx>
#include <vcl/pdf/PDFSegmentType.hxx>
#include <vcl/pdf/PDFTextRenderMode.hxx>
#include <vcl/vectorgraphicdata.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/shdditem.hxx>
#include <svx/xflbmsxy.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xflclit.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <vcl/metric.hxx>
#include <editeng/charscaleitem.hxx>
#include <svx/sdtditm.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdtfsitm.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdetc.hxx>
#include <svl/itemset.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <tools/helpers.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/xlinjoit.hxx>
#include <svx/xlndsit.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xlineit0.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/svditer.hxx>
#include <svx/svdogrp.hxx>
#include <vcl/dibtools.hxx>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <osl/file.hxx>
#include <unicode/normalizer2.h>

using namespace com::sun::star;

ImpSdrPdfImport::ImpSdrPdfImport(SdrModel& rModel, SdrLayerID nLay, const tools::Rectangle& rRect,
                                 Graphic const& rGraphic)
    : maScaleRect(rRect)
    , mnMapScalingOfs(0)
    , mpModel(&rModel)
    , mnLayer(nLay)
    , mnLineWidth(0)
    , maDash(css::drawing::DashStyle_RECT, 0, 0, 0, 0, 0)
    , mbMov(false)
    , mbSize(false)
    , maOfs(0, 0)
    , mfScaleX(1.0)
    , mfScaleY(1.0)
    , maScaleX(1.0)
    , maScaleY(1.0)
    , mbFntDirty(true)
    , mbLastObjWasPolyWithoutLine(false)
    , mbNoLine(false)
    , mbNoFill(false)
    , mnPageCount(0)
    , mdPageHeightPts(0)
    , mpPDFium(vcl::pdf::PDFiumLibrary::get())
{
    mpLineAttr = std::make_unique<SfxItemSet>(
        SfxItemSet::makeFixedSfxItemSet<XATTR_LINE_FIRST, XATTR_LINE_LAST>(rModel.GetItemPool()));
    mpFillAttr = std::make_unique<SfxItemSet>(
        SfxItemSet::makeFixedSfxItemSet<XATTR_FILL_FIRST, XATTR_FILL_LAST>(rModel.GetItemPool()));
    mpTextAttr = std::make_unique<SfxItemSet>(
        SfxItemSet::makeFixedSfxItemSet<EE_ITEMS_START, EE_ITEMS_END>(rModel.GetItemPool()));

    // Load the buffer using pdfium.
    auto const& rVectorGraphicData = rGraphic.getVectorGraphicData();
    const auto* pData = rVectorGraphicData->getBinaryDataContainer().getData();
    sal_Int32 nSize = rVectorGraphicData->getBinaryDataContainer().getSize();
    mpPdfDocument = mpPDFium ? mpPDFium->openDocument(pData, nSize, OString()) : nullptr;
    if (!mpPdfDocument)
        return;

    mnPageCount = mpPdfDocument->getPageCount();

    const std::shared_ptr<GfxLink> xGrfLink = rGraphic.GetSharedGfxLink();
    if (xGrfLink)
        mxImportedFonts = xGrfLink->getImportedFonts();
    if (!mxImportedFonts)
    {
        mxImportedFonts
            = std::make_shared<ImportedFontMap>(CollectFonts(getPrefix(), *mpPdfDocument));
        if (xGrfLink)
            xGrfLink->setImportedFonts(mxImportedFonts);
    }

    // Same as SdModule
    mpVD = VclPtr<VirtualDevice>::Create();
    mpVD->SetReferenceDevice(VirtualDevice::RefDevMode::Dpi600);
    mpVD->SetMapMode(MapMode(MapUnit::Map100thMM));
    mpVD->EnableOutput(false);
    mpVD->SetLineColor();

    // Get TextBounds relative to baseline
    vcl::Font aFnt = mpVD->GetFont();
    aFnt.SetAlignment(ALIGN_BASELINE);
    aFnt.SetFamily(FAMILY_DONTKNOW);
    mpVD->SetFont(aFnt);
}

ImpSdrPdfImport::~ImpSdrPdfImport() = default;

namespace
{
OUString GetPostScriptName(const OUString& rBaseFontName)
{
    OUString sPostScriptName = rBaseFontName;
    /* For a font subset, the PostScript name of the font—the value of the
         * font’s BaseFont entry and the font descriptor’s FontName entry shall
         * begin with a tag followed by a plus sign (+). The tag shall consist of
         * exactly six uppercase letters; the choice of letters is arbitrary, but
         * different subsets in the same PDF file shall have different tags */
    if (sPostScriptName.getLength() > 6 && sPostScriptName[6] == '+')
        sPostScriptName = sPostScriptName.copy(7);
    return sPostScriptName;
}

bool writeFontFile(const OUString& fileUrl, const std::vector<uint8_t>& rFontData)
{
    SAL_INFO("sd.filter", "dumping to: " << fileUrl);

    osl::File file(fileUrl);
    switch (file.open(osl_File_OpenFlag_Create | osl_File_OpenFlag_Write))
    {
        case osl::File::E_None:
            break; // ok
        case osl::File::E_EXIST:
            return true; // Assume it's already been added correctly.
        default:
            SAL_WARN("sd.filter", "Cannot open file for temporary font");
            return false;
    }
    sal_uInt64 writtenTotal = 0;
    while (writtenTotal < rFontData.size())
    {
        sal_uInt64 written;
        if (file.write(rFontData.data() + writtenTotal, rFontData.size() - writtenTotal, written)
            != osl::File::E_None)
        {
            SAL_WARN("sd.filter", "Error writing temporary font file");
            osl::File::remove(fileUrl);
            return false;
        }
        writtenTotal += written;
    }

    return true;
}

FontWeight toOfficeWeight(std::string_view style)
{
    if (o3tl::equalsIgnoreAsciiCase(style, "Regular"))
        return WEIGHT_NORMAL;
    else if (o3tl::equalsIgnoreAsciiCase(style, "Bold"))
        return WEIGHT_BOLD;
    else if (o3tl::equalsIgnoreAsciiCase(style, "BoldItalic"))
        return WEIGHT_BOLD;
    return WEIGHT_DONTKNOW;
}

OUString stripPostScriptStyle(const OUString& postScriptName, FontWeight& eWeight)
{
    OUString sFontName;
    sal_Int32 lastDash = postScriptName.lastIndexOf('-');
    if (lastDash == -1)
    {
        sFontName = postScriptName;
        eWeight = WEIGHT_NORMAL;
    }
    else
    {
        sFontName = postScriptName.copy(0, lastDash);
        eWeight = toOfficeWeight(postScriptName.copy(lastDash + 1).toUtf8());
    }
    return sFontName;
}

OUString getFileUrlForTemporaryFont(sal_Int64 prefix, std::u16string_view name,
                                    std::u16string_view suffix)
{
    return EmbeddedFontsManager::getFileUrlForTemporaryFont(
        Concat2View(OUString::number(prefix) + name), suffix);
}
}

// Possibly there is some alternative route to query pdfium for all fonts without
// iterating through every object to see what font each uses
ImportedFontMap ImpSdrPdfImport::CollectFonts(sal_Int64 nPrefix,
                                              vcl::pdf::PDFiumDocument& rPdfDocument)
{
    ImportedFontMap aImportedFonts;

    std::map<OUString, SubSetInfo> aDifferentSubsetsForFont;
    // map of PostScriptName->Merged Font File for that font
    std::map<OUString, EmbeddedFontInfo> aEmbeddedFonts;

    const int nPageCount = rPdfDocument.getPageCount();

    for (int nPageIndex = 0; nPageIndex < nPageCount; ++nPageIndex)
    {
        auto pPdfPage = rPdfDocument.openPage(nPageIndex);
        if (!pPdfPage)
        {
            SAL_WARN("sd.filter", "ImpSdrPdfImport missing page: " << nPageIndex);
            continue;
        }
        auto pTextPage = pPdfPage->getTextPage();

        const int nPageObjectCount = pPdfPage->getObjectCount();
        for (int nPageObjectIndex = 0; nPageObjectIndex < nPageObjectCount; ++nPageObjectIndex)
        {
            auto pPageObject = pPdfPage->getObject(nPageObjectIndex);
            if (!pPageObject)
            {
                SAL_WARN("sd.filter", "ImpSdrPdfImport missing object: "
                                          << nPageObjectIndex << " on page: " << nPageIndex);
                continue;
            }

            const vcl::pdf::PDFPageObjectType ePageObjectType = pPageObject->getType();
            if (ePageObjectType != vcl::pdf::PDFPageObjectType::Text)
                continue;
            std::unique_ptr<vcl::pdf::PDFiumFont> font = pPageObject->getFont();
            if (!font)
                continue;

            auto itImportedFont = aImportedFonts.find(font->getFontDictObjNum());
            if (itImportedFont == aImportedFonts.end())
            {
                OUString sPostScriptName = GetPostScriptName(pPageObject->getBaseFontName());

                OUString sFontName = pPageObject->getFontName();

                FontWeight eFontWeight(WEIGHT_DONTKNOW);
                OUString sPostScriptFontFamily = stripPostScriptStyle(sPostScriptName, eFontWeight);

                if (sFontName.isEmpty())
                {
                    sFontName = sPostScriptFontFamily;
                    SAL_WARN("sd.filter",
                             "missing font name, attempt to reconstruct from postscriptname as: "
                                 << sFontName);
                }

                if (!font->getIsEmbedded())
                {
                    SAL_WARN("sd.filter", "skipping not embedded font, map: "
                                              << sFontName << " to " << sPostScriptFontFamily);
                    aImportedFonts.insert(OfficeFontInfo{ font->getFontDictObjNum(),
                                                          sPostScriptFontFamily, eFontWeight });
                    continue;
                }

                std::vector<uint8_t> aFontData;
                if (!font->getFontData(aFontData) || aFontData.empty())
                {
                    SAL_WARN("sd.filter", "that's worrying, skipping " << sFontName);
                    continue;
                }

                SubSetInfo* pSubSetInfo;

                SAL_INFO("sd.filter", "importing font: " << font);
                auto itFontName = aDifferentSubsetsForFont.find(sPostScriptName);
                OUString sFontFileName = sPostScriptName;
                if (itFontName != aDifferentSubsetsForFont.end())
                {
                    sFontFileName += OUString::number(itFontName->second.aComponents.size());
                    itFontName->second.aComponents.emplace_back();
                    pSubSetInfo = &itFontName->second;
                }
                else
                {
                    sFontFileName += "0";
                    SubSetInfo aSubSetInfo;
                    aSubSetInfo.aComponents.emplace_back();
                    auto result
                        = aDifferentSubsetsForFont.emplace(sPostScriptName, aSubSetInfo).first;
                    pSubSetInfo = &result->second;
                }
                bool bTTF = EmbeddedFontsManager::analyzeTTF(aFontData.data(), aFontData.size(),
                                                             eFontWeight);
                SAL_INFO_IF(!bTTF, "sd.filter", "not ttf/otf, converting");
                OUString fileUrl
                    = getFileUrlForTemporaryFont(nPrefix, sFontFileName, bTTF ? u".ttf" : u".t1");
                if (!writeFontFile(fileUrl, aFontData))
                    SAL_WARN("sd.filter", "ttf not written");
                else
                    SAL_INFO("sd.filter", "ttf written to: " << fileUrl);
                std::vector<uint8_t> aToUnicodeData;
                if (!font->getFontToUnicode(aToUnicodeData))
                    SAL_WARN("sd.filter", "that's maybe worrying");
                if (!bTTF || !aToUnicodeData.empty())
                {
                    EmbeddedFontInfo fontInfo
                        = convertToOTF(nPrefix, *pSubSetInfo, fileUrl, sFontName, sPostScriptName,
                                       sFontFileName, aToUnicodeData, *font);
                    fileUrl = fontInfo.sFontFile;
                    sFontName = fontInfo.sFontName;
                    eFontWeight = fontInfo.eFontWeight;
                }

                if (fileUrl.getLength())
                {
                    aImportedFonts.insert(
                        OfficeFontInfo{ font->getFontDictObjNum(), sFontName, eFontWeight });
                    aEmbeddedFonts[sPostScriptName]
                        = EmbeddedFontInfo{ sFontName, fileUrl, eFontWeight };
                }
            }
            else
            {
                SAL_INFO("sd.filter", "already saw font " << font << " and used "
                                                          << itImportedFont->sFontName
                                                          << " as name");
            }
        }
    }

    if (!aEmbeddedFonts.empty())
    {
        EmbeddedFontsManager aEmbeddedFontsManager(nullptr);
        for (const auto& fontinfo : aEmbeddedFonts)
        {
            aEmbeddedFontsManager.addEmbeddedFont(fontinfo.second.sFontFile,
                                                  fontinfo.second.sFontName, true);
        }
    }

    return aImportedFonts;
}

void ImpSdrPdfImport::DoObjects(SvdProgressInfo* pProgrInfo, sal_uInt32* pActionsToReport,
                                int nPageIndex)
{
    const int nPageCount = mpPdfDocument->getPageCount();
    if (!(nPageCount > 0 && nPageIndex >= 0 && nPageIndex < nPageCount))
        return;

    // Render next page.
    auto pPdfPage = mpPdfDocument->openPage(nPageIndex);
    if (!pPdfPage)
        return;

    basegfx::B2DSize dPageSize = mpPdfDocument->getPageSize(nPageIndex);

    SetupPageScale(dPageSize.getWidth(), dPageSize.getHeight());

    // Load the page text to extract it when we get text elements.
    auto pTextPage = pPdfPage->getTextPage();

    const int nPageObjectCount = pPdfPage->getObjectCount();
    if (pProgrInfo)
        pProgrInfo->SetActionCount(nPageObjectCount);

    for (int nPageObjectIndex = 0; nPageObjectIndex < nPageObjectCount; ++nPageObjectIndex)
    {
        auto pPageObject = pPdfPage->getObject(nPageObjectIndex);
        ImportPdfObject(pPageObject, pPdfPage, pTextPage, nPageObjectIndex);
        if (pProgrInfo && pActionsToReport)
        {
            (*pActionsToReport)++;

            if (*pActionsToReport >= 16)
            {
                if (!pProgrInfo->ReportActions(*pActionsToReport))
                    break;

                *pActionsToReport = 0;
            }
        }
    }
}

void ImpSdrPdfImport::SetupPageScale(const double dPageWidth, const double dPageHeight)
{
    mfScaleX = mfScaleY = 1.0;

    // Store the page dimensions in Points.
    mdPageHeightPts = dPageHeight;

    Size aPageSize(convertPointToMm100(dPageWidth), convertPointToMm100(dPageHeight));
    Size aScaleRectSize = maScaleRect.GetSize();

    if (aPageSize.Width() && aPageSize.Height() && (!maScaleRect.IsEmpty()))
    {
        maOfs = maScaleRect.TopLeft();

        if (aPageSize.Width() != aScaleRectSize.Width())
        {
            mfScaleX = static_cast<double>(aScaleRectSize.Width())
                       / static_cast<double>(aPageSize.Width());
        }

        if (aPageSize.Height() != aScaleRectSize.Height())
        {
            mfScaleY = static_cast<double>(aScaleRectSize.Height())
                       / static_cast<double>(aPageSize.Height());
        }
    }

    mbMov = maOfs.X() != 0 || maOfs.Y() != 0;
    mbSize = false;
    maScaleX = Fraction(1, 1);
    maScaleY = Fraction(1, 1);

    if (aPageSize.Width() != aScaleRectSize.Width())
    {
        maScaleX = Fraction(aScaleRectSize.Width(), aPageSize.Width());
        mbSize = true;
    }

    if (aPageSize.Height() != aScaleRectSize.Height())
    {
        maScaleY = Fraction(aScaleRectSize.Height(), aPageSize.Height());
        mbSize = true;
    }
}

size_t ImpSdrPdfImport::DoImport(SdrObjList& rOL, size_t nInsPos, int nPageNumber,
                                 SvdProgressInfo* pProgrInfo)
{
    sal_uInt32 nActionsToReport(0);

    // execute
    DoObjects(pProgrInfo, &nActionsToReport, nPageNumber);

    if (pProgrInfo)
    {
        pProgrInfo->ReportActions(nActionsToReport);
        nActionsToReport = 0;
    }

    // MapMode scaling
    MapScaling();

    // To calculate the progress meter, we use GetActionSize()*3.
    // However, maTmpList has a lower entry count limit than GetActionSize(),
    // so the actions that were assumed were too much have to be re-added.
    // nActionsToReport = (rMtf.GetActionSize() - maTmpList.size()) * 2;

    // announce all currently unannounced rescales
    if (pProgrInfo)
    {
        pProgrInfo->ReportRescales(nActionsToReport);
        pProgrInfo->SetInsertCount(maTmpList.size());
    }

    nActionsToReport = 0;

    // insert all objects cached in aTmpList now into rOL from nInsPos
    nInsPos = std::min(nInsPos, rOL.GetObjCount());

    for (rtl::Reference<SdrObject>& pObj : maTmpList)
    {
        rOL.NbcInsertObject(pObj.get(), nInsPos);
        nInsPos++;

        if (pProgrInfo)
        {
            nActionsToReport++;

            if (nActionsToReport >= 32) // update all 32 actions
            {
                pProgrInfo->ReportInserts(nActionsToReport);
                nActionsToReport = 0;
            }
        }
    }

    // report all remaining inserts for the last time
    if (pProgrInfo)
    {
        pProgrInfo->ReportInserts(nActionsToReport);
    }

    return maTmpList.size();
}

void ImpSdrPdfImport::SetAttributes(SdrObject* pObj, bool bForceTextAttr)
{
    mbNoLine = false;
    mbNoFill = false;
    bool bLine(!bForceTextAttr);
    bool bFill(!pObj || (pObj->IsClosedObj() && !bForceTextAttr));
    bool bText(bForceTextAttr || (pObj && pObj->GetOutlinerParaObject()));

    if (bLine)
    {
        if (mnLineWidth)
        {
            mpLineAttr->Put(XLineWidthItem(mnLineWidth));
        }
        else
        {
            mpLineAttr->Put(XLineWidthItem(0));
        }

        if (mpVD->IsLineColor())
        {
            mpLineAttr->Put(XLineStyleItem(drawing::LineStyle_SOLID)); //TODO support dashed lines.
            mpLineAttr->Put(XLineColorItem(OUString(), mpVD->GetLineColor()));
        }
        else
        {
            mpLineAttr->Put(XLineStyleItem(drawing::LineStyle_NONE));
        }

        mpLineAttr->Put(XLineJointItem(css::drawing::LineJoint_NONE));

        // Add LineCap support
        mpLineAttr->Put(XLineCapItem(gaLineCap));

        if (((maDash.GetDots() && maDash.GetDotLen())
             || (maDash.GetDashes() && maDash.GetDashLen()))
            && maDash.GetDistance())
        {
            mpLineAttr->Put(XLineDashItem(OUString(), maDash));
        }
        else
        {
            mpLineAttr->Put(XLineDashItem(OUString(), XDash(css::drawing::DashStyle_RECT)));
        }
    }
    else
    {
        mbNoLine = true;
    }

    if (bFill)
    {
        bool doFill
            = mpVD->GetDrawMode() != DrawModeFlags::NoFill && (moFillColor || moFillPattern);
        if (doFill && moFillColor)
            doFill = !moFillColor->IsTransparent();
        if (doFill)
        {
            if (moFillColor)
            {
                mpFillAttr->Put(XFillStyleItem(drawing::FillStyle_SOLID));
                mpFillAttr->Put(XFillColorItem(OUString(), *moFillColor));
            }
            else
            {
                assert(moFillPattern && "pattern should exist");
                mpFillAttr->Put(XFillStyleItem(drawing::FillStyle_BITMAP));
                mpFillAttr->Put(XFillBitmapItem(OUString(), Graphic(*moFillPattern)));
                mpFillAttr->Put(XFillBmpStretchItem(false));
                mpFillAttr->Put(XFillBmpTileItem(true));
                mpFillAttr->Put(
                    XFillBmpSizeXItem(convertPointToMm100(moFillPattern->GetSizePixel().Width())));
                mpFillAttr->Put(
                    XFillBmpSizeYItem(convertPointToMm100(moFillPattern->GetSizePixel().Height())));
            }
        }
        else
        {
            mpFillAttr->Put(XFillStyleItem(drawing::FillStyle_NONE));
        }
    }
    else
    {
        mbNoFill = true;
    }

    if (bText && mbFntDirty)
    {
        vcl::Font aFnt(mpVD->GetFont());
        const sal_uInt32 nHeight(
            basegfx::fround<sal_uInt32>(aFnt.GetFontSize().Height() * mfScaleY));

        mpTextAttr->Put(SvxFontItem(aFnt.GetFamilyType(), aFnt.GetFamilyName(), aFnt.GetStyleName(),
                                    aFnt.GetPitchMaybeAskConfig(), aFnt.GetCharSet(),
                                    EE_CHAR_FONTINFO));
        mpTextAttr->Put(SvxFontItem(aFnt.GetFamilyType(), aFnt.GetFamilyName(), aFnt.GetStyleName(),
                                    aFnt.GetPitchMaybeAskConfig(), aFnt.GetCharSet(),
                                    EE_CHAR_FONTINFO_CJK));
        mpTextAttr->Put(SvxFontItem(aFnt.GetFamilyType(), aFnt.GetFamilyName(), aFnt.GetStyleName(),
                                    aFnt.GetPitchMaybeAskConfig(), aFnt.GetCharSet(),
                                    EE_CHAR_FONTINFO_CTL));
        mpTextAttr->Put(SvxPostureItem(aFnt.GetItalicMaybeAskConfig(), EE_CHAR_ITALIC));
        mpTextAttr->Put(SvxWeightItem(aFnt.GetWeightMaybeAskConfig(), EE_CHAR_WEIGHT));
        mpTextAttr->Put(SvxFontHeightItem(nHeight, 100, EE_CHAR_FONTHEIGHT));
        mpTextAttr->Put(SvxFontHeightItem(nHeight, 100, EE_CHAR_FONTHEIGHT_CJK));
        mpTextAttr->Put(SvxFontHeightItem(nHeight, 100, EE_CHAR_FONTHEIGHT_CTL));
        mpTextAttr->Put(SvxCharScaleWidthItem(100, EE_CHAR_FONTWIDTH));
        mpTextAttr->Put(SvxUnderlineItem(aFnt.GetUnderline(), EE_CHAR_UNDERLINE));
        mpTextAttr->Put(SvxOverlineItem(aFnt.GetOverline(), EE_CHAR_OVERLINE));
        mpTextAttr->Put(SvxCrossedOutItem(aFnt.GetStrikeout(), EE_CHAR_STRIKEOUT));
        mpTextAttr->Put(SvxShadowedItem(aFnt.IsShadow(), EE_CHAR_SHADOW));

        // #i118485# Setting this item leads to problems (written #i118498# for this)
        // mpTextAttr->Put(SvxAutoKernItem(aFnt.IsKerning(), EE_CHAR_KERNING));

        mpTextAttr->Put(SvxWordLineModeItem(aFnt.IsWordLineMode(), EE_CHAR_WLM));
        mpTextAttr->Put(SvxContourItem(aFnt.IsOutline(), EE_CHAR_OUTLINE));
        mpTextAttr->Put(SvxColorItem(mpVD->GetTextColor(), EE_CHAR_COLOR));
        //... svxfont textitem svditext
        mbFntDirty = false;
    }

    if (!pObj)
        return;

    pObj->SetLayer(mnLayer);

    if (bLine)
    {
        pObj->SetMergedItemSet(*mpLineAttr);
    }

    if (bFill)
    {
        pObj->SetMergedItemSet(*mpFillAttr);
    }

    if (bText)
    {
        pObj->SetMergedItemSet(*mpTextAttr);
        pObj->SetMergedItem(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_LEFT));
    }
}

void ImpSdrPdfImport::InsertObj(SdrObject* pObj1, bool bScale)
{
    rtl::Reference<SdrObject> pObj = pObj1;
    if (bScale && !maScaleRect.IsEmpty())
    {
        if (mbSize)
        {
            pObj->NbcResize(Point(), maScaleX, maScaleY);
        }

        if (mbMov)
        {
            pObj->NbcMove(Size(maOfs.X(), maOfs.Y()));
        }
    }

    if (!pObj)
        return;

    // #i111954# check object for visibility
    // used are SdrPathObj, SdrRectObj, SdrCircObj, SdrGrafObj
    bool bVisible(false);

    if (pObj->HasLineStyle())
    {
        bVisible = true;
    }

    if (!bVisible && pObj->HasFillStyle())
    {
        bVisible = true;
    }

    if (!bVisible)
    {
        SdrTextObj* pTextObj = DynCastSdrTextObj(pObj.get());

        if (pTextObj && pTextObj->HasText())
        {
            bVisible = true;
        }
    }

    if (!bVisible)
    {
        SdrGrafObj* pGrafObj = dynamic_cast<SdrGrafObj*>(pObj.get());

        if (pGrafObj)
        {
            // this may be refined to check if the graphic really is visible. It
            // is here to ensure that graphic objects without fill, line and text
            // get created
            bVisible = true;
        }
    }

    if (bVisible)
    {
        maTmpList.push_back(pObj);

        if (dynamic_cast<SdrPathObj*>(pObj.get()))
        {
            const bool bClosed(pObj->IsClosedObj());

            mbLastObjWasPolyWithoutLine = mbNoLine && bClosed;
        }
        else
        {
            mbLastObjWasPolyWithoutLine = false;
        }
    }
}

bool ImpSdrPdfImport::CheckLastPolyLineAndFillMerge(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    // #i73407# reformulation to use new B2DPolygon classes
    if (mbLastObjWasPolyWithoutLine)
    {
        SdrObject* pTmpObj = !maTmpList.empty() ? maTmpList[maTmpList.size() - 1].get() : nullptr;
        SdrPathObj* pLastPoly = dynamic_cast<SdrPathObj*>(pTmpObj);

        if (pLastPoly)
        {
            if (pLastPoly->GetPathPoly() == rPolyPolygon)
            {
                SetAttributes(nullptr);

                if (!mbNoLine && mbNoFill)
                {
                    pLastPoly->SetMergedItemSet(*mpLineAttr);

                    return true;
                }
            }
        }
    }

    return false;
}

void ImpSdrPdfImport::ImportPdfObject(
    std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
    std::unique_ptr<vcl::pdf::PDFiumPage> const& pPage,
    std::unique_ptr<vcl::pdf::PDFiumTextPage> const& pTextPage, int nPageObjectIndex)
{
    if (!pPageObject)
        return;

    const vcl::pdf::PDFPageObjectType ePageObjectType = pPageObject->getType();
    switch (ePageObjectType)
    {
        case vcl::pdf::PDFPageObjectType::Text:
            ImportText(pPageObject, pPage, pTextPage, nPageObjectIndex);
            break;
        case vcl::pdf::PDFPageObjectType::Path:
            ImportPath(pPageObject, pPage, nPageObjectIndex);
            break;
        case vcl::pdf::PDFPageObjectType::Image:
            ImportImage(pPageObject, nPageObjectIndex);
            break;
        case vcl::pdf::PDFPageObjectType::Shading:
            SAL_WARN("sd.filter", "Got page object SHADING: " << nPageObjectIndex);
            break;
        case vcl::pdf::PDFPageObjectType::Form:
            ImportForm(pPageObject, pPage, pTextPage, nPageObjectIndex);
            break;
        default:
            SAL_WARN("sd.filter", "Unknown PDF page object #" << nPageObjectIndex << " of type: "
                                                              << static_cast<int>(ePageObjectType));
            break;
    }
}

void ImpSdrPdfImport::ImportForm(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                                 std::unique_ptr<vcl::pdf::PDFiumPage> const& pPage,
                                 std::unique_ptr<vcl::pdf::PDFiumTextPage> const& pTextPage,
                                 int /*nPageObjectIndex*/)
{
    // Get the form matrix to perform correct translation/scaling of the form sub-objects.
    const basegfx::B2DHomMatrix aOldMatrix = maCurrentMatrix;

    maCurrentMatrix = pPageObject->getMatrix();

    const int nCount = pPageObject->getFormObjectCount();
    for (int nIndex = 0; nIndex < nCount; ++nIndex)
    {
        auto pFormObject = pPageObject->getFormObject(nIndex);

        ImportPdfObject(pFormObject, pPage, pTextPage, -1);
    }

    // Restore the old one.
    maCurrentMatrix = aOldMatrix;
}

static bool extractEntry(std::string_view line, std::string_view key, OString& ret)
{
    std::string_view result;
    if (o3tl::starts_with(line, key, &result))
    {
        result = o3tl::trim(result);
        if (result[0] == '"' && result[result.size() - 1] == '"')
            result = result.substr(1, result.size() - 2);
        ret = OString(result);
        return true;
    }
    return false;
}

static bool isSimpleFamilyName(std::string_view Weight)
{
    return Weight.empty() || Weight == "Regular" || Weight == "Italic" || Weight == "Bold"
           || Weight == "BoldItalic";
}

// a) change brokenName/brokenCIDName if present to fixedName
// b) remove preamble FontMatrix and overwrite following ones in the %ADOBeginFontDict
// section with that content instead to avoid generating fonts with unusual UnitsPerEm
// of 1 that freetype will reject
static void rewriteFont(std::string_view brokenName, std::string_view brokenCIDName,
                        std::string_view fixedName, const OUString& pfaCIDUrl)
{
    OUString oldCIDUrl = pfaCIDUrl + ".broken";
    if (osl::File::move(pfaCIDUrl, oldCIDUrl) != osl::File::E_None)
    {
        SAL_WARN("sd.filter", "unable to move file");
        return;
    }

    const bool rewriteNames = !brokenName.empty();

    bool fontDict = false;

    OString sGlobalMatrix;

    const OString sBrokenFontLine = "/FontName /"_ostr + brokenName + " def"_ostr;
    const OString sFixedFontLine = "/FontName /"_ostr + fixedName + " def"_ostr;

    const OString sBrokenCIDFontLine = "/CIDFontName /"_ostr + brokenCIDName + " def"_ostr;
    const OString sFixedCIDFontLine = "/CIDFontName /"_ostr + fixedName + " def"_ostr;

    SvFileStream input(oldCIDUrl, StreamMode::READ);
    SvFileStream output(pfaCIDUrl, StreamMode::WRITE | StreamMode::TRUNC);
    OString sLine;
    while (input.ReadLine(sLine))
    {
        if (rewriteNames)
        {
            if (sLine == sBrokenFontLine)
            {
                output.WriteLine(sFixedFontLine);
                continue;
            }
            else if (sLine == sBrokenCIDFontLine)
            {
                output.WriteLine(sFixedCIDFontLine);
                continue;
            }
        }

        if (sLine.startsWith("%ADOBeginFontDict"))
            fontDict = true;
        else if (sLine.startsWith("/FontMatrix "))
        {
            if (!fontDict)
            {
                // Global case, stash and don't emit the global matrix
                sGlobalMatrix = sLine;
                continue;
            }

            if (!sGlobalMatrix.isEmpty())
            {
                // Local case, emit the global matrix instead if
                // there was one, otherwise emit the local matrix
                output.WriteLine(sGlobalMatrix);
                continue;
            }
        }

        output.WriteLine(sLine);
        if (sLine.startsWith("%%BeginData"))
        {
            // Write the rest direct as-is and quit
            output.WriteStream(input);
            break;
        }
    }
}

// https://ccjktype.fonts.adobe.com/2011/12/leveraging-afdko-part-1.html
// https://ccjktype.fonts.adobe.com/2012/01/leveraging-afdko-part-2.html
// https://ccjktype.fonts.adobe.com/2012/01/leveraging-afdko-part-3.html
static bool toPfaCID(SubSetInfo& rSubSetInfo, const OUString& fileUrl,
                     const OUString& postScriptName, bool& bNameKeyed,
                     std::map<int, int>& nameIndexToGlyph, OString& FontName, OString& Weight,
                     OString& FSType)
{
    bNameKeyed = false;
    FSType = "0"_ostr;

    OUString infoUrl = fileUrl + u".1";
    OUString cidFontInfoUrl = fileUrl + u".cidfontinfo";
    OUString nameToCIDMapUrl = fileUrl + u".nametocidmap";
    OUString toMergedMapUrl = fileUrl + u".tomergedmap";

    OString version, Notice, FullName, FamilyName, CIDFontName, CIDFontVersion, srcFontType,
        glyphTag, FontMatrix;
    OString brokenFontName;
    FontName = postScriptName.toUtf8();
    std::map<sal_Int32, OString> glyphIndexToName;

    OUString pfaUrl, pfaCIDUrl;

    rSubSetInfo.aComponents.back().cidFontInfoUrl = cidFontInfoUrl;
    rSubSetInfo.aComponents.back().toMergedMapUrl = toMergedMapUrl;

    pfaUrl = fileUrl + u".pfa";
    pfaCIDUrl = pfaUrl + u".cid";

    rSubSetInfo.aComponents.back().pfaCIDUrl = pfaCIDUrl;

    if (!EmbeddedFontsManager::tx_dump(fileUrl, infoUrl))
    {
        SAL_WARN("sd.filter", "font file info extraction failed");
        return false;
    }
    SAL_INFO("sd.filter", "dump success");
    SvFileStream info(infoUrl, StreamMode::READ);
    OStringBuffer glyphBuffer;
    OString sLine;
    while (info.ReadLine(sLine))
    {
        if (extractEntry(sLine, "version", version))
            continue;
        if (extractEntry(sLine, "cid.CIDFontVersion", CIDFontVersion))
            continue;
        if (extractEntry(sLine, "Notice", Notice))
            continue;
        if (extractEntry(sLine, "FullName", FullName))
            continue;
        if (extractEntry(sLine, "FamilyName", FamilyName))
            continue;
        if (extractEntry(sLine, "FSType", FSType))
            continue;
        if (extractEntry(sLine, "Weight", Weight))
            continue;
        if (extractEntry(sLine, "FontMatrix", FontMatrix))
            continue;
        if (extractEntry(sLine, "sup.srcFontType", srcFontType))
            continue;
        if (extractEntry(sLine, "## glyph[tag]", glyphTag))
            continue;
        if (extractEntry(sLine, "FontName", FontName))
        {
            const bool bBrokenFontName = FontName != postScriptName.toUtf8();
            if (bBrokenFontName)
            {
                SAL_WARN("sd.filter", "expected that FontName of <"
                                          << FontName << "> matches PostScriptName of <"
                                          << postScriptName << ">");
                brokenFontName = FontName;
            }
            continue;
        }
        if (extractEntry(sLine, "cid.CIDFontName", CIDFontName))
        {
            SAL_WARN_IF(CIDFontName != postScriptName.toUtf8(), "sd.filter",
                        "expected that cid.CIDFontName of <"
                            << CIDFontName << "> matches PostScriptName of <" << postScriptName
                            << ">");
            continue;
        }
        if (sLine.startsWith("glyph["))
        {
            sal_Int32 i = 6;
            while (i < sLine.getLength())
            {
                ++i;
                if (sLine[i - 1] == ']')
                    break;
                glyphBuffer.append(sLine[i - 1]);
            }
            sal_Int32 nGlyphIndex = glyphBuffer.makeStringAndClear().toInt32();
            while (i < sLine.getLength())
            {
                ++i;
                if (sLine[i - 1] == '{')
                    break;
            }
            while (i < sLine.getLength())
            {
                ++i;
                if (sLine[i - 1] == ',')
                    break;
                glyphBuffer.append(sLine[i - 1]);
            }
            SAL_INFO("sd.filter",
                     "setting index: " << nGlyphIndex << " as: " << glyphBuffer.toString());
            glyphIndexToName[nGlyphIndex] = glyphBuffer.makeStringAndClear();
            while (i < sLine.getLength())
            {
                ++i;
                if (sLine[i - 1] == '}')
                    break;
                glyphBuffer.append(sLine[i - 1]);
            }
            SAL_INFO("sd.filter", "code text: " << glyphBuffer.toString());
            OString sCode = glyphBuffer.makeStringAndClear();
            sal_Int32 nCodePoint
                = sCode.startsWith("0x") ? o3tl::toInt32(sCode.subView(2), 16) : sCode.toInt32();
            SAL_INFO("sd.filter", "codepoint is: " << nCodePoint);
            nameIndexToGlyph[nCodePoint] = nGlyphIndex;
        }
    }
    SAL_INFO("sd.filter", "details are: " << version << Notice << FullName << FamilyName << Weight
                                          << srcFontType << FontName << CIDFontName
                                          << CIDFontVersion);

    if (version.isEmpty())
        version = CIDFontVersion;

    if (version.isEmpty() || version.toDouble() == 0.0)
    {
        SAL_WARN("sd.filter", "Font version cannot be empty or 0.0");
        version = "0.001"_ostr;
    }

    if (!brokenFontName.isEmpty())
        FontName = postScriptName.toUtf8();

    // Always create cidFontInfo, we will need it if we need to merge fonts
    OString AdobeCopyright, Trademark;
    sal_Int32 nSplit = !Notice.isEmpty() ? Notice.lastIndexOf('.', Notice.getLength() - 1) : -1;
    if (nSplit != -1)
    {
        AdobeCopyright = Notice.copy(0, nSplit + 1);
        Trademark = Notice.copy(nSplit + 1);
    }
    else
        AdobeCopyright = Notice;
    SvFileStream cidFontInfo(cidFontInfoUrl, StreamMode::READWRITE | StreamMode::TRUNC);
    cidFontInfo.WriteLine(Concat2View("FontName\t(" + FontName + ")"));
    cidFontInfo.WriteLine(Concat2View("FullName\t(" + FullName + ")"));
    OString OutputFamilyName = FamilyName;
    if (!isSimpleFamilyName(Weight))
        OutputFamilyName = OutputFamilyName + " " + Weight;
    cidFontInfo.WriteLine(Concat2View("FamilyName\t(" + OutputFamilyName + ")"));
    cidFontInfo.WriteLine(Concat2View("version\t\t(" + version + ")"));
    cidFontInfo.WriteLine("Registry\t(Adobe)");
    cidFontInfo.WriteLine("Ordering\t(Identity)");
    cidFontInfo.WriteLine("Supplement\t0");
    cidFontInfo.WriteLine("XUID\t\t[1 11 9273828]");
    cidFontInfo.WriteLine(Concat2View("FSType\t\t" + FSType));
    cidFontInfo.WriteLine(Concat2View("AdobeCopyright\t(" + AdobeCopyright + ")"));
    cidFontInfo.WriteLine(Concat2View("Trademark\t(" + Trademark + ")"));
    cidFontInfo.Close();

    /*
      ## glyph[tag] {cid,iFD}
      or
      ## glyph[tag] {name,encoding}
    */
    bNameKeyed = glyphTag == "{name,encoding}";
    if (bNameKeyed)
    {
        SAL_INFO("sd.filter", "convert to cid keyed");
        SvFileStream nameToCIDMap(nameToCIDMapUrl, StreamMode::READWRITE | StreamMode::TRUNC);
        nameToCIDMap.WriteLine(Concat2View("mergefonts " + FontName + " 0"));
        for (const auto& glyph : glyphIndexToName)
        {
            SAL_INFO("sd.filter", "writing index: " << glyph.first << " as: " << glyph.second);
            nameToCIDMap.WriteLine(Concat2View(OString::number(glyph.first) + "\t" + glyph.second));
        }
        nameToCIDMap.Close();

        if (!EmbeddedFontsManager::tx_t1(fileUrl, pfaUrl))
        {
            SAL_WARN("sd.filter", "pfa conversion failed");
            return false;
        }

        std::vector<std::pair<OUString, OUString>> fonts;
        fonts.push_back(std::make_pair(nameToCIDMapUrl, pfaUrl));
        if (!EmbeddedFontsManager::mergefonts(cidFontInfoUrl, pfaCIDUrl, fonts))
        {
            SAL_WARN("sd.filter", "conversion 2 failed");
            return false;
        }
    }
    else
    {
        if (!EmbeddedFontsManager::tx_t1(fileUrl, pfaCIDUrl))
        {
            SAL_WARN("sd.filter", "pfa conversion failed");
            return false;
        }
    }

    if (!brokenFontName.isEmpty() || !FontMatrix.isEmpty())
    {
        // If the fontname isn't as expected, or if there is a
        // font matrix present then we rewrite the font.
        rewriteFont(brokenFontName, CIDFontName, FontName, pfaCIDUrl);
    }

    return true;
}

static void appendFourByteHex(OStringBuffer& rBuffer, sal_uInt32 nNumber)
{
    OString sCodePoint = OString::number(nNumber, 16);
    for (sal_Int32 j = sCodePoint.getLength(); j < 4; ++j)
        rBuffer.append('0');
    rBuffer.append(sCodePoint);
}

static OString decomposeLegacyUnicodeLigature(UChar32 cUnicode)
{
    UErrorCode eCode(U_ZERO_ERROR);

    // Put in any legacy unicode ligature decompositions, which we detect as U_DT_COMPAT and
    // a multi-character decomposition.
    UDecompositionType decompType = static_cast<UDecompositionType>(
        u_getIntPropertyValue(cUnicode, UCHAR_DECOMPOSITION_TYPE));
    if (decompType == UDecompositionType::U_DT_COMPAT)
    {
        const icu::Normalizer2* pInstance = icu::Normalizer2::getNFKDInstance(eCode);

        if (pInstance && eCode == U_ZERO_ERROR)
        {
            icu::UnicodeString sDecomposed;
            pInstance->getRawDecomposition(cUnicode, sDecomposed);
            if (sDecomposed.length() > 1)
            {
                OStringBuffer aBuffer;
                for (int32_t i = 0, nLen = sDecomposed.length(); i < nLen; ++i)
                    appendFourByteHex(aBuffer, sDecomposed[i]);
                return aBuffer.makeStringAndClear();
            }
        }
    }

    return OString();
}

const char cmapprefix[] = "%!PS-Adobe-3.0 Resource-CMap\n"
                          "/WMode 0 def\n"
                          "\n"
                          "/CIDInit /ProcSet findresource begin\n"
                          "12 dict begin\n"
                          "begincmap\n"
                          "/CIDSystemInfo\n"
                          "<< /Registry (Adobe)\n"
                          "   /Ordering (UCS)\n"
                          "   /Supplement 0\n"
                          ">> def\n"
                          "/CMapName /Adobe-Identity-UCS def\n"
                          "/CMapType 2 def\n"
                          "1 begincodespacerange\n"
                          "<0000> <ffff>\n"
                          "endcodespacerange\n";

const char cmapsuffix[] = "endcmap\n"
                          "CMapName currentdict /CMap defineresource pop\n"
                          "end\n"
                          "end\n";

namespace
{
struct ToUnicodeData
{
    std::vector<OString> bfcharlines;
    std::vector<OString> bfcharranges;

    // one or the other of these
    const vcl::pdf::PDFiumFont* pFont;
    const std::map<int, int>* pNameIndexToGlyph;

    // For the pdf provided mapping from the font to unicode
    ToUnicodeData(const std::vector<uint8_t>& toUnicodeData, const vcl::pdf::PDFiumFont& pdfFont)
        : pFont(&pdfFont)
        , pNameIndexToGlyph(nullptr)
    {
        SvMemoryStream aInCMap(const_cast<uint8_t*>(toUnicodeData.data()), toUnicodeData.size(),
                               StreamMode::READ);

        OString sLine;
        while (aInCMap.ReadLine(sLine))
        {
            if (sLine.endsWith("beginbfchar"))
            {
                while (aInCMap.ReadLine(sLine))
                {
                    if (sLine.endsWith("endbfchar"))
                        break;
                    bfcharlines.push_back(sLine);
                }
            }
            else if (sLine.endsWith("beginbfrange"))
            {
                while (aInCMap.ReadLine(sLine))
                {
                    if (sLine.endsWith("endbfrange"))
                        break;
                    bfcharranges.push_back(sLine);
                }
            }
        }
    }

    // For name keyed fonts without toUnicode data where we
    // can assume the font encoding is Adobe Standard and
    // reverse map from the name indexes so we can forward
    // map to unicode
    ToUnicodeData(std::map<int, int>& nameIndexToGlyph)
        : pFont(nullptr)
        , pNameIndexToGlyph(&nameIndexToGlyph)
    {
        for (const auto & [ adobe, glyphid ] : nameIndexToGlyph)
        {
            if (glyphid == 0)
                continue;

            const char cChar(adobe);
            OUString sUni(&cChar, 1, RTL_TEXTENCODING_ADOBE_STANDARD);
            sal_Unicode mappedChar = sUni.toChar();

            OStringBuffer aBuffer("<");
            appendFourByteHex(aBuffer, adobe);
            aBuffer.append("> <");
            appendFourByteHex(aBuffer, mappedChar);
            aBuffer.append(">");
            bfcharlines.push_back(aBuffer.toString());
        }
    }

    sal_uInt32 getGlyphIndexFromCharCode(sal_uInt32 nPDFCharCode)
    {
        if (pFont)
            return pFont->getGlyphIndexFromCharCode(nPDFCharCode);
        assert(pNameIndexToGlyph);
        auto it = pNameIndexToGlyph->find(nPDFCharCode);
        return it != pNameIndexToGlyph->end() ? it->second : 0;
    }
};
}

static void buildCMapAndFeatures(const OUString& CMapUrl, SvFileStream& Features,
                                 std::string_view FontName, ToUnicodeData& tud,
                                 SubSetInfo& rSubSetInfo)
{
    SvFileStream CMap(CMapUrl, StreamMode::READWRITE | StreamMode::TRUNC);

    CMap.WriteBytes(cmapprefix, std::size(cmapprefix) - 1);

    sal_Int32 mergeOffset = 1; //Leave space for notdef
    for (const auto& count : rSubSetInfo.aComponents)
        mergeOffset += count.nGlyphCount;

    std::map<sal_Int32, OString> ligatureGlyphToChars;
    std::vector<sal_Int32> glyphs;

    if (!tud.bfcharranges.empty())
    {
        std::vector<OString> cidranges;

        for (const auto& charrange : tud.bfcharranges)
        {
            assert(charrange[0] == '<');
            sal_Int32 nEnd = charrange.indexOf('>', 1);
            assert(charrange[nEnd] == '>');
            sal_Int32 nGlyphRangeStart = o3tl::toInt32(charrange.subView(1, nEnd - 1), 16);

            OString remainder(o3tl::trim(charrange.subView(nEnd + 1)));
            assert(remainder[0] == '<');
            nEnd = remainder.indexOf('>', 1);
            assert(remainder[nEnd] == '>');
            sal_Int32 nGlyphRangeEnd = o3tl::toInt32(remainder.subView(1, nEnd - 1), 16);

            sal_Int32 nGlyphRangeLen = nGlyphRangeEnd - nGlyphRangeStart;

            assert(nGlyphRangeLen >= 0);

            OString sChars(o3tl::trim(remainder.subView(nEnd + 1)));
            assert(sChars[0] == '<' && sChars[sChars.getLength() - 1] == '>');

            // move simple single glyph->char ranges to bfcharlines instead
            if (nGlyphRangeLen == 0)
            {
                OStringBuffer aBuffer("<");
                appendFourByteHex(aBuffer, nGlyphRangeStart);
                aBuffer.append("> " + sChars);
                tud.bfcharlines.push_back(aBuffer.toString());
                continue;
            }

            OString sContents = sChars.copy(1, sChars.getLength() - 2);
            //The assumption that is that cases of ligatures are with a range
            //of a single glyph(?). In which case we have pushed such entries
            //into bfcharlines above.
            assert(sContents.getLength() == 4);
            sal_Int32 nCharRangeStart = o3tl::toInt32(sContents, 16);
            sal_Int32 nCharRangeEnd = nCharRangeStart + nGlyphRangeLen;

            // move simple single glyph->char ranges to bfcharlines instead
            if (nCharRangeStart == nCharRangeEnd)
            {
                OStringBuffer aBuffer("<");
                appendFourByteHex(aBuffer, nGlyphRangeStart);
                aBuffer.append("> <");
                appendFourByteHex(aBuffer, nCharRangeStart);
                aBuffer.append(">");
                tud.bfcharlines.push_back(aBuffer.toString());
                continue;
            }

            OStringBuffer aBuffer("<");
            appendFourByteHex(aBuffer, nCharRangeStart);
            aBuffer.append("> <");
            appendFourByteHex(aBuffer, nCharRangeEnd);
            aBuffer.append("> "_ostr + OString::number(nGlyphRangeStart));
            OString cidrangeline = aBuffer.toString();
            cidranges.push_back(cidrangeline);
            rSubSetInfo.aComponents.back().glyphRangesToChars[nGlyphRangeStart]
                = Range(nCharRangeStart, nCharRangeEnd);
        }

        if (!cidranges.empty())
        {
            // searching for real world examples where this occurs
            OString beginline = OString::number(cidranges.size()) + " begincidrange";
            CMap.WriteLine(beginline);
            for (const auto& rLine : cidranges)
                CMap.WriteLine(rLine);
            CMap.WriteLine("endcidrange");
        }
    }

    if (!tud.bfcharlines.empty())
    {
        OString beginline = OString::number(tud.bfcharlines.size()) + " begincidchar";
        CMap.WriteLine(beginline);
        for (const auto& charline : tud.bfcharlines)
        {
            assert(charline[0] == '<');
            sal_Int32 nEnd = charline.indexOf('>', 1);
            assert(charline[nEnd] == '>');
            sal_Int32 nPDFCharCode = o3tl::toInt32(charline.subView(1, nEnd - 1), 16);
            sal_Int32 nGlyphIndex = tud.getGlyphIndexFromCharCode(nPDFCharCode);
            OString sChars(o3tl::trim(charline.subView(nEnd + 1)));
            assert(sChars[0] == '<' && sChars[sChars.getLength() - 1] == '>');
            OString sContents = sChars.copy(1, sChars.getLength() - 2);
            assert(sContents.getLength() % 4 == 0);
            sal_Int32 nCharsPerCode = sContents.getLength() / 4;
            if (nCharsPerCode > 1)
                ligatureGlyphToChars[nGlyphIndex] = sContents;
            else
            {
                OString sLegacy
                    = decomposeLegacyUnicodeLigature(static_cast<UChar32>(sContents.toUInt32(16)));
                if (!sLegacy.isEmpty())
                    ligatureGlyphToChars[nGlyphIndex] = sLegacy;
            }
            OString cidcharline = sChars + " " + OString::number(nGlyphIndex);
            glyphs.push_back(nGlyphIndex);
            rSubSetInfo.aComponents.back().glyphToChars[nGlyphIndex] = sContents;
            rSubSetInfo.aComponents.back().charsToGlyph[sContents] = nGlyphIndex;
            CMap.WriteLine(cidcharline);
        }
        CMap.WriteLine("endcidchar");

        rSubSetInfo.aComponents.back().nGlyphCount = tud.bfcharlines.size();
    }

    CMap.WriteBytes(cmapsuffix, std::size(cmapsuffix) - 1);

    const OUString& toMergedMapUrl = rSubSetInfo.aComponents.back().toMergedMapUrl;
    SvFileStream toMergedMap(toMergedMapUrl, StreamMode::READWRITE | StreamMode::TRUNC);
    toMergedMap.WriteLine(Concat2View("mergefonts "_ostr + FontName + " 0"_ostr));
    if (rSubSetInfo.aComponents.size() == 1)
        toMergedMap.WriteLine("0\t0");
    for (size_t i = 0; i < glyphs.size(); ++i)
    {
        OString sMapLine = OString::number(i + mergeOffset) + "\t" + OString::number(glyphs[i]);
        toMergedMap.WriteLine(sMapLine);
    }
    toMergedMap.Close();

    CMap.Close();

    if (!ligatureGlyphToChars.empty())
    {
        std::map<OString, sal_Int32>& charsToGlyph = rSubSetInfo.aComponents.back().charsToGlyph;

        Features.WriteLine("languagesystem DFLT dflt;");
        Features.WriteLine("feature liga {");
        for (const auto& ligature : ligatureGlyphToChars)
        {
            sal_Int32 nLigatureGlyph = ligature.first;
            OString sLigatureChars = ligature.second;
            OStringBuffer ligatureLine("substitute");
            for (sal_Int32 i = 0; i < sLigatureChars.getLength(); i += 4)
            {
                OString sLigatureChar = sLigatureChars.copy(i, 4);
                sal_Int32 nCharGlyph = charsToGlyph[sLigatureChar];
                ligatureLine.append(" \\" + OString::number(nCharGlyph));
            }
            ligatureLine.append(" by \\" + OString::number(nLigatureGlyph) + ";");
            Features.WriteLine(ligatureLine);
        }
        Features.WriteLine("} liga;");
    }
}

static OUString buildFontMenuName(const OUString& FontMenuNameDBUrl,
                                  std::u16string_view postScriptName, const OUString& fontName,
                                  std::string_view Weight)
{
    OUString longFontName = fontName;

    // create FontMenuName
    SvFileStream FontMenuNameDB(FontMenuNameDBUrl, StreamMode::READWRITE | StreamMode::TRUNC);
    OUString postScriptFontName = u"["_ustr + postScriptName + u"]"_ustr;
    FontMenuNameDB.WriteByteStringLine(postScriptFontName, RTL_TEXTENCODING_UTF8);
    SAL_INFO("sd.filter",
             "wrote basefont name: " << postScriptFontName << " to: " << FontMenuNameDBUrl);
    OUString setFontName = "f=" + fontName;
    FontMenuNameDB.WriteByteStringLine(setFontName, RTL_TEXTENCODING_UTF8);
    SAL_INFO("sd.filter", "wrote family name: " << setFontName << " to: " << FontMenuNameDBUrl);
    if (!isSimpleFamilyName(Weight))
    {
        longFontName = fontName + " " + OUString::createFromAscii(Weight);
        OUString setLongFontName = "l=" + fontName + " " + OUString::createFromAscii(Weight);
        FontMenuNameDB.WriteByteStringLine(setLongFontName, RTL_TEXTENCODING_UTF8);
        SAL_INFO("sd.filter",
                 "wrote long family name: " << setLongFontName << " to: " << FontMenuNameDBUrl);
        OUString styleName = "s=" + OUString::createFromAscii(Weight);
        FontMenuNameDB.WriteByteStringLine(styleName, RTL_TEXTENCODING_UTF8);
        SAL_INFO("sd.filter", "wrote style name: " << styleName << " to: " << FontMenuNameDBUrl);
    }
    FontMenuNameDB.Close();

    return longFontName;
}

// https://adobe-type-tools.github.io/font-tech-notes/pdfs/5900.RFMFAH_Tutorial.pdf
static EmbeddedFontInfo mergeFontSubsets(sal_Int64 prefix, const OUString& mergedFontUrl,
                                         const OUString& FontMenuNameDBUrl,
                                         const OUString& postScriptName,
                                         const OUString& longFontName, std::string_view Weight,
                                         const SubSetInfo& rSubSetInfo)
{
    SAL_INFO("sd.filter", "merging " << rSubSetInfo.aComponents.size() << " font subsets of "
                                     << postScriptName << " together to create: " << mergedFontUrl);
    std::vector<std::pair<OUString, OUString>> fonts;
    for (size_t i = 0; i < rSubSetInfo.aComponents.size(); ++i)
    {
        // Ignore subsets with no glyphs in them, except for the first one
        // which can have notdef in it
        if (i && !rSubSetInfo.aComponents[i].nGlyphCount)
            continue;
        fonts.push_back(std::make_pair(rSubSetInfo.aComponents[i].toMergedMapUrl,
                                       rSubSetInfo.aComponents[i].pfaCIDUrl));
    }
    if (!EmbeddedFontsManager::mergefonts(rSubSetInfo.aComponents[0].cidFontInfoUrl, mergedFontUrl,
                                          fonts))
    {
        SAL_WARN("sd.filter", "conversion failed");
        return EmbeddedFontInfo();
    }

    OUString mergedCMapUrl = mergedFontUrl + u".CMap";
    OUString mergedFeaturesUrl = mergedFontUrl + u".Features";

    SvFileStream mergedCMap(mergedCMapUrl, StreamMode::READWRITE | StreamMode::TRUNC);

    mergedCMap.WriteBytes(cmapprefix, std::size(cmapprefix) - 1);

    sal_Int32 cidcharcount(0), cidrangecount(0);
    bool differentcidranges(false);
    for (const auto& component : rSubSetInfo.aComponents)
        cidcharcount += component.glyphToChars.size();

    for (size_t i = 0, size = rSubSetInfo.aComponents.size(); i < size; ++i)
    {
        const auto& component = rSubSetInfo.aComponents[i];
        if (i == 0)
        {
            cidrangecount += component.glyphRangesToChars.size();
            continue;
        }

        if (component.glyphRangesToChars != rSubSetInfo.aComponents[i - 1].glyphRangesToChars)
        {
            cidrangecount += component.glyphRangesToChars.size();
            differentcidranges = true;
        }
    }

    assert(!differentcidranges && "TODO: deal with this when an example arises");
    if (differentcidranges)
        return EmbeddedFontInfo();

    std::map<sal_Int32, OString> ligatureGlyphToChars;
    std::map<OString, sal_Int32> charsToGlyph;

    if (cidrangecount)
    {
        OString beginline = OString::number(cidrangecount) + " begincidrange";
        mergedCMap.WriteLine(beginline);

        const auto& component = rSubSetInfo.aComponents[0];
        for (const auto& entry : component.glyphRangesToChars)
        {
            sal_Int32 glyphrangebegin = entry.first;
            sal_Int32 charrangebegin = entry.second.Min();
            sal_Int32 charrangeend = entry.second.Max();

            OStringBuffer aBuffer("<");
            appendFourByteHex(aBuffer, charrangebegin);
            aBuffer.append("> <");
            appendFourByteHex(aBuffer, charrangeend);
            aBuffer.append("> "_ostr + OString::number(glyphrangebegin));
            OString cidrangeline = aBuffer.toString();

            mergedCMap.WriteLine(cidrangeline);
        }

        // glyphoffset += component.nGlyphCount;
        mergedCMap.WriteLine("endcidrange");
    }

    if (cidcharcount)
    {
        sal_Int32 glyphoffset(0);
        OString beginline = OString::number(cidcharcount) + " begincidchar";
        mergedCMap.WriteLine(beginline);

        for (const auto& component : rSubSetInfo.aComponents)
        {
            for (const auto& entry : component.glyphToChars)
            {
                sal_Int32 glyph = entry.first + glyphoffset;
                OString sCharContents = entry.second;
                OString cidcharline = "<" + sCharContents + "> " + OString::number(glyph);

                sal_Int32 nCharsPerCode = sCharContents.getLength() / 4;
                if (nCharsPerCode > 1)
                    ligatureGlyphToChars[glyph] = sCharContents;
                else
                {
                    OString sLegacy = decomposeLegacyUnicodeLigature(
                        static_cast<UChar32>(sCharContents.toUInt32(16)));
                    if (!sLegacy.isEmpty())
                        ligatureGlyphToChars[glyph] = sLegacy;
                }

                charsToGlyph[entry.second] = glyph;
                mergedCMap.WriteLine(cidcharline);
            }
            glyphoffset += component.nGlyphCount;
        }
        mergedCMap.WriteLine("endcidchar");
    }

    mergedCMap.WriteBytes(cmapsuffix, std::size(cmapsuffix) - 1);

    mergedCMap.Close();

    if (!ligatureGlyphToChars.empty())
    {
        SvFileStream Features(mergedFeaturesUrl, StreamMode::READWRITE | StreamMode::TRUNC);
        Features.WriteLine("languagesystem DFLT dflt;");
        Features.WriteLine("feature liga {");
        for (const auto& ligature : ligatureGlyphToChars)
        {
            sal_Int32 nLigatureGlyph = ligature.first;
            OString sLigatureChars = ligature.second;
            OStringBuffer ligatureLine("substitute");
            for (sal_Int32 i = 0; i < sLigatureChars.getLength(); i += 4)
            {
                OString sLigatureChar = sLigatureChars.copy(i, 4);
                sal_Int32 nCharGlyph = charsToGlyph[sLigatureChar];
                ligatureLine.append(" \\" + OString::number(nCharGlyph));
            }
            ligatureLine.append(" by \\" + OString::number(nLigatureGlyph) + ";");
            Features.WriteLine(ligatureLine);
        }
        Features.WriteLine("} liga;");
        Features.Close();
    }

    OUString otfUrl = getFileUrlForTemporaryFont(prefix, postScriptName, u".otf");
    OUString features = !ligatureGlyphToChars.empty() ? mergedFeaturesUrl : OUString();
    if (EmbeddedFontsManager::makeotf(mergedFontUrl, otfUrl, FontMenuNameDBUrl, mergedCMapUrl,
                                      features))
        return { longFontName, otfUrl, toOfficeWeight(Weight) };
    SAL_WARN("sd.filter", "conversion failed");
    return EmbeddedFontInfo();
}

//static
EmbeddedFontInfo ImpSdrPdfImport::convertToOTF(sal_Int64 prefix, SubSetInfo& rSubSetInfo,
                                               const OUString& fileUrl, const OUString& fontName,
                                               const OUString& postScriptName,
                                               std::u16string_view fontFileName,
                                               const std::vector<uint8_t>& toUnicodeData,
                                               const vcl::pdf::PDFiumFont& font)
{
    // Convert to Type 1 CID keyed
    std::map<int, int> nameIndexToGlyph;
    bool bNameKeyed = false;
    OString FontName, Weight, FSType;
    if (!toPfaCID(rSubSetInfo, fileUrl, postScriptName, bNameKeyed, nameIndexToGlyph, FontName,
                  Weight, FSType))
    {
        return EmbeddedFontInfo();
    }

    OUString FeaturesUrl = fileUrl + u".Features";
    SvFileStream Features(FeaturesUrl, StreamMode::READWRITE | StreamMode::TRUNC);

    const OUString& pfaCIDUrl = rSubSetInfo.aComponents.back().pfaCIDUrl;

    // Build CMap from pdfium toUnicodeData, etc.
    OUString CMapUrl = fileUrl + u".CMap";
    bool bCMap = true;
    if (!toUnicodeData.empty())
    {
        ToUnicodeData tud(toUnicodeData, font);
        buildCMapAndFeatures(CMapUrl, Features, FontName, tud, rSubSetInfo);
    }
    else if (bNameKeyed)
    {
        SAL_INFO("sd.filter", "There is no CMap, assuming Adobe Standard encoding.");
        ToUnicodeData tud(nameIndexToGlyph);
        buildCMapAndFeatures(CMapUrl, Features, FontName, tud, rSubSetInfo);
    }
    else
    {
        SAL_WARN("sd.filter", "There is no CMap, pdfium is missing unicodedata");
        bCMap = false;
    }

    Features.WriteLine("table OS/2 {");
    Features.WriteLine(Concat2View("  FSType " + FSType + ";"));
    Features.WriteLine("} OS/2;");

    Features.Close();

    // Create FontMenuName
    OUString FontMenuNameDBUrl = fileUrl + u".FontMenuNameDBUrl";
    OUString longFontName = buildFontMenuName(FontMenuNameDBUrl, postScriptName, fontName, Weight);

    // Merge multiple font subsets together
    if (rSubSetInfo.aComponents.size() > 1)
    {
        OUString mergedFontUrl
            = getFileUrlForTemporaryFont(prefix, postScriptName, u".merged.pfa.cid");
        return mergeFontSubsets(prefix, mergedFontUrl, FontMenuNameDBUrl, postScriptName,
                                longFontName, Weight, rSubSetInfo);
    }

    // Otherwise not merged font, just a single subset
    OUString otfUrl = getFileUrlForTemporaryFont(prefix, fontFileName, u".otf");
    OUString cmap = bCMap ? CMapUrl : OUString();
    if (EmbeddedFontsManager::makeotf(pfaCIDUrl, otfUrl, FontMenuNameDBUrl, cmap, FeaturesUrl))
        return { longFontName, otfUrl, toOfficeWeight(Weight) };
    SAL_WARN("sd.filter", "conversion failed");
    return EmbeddedFontInfo();
}

// There isn't, as far as I know, a way to stroke with a pattern at the moment,
// so extract some sensible color if this is a stroke pattern
Color ImpSdrPdfImport::getStrokeColor(
    std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
    std::unique_ptr<vcl::pdf::PDFiumPage> const& pPage)
{
    if (std::unique_ptr<vcl::pdf::PDFiumBitmap> bitmap
        = pPageObject->getRenderedStrokePattern(*mpPdfDocument, *pPage))
    {
        Bitmap aBitmap(bitmap->createBitmapFromBuffer());
        return aBitmap.GetPixelColor(aBitmap.GetSizePixel().Width() / 2,
                                     aBitmap.GetSizePixel().Height() / 2);
    }
    return pPageObject->getStrokeColor();
}

// Typically for a fill pattern you want to use some pattern fill equivalent
// but if that's not possible then this fallback can be useful
Color ImpSdrPdfImport::getFillColor(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                                    std::unique_ptr<vcl::pdf::PDFiumPage> const& pPage)
{
    if (std::unique_ptr<vcl::pdf::PDFiumBitmap> bitmap
        = pPageObject->getRenderedFillPattern(*mpPdfDocument, *pPage))
    {
        Bitmap aBitmap(bitmap->createBitmapFromBuffer());
        return aBitmap.GetPixelColor(aBitmap.GetSizePixel().Width() / 2,
                                     aBitmap.GetSizePixel().Height() / 2);
    }
    return pPageObject->getFillColor();
}

static bool AllowDim(tools::Long nDim)
{
    if (nDim > 0x20000000 || nDim < -0x20000000)
    {
        SAL_WARN("sd.filter", "skipping huge dimension: " << nDim);
        return false;
    }
    return true;
}

static bool AllowPoint(const Point& rPoint) { return AllowDim(rPoint.X()) && AllowDim(rPoint.Y()); }

static bool AllowRect(const tools::Rectangle& rRect)
{
    return AllowPoint(rRect.TopLeft()) && AllowPoint(rRect.BottomRight());
}

void ImpSdrPdfImport::ImportText(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                                 std::unique_ptr<vcl::pdf::PDFiumPage> const& pPage,
                                 std::unique_ptr<vcl::pdf::PDFiumTextPage> const& pTextPage,
                                 int /*nPageObjectIndex*/)
{
    basegfx::B2DRectangle aTextRect = pPageObject->getBounds();
    basegfx::B2DHomMatrix aMatrix = pPageObject->getMatrix();

    basegfx::B2DHomMatrix aTextMatrix(maCurrentMatrix);

    aTextRect *= aTextMatrix;

    if (!std::isfinite(aTextRect.getMinX()) || !std::isfinite(aTextRect.getMaxX())
        || !std::isfinite(aTextRect.getMinY()) || !std::isfinite(aTextRect.getMaxY()))
    {
        SAL_WARN("sd.filter", "unusable text rectangle: " << aTextRect);
        return;
    }

    const tools::Rectangle aRect = PointsToLogic(aTextRect.getMinX(), aTextRect.getMaxX(),
                                                 aTextRect.getMinY(), aTextRect.getMaxY());
    if (!AllowRect(aRect))
        return;

    OUString sText = pPageObject->getText(pTextPage);

    OUString sFontName;
    FontWeight eFontWeight(WEIGHT_DONTKNOW);
    auto xFont = pPageObject->getFont();
    auto itImportedFont
        = xFont ? mxImportedFonts->find(xFont->getFontDictObjNum()) : mxImportedFonts->end();
    if (itImportedFont != mxImportedFonts->end())
    {
        // We expand a name like "Foo" with non-traditional styles like
        // "SemiBold" to "Foo SemiBold";
        sFontName = itImportedFont->sFontName;
        eFontWeight = itImportedFont->eFontWeight;
    }
    else
    {
        sFontName = pPageObject->getFontName();
        SAL_WARN("sd.filter", "font: " << sFontName << " wasn't collected");
    }

    const double dFontSize = pPageObject->getFontSize();
    double dFontSizeH = fabs(std::hypot(aMatrix.a(), aMatrix.c()) * dFontSize);
    double dFontSizeV = fabs(std::hypot(aMatrix.b(), aMatrix.d()) * dFontSize);

    // We will only really be able to squeeze a font size in whole units of
    // twips through the various layers esp. export and reimport, so work in
    // twips here and LogicToLogic so we don't end up using a value that cannot
    // be roundtripped back.
    const Size aFontSizeTwips(dFontSizeH * 20, dFontSizeV * 20);
    const Size aFontSize(OutputDevice::LogicToLogic(aFontSizeTwips, MapMode(MapUnit::MapTwip),
                                                    MapMode(MapUnit::Map100thMM)));
    vcl::Font aFnt = mpVD->GetFont();
    aFnt.SetFontSize(aFontSize);

    if (!sFontName.isEmpty())
        aFnt.SetFamilyName(sFontName);

    int italicAngle = pPageObject->getFontAngle();
    {
        // Decompose matrix to inspect shear
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;
        aMatrix.decompose(aScale, aTranslate, fRotate, fShearX);
        int nTextRotation = basegfx::fround(basegfx::rad2deg<1>(atan(fShearX)));
        // Add this additional shear to the reported italic angle, where
        // rotation to the right is negative
        italicAngle += -nTextRotation;
    }
    aFnt.SetItalic(italicAngle == 0 ? ITALIC_NONE
                                    : (italicAngle < 0 ? ITALIC_NORMAL : ITALIC_OBLIQUE));
    aFnt.SetWeight(eFontWeight);

    if (aFnt != mpVD->GetFont())
    {
        mpVD->SetFont(aFnt);
        mbFntDirty = true;
    }

    Color aTextColor(COL_TRANSPARENT);
    bool bFill = false;
    bool bUse = true;
    bool bInvisible = false;
    switch (pPageObject->getTextRenderMode())
    {
        case vcl::pdf::PDFTextRenderMode::Fill:
        case vcl::pdf::PDFTextRenderMode::FillClip:
        case vcl::pdf::PDFTextRenderMode::FillStroke:
        case vcl::pdf::PDFTextRenderMode::FillStrokeClip:
            bFill = true;
            break;
        case vcl::pdf::PDFTextRenderMode::Stroke:
        case vcl::pdf::PDFTextRenderMode::StrokeClip:
        case vcl::pdf::PDFTextRenderMode::Unknown:
            break;
        case vcl::pdf::PDFTextRenderMode::Invisible:
            bInvisible = true;
            bUse = false;
            break;
        case vcl::pdf::PDFTextRenderMode::Clip:
            bUse = false;
            break;
    }
    if (bUse)
    {
        Color aColor
            = bFill ? getFillColor(pPageObject, pPage) : getStrokeColor(pPageObject, pPage);
        if (aColor != COL_TRANSPARENT)
            aTextColor = aColor.GetRGBColor();
    }

    if (aTextColor != mpVD->GetTextColor())
    {
        mpVD->SetTextColor(aTextColor);
        mbFntDirty = true;
    }

    InsertTextObject(aRect.TopLeft(), aRect.GetSize(), sText, bInvisible);
}

void ImpSdrPdfImport::InsertTextObject(const Point& rPos, const Size& /*rSize*/,
                                       const OUString& rStr, bool bInvisible)
{
    FontMetric aFontMetric(mpVD->GetFontMetric());
    vcl::Font aFont(mpVD->GetFont());
    assert(aFont.GetAlignment() == ALIGN_BASELINE);

    /* Get our text bounds of this text, which is nominally relative to a 0
       left margin, so we know where the inked rect begins relative to the
       start of the text area, and we can adjust the pdf rectangle by that so
       the SdrRectObj text will get rendered at the same x position.

       Similarly find the relative vertical distance from the inked bounds
       to the baseline and adjust the pdf rect so the SdrRectObj will render
       the text at the same y position.
    */
    tools::Rectangle aOurRect;
    (void)mpVD->GetTextBoundRect(aOurRect, rStr);

    auto nXDiff = aOurRect.Left();
    auto nYDiff = aFontMetric.GetDescent() - aOurRect.Bottom();

    Point aPos(rPos.X() - nXDiff, rPos.Y() + nYDiff);
    // As per ImpEditEngine::CalcParaWidth the width of the text box has to be 1 unit wider than the text
    auto nTextWidth = mpVD->GetTextWidth(rStr) + 1;
    Size aSize(nTextWidth, -aFontMetric.GetLineHeight());

    Point aSdrPos(basegfx::fround<tools::Long>(aPos.X() * mfScaleX + maOfs.X()),
                  basegfx::fround<tools::Long>(aPos.Y() * mfScaleY + maOfs.Y()));
    Size aSdrSize(basegfx::fround<tools::Long>(aSize.Width() * mfScaleX),
                  basegfx::fround<tools::Long>(aSize.Height() * mfScaleY));

    tools::Rectangle aTextRect(aSdrPos, aSdrSize);
    rtl::Reference<SdrRectObj> pText = new SdrRectObj(*mpModel, aTextRect, SdrObjKind::Text);

    pText->SetMergedItem(makeSdrTextUpperDistItem(0));
    pText->SetMergedItem(makeSdrTextLowerDistItem(0));
    pText->SetMergedItem(makeSdrTextRightDistItem(0));
    pText->SetMergedItem(makeSdrTextLeftDistItem(0));

    pText->SetMergedItem(makeSdrTextAutoGrowHeightItem(false));

    if (bInvisible)
        pText->SetVisible(false);

    pText->SetLayer(mnLayer);
    pText->NbcSetText(rStr);
    SetAttributes(pText.get(), true);
    pText->SetSnapRect(aTextRect);

    if (!aFont.IsTransparent())
    {
        SfxItemSet aAttr(SfxItemSet::makeFixedSfxItemSet<XATTR_FILL_FIRST, XATTR_FILL_LAST>(
            *mpFillAttr->GetPool()));
        aAttr.Put(XFillStyleItem(drawing::FillStyle_SOLID));
        aAttr.Put(XFillColorItem(OUString(), aFont.GetFillColor()));
        pText->SetMergedItemSet(aAttr);
    }
    Degree100 nAngle = to<Degree100>(aFont.GetOrientation());
    if (nAngle)
        pText->SdrAttrObj::NbcRotate(aSdrPos, nAngle);
    InsertObj(pText.get(), false);
}

void ImpSdrPdfImport::MapScaling()
{
    const size_t nCount(maTmpList.size());
    const MapMode& rMap = mpVD->GetMapMode();
    Point aMapOrg(rMap.GetOrigin());
    bool bMov2(aMapOrg.X() != 0 || aMapOrg.Y() != 0);

    if (bMov2)
    {
        for (size_t i = mnMapScalingOfs; i < nCount; i++)
        {
            SdrObject* pObj = maTmpList[i].get();

            pObj->NbcMove(Size(aMapOrg.X(), aMapOrg.Y()));
        }
    }

    mnMapScalingOfs = nCount;
}

basegfx::B2DPolyPolygon
ImpSdrPdfImport::GetClip(const std::unique_ptr<vcl::pdf::PDFiumClipPath>& pClipPath,
                         const basegfx::B2DHomMatrix& rPathMatrix,
                         const basegfx::B2DHomMatrix& rTransform)
{
    basegfx::B2DPolyPolygon aClipPolyPoly;

    int nClipPathCount = pClipPath->getPathCount();
    for (int i = 0; i < nClipPathCount; ++i)
    {
        std::vector<std::unique_ptr<vcl::pdf::PDFiumPathSegment>> aPathSegments;
        const int nSegments = pClipPath->getPathSegmentCount(i);
        for (int nSegmentIndex = 0; nSegmentIndex < nSegments; ++nSegmentIndex)
            aPathSegments.push_back(pClipPath->getPathSegment(i, nSegmentIndex));

        appendSegmentsToPolyPoly(aClipPolyPoly, aPathSegments, rPathMatrix);
    }

    aClipPolyPoly.transform(rTransform);

    return aClipPolyPoly;
}

void ImpSdrPdfImport::ImportImage(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                                  int /*nPageObjectIndex*/)
{
    std::unique_ptr<vcl::pdf::PDFiumBitmap> bitmap = pPageObject->getImageBitmap();
    if (!bitmap)
    {
        SAL_WARN("sd.filter", "Failed to get IMAGE");
        return;
    }

    const vcl::pdf::PDFBitmapType format = bitmap->getFormat();
    if (format == vcl::pdf::PDFBitmapType::Unknown)
    {
        SAL_WARN("sd.filter", "Failed to get IMAGE format");
        return;
    }

    basegfx::B2DRectangle aBounds = pPageObject->getBounds();
    float left = aBounds.getMinX();
    // Upside down.
    float bottom = aBounds.getMinY();
    float right = aBounds.getMaxX();
    // Upside down.
    float top = aBounds.getMaxY();

    bool bEntirelyClippedOut = false;

    auto aPathMatrix = pPageObject->getMatrix();

    aPathMatrix *= maCurrentMatrix;

    const basegfx::B2DHomMatrix aTransform(
        basegfx::utils::createScaleTranslateB2DHomMatrix(mfScaleX, mfScaleY, maOfs.X(), maOfs.Y()));

    basegfx::B2DPolyPolygon aClipPolyPoly
        = GetClip(pPageObject->getClipPath(), aPathMatrix, aTransform);

    if (aClipPolyPoly.count())
    {
        // Clip against ClipRegion, use same conversions and transformation on
        // the graphic bounds as were applied to the clipping polypolygon
        const tools::Rectangle aRect = PointsToLogic(aBounds.getMinX(), aBounds.getMaxX(),
                                                     aBounds.getMinY(), aBounds.getMaxY());
        basegfx::B2DPolyPolygon aGraphicBounds(
            basegfx::utils::createPolygonFromRect(vcl::unotools::b2DRectangleFromRectangle(aRect)));
        aGraphicBounds.transform(aTransform);
        const basegfx::B2DPolyPolygon aClippedBounds(basegfx::utils::clipPolyPolygonOnPolyPolygon(
            aGraphicBounds, aClipPolyPoly, true, false));
        // completely clipped out
        bEntirelyClippedOut = aClippedBounds.getB2DRange().isEmpty();
    }

    tools::Rectangle aRect = PointsToLogic(left, right, top, bottom);
    aRect.AdjustRight(1);
    aRect.AdjustBottom(1);

    Bitmap aBitmap = bitmap->createBitmapFromBuffer();
    rtl::Reference<SdrGrafObj> pGraf = new SdrGrafObj(*mpModel, Graphic(aBitmap), aRect);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
    pGraf->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));

    if (bEntirelyClippedOut)
        pGraf->SetVisible(false);

    InsertObj(pGraf.get());
}

void ImpSdrPdfImport::appendSegmentsToPolyPoly(
    basegfx::B2DPolyPolygon& rPolyPoly,
    const std::vector<std::unique_ptr<vcl::pdf::PDFiumPathSegment>>& rPathSegments,
    const basegfx::B2DHomMatrix& rPathMatrix)
{
    basegfx::B2DPolygon aPoly;
    std::vector<basegfx::B2DPoint> aBezier;

    for (const auto& pPathSegment : rPathSegments)
    {
        if (pPathSegment != nullptr)
        {
            basegfx::B2DPoint aB2DPoint = pPathSegment->getPoint();
            aB2DPoint *= rPathMatrix;

            const bool bClose = pPathSegment->isClosed();
            if (bClose)
                aPoly.setClosed(bClose); // TODO: Review

            Point aPoint = PointsToLogic(aB2DPoint.getX(), aB2DPoint.getY());
            aB2DPoint.setX(aPoint.X());
            aB2DPoint.setY(aPoint.Y());

            const vcl::pdf::PDFSegmentType eSegmentType = pPathSegment->getType();
            switch (eSegmentType)
            {
                case vcl::pdf::PDFSegmentType::Lineto:
                    aPoly.append(aB2DPoint);
                    break;

                case vcl::pdf::PDFSegmentType::Bezierto:
                    aBezier.emplace_back(aB2DPoint.getX(), aB2DPoint.getY());
                    if (aBezier.size() == 3)
                    {
                        aPoly.appendBezierSegment(aBezier[0], aBezier[1], aBezier[2]);
                        aBezier.clear();
                    }
                    break;

                case vcl::pdf::PDFSegmentType::Moveto:
                    // New Poly.
                    if (aPoly.count() > 0)
                    {
                        rPolyPoly.append(aPoly, 1);
                        aPoly.clear();
                    }

                    aPoly.append(aB2DPoint);
                    break;

                case vcl::pdf::PDFSegmentType::Unknown:
                default:
                    SAL_WARN("sd.filter", "Unknown path segment type in PDF: "
                                              << static_cast<int>(eSegmentType));
                    break;
            }
        }
    }

    if (aBezier.size() == 3)
    {
        aPoly.appendBezierSegment(aBezier[0], aBezier[1], aBezier[2]);
        aBezier.clear();
    }

    if (aPoly.count() > 0)
    {
        rPolyPoly.append(aPoly, 1);
        aPoly.clear();
    }
}

void ImpSdrPdfImport::ImportPath(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                                 std::unique_ptr<vcl::pdf::PDFiumPage> const& pPage,
                                 int /*nPageObjectIndex*/)
{
    auto aPathMatrix = pPageObject->getMatrix();

    aPathMatrix *= maCurrentMatrix;

    basegfx::B2DPolyPolygon aPolyPoly;

    std::vector<std::unique_ptr<vcl::pdf::PDFiumPathSegment>> aPathSegments;
    const int nSegments = pPageObject->getPathSegmentCount();
    for (int nSegmentIndex = 0; nSegmentIndex < nSegments; ++nSegmentIndex)
    {
        aPathSegments.push_back(pPageObject->getPathSegment(nSegmentIndex));
    }

    appendSegmentsToPolyPoly(aPolyPoly, aPathSegments, aPathMatrix);

    const basegfx::B2DHomMatrix aTransform(
        basegfx::utils::createScaleTranslateB2DHomMatrix(mfScaleX, mfScaleY, maOfs.X(), maOfs.Y()));
    aPolyPoly.transform(aTransform);

    float fWidth = pPageObject->getStrokeWidth();
    const double dWidth = 0.5 * fabs(std::hypot(aPathMatrix.a(), aPathMatrix.c()) * fWidth);
    mnLineWidth = convertPointToMm100(dWidth);

    vcl::pdf::PDFFillMode nFillMode = vcl::pdf::PDFFillMode::Alternate;
    bool bStroke = true; // Assume we have to draw, unless told otherwise.
    if (pPageObject->getDrawMode(nFillMode, bStroke))
    {
        if (nFillMode == vcl::pdf::PDFFillMode::Alternate)
            mpVD->SetDrawMode(DrawModeFlags::Default);
        else if (nFillMode == vcl::pdf::PDFFillMode::Winding)
            mpVD->SetDrawMode(DrawModeFlags::Default);
        else
            mpVD->SetDrawMode(DrawModeFlags::NoFill);
    }

    if (std::unique_ptr<vcl::pdf::PDFiumBitmap> bitmap
        = pPageObject->getRenderedFillPattern(*mpPdfDocument, *pPage))
    {
        moFillPattern = bitmap->createBitmapFromBuffer();
        moFillColor.reset();
    }
    else
    {
        moFillColor = pPageObject->getFillColor();
        moFillPattern.reset();
    }

    if (bStroke)
        mpVD->SetLineColor(getStrokeColor(pPageObject, pPage));
    else
        mpVD->SetLineColor(COL_TRANSPARENT);

    if (!mbLastObjWasPolyWithoutLine || !CheckLastPolyLineAndFillMerge(aPolyPoly))
    {
        rtl::Reference<SdrPathObj> pPath
            = new SdrPathObj(*mpModel, SdrObjKind::Polygon, std::move(aPolyPoly));
        SetAttributes(pPath.get());
        InsertObj(pPath.get(), false);
    }
}

Point ImpSdrPdfImport::PointsToLogic(double x, double y) const
{
    y = correctVertOrigin(y);

    Point aPos(convertPointToMm100(x), convertPointToMm100(y));
    return aPos;
}

tools::Rectangle ImpSdrPdfImport::PointsToLogic(double left, double right, double top,
                                                double bottom) const
{
    top = correctVertOrigin(top);
    bottom = correctVertOrigin(bottom);

    Point aPos(convertPointToMm100(left), convertPointToMm100(top));
    Size aSize(convertPointToMm100(right - left), convertPointToMm100(bottom - top));

    return tools::Rectangle(aPos, aSize);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
