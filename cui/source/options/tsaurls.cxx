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
#include <cuires.hrc>

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

    try
    {
        css::uno::Sequence<OUString> aUserSetTSAURLs(officecfg::Office::Common::Security::Scripting::TSAURLs::get());

        for (auto i = aUserSetTSAURLs.begin(); i != aUserSetTSAURLs.end(); ++i)
        {
            AddTSAURL(*i);
        }
    }
    catch (const uno::Exception &e)
    {
        SAL_WARN("cui.options", "TSAURLsDialog::TSAURLsDialog(): caught exception" << e.Message);
    }
}

IMPL_LINK_NOARG_TYPED(TSAURLsDialog, OKHdl_Impl, Button*, void)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    css::uno::Sequence<OUString> aNewValue(m_aURLs.size());
    size_t n(0);

    for (auto i = m_aURLs.cbegin(); i != m_aURLs.cend(); ++i)
        aNewValue[n++] = *i;
    officecfg::Office::Common::Security::Scripting::TSAURLs::set(aNewValue, batch);
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

IMPL_LINK_NOARG_TYPED(TSAURLsDialog, AddHdl_Impl, Button*, void)
{
    OUString aURL;
    OUString aDesc( get<FixedText>("enteraurl")->GetText() );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    std::unique_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( m_pAddBtn, aURL, aDesc));

    if ( pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aURL );

        AddTSAURL(aURL);
        m_pOKBtn->Enable();
    }
}

IMPL_LINK_NOARG_TYPED(TSAURLsDialog, DeleteHdl_Impl, Button*, void)
{
    sal_Int32 nSel = m_pURLListBox->GetSelectEntryPos();

    if (nSel == LISTBOX_ENTRY_NOTFOUND)
        return;

    m_aURLs.erase(m_pURLListBox->GetEntry(nSel));
    m_pURLListBox->RemoveEntry(nSel);
    m_pOKBtn->Enable();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
