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

/* $Id: hstyle.cpp,v 1.3 2008-04-10 12:07:14 rt Exp $ */

#include "precompile.h"

#include    "hwplib.h"
#include    "hwpfile.h"
#include    "hstyle.h"

enum
{ MAXSTYLENAME = 20 };

#define DATA ((StyleData *)style)

struct StyleData
{
    char name[MAXSTYLENAME + 1];
    CharShape cshape;
    ParaShape pshape;
};

static char buffer[MAXSTYLENAME + 1];

HWPStyle::HWPStyle(void)
{
    nstyles = 0;
    style = 0;
}


HWPStyle::~HWPStyle(void)
{
    delete[]DATA;
    nstyles = 0;
}


int HWPStyle::Num(void) const
{
    return nstyles;
}


char *HWPStyle::GetName(int n) const
{
    if (!(n >= 0 && n < nstyles))
        return 0;
    return DATA[n].name;
}


void HWPStyle::SetName(int n, char *name)
{
    if (n >= 0 && n < nstyles)
    {
        if (name)
            strncpy(DATA[n].name, name, MAXSTYLENAME);
        else
            DATA[n].name[0] = 0;
    }
}


CharShape *HWPStyle::GetCharShape(int n) const
{
    if (!(n >= 0 && n < nstyles))
        return 0;
    return &DATA[n].cshape;
}


void HWPStyle::SetCharShape(int n, CharShape * cshapep)
{
    if (n >= 0 && n < nstyles)
    {
        if (cshapep)
            DATA[n].cshape = *cshapep;
        else
            memset(&DATA[n].cshape, 0, sizeof(CharShape));
    }
}


ParaShape *HWPStyle::GetParaShape(int n) const
{
    if (!(n >= 0 && n < nstyles))
        return 0;
    return &DATA[n].pshape;
}


void HWPStyle::SetParaShape(int n, ParaShape * pshapep)
{
    if (n >= 0 && n < nstyles)
    {
        if (pshapep)
            DATA[n].pshape = *pshapep;
        else
            memset(&DATA[n].pshape, 0, sizeof(ParaShape));
    }
}


bool HWPStyle::Read(HWPFile & hwpf)
{
    CharShape cshape;
    ParaShape pshape;

    hwpf.Read2b(&nstyles, 1);
    style = new StyleData[nstyles];
    if (!style)
        return false;

    for (int ii = 0; ii < nstyles; ii++)
    {
        hwpf.ReadBlock(buffer, MAXSTYLENAME);
        cshape.Read(hwpf);
        pshape.Read(hwpf);

        SetName(ii, buffer);
        SetCharShape(ii, &cshape);
        SetParaShape(ii, &pshape);
        if (hwpf.State())
            return false;
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
