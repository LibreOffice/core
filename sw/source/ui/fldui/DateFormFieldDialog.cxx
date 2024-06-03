/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DateFormFieldDialog.hxx>
#include <IMark.hxx>
#include <xmloff/odffields.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <doc.hxx>

namespace sw
{
DateFormFieldDialog::DateFormFieldDialog(weld::Widget* pParent,
                                         sw::mark::IDateFieldmark* pDateField, SwDoc& rDoc)
    : GenericDialogController(pParent, u"modules/swriter/ui/dateformfielddialog.ui"_ustr,
                              u"DateFormFieldDialog"_ustr)
    , m_pDateField(pDateField)
    , m_pNumberFormatter(rDoc.GetNumberFormatter())
    , m_xFormatLB(
          new SwNumFormatTreeView(m_xBuilder->weld_tree_view(u"date_formats_treeview"_ustr)))
{
    m_xFormatLB->SetFormatType(SvNumFormatType::DATE);
    m_xFormatLB->SetAutomaticLanguage(true);
    m_xFormatLB->SetShowLanguageControl(true);
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
    if (m_pDateField == nullptr)
        return;

    // Try to find out the current date value and replace the content
    // with the right formatted date string
    sw::mark::IFieldmark::parameter_map_t* pParameters = m_pDateField->GetParameters();
    const SvNumberformat* pFormat = m_pNumberFormatter->GetEntry(m_xFormatLB->GetFormat());

    // Get date value first
    std::pair<bool, double> aResult = m_pDateField->GetCurrentDate();

    // Then set the date format
    (*pParameters)[ODF_FORMDATE_DATEFORMAT] <<= pFormat->GetFormatstring();
    (*pParameters)[ODF_FORMDATE_DATEFORMAT_LANGUAGE]
        <<= LanguageTag(pFormat->GetLanguage()).getBcp47();

    // Update current date
    if (aResult.first)
    {
        m_pDateField->SetCurrentDate(aResult.second);
    }
    else
    {
        (*pParameters)[ODF_FORMDATE_CURRENTDATE] <<= OUString();
    }
}

void DateFormFieldDialog::InitControls()
{
    if (m_pDateField == nullptr)
        return;

    sw::mark::IFieldmark::parameter_map_t* pParameters = m_pDateField->GetParameters();

    OUString sFormatString;
    auto pResult = pParameters->find(ODF_FORMDATE_DATEFORMAT);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sFormatString;
    }

    OUString sLang;
    pResult = pParameters->find(ODF_FORMDATE_DATEFORMAT_LANGUAGE);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sLang;
    }

    if (sFormatString.isEmpty() || sLang.isEmpty())
        return;

    LanguageType aLangType = LanguageTag(sLang).getLanguageType();
    sal_uInt32 nFormat = m_pNumberFormatter->GetEntryKey(sFormatString, aLangType);
    if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        sal_Int32 nCheckPos = 0;
        SvNumFormatType nType;
        m_pNumberFormatter->PutEntry(sFormatString, nCheckPos, nType, nFormat,
                                     LanguageTag(sLang).getLanguageType());
    }

    if (aLangType == LANGUAGE_DONTKNOW || nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
        return;

    if (m_xFormatLB->GetCurLanguage() == aLangType)
    {
        m_xFormatLB->SetAutomaticLanguage(true);
    }
    else
    {
        m_xFormatLB->SetAutomaticLanguage(false);
        m_xFormatLB->SetLanguage(aLangType);

        // Change format and change back for regenerating the list
        m_xFormatLB->SetFormatType(SvNumFormatType::ALL);
        m_xFormatLB->SetFormatType(SvNumFormatType::DATE);
    }
    m_xFormatLB->SetDefFormat(nFormat);
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
