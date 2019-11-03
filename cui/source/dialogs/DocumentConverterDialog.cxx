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
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>

#include <algorithm>
#include <atomic>
#include <set>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>
#include <salhelper/thread.hxx>
#include <sfx2/docfilt.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/confignode.hxx>
#include <vcl/svapp.hxx>

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
    for (const auto& name : aClassLogicalNames)
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
OUString getFilterName(const OUString& id) { return id.getToken(1, '\x01'); }

} // namespace

class FileListUpdaterThread : public salhelper::Thread
{
public:
    FileListUpdaterThread(std::vector<OUString>& files,
                          const css::uno::Reference<css::awt::XWindow>& xWindow,
                          const Link<void*, void>& rLink)
        : salhelper::Thread("FileListUpdaterThread")
        , m_rFiles(files)
        , m_xContext(comphelper::getProcessComponentContext())
        , m_aLink(rLink)
    {
        css::uno::Reference<css::task::XInteractionHandler> xInteractionHandler(
            css::task::InteractionHandler::createWithParent(m_xContext, xWindow),
            css::uno::UNO_QUERY_THROW);
        m_xEnv.set(new ucbhelper::CommandEnvironment(
            xInteractionHandler, css::uno::Reference<css::ucb::XProgressHandler>()));
    }

    void process(const OUString& dir, const OUString& glob, bool recurse)
    {
        assert(!isRunning());
        m_sRootDir = dir;
        m_aWC.setGlob(glob);
        m_bRecurse = recurse;
        m_bCancelled = false;

        launch();
    }

    void cancel()
    {
        if (!isRunning())
            return;
        m_bCancelled = true;
        join();
    }

private:
    virtual void execute() override
    {
        if (m_sRootDir.isEmpty())
            return;
        EnumDir(m_sRootDir);
    }

    void EnumDir(const OUString& dir);

    std::vector<OUString>& m_rFiles;
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    css::uno::Reference<css::ucb::XCommandEnvironment> m_xEnv;
    Link<void*, void> m_aLink;
    OUString m_sRootDir;
    WildCard m_aWC = WildCard("", ';');
    bool m_bRecurse = false;
    std::atomic<bool> m_bCancelled = false;
};

