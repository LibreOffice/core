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
#ifndef _SFXMNUCFGA_HXX
#define _SFXMNUCFGA_HXX

#include <tools/string.hxx>
#include <sfx2/minarray.hxx>

//==================================================================

class SfxMenuCfgItemArr;

struct SfxMenuCfgItem
{
    sal_uInt16             nId;        // id of the binding or 0 if none
    String             aTitle;     // title of the item
    String             aHelpText;
    String              aCommand;
    SfxMenuCfgItemArr* pPopup;     // pointer to a popup menu (if any)
};

DECL_PTRARRAY(SfxMenuCfgItemArr, SfxMenuCfgItem*, 4, 4 )


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
