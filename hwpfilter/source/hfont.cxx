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

#include "precompile.h"
#include "hwplib.h"
#include "hwpfile.h"
#include "hfont.h"
/* 이 함수는 HWP 파일을 해석하는 부분이다. */

HWPFont::HWPFont()
{
    for (int ii = 0; ii < NLanguage; ii++)
    {
        nFonts[ii] = 0;
        fontnames[ii] = nullptr;
    }
}


HWPFont::~HWPFont()
{
}


void HWPFont::AddFont(int lang, const char *font)
{
    int nfonts;

    if (!(lang >= 0 && lang < NLanguage))
        return;
    nfonts = nFonts[lang];
    if (MAXFONTS <= nfonts)
        return;
    auto const p = fontnames[lang].get() + FONTNAMELEN * nfonts;
    strncpy(p, font, FONTNAMELEN - 1);
    p[FONTNAMELEN - 1] = '\0'; // just in case, even though the array is zero-initialized
    nFonts[lang]++;
}


const char *HWPFont::GetFontName(int lang, int id)
{
    if (!(lang >= 0 && lang < NLanguage))
        return nullptr;
    if (id < 0 || nFonts[lang] <= id)
        return nullptr;
    return fontnames[lang].get() + id * FONTNAMELEN;
}


static char buffer[FONTNAMELEN];

void HWPFont::Read(HWPFile & hwpf)
{
    int lang = 0;
    short nfonts = 0;

//printf("HWPFont::Read : lang = %d\n",NLanguage);
    for(lang = 0; lang < NLanguage; lang++)
    {
        hwpf.Read2b(&nfonts, 1);
        if (!(nfonts > 0 && nfonts < MAXFONTS))
        {
            (void)hwpf.SetState(HWP_InvalidFileFormat);
            return;
        }
        fontnames[lang].reset(new char[nfonts * FONTNAMELEN]);

        memset(fontnames[lang].get(), 0, nfonts * FONTNAMELEN);
        for (int jj = 0; jj < nfonts; jj++)
        {
            hwpf.ReadBlock(buffer, FONTNAMELEN);
            AddFont(lang, buffer);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
