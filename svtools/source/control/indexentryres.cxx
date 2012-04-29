/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <svtools/svtresid.hxx>
#include <svtools/svtools.hrc>
#include <svtools/indexentryres.hxx>

// -------------------------------------------------------------------------
//
//  wrapper for locale specific translations data of indexentry algorithm
//
// -------------------------------------------------------------------------

class IndexEntryRessourceData
{
    friend class IndexEntryRessource;
    private: /* data */
        rtl::OUString  ma_Name;
        rtl::OUString  ma_Translation;
    private: /* member functions */
        IndexEntryRessourceData () {}
    public:
        IndexEntryRessourceData ( const rtl::OUString &r_Algorithm, const rtl::OUString &r_Translation)
                : ma_Name (r_Algorithm), ma_Translation (r_Translation) {}

        const rtl::OUString& GetAlgorithm () const { return ma_Name; }

        const rtl::OUString& GetTranslation () const { return ma_Translation; }

        ~IndexEntryRessourceData () {}

        IndexEntryRessourceData& operator= (const IndexEntryRessourceData& r_From)
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

#define INDEXENTRY_RESSOURCE_COUNT (STR_SVT_INDEXENTRY_END - STR_SVT_INDEXENTRY_START + 1)

IndexEntryRessource::IndexEntryRessource()
{
        mp_Data = new IndexEntryRessourceData[INDEXENTRY_RESSOURCE_COUNT];

        #define ASCSTR(str) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(str))
        #define RESSTR(rid) SvtResId(rid).toString()

        mp_Data[STR_SVT_INDEXENTRY_ALPHANUMERIC - STR_SVT_INDEXENTRY_START] =
        IndexEntryRessourceData (ASCSTR("alphanumeric"), RESSTR(STR_SVT_INDEXENTRY_ALPHANUMERIC));
        mp_Data[STR_SVT_INDEXENTRY_DICTIONARY - STR_SVT_INDEXENTRY_START] =
        IndexEntryRessourceData (ASCSTR("dict"), RESSTR(STR_SVT_INDEXENTRY_DICTIONARY));
        mp_Data[STR_SVT_INDEXENTRY_PINYIN - STR_SVT_INDEXENTRY_START] =
        IndexEntryRessourceData (ASCSTR("pinyin"), RESSTR(STR_SVT_INDEXENTRY_PINYIN));
        mp_Data[STR_SVT_INDEXENTRY_PINYIN - STR_SVT_INDEXENTRY_START] =
        IndexEntryRessourceData (ASCSTR("radical"), RESSTR(STR_SVT_INDEXENTRY_RADICAL));
        mp_Data[STR_SVT_INDEXENTRY_STROKE - STR_SVT_INDEXENTRY_START] =
        IndexEntryRessourceData (ASCSTR("stroke"), RESSTR(STR_SVT_INDEXENTRY_STROKE));
        mp_Data[STR_SVT_INDEXENTRY_STROKE - STR_SVT_INDEXENTRY_START] =
        IndexEntryRessourceData (ASCSTR("zhuyin"), RESSTR(STR_SVT_INDEXENTRY_ZHUYIN));
        mp_Data[STR_SVT_INDEXENTRY_ZHUYIN - STR_SVT_INDEXENTRY_START] =
        IndexEntryRessourceData (ASCSTR("phonetic (alphanumeric first) (grouped by syllable)"),
                    RESSTR(STR_SVT_INDEXENTRY_PHONETIC_FS));
        mp_Data[STR_SVT_INDEXENTRY_PHONETIC_FS - STR_SVT_INDEXENTRY_START] =
        IndexEntryRessourceData (ASCSTR("phonetic (alphanumeric first) (grouped by consonant)"),
                    RESSTR(STR_SVT_INDEXENTRY_PHONETIC_FC));
        mp_Data[STR_SVT_INDEXENTRY_PHONETIC_FC - STR_SVT_INDEXENTRY_START] =
        IndexEntryRessourceData (ASCSTR("phonetic (alphanumeric last) (grouped by syllable)"),
                    RESSTR(STR_SVT_INDEXENTRY_PHONETIC_LS));
        mp_Data[STR_SVT_INDEXENTRY_PHONETIC_LS - STR_SVT_INDEXENTRY_START] =
        IndexEntryRessourceData (ASCSTR("phonetic (alphanumeric last) (grouped by consonant)"),
                    RESSTR(STR_SVT_INDEXENTRY_PHONETIC_LC));
}

IndexEntryRessource::~IndexEntryRessource()
{
    delete[] mp_Data;
}

const rtl::OUString& IndexEntryRessource::GetTranslation(const rtl::OUString &r_Algorithm)
{
    sal_Int32 nIndex = r_Algorithm.indexOf('.');
    rtl::OUString aLocaleFreeAlgorithm;

    if (nIndex == -1)
        aLocaleFreeAlgorithm = r_Algorithm;
    else {
        nIndex += 1;
        aLocaleFreeAlgorithm = r_Algorithm.copy(nIndex, r_Algorithm.getLength() - nIndex);
    }

    for (sal_uInt32 i = 0; i < INDEXENTRY_RESSOURCE_COUNT; i++)
        if (aLocaleFreeAlgorithm == mp_Data[i].GetAlgorithm())
            return mp_Data[i].GetTranslation();
    return r_Algorithm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
