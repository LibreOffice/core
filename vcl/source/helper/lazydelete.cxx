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

#include <vcl/lazydelete.hxx>
#include <svdata.hxx>
#include <sal/log.hxx>

namespace vcl
{
DeleteOnDeinitBase::~DeleteOnDeinitBase()
{
    ImplSVData* pSVData = ImplGetSVData();
    if (!pSVData)
        return;
    auto& rList = pSVData->maDeinitDeleteList;
    rList.erase(std::remove(rList.begin(), rList.end(), this), rList.end());
}

void DeleteOnDeinitBase::addDeinitContainer(DeleteOnDeinitBase* i_pContainer)
{
    ImplSVData* pSVData = ImplGetSVData();

    SAL_WARN_IF(pSVData->mbDeInit, "vcl", "DeleteOnDeinit added after DeiInitVCL !");
    if (pSVData->mbDeInit)
        return;

    pSVData->maDeinitDeleteList.push_back(i_pContainer);
}

void DeleteOnDeinitBase::ImplDeleteOnDeInit()
{
    ImplSVData* pSVData = ImplGetSVData();
    for (auto const& deinitDelete : pSVData->maDeinitDeleteList)
    {
        deinitDelete->doCleanup();
    }
    pSVData->maDeinitDeleteList.clear();
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
