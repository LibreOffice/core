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
#ifndef INCLUDED_SFX2_GROUPID_HXX
#define INCLUDED_SFX2_GROUPID_HXX

#ifdef __RSC
#define SfxGroupId(x)  (x)
#else

#include <o3tl/strong_int.hxx>
#include <sal/types.h>
#include <functional>

struct SfxGroupIdTag {};
typedef o3tl::strong_int<sal_uInt16, SfxGroupIdTag> SfxGroupId;

// make it hashable for storing in maps
namespace std {

  template <>
  struct hash<SfxGroupId>
  {
    std::size_t operator()(const SfxGroupId& k) const
    {
      return std::hash<sal_uInt16>()((sal_uInt16)k);
    }
  };

}

// Make it easier to declare the constants in the .hrc files.
// This way, we only have to mark the START constant as being SfxGroupId.
inline SfxGroupId operator+(SfxGroupId lhs, sal_uInt16 rhs)
{
    return SfxGroupId((sal_uInt16)lhs + rhs);
}
#endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
