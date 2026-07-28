/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <config_features.h>
#include <config_folders.h>

#include <algorithm>
#include <initializer_list>
#include <map>
#include <optional>
#include <set>
#include <string_view>
#include <utility>
#include <vector>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>

#include <avmedia/mediaplayer.hxx>

#include <basic/sberrors.hxx>
#include <basic/sbstar.hxx>

#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/XSLTFilterDialog.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/scanner/XScannerManager2.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/kit.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertyvalue.hxx>

#include <editeng/contouritem.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/section.hxx>
#include <editeng/editobj.hxx>
#include <editeng/CustomPropertyField.hxx>
#include <editeng/urlfieldhelper.hxx>

#include <sal/log.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/zoomitem.hxx>
#include <sfx2/kit/helper.hxx>
#include <sfx2/kit/unocommandlist.hxx>

#include <svx/compressgraphicdialog.hxx>
#include <svx/ClassificationDialog.hxx>
#include <svx/ClassificationCommon.hxx>
#include <svx/bmpmask.hxx>
#include <svx/extedit.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/f3dchild.hxx>
#include <svx/fontwork.hxx>
#include <svx/fontworkbar.hxx>
#include <svx/graphichelper.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/imapdlg.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdundo.hxx>
#include <svx/svxdlg.hxx>
#include <svx/svxids.hrc>
#include <svx/sdtfsitm.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/zoomslideritem.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/chrtitem.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/drawstyleutils.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <tools/json_writer.hxx>
#include <tools/UnitConversion.hxx>

#include <unotools/useroptions.hxx>

#include <vcl/abstdlg.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp2.hxx>
#include <vcl/weld.hxx>

#include <editeng/cmapitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/numitem.hxx>
#include <svx/svdobj.hxx>
#include <svx/SvxColorChildWindow.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>

#include <svl/poolitem.hxx>
#include <svl/style.hxx>
#include <svl/whiter.hxx>

#include <app.hrc>
#include <strings.hrc>

#include <AnimationChildWindow.hxx>
#include <DesignTemplates.hxx>
#include <DrawDocShell.hxx>
#include <DrawViewShell.hxx>
#include <LayerTabBar.hxx>
#include <Outliner.hxx>
#include <ViewShellHint.hxx>
#include <ViewShellImplementation.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>
#include <SlideSectionManager.hxx>
#include <UndoSlideSection.hxx>
#include <drawview.hxx>
#include <fuarea.hxx>
#include <fubullet.hxx>
#include <fuchar.hxx>
#include <fucushow.hxx>
#include <fuconnct.hxx>
#include <fucopy.hxx>
#include <fudspord.hxx>
#include <fuexecuteinteraction.hxx>
#include <fuexpand.hxx>
#include <fuinsert.hxx>
#include <fuinsfil.hxx>
#include <fuline.hxx>
#include <fulinend.hxx>
#include <fulink.hxx>
#include <fumeasur.hxx>
#include <fumorph.hxx>
#include <fuoaprms.hxx>
#include <fuolbull.hxx>
#include <fupage.hxx>
#include <fuparagr.hxx>
#include <fuprlout.hxx>
#include <fuscale.hxx>
#include <fusel.hxx>
#include <fusldlg.hxx>
#include <fusnapln.hxx>
#include <fusumry.hxx>
#include <futempl.hxx>
#include <futhes.hxx>
#include <futransf.hxx>
#include <futxtatt.hxx>
#include <fuvect.hxx>
#include <futext.hxx>
#include <helpids.h>
#include <sdabstdlg.hxx>
#include <sdattr.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <unokywds.hxx>
#include <slideshow.hxx>
#include <stlsheet.hxx>
#include <undolayer.hxx>
#include <unmodpg.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <sfx2/classificationhelper.hxx>
#include <sdmod.hxx>
#include <model/SlsPageDescriptor.hxx>
#include <model/SlsPageEnumerationProvider.hxx>
#include <SlideSorter.hxx>
#include <view/SlideSorterView.hxx>
#include <SlideSorterViewShell.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsPageSelector.hxx>
#include <controller/SlsClipboard.hxx>
#include <tools/GraphicSizeCheck.hxx>

#include <theme/ThemeColorChanger.hxx>
#include <svx/dialog/ThemeDialog.hxx>
#include <svx/dialog/ThemeColorEditDialog.hxx>
#include <svx/ColorSets.hxx>
#include <COKit/COKit.hxx>

#include <ViewShellBase.hxx>
#include <memory>

#include <sfx2/newstyle.hxx>
#include <SelectLayerDlg.hxx>
#include <unomodel.hxx>

#include <iostream>
#include <boost/property_tree/json_parser.hpp>
#include <rtl/uri.hxx>
#include <editeng/editeng.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace cpo::uno;

#define MIN_ACTIONS_FOR_DIALOG  5000    ///< if there are more meta objects, we show a dialog during the break up

namespace sd {

namespace {

// A design-template name travels from the picker through the chat request into
// the system prompt, so the naming contract is deliberately narrow: at most 64
// characters, each a plain letter, digit, space, hyphen, or underscore. Every
// place a design name crosses the process boundary checks this same contract,
// so a change here is a change of the wire contract, not of one check. A
// template file whose base name does not fit the contract is not offered at
// all - offering it would only invite a pick the server then drops.
bool lcl_IsValidDesignTemplateName(const OUString& rName)
{
    if (rName.isEmpty() || rName.getLength() > 64)
        return false;
    for (sal_Int32 i = 0; i < rName.getLength(); ++i)
    {
        const sal_Unicode cChar = rName[i];
        const bool bAllowed = (cChar >= 'a' && cChar <= 'z') || (cChar >= 'A' && cChar <= 'Z')
                              || (cChar >= '0' && cChar <= '9') || cChar == ' ' || cChar == '-'
                              || cChar == '_';
        if (!bAllowed)
            return false;
    }
    return true;
}

// A slide intent is an opaque word the model sends to pick a master by a
// template's own mapping. The engine never interprets the word, so the only
// rule is a narrow shape: one to thirty-two characters, each a lowercase ASCII
// letter or a hyphen.
bool lcl_IsValidDesignIntentWord(const OUString& rWord)
{
    if (rWord.isEmpty() || rWord.getLength() > 32)
        return false;
    for (sal_Int32 i = 0; i < rWord.getLength(); ++i)
    {
        const sal_Unicode cChar = rWord[i];
        if (!((cChar >= 'a' && cChar <= 'z') || cChar == '-'))
            return false;
    }
    return true;
}

}

std::vector<std::pair<OUString, OUString>> CollectDesignTemplates()
{
    // The bundled set lives in a dedicated subdirectory, deliberately NOT the
    // standard presentation template directory: with an integrator the kit mounts
    // the per-config preset templates over share/template/common/presnt, which
    // would hide bundled templates placed there. The preset directory is listed
    // second so an uploaded template is offered as well, and a bundled template
    // wins when a preset shares its name.
    const OUString aSearchDirs[] = {
        u"$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/cool-ai-templates/"_ustr,
        u"$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/template/common/presnt/"_ustr,
    };

    std::vector<std::pair<OUString, OUString>> aTemplates;
    std::set<OUString> aSeen;
    for (const OUString& rSearchDir : aSearchDirs)
    {
        OUString aDirUrl = rSearchDir;
        rtl::Bootstrap::expandMacros(aDirUrl);
        osl::Directory aDir(aDirUrl);
        if (aDir.open() != osl::FileBase::E_None)
            continue;

        osl::DirectoryItem aItem;
        while (aDir.getNextItem(aItem) == osl::FileBase::E_None)
        {
            osl::FileStatus aStatus(osl_FileStatus_Mask_FileName
                                    | osl_FileStatus_Mask_FileURL);
            if (aItem.getFileStatus(aStatus) != osl::FileBase::E_None)
                continue;

            const OUString aFileName = aStatus.getFileName();
            if (!aFileName.endsWithIgnoreAsciiCase(u".otp"))
                continue;

            const OUString aName = aFileName.copy(0, aFileName.getLength() - 4);
            if (!lcl_IsValidDesignTemplateName(aName))
            {
                KIT_WARN("sd.transform",
                         "Design template skipped, the file name must be at most 64 "
                         "characters of letters, digits, space, hyphen, or underscore: "
                             << aFileName);
                continue;
            }
            // Keep the first template seen for a name, so the bundled set wins.
            if (!aSeen.insert(aName.toAsciiLowerCase()).second)
                continue;
            aTemplates.emplace_back(aName, aStatus.getFileURL());
        }
        aDir.close();
    }
    return aTemplates;
}

DesignMasterRole DesignMasterRoleFromName(std::u16string_view rName)
{
    const OUString aLower = OUString(rName).toAsciiLowerCase();
    auto has = [&aLower](std::u16string_view rKeyword) { return aLower.indexOf(rKeyword) >= 0; };

    // The keyword groups are tested in a fixed order. The divider, closing, and
    // body groups come before "title" because a name often carries "title"
    // alongside a stronger keyword: "Section Title" and "Title & Content Divider"
    // both read as a divider, and a plain "Title" or "Subtitle" falls through to
    // the title group last.
    if (has(u"separator") || has(u"divider") || has(u"section") || has(u"topic")
        || has(u"chapter") || has(u"agenda") || has(u"transition"))
        return DesignMasterRole::Divider;
    if (has(u"ending") || has(u"closing") || has(u"thank") || has(u"farewell")
        || has(u"goodbye") || has(u"finale"))
        return DesignMasterRole::Closing;
    if (has(u"content") || has(u"outline") || has(u"body") || has(u"bullet"))
        return DesignMasterRole::Content;
    if (has(u"title"))
        return DesignMasterRole::Title;
    return DesignMasterRole::Unknown;
}

// The slide-part vocabulary: each part and the name it travels under between
// the model and the engine. The five words are a wire contract, so changing
// one changes the protocol, not just this table.
constexpr std::pair<DesignMasterRole, std::u16string_view> aDesignRoleWireNames[] = {
    { DesignMasterRole::Title, u"opening" },
    { DesignMasterRole::Divider, u"divider" },
    { DesignMasterRole::Content, u"body" },
    { DesignMasterRole::Closing, u"closing" },
};

OUString DesignRoleToWireName(DesignMasterRole eRole)
{
    for (const auto& [ePart, rWireName] : aDesignRoleWireNames)
    {
        if (ePart == eRole)
            return OUString(rWireName);
    }
    return u"other"_ustr;
}

std::optional<DesignMasterRole> WireNameToDesignRole(std::u16string_view rName)
{
    for (const auto& [ePart, rWireName] : aDesignRoleWireNames)
    {
        if (rName == rWireName)
            return ePart;
    }
    return std::nullopt;
}

std::optional<AIDesignManifest> ReadAIDesignManifest(SdDrawDocument& rTemplate)
{
    ::sd::DrawDocShell* pShell = rTemplate.GetDocSh();
    if (!pShell)
        return std::nullopt;

    OUString aJson;
    try
    {
        uno::Reference<document::XDocumentProperties> xDocProps = pShell->getDocProperties();
        if (!xDocProps.is())
            return std::nullopt;
        uno::Reference<beans::XPropertySet> xUserProps(xDocProps->getUserDefinedProperties(),
                                                       uno::UNO_QUERY);
        if (!xUserProps.is())
            return std::nullopt;
        uno::Reference<beans::XPropertySetInfo> xInfo = xUserProps->getPropertySetInfo();
        if (!xInfo.is() || !xInfo->hasPropertyByName(u"AIDesignManifest"_ustr))
            return std::nullopt;
        xUserProps->getPropertyValue(u"AIDesignManifest"_ustr) >>= aJson;
    }
    catch (const cpo::uno::Exception&)
    {
        return std::nullopt;
    }
    if (aJson.isEmpty())
        return std::nullopt;

    boost::property_tree::ptree aTree;
    try
    {
        std::stringstream aStream(std::string(OUStringToOString(aJson, RTL_TEXTENCODING_UTF8)));
        boost::property_tree::read_json(aStream, aTree);
    }
    catch (...)
    {
        KIT_WARN("sd.transform", "AIDesignManifest is not valid JSON, ignoring it.");
        return std::nullopt;
    }

    // A reader of a later schema reads the fields it knows and ignores the rest,
    // so version 1 is the lowest the reader accepts. A missing or older version
    // reads as no manifest and the heuristic runs instead.
    if (aTree.get("schemaVersion", 0) < 1)
    {
        KIT_WARN("sd.transform",
                 "AIDesignManifest schema version is missing or below 1, ignoring it.");
        return std::nullopt;
    }

    // The masters the template actually carries. A manifest entry that names a
    // master the template does not have is dropped.
    std::set<OUString> aMasterNames;
    const sal_uInt16 nMasters = rTemplate.GetMasterSdPageCount(PageKind::Standard);
    for (sal_uInt16 i = 0; i < nMasters; ++i)
    {
        if (SdPage* pMaster = rTemplate.GetMasterSdPage(i, PageKind::Standard))
            aMasterNames.insert(pMaster->GetName());
    }

    AIDesignManifest aManifest;

    if (auto oMasters = aTree.get_child_optional("masters"))
    {
        for (const auto& [rKey, rChild] : *oMasters)
        {
            const OUString aMasterName = OStringToOUString(rKey, RTL_TEXTENCODING_UTF8);
            const OUString aRoleName
                = OStringToOUString(rChild.get_value<std::string>(), RTL_TEXTENCODING_UTF8);
            if (!aMasterNames.contains(aMasterName))
            {
                KIT_WARN("sd.transform", "AIDesignManifest names an unknown master '"
                                             << aMasterName << "', dropping it.");
                continue;
            }
            std::optional<DesignMasterRole> oRole = WireNameToDesignRole(aRoleName);
            if (!oRole)
            {
                KIT_WARN("sd.transform", "AIDesignManifest gives master '"
                                             << aMasterName << "' an unknown part '" << aRoleName
                                             << "', dropping it.");
                continue;
            }
            aManifest.maMasterRoles[aMasterName] = *oRole;
        }
    }

    if (auto oIntents = aTree.get_child_optional("intents"))
    {
        for (const auto& [rKey, rChild] : *oIntents)
        {
            const OUString aIntent = OStringToOUString(rKey, RTL_TEXTENCODING_UTF8);
            const OUString aMasterName
                = OStringToOUString(rChild.get_value<std::string>(), RTL_TEXTENCODING_UTF8);
            if (!lcl_IsValidDesignIntentWord(aIntent))
            {
                KIT_WARN("sd.transform", "AIDesignManifest has a malformed intent word '"
                                             << aIntent << "', dropping it.");
                continue;
            }
            if (!aMasterNames.contains(aMasterName))
            {
                KIT_WARN("sd.transform", "AIDesignManifest maps intent '"
                                             << aIntent << "' to an unknown master '"
                                             << aMasterName << "', dropping it.");
                continue;
            }
            aManifest.maIntentMasters[aIntent] = aMasterName;
        }
    }

    aManifest.maArtDirection
        = OStringToOUString(aTree.get("artDirection", std::string()), RTL_TEXTENCODING_UTF8);

    if (auto oBudgets = aTree.get_child_optional("budgets"))
    {
        // Only a budget of one or more is kept.
        auto readBudget = [&oBudgets](const char* pKey) -> std::optional<sal_Int32> {
            if (auto oValue = oBudgets->get_optional<sal_Int32>(pKey))
                if (*oValue >= 1)
                    return *oValue;
            return std::nullopt;
        };
        aManifest.moMaxSlides = readBudget("maxSlides");
        aManifest.moMaxItemsPerBullets = readBudget("maxItemsPerBullets");
        aManifest.moMaxItemLength = readBudget("maxItemLength");
        aManifest.moMaxTitleLength = readBudget("maxTitleLength");
    }

    return aManifest;
}

std::vector<DesignTemplateMaster> CollectDesignTemplateMasters(SdDrawDocument& rTemplate)
{
    const std::optional<AIDesignManifest> oManifest = ReadAIDesignManifest(rTemplate);

    std::vector<DesignTemplateMaster> aMasters;
    const sal_uInt16 nMasters = rTemplate.GetMasterSdPageCount(PageKind::Standard);
    std::map<OUString, std::size_t> aIndex;
    for (sal_uInt16 i = 0; i < nMasters; ++i)
    {
        const OUString aName = rTemplate.GetMasterSdPage(i, PageKind::Standard)->GetName();
        aIndex.emplace(aName, aMasters.size());

        // A master the manifest declares takes the declared part and skips both
        // heuristic passes, so the manifest wins over the name keyword and the
        // example layout. A master the manifest does not name falls to the name
        // heuristic.
        DesignMasterRole eRole = DesignMasterRoleFromName(aName);
        bool bDeclared = false;
        if (oManifest)
        {
            auto itRole = oManifest->maMasterRoles.find(aName);
            if (itRole != oManifest->maMasterRoles.end())
            {
                eRole = itRole->second;
                bDeclared = true;
            }
        }
        aMasters.push_back({ aName, eRole, AUTOLAYOUT_NONE, 0, bDeclared });
    }

    // Each example slide names a master and shows the layout it pairs with. The
    // layout fixes the part of a master whose name carried no keyword and the
    // manifest did not declare, and every example counts towards how heavily a
    // master is used.
    const sal_uInt16 nSlides = rTemplate.GetSdPageCount(PageKind::Standard);
    for (sal_uInt16 i = 0; i < nSlides; ++i)
    {
        SdPage* pSlide = rTemplate.GetSdPage(i, PageKind::Standard);
        if (!pSlide)
            continue;
        const OUString aMaster = static_cast<SdPage&>(pSlide->TRG_GetMasterPage()).GetName();
        auto it = aIndex.find(aMaster);
        if (it == aIndex.end())
            continue;
        DesignTemplateMaster& rMaster = aMasters[it->second];
        ++rMaster.mnExampleUses;
        const AutoLayout eLayout = pSlide->GetAutoLayout();
        if (rMaster.meExampleLayout == AUTOLAYOUT_NONE)
            rMaster.meExampleLayout = eLayout;
        if (!rMaster.mbDeclared && rMaster.meRole == DesignMasterRole::Unknown)
        {
            // A title-and-subtitle example reads as an opening, a title-only or
            // blank example as a section break, anything with body content as a
            // body slide.
            if (eLayout == AUTOLAYOUT_TITLE)
                rMaster.meRole = DesignMasterRole::Title;
            else if (eLayout == AUTOLAYOUT_TITLE_ONLY || eLayout == AUTOLAYOUT_NONE)
                rMaster.meRole = DesignMasterRole::Divider;
            else
                rMaster.meRole = DesignMasterRole::Content;
        }
    }
    return aMasters;
}

namespace {

const SvxFieldItem* findField(editeng::Section const & rSection)
{
    for (SfxPoolItem const * pPool: rSection.maAttributes)
    {
        if (pPool->Which() == EE_FEATURE_FIELD)
            return static_cast<const SvxFieldItem*>(pPool);
    }
    return nullptr;
}

bool hasCustomPropertyField(std::vector<editeng::Section> const & aSections, std::u16string_view rName)
{
    for (editeng::Section const & rSection : aSections)
    {
        const SvxFieldItem* pFieldItem = findField(rSection);
        if (pFieldItem)
        {
            const editeng::CustomPropertyField* pCustomPropertyField = dynamic_cast<const editeng::CustomPropertyField*>(pFieldItem->GetField());
            if (pCustomPropertyField && pCustomPropertyField->GetName() == rName)
                return true;
        }
    }
    return false;
}

OUString getWeightString(SfxItemSet const & rItemSet)
{
    OUString sWeightString = u"NORMAL"_ustr;

    if (const SfxPoolItem* pItem = rItemSet.GetItem(EE_CHAR_WEIGHT, false))
    {
        const SvxWeightItem* pWeightItem = dynamic_cast<const SvxWeightItem*>(pItem);
        if (pWeightItem && pWeightItem->GetWeight() == WEIGHT_BOLD)
            sWeightString = u"BOLD"_ustr;
    }
    return sWeightString;
}

class TransformWarningCollector
{
    std::vector<std::string> maWarnings;
    static inline TransformWarningCollector* gpCurrent = nullptr;
public:
    TransformWarningCollector() { gpCurrent = this; }
    ~TransformWarningCollector() { gpCurrent = nullptr; }
    static void add(const std::string& rWarning)
    {
        if (gpCurrent)
            gpCurrent->maWarnings.push_back(rWarning);
    }
    const std::vector<std::string>& getWarnings() const { return maWarnings; }
};

void lcl_LogWarning(const std::string& rWarning)
{
    KIT_WARN("sd.transform", rWarning);
    TransformWarningCollector::add(rWarning);
}

void lcl_UnoCommand(const std::string& rText)
{
    if (rText.size() > 0)
    {
        OUString aCmd;
        std::vector<beans::PropertyValue> aArg;
        std::size_t nSpace = rText.find(' ');
        if (nSpace != std::string::npos)
        {
            aCmd = OStringToOUString(rText.substr(0, nSpace), RTL_TEXTENCODING_UTF8);
            std::string aArgText = rText.substr(nSpace + 1);

            aArg = comphelper::JsonToPropertyValues(aArgText);
        }
        else
        {
            aCmd = OStringToOUString(rText, RTL_TEXTENCODING_UTF8);
        }

        OUString aCmdSub;
        if (aCmd.startsWith(".uno:"))
        {
            aCmdSub = aCmd.subView(5);
        }
        else
        {
            lcl_LogWarning("FillApi SlideCmd: uno command not recognized'" + rText + "'");
            return;
        }

        // Check if the uno command is allowed
        const std::map<std::u16string_view, KitUnoCommand>& rUnoCommandList
            = GetKitUnoCommandList();
        auto aCmdData = rUnoCommandList.find(aCmdSub);
        if (aCmdData != rUnoCommandList.end())
        {
            // Make the uno command synchron
            aArg.push_back(comphelper::makePropertyValue(u"SynchronMode"_ustr, true));

            // Todo: check why it does not work on my windows system
            comphelper::dispatchCommand(aCmd, comphelper::containerToSequence(aArg));
        }
        else
        {
            lcl_LogWarning("FillApi SlideCmd: uno command not recognized'" + rText + "'");
        }
    }
}

// Resolves a design-template name to the URL of the template document that
// carries that look. The appearance - slide background, placeholder text styles,
// fonts, and theme - lives in the template's master slide and is copied onto the
// deck through the presentation-layout path. The name is matched against the
// available templates by base file name, ignoring letter case. Returns an empty
// optional when no template matches, so the caller can reject the name rather
// than reach for an arbitrary file.
std::optional<OUString> lcl_ResolveDesignTemplateUrl(std::string_view rName)
{
    const OUString aWanted = OStringToOUString(rName, RTL_TEXTENCODING_UTF8);

    for (const auto& [rTemplateName, rUrl] : CollectDesignTemplates())
    {
        if (rTemplateName.equalsIgnoreAsciiCase(aWanted))
            return rUrl;
    }
    return std::nullopt;
}

// True if the document already has a standard master page with this name.
bool lcl_DocHasMaster(SdDrawDocument* pDoc, std::u16string_view rName)
{
    const sal_uInt16 nCount = pDoc->GetMasterSdPageCount(PageKind::Standard);
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        SdPage* pMaster = pDoc->GetMasterSdPage(i, PageKind::Standard);
        if (pMaster && pMaster->GetName() == rName)
            return true;
    }
    return false;
}

// True for a layout that carries body content (an outline or one of the
// multi-content arrangements). The title-only, title-and-subtitle, and blank
// layouts carry none, so a slide using one of them reads as an opening,
// section break, or closing rather than a body slide.
bool lcl_LayoutHasBody(AutoLayout eLayout)
{
    return eLayout != AUTOLAYOUT_TITLE && eLayout != AUTOLAYOUT_TITLE_ONLY
           && eLayout != AUTOLAYOUT_NONE;
}

// Chooses a template master for each generated slide by the part the slide
// plays - opening, section break, body, or closing - rather than by its
// placeholder layout alone. A design template carries one master per part, but
// several of them often share the same layout (an opening and a section break
// can both be just a title), so the layout cannot tell them apart. The parts
// come from CollectDesignTemplateMasters. A body slide is matched to a body
// master of the same layout when the template has one, otherwise to the body
// master its example slides use most. This is the fallback for a slide the
// model did not give a design of its own.
class DesignMasterChooser
{
public:
    explicit DesignMasterChooser(SdDrawDocument& rTemplate)
    {
        const sal_uInt16 nMasters = rTemplate.GetMasterSdPageCount(PageKind::Standard);
        if (nMasters == 0)
            return;
        maFallback = rTemplate.GetMasterSdPage(0, PageKind::Standard)->GetName();

        // Keep one master per non-body part, preferring the strongest source of
        // its part: a manifest declaration beats a name keyword, which beats a
        // part decided only by an example layout. Gather the body masters with
        // their example layout and usage so a body slide can be matched by
        // layout and, failing that, to the most-used body master.
        std::map<DesignMasterRole, int> aRolePriority;
        for (const DesignTemplateMaster& rMaster : CollectDesignTemplateMasters(rTemplate))
        {
            if (rMaster.meRole == DesignMasterRole::Content)
            {
                maBodyMasters.push_back(
                    { rMaster.maName, rMaster.meExampleLayout, rMaster.mnExampleUses });
                continue;
            }
            if (rMaster.meRole == DesignMasterRole::Unknown)
                continue;
            const int nPriority
                = rMaster.mbDeclared
                      ? 2
                      : (DesignMasterRoleFromName(rMaster.maName) != DesignMasterRole::Unknown ? 1
                                                                                               : 0);
            auto it = maRoleMaster.find(rMaster.meRole);
            if (it == maRoleMaster.end() || nPriority > aRolePriority[rMaster.meRole])
            {
                maRoleMaster[rMaster.meRole] = rMaster.maName;
                aRolePriority[rMaster.meRole] = nPriority;
            }
        }

        std::sort(maBodyMasters.begin(), maBodyMasters.end(),
                  [](const BodyMaster& rA, const BodyMaster& rB) { return rA.mnUsage > rB.mnUsage; });

        // The manifest can map an intent word straight to a master. The reader
        // already dropped any entry whose master the template does not have.
        if (std::optional<AIDesignManifest> oManifest = ReadAIDesignManifest(rTemplate))
            maIntentMasters = std::move(oManifest->maIntentMasters);
    }

