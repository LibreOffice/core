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

#include <rtl/ustring.hxx>

#define SD_IF_SDAPP                     SFX_INTERFACE_SD_START + SfxInterfaceId(0)
#define SD_IF_SDDRAWDOCSHELL            SFX_INTERFACE_SD_START + SfxInterfaceId(1)
#define SD_IF_SDDRAWVIEWSHELL           SFX_INTERFACE_SD_START + SfxInterfaceId(3)
#define SD_IF_SDOUTLINEVIEWSHELL        SFX_INTERFACE_SD_START + SfxInterfaceId(5)
#define SD_IF_SDDRAWTEXTOBJECTBAR       SFX_INTERFACE_SD_START + SfxInterfaceId(7)
#define SD_IF_SDDRAWBEZIEROBJECTBAR     SFX_INTERFACE_SD_START + SfxInterfaceId(8)
#define SD_IF_SDGRAPHICDOCSHELL         SFX_INTERFACE_SD_START + SfxInterfaceId(10)
#define SD_IF_SDGRAPHICVIEWSHELL        SFX_INTERFACE_SD_START + SfxInterfaceId(11)
#define SD_IF_SDDRAWGRAFOBJECTBAR       SFX_INTERFACE_SD_START + SfxInterfaceId(13)
#define SD_IF_SDPRESVIEWSHELL           SFX_INTERFACE_SD_START + SfxInterfaceId(14)
#define SD_IF_SDVIEWSHELLBASE           SFX_INTERFACE_SD_START + SfxInterfaceId(16)
#define SD_IF_SDSLIDESORTERVIEWSHELL    SFX_INTERFACE_SD_START + SfxInterfaceId(19)
#define SD_IF_SDDRAWMEDIAOBJECTBAR      SFX_INTERFACE_SD_START + SfxInterfaceId(23)
#define SD_IF_SDLEFTIMPRESSPANESHELL    SFX_INTERFACE_SD_START + SfxInterfaceId(24)
#define SD_IF_SDLEFTDRAWPANESHELL       SFX_INTERFACE_SD_START + SfxInterfaceId(25)
#define SD_IF_SDDRAWTABLEOBJECTBAR      SFX_INTERFACE_SD_START + SfxInterfaceId(27)
#define SD_IF_SDTOOLPANELPANESHELL      SFX_INTERFACE_SD_START + SfxInterfaceId(29)
#define SD_IF_SDTOOLPANELSHELL          SFX_INTERFACE_SD_START + SfxInterfaceId(30)
#define SD_IF_SDBOTTOMIMPRESSPANESHELL  SFX_INTERFACE_SD_START + SfxInterfaceId(31)
#define SD_IF_SDNOTESPANELVIEWSHELL     SFX_INTERFACE_SD_START + SfxInterfaceId(32)


// Object-Ids for StarDraw UserData
#define SD_ANIMATIONINFO_ID 1
// SVX_IMAPINFO_ID = 2

// Separator between layout name and template name of presentation templates
inline constexpr OUString SD_LT_SEPARATOR = u"~LT~"_ustr;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
