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
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svx/numfmtsh.hxx>
#include <doc.hxx>

namespace sw
{
DateFormFieldDialog::DateFormFieldDialog(vcl::Window* pParent, mark::IFieldmark* pDateField,
                                         SwDoc* pDoc)
    : SvxStandardDialog(pParent, "DateFormFieldDialog", "modules/swriter/ui/dateformfielddialog.ui")
    , m_pDateField(pDateField)
    , m_pNumberFormatter(pDoc->GetNumberFormatter())
{
    get(m_xFormatLB, "date_formats_treeview");
    m_xFormatLB->SetFormatType(css::util::NumberFormat::DATE);
    m_xFormatLB->SetShowLanguageControl(true);
    m_xFormatLB->SetAutomaticLanguage(true);
    m_xFormatLB->SetShowLanguageControl(true);
    m_xFormatLB->SetOneArea(true);

    InitControls();
}

DateFormFieldDialog::~DateFormFieldDialog() { disposeOnce(); }

void DateFormFieldDialog::dispose()
{
    m_xFormatLB.clear();
    SvxStandardDialog::dispose();
}

void DateFormFieldDialog::Apply()
{
    if (m_pDateField != nullptr)
    {
        const SvNumberformat* pFormat = m_pNumberFormatter->GetEntry(m_xFormatLB->GetFormat());
        sw::mark::IFieldmark::parameter_map_t* pParameters = m_pDateField->GetParameters();
        (*pParameters)[ODF_FORMDATE_DATEFORMAT] <<= pFormat->GetFormatstring();

        LanguageType aLang = pFormat->GetLanguage();
        (*pParameters)[ODF_FORMDATE_DATEFORMAT_LANGUAGE] <<= LanguageTag(aLang).getBcp47();
    }
}

void DateFormFieldDialog::InitControls()
{
    if (m_pDateField != nullptr)
    {
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

        if (!sFormatString.isEmpty() && !sLang.isEmpty())
        {
            LanguageType aLangType = LanguageTag(sLang).getLanguageType();
            sal_uInt32 nFormatKey = m_pNumberFormatter->GetEntryKey(sFormatString, aLangType);
            bool bValidFormat = nFormatKey != NUMBERFORMAT_ENTRY_NOT_FOUND;
            if (nFormatKey == NUMBERFORMAT_ENTRY_NOT_FOUND)
            {
                sal_Int32 nCheckPos = 0;
                short nType;
                bValidFormat
                    = m_pNumberFormatter->PutEntry(sFormatString, nCheckPos, nType, nFormatKey,
                                                   LanguageTag(sLang).getLanguageType());
            }

            if (aLangType != LANGUAGE_DONTKNOW && bValidFormat)
            {
                if (m_xFormatLB->GetCurLanguage() == aLangType)
                {
                    m_xFormatLB->SetAutomaticLanguage(true);
                }
                else
                {
                    m_xFormatLB->SetAutomaticLanguage(false);
                    m_xFormatLB->SetLanguage(aLangType);

                    // Change format and change back for regenerating the list
                    m_xFormatLB->SetFormatType(css::util::NumberFormat::ALL);
                    m_xFormatLB->SetFormatType(css::util::NumberFormat::DATE);
                }
                m_xFormatLB->SetDefFormat(nFormatKey);
            }
        }
    }
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