    // The master the template's manifest maps this intent word to, or no value
    // when the manifest maps nothing for it.
    std::optional<OUString> masterForIntent(std::u16string_view rIntent) const
    {
        auto it = maIntentMasters.find(OUString(rIntent));
        if (it != maIntentMasters.end())
            return it->second;
        return std::nullopt;
    }

    // The master for a slide whose part the model named. A body slide takes a
    // body master matching its layout; an opening, divider, or closing takes the
    // master for that part, falling through to another part and then a body or
    // the fallback master when the template carries no master for it.
    OUString masterForPart(DesignMasterRole ePart, AutoLayout eLayout) const
    {
        if (ePart == DesignMasterRole::Content)
            return bodyMaster(eLayout);
        return pickRole({ ePart, DesignMasterRole::Divider, DesignMasterRole::Title,
                          DesignMasterRole::Closing });
    }

    OUString masterForSlide(AutoLayout eLayout, bool bFirst, bool bLast) const
    {
        if (lcl_LayoutHasBody(eLayout))
            return bodyMaster(eLayout);

        // A title-only slide opens the deck, breaks a section, or closes it.
        // The position decides which part to prefer, and the template may not
        // carry every part, so fall through a short order of preference.
        if (bFirst)
            return pickRole({ DesignMasterRole::Title, DesignMasterRole::Divider,
                              DesignMasterRole::Closing });
        if (bLast)
            return pickRole({ DesignMasterRole::Closing, DesignMasterRole::Divider,
                              DesignMasterRole::Title });
        return pickRole(
            { DesignMasterRole::Divider, DesignMasterRole::Title, DesignMasterRole::Closing });
    }

private:
    struct BodyMaster
    {
        OUString maName;
        AutoLayout meLayout;
        sal_uInt16 mnUsage;
    };

    OUString pickRole(std::initializer_list<DesignMasterRole> aRoles) const
    {
        for (DesignMasterRole eRole : aRoles)
        {
            auto it = maRoleMaster.find(eRole);
            if (it != maRoleMaster.end())
                return it->second;
        }
        if (!maBodyMasters.empty())
            return maBodyMasters.front().maName;
        // When the template exposes no classifiable master at all, the
        // fallback is the first master, which can be a plain utility one.
        return maFallback;
    }

    // A body master matching the slide's layout, else the most-used body master,
    // else the fallback master.
    OUString bodyMaster(AutoLayout eLayout) const
    {
        for (const BodyMaster& rBody : maBodyMasters)
            if (rBody.meLayout == eLayout)
                return rBody.maName;
        if (!maBodyMasters.empty())
            return maBodyMasters.front().maName;
        return maFallback;
    }

    OUString maFallback;
    std::map<DesignMasterRole, OUString> maRoleMaster;
    std::map<OUString, OUString> maIntentMasters;
    std::vector<BodyMaster> maBodyMasters;
};

bool lcl_ReplaceWithImage(SdDrawDocument* pDoc, SdPage* pPage, int nObjId,
                                 const std::string& rImageUrl, const OUString& rAltText,
                                 SfxViewShell* pViewShell, int nPartId)
{
    OUString aURL = OStringToOUString(rImageUrl, RTL_TEXTENCODING_UTF8);

    Graphic aGraphic;
    ErrCode nError = GraphicFilter::LoadGraphic(
        aURL, OUString(), aGraphic, &GraphicFilter::GetGraphicFilter());
    if (nError != ERRCODE_NONE)
    {
        lcl_LogWarning("FillApi SlideCmd: Failed to load graphic from '" + rImageUrl + "'");
        return false;
    }

    rtl::Reference<SdrObject> pPickObj = pPage->GetObj(nObjId);
    rtl::Reference<SdrGrafObj> pNewGrafObj
        = new SdrGrafObj(*pDoc, aGraphic, pPickObj->GetLogicRect());
    pNewGrafObj->AdjustToMaxRect(pPickObj->GetLogicRect());
    pNewGrafObj->SetOutlinerParaObject(std::nullopt);
    pNewGrafObj->SetEmptyPresObj(false);
    // The image's text alternative for people using a screen reader. Stored as
    // the object title, which is what the accessibility layer reads for a
    // graphic and what exports as the svg:title of the shape.
    if (!rAltText.isEmpty())
        pNewGrafObj->SetTitle(rAltText);

    // Record undo before the replace: the action reads its object list from
    // the old object, which ReplaceObject removes from the page.
    if (pDoc->IsUndoEnabled())
        pDoc->AddUndo(pDoc->GetSdrUndoFactory().CreateUndoReplaceObject(*pPickObj, *pNewGrafObj));

    pPage->ReplaceObject(pNewGrafObj.get(), pPickObj->GetOrdNum());

    KitHelper::notifyInvalidation(pViewShell, nPartId, nullptr);
    return true;
}

// State shared by the slide commands of one transform. The page counts are
// the document's standard slide and master slide counts, refreshed before
// each command runs. mnActPageId is the slide the view currently shows (-1
// before the first command). mnNextPageId is the slide the next command
// works on; navigation aligns the two between commands. moApplyTemplateUrl
// holds the design-template URL resolved by an ApplyTemplate command; the
// template is applied once, after the whole command sequence. maSlideParts
// holds the slide part a SetSlidePart command gave a slide, keyed by the slide;
// the apply step places the slide on the master for that part before falling
// back to its own choice. maSlideIntents holds the intent word a SetSlideIntent
// command gave a slide; when the template's manifest maps that word to a master
// the apply step prefers it over the part. maTouchedPages holds the standard
// slides this
// transform inserted or modified; a slide the commands left alone is not in
// it.
struct SlideCommandContext
{
    SdDrawDocument* mpDoc;
    DrawView* mpDrawView;
    ::sd::View* mpView;
    ViewShellBase* mpViewShellBase;
    SfxUndoManager* mpUndoManager;
    bool mbUndo;
    sal_uInt16 mnPageCount = 0;
    sal_uInt16 mnMasterPageCount = 0;
    int mnActPageId = -1;
    int mnNextPageId = 0;
    std::optional<OUString> moApplyTemplateUrl = std::nullopt;
    std::map<const SdPage*, DesignMasterRole> maSlideParts = {};
    std::map<const SdPage*, OUString> maSlideIntents = {};
    std::set<const SdPage*> maTouchedPages = {};

    void touch(const SdPage* pPage)
    {
        if (pPage)
            maTouchedPages.insert(pPage);
    }
};

// Undo for a name or autolayout change. ModifyPageUndoAction snapshots the
// page state on construction, so call this before applying the change.
void lcl_AddModifyPageUndo(const SlideCommandContext& rCtx, SdPage* pPg,
                           const OUString& rNewName, AutoLayout eNewLayout)
{
    if (!rCtx.mbUndo || !pPg)
        return;
    SdrLayerAdmin& rLayerAdmin = rCtx.mpDoc->GetLayerAdmin();
    SdrLayerID aBg = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
    SdrLayerID aBgObj = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);
    SdrLayerIDSet aVisibleLayers = pPg->TRG_GetMasterPageVisibleLayers();
    rCtx.mpUndoManager->AddUndoAction(std::make_unique<ModifyPageUndoAction>(
        *rCtx.mpDoc, pPg, rNewName, eNewLayout, aVisibleLayers.IsSet(aBg),
        aVisibleLayers.IsSet(aBgObj)));
}

void handleJumpToSlide(SlideCommandContext& rCtx, const std::string& rKey,
                       const boost::property_tree::ptree& rValue)
{
    std::string aIndex = rValue.get_value<std::string>();
    if (aIndex == "last")
    {
        rCtx.mnNextPageId = rCtx.mnPageCount - 1;
    }
    else
    {
        rCtx.mnNextPageId = rValue.get_value<int>();
        if (rCtx.mnNextPageId >= rCtx.mnPageCount)
        {
            lcl_LogWarning("FillApi SlideCmd: Slide idx >= Slide count. '" + rKey + ": "
                           + aIndex
                           + "' (Slide count = " + std::to_string(rCtx.mnPageCount));
            rCtx.mnNextPageId = rCtx.mnPageCount - 1;
        }
        else if (rCtx.mnNextPageId < 0)
        {
            lcl_LogWarning("FillApi SlideCmd: Slide idx < 0. '" + rKey + ": " + aIndex
                           + "'");
            rCtx.mnNextPageId = 0;
        }
    }
}

void handleJumpToSlideByName(SlideCommandContext& rCtx, const std::string& rKey,
                             const boost::property_tree::ptree& rValue)
{
    std::string aPageName = rValue.get_value<std::string>();
    int nId = 0;
    while (nId < rCtx.mnPageCount
           && rCtx.mpDoc->GetSdPage(nId, PageKind::Standard)->GetName()
                  != OStringToOUString(aPageName, RTL_TEXTENCODING_UTF8))
    {
        nId++;
    }
    if (nId < rCtx.mnPageCount)
    {
        rCtx.mnNextPageId = nId;
    }
    else
    {
        lcl_LogWarning("FillApi SlideCmd: Slide name not found at: '" + rKey + ": "
                       + aPageName + "'");
    }
}

void handleInsertMasterSlide(SlideCommandContext& rCtx, const std::string& rKey,
                             const boost::property_tree::ptree& rValue)
{
    int nMasterPageId = 0;
    if (rKey == "InsertMasterSlideByName")
    {
        int nMId = 0;
        std::string aMPageName = rValue.get_value<std::string>();
        while (nMId < rCtx.mnMasterPageCount
               && rCtx.mpDoc->GetMasterSdPage(nMId, PageKind::Standard)->GetName()
                      != OStringToOUString(aMPageName, RTL_TEXTENCODING_UTF8))
        {
            nMId++;
        }
        if (nMId < rCtx.mnMasterPageCount)
        {
            nMasterPageId = nMId;
        }
        else
        {
            lcl_LogWarning("FillApi SlideCmd: MasterSlide name not found at: '" + rKey
                           + ": " + aMPageName + "'");
        }
    }
    else
    {
        nMasterPageId = rValue.get_value<int>();
    }

    if (nMasterPageId >= rCtx.mnMasterPageCount)
    {
        lcl_LogWarning("FillApi SlideCmd: Slide idx >= MasterSlide count. '" + rKey + ": "
                       + std::to_string(nMasterPageId)
                       + "' (Slide count = " + std::to_string(rCtx.mnMasterPageCount));
        nMasterPageId = rCtx.mnMasterPageCount - 1;
    }
    else if (nMasterPageId < 0)
    {
        lcl_LogWarning("FillApi SlideCmd: Slide idx < 0. '" + rKey + ": "
                       + std::to_string(nMasterPageId) + "'");
        nMasterPageId = 0;
    }

    SdPage* pMPage = rCtx.mpDoc->GetMasterSdPage(nMasterPageId, PageKind::Standard);
    SdPage* pPage = rCtx.mpDoc->GetSdPage(rCtx.mnActPageId, PageKind::Standard);

    // It will move to the next slide.
    rCtx.mnNextPageId
        = rCtx.mpDoc->CreatePage(pPage, PageKind::Standard, OUString(), OUString(),
                                 AUTOLAYOUT_TITLE_CONTENT, AUTOLAYOUT_NOTES, true, true,
                                 pPage->GetPageNum() + 2);

    SdPage* pPageStandard = rCtx.mpDoc->GetSdPage(rCtx.mnNextPageId, PageKind::Standard);
    SdPage* pPageNote = rCtx.mpDoc->GetSdPage(rCtx.mnNextPageId, PageKind::Notes);
    rCtx.touch(pPageStandard);

    // Change master value
    pPageStandard->TRG_SetMasterPage(*pMPage);
    // A notes page takes the notes master paired with the chosen standard
    // master. If the document has no notes master at that index, keep the one
    // CreatePage inherited.
    if (nMasterPageId < rCtx.mpDoc->GetMasterSdPageCount(PageKind::Notes))
    {
        SdPage* pNotesMPage = rCtx.mpDoc->GetMasterSdPage(nMasterPageId, PageKind::Notes);
        pPageNote->TRG_SetMasterPage(*pNotesMPage);
    }

    if (rCtx.mbUndo)
    {
        rCtx.mpDrawView->AddUndo(SdrUndoFactory::CreateUndoNewPage(*pPageStandard));
        rCtx.mpDrawView->AddUndo(SdrUndoFactory::CreateUndoNewPage(*pPageNote));
    }
}

void handleApplyTemplate(SlideCommandContext& rCtx, const std::string& /*rKey*/,
                         const boost::property_tree::ptree& rValue)
{
    // Record the template; it is applied to every slide after all commands run,
    // so slides inserted later are themed too, not just the slide that is
    // current when this command appears.
    std::string aName = rValue.get_value<std::string>();
    std::optional<OUString> oUrl = lcl_ResolveDesignTemplateUrl(aName);
    if (!oUrl)
        lcl_LogWarning("FillApi SlideCmd ApplyTemplate: unknown template name '" + aName + "'");
    else
        rCtx.moApplyTemplateUrl = oUrl;
}

void handleDeleteSlide(SlideCommandContext& rCtx, const std::string& rKey,
                       const boost::property_tree::ptree& rValue)
{
    int nPageIdToDel = rCtx.mnActPageId;
    if (rValue.get_value<std::string>() != "")
    {
        nPageIdToDel = rValue.get_value<int>();
    }

    if (rCtx.mnPageCount > 1)
    {
        if (nPageIdToDel >= rCtx.mnPageCount)
        {
            lcl_LogWarning("FillApi SlideCmd: Slide idx >= Slide count. '" + rKey + ": "
                           + std::to_string(nPageIdToDel)
                           + "' (Slide count = " + std::to_string(rCtx.mnPageCount));
            nPageIdToDel = rCtx.mnPageCount - 1;
        }
        else if (nPageIdToDel < 0)
        {
            lcl_LogWarning("FillApi SlideCmd: Slide idx < 0. '" + rKey + ": "
                           + std::to_string(nPageIdToDel) + "'");
            nPageIdToDel = 0;
        }
        SdPage* pDelStd = rCtx.mpDoc->GetSdPage(nPageIdToDel, PageKind::Standard);
        if (rCtx.mbUndo)
        {
            // Capture the notes page before removal.
            SdPage* pDelNotes = rCtx.mpDoc->GetSdPage(nPageIdToDel, PageKind::Notes);
            if (pDelNotes)
                rCtx.mpDrawView->AddUndo(SdrUndoFactory::CreateUndoDeletePage(*pDelNotes));
            if (pDelStd)
                rCtx.mpDrawView->AddUndo(SdrUndoFactory::CreateUndoDeletePage(*pDelStd));
        }
        rCtx.mpDoc->RemovePage(nPageIdToDel * 2 + 1);
        rCtx.mpDoc->RemovePage(nPageIdToDel * 2 + 1);
        // Forget the removed page; its address no longer names a slide of
        // this document.
        rCtx.maTouchedPages.erase(pDelStd);
        rCtx.maSlideParts.erase(pDelStd);
        rCtx.maSlideIntents.erase(pDelStd);

        if (nPageIdToDel <= rCtx.mnActPageId)
        {
            rCtx.mnNextPageId--;
        }
    }
    else
    {
        lcl_LogWarning("FillApi SlideCmd: Not enough Slide to delete 1. '" + rKey + ": "
                       + std::to_string(nPageIdToDel));
    }
}

void handleMoveSlide(SlideCommandContext& rCtx, const std::string& rKey,
                     const boost::property_tree::ptree& rValue)
{
    int nMoveFrom = rCtx.mnActPageId;
    if (rKey.starts_with("MoveSlide."))
    {
        nMoveFrom = stoi(rKey.substr(10));
    }
    int nMoveTo = rValue.get_value<int>();

    if (nMoveFrom == nMoveTo)
    {
        lcl_LogWarning("FillApi SlideCmd: Move slide to the same position. '" + rKey + ": "
                       + std::to_string(nMoveTo));
    }
    else if (nMoveFrom >= rCtx.mnPageCount || nMoveTo > rCtx.mnPageCount)
    {
        lcl_LogWarning("FillApi SlideCmd: Slide idx >= Slide count. '" + rKey + ": "
                       + std::to_string(nMoveTo));
    }
    else if (nMoveFrom < 0 || nMoveTo < 0)
    {
        lcl_LogWarning("FillApi SlideCmd: Slide idx < 0. '" + rKey + ": "
                       + std::to_string(nMoveTo));
    }
    else
    {
        // Move both the standard and the Note Page.
        // First move the page that will not change
        // the order of the other page.
        int nFirst = 1;
        if (nMoveFrom < nMoveTo)
        {
            nFirst = 2;
        }
        int nSecond = 3 - nFirst;

        if (rCtx.mbUndo)
        {
            SdrPage* pMv = rCtx.mpDoc->GetPage(nMoveFrom * 2 + nFirst);
            if (pMv)
                rCtx.mpDrawView->AddUndo(SdrUndoFactory::CreateUndoSetPageNum(
                    *pMv, nMoveFrom * 2 + nFirst, nMoveTo * 2 + nFirst));
        }
        rCtx.mpDoc->MovePage(nMoveFrom * 2 + nFirst, nMoveTo * 2 + nFirst);
        if (rCtx.mbUndo)
        {
            SdrPage* pMv = rCtx.mpDoc->GetPage(nMoveFrom * 2 + nSecond);
            if (pMv)
                rCtx.mpDrawView->AddUndo(SdrUndoFactory::CreateUndoSetPageNum(
                    *pMv, nMoveFrom * 2 + nSecond, nMoveTo * 2 + nSecond));
        }
        rCtx.mpDoc->MovePage(nMoveFrom * 2 + nSecond, nMoveTo * 2 + nSecond);

        // If the act page is moved, then follow it.
        if (rCtx.mnActPageId == nMoveFrom)
        {
            rCtx.mnNextPageId = nMoveTo;
        }
        else if (nMoveFrom < rCtx.mnActPageId && nMoveTo >= rCtx.mnActPageId)
        {
            rCtx.mnNextPageId = rCtx.mnActPageId - 1;
        }
        else if (nMoveFrom > rCtx.mnActPageId && nMoveTo <= rCtx.mnActPageId)
        {
            rCtx.mnNextPageId = rCtx.mnActPageId + 1;
        }
    }
}

void handleDuplicateSlide(SlideCommandContext& rCtx, const std::string& rKey,
                          const boost::property_tree::ptree& rValue)
{
    int nDupSlideId = rCtx.mnActPageId;
    if (rValue.get_value<std::string>() != "")
    {
        nDupSlideId = rValue.get_value<int>();
    }

    if (nDupSlideId >= rCtx.mnPageCount)
    {
        lcl_LogWarning("FillApi SlideCmd: Slide idx >= Slide count. '" + rKey + ": "
                       + std::to_string(nDupSlideId)
                       + "' (Slide count = " + std::to_string(rCtx.mnPageCount));
        nDupSlideId = rCtx.mnPageCount - 1;
    }
    else if (nDupSlideId < 0)
    {
        lcl_LogWarning("FillApi SlideCmd: Slide idx < 0. '" + rKey + ": "
                       + std::to_string(nDupSlideId) + "'");
        nDupSlideId = 0;
    }
    SdPage* pSourceStd = rCtx.mpDoc->GetSdPage(nDupSlideId, PageKind::Standard);
    rCtx.mpDoc->DuplicatePage(nDupSlideId);
    SdPage* pDupStd = rCtx.mpDoc->GetSdPage(nDupSlideId + 1, PageKind::Standard);
    rCtx.touch(pDupStd);
    // The copy plays the same part and carries the same intent as its source.
    if (pDupStd)
    {
        auto itPart = rCtx.maSlideParts.find(pSourceStd);
        if (itPart != rCtx.maSlideParts.end())
            rCtx.maSlideParts[pDupStd] = itPart->second;
        auto itIntent = rCtx.maSlideIntents.find(pSourceStd);
        if (itIntent != rCtx.maSlideIntents.end())
            rCtx.maSlideIntents[pDupStd] = itIntent->second;
    }
    if (rCtx.mbUndo)
    {
        SdPage* pDupNotes = rCtx.mpDoc->GetSdPage(nDupSlideId + 1, PageKind::Notes);
        if (pDupStd)
            rCtx.mpDrawView->AddUndo(SdrUndoFactory::CreateUndoNewPage(*pDupStd));
        if (pDupNotes)
            rCtx.mpDrawView->AddUndo(SdrUndoFactory::CreateUndoNewPage(*pDupNotes));
    }
    // Jump to the created page.
    rCtx.mnNextPageId = nDupSlideId + 1;
    // Make sure the current page will be set also.
    rCtx.mnActPageId = nDupSlideId;
}

void handleChangeLayout(SlideCommandContext& rCtx, const std::string& rKey,
                        const boost::property_tree::ptree& rValue)
{
    AutoLayout nLayoutId;
    if (rKey == "ChangeLayoutByName")
    {
        std::string aLayoutName = rValue.get_value<std::string>();

        nLayoutId = SdPage::stringToAutoLayout(
            OStringToOUString(aLayoutName, RTL_TEXTENCODING_UTF8));
        if (nLayoutId == AUTOLAYOUT_END)
        {
            lcl_LogWarning("FillApi SlideCmd: Layout name not found at: '" + rKey + ": "
                           + aLayoutName + "'");
            nLayoutId = AUTOLAYOUT_TITLE_CONTENT;
        }
    }
    else
    {
        nLayoutId = static_cast<AutoLayout>(rValue.get_value<int>());
        if (nLayoutId < AUTOLAYOUT_START || nLayoutId >= AUTOLAYOUT_END)
        {
            lcl_LogWarning("FillApi SlideCmd: Wrong Layout index at: '" + rKey + ": "
                           + std::to_string(nLayoutId) + "'");
            nLayoutId = AUTOLAYOUT_TITLE_CONTENT;
        }
    }

    // Todo warning:  ... if (nLayoutId >= ???)
    SdPage* pLayoutPage = rCtx.mpDoc->GetSdPage(rCtx.mnActPageId, PageKind::Standard);
    lcl_AddModifyPageUndo(rCtx, pLayoutPage, pLayoutPage->GetName(), nLayoutId);
    pLayoutPage->SetAutoLayout(nLayoutId, true);
    rCtx.touch(pLayoutPage);
}

void handleRenameSlide(SlideCommandContext& rCtx, const std::string& /*rKey*/,
                       const boost::property_tree::ptree& rValue)
{
    SdPage* pPageStandard = rCtx.mpDoc->GetSdPage(rCtx.mnActPageId, PageKind::Standard);
    OUString aNewName
        = OStringToOUString(rValue.get_value<std::string>(), RTL_TEXTENCODING_UTF8);
    lcl_AddModifyPageUndo(rCtx, pPageStandard, aNewName, pPageStandard->GetAutoLayout());
    pPageStandard->SetName(aNewName);
    rCtx.touch(pPageStandard);
}

void handleSetText(SlideCommandContext& rCtx, const std::string& rKey,
                   const boost::property_tree::ptree& rValue)
{
    int nObjId = stoi(rKey.substr(8));

    SdPage* pPageStandard = rCtx.mpDoc->GetSdPage(rCtx.mnActPageId, PageKind::Standard);
    int nObjCount = pPageStandard->GetObjCount();
    if (nObjId < 0)
    {
        lcl_LogWarning("FillApi SlideCmd SetText: Object idx < 0. '" + rKey + "'");
    }
    else if (nObjId < nObjCount)
    {
        SdrObject* pSdrObj = pPageStandard->GetObj(nObjId);
        if (pSdrObj->IsSdrTextObj())
        {
            SdrTextObj* pSdrTxt = static_cast<SdrTextObj*>(pSdrObj);
            if (rCtx.mbUndo)
                rCtx.mpDrawView->AddUndo(
                    rCtx.mpDoc->GetSdrUndoFactory().CreateUndoObjectSetText(*pSdrObj, 0));
            pSdrTxt->SetText(
                OStringToOUString(rValue.get_value<std::string>(), RTL_TEXTENCODING_UTF8));

            // Todo: maybe with empty string it should work elseway?
            pSdrObj->SetEmptyPresObj(false);
            rCtx.touch(pPageStandard);
        }
    }
    else
    {
        lcl_LogWarning("FillApi SlideCmd SetText: Object idx >= Object Count. '" + rKey
                       + "' (Object Count = " + std::to_string(nObjCount) + ")");
    }
}

