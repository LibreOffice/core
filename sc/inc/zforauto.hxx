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


