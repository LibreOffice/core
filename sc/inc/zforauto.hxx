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

#ifndef _ZFORAUTO_HXX_
#define _ZFORAUTO_HXX_

#include <tools/string.hxx>
#include <i18nlangtag/lang.h>


class SvStream;
class SvNumberFormatter;


class ScNumFormatAbbrev
{
    OUString sFormatstring;
    LanguageType eLnge;
    LanguageType eSysLnge;
public:
    ScNumFormatAbbrev();
    ScNumFormatAbbrev(const ScNumFormatAbbrev& aFormat);
    ScNumFormatAbbrev(sal_uLong nFormat, SvNumberFormatter& rFormatter);
    void Load( SvStream& rStream, CharSet eByteStrSet );            // loading of the numberformats
    void Save( SvStream& rStream, CharSet eByteStrSet ) const;   // saving of the numberformats
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