void handleSetNotes(SlideCommandContext& rCtx, const std::string& /*rKey*/,
                    const boost::property_tree::ptree& rValue)
{
    if (rCtx.mnActPageId < 0)
        return;
    SdPage* pNotesPage
        = rCtx.mpDoc->GetSdPage(static_cast<sal_uInt16>(rCtx.mnActPageId), PageKind::Notes);
    if (!pNotesPage)
        return;

    SdrObject* pObj = pNotesPage->GetPresObj(PresObjKind::Notes);
    if (!pObj)
    {
        // A deck saved before its notes page carried a notes placeholder has
        // none yet. Re-initialise the notes page to the standard notes layout,
        // which builds the placeholder, then look again. Slides this transform
        // inserts already come with the notes layout, so this only matters for
        // pre-existing decks.
        pNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, true);
        pObj = pNotesPage->GetPresObj(PresObjKind::Notes);
    }
    if (!pObj || !pObj->IsSdrTextObj())
    {
        lcl_LogWarning("FillApi SlideCmd SetNotes: no notes placeholder on this slide.");
        return;
    }

    SdrTextObj* pSdrTxt = static_cast<SdrTextObj*>(pObj);
    if (rCtx.mbUndo)
        rCtx.mpDrawView->AddUndo(
            rCtx.mpDoc->GetSdrUndoFactory().CreateUndoObjectSetText(*pObj, 0));
    pSdrTxt->SetText(
        OStringToOUString(rValue.get_value<std::string>(), RTL_TEXTENCODING_UTF8));
    pObj->SetEmptyPresObj(false);

    // Re-theming keys on the standard slide, so mark that one rather than the
    // notes page.
    rCtx.touch(
        rCtx.mpDoc->GetSdPage(static_cast<sal_uInt16>(rCtx.mnActPageId), PageKind::Standard));
}

void handleInsertImage(SlideCommandContext& rCtx, const std::string& rKey,
                       const boost::property_tree::ptree& rValue)
{
    int nObjId;
    try
    {
        nObjId = stoi(rKey.substr(12));
    }
    catch (const std::exception&)
    {
        lcl_LogWarning("FillApi SlideCmd InsertImage: invalid object index in '" + rKey
                       + "'");
        return;
    }

    SdPage* pPageStandard = rCtx.mpDoc->GetSdPage(rCtx.mnActPageId, PageKind::Standard);
    int nObjCount = pPageStandard->GetObjCount();
    if (nObjId < 0)
    {
        lcl_LogWarning("FillApi SlideCmd InsertImage: Object idx < 0. '" + rKey + "'");
    }
    else if (nObjId >= nObjCount)
    {
        lcl_LogWarning("FillApi SlideCmd InsertImage: Object idx >= Object Count. '" + rKey
                       + "' (Object Count = " + std::to_string(nObjCount) + ")");
    }
    else
    {
        std::string aImageUrl = rValue.get_value<std::string>();
        if (lcl_ReplaceWithImage(rCtx.mpDoc, pPageStandard, nObjId, aImageUrl, OUString(),
                                 rCtx.mpViewShellBase, rCtx.mnActPageId))
            rCtx.touch(pPageStandard);
    }
}

void handleInsertImageAt(SlideCommandContext& rCtx, const std::string& rKey,
                         const boost::property_tree::ptree& rValue)
{
    // Format: InsertImageAt.SLIDE.OBJ
    // Inserts image on a specific slide without changing the active page view.
    std::string aSuffix = rKey.substr(14);
    auto nDotPos = aSuffix.find('.');
    if (nDotPos == std::string::npos)
    {
        lcl_LogWarning("FillApi SlideCmd InsertImageAt: missing dot separator in '" + rKey
                       + "'");
        return;
    }

    int nSlideId;
    int nObjId;
    try
    {
        nSlideId = stoi(aSuffix.substr(0, nDotPos));
        nObjId = stoi(aSuffix.substr(nDotPos + 1));
    }
    catch (const std::exception&)
    {
        lcl_LogWarning("FillApi SlideCmd InsertImageAt: invalid index in '" + rKey + "'");
        return;
    }

    int nPageCnt = rCtx.mpDoc->GetSdPageCount(PageKind::Standard);
    if (nSlideId < 0 || nSlideId >= nPageCnt)
    {
        lcl_LogWarning("FillApi SlideCmd InsertImageAt: slide idx out of range. '" + rKey
                       + "' (PageCount = " + std::to_string(nPageCnt) + ")");
        return;
    }

    SdPage* pPage = rCtx.mpDoc->GetSdPage(nSlideId, PageKind::Standard);
    int nObjCount = pPage->GetObjCount();
    if (nObjId < 0 || nObjId >= nObjCount)
    {
        lcl_LogWarning("FillApi SlideCmd InsertImageAt: obj idx out of range. '" + rKey
                       + "' (ObjCount = " + std::to_string(nObjCount) + ")");
        return;
    }

    // The value is either the image URL as a plain string, or an object that
    // carries the URL and a text alternative: {"url": "...", "alt": "..."}.
    std::string aImageUrl;
    OUString aAltText;
    if (auto oUrl = rValue.get_child_optional("url"))
    {
        aImageUrl = oUrl->get_value<std::string>();
        aAltText = OStringToOUString(rValue.get("alt", std::string()), RTL_TEXTENCODING_UTF8);
    }
    else
    {
        aImageUrl = rValue.get_value<std::string>();
    }

    if (lcl_ReplaceWithImage(rCtx.mpDoc, pPage, nObjId, aImageUrl, aAltText,
                             rCtx.mpViewShellBase, nSlideId))
        rCtx.touch(pPage);
}

void handleMarkObject(SlideCommandContext& rCtx, const std::string& rKey,
                      const boost::property_tree::ptree& rValue)
{
    bool bUnMark = rKey == "UnMarkObject";
    int nObjId = rValue.get_value<int>();

    SdPage* pPageStandard = rCtx.mpDoc->GetSdPage(rCtx.mnActPageId, PageKind::Standard);
    int nObjCount = pPageStandard->GetObjCount();

    // Todo: check id vs count
    if (nObjId < 0)
    {
        lcl_LogWarning("FillApi SlideCmd: Object idx < 0 at: '" + rKey
                       + std::to_string(nObjId) + "'");
    }
    if (nObjId < nObjCount)
    {
        SdrObject* pSdrObj = pPageStandard->GetObj(nObjId);
        rCtx.mpDrawView->MarkObj(pSdrObj, rCtx.mpDrawView->GetSdrPageView(), bUnMark);
        // A mark precedes an edit of the marked object, so the slide counts
        // as one this transform works on.
        rCtx.touch(pPageStandard);
    }
    else
    {
        lcl_LogWarning("FillApi SlideCmd: Object idx > Object Count. '" + rKey
                       + std::to_string(nObjId)
                       + "' (Object Count = " + std::to_string(nObjId));
    }
}

void handleEditTextObject(SlideCommandContext& rCtx, const std::string& rKey,
                          const boost::property_tree::ptree& rValue)
{
    int nObjId = stoi(rKey.substr(15));
    SdPage* pPageStandard = rCtx.mpDoc->GetSdPage(rCtx.mnActPageId, PageKind::Standard);
    int nObjCount = pPageStandard->GetObjCount();
    if (nObjId < 0)
    {
        lcl_LogWarning("FillApi SlideCmd EditTextObject: Object idx < 0. '" + rKey + "'");
        return;
    }
    if (nObjId >= nObjCount)
    {
        lcl_LogWarning("FillApi SlideCmd EditTextObject: Object idx >= Object Count. '"
                       + rKey + "' (Object Count = " + std::to_string(nObjCount) + ")");
        return;
    }

    SdrObject* pSdrObj = pPageStandard->GetObj(nObjId);
    if (!pSdrObj->IsSdrTextObj())
    {
        lcl_LogWarning("FillApi SlideCmd EditTextObject: Object is not a TextObject. '"
                       + rKey + "'");
        return;
    }

    SdrTextObj* pSdrTxt = static_cast<SdrTextObj*>(pSdrObj);
    rCtx.touch(pPageStandard);
    SdrView* pView1 = rCtx.mpView;
    pView1->MarkObj(pSdrTxt, pView1->GetSdrPageView());
    pView1->SdrBeginTextEdit(pSdrTxt);
    EditView& rEditView = pView1->GetTextEditOutlinerView()->GetEditView();
    for (const auto& aItem4Obj : rValue)
    {
        const auto& aItem4
            = aItem4Obj.first == "" ? *aItem4Obj.second.ordered_begin() : aItem4Obj;

        if (aItem4.first == "SelectText")
        {
            std::vector<int> aValues;
            for (const auto& aItem5 : aItem4.second)
            {
                //if == last?
                aValues.push_back(aItem5.second.get_value<int>());
            }
            if (aValues.size() == 0)
            {
                //select the whole text
                aValues.push_back(0);
                aValues.push_back(0);
                aValues.push_back(EE_PARA_MAX);
                aValues.push_back(EE_TEXTPOS_MAX);
            }
            else if (aValues.size() == 1)
            {
                //select the paragraph
                aValues.push_back(0);
                aValues.push_back(aValues[0]);
                aValues.push_back(EE_TEXTPOS_MAX);
            }
            else if (aValues.size() == 2)
            {
                // set the cursor without selecting anything
                aValues.push_back(aValues[0]);
                aValues.push_back(aValues[1]);
            }
            else if (aValues.size() == 3)
            {
                aValues.push_back(EE_TEXTPOS_MAX);
            }

            const ESelection rNewSel(aValues[0], aValues[1], aValues[2], aValues[3]);
            rEditView.SetSelection(rNewSel);
        }
        else if (aItem4.first == "SelectParagraph")
        {
            int nParaId = aItem4.second.get_value<int>();

            const ESelection rNewSel(nParaId, 0, nParaId, EE_TEXTPOS_MAX);
            rEditView.SetSelection(rNewSel);
        }
        else if (aItem4.first == "InsertText")
        {
            OUString aText = OStringToOUString(aItem4.second.get_value<std::string>(),
                                               RTL_TEXTENCODING_UTF8);
            // It select the inserted text also
            rEditView.InsertText(aText, true);
        }
        else if (aItem4.first == "UnoCommand")
        {
            std::string aText = aItem4.second.get_value<std::string>();
            lcl_UnoCommand(aText);
        }
    }
    pView1->SdrEndTextEdit();
}

void handleUnoCommand(SlideCommandContext& /*rCtx*/, const std::string& /*rKey*/,
                      const boost::property_tree::ptree& rValue)
{
    std::string aText = rValue.get_value<std::string>();
    lcl_UnoCommand(aText);
}

void handleSetSlidePart(SlideCommandContext& rCtx, const std::string& /*rKey*/,
                        const boost::property_tree::ptree& rValue)
{
    if (rCtx.mnActPageId < 0)
        return;
    SdPage* pPage
        = rCtx.mpDoc->GetSdPage(static_cast<sal_uInt16>(rCtx.mnActPageId), PageKind::Standard);
    if (!pPage)
        return;
    // Record the part the model gave this slide. The apply step places the slide
    // on the template master for that part once the template is open. A word that
    // is not a known part is dropped, leaving the slide to the fallback chooser.
    const OUString aPart
        = OStringToOUString(rValue.get_value<std::string>(), RTL_TEXTENCODING_UTF8);
    if (std::optional<DesignMasterRole> oRole = WireNameToDesignRole(aPart))
    {
        rCtx.maSlideParts[pPage] = *oRole;
        rCtx.touch(pPage);
    }
}

void handleSetSlideIntent(SlideCommandContext& rCtx, const std::string& /*rKey*/,
                          const boost::property_tree::ptree& rValue)
{
    if (rCtx.mnActPageId < 0)
        return;
    SdPage* pPage
        = rCtx.mpDoc->GetSdPage(static_cast<sal_uInt16>(rCtx.mnActPageId), PageKind::Standard);
    if (!pPage)
        return;
    // Record the intent word the model gave this slide. The apply step prefers
    // the template master the manifest maps the word to over the part-based
    // choice. The engine does not interpret the word, so a word that does not
    // fit the narrow shape is dropped and the slide falls to the part and then
    // the positional choice.
    const OUString aIntent
        = OStringToOUString(rValue.get_value<std::string>(), RTL_TEXTENCODING_UTF8);
    if (lcl_IsValidDesignIntentWord(aIntent))
    {
        rCtx.maSlideIntents[pPage] = aIntent;
        rCtx.touch(pPage);
    }
}

// One entry of the slide-command vocabulary. maName is the command's key in
// the transform JSON. With mbPrefixMatch the key is matched by prefix because
// it carries arguments after the name, like the object index in "SetText.0".
// With mbDirectPageTarget the key itself names the slide the command works
// on, instead of the slide the view currently shows.
struct SlideCommand
{
    std::string_view maName;
    bool mbPrefixMatch;
    bool mbDirectPageTarget;
    void (*mpHandler)(SlideCommandContext& rCtx, const std::string& rKey,
                      const boost::property_tree::ptree& rValue);
};

constexpr SlideCommand aSlideCommands[] = {
    { "JumpToSlide", false, false, &handleJumpToSlide },
    { "JumpToSlideByName", false, false, &handleJumpToSlideByName },
    { "InsertMasterSlide", false, false, &handleInsertMasterSlide },
    { "InsertMasterSlideByName", false, false, &handleInsertMasterSlide },
    { "ApplyTemplate", false, false, &handleApplyTemplate },
    { "DeleteSlide", false, false, &handleDeleteSlide },
    { "MoveSlide", true, false, &handleMoveSlide },
    { "DuplicateSlide", false, false, &handleDuplicateSlide },
    { "ChangeLayout", false, false, &handleChangeLayout },
    { "ChangeLayoutByName", false, false, &handleChangeLayout },
    { "RenameSlide", false, false, &handleRenameSlide },
    { "SetText.", true, false, &handleSetText },
    { "InsertImage.", true, false, &handleInsertImage },
    { "InsertImageAt.", true, true, &handleInsertImageAt },
    { "MarkObject", false, false, &handleMarkObject },
    { "UnMarkObject", false, false, &handleMarkObject },
    { "EditTextObject.", true, false, &handleEditTextObject },
    { "UnoCommand", false, false, &handleUnoCommand },
    { "SetSlidePart", false, false, &handleSetSlidePart },
    { "SetSlideIntent", false, false, &handleSetSlideIntent },
    { "SetNotes", false, false, &handleSetNotes },
};

// Looks up the slide command a JSON key addresses. Returns null for a key
// that is not part of the vocabulary.
const SlideCommand* lcl_FindSlideCommand(const std::string& rKey)
{
    for (const SlideCommand& rCommand : aSlideCommands)
    {
        const bool bMatch = rCommand.mbPrefixMatch ? rKey.starts_with(rCommand.maName)
                                                   : rKey == rCommand.maName;
        if (bMatch)
            return &rCommand;
    }
    return nullptr;
}

class ClassificationCommon
{
protected:
    sd::DrawViewShell& m_rDrawViewShell;
    uno::Reference<document::XDocumentProperties> m_xDocumentProperties;
    uno::Reference<beans::XPropertyContainer> m_xPropertyContainer;
    sfx::ClassificationKeyCreator m_aKeyCreator;
public:
    ClassificationCommon(sd::DrawViewShell& rDrawViewShell, const css::uno::Reference<css::document::XDocumentProperties>& rDocProps)
        : m_rDrawViewShell(rDrawViewShell)
        , m_xDocumentProperties(rDocProps)
        , m_xPropertyContainer(m_xDocumentProperties->getUserDefinedProperties())
        , m_aKeyCreator(SfxClassificationHelper::getPolicyType())
    {}
};

class ClassificationCollector : public ClassificationCommon
{
private:
    std::vector<svx::ClassificationResult> m_aResults;

    void iterateSectionsAndCollect(std::vector<editeng::Section> const & rSections, EditTextObject const & rEditText)
    {
        sal_Int32 nCurrentParagraph = -1;
        OUString sBlank;

        for (editeng::Section const & rSection : rSections)
        {
            // Insert new paragraph if needed
            while (nCurrentParagraph < rSection.mnParagraph)
            {
                nCurrentParagraph++;
                // Get Weight of current paragraph
                OUString sWeightProperty = getWeightString(rEditText.GetParaAttribs(nCurrentParagraph));
                // Insert new paragraph into collection
                m_aResults.emplace_back(svx::ClassificationType::PARAGRAPH, sWeightProperty, sBlank, sBlank);
            }

            const SvxFieldItem* pFieldItem = findField(rSection);
            const editeng::CustomPropertyField* pCustomPropertyField = pFieldItem ?
                dynamic_cast<const editeng::CustomPropertyField*>(pFieldItem->GetField()) :
                nullptr;
            if (pCustomPropertyField)
            {
                const OUString& aKey = pCustomPropertyField->GetName();
                if (m_aKeyCreator.isMarkingTextKey(aKey))
                {
                    m_aResults.emplace_back(svx::ClassificationType::TEXT,
                                           svx::classification::getProperty(m_xPropertyContainer, aKey),
                                           sBlank, sBlank);
                }
                else if (m_aKeyCreator.isCategoryNameKey(aKey) || m_aKeyCreator.isCategoryIdentifierKey(aKey))
                {
                    m_aResults.emplace_back(svx::ClassificationType::CATEGORY,
                                           svx::classification::getProperty(m_xPropertyContainer, aKey),
                                           sBlank, sBlank);
                }
                else if (m_aKeyCreator.isMarkingKey(aKey))
                {
                    m_aResults.emplace_back(svx::ClassificationType::MARKING,
                                           svx::classification::getProperty(m_xPropertyContainer, aKey),
                                           sBlank, sBlank);
                }
                else if (m_aKeyCreator.isIntellectualPropertyPartKey(aKey))
                {
                    m_aResults.emplace_back(svx::ClassificationType::INTELLECTUAL_PROPERTY_PART,
                                           svx::classification::getProperty(m_xPropertyContainer, aKey),
                                           sBlank, sBlank);
                }
            }
        }
    }

public:
    ClassificationCollector(sd::DrawViewShell & rDrawViewShell, const css::uno::Reference<css::document::XDocumentProperties>& rDocProps)
        : ClassificationCommon(rDrawViewShell, rDocProps)
    {}

    std::vector<svx::ClassificationResult> const & getResults() const
    {
        return m_aResults;
    }

    void collect()
    {
        // Set to MASTER mode
        EditMode eOldMode = m_rDrawViewShell.GetEditMode();
        if (eOldMode != EditMode::MasterPage)
            m_rDrawViewShell.ChangeEditMode(EditMode::MasterPage, false);

        // Scoped guard to revert to the previous mode
        comphelper::ScopeGuard const aGuard([this, eOldMode] () {
            m_rDrawViewShell.ChangeEditMode(eOldMode, false);
        });

        const sal_uInt16 nCount = m_rDrawViewShell.GetDoc()->GetMasterSdPageCount(PageKind::Standard);

        for (sal_uInt16 nPageIndex = 0; nPageIndex < nCount; ++nPageIndex)
        {
            SdPage* pMasterPage = m_rDrawViewShell.GetDoc()->GetMasterSdPage(nPageIndex, PageKind::Standard);
            for (const rtl::Reference<SdrObject>& pObject : *pMasterPage)
            {
                SdrRectObj* pRectObject = dynamic_cast<SdrRectObj*>(pObject.get());
                if (pRectObject && pRectObject->GetTextKind() == SdrObjKind::Text)
                {
                    OutlinerParaObject* pOutlinerParagraphObject = pRectObject->GetOutlinerParaObject();
                    if (pOutlinerParagraphObject)
                    {
                        const EditTextObject& rEditText = pOutlinerParagraphObject->GetTextObject();
                        std::vector<editeng::Section> aSections;
                        rEditText.GetAllSections(aSections);

                        // Search for a custom property field that has the classification category identifier key
                        if (hasCustomPropertyField(aSections, m_aKeyCreator.makeCategoryNameKey()))
                        {
                            iterateSectionsAndCollect(aSections, rEditText);
                            return;
                        }
                    }
                }
            }
        }
    }
};

class ClassificationInserter : public ClassificationCommon
{
private:
    /// Delete the previous existing classification object(s) - if they exist
    void deleteExistingObjects()
    {
        OUString sKey = m_aKeyCreator.makeCategoryNameKey();

        const sal_uInt16 nCount = m_rDrawViewShell.GetDoc()->GetMasterSdPageCount(PageKind::Standard);

        for (sal_uInt16 nPageIndex = 0; nPageIndex < nCount; ++nPageIndex)
        {
            SdPage* pMasterPage = m_rDrawViewShell.GetDoc()->GetMasterSdPage(nPageIndex, PageKind::Standard);
            for (const rtl::Reference<SdrObject>& pObject : *pMasterPage)
            {
                SdrRectObj* pRectObject = dynamic_cast<SdrRectObj*>(pObject.get());
                if (pRectObject && pRectObject->GetTextKind() == SdrObjKind::Text)
                {
                    OutlinerParaObject* pOutlinerParagraphObject = pRectObject->GetOutlinerParaObject();
                    if (pOutlinerParagraphObject)
                    {
                        const EditTextObject& rEditText = pOutlinerParagraphObject->GetTextObject();
                        std::vector<editeng::Section> aSections;
                        rEditText.GetAllSections(aSections);

                        if (hasCustomPropertyField(aSections, sKey))
                        {
                            pMasterPage->RemoveObject(pRectObject->GetOrdNum());
                        }
                    }
                }
            }
        }
    }

    void fillTheOutliner(Outliner* pOutliner, std::vector<svx::ClassificationResult> const & rResults)
    {
        sal_Int32 nParagraph = -1;
        for (svx::ClassificationResult const & rResult : rResults)
        {

            ESelection aPosition(nParagraph, EE_TEXTPOS_MAX);

            switch (rResult.meType)
            {
                case svx::ClassificationType::TEXT:
                {
                    OUString sKey = m_aKeyCreator.makeNumberedTextKey();
                    svx::classification::addOrInsertDocumentProperty(m_xPropertyContainer, sKey, rResult.msName);
                    pOutliner->QuickInsertField(SvxFieldItem(editeng::CustomPropertyField(sKey, rResult.msName), EE_FEATURE_FIELD), aPosition);
                }
                break;

                case svx::ClassificationType::CATEGORY:
                {
                    OUString sKey = m_aKeyCreator.makeCategoryNameKey();
                    pOutliner->QuickInsertField(SvxFieldItem(editeng::CustomPropertyField(sKey, rResult.msName), EE_FEATURE_FIELD), aPosition);
                }
                break;

                case svx::ClassificationType::MARKING:
                {
                    OUString sKey = m_aKeyCreator.makeNumberedMarkingKey();
                    svx::classification::addOrInsertDocumentProperty(m_xPropertyContainer, sKey, rResult.msName);
                    pOutliner->QuickInsertField(SvxFieldItem(editeng::CustomPropertyField(sKey, rResult.msName), EE_FEATURE_FIELD), aPosition);
                }
                break;

                case svx::ClassificationType::INTELLECTUAL_PROPERTY_PART:
                {
                    OUString sKey = m_aKeyCreator.makeNumberedIntellectualPropertyPartKey();
                    svx::classification::addOrInsertDocumentProperty(m_xPropertyContainer, sKey, rResult.msName);
                    pOutliner->QuickInsertField(SvxFieldItem(editeng::CustomPropertyField(sKey, rResult.msName), EE_FEATURE_FIELD), aPosition);
                }
                break;

                case svx::ClassificationType::PARAGRAPH:
                {
                    nParagraph++;
                    pOutliner->Insert(u""_ustr);

                    SfxItemSetFixed<EE_ITEMS_START, EE_ITEMS_END> aItemSet(m_rDrawViewShell.GetDoc()->GetPool());

                    if (rResult.msName == "BOLD")
                        aItemSet.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT));
                    else
                        aItemSet.Put(SvxWeightItem(WEIGHT_NORMAL, EE_CHAR_WEIGHT));

                    SvxNumRule aDefaultNumRule(SvxNumRuleFlags::NONE, 0, false);
                    aItemSet.Put(SvxNumBulletItem(std::move(aDefaultNumRule), EE_PARA_NUMBULLET));

                    pOutliner->SetParaAttribs(nParagraph, aItemSet);
                }
                break;

                default:
                break;
            }
        }
    }

