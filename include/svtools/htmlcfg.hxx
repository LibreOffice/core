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

#include <svtools/svtdllapi.h>
#include <rtl/textenc.h>

#define HTML_FONT_COUNT 7

// !!!be aware!!!: the following defines are _not_ used as values in the configuration file
//                  this is because of compatibility reasons
#define HTML_CFG_MSIE 1 // Internet Explorer
#define HTML_CFG_WRITER 2 // Writer
#define HTML_CFG_NS40 3 // Netscape 4.0

#define HTML_CFG_MAX HTML_CFG_NS40

namespace SvxHtmlOptions
{
SVT_DLLPUBLIC sal_uInt16 GetFontSize(sal_uInt16 nPos);

SVT_DLLPUBLIC sal_uInt16 GetExportMode();

SVT_DLLPUBLIC bool IsPrintLayoutExtension();

SVT_DLLPUBLIC bool IsDefaultTextEncoding();
SVT_DLLPUBLIC rtl_TextEncoding GetTextEncoding();
SVT_DLLPUBLIC void SetTextEncoding(rtl_TextEncoding);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
