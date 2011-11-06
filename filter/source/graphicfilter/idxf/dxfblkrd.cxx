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