public:
    ClassificationInserter(sd::DrawViewShell & rDrawViewShell, const css::uno::Reference<css::document::XDocumentProperties>& rDocProps)
        : ClassificationCommon(rDrawViewShell, rDocProps)
    {
    }

    void insert(std::vector<svx::ClassificationResult> const & rResults)
    {
        // Set to MASTER mode
        EditMode eOldMode = m_rDrawViewShell.GetEditMode();
        if (eOldMode != EditMode::MasterPage)
            m_rDrawViewShell.ChangeEditMode(EditMode::MasterPage, false);

        // Scoped guard to revert the mode
        comphelper::ScopeGuard const aGuard([this, eOldMode] () {
            m_rDrawViewShell.ChangeEditMode(eOldMode, false);
        });

        // Delete the previous existing object - if exists
        deleteExistingObjects();

        // Clear properties
        svx::classification::removeAllProperties(m_xPropertyContainer);

        SfxClassificationHelper aHelper(m_xDocumentProperties);

        // Apply properties from the BA policy
        for (svx::ClassificationResult const & rResult : rResults)
        {
            if (rResult.meType == svx::ClassificationType::CATEGORY)
                aHelper.SetBACName(rResult.msName, SfxClassificationHelper::getPolicyType());
        }

        // Insert full text as document property
        svx::classification::insertFullTextualRepresentationAsDocumentProperty(m_xPropertyContainer, m_aKeyCreator, rResults);

        // Create the outliner from the
        Outliner* pOutliner = m_rDrawViewShell.GetDoc()->GetInternalOutliner();
        OutlinerMode eOutlinerMode = pOutliner->GetOutlinerMode();

        comphelper::ScopeGuard const aOutlinerGuard([pOutliner, eOutlinerMode] () {
            pOutliner->Init(eOutlinerMode);
        });

        pOutliner->Init(OutlinerMode::TextObject);

        // Fill the outliner with the text from classification result
        fillTheOutliner(pOutliner, rResults);

        // Calculate to outliner text size
        pOutliner->UpdateFields();
        pOutliner->SetUpdateLayout(true);
        Size aTextSize(pOutliner->CalcTextSize());
        pOutliner->SetUpdateLayout(false);

        // Create objects, apply the outliner and add them (objects) to all master pages
        const sal_uInt16 nCount = m_rDrawViewShell.GetDoc()->GetMasterSdPageCount(PageKind::Standard);

        for (sal_uInt16 nPageIndex = 0; nPageIndex < nCount; ++nPageIndex)
        {
            SdPage* pMasterPage = m_rDrawViewShell.GetDoc()->GetMasterSdPage(nPageIndex, PageKind::Standard);
            if (!pMasterPage)
                continue;

            rtl::Reference<SdrRectObj> pObject = new SdrRectObj(
                *m_rDrawViewShell.GetDoc(), // TTTT should be reference
                ::tools::Rectangle(), SdrObjKind::Text);
            pObject->SetMergedItem(makeSdrTextAutoGrowWidthItem(true));
            pObject->SetOutlinerParaObject(pOutliner->CreateParaObject());
            pMasterPage->InsertObject(pObject.get());

            // Calculate position
            ::tools::Rectangle aRectangle(Point(), pMasterPage->GetSize());
            Point aPosition(aRectangle.Center().X(), aRectangle.Bottom());

            aPosition.AdjustX( -(aTextSize.Width() / 2) );
            aPosition.AdjustY( -(aTextSize.Height()) );

            pObject->SetLogicRect(::tools::Rectangle(aPosition, aTextSize));
        }
    }
};

}

void DrawViewShell::FuTransformDocumentStructure(SfxRequest& rReq)
{
    // Collect warnings from sub-commands so we can report them to the caller.
    TransformWarningCollector aWarnings;

    // get the parameter, what to transform
    OUString aDataJson;
    const SfxStringItem* pDataJson = rReq.GetArg<SfxStringItem>(FN_PARAM_1);
    if (pDataJson)
    {
        aDataJson = pDataJson->GetValue();
        aDataJson = rtl::Uri::decode(aDataJson, rtl_UriDecodeStrict, RTL_TEXTENCODING_UTF8);
    }

    // parse the JSON transform parameter
    boost::property_tree::ptree aTree;
    std::stringstream aStream(
        (std::string(OUStringToOString(aDataJson, RTL_TEXTENCODING_UTF8))));
    try
    {
        boost::property_tree::read_json(aStream, aTree);
    }
    catch (...)
    {
        lcl_LogWarning("FillApi Transform parameter, Wrong JSON format. ");
        throw;
    }

    // Group all changes below into one undo step so a single undo reverts the
    // whole transform. An empty list action (e.g. navigation only) is dropped.
    SfxUndoManager* pUndoManager = GetDocSh() ? GetDocSh()->GetUndoManager() : nullptr;
    const bool bUndo = mpDrawView && mpDrawView->IsUndoEnabled() && pUndoManager;
    if (bUndo)
        pUndoManager->EnterListAction(SdResId(STR_UNDO_TRANSFORM_DOCUMENT),
                                      SdResId(STR_UNDO_TRANSFORM_DOCUMENT), 0,
                                      GetViewShellBase().GetViewShellId());

    // Close the undo group when the function returns, even if a malformed
    // command in the loop below throws.
    comphelper::ScopeGuard aUndoGuard(
        [bUndo, pUndoManager]
        {
            if (bUndo)
                pUndoManager->LeaveListAction();
        });

    // The state the slide-command handlers share, including the design-template
    // URL that is applied after the command loop.
    SlideCommandContext aCtx{ GetDoc(), mpDrawView.get(), GetView(), &GetViewShellBase(),
                              pUndoManager, bUndo };

    // Iterate through the JSON data loaded into a tree structure
    for (const auto& aItem : aTree)
    {
        if (aItem.first == "Transforms")
        {
            // Handle all transformations
            for (const auto& aItem2Obj : aItem.second)
            {
                // handle `"Transforms": { `  and `"Transforms": [` cases as well
                // if an element is an object `{...}`, then get the first element of the object
                const auto& aItem2
                    = aItem2Obj.first == "" ? *aItem2Obj.second.ordered_begin() : aItem2Obj;

                if (aItem2.first == "SlideCommands")
                {
                    aCtx.mnActPageId = -1;
                    aCtx.mnNextPageId = 0;
                    for (const auto& aItem3Obj : aItem2.second)
                    {
                        // It accept direct property, or object as well
                        const auto& aItem3 = aItem3Obj.first == ""
                                                 ? *aItem3Obj.second.ordered_begin()
                                                 : aItem3Obj;

                        aCtx.mnPageCount = GetDoc()->GetSdPageCount(PageKind::Standard);
                        aCtx.mnMasterPageCount
                            = GetDoc()->GetMasterSdPageCount(PageKind::Standard);

                        const SlideCommand* pCommand = lcl_FindSlideCommand(aItem3.first);

                        // A command that names its target slide in its key runs
                        // without view navigation, to avoid jumping away from
                        // the user's current slide.
                        const bool bDirectPageTarget
                            = pCommand && pCommand->mbDirectPageTarget;
                        if (!bDirectPageTarget && aCtx.mnActPageId != aCtx.mnNextPageId)
                        {
                            // Make it sure it always point to a real page
                            if (aCtx.mnNextPageId < 0)
                                aCtx.mnNextPageId = 0;
                            if (aCtx.mnNextPageId >= aCtx.mnPageCount)
                                aCtx.mnNextPageId = aCtx.mnPageCount - 1;

                            aCtx.mnActPageId = aCtx.mnNextPageId;
                            // Make the view show the page the command runs on
                            maTabControl->SetCurPageId(aCtx.mnActPageId);
                            SdPage* pPageStandard
                                = GetDoc()->GetSdPage(aCtx.mnActPageId, PageKind::Standard);
                            mpDrawView->ShowSdrPage(pPageStandard);
                        }

                        if (pCommand)
                            pCommand->mpHandler(aCtx, aItem3.first, aItem3.second);
                    }
                }
            }
        }
        else if (aItem.first == "UnoCommand")
        {
            KitHelper::dispatchUnoCommand(aItem.second);
        }
    }

    // Apply the chosen design template once all slides exist, so slides inserted
    // after the ApplyTemplate command are themed too - not just the one current
    // when the command appeared. The template ships several masters, one per
    // slide part such as opening, section break, body, and closing. Put each
    // generated slide on the master for the part the model gave it with a
    // SetSlidePart command, so a deck uses the opening and divider designs as
    // well as the body design, not one master throughout. A slide the model gave
    // no part, or one whose part the template has no master for, is placed by
    // DesignMasterChooser instead; see it for how a master's name and its
    // example slide decide its part.
    if (aCtx.moApplyTemplateUrl)
    {
        SdDrawDocument* pTemplate = GetDoc()->OpenBookmarkDoc(*aCtx.moApplyTemplateUrl);
        const sal_uInt16 nStdCount = GetDoc()->GetSdPageCount(PageKind::Standard);
        if (pTemplate && pTemplate->GetMasterSdPageCount(PageKind::Standard) > 0
            && nStdCount > 0)
        {
            const DesignMasterChooser aChooser(*pTemplate);

            // The first application of a template - none of its masters are in
            // the document yet - themes every slide, so a design picked after
            // the content exists covers the whole deck. A later application of
            // the same template re-themes only the slides this transform
            // touched: the other slides keep the geometry and master the user
            // may have given them by hand since the last turn.
            bool bFirstApplication = true;
            const sal_uInt16 nTemplateMasters
                = pTemplate->GetMasterSdPageCount(PageKind::Standard);
            for (sal_uInt16 i = 0; bFirstApplication && i < nTemplateMasters; ++i)
            {
                if (lcl_DocHasMaster(
                        GetDoc(),
                        pTemplate->GetMasterSdPage(i, PageKind::Standard)->GetName()))
                    bFirstApplication = false;
            }

            for (sal_uInt16 i = 0; i < nStdCount; ++i)
            {
                SdPage* pPage = GetDoc()->GetSdPage(i, PageKind::Standard);
                if (!bFirstApplication && !aCtx.maTouchedPages.contains(pPage))
                    continue;

                // An intent the manifest maps to a master wins; then the part;
                // then the slide's position in the deck.
                OUString aMaster;
                auto itIntent = aCtx.maSlideIntents.find(pPage);
                if (itIntent != aCtx.maSlideIntents.end())
                {
                    if (std::optional<OUString> oMaster = aChooser.masterForIntent(itIntent->second))
                        aMaster = *oMaster;
                }
                if (aMaster.isEmpty())
                {
                    auto itPart = aCtx.maSlideParts.find(pPage);
                    if (itPart != aCtx.maSlideParts.end())
                        aMaster = aChooser.masterForPart(itPart->second, pPage->GetAutoLayout());
                }
                if (aMaster.isEmpty())
                    aMaster = aChooser.masterForSlide(pPage->GetAutoLayout(), i == 0,
                                                      i + 1 == nStdCount);

                // Import the master from the template the first time it is used;
                // afterwards take the copy already in this document so the styles
                // are not re-imported (which would collide on names).
                SdDrawDocument* pSource
                    = lcl_DocHasMaster(GetDoc(), aMaster) ? GetDoc() : pTemplate;
                GetDoc()->SetMasterPage(i, aMaster, pSource, false, false);

                // Re-run the autolayout so the slide's placeholders take the new
                // master's placeholder positions and sizes. The bInit flag
                // forces the placeholders to be re-arranged onto the master's
                // geometry, the same as applying the layout from the slide menu.
                // Without it a placeholder keeps the geometry it had on the old
                // master and can sit over a master's side band or run off the
                // designed text area.
                pPage->SetAutoLayout(pPage->GetAutoLayout(), true);
            }
        }
        if (pTemplate)
            GetDoc()->CloseBookmarkDoc();
    }

    // Build a JSON result so the caller knows what happened.
    tools::JsonWriter aJson;
    aJson.put("success", true);
    if (!aWarnings.getWarnings().empty())
    {
        auto aNode = aJson.startArray("warnings");
        for (const auto& rWarn : aWarnings.getWarnings())
            aJson.putSimpleValue(OUString::fromUtf8(rWarn));
    }
    rReq.SetReturnValue(SfxStringItem(FN_PARAM_1, OUString::fromUtf8(aJson.finishAndGetAsOString())));
}

/**
 * SfxRequests for temporary actions
 */

