/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dxfblkrd.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:49:24 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_goodies.hxx"

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



