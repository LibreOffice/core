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

TSAURLsDialog::TSAURLsDialog(Window* pParent)
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

        for (sal_Int32 i = 0; i < aUserSetTSAURLs.getLength(); ++i)
        {
            AddTSAURL(aUserSetTSAURLs[i]);
        }
    }
    catch (const uno::Exception &e)
    {
        SAL_WARN("cui.options", "TSAURLsDialog::TSAURLsDialog(): caught exception" << e.Message);
    }
}

IMPL_LINK_NOARG(TSAURLsDialog, OKHdl_Impl)
{
    boost::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    css::uno::Sequence<OUString> aNewValue(m_aURLs.size());
    size_t n(0);

    for (std::set<OUString>::iterator i = m_aURLs.begin(); i != m_aURLs.end(); ++i)
        aNewValue[n++] = *i;
    officecfg::Office::Common::Security::Scripting::TSAURLs::set(aNewValue, batch);
    batch->commit();

    EndDialog(RET_OK);

    return 0;
}

TSAURLsDialog::~TSAURLsDialog()
{
}

void TSAURLsDialog::AddTSAURL(const OUString& rURL)
{
    m_aURLs.insert(rURL);

    m_pURLListBox->SetUpdateMode(false);
    m_pURLListBox->Clear();

    for (std::set<OUString>::iterator i = m_aURLs.begin(); i != m_aURLs.end(); ++i)
    {
        m_pURLListBox->InsertEntry(*i);
    }

    m_pURLListBox->SetUpdateMode(true);
}

IMPL_LINK_NOARG(TSAURLsDialog, AddHdl_Impl)
{
    OUString aURL;
    OUString aDesc( get<FixedText>("enteraurl")->GetText() );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    boost::scoped_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( m_pAddBtn, aURL, aDesc));

    if ( pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aURL );

        AddTSAURL(aURL);
        m_pOKBtn->Enable();
    }

    return 0;
}

IMPL_LINK_NOARG(TSAURLsDialog, DeleteHdl_Impl)
{
    sal_Int32 nSel = m_pURLListBox->GetSelectEntryPos();

    if (nSel == LISTBOX_ENTRY_NOTFOUND)
        return 0;

    m_aURLs.erase(m_pURLListBox->GetEntry(nSel));
    m_pURLListBox->RemoveEntry(nSel);
    m_pOKBtn->Enable();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
