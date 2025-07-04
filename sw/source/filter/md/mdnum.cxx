/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <doc.hxx>
#include <ndtxt.hxx>

#include "mdnum.hxx"

void SwMdNumRuleInfo::Set(const SwTextNode& rTextNd)
{
    const SwNumRule* pTextNdNumRule(rTextNd.GetNumRule());
    if (pTextNdNumRule && pTextNdNumRule != rTextNd.GetDoc().GetOutlineNumRule())
    {
        m_pNumRule = const_cast<SwNumRule*>(pTextNdNumRule);
        m_nDeep = o3tl::narrowing<sal_uInt16>(m_pNumRule ? rTextNd.GetActualListLevel() + 1 : 0);
        m_bNumbered = rTextNd.IsCountedInList();
        m_bRestart = rTextNd.IsListRestart() && !rTextNd.HasAttrListRestartValue();
    }
    else
    {
        m_pNumRule = nullptr;
        m_nDeep = 0;
        m_bNumbered = m_bRestart = false;
    }
}

bool SwMdNumRuleInfo::IsRestart(const SwMdNumRuleInfo& rPrev) const
{
    assert(rPrev.GetNumRule() == GetNumRule());

    if (rPrev.GetDepth() < GetDepth())
        return false;
    return m_bRestart;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
