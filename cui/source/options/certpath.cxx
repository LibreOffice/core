/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <officecfg/Office/Common.hxx>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <osl/thread.h>
#include <tools/diagnose_ex.h>
#include "certpath.hxx"

#include <com/sun/star/mozilla/MozillaBootstrap.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

CertPathDialog::CertPathDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/certdialog.ui", "CertDialog")
    , m_xManualButton(m_xBuilder->weld_button("add"))
    , m_xOKButton(m_xBuilder->weld_button("ok"))
    , m_xCertPathList(m_xBuilder->weld_tree_view("paths"))
    , m_sAddDialogText(m_xBuilder->weld_label("certdir")->get_label())
    , m_sManualLabel(m_xBuilder->weld_label("manual")->get_label())
{
    m_xCertPathList->set_size_request(m_xCertPathList->get_approximate_digit_width() * 70,
                                      m_xCertPathList->get_height_rows(6));

    m_xCertPathList->enable_toggle_buttons(weld::ColumnToggleType::Radio);
    m_xCertPathList->connect_toggled(LINK(this, CertPathDialog, CheckHdl_Impl));

    m_xManualButton->connect_clicked( LINK( this, CertPathDialog, ManualHdl_Impl ) );
    m_xOKButton->connect_clicked( LINK( this, CertPathDialog, OKHdl_Impl ) );

    try
    {
        // In the reverse order of preference for the default selected profile
        mozilla::MozillaProductType const productTypes[3] = {
            mozilla::MozillaProductType_Thunderbird,
            mozilla::MozillaProductType_Firefox,
            mozilla::MozillaProductType_Mozilla };
        const char* const productNames[3] = {
            "thunderbird",
            "firefox",
            "mozilla" };
        bool bSelected = false;

        uno::Reference<mozilla::XMozillaBootstrap> xMozillaBootstrap = mozilla::MozillaBootstrap::create( comphelper::getProcessComponentContext() );

        for (sal_Int32 i = 0; i < sal_Int32(SAL_N_ELEMENTS(productTypes)); ++i)
        {
            sal_Int32 nProfileCount = xMozillaBootstrap->getProfileCount(productTypes[i]);
            if (nProfileCount <= 0)
                continue;
            OUString sDefaultProfile = xMozillaBootstrap->getDefaultProfile(productTypes[i]);
            uno::Sequence<OUString> aProfileList(nProfileCount);
#ifndef NDEBUG
            sal_Int32 nListLen =
#endif
                 xMozillaBootstrap->getProfileList(productTypes[i], aProfileList);
            assert((nProfileCount == nListLen) && (nListLen == aProfileList.getLength()));

            for (const auto& sProfileName : std::as_const(aProfileList))
            {
                OUString sEntry = OUString::createFromAscii(productNames[i]) + ":" + sProfileName;
                OUString sProfilePath = xMozillaBootstrap->getProfilePath(productTypes[i], sProfileName);
                const bool bSelectDefaultProfile = !bSelected && sProfileName == sDefaultProfile;
                AddCertPath(sEntry, sProfilePath, bSelectDefaultProfile);
                if (bSelectDefaultProfile)
                    bSelected = true;
            }
        }
    }
    catch (const uno::Exception&)
    {
    }

    try
    {
        AddManualCertPath(officecfg::Office::Common::Security::Scripting::CertDir::get().value_or(OUString()));
        if (m_sManualPath.isEmpty())
            AddManualCertPath(officecfg::Office::Common::Security::Scripting::ManualCertDir::get(), false);
    }
    catch (const uno::Exception &)
    {
        TOOLS_WARN_EXCEPTION("cui.options", "CertPathDialog::CertPathDialog()");
    }

    const char* pEnv = getenv("MOZILLA_CERTIFICATE_FOLDER");
    if (pEnv)
        AddCertPath("$MOZILLA_CERTIFICATE_FOLDER", OUString(pEnv, strlen(pEnv), osl_getThreadTextEncoding()));
}

void CertPathDialog::AddManualCertPath(const OUString& sUserSetCertPath, bool bSelect)
{
    if (sUserSetCertPath.isEmpty())
        return;

    // check existence
    ::osl::DirectoryItem aUserPathItem;
    OUString sUserSetCertURLPath;
    osl::FileBase::getFileURLFromSystemPath(sUserSetCertPath, sUserSetCertURLPath);
    if (::osl::FileBase::E_None == ::osl::DirectoryItem::get(sUserSetCertURLPath, aUserPathItem))
    {
        ::osl::FileStatus aStatus( osl_FileStatus_Mask_Validate );
        if (::osl::FileBase::E_None == aUserPathItem.getFileStatus(aStatus))
            // the cert path exists
            AddCertPath(m_sManualLabel, sUserSetCertPath, bSelect);
    }
}

