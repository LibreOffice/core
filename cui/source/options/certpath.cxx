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
#include <svtools/stdctrl.hxx>
#include "svtools/treelistentry.hxx"
#include <unotools/securityoptions.hxx>
#include <cuires.hrc>
#include "certpath.hxx"
#include "dialmgr.hxx"

#include <com/sun/star/mozilla/MozillaBootstrap.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

CertPathDialog::CertPathDialog(vcl::Window* pParent)
    : ModalDialog(pParent, "CertDialog", "cui/ui/certdialog.ui")
{
    get(m_pOKBtn, "ok");
    get(m_pAddBtn, "add");
    get(m_pCertPathListContainer, "paths");
    Size aSize(LogicToPixel(Size(210, 60), MAP_APPFONT));
    m_pCertPathListContainer->set_width_request(aSize.Width());
    m_pCertPathListContainer->set_height_request(aSize.Height());
    m_pCertPathList =
        VclPtr<svx::SvxRadioButtonListBox>::Create(*m_pCertPathListContainer, 0);
    m_sAddDialogText = get<FixedText>("certdir")->GetText();
    m_sManual = get<FixedText>("manual")->GetText();

    static long aStaticTabs[]=
    {
        3, 0, 15, 75
    };

    m_pCertPathList->SvSimpleTable::SetTabs( aStaticTabs );

    OUString sProfile(get<FixedText>("profile")->GetText());
    OUString sDirectory(get<FixedText>("dir")->GetText());

    OUStringBuffer sHeader;
    sHeader.append('\t').append(sProfile).append('\t').append(sDirectory);
    m_pCertPathList->InsertHeaderEntry( sHeader.makeStringAndClear(), HEADERBAR_APPEND, HeaderBarItemBits::LEFT );
    m_pCertPathList->SetCheckButtonHdl( LINK( this, CertPathDialog, CheckHdl_Impl ) );

    m_pAddBtn->SetClickHdl( LINK( this, CertPathDialog, AddHdl_Impl ) );
    m_pOKBtn->SetClickHdl( LINK( this, CertPathDialog, OKHdl_Impl ) );

    try
    {
        mozilla::MozillaProductType productTypes[3] = {
            mozilla::MozillaProductType_Thunderbird,
            mozilla::MozillaProductType_Firefox,
            mozilla::MozillaProductType_Mozilla };
        const char* productNames[3] = {
            "thunderbird",
            "firefox",
            "mozilla" };
        sal_Int32 nProduct = SAL_N_ELEMENTS(productTypes);

        uno::Reference<mozilla::XMozillaBootstrap> xMozillaBootstrap = mozilla::MozillaBootstrap::create( comphelper::getProcessComponentContext() );

        for (sal_Int32 i = 0; i < nProduct; ++i)
        {
            OUString profile = xMozillaBootstrap->getDefaultProfile(productTypes[i]);

            if (!profile.isEmpty())
            {
                OUString sProfilePath = xMozillaBootstrap->getProfilePath( productTypes[i], profile );
                OUStringBuffer sEntry;
                sEntry.append('\t').appendAscii(productNames[i]).append(':').append(profile).append('\t').append(sProfilePath);
                SvTreeListEntry *pEntry = m_pCertPathList->InsertEntry(sEntry.makeStringAndClear());
                OUString* pCertPath = new OUString(sProfilePath);
                pEntry->SetUserData(pCertPath);
            }
        }
    }
    catch (const uno::Exception&)
    {
    }

    SvTreeListEntry *pEntry = m_pCertPathList->GetEntry(0);
    if (pEntry)
    {
        m_pCertPathList->SetCheckButtonState(pEntry, SV_BUTTON_CHECKED);
        HandleCheckEntry(pEntry);
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
        SAL_WARN("cui.options", "CertPathDialog::CertPathDialog(): caught exception" << e.Message);
    }

    const char* pEnv = getenv("MOZILLA_CERTIFICATE_FOLDER");
    if (pEnv)
        AddCertPath("$MOZILLA_CERTIFICATE_FOLDER", OUString(pEnv, strlen(pEnv), osl_getThreadTextEncoding()));
}

