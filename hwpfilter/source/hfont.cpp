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



/* $Id: hfont.cpp,v 1.4 2008-06-04 09:56:49 vg Exp $ */

#include "precompile.h"
#include "hwplib.h"
#include "hwpfile.h"
#include "hfont.h"
/* 이 함수는 HWP 파일을 해석하는 부분이다. */

HWPFont::HWPFont(void)
{
    for (int ii = 0; ii < NLanguage; ii++)
    {
        nFonts[ii] = 0;
        fontnames[ii] = NULL;
    }
}


HWPFont::~HWPFont(void)
{
    for (int ii = 0; ii < NLanguage; ii++)
    {
        nFonts[ii] = 0;
        delete[]fontnames[ii];
    }
}


int HWPFont::AddFont(int lang, const char *font)
{
    int nfonts;

    if (!(lang >= 0 && lang < NLanguage))
        return 0;
    nfonts = nFonts[lang];
    if (MAXFONTS <= nfonts)
        return 0;
    strncpy(fontnames[lang] + FONTNAMELEN * nfonts, font, FONTNAMELEN - 1);
    nFonts[lang]++;
    return nfonts;
}


const char *HWPFont::GetFontName(int lang, int id)
{
    if (!(lang >= 0 && lang < NLanguage))
        return 0;
    if (id < 0 || nFonts[lang] <= id)
        return 0;
    return fontnames[lang] + id * FONTNAMELEN;
}


static char buffer[FONTNAMELEN];

bool HWPFont::Read(HWPFile & hwpf)
{
    int lang = 0;
    short nfonts = 0;

//printf("HWPFont::Read : lang = %d\n",NLanguage);
    for(lang = 0; lang < NLanguage; lang++)
    {
        hwpf.Read2b(&nfonts, 1);
        if (!(nfonts > 0 && nfonts < MAXFONTS))
        {
            return !hwpf.SetState(HWP_InvalidFileFormat);
        }
        fontnames[lang] = new char[nfonts * FONTNAMELEN];

        memset(fontnames[lang], 0, nfonts * FONTNAMELEN);
        for (int jj = 0; jj < nfonts; jj++)
        {
            hwpf.ReadBlock(buffer, FONTNAMELEN);
            AddFont(lang, buffer);
        }
    }

    return !hwpf.State();
}
