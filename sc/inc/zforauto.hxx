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



#ifndef _ZFORAUTO_HXX_
#define _ZFORAUTO_HXX_

#include <tools/string.hxx>
#include <i18npool/lang.h>


class SvStream;
class SvNumberFormatter;


class ScNumFormatAbbrev
{
    String sFormatstring;
    LanguageType eLnge;
    LanguageType eSysLnge;
public:
    ScNumFormatAbbrev();
    ScNumFormatAbbrev(const ScNumFormatAbbrev& aFormat);
    ScNumFormatAbbrev(sal_uLong nFormat, SvNumberFormatter& rFormatter);
    void Load( SvStream& rStream, CharSet eByteStrSet );            // Laden der Zahlenformate
    void Save( SvStream& rStream, CharSet eByteStrSet ) const;   // Speichern der Zahlenformate
    void PutFormatIndex(sal_uLong nFormat, SvNumberFormatter& rFormatter);
    sal_uLong GetFormatIndex( SvNumberFormatter& rFormatter);
    inline int operator==(const ScNumFormatAbbrev& rNumFormat) const
    {
        return ((sFormatstring == rNumFormat.sFormatstring)
            && (eLnge == rNumFormat.eLnge)
            && (eSysLnge == rNumFormat.eSysLnge));
    }
    inline ScNumFormatAbbrev& operator=(const ScNumFormatAbbrev& rNumFormat)
    {
        sFormatstring = rNumFormat.sFormatstring;
        eLnge = rNumFormat.eLnge;
        eSysLnge = rNumFormat.eSysLnge;
        return *this;
    }
    inline LanguageType GetLanguage() const { return eLnge; }
};



#endif