IMPL_LINK_NOARG(CertPathDialog, OKHdl_Impl, weld::Button&, void)
{
    try
    {
        std::shared_ptr< comphelper::ConfigurationChanges > batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Security::Scripting::CertDir::set(
            getDirectory(), batch);
        officecfg::Office::Common::Security::Scripting::ManualCertDir::set(m_sManualPath, batch);
        batch->commit();
    }
    catch (const uno::Exception &)
    {
        TOOLS_WARN_EXCEPTION("cui.options", "CertPathDialog::OKHdl_Impl()");
    }

    m_xDialog->response(RET_OK);
}

OUString CertPathDialog::getDirectory() const
{
    int nEntry = m_xCertPathList->get_selected_index();
    if (nEntry == -1)
        return OUString();
    return m_xCertPathList->get_id(nEntry);
}

CertPathDialog::~CertPathDialog()
{
}

IMPL_LINK(CertPathDialog, CheckHdl_Impl, const weld::TreeView::iter_col&, rRowCol, void)
{
    HandleEntryChecked(m_xCertPathList->get_iter_index_in_parent(rRowCol.first));
}

void CertPathDialog::HandleEntryChecked(int nRow)
{
    const bool bChecked = m_xCertPathList->get_toggle(nRow) == TRISTATE_TRUE;
    if (bChecked)
    {
        // we have radio button behavior -> so uncheck the other entries
        m_xCertPathList->select(nRow);
        const int nCount = m_xCertPathList->n_children();
        for (int i = 0; i < nCount; ++i)
        {
            if (i != nRow)
                m_xCertPathList->set_toggle(i, TRISTATE_FALSE);
        }
    }
}

void CertPathDialog::AddCertPath(const OUString &rProfile, const OUString &rPath, const bool bSelect)
{
    int nRow = -1;
    for (int i = 0, nCount = m_xCertPathList->n_children(); i < nCount; ++i)
    {
        OUString sCertPath = m_xCertPathList->get_id(i);
        //already exists, just select the original one
        if (sCertPath == rPath)
        {
            const bool bWantSelected = bSelect || m_xCertPathList->get_toggle(i);
            m_xCertPathList->set_toggle(i, bWantSelected ? TRISTATE_TRUE : TRISTATE_FALSE);
            HandleEntryChecked(i);
            return;
        }
        else if (m_xCertPathList->get_text(i, 0) == rProfile)
            nRow = i;
    }

    if (m_sManualLabel == rProfile)
        m_sManualPath = rPath;

    if (nRow < 0)
    {
        m_xCertPathList->append();
        nRow = m_xCertPathList->n_children() - 1;
    }
    m_xCertPathList->set_toggle(nRow, bSelect ? TRISTATE_TRUE : TRISTATE_FALSE);
    m_xCertPathList->set_text(nRow, rProfile, 0);
    m_xCertPathList->set_text(nRow, rPath, 1);
    m_xCertPathList->set_id(nRow, rPath);
    HandleEntryChecked(nRow);
}

IMPL_LINK_NOARG(CertPathDialog, ManualHdl_Impl, weld::Button&, void)
{
    try
    {
        uno::Reference<ui::dialogs::XFolderPicker2> xFolderPicker = ui::dialogs::FolderPicker::create(comphelper::getProcessComponentContext());

        OUString sURL;
        if (!m_sManualPath.isEmpty())
            osl::FileBase::getFileURLFromSystemPath(m_sManualPath, sURL);
        if (sURL.isEmpty())
            osl::Security().getHomeDir(sURL);
        xFolderPicker->setDisplayDirectory(sURL);
        xFolderPicker->setDescription(m_sAddDialogText);

        if (xFolderPicker->execute() == ui::dialogs::ExecutableDialogResults::OK)
        {
            sURL = xFolderPicker->getDirectory();
            OUString aPath;
            if (osl::FileBase::E_None == osl::FileBase::getSystemPathFromFileURL(sURL, aPath))
                AddCertPath(m_sManualLabel, aPath);
        }
    }
    catch (const uno::Exception &)
    {
        TOOLS_WARN_EXCEPTION("cui.options", "");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
