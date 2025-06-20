/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DocumentModelAccessor.hxx>

#include <document.hxx>
#include <docpool.hxx>
#include <svl/intitem.hxx>
#include <svl/zformat.hxx>
#include <svl/zforlist.hxx>
#include <svl/numformat.hxx>
#include <svl/itempool.hxx>

namespace
{
struct CurrencyIDLess
{
    bool operator()(const sfx::CurrencyID& lhs, const sfx::CurrencyID& rhs) const
    {
        return std::tie(lhs.aSymbol, lhs.aExtension, lhs.eLanguage)
               < std::tie(rhs.aSymbol, rhs.aExtension, rhs.eLanguage);
    }
};
struct CurrencyIDEquals
{
    bool operator()(const sfx::CurrencyID& lhs, const sfx::CurrencyID& rhs) const
    {
        return std::tie(lhs.aSymbol, lhs.aExtension, lhs.eLanguage)
               == std::tie(rhs.aSymbol, rhs.aExtension, rhs.eLanguage);
    }
};

} // anonymous

namespace sc
{
std::vector<sfx::CurrencyID> DocumentModelAccessor::getDocumentCurrencies() const
{
    SvNumberFormatter* pFormatter = m_pDocument->GetFormatTable();
    if (!pFormatter)
        return {};

    std::vector<sfx::CurrencyID> aCurrencyIDs;

    const SvNFFormatData& rFormatData = pFormatter->GetROFormatData();
    const SvNFFormatData::FormatEntryMap& rEntryMap = rFormatData.GetFormatEntryMap();
    for (const auto& rPair : rEntryMap)
    {
        SvNumberformat const* pEntry = rPair.second.get();
        if (pEntry && pEntry->GetMaskedType() == SvNumFormatType::CURRENCY
            && pEntry->HasNewCurrency() && pEntry->GetLanguage() != LANGUAGE_SYSTEM
            && !pEntry->IsAdditionalBuiltin())
        {
            OUString aSymbol;
            OUString aExtension;
            pEntry->GetNewCurrencySymbol(aSymbol, aExtension);
            aCurrencyIDs.push_back({ aSymbol, aExtension, pEntry->GetLanguage() });
        }
    }

    // remove duplicates
    std::sort(aCurrencyIDs.begin(), aCurrencyIDs.end(), CurrencyIDLess());
    aCurrencyIDs.erase(std::unique(aCurrencyIDs.begin(), aCurrencyIDs.end(), CurrencyIDEquals()),
                       aCurrencyIDs.end());
    return aCurrencyIDs;
}

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
