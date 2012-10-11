/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <officecfg/Office/Common.hxx>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <svtools/stdctrl.hxx>
#include <unotools/securityoptions.hxx>
#include <cuires.hrc>
#include "certpath.hxx"
#include "certpath.hrc"
#include "dialmgr.hxx"

#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

CertPathDialog::CertPathDialog( Window* pParent ) :
     ModalDialog( pParent, CUI_RES( RID_SVXDLG_CERTPATH ) )
    , m_aCertPathFL       ( this, CUI_RES( FL_CERTPATH ) )
    , m_aCertPathFT       ( this, CUI_RES( FT_CERTPATH ) )
    , m_aCertPathListContainer( this, CUI_RES( LB_CERTPATH ) )
    , m_aCertPathList( m_aCertPathListContainer )
    , m_aAddBtn           ( this, CUI_RES( PB_ADD ) )
    , m_aButtonsFL       ( this, CUI_RES( FL_BUTTONS ) )
    , m_aOKBtn           ( this, CUI_RES( PB_OK ) )
    , m_aCancelBtn       ( this, CUI_RES( PB_CANCEL ) )
    , m_aHelpBtn         ( this, CUI_RES( PB_HELP ) )
    , m_sAddDialogText(CUI_RESSTR(STR_ADDDLGTEXT))
    , m_sManual(CUI_RESSTR(STR_MANUAL))
{
    static long aStaticTabs[]=
    {
        3, 0, 15, 75
    };

    m_aCertPathList.SvxSimpleTable::SetTabs( aStaticTabs );

    rtl::OUString sProfile(CUI_RESSTR(STR_PROFILE));
    rtl::OUString sDirectory(CUI_RESSTR(STR_DIRECTORY));

    rtl::OUStringBuffer sHeader;
    sHeader.append('\t').append(sProfile).append('\t').append(sDirectory);
    m_aCertPathList.InsertHeaderEntry( sHeader.makeStringAndClear(), HEADERBAR_APPEND, HIB_LEFT );
    m_aCertPathList.SetCheckButtonHdl( LINK( this, CertPathDialog, CheckHdl_Impl ) );

    m_aAddBtn.SetClickHdl( LINK( this, CertPathDialog, AddHdl_Impl ) );
    m_aOKBtn.SetClickHdl( LINK( this, CertPathDialog, OKHdl_Impl ) );

    FreeResource();

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

        uno::Reference<uno::XInterface> xInstance = comphelper::getProcessServiceFactory()->createInstance(
            "com.sun.star.mozilla.MozillaBootstrap");

        uno::Reference<mozilla::XMozillaBootstrap> xMozillaBootstrap(xInstance, uno::UNO_QUERY_THROW);

        for (sal_Int32 i = 0; i < nProduct; ++i)
        {
            ::rtl::OUString profile = xMozillaBootstrap->getDefaultProfile(productTypes[i]);

            if (!profile.isEmpty())
            {
                ::rtl::OUString sProfilePath = xMozillaBootstrap->getProfilePath( productTypes[i], profile );
                rtl::OUStringBuffer sEntry;
                sEntry.append('\t').appendAscii(productNames[i]).append(':').append(profile).append('\t').append(sProfilePath);
                SvTreeListEntry *pEntry = m_aCertPathList.InsertEntry(sEntry.makeStringAndClear());
                rtl::OUString* pCertPath = new rtl::OUString(sProfilePath);
                pEntry->SetUserData(pCertPath);
            }
        }
    }
    catch (const uno::Exception&)
    {
    }

    SvTreeListEntry *pEntry = m_aCertPathList.GetEntry(0);
    if (pEntry)
    {
        m_aCertPathList.SetCheckButtonState(pEntry, SV_BUTTON_CHECKED);
        HandleCheckEntry(pEntry);
    }

    try
    {
        rtl::OUString sUserSetCertPath =
            officecfg::Office::Common::Security::Scripting::CertDir::get().get_value_or(rtl::OUString());

        if (!sUserSetCertPath.isEmpty())
            AddCertPath(m_sManual, sUserSetCertPath);
    }
    catch (const uno::Exception &e)
    {
        SAL_WARN("cui.options", "CertPathDialog::CertPathDialog(): caught exception" << e.Message);
    }

    const char* pEnv = getenv("MOZILLA_CERTIFICATE_FOLDER");
    if (pEnv)
        AddCertPath("$MOZILLA_CERTIFICATE_FOLDER", rtl::OUString(pEnv, strlen(pEnv), osl_getThreadTextEncoding()));
}

