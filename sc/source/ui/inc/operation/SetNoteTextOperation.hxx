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

class ScDocShell;

namespace sc
{
/** Operation which sets a note text at a given cell address. */
class SetNoteTextOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScAddress maPos;
    OUString maText;

    bool runImplementation() override;

public:
    SetNoteTextOperation(ScDocShell& rDocShell, const ScAddress& rPos, const OUString& rText,
                         bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
