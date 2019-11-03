/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DocumentConverterDialog.hxx>

#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <algorithm>
#include <set>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <sal/log.hxx>
#include <sfx2/docfilt.hxx>
#include <unotools/confignode.hxx>

namespace
{
utl::OConfigurationTreeRoot getConfigNode()
{
    return utl::OConfigurationTreeRoot::createWithComponentContext(
        comphelper::getProcessComponentContext(), "org.openoffice.Office.UI/FilterClassification",
        -1, utl::OConfigurationTreeRoot::CM_READONLY);
}

css::uno::Sequence<OUString> getGlobalClassesOrdered(const utl::OConfigurationNode& rNode)
{
    css::uno::Sequence<OUString> aGlobalClasses;
    rNode.getNodeValue("GlobalFilters/Order") >>= aGlobalClasses;
    return aGlobalClasses;
}

std::unordered_map<OUString, OUString> getClassNames(const utl::OConfigurationNode& rNode,
                                                     const OUString& sSubnode)
{
    std::unordered_map<OUString, OUString> aResult;
    const auto aClassesNode = rNode.openNode(sSubnode);
    const auto aClassLogicalNames = aClassesNode.getNodeNames();
    for (const auto name : aClassLogicalNames)
    {
        const utl::OConfigurationNode aClassDesc = aClassesNode.openNode(name);
        OUString sDisplayName;
        aClassDesc.getNodeValue("DisplayName") >>= sDisplayName;
        aResult[name] = sDisplayName;
    }
    return aResult;
}

bool FilterCompProc(const std::shared_ptr<const SfxFilter>& f1,
                    const std::shared_ptr<const SfxFilter>& f2)
{
    const bool b1default = bool(f1->GetFilterFlags() & SfxFilterFlags::DEFAULT);
    const bool b2default = bool(f2->GetFilterFlags() & SfxFilterFlags::DEFAULT);
    if (b1default != b2default)
        return b1default;

    if (f1->IsOwnFormat() != f2->IsOwnFormat())
        return f1->IsOwnFormat();
    if (f1->IsOwnFormat() && (f1->IsOwnTemplateFormat() != f2->IsOwnTemplateFormat()))
        return f2->IsOwnTemplateFormat();
    if (f1->IsAlienFormat() != f2->IsAlienFormat())
        return f2->IsAlienFormat();
    if (f1->IsExoticFormat() != f2->IsExoticFormat())
        return f2->IsExoticFormat();

    // Prefer filters that are prefered to at least something to those that are not
    const bool b1prefered = bool(f1->GetFilterFlags() & SfxFilterFlags::PREFERED);
    const bool b2prefered = bool(f2->GetFilterFlags() & SfxFilterFlags::PREFERED);
    if (b1prefered != b2prefered)
        return b1prefered;

    return f1->GetUIName() < f2->GetUIName();
}

void SortFilters(DocumentConverterDialog::FilterList& list)
{
    std::sort(list.begin(), list.end(), FilterCompProc);
}

OUString genId(const OUString& sname, const OUString& filter) { return sname + "\x01" + filter; }

OUString getServiceName(const OUString& id) { return id.getToken(0, '\x01'); }

} // namespace

