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
#include <sal/log.hxx>
#include <vcl/treelistentry.hxx>
#include <unotools/securityoptions.hxx>
#include "certpath.hxx"

#include <com/sun/star/mozilla/MozillaBootstrap.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

CertPathDialog::CertPathDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/certdialog.ui", "CertDialog")
    , m_xAddBtn(m_xBuilder->weld_button("add"))
    , m_xOKBtn(m_xBuilder->weld_button("ok"))
    , m_xCertPathList(m_xBuilder->weld_tree_view("paths"))
    , m_xAddDialogLabel(m_xBuilder->weld_label("certdir"))
    , m_xManualLabel(m_xBuilder->weld_label("manual"))
{
    m_sAddDialogText = m_xAddDialogLabel->get_label();
    m_sManual = m_xManualLabel->get_label();

    m_xCertPathList->set_size_request(m_xCertPathList->get_approximate_digit_width() * 70,
                                      m_xCertPathList->get_height_rows(6));

    std::vector<int> aWidths;
    aWidths.push_back(m_xCertPathList->get_checkbox_column_width());
    aWidths.push_back(m_xCertPathList->get_approximate_digit_width() * 20);
    m_xCertPathList->set_column_fixed_widths(aWidths);

    std::vector<int> aRadioColumns;
    aRadioColumns.push_back(0);
    m_xCertPathList->set_toggle_columns_as_radio(aRadioColumns);

    m_xCertPathList->connect_toggled(LINK(this, CertPathDialog, CheckHdl_Impl));
    m_xAddBtn->connect_clicked( LINK( this, CertPathDialog, AddHdl_Impl ) );
    m_xOKBtn->connect_clicked( LINK( this, CertPathDialog, OKHdl_Impl ) );

    try
    {
        mozilla::MozillaProductType const productTypes[3] = {
            mozilla::MozillaProductType_Thunderbird,
            mozilla::MozillaProductType_Firefox,
            mozilla::MozillaProductType_Mozilla };
        const char* const productNames[3] = {
            "thunderbird",
            "firefox",
            "mozilla" };

        uno::Reference<mozilla::XMozillaBootstrap> xMozillaBootstrap = mozilla::MozillaBootstrap::create( comphelper::getProcessComponentContext() );

        for (sal_Int32 i = 0; i < sal_Int32(SAL_N_ELEMENTS(productTypes)); ++i)
        {
            OUString profile = xMozillaBootstrap->getDefaultProfile(productTypes[i]);

            if (!profile.isEmpty())
            {
                m_xCertPathList->append();
                const int nRow = m_xCertPathList->n_children() - 1;
                m_xCertPathList->set_toggle(nRow, false, 0);
                OUString sEntry = OUString::createFromAscii(productNames[i]) + ":" + profile;
                m_xCertPathList->set_text(nRow, sEntry, 1);
                OUString sProfilePath = xMozillaBootstrap->getProfilePath( productTypes[i], profile );
                m_xCertPathList->set_text(nRow, sProfilePath, 2);
                m_xCertPathList->set_id(nRow, sProfilePath);
            }
        }
    }
    catch (const uno::Exception&)
    {
    }

    try
    {
        OUString sUserSetCertPath =
            officecfg::Office::Common::Security::Scripting::CertDir::get().get_value_or(OUString());

        if (!sUserSetCertPath.isEmpty())
            AddCertPath(m_sManual, sUserSetCertPath);
    }
    catch (const uno::Exception &e)
    {
        SAL_WARN("cui.options", "CertPathDialog::CertPathDialog(): " << e);
    }

    const char* pEnv = getenv("MOZILLA_CERTIFICATE_FOLDER");
    if (pEnv)
        AddCertPath("$MOZILLA_CERTIFICATE_FOLDER", OUString(pEnv, strlen(pEnv), osl_getThreadTextEncoding()));

    if (m_xCertPathList->n_children())
    {
        m_xCertPathList->set_toggle(0, true, 0);
        HandleEntryChecked(0);
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
        batch->commit();
    }
    catch (const uno::Exception &e)
    {
        SAL_WARN("cui.options", "CertPathDialog::OKHdl_Impl(): " << e);
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

IMPL_LINK(CertPathDialog, CheckHdl_Impl, const row_col&, rRowCol, void)
{
    HandleEntryChecked(rRowCol.first);
}

void CertPathDialog::HandleEntryChecked(int nRow)
{
    m_xCertPathList->select(nRow);
    bool bChecked = m_xCertPathList->get_toggle(nRow, 0);
    if (bChecked)
    {
        // we have radio button behavior -> so uncheck the other entries
        int nCount = m_xCertPathList->n_children();
        for (int i = 0; i < nCount; ++i)
        {
            if (i != nRow)
                m_xCertPathList->set_toggle(i, false, 0);
        }
    }
}

void CertPathDialog::AddCertPath(const OUString &rProfile, const OUString &rPath)
{
    for (int i = 0, nCount = m_xCertPathList->n_children(); i < nCount; ++i)
    {
        OUString sCertPath = m_xCertPathList->get_id(i);
        //already exists, just select the original one
        if (sCertPath == rPath)
        {
            m_xCertPathList->set_toggle(i, true, 0);
            HandleEntryChecked(i);
            return;
        }
    }

    m_xCertPathList->append();
    const int nRow = m_xCertPathList->n_children() - 1;
    m_xCertPathList->set_toggle(nRow, true, 0);
    m_xCertPathList->set_text(nRow, rProfile, 1);
    m_xCertPathList->set_text(nRow, rPath, 2);
    m_xCertPathList->set_id(nRow, rPath);
    HandleEntryChecked(nRow);
}

IMPL_LINK_NOARG(CertPathDialog, AddHdl_Impl, weld::Button&, void)
{
    try
    {
        uno::Reference<ui::dialogs::XFolderPicker2> xFolderPicker = ui::dialogs::FolderPicker::create(comphelper::getProcessComponentContext());

        OUString sURL;
        osl::Security().getHomeDir(sURL);
        xFolderPicker->setDisplayDirectory(sURL);
        xFolderPicker->setDescription(m_sAddDialogText);

        if (xFolderPicker->execute() == ui::dialogs::ExecutableDialogResults::OK)
        {
            sURL = xFolderPicker->getDirectory();
            OUString aPath;
            if (osl::FileBase::E_None == osl::FileBase::getSystemPathFromFileURL(sURL, aPath))
                AddCertPath(m_sManual, aPath);
        }
    }
    catch (uno::Exception & e)
    {
        SAL_WARN("cui.options", e);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
