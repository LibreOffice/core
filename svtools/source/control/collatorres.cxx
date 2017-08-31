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

#include <svtools/svtresid.hxx>
#include <svtools/strings.hrc>
#include <svtools/collatorres.hxx>

//  implementation of the collator-algorithm-name translation
CollatorResource::CollatorResource()
{
    m_aData.emplace_back("alphanumeric", SvtResId(STR_SVT_COLLATE_ALPHANUMERIC));
    m_aData.emplace_back("charset", SvtResId(STR_SVT_COLLATE_CHARSET));
    m_aData.emplace_back("dict", SvtResId(STR_SVT_COLLATE_DICTIONARY));
    m_aData.emplace_back("normal", SvtResId(STR_SVT_COLLATE_NORMAL));
    m_aData.emplace_back("pinyin", SvtResId(STR_SVT_COLLATE_PINYIN));
    m_aData.emplace_back("radical", SvtResId(STR_SVT_COLLATE_RADICAL));
    m_aData.emplace_back("stroke", SvtResId(STR_SVT_COLLATE_STROKE));
    m_aData.emplace_back("unicode", SvtResId(STR_SVT_COLLATE_UNICODE));
    m_aData.emplace_back("zhuyin", SvtResId(STR_SVT_COLLATE_ZHUYIN));
    m_aData.emplace_back("phonebook", SvtResId(STR_SVT_COLLATE_PHONEBOOK));
    m_aData.emplace_back("phonetic (alphanumeric first)", SvtResId(STR_SVT_COLLATE_PHONETIC_F));
    m_aData.emplace_back("phonetic (alphanumeric last)", SvtResId(STR_SVT_COLLATE_PHONETIC_L));
}

const OUString&
CollatorResource::GetTranslation(const OUString &r_Algorithm)
{
    sal_Int32 nIndex = r_Algorithm.indexOf('.');
    OUString aLocaleFreeAlgorithm;

    if (nIndex == -1)
    {
        aLocaleFreeAlgorithm = r_Algorithm;
    }
    else
    {
        nIndex += 1;
        aLocaleFreeAlgorithm = r_Algorithm.copy(nIndex);
    }

    for (size_t i = 0; i < m_aData.size(); ++i)
    {
        if (aLocaleFreeAlgorithm == m_aData[i].GetAlgorithm())
            return m_aData[i].GetTranslation();
    }

    return r_Algorithm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
