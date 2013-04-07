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

// -------------------------------------------------------------------------
//
//  wrapper for locale specific translations data of indexentry algorithm
//
// -------------------------------------------------------------------------

class IndexEntryResourceData
{
    friend class IndexEntryResource;
    private: /* data */
        OUString  ma_Name;
        OUString  ma_Translation;
    private: /* member functions */
        IndexEntryResourceData () {}
    public:
        IndexEntryResourceData ( const OUString &r_Algorithm, const OUString &r_Translation)
                : ma_Name (r_Algorithm), ma_Translation (r_Translation) {}

        const OUString& GetAlgorithm () const { return ma_Name; }

        const OUString& GetTranslation () const { return ma_Translation; }

        ~IndexEntryResourceData () {}

        IndexEntryResourceData& operator= (const IndexEntryResourceData& r_From)
        {
            ma_Name         = r_From.GetAlgorithm();
            ma_Translation  = r_From.GetTranslation();
            return *this;
        }
};

// -------------------------------------------------------------------------
//
//  implementation of the indexentry-algorithm-name translation
//
// -------------------------------------------------------------------------

#define INDEXENTRY_RESOURCE_COUNT (STR_SVT_INDEXENTRY_END - STR_SVT_INDEXENTRY_START + 1)

IndexEntryResource::IndexEntryResource()
{
        mp_Data = new IndexEntryResourceData[INDEXENTRY_RESOURCE_COUNT];

        #define RESSTR(rid) SvtResId(rid).toString()

        mp_Data[STR_SVT_INDEXENTRY_ALPHANUMERIC - STR_SVT_INDEXENTRY_START] =
        IndexEntryResourceData ("alphanumeric", RESSTR(STR_SVT_INDEXENTRY_ALPHANUMERIC));
        mp_Data[STR_SVT_INDEXENTRY_DICTIONARY - STR_SVT_INDEXENTRY_START] =
        IndexEntryResourceData ("dict", RESSTR(STR_SVT_INDEXENTRY_DICTIONARY));
        mp_Data[STR_SVT_INDEXENTRY_PINYIN - STR_SVT_INDEXENTRY_START] =
        IndexEntryResourceData ("pinyin", RESSTR(STR_SVT_INDEXENTRY_PINYIN));
        mp_Data[STR_SVT_INDEXENTRY_PINYIN - STR_SVT_INDEXENTRY_START] =
        IndexEntryResourceData ("radical", RESSTR(STR_SVT_INDEXENTRY_RADICAL));
        mp_Data[STR_SVT_INDEXENTRY_STROKE - STR_SVT_INDEXENTRY_START] =
        IndexEntryResourceData ("stroke", RESSTR(STR_SVT_INDEXENTRY_STROKE));
        mp_Data[STR_SVT_INDEXENTRY_STROKE - STR_SVT_INDEXENTRY_START] =
        IndexEntryResourceData ("zhuyin", RESSTR(STR_SVT_INDEXENTRY_ZHUYIN));
        mp_Data[STR_SVT_INDEXENTRY_ZHUYIN - STR_SVT_INDEXENTRY_START] =
        IndexEntryResourceData ("phonetic (alphanumeric first) (grouped by syllable)",
                    RESSTR(STR_SVT_INDEXENTRY_PHONETIC_FS));
        mp_Data[STR_SVT_INDEXENTRY_PHONETIC_FS - STR_SVT_INDEXENTRY_START] =
        IndexEntryResourceData ("phonetic (alphanumeric first) (grouped by consonant)",
                    RESSTR(STR_SVT_INDEXENTRY_PHONETIC_FC));
        mp_Data[STR_SVT_INDEXENTRY_PHONETIC_FC - STR_SVT_INDEXENTRY_START] =
        IndexEntryResourceData ("phonetic (alphanumeric last) (grouped by syllable)",
                    RESSTR(STR_SVT_INDEXENTRY_PHONETIC_LS));
        mp_Data[STR_SVT_INDEXENTRY_PHONETIC_LS - STR_SVT_INDEXENTRY_START] =
        IndexEntryResourceData ("phonetic (alphanumeric last) (grouped by consonant)",
                    RESSTR(STR_SVT_INDEXENTRY_PHONETIC_LC));
}

IndexEntryResource::~IndexEntryResource()
{
    delete[] mp_Data;
}

const OUString& IndexEntryResource::GetTranslation(const OUString &r_Algorithm)
{
    sal_Int32 nIndex = r_Algorithm.indexOf('.');
    OUString aLocaleFreeAlgorithm;

    if (nIndex == -1)
        aLocaleFreeAlgorithm = r_Algorithm;
    else {
        nIndex += 1;
        aLocaleFreeAlgorithm = r_Algorithm.copy(nIndex, r_Algorithm.getLength() - nIndex);
    }

    for (sal_uInt32 i = 0; i < INDEXENTRY_RESOURCE_COUNT; i++)
        if (aLocaleFreeAlgorithm == mp_Data[i].GetAlgorithm())
            return mp_Data[i].GetTranslation();
    return r_Algorithm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
