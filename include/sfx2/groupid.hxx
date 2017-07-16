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

// group ids

// !! If you add a new group, please change sfxbasecontroller.cxx and
// !! com.sun.star.frame.CommandGroup accordingly!

#define RID_GROUPS_SFXOFFSET                SfxGroupId(32700)
#define GID_INTERN                          (RID_GROUPS_SFXOFFSET+ 0)
#define GID_APPLICATION                     (RID_GROUPS_SFXOFFSET+ 1)
#define GID_DOCUMENT                        (RID_GROUPS_SFXOFFSET+ 2)
#define GID_VIEW                            (RID_GROUPS_SFXOFFSET+ 3)
#define GID_EDIT                            (RID_GROUPS_SFXOFFSET+ 4)
#define GID_MACRO                           (RID_GROUPS_SFXOFFSET+ 5)
#define GID_OPTIONS                         (RID_GROUPS_SFXOFFSET+ 6)
#define GID_MATH                            (RID_GROUPS_SFXOFFSET+ 7)
#define GID_NAVIGATOR                       (RID_GROUPS_SFXOFFSET+ 8)
#define GID_INSERT                          (RID_GROUPS_SFXOFFSET+ 9)
#define GID_FORMAT                          (RID_GROUPS_SFXOFFSET+10)
#define GID_TEMPLATE                        (RID_GROUPS_SFXOFFSET+11)
#define GID_TEXT                            (RID_GROUPS_SFXOFFSET+12)
#define GID_FRAME                           (RID_GROUPS_SFXOFFSET+13)
#define GID_GRAPHIC                         (RID_GROUPS_SFXOFFSET+14)
#define GID_TABLE                           (RID_GROUPS_SFXOFFSET+15)
#define GID_ENUMERATION                     (RID_GROUPS_SFXOFFSET+16)
#define GID_DATA                            (RID_GROUPS_SFXOFFSET+17)
#define GID_SPECIAL                         (RID_GROUPS_SFXOFFSET+18)
#define GID_IMAGE                           (RID_GROUPS_SFXOFFSET+19)
#define GID_CHART                           (RID_GROUPS_SFXOFFSET+20)
#define GID_EXPLORER                        (RID_GROUPS_SFXOFFSET+21)
#define GID_CONNECTOR                       (RID_GROUPS_SFXOFFSET+22)
#define GID_MODIFY                          (RID_GROUPS_SFXOFFSET+23)
#define GID_DRAWING                         (RID_GROUPS_SFXOFFSET+24)
#define GID_CONTROLS                        (RID_GROUPS_SFXOFFSET+25)
#define GID_NAVIGATION                      (RID_GROUPS_SFXOFFSET+26)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
