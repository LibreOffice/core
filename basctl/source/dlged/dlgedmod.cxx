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

#include "dlgedmod.hxx"
#include "dlgedpage.hxx"
#include <tools/debug.hxx>

namespace basctl
{

DBG_NAME(DlgEdModel)

TYPEINIT1(DlgEdModel,SdrModel);

//----------------------------------------------------------------------------

DlgEdModel::DlgEdModel()
    :SdrModel(NULL, NULL)
{
    DBG_CTOR(DlgEdModel,0);
}

//----------------------------------------------------------------------------

DlgEdModel::~DlgEdModel()
{
    DBG_DTOR(DlgEdModel,0);
}

//----------------------------------------------------------------------------

SdrPage* DlgEdModel::AllocPage(bool bMasterPage)
{
    DBG_CHKTHIS(DlgEdModel, 0);
    return new DlgEdPage(*this, bMasterPage);
}

//----------------------------------------------------------------------------

void DlgEdModel::DlgEdModelChanged(bool bChanged )
{
    SetChanged( bChanged );
}

//----------------------------------------------------------------------------

Window* DlgEdModel::GetCurDocViewWin()
{
    return 0;
}

//----------------------------------------------------------------------------

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