DocumentConverterDialog::DocumentConverterDialog(weld::Widget* pParent)
    : GenericDialogController(pParent, "cui/ui/documentconverterdialog.ui",
                              "DocumentConverterDialog")
    , m_xImportDocTypes(m_xBuilder->weld_combo_box("Select_type_load_dropdown"))
    , m_xExportDocTypes(m_xBuilder->weld_combo_box("Select_type_save_dropdown"))
{
    // See sfx2/source/dialog/filtergrouping.cxx

    auto xFactory = comphelper::getProcessServiceFactory();
    css::uno::Reference<css::container::XContainerQuery> xFilterCont(
        xFactory->createInstance("com.sun.star.document.FilterFactory"), css::uno::UNO_QUERY);
    if (!xFilterCont.is())
        return;

    // create the list of filters
    OUString sQuery = "getSortedFilterList():module=:iflags=0:eflags="
                      + OUString::number(sal_Int32(SfxFilterFlags::NOTINFILEDLG));

    css::uno::Reference<css::container::XEnumeration> xFilterList;
    try
    {
        xFilterList = xFilterCont->createSubSetEnumerationByQuery(sQuery);
    }
    catch (const css::uno::Exception&)
    {
        SAL_WARN("sfx.dialog", "Could not get filters from the configuration!");
    }

    std::set<OUString> aImportServices, aExportServices;
    while (xFilterList && xFilterList->hasMoreElements())
    {
        comphelper::SequenceAsHashMap aFilterProps(xFilterList->nextElement());
        OUString sFilterName = aFilterProps.getUnpackedValueOrDefault("Name", OUString());
        if (!sFilterName.isEmpty())
        {
            auto pf = SfxFilter::GetFilterByName(sFilterName);
            if (pf->IsEnabled())
            {
                if (pf->CanImport())
                {
                    m_aImportLists[pf->GetServiceName()].push_back(pf);
                    aImportServices.insert(pf->GetServiceName());
                }
                if (pf->CanExport())
                {
                    m_aExportLists[pf->GetServiceName()].push_back(pf);
                    aExportServices.insert(pf->GetServiceName());
                }
            }
        }
    }

    const auto aConfigNode = getConfigNode();
    auto aClassNames = getClassNames(aConfigNode, "GlobalFilters/Classes");
    // Global names ordered according to configuration
    const auto aClassesOrdered = getGlobalClassesOrdered(aConfigNode);
    for (const auto& s : aClassesOrdered)
    {
        // Only add if both export and import filters exist, i.e. a conversion is possible
        if (aImportServices.count(s) && aExportServices.count(s))
            m_aServiceList.emplace_back(s, aClassNames[s], "");
    }
    // Now local names unordered
    for (const auto& pair : getClassNames(aConfigNode, "LocalFilters/Classes"))
    {
        // Only add if both export and import filters exist, i.e. a conversion is possible
        if (aImportServices.count(pair.first) && aExportServices.count(pair.first))
            m_aServiceList.emplace_back(pair.first, pair.second, "");
    }

    for (auto & [ sname, suiname, smask ] : m_aServiceList)
    {
        auto& rImportList = m_aImportLists[sname];
        auto& rExportList = m_aExportLists[sname];
        SortFilters(rImportList);
        SortFilters(rExportList);

        m_xImportDocTypes->append_separator(suiname);
        m_xImportDocTypes->append(sname, suiname);
        for (const auto& pf : rImportList)
        {
            m_xImportDocTypes->append(genId(sname, pf->GetFilterName()),
                                      pf->GetUIName() + " (" + pf->GetWildcard().getGlob() + ")");
            if (!smask.isEmpty())
                smask += ";";
            smask += pf->GetWildcard().getGlob();
        }
    }
    m_xImportDocTypes->connect_changed(
        LINK(this, DocumentConverterDialog, ImportDocTypesSelChange));
}

IMPL_LINK_NOARG(DocumentConverterDialog, ImportDocTypesSelChange, weld::ComboBox&, void)
{
    const OUString sname = getServiceName(m_xImportDocTypes->get_active_id());
    if (getServiceName(m_xExportDocTypes->get_active_id()) == sname)
        return;

    m_xExportDocTypes->clear();
    for (const auto& pf : m_aExportLists[sname])
    {
        m_xExportDocTypes->append(genId(sname, pf->GetFilterName()),
                                  pf->GetUIName() + " (" + pf->GetWildcard().getGlob() + ")");
    }
    m_xExportDocTypes->set_active(0);
}

short DocumentConverterDialog::run()
{
    short nRet = GenericDialogController::run();
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