IMPL_LINK_NOARG(CertPathDialog, OKHdl_Impl)
{
    fprintf(stderr, "dir is %s\n", rtl::OUStringToOString(getDirectory(), RTL_TEXTENCODING_UTF8).getStr());

    try
    {
        boost::shared_ptr< comphelper::ConfigurationChanges > batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Security::Scripting::CertDir::set(
            getDirectory(), batch);
        batch->commit();
    }
    catch (const uno::Exception &e)
    {
        SAL_WARN("cui.options", "CertPathDialog::OKHdl_Impl(): caught exception" << e.Message);
    }

    EndDialog(true);

    return 0;
}

rtl::OUString CertPathDialog::getDirectory() const
{
    SvTreeListEntry* pEntry = m_aCertPathList.FirstSelected();
    void* pCertPath = pEntry ? pEntry->GetUserData() : NULL;
    return pCertPath ? *static_cast<rtl::OUString*>(pCertPath) : rtl::OUString();
}

CertPathDialog::~CertPathDialog()
{
    SvTreeListEntry* pEntry = m_aCertPathList.First();
    while (pEntry)
    {
        rtl::OUString* pCertPath = static_cast<rtl::OUString*>(pEntry->GetUserData());
        delete pCertPath;
        pEntry = m_aCertPathList.Next( pEntry );
    }
}

IMPL_LINK( CertPathDialog, CheckHdl_Impl, SvxSimpleTable *, pList )
{
    SvTreeListEntry* pEntry = pList ? m_aCertPathList.GetEntry(m_aCertPathList.GetCurMousePoint())
                                : m_aCertPathList.FirstSelected();
    if (pEntry)
        m_aCertPathList.HandleEntryChecked(pEntry);
    return 0;
}

void CertPathDialog::HandleCheckEntry( SvTreeListEntry* _pEntry )
{
    m_aCertPathList.Select( _pEntry, true );
    SvButtonState eState = m_aCertPathList.GetCheckButtonState( _pEntry );

    if (SV_BUTTON_CHECKED == eState)
    {
        // uncheck the other entries
        SvTreeListEntry* pEntry = m_aCertPathList.First();
        while (pEntry)
        {
            if (pEntry != _pEntry)
                m_aCertPathList.SetCheckButtonState(pEntry, SV_BUTTON_UNCHECKED);
            pEntry = m_aCertPathList.Next(pEntry);
        }
    }
    else
        m_aCertPathList.SetCheckButtonState(_pEntry, SV_BUTTON_CHECKED);
}

void CertPathDialog::AddCertPath(const rtl::OUString &rProfile, const rtl::OUString &rPath)
{
    SvTreeListEntry* pEntry = m_aCertPathList.First();
    while (pEntry)
    {
        rtl::OUString* pCertPath = static_cast<rtl::OUString*>(pEntry->GetUserData());
        //already exists, just select the original one
        if (pCertPath->equals(rPath))
        {
            m_aCertPathList.SetCheckButtonState(pEntry, SV_BUTTON_CHECKED);
            HandleCheckEntry(pEntry);
            return;
        }
        pEntry = m_aCertPathList.Next(pEntry);
    }

    rtl::OUStringBuffer sEntry;
    sEntry.append('\t').append(rProfile).append('\t').append(rPath);
    pEntry = m_aCertPathList.InsertEntry(sEntry.makeStringAndClear());
    rtl::OUString* pCertPath = new rtl::OUString(rPath);
    pEntry->SetUserData(pCertPath);
    m_aCertPathList.SetCheckButtonState(pEntry, SV_BUTTON_CHECKED);
    HandleCheckEntry(pEntry);
}

IMPL_LINK_NOARG(CertPathDialog, AddHdl_Impl)
{
    try
    {
        uno::Reference<ui::dialogs::XFolderPicker2> xFolderPicker = ui::dialogs::FolderPicker::create(comphelper::getProcessComponentContext());

        rtl::OUString sURL;
        osl::Security().getHomeDir(sURL);
        xFolderPicker->setDisplayDirectory(sURL);
        xFolderPicker->setDescription(m_sAddDialogText);

        if (xFolderPicker->execute() == ui::dialogs::ExecutableDialogResults::OK)
        {
            sURL = xFolderPicker->getDirectory();
            rtl::OUString aPath;
            if (osl::FileBase::E_None == osl::FileBase::getSystemPathFromFileURL(sURL, aPath))
                AddCertPath(m_sManual, aPath);
        }
    }
    catch (const uno::Exception&)
    {
        SAL_WARN( "cui.options", "CertPathDialog::AddHdl_Impl(): caught exception" );
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
