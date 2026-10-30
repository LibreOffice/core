/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <action.hxx>
#include <document.hxx>
#include <strings.hxx>
#include <view.hxx>
#include <comphelper/kit.hxx>
#include <editeng/editeng.hxx>

SmFormatAction::SmFormatAction(SmDocShell *pDocSh,
                               const SmFormat& rOldFormat,
                               const SmFormat& rNewFormat) :
    pDoc( pDocSh ),
    aOldFormat( rOldFormat ),
    aNewFormat( rNewFormat )
{
}

void SmFormatAction::Undo()
{
    pDoc->SetFormat(aOldFormat);
}

void SmFormatAction::Redo()
{
    pDoc->SetFormat(aNewFormat);
}

void SmFormatAction::Repeat(SfxRepeatTarget& rDocSh)
{
    dynamic_cast< SmDocShell & >(rDocSh).SetFormat(aNewFormat);
}

OUString SmFormatAction::GetComment() const
{
    return RID_UNDOFORMATNAME;
}

SmTextAction::SmTextAction(SmDocShell* pDocSh, OUString aOld, OUString aNew)
    : pDoc(pDocSh)
    , aOldText(std::move(aOld))
    , aNewText(std::move(aNew))
{
}

namespace
{
// the formula takes that text, and its command text follows, the undo manager keeps no step for
// that while it undoes or redoes; the kit shows the caret of the formula it draws, which starts
// anew with the text
void lcl_setText(SmDocShell& rDoc, const OUString& rText)
{
    EditEngine& rEditEngine = rDoc.GetEditEngine();
    rEditEngine.QuickInsertText(rText, ESelection::All());
    rEditEngine.QuickFormatDoc();
    rDoc.SetText(rText);
    if (SmViewShell* pView = comphelper::COKit::isActive() ? SmGetActiveView() : nullptr)
        pView->SendCaretToKit();
}
}

void SmTextAction::Undo() { lcl_setText(*pDoc, aOldText); }

void SmTextAction::Redo() { lcl_setText(*pDoc, aNewText); }

OUString SmTextAction::GetComment() const
{
    return RID_UNDOEDITNAME;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
