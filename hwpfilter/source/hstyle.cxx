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

#include <comphelper/newarray.hxx>

#include "hwplib.h"
#include "hwpfile.h"
#include "hstyle.h"

enum
{ MAXSTYLENAME = 20 };

#define DATA static_cast<StyleData *>(style)

struct StyleData
{
    char name[MAXSTYLENAME + 1];
    CharShape cshape;
    ParaShape pshape;
};

static char buffer[MAXSTYLENAME + 1];

HWPStyle::HWPStyle()
{
    nstyles = 0;
    style = nullptr;
}


HWPStyle::~HWPStyle()
{
    delete[]DATA;
    nstyles = 0;
}


char *HWPStyle::GetName(int n) const
{
    if (!(n >= 0 && n < nstyles))
        return nullptr;
    return DATA[n].name;
}


void HWPStyle::SetName(int n, char const *name)
{
    if (n >= 0 && n < nstyles)
    {
        if (name)
        {
#if defined __GNUC__ && __GNUC__ == 8 && __GNUC_MINOR__ == 2 && !defined __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
            auto const p = DATA[n].name;
            strncpy(p, name, MAXSTYLENAME);
            p[MAXSTYLENAME] = '\0'; // just in case, even though the array is zero-initialized
#if defined __GNUC__ && __GNUC__ == 8 && __GNUC_MINOR__ == 2 && !defined __clang__
#pragma GCC diagnostic pop
#endif
        }
        else
            DATA[n].name[0] = 0;
    }
}


CharShape *HWPStyle::GetCharShape(int n) const
{
    if (!(n >= 0 && n < nstyles))
        return nullptr;
    return &DATA[n].cshape;
}


void HWPStyle::SetCharShape(int n, CharShape const * cshapep)
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
        return nullptr;
    return &DATA[n].pshape;
}


void HWPStyle::SetParaShape(int n, ParaShape const * pshapep)
{
    if (n >= 0 && n < nstyles)
    {
        if (pshapep)
            DATA[n].pshape = *pshapep;
        else
            DATA[n].pshape = ParaShape();
    }
}


void HWPStyle::Read(HWPFile & hwpf)
{
    CharShape cshape;
    ParaShape pshape;

    hwpf.Read2b(&nstyles, 1);
    style = ::comphelper::newArray_null<StyleData>(nstyles);
    if (!style)
        return;

    for (int ii = 0; ii < nstyles; ii++)
    {
        hwpf.ReadBlock(buffer, MAXSTYLENAME);
        cshape.Read(hwpf);
        pshape.Read(hwpf);

        SetName(ii, buffer);
        SetCharShape(ii, &cshape);
        SetParaShape(ii, &pshape);
        if (hwpf.State())
            return;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
