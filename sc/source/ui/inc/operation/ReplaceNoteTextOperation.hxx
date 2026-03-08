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
#include <address.hxx>
#include <rtl/ustring.hxx>

#include <optional>

class ScDocShell;

namespace sc
{
/** Sets or replaces the content of the note at the input address. */
class ReplaceNoteTextOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScAddress maPos;
    OUString maText;
    std::optional<OUString> moAuthor;
    std::optional<OUString> moDate;

    bool runImplementation() override;

public:
    ReplaceNoteTextOperation(ScDocShell& rDocShell, const ScAddress& rPos,
                             const OUString& rNoteText, const OUString* pAuthor,
                             const OUString* pDate, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
