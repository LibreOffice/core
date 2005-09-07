/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hfont.cpp,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:33:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/* $Id: hfont.cpp,v 1.2 2005-09-07 16:33:43 rt Exp $ */

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


int HWPFont::NFonts(int lang)
{
    if (!(lang >= 0 && lang < NLanguage))
        return 0;
    return nFonts[lang];
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
