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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <string.h>
#include <dxfblkrd.hxx>


//----------------------------------------------------------------------------
//---------------- DXFBlock --------------------------------------------------
//----------------------------------------------------------------------------


DXFBlock::DXFBlock()
{
    pSucc=NULL;
}


DXFBlock::~DXFBlock()
{
}


void DXFBlock::Read(DXFGroupReader & rDGR)
{
    sName[0]=0;
    sAlsoName[0]=0;
    aBasePoint.fx=0.0;
    aBasePoint.fy=0.0;
    aBasePoint.fz=0.0;
    nFlags=0;
    sXRef[0]=0;

    while (rDGR.Read()!=0)
    {
        switch (rDGR.GetG())
        {
            case  2: strncpy( sName, rDGR.GetS(), DXF_MAX_STRING_LEN + 1 ); break;
            case  3: strncpy( sAlsoName, rDGR.GetS(), DXF_MAX_STRING_LEN + 1 ); break;
            case 70: nFlags=rDGR.GetI(); break;
            case 10: aBasePoint.fx=rDGR.GetF(); break;
            case 20: aBasePoint.fy=rDGR.GetF(); break;
            case 30: aBasePoint.fz=rDGR.GetF(); break;
            case  1: strncpy( sXRef, rDGR.GetS(), DXF_MAX_STRING_LEN + 1 ); break;
        }
    }
    DXFEntities::Read(rDGR);
}


//----------------------------------------------------------------------------
//---------------- DXFBlocks -------------------------------------------------
//----------------------------------------------------------------------------


DXFBlocks::DXFBlocks()
{
    pFirst=NULL;
}


DXFBlocks::~DXFBlocks()
{
    Clear();
}


void DXFBlocks::Read(DXFGroupReader & rDGR)
{
    DXFBlock * pB, * * ppSucc;

    ppSucc=&pFirst;
    while (*ppSucc!=NULL) ppSucc=&((*ppSucc)->pSucc);

    for (;;) {
        while (rDGR.GetG()!=0) rDGR.Read();
        if (strcmp(rDGR.GetS(),"ENDSEC")==0 ||
            strcmp(rDGR.GetS(),"EOF")==0) break;
        if (strcmp(rDGR.GetS(),"BLOCK")==0) {
            pB=new DXFBlock;
            pB->Read(rDGR);
            *ppSucc=pB;
            ppSucc=&(pB->pSucc);
        }
        else rDGR.Read();
    }
}


DXFBlock * DXFBlocks::Search(const char * sName) const
{
    DXFBlock * pB;
    for (pB=pFirst; pB!=NULL; pB=pB->pSucc) {
        if (strcmp(sName,pB->sName)==0) break;
    }
    return pB;
}


void DXFBlocks::Clear()
{
    DXFBlock * ptmp;

    while (pFirst!=NULL) {
        ptmp=pFirst;
        pFirst=ptmp->pSucc;
        delete ptmp;
    }
}



