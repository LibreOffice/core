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

#include "navicfg.hxx"
#include "content.hxx"

//TODO: #define CFGPATH_NAVIPI          "Office.Calc/Navigator"

ScNavipiCfg::ScNavipiCfg() :
//TODO: ConfigItem( OUString( CFGPATH_NAVIPI ) ),
    nListMode(0),
    nDragMode(0),
    nRootType(ScContentId::ROOT)
{
}

void ScNavipiCfg::SetListMode(sal_uInt16 nNew)
{
    if ( nListMode != nNew )
    {
        nListMode = nNew;
//TODO:     SetModified();
    }
}

void ScNavipiCfg::SetDragMode(sal_uInt16 nNew)
{
    if ( nDragMode != nNew )
    {
        nDragMode = nNew;
//TODO:     SetModified();
    }
}

void ScNavipiCfg::SetRootType(ScContentId nNew)
{
    if ( nRootType != nNew )
    {
        nRootType = nNew;
//TODO:     SetModified();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
