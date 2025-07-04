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

#pragma once

#include <swtypes.hxx>
#include <string.h>

constexpr tools::Long MD_NUMBER_BULLET_MARGINLEFT = o3tl::toTwips(125, o3tl::Length::mm10);
constexpr tools::Long MD_NUMBER_BULLET_INDENT = -o3tl::toTwips(5, o3tl::Length::mm);

class SwTextNode;
class SwNumRule;

class SwMdNumRuleInfo
{
    sal_uInt16 m_aNumStarts[MAXLEVEL];
    SwNumRule* m_pNumRule; // current numbering
    sal_uInt16 m_nDeep; // current numbering depth (1, 2, 3, ...)
    bool m_bRestart; // Export: restart numbering
    bool m_bNumbered; // Export: paragraph is numbered

public:
    SwMdNumRuleInfo()
        : m_pNumRule(nullptr)
        , m_nDeep(0)
        , m_bRestart(false)
        , m_bNumbered(false)
    {
        memset(&m_aNumStarts, 0xff, sizeof(m_aNumStarts));
    }

    SwMdNumRuleInfo(const SwMdNumRuleInfo& rInf)
        : m_pNumRule(rInf.m_pNumRule)
        , m_nDeep(rInf.m_nDeep)
        , m_bRestart(rInf.m_bRestart)
        , m_bNumbered(rInf.m_bNumbered)
    {
        memcpy(&m_aNumStarts, &rInf.m_aNumStarts, sizeof(m_aNumStarts));
    }

    inline void Set(const SwMdNumRuleInfo& rInf);
    void Set(const SwTextNode& rTextNd);

    explicit SwMdNumRuleInfo(const SwTextNode& rTextNd) { Set(rTextNd); }
    inline SwMdNumRuleInfo& operator=(const SwMdNumRuleInfo& rInf);

    inline void Clear();

    void SetNumRule(const SwNumRule* pRule) { m_pNumRule = const_cast<SwNumRule*>(pRule); }
    SwNumRule* GetNumRule() { return m_pNumRule; }
    const SwNumRule* GetNumRule() const { return m_pNumRule; }

    void SetDepth(sal_uInt16 nDepth) { m_nDeep = nDepth; }
    sal_uInt16 GetDepth() const { return m_nDeep; }
    void IncDepth() { ++m_nDeep; }
    void DecDepth()
    {
        if (m_nDeep != 0)
            --m_nDeep;
    }
    inline sal_uInt8 GetLevel() const;

    bool IsRestart(const SwMdNumRuleInfo& rPrev) const;

    bool IsNumbered() const { return m_bNumbered; }

    inline void SetNodeStartValue(sal_uInt8 nLvl, sal_uInt16 nVal = USHRT_MAX);
    sal_uInt16 GetNodeStartValue(sal_uInt8 nLvl) const { return m_aNumStarts[nLvl]; }
};

inline SwMdNumRuleInfo& SwMdNumRuleInfo::operator=(const SwMdNumRuleInfo& rInf)
{
    Set(rInf);
    return *this;
}

inline void SwMdNumRuleInfo::Set(const SwMdNumRuleInfo& rInf)
{
    m_pNumRule = rInf.m_pNumRule;
    m_nDeep = rInf.m_nDeep;
    m_bNumbered = rInf.m_bNumbered;
    m_bRestart = rInf.m_bRestart;
    memcpy(&m_aNumStarts, &rInf.m_aNumStarts, sizeof(m_aNumStarts));
}

inline void SwMdNumRuleInfo::Clear()
{
    m_pNumRule = nullptr;
    m_nDeep = 0;
    m_bRestart = m_bNumbered = false;
    memset(&m_aNumStarts, 0xff, sizeof(m_aNumStarts));
}

inline sal_uInt8 SwMdNumRuleInfo::GetLevel() const
{
    return static_cast<sal_uInt8>(m_pNumRule != nullptr && m_nDeep != 0
                                      ? (m_nDeep <= MAXLEVEL ? m_nDeep - 1 : MAXLEVEL - 1)
                                      : 0);
}

inline void SwMdNumRuleInfo::SetNodeStartValue(sal_uInt8 nLvl, sal_uInt16 nVal)
{
    m_aNumStarts[nLvl] = nVal;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
