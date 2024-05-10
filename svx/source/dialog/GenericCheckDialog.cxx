/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <svx/GenericCheckDialog.hxx>
#include <vcl/svapp.hxx>

namespace svx
{
GenericCheckEntry::GenericCheckEntry(weld::Container* pParent,
                                     std::unique_ptr<CheckData>& pCheckData)
    : m_xBuilder(Application::CreateBuilder(pParent, u"svx/ui/genericcheckentry.ui"_ustr))
    , m_xContainer(m_xBuilder->weld_container(u"checkEntryBox"_ustr))
    , m_xLabel(m_xBuilder->weld_label(u"label"_ustr))
    , m_xMarkButton(m_xBuilder->weld_button(u"markButton"_ustr))
    , m_xPropertiesButton(m_xBuilder->weld_button(u"propertiesButton"_ustr))
    , m_pCheckData(pCheckData)
{
    m_xLabel->set_label(m_pCheckData->getText());
    m_xMarkButton->set_visible(m_pCheckData->canMarkObject());
    m_xMarkButton->connect_clicked(LINK(this, GenericCheckEntry, MarkButtonClicked));
    m_xPropertiesButton->set_visible(m_pCheckData->hasProperties());
    m_xPropertiesButton->connect_clicked(LINK(this, GenericCheckEntry, PropertiesButtonClicked));

    m_xContainer->show();
}

IMPL_LINK_NOARG(GenericCheckEntry, MarkButtonClicked, weld::Button&, void)
{
    m_pCheckData->markObject();
}

IMPL_LINK_NOARG(GenericCheckEntry, PropertiesButtonClicked, weld::Button&, void)
{
    m_pCheckData->runProperties();
}

GenericCheckDialog::GenericCheckDialog(weld::Window* pParent,
                                       CheckDataCollection& rCheckDataCollection)
    : GenericDialogController(pParent, u"svx/ui/genericcheckdialog.ui"_ustr,
                              u"GenericCheckDialog"_ustr)
    , m_rCheckDataCollection(rCheckDataCollection)
    , m_xCheckBox(m_xBuilder->weld_box(u"checkBox"_ustr))
{
    set_title(m_rCheckDataCollection.getTitle());
}

GenericCheckDialog::~GenericCheckDialog() {}

short GenericCheckDialog::run()
{
    sal_Int32 i = 0;

    for (std::unique_ptr<CheckData>& pCheckData : m_rCheckDataCollection.getCollection())
    {
        auto xEntry = std::make_unique<GenericCheckEntry>(m_xCheckBox.get(), pCheckData);
        m_xCheckBox->reorder_child(xEntry->get_widget(), i++);
        m_aCheckEntries.push_back(std::move(xEntry));
    }
    return GenericDialogController::run();
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
