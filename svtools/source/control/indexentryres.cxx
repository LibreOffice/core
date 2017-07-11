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
#include <svtools/svtools.hrc>
#include <svtools/indexentryres.hxx>

//  implementation of the indexentry-algorithm-name translation
IndexEntryResource::IndexEntryResource()
{
    m_aData.push_back(IndexEntryResourceData("alphanumeric", SvtResId(STR_SVT_INDEXENTRY_ALPHANUMERIC)));
    m_aData.push_back(IndexEntryResourceData("dict", SvtResId(STR_SVT_INDEXENTRY_DICTIONARY)));
    m_aData.push_back(IndexEntryResourceData("pinyin", SvtResId(STR_SVT_INDEXENTRY_PINYIN)));
    m_aData.push_back(IndexEntryResourceData("radical", SvtResId(STR_SVT_INDEXENTRY_RADICAL)));
    m_aData.push_back(IndexEntryResourceData("stroke", SvtResId(STR_SVT_INDEXENTRY_STROKE)));
    m_aData.push_back(IndexEntryResourceData("zhuyin", SvtResId(STR_SVT_INDEXENTRY_ZHUYIN)));
    m_aData.push_back(IndexEntryResourceData("phonetic (alphanumeric first) (grouped by syllable)", SvtResId(STR_SVT_INDEXENTRY_PHONETIC_FS)));
    m_aData.push_back(IndexEntryResourceData("phonetic (alphanumeric first) (grouped by consonant)", SvtResId(STR_SVT_INDEXENTRY_PHONETIC_FC)));
    m_aData.push_back(IndexEntryResourceData("phonetic (alphanumeric last) (grouped by syllable)", SvtResId(STR_SVT_INDEXENTRY_PHONETIC_LS)));
    m_aData.push_back(IndexEntryResourceData("phonetic (alphanumeric last) (grouped by consonant)", SvtResId(STR_SVT_INDEXENTRY_PHONETIC_LC)));
}

const OUString& IndexEntryResource::GetTranslation(const OUString &r_Algorithm)
{
    sal_Int32 nIndex = r_Algorithm.indexOf('.');
    OUString aLocaleFreeAlgorithm;

    if (nIndex == -1)
        aLocaleFreeAlgorithm = r_Algorithm;
    else {
        nIndex += 1;
        aLocaleFreeAlgorithm = r_Algorithm.copy(nIndex);
    }

    for (size_t i = 0; i < m_aData.size(); ++i)
        if (aLocaleFreeAlgorithm == m_aData[i].GetAlgorithm())
            return m_aData[i].GetTranslation();
    return r_Algorithm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
