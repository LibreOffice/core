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
#pragma once

#include <sal/config.h>

#include <config_features.h>

#include <functional>

#include <rtl/string.hxx>

#include <swdllapi.h>
#include <nodeoffset.hxx>
#include <swundo.hxx>

class SwWrtShell;
class SwPaM;

namespace SwTranslateHelper
{
SW_DLLPUBLIC OString ExportPaMToHTML(SwPaM* pCursor);
SW_DLLPUBLIC void PasteHTMLToPaM(SwWrtShell& rWrtSh, const SwPaM* pCursor, const OString& rData);

/// Groups a bulk update of several ranges into one undo step and one layout action, and keeps the
/// caret in place across it. Pasting into a range moves the shell cursor there, so the caret is
/// saved when the object is created and put back when it is destroyed. The saved cursor follows
/// the content edits, so it still points at the original spot when it is restored.
class SW_DLLPUBLIC BulkUpdateGuard
{
public:
    BulkUpdateGuard(SwWrtShell& rWrtSh, SwUndoId eUndoId);
    ~BulkUpdateGuard();

    BulkUpdateGuard(const BulkUpdateGuard&) = delete;
    BulkUpdateGuard& operator=(const BulkUpdateGuard&) = delete;

private:
    SwWrtShell& m_rWrtSh;
    SwUndoId m_eUndoId;
};
/// Node range to walk for translation: the current selection when there is one, otherwise
/// the whole document body, but never the header/footer/footnote sections that also live in
/// the shell's node array outside the body.
SW_DLLPUBLIC void GetTranslationNodeRange(SwWrtShell& rWrtSh, SwNodeOffset& rStartNode,
                                          SwNodeOffset& rEndNode);

/// Translates the exported HTML of one text node. Returns an empty string and
/// describes the failure in rError when the translation failed.
using TranslateFunc = std::function<OString(const OString& rHtml, OString& rError)>;

/// Translates the current selection (or the whole document when there is no
/// selection) text node by text node, replacing the original text with the
/// result of rTranslate for the exported HTML of each node. A table box
/// selection is translated box by box. Returns false when a translation
/// failed, after reporting the failure to the user.
SW_DLLPUBLIC bool TranslateRanges(SwWrtShell& rWrtSh, const TranslateFunc& rTranslate,
                                  const bool& rCancelTranslation);
#if HAVE_FEATURE_CURL
SW_DLLPUBLIC void TranslateDocument(SwWrtShell& rWrtSh, const OString& rTargetLang);
SW_DLLPUBLIC bool TranslateDocumentCancellable(SwWrtShell& rWrtSh, const OString& rTargetLang,
                                               const bool& rCancelTranslation);
SW_DLLPUBLIC bool IsTranslationServiceConfigured();
#endif
}
