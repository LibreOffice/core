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

#ifdef _WIN32
#include <shlobj.h>
#endif

#include <sot/sotdllapi.h>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>

struct SotAction_Impl
{
    SotClipboardFormatId const   nFormatId;          // Clipboard Id
    sal_uInt16 const             nAction;            // Action Id
    SotExchangeActionFlags const nFlags;             // Action Id
    sal_uInt8 const              nContextCheckId;    // additional check of content in clipboard

    constexpr SotAction_Impl(SotClipboardFormatId _nFormatId, sal_uInt16 _nAction, SotExchangeActionFlags _nFlags, sal_uInt8 _nContextCheckId)
        : nFormatId(_nFormatId), nAction(_nAction), nFlags(_nFlags), nContextCheckId(_nContextCheckId) {}
    constexpr SotAction_Impl(SotClipboardFormatId _nFormatId, sal_uInt16 _nAction)
        : nFormatId(_nFormatId), nAction(_nAction), nFlags(SotExchangeActionFlags::NONE), nContextCheckId(0) {}
};

namespace sot
{
SOT_DLLPUBLIC const SotAction_Impl* GetExchangeDestinationWriterFreeAreaCopy();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
