/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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
#include <config_wasm_strip.h>

#include <swtypes.hxx>

class SwWrtShell;
class SwPaM;
class SwNode;
class SwTextNode;

namespace SwTranslateHelper
{
struct SW_DLLPUBLIC TranslateAPIConfig final
{
    const OString m_xAPIUrl;
    const OString m_xAuthKey;
    const OString m_xTargetLanguage;
};
SW_DLLPUBLIC OString ExportPaMToHTML(SwPaM* pCursor);
SW_DLLPUBLIC void PasteHTMLToPaM(SwWrtShell& rWrtSh, const SwPaM* pCursor, const OString& rData);
#if HAVE_FEATURE_CURL
SW_DLLPUBLIC void TranslateDocument(SwWrtShell& rWrtSh, const TranslateAPIConfig& rConfig);
SW_DLLPUBLIC void TranslateDocumentCancellable(SwWrtShell& rWrtSh,
                                               const TranslateAPIConfig& rConfig,
                                               const bool& rCancelTranslation);
#endif
}
