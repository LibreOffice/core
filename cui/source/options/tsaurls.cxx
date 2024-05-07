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
#include <comphelper/diagnose_ex.hxx>

#include "tsaurls.hxx"

using namespace ::com::sun::star;

TSAURLsDialog::TSAURLsDialog(weld::Window* pParent)
    : GenericDialogController(pParent, u"cui/ui/tsaurldialog.ui"_ustr, u"TSAURLDialog"_ustr)
    , m_xAddBtn(m_xBuilder->weld_button(u"add"_ustr))
    , m_xDeleteBtn(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xURLListBox(m_xBuilder->weld_tree_view(u"urls"_ustr))
    , m_xEnterAUrl(m_xBuilder->weld_label(u"enteraurl"_ustr))
{
    m_xURLListBox->set_size_request(m_xURLListBox->get_approximate_digit_width() * 28,
                                    m_xURLListBox->get_height_rows(8));
    m_xOKBtn->set_sensitive(false);

    m_xAddBtn->connect_clicked(LINK(this, TSAURLsDialog, AddHdl_Impl));
    m_xDeleteBtn->connect_clicked(LINK(this, TSAURLsDialog, DeleteHdl_Impl));
    m_xOKBtn->connect_clicked(LINK(this, TSAURLsDialog, OKHdl_Impl));
    m_xURLListBox->connect_changed(LINK(this, TSAURLsDialog, SelectHdl));

    try
    {
        std::optional<css::uno::Sequence<OUString>> aUserSetTSAURLs(
            officecfg::Office::Common::Security::Scripting::TSAURLs::get());
        if (aUserSetTSAURLs)
        {
            const css::uno::Sequence<OUString>& rUserSetTSAURLs = *aUserSetTSAURLs;
            for (auto const& userSetTSAURL : rUserSetTSAURLs)
            {
                AddTSAURL(userSetTSAURL);
            }
        }
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("cui.options", "TSAURLsDialog::TSAURLsDialog()");
    }

    if (m_xURLListBox->get_selected_index() == -1)
    {
        m_xDeleteBtn->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(TSAURLsDialog, OKHdl_Impl, weld::Button&, void)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());

    officecfg::Office::Common::Security::Scripting::TSAURLs::set(
        comphelper::containerToSequence(m_aURLs), batch);
    batch->commit();

    m_xDialog->response(RET_OK);
}

TSAURLsDialog::~TSAURLsDialog() {}

void TSAURLsDialog::AddTSAURL(const OUString& rURL)
{
    m_aURLs.insert(rURL);

    m_xURLListBox->freeze();
    m_xURLListBox->clear();

    for (auto const& url : m_aURLs)
    {
        m_xURLListBox->append_text(url);
    }

    m_xURLListBox->thaw();
}

IMPL_LINK_NOARG(TSAURLsDialog, AddHdl_Impl, weld::Button&, void)
{
    OUString aDesc(m_xEnterAUrl->get_label());

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(
        pFact->CreateSvxNameDialog(m_xDialog.get(), OUString(), aDesc));

    if (pDlg->Execute() == RET_OK)
    {
        AddTSAURL(pDlg->GetName());
        m_xOKBtn->set_sensitive(true);
    }
    m_xURLListBox->unselect_all();
    // After operations in a ListBox we have nothing selected
    m_xDeleteBtn->set_sensitive(false);
}

IMPL_LINK_NOARG(TSAURLsDialog, SelectHdl, weld::TreeView&, void)
{
    m_xDeleteBtn->set_sensitive(true);
}

IMPL_LINK_NOARG(TSAURLsDialog, DeleteHdl_Impl, weld::Button&, void)
{
    int nSel = m_xURLListBox->get_selected_index();
    if (nSel == -1)
        return;

    m_aURLs.erase(m_xURLListBox->get_text(nSel));
    m_xURLListBox->remove(nSel);
    m_xURLListBox->unselect_all();
    // After operations in a ListBox we have nothing selected
    m_xDeleteBtn->set_sensitive(false);
    m_xOKBtn->set_sensitive(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
