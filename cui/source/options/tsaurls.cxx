/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <officecfg/Office/Common.hxx>
#include <svx/svxdlg.hxx>
#include <comphelper/sequence.hxx>

#include "tsaurls.hxx"

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

using namespace ::com::sun::star;

TSAURLsDialog::TSAURLsDialog(vcl::Window* pParent)
    : ModalDialog(pParent, "TSAURLDialog", "cui/ui/tsaurldialog.ui")
{
    get(m_pAddBtn, "add");
    get(m_pDeleteBtn, "delete");
    get(m_pOKBtn, "ok");
    get(m_pURLListBox, "urls");

    m_pURLListBox->SetDropDownLineCount(8);
    m_pURLListBox->set_width_request(m_pURLListBox->approximate_char_width() * 32);
    m_pOKBtn->Disable();

    m_pAddBtn->SetClickHdl( LINK( this, TSAURLsDialog, AddHdl_Impl ) );
    m_pDeleteBtn->SetClickHdl( LINK( this, TSAURLsDialog, DeleteHdl_Impl ) );
    m_pOKBtn->SetClickHdl( LINK( this, TSAURLsDialog, OKHdl_Impl ) );
    m_pURLListBox->SetSelectHdl( LINK( this, TSAURLsDialog, SelectHdl ) );

    try
    {
        boost::optional<css::uno::Sequence<OUString>> aUserSetTSAURLs(officecfg::Office::Common::Security::Scripting::TSAURLs::get());
        if (aUserSetTSAURLs)
        {
            const css::uno::Sequence<OUString>& rUserSetTSAURLs = aUserSetTSAURLs.get();
            for (auto i = rUserSetTSAURLs.begin(); i != rUserSetTSAURLs.end(); ++i)
            {
                AddTSAURL(*i);
            }
        }
    }
    catch (const uno::Exception &e)
    {
        SAL_WARN("cui.options", "TSAURLsDialog::TSAURLsDialog(): " << e);
    }

    if ( m_pURLListBox->GetSelectedEntryCount() == 0 )
    {
        m_pDeleteBtn->Disable();
    }
}

IMPL_LINK_NOARG(TSAURLsDialog, OKHdl_Impl, Button*, void)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    officecfg::Office::Common::Security::Scripting::TSAURLs::set(comphelper::containerToSequence(m_aURLs), batch);
    batch->commit();

    EndDialog(RET_OK);
}

TSAURLsDialog::~TSAURLsDialog()
{
    disposeOnce();
}

void TSAURLsDialog::dispose()
{
    m_pAddBtn.clear();
    m_pDeleteBtn.clear();
    m_pOKBtn.clear();
    m_pURLListBox.clear();

    ModalDialog::dispose();
}

void TSAURLsDialog::AddTSAURL(const OUString& rURL)
{
    m_aURLs.insert(rURL);

    m_pURLListBox->SetUpdateMode(false);
    m_pURLListBox->Clear();

    for (auto i = m_aURLs.cbegin(); i != m_aURLs.cend(); ++i)
    {
        m_pURLListBox->InsertEntry(*i);
    }

    m_pURLListBox->SetUpdateMode(true);
}

IMPL_LINK_NOARG(TSAURLsDialog, AddHdl_Impl, Button*, void)
{
    OUString aURL;
    OUString aDesc( get<FixedText>("enteraurl")->GetText() );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( m_pAddBtn, aURL, aDesc));

    if ( pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aURL );

        AddTSAURL(aURL);
        m_pOKBtn->Enable();
    }
    // After operations in a ListBox we have nothing selected
    m_pDeleteBtn->Disable();
}

IMPL_LINK_NOARG(TSAURLsDialog, SelectHdl, ListBox&, void)
{
    m_pDeleteBtn->Enable();
}

IMPL_LINK_NOARG(TSAURLsDialog, DeleteHdl_Impl, Button*, void)
{
    sal_Int32 nSel = m_pURLListBox->GetSelectedEntryPos();

    if (nSel == LISTBOX_ENTRY_NOTFOUND)
        return;

    m_aURLs.erase(m_pURLListBox->GetEntry(nSel));
    m_pURLListBox->RemoveEntry(nSel);
    // After operations in a ListBox we have nothing selected
    m_pDeleteBtn->Disable();
    m_pOKBtn->Enable();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
