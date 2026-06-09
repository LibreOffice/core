/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the LibreOffice contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
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
/** Inserts a threaded comment at the input address, registering an undo action
    and setting the document-modified flag. */
class InsertThreadedCommentOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScAddress maPos;
    OUString maText;
    std::optional<OUString> moAuthor;

    bool runImplementation() override;

public:
    InsertThreadedCommentOperation(ScDocShell& rDocShell, const ScAddress& rPos,
                                   const OUString& rText, const OUString* pAuthor, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
