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

#ifndef _FRAMEWORK_ACCELERATORCONST_H_
#define _FRAMEWORK_ACCELERATORCONST_H_

#include <general.h>


namespace framework{

#define DOCTYPE_ACCELERATORS            DECLARE_ASCII("<!DOCTYPE accel:acceleratorlist PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"accelerator.dtd\">")

#define ATTRIBUTE_TYPE_CDATA            DECLARE_ASCII("CDATA")

#define XMLNS_XLINK                     DECLARE_ASCII("xlink")

// same items with a name space alias
#define AL_ELEMENT_ACCELERATORLIST      DECLARE_ASCII("accel:acceleratorlist")
#define AL_ELEMENT_ITEM                 DECLARE_ASCII("accel:item"           )

#define AL_XMLNS_ACCEL                  DECLARE_ASCII("xmlns:accel")
#define AL_XMLNS_XLINK                  DECLARE_ASCII("xmlns:xlink")

#define AL_ATTRIBUTE_URL                DECLARE_ASCII("xlink:href" )
#define AL_ATTRIBUTE_KEYCODE            DECLARE_ASCII("accel:code" )
#define AL_ATTRIBUTE_MOD_SHIFT          DECLARE_ASCII("accel:shift")
#define AL_ATTRIBUTE_MOD_MOD1           DECLARE_ASCII("accel:mod1" )
#define AL_ATTRIBUTE_MOD_MOD2           DECLARE_ASCII("accel:mod2" )
#define AL_ATTRIBUTE_MOD_MOD3           DECLARE_ASCII("accel:mod3" )

// same items with full qualified name space
#define NS_ELEMENT_ACCELERATORLIST      DECLARE_ASCII("http://openoffice.org/2001/accel^acceleratorlist")
#define NS_ELEMENT_ITEM                 DECLARE_ASCII("http://openoffice.org/2001/accel^item"           )

#define NS_XMLNS_ACCEL                  DECLARE_ASCII("http://openoffice.org/2001/accel")
#define NS_XMLNS_XLINK                  DECLARE_ASCII("http://www.w3.org/1999/xlink"    )

#define NS_ATTRIBUTE_URL                DECLARE_ASCII("http://www.w3.org/1999/xlink^href"     )
#define NS_ATTRIBUTE_KEYCODE            DECLARE_ASCII("http://openoffice.org/2001/accel^code" )
#define NS_ATTRIBUTE_MOD_SHIFT          DECLARE_ASCII("http://openoffice.org/2001/accel^shift")
#define NS_ATTRIBUTE_MOD_MOD1           DECLARE_ASCII("http://openoffice.org/2001/accel^mod1" )
#define NS_ATTRIBUTE_MOD_MOD2           DECLARE_ASCII("http://openoffice.org/2001/accel^mod2" )
#define NS_ATTRIBUTE_MOD_MOD3           DECLARE_ASCII("http://openoffice.org/2001/accel^mod3" )

} // namespace framework

#endif // _FRAMEWORK_ACCELERATORCONST_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
