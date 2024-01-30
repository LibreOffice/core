/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sfx2/dllapi.h>
#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <vector>

namespace sfx
{
/** Currency ID, to identify the currency in the currency list */
struct SFX2_DLLPUBLIC CurrencyID
{
    OUString aSymbol;
    OUString aExtension;
    LanguageType eLanguage;
};

/** Document model accessor, used to access parts of the document model.
 *
 * This is useful when some common parts of the model are needed, but can
 * only access the model indirecly from other modules that can access
 * SfxObjectShell, but don't have a direct access to the document model
 * (and access through UNO would be inconvenient).
 *
 * For example - get information about various parts of the document in
 * generic dialogs (in CUI).
 */
class SFX2_DLLPUBLIC IDocumentModelAccessor
{
public:
    virtual std::vector<CurrencyID> getDocumentCurrencies() const { return {}; }
    virtual ~IDocumentModelAccessor() = default;
};

} // end sfx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
