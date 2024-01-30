/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/IDocumentModelAccessor.hxx>
#include <document.hxx>

namespace sc
{
/** DocumentModelAccessor implementation for Calc */
class DocumentModelAccessor : public sfx::IDocumentModelAccessor
{
private:
    std::shared_ptr<ScDocument> m_pDocument;

public:
    DocumentModelAccessor(std::shared_ptr<ScDocument> const& pDocument)
        : m_pDocument(pDocument)
    {
    }

    std::vector<sfx::CurrencyID> getDocumentCurrencies() const override;
};

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
