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

#include <memory>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <com/sun/star/i18n/KParseTokens.hpp>
#include <com/sun/star/i18n/KParseType.hpp>
#include <i18nlangtag/lang.h>
#include <tools/lineend.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/syslocale.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <rtl/character.hxx>
#include <node.hxx>
#include <parse6.hxx>
#include <strings.hrc>
#include <smmod.hxx>
#include <cfgitem.hxx>
#include <cassert>
#include <stack>
#include <starmathdatabase.hxx>

using namespace ::com::sun::star::i18n;

SmParser6::SmParser6()
    : AbstractSmParser()
    , m_nCurError(0)
    , m_nBufferIndex(0)
    , m_nTokenIndex(0)
    , m_nRow(0)
    , m_nColOff(0)
    , m_bImportSymNames(false)
    , m_bExportSymNames(false)
    , m_nParseDepth(0)
    , m_aNumCC(LanguageTag(LANGUAGE_ENGLISH_US))
    , m_pSysCC(SM_MOD()->GetSysLocale().GetCharClassPtr())
{
}

SmParser6::~SmParser6() {}

std::unique_ptr<SmTableNode> SmParser6::Parse(const OUString& rBuffer)
{
    m_aBufferString = rBuffer;
    return nullptr;
}

std::unique_ptr<SmNode> SmParser6::ParseExpression(const OUString& rBuffer)
{
    m_aBufferString = rBuffer;
    return nullptr;
}

const SmErrorDesc* SmParser6::NextError()
{
    if (!m_aErrDescList.empty())
        if (m_nCurError > 0)
            return &m_aErrDescList[--m_nCurError];
        else
        {
            m_nCurError = 0;
            return &m_aErrDescList[m_nCurError];
        }
    else
        return nullptr;
}

const SmErrorDesc* SmParser6::PrevError()
{
    if (!m_aErrDescList.empty())
        if (m_nCurError < static_cast<int>(m_aErrDescList.size() - 1))
            return &m_aErrDescList[++m_nCurError];
        else
        {
            m_nCurError = static_cast<int>(m_aErrDescList.size() - 1);
            return &m_aErrDescList[m_nCurError];
        }
    else
        return nullptr;
}

const SmErrorDesc* SmParser6::GetError() const
{
    if (m_aErrDescList.empty())
        return nullptr;
    return &m_aErrDescList.front();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