void FileListUpdaterThread::EnumDir(const OUString& dir)
{
    ucbhelper::Content aFolder(dir, m_xEnv, m_xContext);
    if (auto xResultSet = aFolder.createCursor({ "TargetURL", "IsFolder", "IsHidden" }))
    {
        css::uno::Reference<css::sdbc::XRow> xRow(xResultSet, css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::ucb::XContentAccess> xContentAccess(xResultSet,
                                                                     css::uno::UNO_QUERY_THROW);
        while (!m_bCancelled && xResultSet->next())
        {
            const bool bIsHidden = xRow->getBoolean(3);
            if (bIsHidden)
                continue;
            OUString sUrl = xRow->getString(1);
            if (sUrl.isEmpty())
                sUrl = xContentAccess->queryContentIdentifierString();
            const bool bIsFolder = xRow->getBoolean(2);
            if (bIsFolder)
            {
                if (m_bRecurse)
                    EnumDir(sUrl);
            }
            else if (m_aWC.Matches(sUrl))
            {
                m_rFiles.push_back(sUrl);
                Application::PostUserEvent(m_aLink, new std::pair<OUString, OUString>(sUrl, dir));
            }
        }
    }
}

DocumentConverterDialog::DocumentConverterDialog(weld::Widget* pParent)
    : GenericDialogController(pParent, "cui/ui/documentconverterdialog.ui",
                              "DocumentConverterDialog")
    , m_xOk(m_xBuilder->weld_button("ok"))
    , m_xImportDir(m_xBuilder->weld_entry("Load_from_field"))
    , m_xIncludeSubdirs(m_xBuilder->weld_check_button("Include_subdirs_check"))
    , m_xImportDocTypes(m_xBuilder->weld_combo_box("Select_type_load_dropdown"))
    , m_xImportMaskLabel(m_xBuilder->weld_label("Set_up_file_mask_label"))
    , m_xImportMask(m_xBuilder->weld_entry("Set_up_mask_field"))
    , m_xExportDir(m_xBuilder->weld_entry("Save_to_field"))
    , m_xExportDocTypes(m_xBuilder->weld_combo_box("Select_type_save_dropdown"))
    , m_xSelectImportDir(m_xBuilder->weld_button("Select_load_directory_button"))
    , m_xSelectExportDir(m_xBuilder->weld_button("Select_save_directory_button"))
    , m_xSelectedFiles(m_xBuilder->weld_tree_view("fileList"))
    , m_pFileListUpdater(
          new FileListUpdaterThread(m_aSelectedFiles, getDialog()->GetXWindow(),
                                    LINK(this, DocumentConverterDialog, AddFileToList)))
{
    // See sfx2/source/dialog/filtergrouping.cxx

    auto xFactory = comphelper::getProcessServiceFactory();
    css::uno::Reference<css::container::XContainerQuery> xFilterCont(
        xFactory->createInstance("com.sun.star.document.FilterFactory"), css::uno::UNO_QUERY);
    if (!xFilterCont.is())
        return;

    css::uno::Reference<css::container::XEnumeration> xFilterList;
    try
    {
        // create the list of filters
        OUString sQuery = "getSortedFilterList():module=:iflags=0:eflags="
                          + OUString::number(sal_Int32(SfxFilterFlags::NOTINFILEDLG));
        xFilterList = xFilterCont->createSubSetEnumerationByQuery(sQuery);
    }
    catch (const css::uno::Exception&)
    {
        SAL_WARN("cui.dialogs", "Could not get filters from the configuration!");
    }

    std::set<OUString> aImportServices, aExportServices;
    while (xFilterList && xFilterList->hasMoreElements())
    {
        comphelper::SequenceAsHashMap aFilterProps(xFilterList->nextElement());
        OUString sFilterName = aFilterProps.getUnpackedValueOrDefault("Name", OUString());
        if (!sFilterName.isEmpty())
        {
            const auto pf = SfxFilter::GetFilterByName(sFilterName);
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
    m_xImportDocTypes->set_active(0);
    m_xSelectImportDir->connect_clicked(LINK(this, DocumentConverterDialog, SelectDirClick));
    m_xSelectExportDir->connect_clicked(LINK(this, DocumentConverterDialog, SelectDirClick));
    m_xImportDir->connect_changed(LINK(this, DocumentConverterDialog, DirChanged));
    m_xExportDir->connect_changed(LINK(this, DocumentConverterDialog, DirChanged));
    m_xIncludeSubdirs->connect_toggled(LINK(this, DocumentConverterDialog, IncludeSubdirsToggled));
    ImportDocTypesSelChange(*m_xImportDocTypes);
}

DocumentConverterDialog::~DocumentConverterDialog()
{
    if (m_pFileListUpdater->isRunning())
    {
        m_pFileListUpdater->cancel();
        m_pFileListUpdater->join();
    }
    m_pFileListUpdater.clear();
}

IMPL_LINK_NOARG(DocumentConverterDialog, ImportDocTypesSelChange, weld::ComboBox&, void)
{
    const OUString& id = m_xImportDocTypes->get_active_id();
    const OUString sname = getServiceName(id);

    const bool bCommonServiceEntry = getFilterName(id).isEmpty();
    m_xImportMaskLabel->set_visible(bCommonServiceEntry);
    m_xImportMask->set_visible(bCommonServiceEntry);

    if (getServiceName(m_xExportDocTypes->get_active_id()) != sname)
    {
        m_xExportDocTypes->clear();
        for (const auto& pf : m_aExportLists[sname])
        {
            m_xExportDocTypes->append(genId(sname, pf->GetFilterName()),
                                      pf->GetUIName() + " (" + pf->GetDefaultExtension() + ")");
        }
        m_xExportDocTypes->set_active(0);
    }

    UpdateFileSelection();
    UpdateControlsState();
}

IMPL_LINK(DocumentConverterDialog, SelectDirClick, weld::Button&, rButton, void)
{
    std::unique_ptr<weld::Entry>& rEntry
        = &rButton == m_xSelectImportDir.get() ? m_xImportDir : m_xExportDir;
    OUString sFolder(rEntry->get_text());
    osl::File::getFileURLFromSystemPath(sFolder, sFolder);

    const auto xContext = comphelper::getProcessComponentContext();
    const auto xFolderPicker = css::ui::dialogs::FolderPicker::create(xContext);
    xFolderPicker->setDisplayDirectory(sFolder);
    short nRet = xFolderPicker->execute();
    if (css::ui::dialogs::ExecutableDialogResults::OK != nRet)
        return;

    sFolder = xFolderPicker->getDirectory();
    osl::File::getSystemPathFromFileURL(sFolder, sFolder);
    rEntry->set_text(sFolder);
    DirChanged(*rEntry);
}

IMPL_LINK(DocumentConverterDialog, DirChanged, weld::Entry&, rEntry, void)
{
    if (&rEntry == m_xImportDir.get())
        UpdateFileSelection();
    UpdateControlsState();
}

IMPL_LINK_NOARG(DocumentConverterDialog, IncludeSubdirsToggled, weld::ToggleButton&, void)
{
    UpdateFileSelection();
}

IMPL_LINK(DocumentConverterDialog, AddFileToList, void*, pArg, void)
{
    auto pFile = static_cast<std::pair<OUString, OUString>*>(pArg);
    OUString filePath;
    osl::File::getSystemPathFromFileURL(pFile->first, filePath);
    m_xSelectedFiles->append(pFile->first, filePath);
    delete pFile;
}

void DocumentConverterDialog::UpdateControlsState()
{
    const bool bReadyToConvert = !m_xImportDir->get_text().isEmpty()
                                 && !m_xExportDir->get_text().isEmpty()
                                 && !m_xImportDocTypes->get_active_id().isEmpty()
                                 && !m_xExportDocTypes->get_active_id().isEmpty();
    m_xOk->set_sensitive(bReadyToConvert);
}

void DocumentConverterDialog::UpdateFileSelection()
{
    if (m_pFileListUpdater->isRunning())
    {
        m_pFileListUpdater->cancel();
        m_pFileListUpdater->join();
    }

    m_aSelectedFiles.clear();
    m_xSelectedFiles->clear();

    OUString sImportDir = m_xImportDir->get_text();
    if (sImportDir.isEmpty())
        return;
    const OUString& id = m_xImportDocTypes->get_active_id();
    if (id.isEmpty())
        return;

    osl::File::getFileURLFromSystemPath(sImportDir, sImportDir);
    OUString sGlob;
    if (const OUString filter = getFilterName(id); !filter.isEmpty())
    {
        const auto pf = SfxFilter::GetFilterByName(filter);
        sGlob = pf->GetWildcard().getGlob();
    }
    else
    {
        const OUString service = getServiceName(id);
        for (auto & [ sname, suiname, smask ] : m_aServiceList)
            if (sname == service)
            {
                (void)suiname;
                sGlob = smask;
                break;
            }
    }
    const bool bRecurse = m_xIncludeSubdirs->get_state() == TriState::TRISTATE_TRUE;

    m_pFileListUpdater->process(sImportDir, sGlob, bRecurse);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
