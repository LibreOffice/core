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



/* $Id: hstyle.cpp,v 1.3 2008-04-10 12:07:14 rt Exp $ */

#define NOMINMAX
#include "precompile.h"

#include <comphelper/newarray.hxx>

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
    style = ::comphelper::newArray_null<StyleData>(nstyles);
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
