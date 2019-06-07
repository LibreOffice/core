/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DateFormFieldDialog.hxx>
#include <vcl/event.hxx>
#include <IMark.hxx>
#include <xmloff/odffields.hxx>

namespace sw
{
DateFormFieldDialog::DateFormFieldDialog(weld::Widget* pParent, mark::IFieldmark* pDateField)
    : GenericDialogController(pParent, "modules/swriter/ui/dateformfielddialog.ui",
                              "DateFormFieldDialog")
    , m_pDateField(pDateField)
    , m_xFormatLB(new SwNumFormatTreeView(m_xBuilder->weld_tree_view("date_formats_treeview")))
{
    m_xFormatLB->SetFormatType(SvNumFormatType::DATETIME);
    m_xFormatLB->SetShowLanguageControl(true);
    m_xFormatLB->SetAutomaticLanguage(true);
    m_xFormatLB->SetOneArea(true);

    // Set a default height
    weld::TreeView& rTreeView = dynamic_cast<weld::TreeView&>(m_xFormatLB->get_widget());
    rTreeView.set_size_request(rTreeView.get_preferred_size().Width(),
                               rTreeView.get_height_rows(10));
    InitControls();
}

DateFormFieldDialog::~DateFormFieldDialog() {}

void DateFormFieldDialog::Apply()
{
    if (m_pDateField != nullptr)
    {
        mark::IFieldmark::parameter_map_t* pParameters = m_pDateField->GetParameters();
        (*pParameters)[ODF_FORMDATE_DATEFORMAT] <<= m_xFormatLB->GetFormat();
    }
}

void DateFormFieldDialog::InitControls()
{
    if (m_pDateField != nullptr)
    {
        mark::IFieldmark::parameter_map_t* pParameters = m_pDateField->GetParameters();

        auto pResult = pParameters->find(ODF_FORMDATE_DATEFORMAT);
        if (pResult != pParameters->end())
        {
            sal_uInt32 nDateFormat = 0;
            pResult->second >>= nDateFormat;
            m_xFormatLB->SetDefFormat(nDateFormat);
        }
    }
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
