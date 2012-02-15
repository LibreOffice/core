/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#include <svtools/svtdata.hxx>
#include <svtools/svtools.hrc>
#include <svtools/collatorres.hxx>

// -------------------------------------------------------------------------
//
//  wrapper for locale specific translations data of collator algorithm
//
// -------------------------------------------------------------------------

class CollatorRessourceData
{
    friend class CollatorRessource;
    private: /* data */
        String          ma_Name;
        String          ma_Translation;
    private: /* member functions */
        CollatorRessourceData () {}
    public:
        CollatorRessourceData ( const String &r_Algorithm, const String &r_Translation)
                    : ma_Name (r_Algorithm), ma_Translation (r_Translation) {}

        const String&   GetAlgorithm () const { return ma_Name; }

        const String&   GetTranslation () const { return ma_Translation; }

        ~CollatorRessourceData () {}

        CollatorRessourceData& operator= (const CollatorRessourceData& r_From)
        {
            ma_Name         = r_From.GetAlgorithm();
            ma_Translation  = r_From.GetTranslation();
            return *this;
        }
};

// -------------------------------------------------------------------------
//
//  implementation of the collator-algorithm-name translation
//
// -------------------------------------------------------------------------

#define COLLATOR_RESSOURCE_COUNT (STR_SVT_COLLATE_END - STR_SVT_COLLATE_START + 1)

CollatorRessource::CollatorRessource()
{
    mp_Data = new CollatorRessourceData[COLLATOR_RESSOURCE_COUNT];

    #define ASCSTR(str) String(RTL_CONSTASCII_USTRINGPARAM(str))
    #define RESSTR(rid) String(SvtResId(rid))


    mp_Data[0] = CollatorRessourceData (ASCSTR("alphanumeric"), RESSTR(STR_SVT_COLLATE_ALPHANUMERIC));
    mp_Data[1] = CollatorRessourceData (ASCSTR("charset"), RESSTR(STR_SVT_COLLATE_CHARSET));
    mp_Data[2] = CollatorRessourceData (ASCSTR("dict"), RESSTR(STR_SVT_COLLATE_DICTIONARY));
    mp_Data[3] = CollatorRessourceData (ASCSTR("normal"), RESSTR(STR_SVT_COLLATE_NORMAL));
    mp_Data[4] = CollatorRessourceData (ASCSTR("pinyin"), RESSTR(STR_SVT_COLLATE_PINYIN));
    mp_Data[5] = CollatorRessourceData (ASCSTR("radical"), RESSTR(STR_SVT_COLLATE_RADICAL));
    mp_Data[6] = CollatorRessourceData (ASCSTR("stroke"), RESSTR(STR_SVT_COLLATE_STROKE));
    mp_Data[7] = CollatorRessourceData (ASCSTR("unicode"), RESSTR(STR_SVT_COLLATE_UNICODE));
    mp_Data[8] = CollatorRessourceData (ASCSTR("zhuyin"), RESSTR(STR_SVT_COLLATE_ZHUYIN));
    mp_Data[9] = CollatorRessourceData (ASCSTR("phonebook"), RESSTR(STR_SVT_COLLATE_PHONEBOOK));
    mp_Data[10] = CollatorRessourceData (ASCSTR("phonetic (alphanumeric first)"), RESSTR(STR_SVT_COLLATE_PHONETIC_F));
    mp_Data[11] = CollatorRessourceData (ASCSTR("phonetic (alphanumeric last)"), RESSTR(STR_SVT_COLLATE_PHONETIC_L));
}

CollatorRessource::~CollatorRessource()
{
    delete[] mp_Data;
}

const String&
CollatorRessource::GetTranslation (const String &r_Algorithm)
{
    xub_StrLen nIndex = r_Algorithm.Search('.');
    String aLocaleFreeAlgorithm;

    if (nIndex == STRING_NOTFOUND)
    {
        aLocaleFreeAlgorithm = r_Algorithm;
    }
    else
    {
        nIndex += 1;
        aLocaleFreeAlgorithm = String(r_Algorithm, nIndex, r_Algorithm.Len() - nIndex);
    }

    for (sal_uInt32 i = 0; i < COLLATOR_RESSOURCE_COUNT; i++)
    {
        if (aLocaleFreeAlgorithm == mp_Data[i].GetAlgorithm())
            return mp_Data[i].GetTranslation();
    }

    return r_Algorithm;
}

