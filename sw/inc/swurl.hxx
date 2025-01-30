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
#ifndef INCLUDED_SW_INC_SWURL_HXX
#define INCLUDED_SW_INC_SWURL_HXX

#include <rtl/ustring.hxx>
#include <o3tl/typed_flags_set.hxx>

class SwViewShell;
class SwWrtShell;

enum class LoadUrlFlags {
    NONE    = 0x00,
    NewView = 0x01
};
namespace o3tl {
    template<> struct typed_flags<LoadUrlFlags> : is_typed_flags<LoadUrlFlags, 0x01> {};
}

bool LoadURL( SwWrtShell& rSh, const OUString& rName,
              LoadUrlFlags nFilter, const OUString& rTargetFrameName );
bool LoadURL( SwViewShell* rSh, const OUString& rName,
              LoadUrlFlags nFilter, const OUString& rTargetFrameName );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
