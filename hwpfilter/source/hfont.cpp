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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