void DrawViewShell::FuTemporary(SfxRequest& rReq)
{
    DBG_ASSERT( mpDrawView, "sd::DrawViewShell::FuTemporary(), no draw view!" );
    if( !mpDrawView )
        return;

    CheckLineTo (rReq);

    DeactivateCurrentFunction();

    sal_uInt16 nSId = rReq.GetSlot();
    const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();

    switch ( nSId )
    {
        case SID_OUTLINE_TEXT_AUTOFIT:
        {
            SfxUndoManager* pUndoManager = GetDocSh()->GetUndoManager();
            if( rMarkList.GetMarkCount() == 1 )
            {
                pUndoManager->EnterListAction(u""_ustr, u""_ustr, 0, GetViewShellBase().GetViewShellId());
                mpDrawView->BegUndo();

                SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                bool bSet = pObj->GetMergedItemSet().GetItem<SdrTextFitToSizeTypeItem>(SDRATTR_TEXT_FITTOSIZE)->GetValue() != drawing::TextFitToSizeType_NONE;

                mpDrawView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoAttrObject(*pObj));

                if (!bSet)
                {
                    //If we are turning on AutoFit we have to turn these off if already on
                    if (pObj->GetMergedItemSet().GetItem<SdrOnOffItem>(SDRATTR_TEXT_AUTOGROWHEIGHT)->GetValue())
                        pObj->SetMergedItem(makeSdrTextAutoGrowHeightItem(false));
                    if (pObj->GetMergedItemSet().GetItem<SdrOnOffItem>(SDRATTR_TEXT_AUTOGROWWIDTH)->GetValue())
                        pObj->SetMergedItem(makeSdrTextAutoGrowWidthItem(false));
                }

                pObj->SetMergedItem(SdrTextFitToSizeTypeItem(bSet ? drawing::TextFitToSizeType_NONE : drawing::TextFitToSizeType_AUTOFIT));

                mpDrawView->EndUndo();
                pUndoManager->LeaveListAction();
            }
            Cancel();
            rReq.Done();
        }
        break;

        // area and line attributes: shall have
        // an own Execute method (like StateMethode)
        case SID_ATTR_FILL_STYLE:
        case SID_ATTR_FILL_COLOR:
        case SID_ATTR_FILL_GRADIENT:
        case SID_ATTR_FILL_HATCH:
        case SID_ATTR_FILL_BITMAP:
        case SID_ATTR_FILL_SHADOW:
        case SID_ATTR_SHADOW_COLOR:
        case SID_ATTR_SHADOW_TRANSPARENCE:
        case SID_ATTR_SHADOW_BLUR:
        case SID_ATTR_SHADOW_XDISTANCE:
        case SID_ATTR_SHADOW_YDISTANCE:
        case SID_ATTR_FILL_USE_SLIDE_BACKGROUND:
        case SID_ATTR_FILL_TRANSPARENCE:
        case SID_ATTR_FILL_FLOATTRANSPARENCE:

        case SID_ATTR_LINE_STYLE:
        case SID_ATTR_LINE_DASH:
        case SID_ATTR_LINE_WIDTH:
        case SID_ATTR_LINE_COLOR:
        case SID_ATTR_LINEEND_STYLE:
        case SID_ATTR_LINE_START:
        case SID_ATTR_LINE_END:
        case SID_ATTR_LINE_TRANSPARENCE:
        case SID_ATTR_LINE_JOINT:
        case SID_ATTR_LINE_CAP:

        case SID_ATTR_TEXT_FITTOSIZE:
        {
            if( rReq.GetArgs() )
            {
                const SfxItemSet* pReqArgs = rReq.GetArgs();
                std::unique_ptr<SfxItemSet> pNewArgs = pReqArgs->Clone();
                svx::convertDrawStyleArguments(*pNewArgs);

                const bool bUndo = mpDrawView->IsUndoEnabled();
                if (bUndo)
                    mpDrawView->BegUndo();

                mpDrawView->SetAttributes(*pNewArgs);
                svx::applyBareLineColorToMarked(*mpDrawView, *pReqArgs);

                if (bUndo)
                    mpDrawView->EndUndo();

                rReq.Done();
            }
            else
            {
                switch( rReq.GetSlot() )
                {
                    case SID_ATTR_FILL_SHADOW:
                    case SID_ATTR_SHADOW_COLOR:
                    case SID_ATTR_SHADOW_TRANSPARENCE:
                    case SID_ATTR_SHADOW_BLUR:
                    case SID_ATTR_SHADOW_XDISTANCE:
                    case SID_ATTR_SHADOW_YDISTANCE:
                    case SID_ATTR_FILL_STYLE:
                    case SID_ATTR_FILL_COLOR:
                    case SID_ATTR_FILL_GRADIENT:
                    case SID_ATTR_FILL_HATCH:
                    case SID_ATTR_FILL_BITMAP:
                    case SID_ATTR_FILL_USE_SLIDE_BACKGROUND:
                    case SID_ATTR_FILL_TRANSPARENCE:
                    case SID_ATTR_FILL_FLOATTRANSPARENCE:
                        GetViewFrame()->GetDispatcher()->Execute( SID_ATTRIBUTES_AREA, SfxCallMode::ASYNCHRON );
                        break;
                    case SID_ATTR_LINE_STYLE:
                    case SID_ATTR_LINE_DASH:
                    case SID_ATTR_LINE_WIDTH:
                    case SID_ATTR_LINE_COLOR:
                    case SID_ATTR_LINE_TRANSPARENCE:
                    case SID_ATTR_LINE_JOINT:
                    case SID_ATTR_LINE_CAP:
                        GetViewFrame()->GetDispatcher()->Execute( SID_ATTRIBUTES_LINE, SfxCallMode::ASYNCHRON );
                        break;
                    case SID_ATTR_TEXT_FITTOSIZE:
                        GetViewFrame()->GetDispatcher()->Execute( SID_TEXTATTR_DLG, SfxCallMode::ASYNCHRON );
                        break;
                }
            }
            Cancel();
        }
        break;

        case SID_HYPHENATION:
        {
            const SfxBoolItem* pItem = rReq.GetArg<SfxBoolItem>(SID_HYPHENATION);

            if( pItem )
            {
                SfxItemSetFixed<EE_PARA_HYPHENATE, EE_PARA_HYPHENATE> aSet( GetPool() );
                bool bValue = pItem->GetValue();
                aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, bValue ) );
                mpDrawView->SetAttributes( aSet );
            }
            else // only for testing purpose
            {
                OSL_FAIL(" no value for hyphenation!");
                SfxItemSetFixed<EE_PARA_HYPHENATE, EE_PARA_HYPHENATE> aSet( GetPool() );
                aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, true ) );
                mpDrawView->SetAttributes( aSet );
            }
            rReq.Done();
            Cancel();
        }
        break;

        case FN_TRANSFORM_DOCUMENT_STRUCTURE:
        {
            FuTransformDocumentStructure(rReq);
            rReq.Done();
        }
        break;

        case SID_INSERTPAGE:
        case SID_INSERTPAGE_QUICK:
        {
            SdPage* pNewPage = CreateOrDuplicatePage (rReq, mePageKind, GetActualPage());
            Cancel();
            if(HasCurrentFunction(SID_BEZIER_EDIT) )
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            if (pNewPage != nullptr)
                SwitchPage((pNewPage->GetPageNum()-1)/2);
            rReq.Done ();
        }
        break;
        case SID_INSERT_CANVAS_SLIDE:
        {
            sal_uInt16 nCanvasPageIndex = GetDoc()->GetOrInsertCanvasPage();
            Cancel(); // Don't know what this does
            SwitchPage(nCanvasPageIndex);
            GetDoc()->NotifyKitHasOverviewPage(true);
            rReq.Done();
        }
        break;

        case SID_SHUFFLE_PAGES:
        {
            if (!GetDoc()->HasCanvasPage())
                break;
            GetDoc()->ReshufflePages();
            Cancel();
            rReq.Done();
        }
        break;

        case SID_DUPLICATE_PAGE:
        {
            auto slideSorter = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(GetViewShellBase());
            SdPage* pNewPage = nullptr;
            if(slideSorter)
                DuplicateSelectedSlides(rReq);
            else
                pNewPage = CreateOrDuplicatePage (rReq, mePageKind, GetActualPage());
            Cancel();
            if(HasCurrentFunction(SID_BEZIER_EDIT) )
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            if(!slideSorter && pNewPage != nullptr)
                SwitchPage((pNewPage->GetPageNum()-1)/2);
            rReq.Done();
        }
        break;

        case SID_INSERT_MASTER_PAGE:
        {
            // Use the API to create a new page.
            rtl::Reference<SdXImpressDocument> xMasterPagesSupplier (
                GetDoc()->getUnoModel());
            if (xMasterPagesSupplier.is())
            {
                Reference<drawing::XDrawPages> xMasterPages (
                    xMasterPagesSupplier->getMasterPages());
                if (xMasterPages.is())
                {
                    sal_uInt16 nIndex = GetCurPagePos() + 1;
                    xMasterPages->insertNewByIndex (nIndex);

                    // Create shapes for the default layout.
                    SdPage* pMasterPage = GetDoc()->GetMasterSdPage(
                        nIndex, PageKind::Standard);
                    pMasterPage->CreateTitleAndLayout (true,true);
                }
            }

            Cancel();
            if(HasCurrentFunction(SID_BEZIER_EDIT))
                GetViewFrame()->GetDispatcher()->Execute(
                    SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            rReq.Done ();
        }
        break;

        case SID_MODIFYPAGE:
        {
            if (mePageKind==PageKind::Standard || mePageKind==PageKind::Notes ||
                (mePageKind==PageKind::Handout && meEditMode==EditMode::MasterPage) )
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }
                sal_uInt16 nPage = maTabControl->GetCurPagePos();
                mpActualPage = GetDoc()->GetSdPage(nPage, mePageKind);
                ::sd::ViewShell::mpImpl->ProcessModifyPageSlot (
                    rReq,
                    mpActualPage,
                    mePageKind);
            }

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_ASSIGN_LAYOUT:
        {
            if (mePageKind==PageKind::Standard || mePageKind==PageKind::Notes || (mePageKind==PageKind::Handout && meEditMode==EditMode::MasterPage))
            {
                if ( mpDrawView->IsTextEdit() )
                    mpDrawView->SdrEndTextEdit();

                ::sd::ViewShell::mpImpl->AssignLayout(rReq, mePageKind);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_RENAMEPAGE:
        case SID_RENAME_MASTER_PAGE:
        {
            if (mePageKind==PageKind::Standard || mePageKind==PageKind::Notes )
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }

                sal_uInt16 nPage = maTabControl->GetCurPagePos();
                SdPage* pCurrentPage = ( GetEditMode() == EditMode::Page )
                    ? GetDoc()->GetSdPage( nPage, GetPageKind() )
                    : GetDoc()->GetMasterSdPage( nPage, GetPageKind() );

                OUString aTitle = SdResId(STR_TITLE_RENAMESLIDE);
                OUString aDescr = SdResId(STR_DESC_RENAMESLIDE);
                const OUString& aPageName = pCurrentPage->GetName();

                if(rReq.GetArgs())
                {
                    OUString aName = rReq.GetArgs()->GetItem<const SfxStringItem>(SID_RENAMEPAGE)->GetValue();

                    bool bResult = RenameSlide( maTabControl->GetPageId(nPage), aName );
                    DBG_ASSERT( bResult, "Couldn't rename slide" );
                }
                else
                {
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    ScopedVclPtr<AbstractSvxNameDialog> aNameDlg(pFact->CreateSvxNameDialog(GetFrameWeld(), aPageName, aDescr));
                    aNameDlg->SetText( aTitle );
                    aNameDlg->SetCheckNameHdl( LINK( this, DrawViewShell, RenameSlideHdl ) );
                    aNameDlg->SetEditHelpId( HID_SD_NAMEDIALOG_PAGE );

                    if( aNameDlg->Execute() == RET_OK )
                    {
                        OUString aNewName = aNameDlg->GetName();
                        if (aNewName != aPageName)
                        {
                            bool bResult = RenameSlide( maTabControl->GetPageId(nPage), aNewName );
                            DBG_ASSERT( bResult, "Couldn't rename slide" );
                        }
                    }
                }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_RENAMEPAGE_QUICK:
        {
            if (mePageKind==PageKind::Standard || mePageKind==PageKind::Notes )
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }

                maTabControl->StartEditMode( maTabControl->GetCurPageId() );
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_ADD_SLIDE_SECTION:
        {
            SdPage* pPage = GetActualPage();
            if (pPage)
            {
                sal_uInt16 nPage = (pPage->GetPageNum() - 1) / 2;
                sd::SlideSectionManager& rMgr = GetDoc()->GetSectionManager();
                auto pUndo = GetDoc()->IsUndoEnabled()
                    ? std::make_unique<sd::UndoSlideSection>(
                          *GetDoc(), SdResId(STR_UNDO_ADD_SLIDE_SECTION))
                    : nullptr;
                rMgr.AddSection(nPage, SdResId(STR_DEFAULT_SLIDE_SECTION_NAME));
                if (pUndo)
                    GetDoc()->AddUndo(std::move(pUndo));
                GetDocSh()->SetModified();
            }
        }
        break;

        case SID_REMOVE_SLIDE_SECTION:
        {
            SdPage* pPage = GetActualPage();
            if (pPage)
            {
                sal_uInt16 nPage = (pPage->GetPageNum() - 1) / 2;
                sd::SlideSectionManager& rMgr = GetDoc()->GetSectionManager();
                sal_Int32 nSectionIdx = rMgr.GetSectionIndexForSlide(nPage);
                if (nSectionIdx >= 0 && rMgr.IsSectionStart(nPage))
                {
                    auto pUndo = GetDoc()->IsUndoEnabled()
                        ? std::make_unique<sd::UndoSlideSection>(
                              *GetDoc(), SdResId(STR_UNDO_REMOVE_SLIDE_SECTION))
                        : nullptr;
                    rMgr.RemoveSection(nSectionIdx);
                    if (pUndo)
                        GetDoc()->AddUndo(std::move(pUndo));
                    GetDocSh()->SetModified();
                }
            }
        }
        break;

        case SID_REMOVE_SLIDE_SECTION_AND_SLIDES:
        {
            SdPage* pPage = GetActualPage();
            if (pPage)
            {
                sal_uInt16 nPage = (pPage->GetPageNum() - 1) / 2;
                sd::SlideSectionManager& rMgr = GetDoc()->GetSectionManager();
                sal_Int32 nSectionIdx = rMgr.GetSectionIndexForSlide(nPage);
                if (nSectionIdx >= 0 && rMgr.IsSectionStart(nPage))
                {
                    const bool bUndo = GetDoc()->IsUndoEnabled();
                    if (bUndo)
                        GetView()->BegUndo(
                            SdResId(STR_UNDO_REMOVE_SLIDE_SECTION_AND_SLIDES));

                    rMgr.RemoveSectionSlides(nSectionIdx);

                    if (bUndo)
                        GetView()->EndUndo();
                    GetDocSh()->SetModified();
                    ResetActualPage();
                }
            }
        }
        break;

        case SID_RENAME_SLIDE_SECTION:
        {
            sd::SlideSectionManager& rMgr = GetDoc()->GetSectionManager();
            if (rReq.GetArgs())
            {
                const SfxInt32Item* pIndexItem
                    = rReq.GetArgs()->GetItem<SfxInt32Item>(SID_RENAME_SLIDE_SECTION, false);
                const SfxStringItem* pNameItem
                    = rReq.GetArgs()->GetItem<SfxStringItem>(SID_RENAMEPAGE, false);
                if (pIndexItem && pNameItem)
                {
                    sal_Int32 nSectionIndex = pIndexItem->GetValue();
                    OUString aNewName = pNameItem->GetValue();
                    if (nSectionIndex >= 0 && nSectionIndex < rMgr.GetSectionCount()
                        && !aNewName.isEmpty())
                    {
                        auto pUndo = GetDoc()->IsUndoEnabled()
                            ? std::make_unique<sd::UndoSlideSection>(
                                  *GetDoc(), SdResId(STR_UNDO_RENAME_SLIDE_SECTION))
                            : nullptr;
                        rMgr.RenameSection(nSectionIndex, aNewName);
                        if (pUndo)
                            GetDoc()->AddUndo(std::move(pUndo));
                        GetDocSh()->SetModified();
                    }
                }
            }
        }
        break;

        case SID_MOVE_SLIDE_SECTION_UP:
        {
            SdPage* pPage = GetActualPage();
            if (pPage)
            {
                sal_uInt16 nPage = (pPage->GetPageNum() - 1) / 2;
                sd::SlideSectionManager& rMgr = GetDoc()->GetSectionManager();
                sal_Int32 nSectionIdx = rMgr.GetSectionIndexForSlide(nPage);
                if (nSectionIdx > 0)
                {
                    // Group the section metadata undo with the page-reorder undo
                    // recorded by MovePages() inside MoveSection().
                    const bool bUndo = GetDoc()->IsUndoEnabled();
                    if (bUndo)
                        GetDoc()->BegUndo(SdResId(STR_UNDO_MOVE_SLIDE_SECTION));
                    auto pUndo = bUndo
                        ? std::make_unique<sd::UndoSlideSection>(
                              *GetDoc(), SdResId(STR_UNDO_MOVE_SLIDE_SECTION))
                        : nullptr;
                    rMgr.MoveSection(nSectionIdx, nSectionIdx - 1);
                    if (bUndo)
                    {
                        GetDoc()->AddUndo(std::move(pUndo));
                        GetDoc()->EndUndo();
                    }
                    GetDocSh()->SetModified();
                }
            }
        }
        break;

        case SID_MOVE_SLIDE_SECTION_DOWN:
        {
            SdPage* pPage = GetActualPage();
            if (pPage)
            {
                sal_uInt16 nPage = (pPage->GetPageNum() - 1) / 2;
                sd::SlideSectionManager& rMgr = GetDoc()->GetSectionManager();
                sal_Int32 nSectionIdx = rMgr.GetSectionIndexForSlide(nPage);
                if (nSectionIdx >= 0 && nSectionIdx < rMgr.GetSectionCount() - 1)
                {
                    const bool bUndo = GetDoc()->IsUndoEnabled();
                    if (bUndo)
                        GetDoc()->BegUndo(SdResId(STR_UNDO_MOVE_SLIDE_SECTION));
                    auto pUndo = bUndo
                        ? std::make_unique<sd::UndoSlideSection>(
                              *GetDoc(), SdResId(STR_UNDO_MOVE_SLIDE_SECTION))
                        : nullptr;
                    rMgr.MoveSection(nSectionIdx, nSectionIdx + 1);
                    if (bUndo)
                    {
                        GetDoc()->AddUndo(std::move(pUndo));
                        GetDoc()->EndUndo();
                    }
                    GetDocSh()->SetModified();
                }
            }
        }
        break;

        case SID_PAGESIZE :  // either this (no menu entries or something else!)
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();

            if (pArgs && pArgs->Count () == 3)
            {
                const SfxUInt32Item* pWidth = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGEWIDTH);
                const SfxUInt32Item* pHeight = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGEHEIGHT);
                const SfxBoolItem* pScaleAll = rReq.GetArg<SfxBoolItem>(ID_VAL_SCALEOBJECTS);

                Size aSize (pWidth->GetValue (), pHeight->GetValue ());

                SetupPage (aSize, 0, 0, 0, 0, true, false, pScaleAll->GetValue ());
                rReq.Ignore ();
                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            rReq.Ignore ();
            break;
        }

        case SID_PAGEMARGIN :  // or this (no menu entries or something else!)
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();

            if (pArgs && pArgs->Count () == 5)
            {
                const SfxUInt32Item* pLeft = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGELEFT);
                const SfxUInt32Item* pRight = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGERIGHT);
                const SfxUInt32Item* pUpper = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGETOP);
                const SfxUInt32Item* pLower = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGEBOTTOM);
                const SfxBoolItem* pScaleAll = rReq.GetArg<SfxBoolItem>(ID_VAL_SCALEOBJECTS);

                Size aEmptySize (0, 0);

                SetupPage (aEmptySize, pLeft->GetValue (), pRight->GetValue (),
                           pUpper->GetValue (), pLower->GetValue (),
                           false, true, pScaleAll->GetValue ());
                rReq.Ignore ();
                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            rReq.Ignore ();
            break;
        }

        case SID_ATTR_ZOOMSLIDER:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            const SfxUInt16Item* pScale = (pArgs && pArgs->Count () == 1) ?
                rReq.GetArg(SID_ATTR_ZOOMSLIDER) : nullptr;
            if (pScale && CHECK_RANGE (5, pScale->GetValue (), 3000))
            {
                SetZoom (pScale->GetValue ());

                SfxBindings& rBindings = GetViewFrame()->GetBindings();
                rBindings.Invalidate( SID_ATTR_ZOOM );
                rBindings.Invalidate( SID_ZOOM_IN );
                rBindings.Invalidate( SID_ZOOM_OUT );
                rBindings.Invalidate( SID_ATTR_ZOOMSLIDER );

            }

            Cancel();
            rReq.Done ();
            break;
        }

        case SID_ATTR_ZOOM:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            mbZoomOnPage = false;

            if ( pArgs )
            {
                SvxZoomType eZT = pArgs->Get( SID_ATTR_ZOOM ).GetType();
                switch( eZT )
                {
                    case SvxZoomType::PERCENT:
                    {
                        sal_uInt16 nZoom = pArgs->Get( SID_ATTR_ZOOM ).GetValue();
                        SetZoom( static_cast<::tools::Long>( nZoom ) );
                    }
                        break;

                    case SvxZoomType::OPTIMAL:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_ALL,
                                    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
                        break;

                    case SvxZoomType::PAGEWIDTH:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE_WIDTH,
                                    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
                        break;

                    case SvxZoomType::WHOLEPAGE:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE,
                                    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
                        break;
                    case SvxZoomType::PAGEWIDTH_NOBORDER:
                        OSL_FAIL("sd::DrawViewShell::FuTemporary(), SvxZoomType::PAGEWIDTH_NOBORDER not handled!" );
                        break;
                }
                rReq.Ignore ();
            }
            else
            {
                // open zoom dialog
                SetCurrentFunction( FuScale::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            }
            Cancel();
        }
        break;

        case SID_CHANGEBEZIER:
        case SID_CHANGEPOLYGON:
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            }

            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                if( rReq.GetSlot() == SID_CHANGEBEZIER )
                {
                    weld::WaitObject aWait(GetFrameWeld());
                    mpDrawView->ConvertMarkedToPathObj(false);
                }
                else
                {
                    if( mpDrawView->IsVectorizeAllowed() )
                    {
                        SetCurrentFunction( FuVectorize::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
                    }
                    else
                    {
                        weld::WaitObject aWait(GetFrameWeld());
                        mpDrawView->ConvertMarkedToPolyObj();
                    }
                }

                Invalidate(SID_CHANGEBEZIER);
                Invalidate(SID_CHANGEPOLYGON);
            }
            Cancel();

            if( HasCurrentFunction(SID_BEZIER_EDIT) )
            {   // where applicable, activate right edit action
                GetViewFrame()->GetDispatcher()->Execute(SID_SWITCH_POINTEDIT,
                                        SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
            }
            rReq.Ignore ();
            break;

        case SID_CONVERT_TO_CONTOUR:
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            }

            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->ConvertMarkedToPathObj(true);

                Invalidate(SID_CONVERT_TO_CONTOUR);
            }
            Cancel();

            rReq.Ignore ();
            break;

        case SID_CONVERT_TO_METAFILE:
        case SID_CONVERT_TO_BITMAP:
        {
            // End text edit mode when it is active because the metafile or
            // bitmap that will be created does not support it.
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            }

            if ( mpDrawView->IsPresObjSelected(true,true,true) )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());

                // create SdrGrafObj from metafile/bitmap
                Graphic aGraphic;
                switch (nSId)
                {
                    case SID_CONVERT_TO_METAFILE:
                    {
                        // switch on undo for the next operations
                        mpDrawView->BegUndo(SdResId(STR_UNDO_CONVERT_TO_METAFILE));
                        GDIMetaFile aMetaFile(mpDrawView->GetMarkedObjMetaFile());
                        aGraphic = Graphic(aMetaFile);
                    }
                    break;
                    case SID_CONVERT_TO_BITMAP:
                    {
                        // Disable spelling during conversion
                        bool bOnlineSpell = GetDoc()->GetOnlineSpell();
                        GetDoc()->SetOnlineSpell(false);

                        // switch on undo for the next operations
                        mpDrawView->BegUndo(SdResId(STR_UNDO_CONVERT_TO_BITMAP));
                        aGraphic = Graphic(mpDrawView->GetMarkedObjBitmap());
                        // Restore online spelling
                        GetDoc()->SetOnlineSpell(bOnlineSpell);
                    }
                    break;
                }

                // create new object
                rtl::Reference<SdrGrafObj> pGraphicObj = new SdrGrafObj(
                    *GetDoc(),
                    aGraphic);

                // get some necessary info and ensure it
                const size_t nMarkCount(rMarkList.GetMarkCount());
                SdrPageView* pPageView = mpDrawView->GetSdrPageView();
                OSL_ENSURE(nMarkCount, "DrawViewShell::FuTemporary: SID_CONVERT_TO_BITMAP with empty selection (!)");
                OSL_ENSURE(pPageView, "DrawViewShell::FuTemporary: SID_CONVERT_TO_BITMAP without SdrPageView (!)");

                // fit rectangle of new graphic object to selection's mark rect
                ::tools::Rectangle aAllMarkedRect;
                rMarkList.TakeBoundRect(pPageView, aAllMarkedRect);
                pGraphicObj->SetLogicRect(aAllMarkedRect);

                // #i71540# to keep the order, it is necessary to replace the lowest object
                // of the selection with the new object. This also means that with multi
                // selection, all other objects need to be deleted first
                SdrMark* pFirstMark = rMarkList.GetMark(0);
                SdrObject* pReplacementCandidate = pFirstMark->GetMarkedSdrObj();

                if(nMarkCount > 1)
                {
                    // take first object out of selection
                    mpDrawView->MarkObj(pReplacementCandidate, pPageView, true, true);

                    // clear remaining selection
                    mpDrawView->DeleteMarkedObj();
                }

                // #i124816# copy layer from lowest object which gets replaced
                pGraphicObj->SetLayer(pReplacementCandidate->GetLayer());

                // now replace lowest object with new one
                mpDrawView->ReplaceObjectAtView(pReplacementCandidate, *pPageView, pGraphicObj.get());

                // switch off undo
                mpDrawView->EndUndo();
            }
        }

        Cancel();

        rReq.Done ();
        break;

        case SID_REMOVE_HYPERLINK:
        {
            if (mpDrawView->IsTextEdit())
            {
                OutlinerView* pOutView = mpDrawView->GetTextEditOutlinerView();
                if (pOutView)
                    URLFieldHelper::RemoveURLField(pOutView->GetEditView());
            }
        }
        Cancel();
        rReq.Done ();
        break;

        case SID_SET_DEFAULT:
        {
            std::optional<SfxItemSet> pSet;

            if (mpDrawView->IsTextEdit())
            {
                pSet.emplace( GetPool(), svl::Items<EE_ITEMS_START, EE_ITEMS_END> );
                mpDrawView->SetAttributes( *pSet, true );
            }
            else
            {
                const size_t nCount = rMarkList.GetMarkCount();

                // For every presentation object a SfxItemSet of hard attributes
                // and the UserCall is stored in this list. This is because
                // at the following mpDrawView->SetAttributes( *pSet, true )
                // they get lost and have to be restored.
                std::vector<std::pair<std::unique_ptr<SfxItemSet>,SdrObjUserCall*> > aAttrList;
                SdPage* pPresPage = static_cast<SdPage*>( mpDrawView->GetSdrPageView()->GetPage() );

                for ( size_t i = 0; i < nCount; ++i )
                {
                    SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

                    if( pPresPage->IsPresObj( pObj ) )
                    {
                        auto pNewSet = std::make_unique<SfxItemSetFixed<SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT>>( GetDoc()->GetPool() );
                        pNewSet->Put(pObj->GetMergedItemSet());
                        aAttrList.emplace_back(std::move(pNewSet), pObj->GetUserCall());
                    }
                }

                pSet.emplace( GetPool() );
                mpDrawView->SetAttributes( *pSet, true );

                sal_uLong j = 0;

                for ( size_t i = 0; i < nCount; ++i )
                {
                    SfxStyleSheet* pSheet = nullptr;
                    SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

                    if (pObj->GetObjIdentifier() == SdrObjKind::TitleText)
                    {
                        pSheet = mpActualPage->GetStyleSheetForPresObj(PresObjKind::Title);
                        if (pSheet)
                            pObj->SetStyleSheet(pSheet, false);
                    }
                    else if(pObj->GetObjIdentifier() == SdrObjKind::OutlineText)
                    {
                        for (sal_uInt16 nLevel = 1; nLevel < 10; nLevel++)
                        {
                            pSheet = mpActualPage->GetStyleSheetForPresObj( PresObjKind::Outline );
                            DBG_ASSERT(pSheet, "Template for outline object not found");
                            if (pSheet)
                            {
                                pObj->StartListening(*pSheet);

                                if( nLevel == 1 )
                                    // text frame listens on StyleSheet of level1
                                    pObj->NbcSetStyleSheet(pSheet, false);
                            }
                        }
                    }

                    if( pPresPage->IsPresObj( pObj ) )
                    {
                        std::pair<std::unique_ptr<SfxItemSet>,SdrObjUserCall*> &rAttr = aAttrList[j++];

                        std::unique_ptr<SfxItemSet> & pNewSet(rAttr.first);
                        SdrObjUserCall* pUserCall = rAttr.second;

                        if ( pNewSet && pNewSet->GetItemState( SDRATTR_TEXT_MINFRAMEHEIGHT ) == SfxItemState::SET )
                        {
                            pObj->SetMergedItem(pNewSet->Get(SDRATTR_TEXT_MINFRAMEHEIGHT));
                        }

                        if ( pNewSet && pNewSet->GetItemState( SDRATTR_TEXT_AUTOGROWHEIGHT ) == SfxItemState::SET )
                        {
                            pObj->SetMergedItem(pNewSet->Get(SDRATTR_TEXT_AUTOGROWHEIGHT));
                        }

                        if( pUserCall )
                            pObj->SetUserCall( pUserCall );
                    }
                }
            }

            pSet.reset();
            Cancel();
        }
        break;

        case SID_DELETE_SNAPITEM:
        {
            SdrPageView* pPV;
            Point   aMPos = GetActiveWindow()->PixelToLogic( maMousePos );
            sal_uInt16  nHitLog = static_cast<sal_uInt16>(GetActiveWindow()->PixelToLogic( Size(
                FuPoor::HITPIX, 0 ) ).Width());
            sal_uInt16  nHelpLine;

            if( mpDrawView->PickHelpLine( aMPos, nHitLog, *GetActiveWindow()->GetOutDev(), nHelpLine, pPV) )
            {
                pPV->DeleteHelpLine( nHelpLine );
            }
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_DELETE_PAGE:
        case SID_DELETE_MASTER_PAGE:
            DeleteActualPage();
            Cancel();
            rReq.Ignore ();
        break;

        case SID_DELETE_LAYER:
            DeleteActualLayer();
            Cancel();
            rReq.Ignore ();
        break;

        case SID_ORIGINAL_SIZE:
            mpDrawView->SetMarkedOriginalSize();
            Cancel();
            rReq.Done();
        break;

        case SID_DRAW_FONTWORK:
        case SID_DRAW_FONTWORK_VERTICAL:
        {
            svx::FontworkBar::execute(*mpView, rReq, GetViewFrame()->GetBindings());       // SJ: can be removed  (I think)
            Cancel();
            rReq.Done();
        }
        break;

        case SID_SAVE_GRAPHIC:
        {
            if( rMarkList.GetMarkCount() == 1 )
            {
                if (const SdrGrafObj* pObj = dynamic_cast<const SdrGrafObj*>(rMarkList.GetMark(0)->GetMarkedSdrObj()))
                    if (pObj->GetGraphicType() == GraphicType::Bitmap || pObj->GetGraphicType() == GraphicType::GdiMetafile)
                    {
                        if (comphelper::COKit::isActive())
                        {
                            const GraphicObject& aGraphicObject(pObj->GetGraphicObject());
                            OUString sTempFileURL = GraphicHelper::ExportGraphicToTempFile(
                                aGraphicObject.GetGraphic(), u"");
                            if (!sTempFileURL.isEmpty())
                            {
                                SfxViewShell* pViewShell = SfxViewShell::Current();
                                if (pViewShell)
                                    pViewShell->viewCallback(
                                        COKitCallbackType::EXPORT_FILE, sTempFileURL.toUtf8());
                            }
                        }
                        else
                        {
                            weld::Window* pFrame = GetFrameWeld();
                            GraphicAttr aGraphicAttr = pObj->GetGraphicAttr();
                            short nState = RET_CANCEL;
                            if (aGraphicAttr != GraphicAttr()) // the image has been modified
                            {
                                if (pFrame)
                                {
                                    nState = GraphicHelper::HasToSaveTransformedImage(pFrame);
                                }
                            }
                            else
                            {
                                nState = RET_NO;
                            }

                            if (nState == RET_YES)
                            {
                                GraphicHelper::ExportGraphic(pFrame, pObj->GetTransformedGraphic(), u""_ustr);
                            }
                            else if (nState == RET_NO)
                            {
                                const GraphicObject& aGraphicObject(pObj->GetGraphicObject());
                                GraphicHelper::ExportGraphic(pFrame, aGraphicObject.GetGraphic(), u""_ustr);
                            }
                        }
                    }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_EXTERNAL_EDIT:
        {
            if( rMarkList.GetMarkCount() == 1 )
            {
                SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                if( auto pGraphicObj = dynamic_cast<SdrGrafObj*>( pObj ) )
                    if( pGraphicObj->GetGraphicType() == GraphicType::Bitmap )
                    {
                        GraphicObject aGraphicObject( pGraphicObj->GetGraphicObject() );
                        m_ExternalEdits.push_back(
                            std::make_unique<SdrExternalToolEdit>(
                                mpDrawView.get(), pGraphicObj));
                        m_ExternalEdits.back()->Edit( &aGraphicObject );
                    }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_COMPRESS_GRAPHIC:
        {
            if( rMarkList.GetMarkCount() == 1 )
            {
                rtl::Reference<SdrObject> xObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

                if( auto pGraphicObj = dynamic_cast<SdrGrafObj*>(xObj.get()) )
                {
                    if( pGraphicObj->GetGraphicType() == GraphicType::Bitmap )
                    {
                        auto xDialog = std::make_shared<CompressGraphicsDialog>(GetFrameWeld(), pGraphicObj, GetViewFrame()->GetBindings());
                        OUString aUndoString = rMarkList.GetMarkDescription() + " Compress";
                        ::sd::View* pView = mpDrawView.get();
                        weld::DialogController::runAsync(xDialog, [pView, xObj, xDialog, aUndoString](sal_uInt32 nResult) {
                            if (nResult == RET_OK)
                            {
                                rtl::Reference<SdrGrafObj> pNewObject = xDialog->GetCompressedSdrGrafObj();
                                SdrPageView* pPageView = pView->GetSdrPageView();
                                pView->BegUndo( aUndoString );
                                pView->ReplaceObjectAtView( xObj.get(), *pPageView, pNewObject.get() );
                                pView->EndUndo();
                            }
                        });
                    }
                }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_GRAPHIC_SIZE_CHECK:
        {
            sd::GraphicSizeCheckGUIResult aResult(GetDoc());
            svx::GenericCheckDialog aDialog(GetFrameWeld(), aResult);
            aDialog.run();

            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_ATTRIBUTES_LINE:  // BASIC
        {
            SetCurrentFunction( FuLine::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            if (rReq.GetArgs())
                Cancel();
        }
        break;

        case SID_ATTRIBUTES_AREA:  // BASIC
        {
            SetCurrentFunction( FuArea::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            if (rReq.GetArgs())
                Cancel();
        }
        break;

        case SID_ATTR_TRANSFORM:
        {
            SetCurrentFunction( FuTransform::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            // tdf#138963 conditions tested for here must be the same as those
            // of the early returns from FuTransform::DoExecute
            if (rReq.GetArgs() || rMarkList.GetMarkCount() == 0)
            {
                Invalidate(SID_RULER_OBJECT);
                Cancel();
            }
        }
        break;
        case SID_MOVE_SHAPE_HANDLE:
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();
            if (pArgs && pArgs->Count () >= 3)
            {
                const SfxUInt32Item* handleNumItem = rReq.GetArg<SfxUInt32Item>(FN_PARAM_1);
                const SfxUInt32Item* newPosXTwips = rReq.GetArg<SfxUInt32Item>(FN_PARAM_2);
                const SfxUInt32Item* newPosYTwips = rReq.GetArg<SfxUInt32Item>(FN_PARAM_3);
                const SfxInt32Item* OrdNum = rReq.GetArg<SfxInt32Item>(FN_PARAM_4);
                const SfxBoolItem* pPreview = rReq.GetArg<SfxBoolItem>(FN_PARAM_5);

                const sal_uInt32 handleNum = handleNumItem->GetValue();
                const ::tools::Long newPosX = convertTwipToMm100(newPosXTwips->GetValue());
                const ::tools::Long newPosY = convertTwipToMm100(newPosYTwips->GetValue());

                mpDrawView->MoveShapeHandle(handleNum, Point(newPosX, newPosY), OrdNum ? OrdNum->GetValue() : -1,
                                            pPreview && pPreview->GetValue());
                Cancel();
            }
            break;
        }
        case SID_CHAR_DLG_EFFECT:
        case SID_CHAR_DLG:  // BASIC
        {
            SetCurrentFunction( FuChar::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_PARA_DLG:
        {
            SetCurrentFunction( FuParagraph::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case FN_NUM_BULLET_ON:
        {
            SfxUInt16Item aItem(FN_SVX_SET_BULLET, FuBulletAndPosition::BULLET_TOGGLE);
            GetViewFrame()->GetDispatcher()->ExecuteList(FN_SVX_SET_BULLET,
                    SfxCallMode::RECORD, { &aItem });
        }
        break;

        case FN_NUM_NUMBERING_ON:
        {
            SfxUInt16Item aItem(FN_SVX_SET_NUMBER, FuBulletAndPosition::BULLET_TOGGLE);
            GetViewFrame()->GetDispatcher()->ExecuteList(FN_SVX_SET_NUMBER,
                    SfxCallMode::RECORD, { &aItem });
        }
        break;

        case SID_OUTLINE_BULLET:
        case FN_SVX_SET_BULLET:
        case FN_SVX_SET_NUMBER:
        {
            SetCurrentFunction( FuBulletAndPosition::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
            SfxBindings& rBindings = GetViewFrame()->GetBindings();
            rBindings.Invalidate( FN_NUM_BULLET_ON );
            rBindings.Invalidate( FN_NUM_NUMBERING_ON );
        }
        break;

        case FN_INSERT_SOFT_HYPHEN:
        case FN_INSERT_HARDHYPHEN:
        case FN_INSERT_HARD_SPACE:
        case FN_INSERT_NNBSP:
        case SID_INSERT_RLM :
        case SID_INSERT_LRM :
        case SID_INSERT_WJ :
        case SID_INSERT_ZWSP:
        case SID_CHARMAP:
        {
            SetCurrentFunction( FuBullet::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_PRESENTATION_LAYOUT:
        {
            SetCurrentFunction( FuPresentationLayout::Create(*this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq) );
            Cancel();
        }
        break;

        case SID_PASTE_SPECIAL:
        {
            SetCurrentFunction( FuInsertClipboard::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CHANGE_PICTURE:
        case SID_INSERT_GRAPHIC:
        {
            SetCurrentFunction( FuInsertGraphic::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq,
                                                         nSId == SID_CHANGE_PICTURE ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERT_AVMEDIA:
        {
            SetCurrentFunction( FuInsertAVMedia::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERT_OBJECT:
        case SID_INSERT_FLOATINGFRAME:
        case SID_INSERT_MATH:
        case SID_INSERT_DIAGRAM:
        case SID_ATTR_TABLE:
        {
            SetCurrentFunction( FuInsertOLE::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            // Set the selection tool as the old one. This in particular important for the
            // zoom function, in which clicking without dragging zooms as well, and that
            // makes exiting the object editing mode impossible.
            if (dynamic_cast<FuSelection*>( GetOldFunction().get() ) == nullptr)
                SetOldFunction( FuSelection::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;
        case SID_CLASSIFICATION_APPLY:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem = nullptr;
            if (pArgs && pArgs->GetItemState(nSId, false, &pItem) == SfxItemState::SET)
            {
                const OUString& rName = static_cast<const SfxStringItem*>(pItem)->GetValue();
                auto eType = SfxClassificationPolicyType::IntellectualProperty;
                if (pArgs->GetItemState(SID_TYPE_NAME, false, &pItem) == SfxItemState::SET)
                {
                    const OUString& rType = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    eType = SfxClassificationHelper::stringToPolicyType(rType);
                }
                if (SfxViewFrame* pViewFrame = GetViewFrame())
                {
                    if (SfxObjectShell* pObjectShell = pViewFrame->GetObjectShell())
                    {
                        SfxClassificationHelper aHelper(pObjectShell->getDocProperties());
                        aHelper.SetBACName(rName, eType);
                    }
                }
            }
            else
                SAL_WARN("sd.ui", "missing parameter for SID_CLASSIFICATION_APPLY");

            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_CLASSIFICATION_DIALOG:
        {
            if (SfxObjectShell* pObjShell = SfxObjectShell::Current())
            {
                css::uno::Reference<css::document::XDocumentProperties> xDocProps(pObjShell->getDocProperties());
                auto xDialog = std::make_shared<svx::ClassificationDialog>(GetFrameWeld(), xDocProps, false, [](){} );
                ClassificationCollector aCollector(*this, xDocProps);
                aCollector.collect();

                xDialog->setupValues(std::vector(aCollector.getResults()));

                if (RET_OK == xDialog->run())
                {
                    ClassificationInserter aInserter(*this, xDocProps);
                    aInserter.insert(xDialog->getResult());
                }
                xDialog.reset();
            }

            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_COPYOBJECTS:
        {
            if ( mpDrawView->IsPresObjSelected(false) )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }

                SetCurrentFunction( FuCopy::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            }
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERTFILE:  // BASIC
        {
            Broadcast (ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_START));
            SetCurrentFunction( FuInsertFile::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Broadcast (ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_END));
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_SELECT_BACKGROUND:
        case SID_SAVE_BACKGROUND:
        case SID_ATTR_PAGE_SIZE:
        case SID_ATTR_PAGE:
        case SID_PAGESETUP:  // BASIC ??
        {
            SetCurrentFunction( FuPage::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore (); // we generate independent macros !!
        }
        break;

        case SID_BEFORE_OBJ:
        case SID_BEHIND_OBJ:
        {
            SetCurrentFunction( FuDisplayOrder::Create(*this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq) );
            rReq.Done();
            // finishes itself, no Cancel() needed!
        }
        break;

        case SID_REVERSE_ORDER:   // BASIC
        {
            mpDrawView->ReverseOrderOfMarked();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_ANIMATION_EFFECTS:
        {
            SetCurrentFunction( FuObjectAnimationParameters::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq) );
            Cancel();
        }
        break;

        case SID_EXECUTE_ANIMATION_EFFECT:
        {
            SetCurrentFunction(FuExecuteInteraction::Create(*this, GetActiveWindow(),
                                                            mpDrawView.get(), *GetDoc(), rReq));
            Cancel();
        }
        break;

        case SID_LINEEND_POLYGON:
        {
            SetCurrentFunction( FuLineEnd::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_CAPTUREPOINT:
            // negative value to signal call from menu
            maMousePos = Point(-1,-1);
            [[fallthrough]];
        case SID_SET_SNAPITEM:
        {
            SetCurrentFunction( FuSnapLine::Create(*this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq) );
            Cancel();
        }
        break;

        case SID_MANAGE_LINKS:
        {
            SetCurrentFunction( FuLink::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_THESAURUS:
        {
            SetCurrentFunction( FuThesaurus::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_TEXTATTR_DLG:
        {
            if (mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();
            SetCurrentFunction( FuTextAttrDlg::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_MEASURE_DLG:
        {
            SetCurrentFunction( FuMeasureDlg::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CONNECTION_DLG:
        {
            SetCurrentFunction( FuConnectionDlg::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_CONNECTION_NEW_ROUTING:
        {
            SfxItemSetFixed<SDRATTR_EDGELINE1DELTA, SDRATTR_EDGELINE3DELTA> aDefAttr( GetPool() );
            GetView()->SetAttributes( aDefAttr, true ); // (ReplaceAll)

            Cancel();
            rReq.Done();
        }
        break;

        case SID_TWAIN_SELECT:
        {
            if( mxScannerManager.is() )
            {
                try
                {
                    const cpo::uno::Sequence< css::scanner::ScannerContext >
                        aContexts( mxScannerManager->getAvailableScanners() );

                    if( aContexts.hasElements() )
                    {
                        css::scanner::ScannerContext aContext( aContexts.getConstArray()[ 0 ] );

                        Reference<lang::XInitialization> xInit(mxScannerManager, UNO_QUERY);
                        if (xInit.is())
                        {
                            //  initialize dialog
                            weld::Window* pWindow = rReq.GetFrameWeld();
                            cpo::uno::Sequence<cpo::uno::Any> aSeq(comphelper::InitAnyPropertySequence(
                            {
                                {"ParentWindow", pWindow ? cpo::uno::Any(pWindow->GetXWindow()) : cpo::uno::Any(Reference<awt::XWindow>())}
                            }));
                            xInit->initialize( aSeq );
                        }

                        mxScannerManager->configureScannerAndScan( aContext, mxScannerListener );
                    }
                }
                catch(...)
                {
                }
            }

            Cancel();
            rReq.Done();
        }
        break;

        case SID_TWAIN_TRANSFER:
        {
            bool bDone = false;

            if( mxScannerManager.is() )
            {
                try
                {
                    const cpo::uno::Sequence< css::scanner::ScannerContext > aContexts( mxScannerManager->getAvailableScanners() );

                    if( aContexts.hasElements() )
                    {
                        mxScannerManager->startScan( aContexts.getConstArray()[ 0 ], mxScannerListener );
                        bDone = true;
                    }
                }
                catch( ... )
                {
                }
            }

            if( !bDone )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
#ifndef UNX
                                                              SdResId(STR_TWAIN_NO_SOURCE)
#else
                                                              SdResId(STR_TWAIN_NO_SOURCE_UNX)
#endif
                                                              ));
                xInfoBox->run();

            }
            else
            {
                SfxBindings& rBindings = GetViewFrame()->GetBindings();
                rBindings.Invalidate( SID_TWAIN_SELECT );
                rBindings.Invalidate( SID_TWAIN_TRANSFER );
            }

            Cancel();
            rReq.Done();
        }
        break;

        case SID_POLYGON_MORPHING:
        {
            SetCurrentFunction( FuMorph::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_INSERTLAYER:
        {
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
            }

            SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
            sal_uInt16 nLayerCnt = rLayerAdmin.GetLayerCount();
            sal_uInt16 nLayer = nLayerCnt - 2 + 1;
            OUString aLayerName = SdResId(STR_LAYER) + OUString::number(nLayer);
            OUString aLayerTitle, aLayerDesc;
            bool bIsVisible = false;
            bool bIsLocked = false;
            bool bIsPrintable = false;

            const SfxItemSet* pArgs = rReq.GetArgs();

            if (! pArgs)
            {
                SfxItemSetFixed<ATTR_LAYER_START, ATTR_LAYER_END> aNewAttr( GetDoc()->GetPool() );

                aNewAttr.Put( makeSdAttrLayerName( aLayerName ) );
                aNewAttr.Put( makeSdAttrLayerTitle() );
                aNewAttr.Put( makeSdAttrLayerDesc() );
                aNewAttr.Put( makeSdAttrLayerVisible() );
                aNewAttr.Put( makeSdAttrLayerPrintable() );
                aNewAttr.Put( makeSdAttrLayerLocked() );
                aNewAttr.Put( makeSdAttrLayerThisPage() );

                SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                vcl::Window* pWin = GetActiveWindow();
                ScopedVclPtr<AbstractSdInsertLayerDlg> pDlg( pFact->CreateSdInsertLayerDlg(pWin ? pWin->GetFrameWeld() : nullptr, aNewAttr, true, SdResId(STR_INSERTLAYER)) );
                pDlg->SetHelpId( SdModule::get()->GetSlotPool()->GetSlot( SID_INSERTLAYER )->GetCommand() );

                // test for already existing names
                bool bLoop = true;
                while( bLoop && pDlg->Execute() == RET_OK )
                {
                    pDlg->GetAttr( aNewAttr );
                    aLayerName   = aNewAttr.Get(ATTR_LAYER_NAME).GetValue ();

                    if( rLayerAdmin.GetLayer( aLayerName )
                        || aLayerName.isEmpty()
                        || LayerTabBar::IsLocalizedNameOfStandardLayer( aLayerName) )
                    {
                        // name already exists
                        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetFrameWeld(),
                                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                                   SdResId(STR_WARN_NAME_DUPLICATE)));
                        xWarn->run();
                    }
                    else
                        bLoop = false;
                }
                if( bLoop ) // was canceled
                {
                    pDlg.disposeAndClear();
                    Cancel();
                    rReq.Ignore ();
                    break;
                }
                else
                {
                    aLayerTitle  = aNewAttr.Get(ATTR_LAYER_TITLE).GetValue();
                    aLayerDesc   = aNewAttr.Get(ATTR_LAYER_DESC).GetValue ();
                    bIsVisible   = static_cast<const SfxBoolItem &>( aNewAttr.Get (ATTR_LAYER_VISIBLE)).GetValue ();
                    bIsLocked    = static_cast<const SfxBoolItem &>( aNewAttr.Get (ATTR_LAYER_LOCKED)).GetValue () ;
                    bIsPrintable = static_cast<const SfxBoolItem &>( aNewAttr.Get (ATTR_LAYER_PRINTABLE)).GetValue () ;
                }
            }
            else if (pArgs->Count () != 4)
            {
#if HAVE_FEATURE_SCRIPTING
                 StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
                 Cancel();
                 rReq.Ignore ();
                 break;
            }
            else
            {
                 const SfxStringItem* pLayerName = rReq.GetArg<SfxStringItem>(ID_VAL_LAYERNAME);
                 const SfxBoolItem* pIsVisible = rReq.GetArg<SfxBoolItem>(ID_VAL_ISVISIBLE);
                 const SfxBoolItem* pIsLocked = rReq.GetArg<SfxBoolItem>(ID_VAL_ISLOCKED);
                 const SfxBoolItem* pIsPrintable = rReq.GetArg<SfxBoolItem>(ID_VAL_ISPRINTABLE);

                 aLayerName   = pLayerName->GetValue ();
                 bIsVisible   = pIsVisible->GetValue ();
                 bIsLocked    = pIsLocked->GetValue ();
                 bIsPrintable = pIsPrintable->GetValue ();
            }

            OUString aPrevLayer = mpDrawView->GetActiveLayer();
            SdrLayer* pLayer;
            sal_uInt16 nPrevLayer = 0;
            nLayerCnt = rLayerAdmin.GetLayerCount();

            for ( nLayer = 0; nLayer < nLayerCnt; nLayer++ )
            {
                pLayer = rLayerAdmin.GetLayer(nLayer);
                OUString aName = pLayer->GetName();

                if ( aPrevLayer == aName )
                {
                    nPrevLayer = std::max(nLayer, sal_uInt16(4));
                }
            }

            mpDrawView->InsertNewLayer(aLayerName, nPrevLayer + 1);
            pLayer = rLayerAdmin.GetLayer(aLayerName);
            if( pLayer )
            {
                pLayer->SetTitle( aLayerTitle );
                pLayer->SetDescription( aLayerDesc );
            }

            mpDrawView->SetLayerVisible( aLayerName, bIsVisible );
            mpDrawView->SetLayerLocked( aLayerName, bIsLocked);
            mpDrawView->SetLayerPrintable(aLayerName, bIsPrintable);

            mpDrawView->SetActiveLayer(aLayerName);

            ResetActualLayer();

            GetDoc()->SetChanged();

            GetViewFrame()->GetDispatcher()->Execute(SID_SWITCHLAYER,
                    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_MODIFYLAYER:
        {
            if(!GetLayerTabControl()) // #i87182#
            {
                OSL_ENSURE(false, "No LayerTabBar (!)");
                Cancel();
                rReq.Ignore();
                break;
            }

            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
            }

            SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
            sal_uInt16 nCurPage = GetLayerTabControl()->GetCurPageId();
            OUString aLayerName = GetLayerTabControl()->GetLayerName(nCurPage);
            SdrLayer* pLayer = rLayerAdmin.GetLayer(aLayerName);

            OUString aLayerTitle = pLayer->GetTitle();
            OUString aLayerDesc = pLayer->GetDescription();

            OUString aOldLayerName(aLayerName);
            OUString aOldLayerTitle(aLayerTitle);
            OUString aOldLayerDesc(aLayerDesc);

            bool bIsVisible, bIsLocked, bIsPrintable;
            bool bOldIsVisible = bIsVisible = mpDrawView->IsLayerVisible(aLayerName);
            bool bOldIsLocked = bIsLocked = mpDrawView->IsLayerLocked(aLayerName);
            bool bOldIsPrintable = bIsPrintable = mpDrawView->IsLayerPrintable(aLayerName);

            const SfxItemSet* pArgs = rReq.GetArgs();
            // is it allowed to delete the layer?
            bool bDelete = !( LayerTabBar::IsRealNameOfStandardLayer(aLayerName) );

            if (! pArgs)
            {
                SfxItemSetFixed<ATTR_LAYER_START, ATTR_LAYER_END> aNewAttr( GetDoc()->GetPool() );

                aNewAttr.Put( makeSdAttrLayerName( aLayerName ) );
                aNewAttr.Put( makeSdAttrLayerTitle( aLayerTitle ) );
                aNewAttr.Put( makeSdAttrLayerDesc( aLayerDesc ) );
                aNewAttr.Put( makeSdAttrLayerVisible( bIsVisible ) );
                aNewAttr.Put( makeSdAttrLayerLocked( bIsLocked ) );
                aNewAttr.Put( makeSdAttrLayerPrintable( bIsPrintable ) );
                aNewAttr.Put( makeSdAttrLayerThisPage() );

                SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                vcl::Window* pWin = GetActiveWindow();
                ScopedVclPtr<AbstractSdInsertLayerDlg> pDlg( pFact->CreateSdInsertLayerDlg(pWin ? pWin->GetFrameWeld() : nullptr, aNewAttr, bDelete, SdResId(STR_MODIFYLAYER)) );
                pDlg->SetHelpId( SdModule::get()->GetSlotPool()->GetSlot( SID_MODIFYLAYER )->GetCommand() );

                // test for already existing names
                bool    bLoop = true;
                sal_uInt16  nRet = 0;
                while( bLoop )
                {
                    nRet = pDlg->Execute();
                    if (nRet != RET_OK)
                        break;
                    pDlg->GetAttr( aNewAttr );
                    aLayerName   = aNewAttr.Get(ATTR_LAYER_NAME).GetValue ();
                    if (bDelete)
                    {
                        if( (rLayerAdmin.GetLayer( aLayerName ) && aLayerName != aOldLayerName)
                            || LayerTabBar::IsRealNameOfStandardLayer(aLayerName)
                            || LayerTabBar::IsLocalizedNameOfStandardLayer(aLayerName)
                            || aLayerName.isEmpty() )
                        {
                            // name already exists
                            std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetFrameWeld(),
                                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                                   SdResId(STR_WARN_NAME_DUPLICATE)));
                            xWarn->run();
                        }
                        else
                            bLoop = false;
                    }
                    else
                        bLoop = false; // altering name is already disabled in the dialog itself
                }
                switch (nRet)
                {
                    case RET_OK :
                        aLayerTitle  = aNewAttr.Get(ATTR_LAYER_TITLE).GetValue ();
                        aLayerDesc   = aNewAttr.Get(ATTR_LAYER_DESC).GetValue ();
                        bIsVisible   = static_cast<const SfxBoolItem &>( aNewAttr.Get (ATTR_LAYER_VISIBLE)).GetValue ();
                        bIsLocked    = static_cast<const SfxBoolItem &>( aNewAttr.Get (ATTR_LAYER_LOCKED)).GetValue ();
                        bIsPrintable = static_cast<const SfxBoolItem &>( aNewAttr.Get (ATTR_LAYER_PRINTABLE)).GetValue ();
                        break;

                    default :
                        pDlg.disposeAndClear();
                        rReq.Ignore ();
                        Cancel ();
                        return;
                }
            }
            else if (pArgs->Count () == 4)
            {
                const SfxStringItem* pLayerName = rReq.GetArg<SfxStringItem>(ID_VAL_LAYERNAME);
                const SfxBoolItem* pIsVisible = rReq.GetArg<SfxBoolItem>(ID_VAL_ISVISIBLE);
                const SfxBoolItem* pIsLocked = rReq.GetArg<SfxBoolItem>(ID_VAL_ISLOCKED);
                const SfxBoolItem* pIsPrintable = rReq.GetArg<SfxBoolItem>(ID_VAL_ISPRINTABLE);

                aLayerName   = pLayerName->GetValue ();
                bIsVisible   = pIsVisible->GetValue ();
                bIsLocked    = pIsLocked->GetValue ();
                bIsPrintable = pIsPrintable->GetValue ();
            }
            else
            {
#if HAVE_FEATURE_SCRIPTING
                StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
                Cancel ();
                rReq.Ignore ();
                break;
            }

            SfxUndoManager* pManager = GetDoc()->GetDocSh()->GetUndoManager();
            std::unique_ptr<SdLayerModifyUndoAction> pAction( new SdLayerModifyUndoAction(
                *GetDoc(),
                pLayer,
                // old values
                aOldLayerName,
                aOldLayerTitle,
                aOldLayerDesc,
                bOldIsVisible,
                bOldIsLocked,
                bOldIsPrintable,
                // new values
                aLayerName,
                aLayerTitle,
                aLayerDesc,
                bIsVisible,
                bIsLocked,
                bIsPrintable
                ) );
            pManager->AddUndoAction( std::move(pAction) );

            ModifyLayer( pLayer, aLayerName, aLayerTitle, aLayerDesc, bIsVisible, bIsLocked, bIsPrintable );

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_TOGGLELAYERVISIBILITY:
        {
            // tdf#113439; duplicates LayerTabBar::MouseButtonDown()
            sal_uInt16 aTabId = GetLayerTabControl()->GetCurPageId();
            OUString aName( GetLayerTabControl()->GetLayerName(aTabId) );

            SdrPageView* pPV = mpDrawView->GetSdrPageView();
            bool bVisible = !pPV->IsLayerVisible(aName);

            pPV->SetLayerVisible(aName, bVisible);

            ResetActualLayer();
            GetDoc()->SetChanged();

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_RENAMELAYER:
        {
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
            }

            if(GetLayerTabControl()) // #i87182#
            {
                GetLayerTabControl()->StartEditMode(GetLayerTabControl()->GetCurPageId());
            }
            else
            {
                OSL_ENSURE(false, "No LayerTabBar (!)");
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_EDIT_HYPERLINK :
        {
            // Ensure the field is selected first
            OutlinerView* pOutView = mpDrawView->GetTextEditOutlinerView();
            if (pOutView)
                pOutView->SelectFieldAtCursor();

            GetViewFrame()->GetDispatcher()->Execute( SID_HYPERLINK_DIALOG );

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_INSERT_HYPERLINK :
        {
            GetViewFrame()->GetDispatcher()->Execute( SID_HYPERLINK_DIALOG );

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OPEN_HYPERLINK:
        {
            OutlinerView* pOutView = mpDrawView->GetTextEditOutlinerView();
            if ( pOutView )
            {
                const SvxFieldItem* pFieldItem
                    = pOutView->GetFieldAtSelection(/*AlsoCheckBeforeCursor=*/true);
                const SvxFieldData* pField = pFieldItem ? pFieldItem->GetField() : nullptr;
                if( auto pURLField = dynamic_cast< const SvxURLField *>( pField ) )
                {
                    SfxStringItem aUrl( SID_FILE_NAME, pURLField->GetURL() );
                    SfxStringItem aTarget( SID_TARGETNAME, pURLField->GetTargetFrame() );

                    OUString aReferName;
                    SfxViewFrame* pFrame = GetViewFrame();
                    SfxMedium* pMed = pFrame->GetObjectShell()->GetMedium();
                    if (pMed)
                        aReferName = pMed->GetName();

                    SfxFrameItem aFrm( SID_DOCFRAME, pFrame );
                    SfxStringItem aReferer( SID_REFERER, aReferName );

                    SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, false );
                    SfxBoolItem aBrowsing( SID_BROWSE, true );

                    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
                    {
                        pViewFrm->GetDispatcher()->ExecuteList(SID_OPENDOC,
                            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                            { &aUrl, &aTarget, &aFrm, &aReferer,
                                &aNewView, &aBrowsing });
                    }
                }
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_COPY_HYPERLINK_LOCATION:
        {
            OutlinerView* pOutView = mpDrawView->GetTextEditOutlinerView();
            if ( pOutView )
            {
                const SvxFieldItem* pFieldItem
                    = pOutView->GetFieldAtSelection(/*AlsoCheckBeforeCursor=*/true);
                const SvxFieldData* pField = pFieldItem ? pFieldItem->GetField() : nullptr;
                if (const SvxURLField* pURLField = dynamic_cast<const SvxURLField*>(pField))
                {
                    uno::Reference<datatransfer::clipboard::XClipboard> xClipboard
                        = pOutView->GetWindow()->GetClipboard();

                    vcl::unohelper::TextDataObject::CopyStringTo(pURLField->GetURL(), xClipboard, SfxViewShell::Current());
                }
            }

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_HYPERLINK_SETLINK:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if (pReqArgs)
            {
                const SvxHyperlinkItem* pHLItem =
                    &pReqArgs->Get(SID_HYPERLINK_SETLINK);

                if (pHLItem->GetInsertMode() == HLINK_FIELD)
                {
                    InsertURLField(pHLItem->GetURL(), pHLItem->GetName(),
                                   pHLItem->GetTargetFrame(), pHLItem->GetIntName());
                }
                else if (pHLItem->GetInsertMode() == HLINK_BUTTON)
                {
                    InsertURLButton(pHLItem->GetURL(), pHLItem->GetName(),
                                    pHLItem->GetTargetFrame(), nullptr);
                }
                else if (pHLItem->GetInsertMode() == HLINK_DEFAULT)
                {
                    OutlinerView* pOlView = mpDrawView->GetTextEditOutlinerView();
                    // When the provided text is just a hint, prefer the selection over it; if both
                    // are missing, use the URL.
                    OUString sLinkText;
                    if (pHLItem->GetTextIsHint() && pOlView && pOlView->HasSelection())
                        sLinkText = pOlView->GetSelected();
                    else if (!pHLItem->GetName().isEmpty())
                        sLinkText = pHLItem->GetName();
                    else
                        sLinkText = pHLItem->GetURL();

                    if (pOlView || comphelper::COKit::isActive())
                    {
                        InsertURLField(pHLItem->GetURL(), sLinkText,
                                       pHLItem->GetTargetFrame(), pHLItem->GetIntName());
                    }
                    else
                    {
                        InsertURLButton(pHLItem->GetURL(), pHLItem->GetName(),
                                        pHLItem->GetTargetFrame(), nullptr);
                    }
                }
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_HIDE_LAST_LEVEL:
        {
            ESelection aSel;
            // fdo#78151 editing a PresObjKind::Outline in a master page ?
            ::Outliner* pOL = GetOutlinerForMasterPageOutlineTextObj(aSel);
            if (pOL)
            {
                //we are on the last paragraph
                aSel.Adjust();
                if (aSel.end.nPara == pOL->GetParagraphCount() - 1)
                {
                    sal_uInt16 nDepth = pOL->GetDepth(aSel.end.nPara);
                    //there exists a previous numbering level
                    if (nDepth != sal_uInt16(-1) && nDepth > 0)
                    {
                        Paragraph* pPara = pOL->GetParagraph(aSel.end.nPara);
                        pOL->Remove(pPara, 1);
                    }
                }
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_SHOW_NEXT_LEVEL:
        {
            const TranslateId STR_PRESOBJ_MPOUTLINE_ARY[]
            {
                STR_PRESOBJ_MPOUTLINE,
                STR_PRESOBJ_MPOUTLLAYER2,
                STR_PRESOBJ_MPOUTLLAYER3,
                STR_PRESOBJ_MPOUTLLAYER4,
                STR_PRESOBJ_MPOUTLLAYER5,
                STR_PRESOBJ_MPOUTLLAYER6,
                STR_PRESOBJ_MPOUTLLAYER7,
                STR_PRESOBJ_MPNOTESTITLE,
                STR_PRESOBJ_MPNOTESTEXT,
                STR_PRESOBJ_NOTESTEXT
            };

            ESelection aSel;
            // fdo#78151 editing a PresObjKind::Outline in a master page ?
            ::Outliner* pOL = GetOutlinerForMasterPageOutlineTextObj(aSel);
            if (pOL)
            {
                //we are on the last paragraph
                aSel.Adjust();
                if (aSel.end.nPara == pOL->GetParagraphCount() - 1)
                {
                    sal_uInt16 nDepth = pOL->GetDepth(aSel.end.nPara);
                    //there exists a previous numbering level
                    if (nDepth < 8)
                    {
                        sal_uInt16 nNewDepth = nDepth+1;
                        pOL->Insert(SdResId(STR_PRESOBJ_MPOUTLINE_ARY[nNewDepth]), EE_PARA_MAX, nNewDepth);
                    }
                }
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_INSERT_FLD_DATE_FIX:
        case SID_INSERT_FLD_DATE_VAR:
        case SID_INSERT_FLD_TIME_FIX:
        case SID_INSERT_FLD_TIME_VAR:
        case SID_INSERT_FLD_AUTHOR:
        case SID_INSERT_FLD_PAGE:
        case SID_INSERT_FLD_PAGE_TITLE:
        case SID_INSERT_FLD_PAGES:
        case SID_INSERT_FLD_FILE:
        {
            sal_uInt16 nMul = 1;
            std::unique_ptr<SvxFieldItem> pFieldItem;

            switch( nSId )
            {
                case SID_INSERT_FLD_DATE_FIX:
                    pFieldItem.reset(new SvxFieldItem(
                        SvxDateField( Date( Date::SYSTEM ), SvxDateType::Fix ), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_DATE_VAR:
                    pFieldItem.reset(new SvxFieldItem( SvxDateField(), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_TIME_FIX:
                    pFieldItem.reset(new SvxFieldItem(
                        SvxExtTimeField( ::tools::Time( ::tools::Time::SYSTEM ), SvxTimeType::Fix ), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_TIME_VAR:
                    pFieldItem.reset(new SvxFieldItem( SvxExtTimeField(), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_AUTHOR:
                {
                    SvtUserOptions aUserOptions;
                    pFieldItem.reset(new SvxFieldItem(
                            SvxAuthorField(
                                aUserOptions.GetFirstName(), aUserOptions.GetLastName(), aUserOptions.GetID() ), EE_FEATURE_FIELD ));
                }
                break;

                case SID_INSERT_FLD_PAGE:
                {
                    pFieldItem.reset(new SvxFieldItem( SvxPageField(), EE_FEATURE_FIELD ));
                    nMul = 3;
                }
                break;

                case SID_INSERT_FLD_PAGE_TITLE:
                {
                    pFieldItem.reset(new SvxFieldItem( SvxPageTitleField(), EE_FEATURE_FIELD));
                    nMul = 3;
                }
                break;

                case SID_INSERT_FLD_PAGES:
                {
                    pFieldItem.reset(new SvxFieldItem( SvxPagesField(), EE_FEATURE_FIELD ));
                    nMul = 3;
                }
                break;

                case SID_INSERT_FLD_FILE:
                {
                    OUString aName;
                    if( GetDocSh()->HasName() )
                        aName = GetDocSh()->GetMedium()->GetName();
                    pFieldItem.reset(new SvxFieldItem( SvxExtFileField( aName ), EE_FEATURE_FIELD ));
                }
                break;
            }

            OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();

            if( pOLV )
            {
                const SvxFieldItem* pOldFldItem = pOLV->GetFieldAtSelection();

                if( pOldFldItem && ( nullptr != dynamic_cast< const SvxURLField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxDateField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxTimeField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxExtTimeField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxExtFileField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxAuthorField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxPageField *>( pOldFldItem->GetField() ) ) )
                {
                    // select field, then it will be deleted when inserting
                    ESelection aSel = pOLV->GetSelection();
                    if (aSel.start.nIndex == aSel.end.nIndex)
                        aSel.end.nIndex++;
                    pOLV->SetSelection( aSel );
                }

                if( pFieldItem )
                    pOLV->InsertField( *pFieldItem );
            }
            else
            {
                Outliner* pOutl = GetDoc()->GetInternalOutliner();
                pOutl->Init( OutlinerMode::TextObject );
                OutlinerMode nOutlMode = pOutl->GetOutlinerMode();
                pOutl->SetStyleSheet( 0, nullptr );
                pOutl->QuickInsertField( *pFieldItem, ESelection() );
                std::optional<OutlinerParaObject> pOutlParaObject = pOutl->CreateParaObject();

                rtl::Reference<SdrRectObj> pRectObj = new SdrRectObj(
                    *GetDoc(), ::tools::Rectangle(), SdrObjKind::Text);
                pRectObj->SetMergedItem(makeSdrTextAutoGrowWidthItem(true));

                pOutl->UpdateFields();
                pOutl->SetUpdateLayout( true );
                Size aSize( pOutl->CalcTextSize() );
                aSize.setWidth( aSize.Width() * nMul );
                pOutl->SetUpdateLayout( false );

                Point aPos;
                ::tools::Rectangle aRect( aPos, GetActiveWindow()->GetOutputSizePixel() );
                aPos = aRect.Center();
                aPos = GetActiveWindow()->PixelToLogic(aPos);
                aPos.AdjustX( -(aSize.Width() / 2) );
                aPos.AdjustY( -(aSize.Height() / 2) );

                ::tools::Rectangle aLogicRect(aPos, aSize);
                pRectObj->SetLogicRect(aLogicRect);
                pRectObj->SetOutlinerParaObject( std::move(pOutlParaObject) );
                mpDrawView->InsertObjectAtView(pRectObj.get(), *mpDrawView->GetSdrPageView());
                pOutl->Init( nOutlMode );
            }

            pFieldItem.reset();

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_MODIFY_FIELD:
        {
            OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();

            if( pOLV )
            {
                const SvxFieldItem* pFldItem = pOLV->GetFieldAtSelection();

                if( pFldItem && (nullptr != dynamic_cast< const SvxDateField *>( pFldItem->GetField() ) ||
                                 nullptr != dynamic_cast< const SvxAuthorField *>( pFldItem->GetField() ) ||
                                 nullptr != dynamic_cast< const SvxExtFileField *>( pFldItem->GetField() ) ||
                                 nullptr != dynamic_cast< const SvxExtTimeField *>( pFldItem->GetField() ) ) )
                {
                    // Dialog...
                    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                    vcl::Window* pWin = GetActiveWindow();
                    ScopedVclPtr<AbstractSdModifyFieldDlg> pDlg( pFact->CreateSdModifyFieldDlg(pWin ? pWin->GetFrameWeld() : nullptr, pFldItem->GetField(), pOLV->GetAttribs() ) );
                    if( pDlg->Execute() == RET_OK )
                    {
                        // To make a correct SetAttribs() call at the utlinerView
                        // it is necessary to split the actions here
                        std::unique_ptr<SvxFieldData> pField(pDlg->GetField());
                        ESelection aSel = pOLV->GetSelection();
                        bool bSelectionWasModified(false);

                        if( pField )
                        {
                            SvxFieldItem aFieldItem( *pField, EE_FEATURE_FIELD );

                            if (aSel.start.nIndex == aSel.end.nIndex)
                            {
                                bSelectionWasModified = true;
                                aSel.end.nIndex++;
                                pOLV->SetSelection( aSel );
                            }

                            pOLV->InsertField( aFieldItem );

                            // select again for eventual SetAttribs call
                            pOLV->SetSelection( aSel );
                        }

                        SfxItemSet aSet( pDlg->GetItemSet() );

                        if( aSet.Count() )
                        {
                            pOLV->SetAttribs( aSet );

                            ::Outliner& rOutliner = pOLV->GetOutliner();
                            rOutliner.UpdateFields();
                        }

                        if(pField)
                        {
                            // restore selection to original
                            if(bSelectionWasModified)
                            {
                                aSel.end.nIndex--;
                                pOLV->SetSelection( aSel );
                            }
                        }
                    }
                }
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_OPEN_XML_FILTERSETTINGS:
        {
            try
            {
                css::uno::Reference < css::ui::dialogs::XExecutableDialog > xDialog = css::ui::dialogs::XSLTFilterDialog::create( ::comphelper::getProcessComponentContext() );
                xDialog->execute();
            }
            catch( css::uno::RuntimeException& )
            {
                DBG_UNHANDLED_EXCEPTION("sd.view");
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_GROUP:  // BASIC
        {
            if ( mpDrawView->IsPresObjSelected( true, true, true ) )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                mpDrawView->GroupMarked();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_UNGROUP:  // BASIC
        {
            mpDrawView->UnGroupMarked();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_NAME_GROUP:
        {
            // only allow for single object selection since the name of an object needs
            // to be unique
            if(1 == rMarkList.GetMarkCount())
            {
                // #i68101#
                rtl::Reference<SdrObject> pSelected = rMarkList.GetMark(0)->GetMarkedSdrObj();
                OSL_ENSURE(pSelected, "DrawViewShell::FuTemp03: nMarkCount, but no object (!)");
                OUString aName(pSelected->GetName());

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                VclPtr<AbstractSvxObjectNameDialog> pDlg(pFact->CreateSvxObjectNameDialog(GetFrameWeld(), aName));

                pDlg->SetCheckNameHdl(LINK(this, DrawViewShell, NameObjectHdl));

                pDlg->StartExecuteAsync(
                    [this, pDlg, pSelected] (sal_Int32 nResult)->void
                    {
                        if (nResult == RET_OK)
                        {
                            pSelected->SetName(pDlg->GetName());

                            SdPage* pPage = GetActualPage();
                            if (pPage)
                                pPage->notifyObjectRenamed(pSelected.get());
                        }
                        pDlg->disposeOnce();
                        SfxBindings& rBindings = GetViewFrame()->GetBindings();
                        rBindings.Invalidate( SID_NAVIGATOR_STATE, true );
                        rBindings.Invalidate( SID_CONTEXT );
                    }
                );
            }

            Cancel();
            rReq.Ignore();
            break;
        }

        // #i68101#
        case SID_OBJECT_TITLE_DESCRIPTION:
        {
            if(1 == rMarkList.GetMarkCount())
            {
                rtl::Reference<SdrObject> pSelected = rMarkList.GetMark(0)->GetMarkedSdrObj();
                OSL_ENSURE(pSelected, "DrawViewShell::FuTemp03: nMarkCount, but no object (!)");
                OUString aTitle(pSelected->GetTitle());
                OUString aDescription(pSelected->GetDescription());
                bool isDecorative(pSelected->IsDecorative());

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                VclPtr<AbstractSvxObjectTitleDescDialog> pDlg(pFact->CreateSvxObjectTitleDescDialog(
                            GetFrameWeld(), aTitle, aDescription, isDecorative));

                pDlg->StartExecuteAsync(
                    [this, pDlg, pSelected] (sal_Int32 nResult)->void
                    {
                        if (nResult == RET_OK)
                        {
                            pSelected->SetTitle(pDlg->GetTitle());
                            pSelected->SetDescription(pDlg->GetDescription());
                            pSelected->SetDecorative(pDlg->IsDecorative());
                        }
                        pDlg->disposeOnce();
                        SfxBindings& rBindings = GetViewFrame()->GetBindings();
                        rBindings.Invalidate( SID_NAVIGATOR_STATE, true );
                        rBindings.Invalidate( SID_CONTEXT );
                    }
                );
            }

            Cancel();
            rReq.Ignore();
            break;
        }

        case SID_SETLAYER:
        {
            const size_t nMarkCount = rMarkList.GetMarkCount();
            if (nMarkCount >= 1 && mpLayerTabBar)
            {
                SdSelectLayerDlg aDlg(GetFrameWeld());

                weld::TreeView& rTreeView = aDlg.GetTreeView();
                auto nPageCount = mpLayerTabBar->GetPageCount();
                for (auto i = 0; i < nPageCount; i++)
                    rTreeView.append_text(LayerTabBar::convertToLocalizedName(
                                        mpLayerTabBar->GetLayerName(mpLayerTabBar->GetPageId(i))));
                rTreeView.select(0);

                if (aDlg.run() == RET_OK && rTreeView.get_selected_index() != -1)
                {
                    SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
                    SdrLayerID aSdrLayerId = rLayerAdmin.GetLayerID(mpLayerTabBar->GetLayerName(
                                        mpLayerTabBar->GetPageId(rTreeView.get_selected_index())));
                    for (size_t i = 0; i < nMarkCount; ++i)
                    {
                        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
                        pObj->SetLayer(aSdrLayerId);
                    }
                }
            }
            Cancel();
            rReq.Ignore();
            break;
        }

        case SID_ENTER_GROUP:  // BASIC
        {
            mpDrawView->EnterMarkedGroup();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_LEAVE_GROUP:  // BASIC
        {
            mpDrawView->LeaveOneGroup();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_LEAVE_ALL_GROUPS:  // BASIC
        {
            mpDrawView->LeaveAllGroup();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_TEXT_COMBINE:  // BASIC
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->CombineMarkedTextObjects();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_COMBINE:  // BASIC
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->CombineMarkedObjects(false);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_DISTRIBUTE_HLEFT:
        case SID_DISTRIBUTE_HCENTER:
        case SID_DISTRIBUTE_HDISTANCE:
        case SID_DISTRIBUTE_HRIGHT:
        case SID_DISTRIBUTE_VTOP:
        case SID_DISTRIBUTE_VCENTER:
        case SID_DISTRIBUTE_VDISTANCE:
        case SID_DISTRIBUTE_VBOTTOM:
        {
            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                mpDrawView->DistributeMarkedObjects(nSId);
            }
            Cancel();
            rReq.Done ();
        }
        break;
        case SID_POLY_MERGE:
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->MergeMarkedObjects(SdrMergeMode::Merge);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_POLY_SUBSTRACT:
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->MergeMarkedObjects(SdrMergeMode::Subtract);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_POLY_INTERSECT:
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->MergeMarkedObjects(SdrMergeMode::Intersect);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_EQUALIZEWIDTH:
        case SID_EQUALIZEHEIGHT:
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            mpDrawView->EqualizeMarkedObjects(nSId == SID_EQUALIZEWIDTH);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_DISMANTLE:  // BASIC
        {
            if ( mpDrawView->IsDismantlePossible() )
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->DismantleMarkedObjects();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_CONNECT:  // BASIC
        {
            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->CombineMarkedObjects();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_BREAK:  // BASIC
        {
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
            }

            if ( mpDrawView->IsBreak3DObjPossible() )
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->Break3DObj();
            }
            else if ( mpDrawView->IsDismantlePossible(true) )
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->DismantleMarkedObjects(true);
            }
            else if ( mpDrawView->IsImportMtfPossible() )
            {
                weld::WaitObject aWait(GetFrameWeld());
                const size_t nCnt=rMarkList.GetMarkCount();

                // determine the sum of meta objects of all selected meta files
                sal_uLong nCount = 0;
                for(size_t nm=0; nm<nCnt; ++nm)
                {
                    SdrMark*     pM=rMarkList.GetMark(nm);
                    SdrObject*   pObj=pM->GetMarkedSdrObj();
                    SdrGrafObj*  pGraf= dynamic_cast< SdrGrafObj *>( pObj );
                    SdrOle2Obj*  pOle2= dynamic_cast< SdrOle2Obj *>( pObj );

                    if (pGraf != nullptr)
                    {
                        if (pGraf->HasGDIMetaFile())
                        {
                            nCount += pGraf->GetGraphic().GetGDIMetaFile().GetActionSize();
                        }
                        else if (pGraf->isEmbeddedVectorGraphicData())
                        {
                            nCount += pGraf->getMetafileFromEmbeddedVectorGraphicData().GetActionSize();
                        }
                    }

                    if (pOle2)
                        if (const Graphic* pGraphic = pOle2->GetGraphic())
                            nCount += pGraphic->GetGDIMetaFile().GetActionSize();
                }

                // decide with the sum of all meta objects if we should show a dialog
                if(nCount < MIN_ACTIONS_FOR_DIALOG)
                {
                    // nope, no dialog
                    mpDrawView->DoImportMarkedMtf();
                }
                else
                {
                    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                    ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateBreakDlg(GetFrameWeld(), mpDrawView.get(), GetDocSh(), nCount, static_cast<sal_uLong>(nCnt) ));
                    pDlg->Execute();
                }
            }

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_CONVERT_TO_3D:
        {
            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                if (mpDrawView->IsConvertTo3DObjPossible())
                {
                    if (mpDrawView->IsTextEdit())
                    {
                        mpDrawView->SdrEndTextEdit();
                    }

                    weld::WaitObject aWait(GetFrameWeld());
                    mpDrawView->ConvertMarkedObjTo3D();
                }
            }

            Cancel();
            rReq.Done();
        }
        break;

        case SID_FRAME_TO_TOP:  // BASIC
        {
            mpDrawView->PutMarkedToTop();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_MOREFRONT:  // BASIC
        case SID_FRAME_UP:  // BASIC
        {
            mpDrawView->MovMarkedToTop();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_MOREBACK:  // BASIC
        case SID_FRAME_DOWN:  // BASIC
        {
            mpDrawView->MovMarkedToBtm();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_FRAME_TO_BOTTOM:   // BASIC
        {
            mpDrawView->PutMarkedToBtm();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_HORIZONTAL:  // BASIC
        case SID_FLIP_HORIZONTAL:
        {
            mpDrawView->MirrorAllMarkedHorizontal();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_VERTICAL:  // BASIC
        case SID_FLIP_VERTICAL:
        {
            mpDrawView->MirrorAllMarkedVertical();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_LEFT:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SdrHorAlign::Left, SdrVertAlign::NONE);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_CENTER:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SdrHorAlign::Center, SdrVertAlign::NONE);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_RIGHT:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SdrHorAlign::Right, SdrVertAlign::NONE);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_UP:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SdrHorAlign::NONE, SdrVertAlign::Top);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_MIDDLE:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SdrHorAlign::NONE, SdrVertAlign::Center);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_ALIGN_PAGE:
        {
            mpDrawView->AlignMarkedObjects(SdrHorAlign::Center, SdrVertAlign::Center);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_DOWN:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SdrHorAlign::NONE, SdrVertAlign::Bottom);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_SELECTALL:  // BASIC
        {
            if( (dynamic_cast<FuSelection*>( GetOldFunction().get() ) != nullptr) &&
                !GetView()->IsFrameDragSingles() && GetView()->HasMarkablePoints())
            {
                if ( !mpDrawView->IsAction() )
                    mpDrawView->MarkAllPoints();
            }
            else
                mpDrawView->SelectAll();

            FreshNavigatrTree();

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_STYLE_NEW: // BASIC ???
        case SID_STYLE_APPLY:
        case SID_STYLE_EDIT:
        case SID_STYLE_DELETE:
        case SID_STYLE_HIDE:
        case SID_STYLE_SHOW:
        case SID_STYLE_FAMILY:
        case SID_STYLE_WATERCAN:
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            if (!rReq.GetArgs()
                && (nSId == SID_STYLE_EDIT || nSId == SID_STYLE_UPDATE_BY_EXAMPLE
                     || nSId == SID_STYLE_NEW_BY_EXAMPLE))
            {
                SfxStyleSheet* pStyleSheet = mpDrawView->GetStyleSheet();
                if( pStyleSheet && pStyleSheet->GetFamily() == SfxStyleFamily::Page)
                    pStyleSheet = static_cast<SdStyleSheet*>(pStyleSheet)->GetPseudoStyleSheet();

                if( (pStyleSheet == nullptr) && GetView()->IsTextEdit() )
                {
                    GetView()->SdrEndTextEdit();

                    pStyleSheet = mpDrawView->GetStyleSheet();
                    if(pStyleSheet && pStyleSheet->GetFamily() == SfxStyleFamily::Page)
                        pStyleSheet = static_cast<SdStyleSheet*>(pStyleSheet)->GetPseudoStyleSheet();
                }

                if( pStyleSheet == nullptr )
                {
                    rReq.Ignore();
                    break;
                }

                SfxAllItemSet aSet(GetDoc()->GetPool());

                aSet.Put(SfxUInt16Item(SID_STYLE_FAMILY,
                                       static_cast<sal_uInt16>(pStyleSheet->GetFamily())));

                if (nSId == SID_STYLE_NEW_BY_EXAMPLE)
                {
                    weld::Window* pWindow = GetViewFrame()->GetFrameWeld();
                    std::shared_ptr<SfxNewStyleDlg> xDlg(
                        new SfxNewStyleDlg(pWindow, *pStyleSheet->GetPool(),
                                           pStyleSheet->GetFamily()));
                    OUString aStyleName = pStyleSheet->GetName();
                    weld::GenericDialogController::runAsync(
                        xDlg, [this, xDlg, aSet, aStyleName, nSId](sal_Int32 nResult) mutable {
                            if (nResult == RET_OK)
                            {
                                aSet.Put(SfxStringItem(SID_STYLE_NEW_BY_EXAMPLE,
                                                       xDlg->GetName()));
                                aSet.Put(SfxStringItem(SID_STYLE_REFERENCE, aStyleName));
                                SfxRequest aReq(*GetViewFrame(), nSId);
                                aReq.SetArgs(aSet);
                                SetCurrentFunction(FuTemplate::Create(
                                    *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), aReq));
                            }
                            Cancel();
                        });
                    rReq.Ignore();
                    break;
                }

                aSet.Put(SfxStringItem(nSId, pStyleSheet->GetName()));

                rReq.SetArgs(aSet);
            }

            if( rReq.GetArgs() )
            {
                SetCurrentFunction( FuTemplate::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
                if( rReq.GetSlot() == SID_STYLE_APPLY )
                    GetViewFrame()->GetBindings().Invalidate( SID_STYLE_APPLY );
                Cancel();
            }
            else if( rReq.GetSlot() == SID_STYLE_APPLY )
                GetViewFrame()->GetDispatcher()->Execute( SID_STYLE_DESIGNER, SfxCallMode::ASYNCHRON );
            rReq.Ignore ();
        }
        break;

        case SID_IMAP:
        {
            sal_uInt16      nId = SvxIMapDlgChildWindow::GetChildWindowId();

            GetViewFrame()->ToggleChildWindow( nId );
            GetViewFrame()->GetBindings().Invalidate( SID_IMAP );

            if ( GetViewFrame()->HasChildWindow( nId )
                && ( ( ViewShell::Implementation::GetImageMapDialog() ) != nullptr ) )
            {
                if ( rMarkList.GetMarkCount() == 1 )
                    UpdateIMapDlg( rMarkList.GetMark( 0 )->GetMarkedSdrObj() );
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_GRID_FRONT:
        {
            mpDrawView->SetGridFront( !mpDrawView->IsGridFront() );
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_HELPLINES_FRONT:
        {
            mpDrawView->SetHlplFront( !mpDrawView->IsHlplFront() );
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_FONTWORK:
        {
            if ( rReq.GetArgs() )
            {
                GetViewFrame()->SetChildWindow(SvxFontWorkChildWindow::GetChildWindowId(),
                                        static_cast<const SfxBoolItem&>(rReq.GetArgs()->
                                        Get(SID_FONTWORK)).GetValue());
            }
            else
            {
                GetViewFrame()->ToggleChildWindow( SvxFontWorkChildWindow::GetChildWindowId() );
            }

            GetViewFrame()->GetBindings().Invalidate(SID_FONTWORK);
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_COLOR_CONTROL:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow(SvxColorChildWindow::GetChildWindowId(),
                                        rReq.GetArgs()->Get(SID_COLOR_CONTROL).GetValue());
            else
                GetViewFrame()->ToggleChildWindow(SvxColorChildWindow::GetChildWindowId() );

            GetViewFrame()->GetBindings().Invalidate(SID_COLOR_CONTROL);
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_EXTRUSION_TOGGLE:
        case SID_EXTRUSION_TILT_DOWN:
        case SID_EXTRUSION_TILT_UP:
        case SID_EXTRUSION_TILT_LEFT:
        case SID_EXTRUSION_TILT_RIGHT:
        case SID_EXTRUSION_3D_COLOR:
        case SID_EXTRUSION_DEPTH:
        case SID_EXTRUSION_DIRECTION:
        case SID_EXTRUSION_PROJECTION:
        case SID_EXTRUSION_LIGHTING_DIRECTION:
        case SID_EXTRUSION_LIGHTING_INTENSITY:
        case SID_EXTRUSION_SURFACE:
        case SID_EXTRUSION_DEPTH_FLOATER:
        case SID_EXTRUSION_DIRECTION_FLOATER:
        case SID_EXTRUSION_LIGHTING_FLOATER:
        case SID_EXTRUSION_SURFACE_FLOATER:
        case SID_EXTRUSION_DEPTH_DIALOG:
            svx::ExtrusionBar::execute( mpDrawView.get(), rReq, GetViewFrame()->GetBindings() );
            Cancel();
            rReq.Ignore ();
            break;

        case SID_FONTWORK_SHAPE:
        case SID_FONTWORK_SHAPE_TYPE:
        case SID_FONTWORK_ALIGNMENT:
        case SID_FONTWORK_SAME_LETTER_HEIGHTS:
        case SID_FONTWORK_CHARACTER_SPACING:
        case SID_FONTWORK_KERN_CHARACTER_PAIRS:
        case SID_FONTWORK_GALLERY_FLOATER:
        case SID_FONTWORK_CHARACTER_SPACING_FLOATER:
        case SID_FONTWORK_ALIGNMENT_FLOATER:
        case SID_FONTWORK_CHARACTER_SPACING_DIALOG:
            svx::FontworkBar::execute(*mpDrawView, rReq, GetViewFrame()->GetBindings());
            Cancel();
            rReq.Ignore ();
            break;

        case SID_BMPMASK:
        {
            GetViewFrame()->ToggleChildWindow( SvxBmpMaskChildWindow::GetChildWindowId() );
            GetViewFrame()->GetBindings().Invalidate( SID_BMPMASK );

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_NAVIGATOR:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow(SID_NAVIGATOR,
                                        static_cast<const SfxBoolItem&>(rReq.GetArgs()->
                                        Get(SID_NAVIGATOR)).GetValue());
            else
                GetViewFrame()->ToggleChildWindow( SID_NAVIGATOR );

            GetViewFrame()->GetBindings().Invalidate(SID_NAVIGATOR);
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_SLIDE_TRANSITIONS_PANEL:
        case SID_MASTER_SLIDES_PANEL:
        case SID_CUSTOM_ANIMATION_PANEL:
        case SID_GALLERY:
        {
            // First make sure that the sidebar is visible
            GetViewFrame()->ShowChildWindow(SID_SIDEBAR);

            OUString panelId;
            if (nSId == SID_CUSTOM_ANIMATION_PANEL)
                panelId = u"SdCustomAnimationPanel"_ustr;
            else if (nSId == SID_GALLERY)
                panelId = u"GalleryPanel"_ustr;
            else if (nSId == SID_SLIDE_TRANSITIONS_PANEL)
                panelId = u"SdSlideTransitionPanel"_ustr;
            else if (nSId == SID_MASTER_SLIDES_PANEL)
                panelId = u"SdAllMasterPagesPanel"_ustr;

            ::sfx2::sidebar::Sidebar::TogglePanel(
                panelId,
                GetViewFrame()->GetFrame().GetFrameInterface());

            Cancel();
            rReq.Done();
        }
        break;

        case SID_ANIMATION_OBJECTS:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow(
                    AnimationChildWindow::GetChildWindowId(),
                    static_cast<const SfxBoolItem&>(rReq.GetArgs()->
                        Get(SID_ANIMATION_OBJECTS)).GetValue());
            else
                GetViewFrame()->ToggleChildWindow(
                    AnimationChildWindow::GetChildWindowId() );

            GetViewFrame()->GetBindings().Invalidate(SID_ANIMATION_OBJECTS);
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_3D_WIN:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow( Svx3DChildWindow::GetChildWindowId(),
                                        static_cast<const SfxBoolItem&>(rReq.GetArgs()->
                                        Get( SID_3D_WIN )).GetValue());
            else
                GetViewFrame()->ToggleChildWindow( Svx3DChildWindow::GetChildWindowId() );

            GetViewFrame()->GetBindings().Invalidate( SID_3D_WIN );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CONVERT_TO_3D_LATHE_FAST:
        {
            /* The call is enough. The initialization via Start3DCreation and
               CreateMirrorPolygons is no longer needed if the parameter
               true is provided. Then a tilted rotary body with an axis left
               besides the bounding rectangle of the selected objects is drawn
               immediately and without user interaction.  */
            mpDrawView->SdrEndTextEdit();
            if(GetActiveWindow())
                GetActiveWindow()->EnterWait();
            mpDrawView->End3DCreation(true);
            Cancel();
            rReq.Ignore();
            if(GetActiveWindow())
                GetActiveWindow()->LeaveWait();
        }
        break;

        case SID_PRESENTATION_DLG:
        {
            SetCurrentFunction( FuSlideShowDlg::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_REMOTE_DLG:
        {
#ifdef ENABLE_SDREMOTE
             SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
             ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateRemoteDialog(GetFrameWeld()));
             pDlg->Execute();
#endif
        }
        break;

        case SID_CUSTOMSHOW_DLG:
        {
            SetCurrentFunction( FuCustomShowDlg::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_EXPAND_PAGE:
        {
            SetCurrentFunction( FuExpandPage::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_SUMMARY_PAGE:
        {
            mpDrawView->SdrEndTextEdit();
            SetCurrentFunction( FuSummaryPage::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            Cancel();
        }
        break;

#if HAVE_FEATURE_AVMEDIA
        case SID_AVMEDIA_PLAYER:
        {
            GetViewFrame()->ToggleChildWindow( ::avmedia::MediaPlayer::GetChildWindowId() );
            GetViewFrame()->GetBindings().Invalidate( SID_AVMEDIA_PLAYER );
            Cancel();
            rReq.Ignore ();
        }
        break;
#endif

        case SID_PRESENTATION_MINIMIZER:
        {
            const Reference<XComponentContext>& xContext(::comphelper::getProcessComponentContext());
            Reference<util::XURLTransformer> xParser(util::URLTransformer::create(xContext));
            Reference<frame::XDispatchProvider> xProvider(GetViewShellBase().GetController()->getFrame(), UNO_QUERY);
            if (xProvider.is())
            {
                util::URL aURL;
                aURL.Complete = u"vnd.com.sun.star.comp.PresentationMinimizer:execute"_ustr;
                xParser->parseStrict(aURL);
                uno::Reference<frame::XDispatch> xDispatch(xProvider->queryDispatch(aURL, OUString(), 0));
                if (xDispatch.is())
                {
                    xDispatch->dispatch(aURL, cpo::uno::Sequence< beans::PropertyValue >());
                }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_DISPLAY_MASTER_BACKGROUND:
        case SID_DISPLAY_MASTER_OBJECTS:
        {
            // Determine current page and toggle visibility of layers
            // associated with master page background or master page shapes.
            // FIXME: This solution is wrong, because shapes of master pages need
            // not be on layer "background" or "backgroundobjects".
            // See tdf#118613
            SdPage* pPage = GetActualPage();
            if (pPage != nullptr
                && GetDoc() != nullptr)
            {
                SdrLayerIDSet aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
                SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
                SdrLayerID aLayerId;
                if (nSId == SID_DISPLAY_MASTER_BACKGROUND)
                    aLayerId = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
                else
                    aLayerId = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);
                aVisibleLayers.Set(aLayerId, !aVisibleLayers.IsSet(aLayerId));
                pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
            }
            Cancel();
            rReq.Done(); // Mark task as done to auto-update the state of each buttons tdf#132816
        }
        break;

        case SID_PHOTOALBUM:
        {
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            vcl::Window* pWin = GetActiveWindow();
            ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateSdPhotoAlbumDialog(
                pWin ? pWin->GetFrameWeld() : nullptr,
                *GetDoc()));

            pDlg->Execute();
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERT_QRCODE:
        case SID_EDIT_QRCODE:
        {
            VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
            const uno::Reference<frame::XModel> xModel = GetViewShellBase().GetController()->getModel();
            VclPtr<AbstractQrCodeGenDialog> pDlg(pFact->CreateQrCodeGenDialog(
                GetFrameWeld(), xModel, rReq.GetSlot() == SID_EDIT_QRCODE));
            pDlg->StartExecuteAsync([pDlg](sal_Int32) {
                pDlg->disposeOnce();
            });
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_THEME_DIALOG:
        {
            SdrPage* pMasterPage = &GetActualPage()->TRG_GetMasterPage();
            auto pTheme = pMasterPage->getSdrPageProperties().getTheme();
            auto pDialog = std::make_shared<svx::ThemeDialog>(GetFrameWeld(), pTheme.get());
            auto* pDocShell = GetDocSh();
            weld::DialogController::runAsync(pDialog, [pDialog, pMasterPage, pDocShell](sal_uInt32 nResult)
            {
                if (RET_OK != nResult)
                    return;

                auto pColorSet = pDialog->getCurrentColorSet();
                if (pColorSet)
                {
                    sd::ThemeColorChanger aChanger(pMasterPage, pDocShell);
                    aChanger.apply(pColorSet);
                }
            });

            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_ADD_THEME:
        {
            // Create empty color set as starting point for new theme
            auto pCurrentColorSet = std::make_shared<model::ColorSet>(OUString());

            // Open ThemeColorEditDialog to create/edit the new color set
            auto pSubDialog = std::make_shared<svx::ThemeColorEditDialog>(GetFrameWeld(), *pCurrentColorSet);

            weld::DialogController::runAsync(pSubDialog, [pSubDialog](sal_uInt32 nResult) {
                if (nResult != RET_OK)
                    return;

                auto aColorSet = pSubDialog->getColorSet();
                if (!aColorSet.getName().isEmpty())
                {
                    // Add the new color set to the global collection
                    svx::ColorSets::get().insert(aColorSet);
                }
            });

            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_APPLY_THEME:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            if (pArgs)
            {
                const SfxPoolItem* pItem;
                if (pArgs->GetItemState(FN_PARAM_1, true, &pItem) == SfxItemState::SET)
                {
                    OUString aThemeName = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    auto pColorSet = svx::ColorSets::get().getColorSet(aThemeName);

                    if (pColorSet)
                    {
                        SdrPage* pMasterPage = &GetActualPage()->TRG_GetMasterPage();
                        auto* pDocShell = GetDocSh();

                        auto pSharedColorSet = std::shared_ptr<model::ColorSet>(new model::ColorSet(*pColorSet));
                        sd::ThemeColorChanger aChanger(pMasterPage, pDocShell);
                        aChanger.apply(pSharedColorSet);
                    }
                }
            }

            Cancel();
            rReq.Done();
        }
        break;

        case SID_ATTR_GLOW_COLOR:
        case SID_ATTR_GLOW_RADIUS:
        case SID_ATTR_GLOW_TRANSPARENCY:
        case SID_ATTR_GLOW_TEXT_COLOR:
        case SID_ATTR_GLOW_TEXT_RADIUS:
        case SID_ATTR_GLOW_TEXT_TRANSPARENCY:
        case SID_ATTR_SOFTEDGE_RADIUS:
        case SID_ATTR_TEXTCOLUMNS_NUMBER:
        case SID_ATTR_TEXTCOLUMNS_SPACING:
        case SID_ATTR_TEXT_AUTOGROWWIDTH:
        case SID_ATTR_TEXT_AUTOGROWHEIGHT:
        case SID_ATTR_TEXT_LEFTDIST:
        case SID_ATTR_TEXT_RIGHTDIST:
        case SID_ATTR_TEXT_UPPERDIST:
        case SID_ATTR_TEXT_LOWERDIST:
            if (const SfxItemSet* pNewArgs = rReq.GetArgs())
                mpDrawView->SetAttributes(*pNewArgs);
            rReq.Done();
            Cancel();
            break;

        case SID_PASTE_SLIDE:
        case SID_COPY_SLIDE:
        {
            sd::slidesorter::SlideSorterViewShell::GetSlideSorter(GetViewShellBase())
                ->GetSlideSorter()
                .GetController()
                .FuSupport(rReq);
            Cancel();
            rReq.Done();
        }
        break;
        default:
        {
            SAL_WARN( "sd.ui", "Slot without function" );
            Cancel();
            rReq.Ignore ();
        }
        break;
    }

    if(HasCurrentFunction())
    {
        GetCurrentFunction()->Activate();
    }
}

void DrawViewShell::ExecChar( SfxRequest &rReq )
{
    SdDrawDocument* pDoc = GetDoc();
    if (!pDoc || !mpDrawView)
        return;

    SfxItemSet aEditAttr( pDoc->GetPool() );
    mpDrawView->GetAttributes( aEditAttr );

    //modified by wj for sym2_1580, if put old itemset into new set,
    //when mpDrawView->SetAttributes(aNewAttr) it will invalidate all the item
    // and use old attr to update all the attributes
//  SfxItemSet aNewAttr( GetPool(),
//  EE_ITEMS_START, EE_ITEMS_END );
//  aNewAttr.Put( aEditAttr, false );
    SfxItemSet aNewAttr( pDoc->GetPool() );
    //modified end

    sal_uInt16 nSId = rReq.GetSlot();

    switch ( nSId )
    {
    case SID_ATTR_CHAR_FONT:
        if( rReq.GetArgs() )
        {
            const SvxFontItem* pItem = rReq.GetArg(SID_ATTR_CHAR_FONT);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_FONTHEIGHT:
        if( rReq.GetArgs() )
        {
            const SvxFontHeightItem* pItem = rReq.GetArg(SID_ATTR_CHAR_FONTHEIGHT);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_WEIGHT:
        if( rReq.GetArgs() )
        {
            const SvxWeightItem* pItem = rReq.GetArg(SID_ATTR_CHAR_WEIGHT);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_POSTURE:
        if( rReq.GetArgs() )
        {
            const SvxPostureItem* pItem = rReq.GetArg(SID_ATTR_CHAR_POSTURE);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_UNDERLINE:
        if( rReq.GetArgs() )
        {
            const SvxUnderlineItem* pItem = rReq.GetArg(SID_ATTR_CHAR_UNDERLINE);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
            else
            {
                FontLineStyle eFU = aEditAttr.Get( EE_CHAR_UNDERLINE ).GetLineStyle();
                aNewAttr.Put( SvxUnderlineItem( eFU != LINESTYLE_NONE ?LINESTYLE_NONE : LINESTYLE_SINGLE,  EE_CHAR_UNDERLINE ) );
            }
        }
        break;
    case SID_ATTR_CHAR_OVERLINE:
        if( rReq.GetArgs() )
        {
            const SvxOverlineItem* pItem = rReq.GetArg(SID_ATTR_CHAR_OVERLINE);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
            else
            {
                FontLineStyle eFU = aEditAttr.Get( EE_CHAR_OVERLINE ).GetLineStyle();
                aNewAttr.Put( SvxOverlineItem( eFU != LINESTYLE_NONE ?LINESTYLE_NONE : LINESTYLE_SINGLE,  EE_CHAR_OVERLINE ) );
            }
        }
        break;

        case SID_ULINE_VAL_NONE:
        {
            aNewAttr.Put(SvxUnderlineItem(LINESTYLE_NONE, EE_CHAR_UNDERLINE));
            break;
        }

        case SID_ULINE_VAL_SINGLE:
        case SID_ULINE_VAL_DOUBLE:
        case SID_ULINE_VAL_DOTTED:
        {
            FontLineStyle eOld = aEditAttr.Get(EE_CHAR_UNDERLINE).GetLineStyle();
            FontLineStyle eNew = eOld;

            switch (nSId)
            {
                case SID_ULINE_VAL_SINGLE:
                    eNew = ( eOld == LINESTYLE_SINGLE ) ? LINESTYLE_NONE : LINESTYLE_SINGLE;
                    break;
                case SID_ULINE_VAL_DOUBLE:
                    eNew = ( eOld == LINESTYLE_DOUBLE ) ? LINESTYLE_NONE : LINESTYLE_DOUBLE;
                    break;
                case SID_ULINE_VAL_DOTTED:
                    eNew = ( eOld == LINESTYLE_DOTTED ) ? LINESTYLE_NONE : LINESTYLE_DOTTED;
                    break;
            }

            SvxUnderlineItem aUnderline(eNew, EE_CHAR_UNDERLINE);
            aNewAttr.Put(aUnderline);
        }
        break;

    case SID_ATTR_CHAR_SHADOWED:
        if( rReq.GetArgs() )
        {
            const SvxShadowedItem* pItem = rReq.GetArg(SID_ATTR_CHAR_SHADOWED);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_CONTOUR:
        if( rReq.GetArgs() )
        {
            const SvxContourItem* pItem = rReq.GetArg(SID_ATTR_CHAR_CONTOUR);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;

    case SID_ATTR_CHAR_STRIKEOUT:
        if( rReq.GetArgs() )
        {
            const SvxCrossedOutItem* pItem = rReq.GetArg(SID_ATTR_CHAR_STRIKEOUT);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_COLOR:
        if( rReq.GetArgs() )
        {
            const SvxColorItem* pItem = rReq.GetArg(SID_ATTR_CHAR_COLOR);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_KERNING:
        if( rReq.GetArgs() )
        {
            const SvxKerningItem* pItem = rReq.GetArg(SID_ATTR_CHAR_KERNING);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_CASEMAP:
        if( rReq.GetArgs() )
        {
            const SvxCaseMapItem* pItem = rReq.GetArg(SID_ATTR_CHAR_CASEMAP);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_SET_SMALL_CAPS:
        {
            SvxCaseMap eCaseMap = aEditAttr.Get(EE_CHAR_CASEMAP).GetCaseMap();
            if (eCaseMap == SvxCaseMap::SmallCaps)
                eCaseMap = SvxCaseMap::NotMapped;
            else
                eCaseMap = SvxCaseMap::SmallCaps;
            SvxCaseMapItem aItem(eCaseMap, EE_CHAR_CASEMAP);
            aNewAttr.Put(aItem);
        }
        break;
    case SID_SET_SUB_SCRIPT:
        {
            SvxEscapementItem aItem( EE_CHAR_ESCAPEMENT );
            SvxEscapement eEsc = aEditAttr.Get(EE_CHAR_ESCAPEMENT).GetEscapement();
            if( eEsc == SvxEscapement::Subscript )
                aItem.SetEscapement( SvxEscapement::Off );
            else
                aItem.SetEscapement( SvxEscapement::Subscript );
            aNewAttr.Put( aItem );
        }
        break;
    case SID_SET_SUPER_SCRIPT:
        {
            SvxEscapementItem aItem( EE_CHAR_ESCAPEMENT );
            SvxEscapement eEsc = aEditAttr.Get(EE_CHAR_ESCAPEMENT).GetEscapement();
            if( eEsc == SvxEscapement::Superscript )
                aItem.SetEscapement( SvxEscapement::Off );
            else
                aItem.SetEscapement( SvxEscapement::Superscript );
            aNewAttr.Put( aItem );
        }
        break;
    case SID_SHRINK_FONT_SIZE:
    case SID_GROW_FONT_SIZE:
        {
            const SvxFontListItem* pFonts = GetDocSh()->GetItem( SID_ATTR_CHAR_FONTLIST );
            const FontList* pFontList = pFonts ? pFonts->GetFontList() : nullptr;
            if( pFontList )
            {
                FuText::ChangeFontSize( nSId == SID_GROW_FONT_SIZE, nullptr, pFontList, mpView );
                GetViewFrame()->GetBindings().Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
            }
            break;
        }
    case SID_ATTR_CHAR_BACK_COLOR:
        if( rReq.GetArgs() )
        {
            const SvxColorItem* pItem = rReq.GetArg<SvxColorItem>(SID_ATTR_CHAR_BACK_COLOR);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    default:
        break;
    }

    mpDrawView->SetAttributes(aNewAttr);
    rReq.Done();
    Cancel();
}

/** This method consists basically of three parts:
    1. Process the arguments of the SFX request.
    2. Use the model to create a new page or duplicate an existing one.
    3. Update the tab control and switch to the new page.
*/
SdPage* DrawViewShell::CreateOrDuplicatePage (
    SfxRequest& rRequest,
    PageKind ePageKind,
    SdPage* pPage,
    const sal_Int32 nInsertPosition)
{
    SdPage* pNewPage = nullptr;
    if (ePageKind == PageKind::Standard && meEditMode != EditMode::MasterPage)
    {
        if ( mpDrawView->IsTextEdit() )
        {
            mpDrawView->SdrEndTextEdit();
        }
        pNewPage = ViewShell::CreateOrDuplicatePage (rRequest, ePageKind, pPage, nInsertPosition);
    }
    return pNewPage;
}

void DrawViewShell::DuplicateSelectedSlides (SfxRequest& rRequest)
{
    ::sd::slidesorter::SlideSorterViewShell* pSlideSorterViewShell =
        sd::slidesorter::SlideSorterViewShell::GetSlideSorter(GetViewShellBase());
    if (!pSlideSorterViewShell)
        return;

    // Create a list of the pages that are to be duplicated.  The process of
    // duplication alters the selection.
    sal_Int32 nInsertPosition (0);
    ::std::vector<SdPage*> aPagesToDuplicate;
    sd::slidesorter::SlideSorter &rSlideSorter = pSlideSorterViewShell->GetSlideSorter();
    sd::slidesorter::model::PageEnumeration aSelectedPages (
        sd::slidesorter::model::PageEnumerationProvider::CreateSelectedPagesEnumeration(rSlideSorter.GetModel()));
    while (aSelectedPages.HasMoreElements())
    {
        sd::slidesorter::model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
        if (pDescriptor && pDescriptor->GetPage())
        {
            aPagesToDuplicate.push_back(pDescriptor->GetPage());
            nInsertPosition = pDescriptor->GetPage()->GetPageNum()+2;
        }
    }

    // Duplicate the pages in aPagesToDuplicate and collect the newly
    // created pages in aPagesToSelect.
    const bool bUndo (aPagesToDuplicate.size()>1 && rSlideSorter.GetView().IsUndoEnabled());
    if (bUndo)
        rSlideSorter.GetView().BegUndo(SdResId(STR_INSERTPAGE));

    ::std::vector<SdPage*> aPagesToSelect;
    for(::std::vector<SdPage*>::const_iterator
            iPage(aPagesToDuplicate.begin()),
            iEnd(aPagesToDuplicate.end());
        iPage!=iEnd;
        ++iPage, nInsertPosition+=2)
    {
        aPagesToSelect.push_back(
            rSlideSorter.GetViewShell().CreateOrDuplicatePage(
                rRequest, PageKind::Standard, *iPage, nInsertPosition));
    }
    aPagesToDuplicate.clear();

    if (bUndo)
        rSlideSorter.GetView().EndUndo();

    // Set the selection to the pages in aPagesToSelect.
    sd::slidesorter::controller::PageSelector& rSelector (rSlideSorter.GetController().GetPageSelector());
    rSelector.DeselectAllPages();
    for (auto const& it: aPagesToSelect)
    {
        rSelector.SelectPage(it);
    }
}

void DrawViewShell::ExecutePropPanelAttr (SfxRequest const & rReq)
{
    SdDrawDocument* pDoc = GetDoc();
    if (!pDoc || !mpDrawView)
        return;

    sal_uInt16 nSId = rReq.GetSlot();
    SfxItemSet aAttrs( pDoc->GetPool() );

    switch ( nSId )
    {
    case SID_TABLE_VERT_NONE:
    case SID_TABLE_VERT_CENTER:
    case SID_TABLE_VERT_BOTTOM:
        SdrTextVertAdjust eTVA = SDRTEXTVERTADJUST_TOP;
        if (nSId == SID_TABLE_VERT_CENTER)
            eTVA = SDRTEXTVERTADJUST_CENTER;
        else if (nSId == SID_TABLE_VERT_BOTTOM)
            eTVA = SDRTEXTVERTADJUST_BOTTOM;

        aAttrs.Put( SdrTextVertAdjustItem(eTVA) );
        mpDrawView->SetAttributes(aAttrs);

        break;
    }
}

void DrawViewShell::GetStatePropPanelAttr(SfxItemSet& rSet)
{
    SfxWhichIter    aIter( rSet );
    sal_uInt16          nWhich = aIter.FirstWhich();

    SdDrawDocument* pDoc = GetDoc();
    if (!pDoc || !mpDrawView)
        return;

    SfxItemSet aAttrs( pDoc->GetPool() );
    mpDrawView->GetAttributes( aAttrs );

    while ( nWhich )
    {
        sal_uInt16 nSlotId = SfxItemPool::IsWhich(nWhich)
            ? GetPool().GetSlotId(nWhich)
            : nWhich;
        switch ( nSlotId )
        {
            case SID_TABLE_VERT_NONE:
            case SID_TABLE_VERT_CENTER:
            case SID_TABLE_VERT_BOTTOM:
                bool bContour = false;
                SfxItemState eConState = aAttrs.GetItemState( SDRATTR_TEXT_CONTOURFRAME );
                if( eConState != SfxItemState::INVALID )
                {
                    bContour = aAttrs.Get( SDRATTR_TEXT_CONTOURFRAME ).GetValue();
                }
                if (bContour) break;

                SfxItemState eVState = aAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST );
                //SfxItemState eHState = aAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST );

                //if(SfxItemState::INVALID != eVState && SfxItemState::INVALID != eHState)
                if(SfxItemState::INVALID != eVState)
                {
                    SdrTextVertAdjust eTVA = aAttrs.Get(SDRATTR_TEXT_VERTADJUST).GetValue();
                    bool bSet = (nSlotId == SID_TABLE_VERT_NONE && eTVA == SDRTEXTVERTADJUST_TOP) ||
                            (nSlotId == SID_TABLE_VERT_CENTER && eTVA == SDRTEXTVERTADJUST_CENTER) ||
                            (nSlotId == SID_TABLE_VERT_BOTTOM && eTVA == SDRTEXTVERTADJUST_BOTTOM);
                    rSet.Put(SfxBoolItem(nSlotId, bSet));
                }
                else
                {
                    rSet.Put(SfxBoolItem(nSlotId, false));
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
