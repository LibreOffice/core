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
 *   License, Version 2.0 (the "License"; you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FRAMEWORK_INC_ACCELERATORCONST_H
#define INCLUDED_FRAMEWORK_INC_ACCELERATORCONST_H

#include "general.h"

namespace framework{

#define DOCTYPE_ACCELERATORS            "<!DOCTYPE accel:acceleratorlist PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"accelerator.dtd\">"

#define ATTRIBUTE_TYPE_CDATA            "CDATA"

#define XMLNS_XLINK                     "xlink"

// same items with a name space alias
#define AL_ELEMENT_ACCELERATORLIST      "accel:acceleratorlist"
#define AL_ELEMENT_ITEM                 "accel:item"

#define AL_XMLNS_ACCEL                  "xmlns:accel"
#define AL_XMLNS_XLINK                  "xmlns:xlink"

#define AL_ATTRIBUTE_URL                "xlink:href"
#define AL_ATTRIBUTE_KEYCODE            "accel:code"
#define AL_ATTRIBUTE_MOD_SHIFT          "accel:shift"
#define AL_ATTRIBUTE_MOD_MOD1           "accel:mod1"
#define AL_ATTRIBUTE_MOD_MOD2           "accel:mod2"
#define AL_ATTRIBUTE_MOD_MOD3           "accel:mod3"

// same items with full qualified name space
#define NS_XMLNS_ACCEL                  "http://openoffice.org/2001/accel"
#define NS_XMLNS_XLINK                  "http://www.w3.org/1999/xlink"

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_ACCELERATORCONST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