IMPL_LINK_NOARG_TYPED(CertPathDialog, OKHdl_Impl, Button*, void)
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
        SAL_WARN("cui.options", "CertPathDialog::OKHdl_Impl(): caught exception" << e.Message);
    }

    EndDialog(RET_OK);
}

OUString CertPathDialog::getDirectory() const
{
    SvTreeListEntry* pEntry = m_pCertPathList->FirstSelected();
    void* pCertPath = pEntry ? pEntry->GetUserData() : nullptr;
    return pCertPath ? *static_cast<OUString*>(pCertPath) : OUString();
}

CertPathDialog::~CertPathDialog()
{
    disposeOnce();
}

void CertPathDialog::dispose()
{
    SvTreeListEntry* pEntry = m_pCertPathList->First();
    while (pEntry)
    {
        OUString* pCertPath = static_cast<OUString*>(pEntry->GetUserData());
        delete pCertPath;
        pEntry = m_pCertPathList->Next( pEntry );
    }
    m_pCertPathList.disposeAndClear();
    m_pCertPathListContainer.clear();
    m_pAddBtn.clear();
    m_pOKBtn.clear();
    ModalDialog::dispose();
}

IMPL_LINK_TYPED( CertPathDialog, CheckHdl_Impl, SvTreeListBox*, pList, void )
{
    SvTreeListEntry* pEntry = pList ? m_pCertPathList->GetEntry(m_pCertPathList->GetCurMousePoint())
                                : m_pCertPathList->FirstSelected();
    if (pEntry)
        m_pCertPathList->HandleEntryChecked(pEntry);
}

void CertPathDialog::HandleCheckEntry( SvTreeListEntry* _pEntry )
{
    m_pCertPathList->Select( _pEntry );
    SvButtonState eState = m_pCertPathList->GetCheckButtonState( _pEntry );

    if (SV_BUTTON_CHECKED == eState)
    {
        // uncheck the other entries
        SvTreeListEntry* pEntry = m_pCertPathList->First();
        while (pEntry)
        {
            if (pEntry != _pEntry)
                m_pCertPathList->SetCheckButtonState(pEntry, SV_BUTTON_UNCHECKED);
            pEntry = m_pCertPathList->Next(pEntry);
        }
    }
    else
        m_pCertPathList->SetCheckButtonState(_pEntry, SV_BUTTON_CHECKED);
}

void CertPathDialog::AddCertPath(const OUString &rProfile, const OUString &rPath)
{
    SvTreeListEntry* pEntry = m_pCertPathList->First();
    while (pEntry)
    {
        OUString* pCertPath = static_cast<OUString*>(pEntry->GetUserData());
        //already exists, just select the original one
        if (pCertPath->equals(rPath))
        {
            m_pCertPathList->SetCheckButtonState(pEntry, SV_BUTTON_CHECKED);
            HandleCheckEntry(pEntry);
            return;
        }
        pEntry = m_pCertPathList->Next(pEntry);
    }

    OUStringBuffer sEntry;
    sEntry.append('\t').append(rProfile).append('\t').append(rPath);
    pEntry = m_pCertPathList->InsertEntry(sEntry.makeStringAndClear());
    OUString* pCertPath = new OUString(rPath);
    pEntry->SetUserData(pCertPath);
    m_pCertPathList->SetCheckButtonState(pEntry, SV_BUTTON_CHECKED);
    HandleCheckEntry(pEntry);
}

IMPL_LINK_NOARG_TYPED(CertPathDialog, AddHdl_Impl, Button*, void)
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
        SAL_WARN("cui.options", "caught UNO exception: " << e.Message);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
