/*************************************************************************
 *
 *  $RCSfile: hstyle.cpp,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:40:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2001 by Mizi Research Inc.
 *  Copyright 2003 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Mizi Research Inc.
 *
 *  Copyright: 2001 by Mizi Research Inc.
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* $Id: hstyle.cpp,v 1.1 2003-10-15 14:40:42 dvo Exp $ */

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
