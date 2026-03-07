/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Operation.hxx"
#include <i18nlangtag/lang.h>

enum class TransliterationFlags;

class ScDocFunc;
class ScDocShell;
class ScMarkData;

namespace sc
{
/** Operation which transliterates text in marked cells. */
class TransliterateTextOperation : public Operation
{
private:
    ScDocFunc& mrDocFunc;
    ScDocShell& mrDocShell;
    ScMarkData const& mrMark;
    TransliterationFlags mnType;

    bool runImplementation() override;

public:
    TransliterateTextOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell, const ScMarkData& rMark,
                               TransliterationFlags nType, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
