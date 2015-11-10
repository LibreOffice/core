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


#include <string.h>
#include <dxfblkrd.hxx>



//---------------- DXFBlock --------------------------------------------------



DXFBlock::DXFBlock()
    : pSucc(nullptr)
    , nFlags(0)
{
}


DXFBlock::~DXFBlock()
{
}


void DXFBlock::Read(DXFGroupReader & rDGR)
{
    m_sName = "";
    m_sAlsoName = "";
    aBasePoint.fx=0.0;
    aBasePoint.fy=0.0;
    aBasePoint.fz=0.0;
    nFlags=0;
    m_sXRef = "";

    while (rDGR.Read()!=0)
    {
        switch (rDGR.GetG())
        {
            case  2: m_sName = rDGR.GetS(); break;
            case  3: m_sAlsoName = rDGR.GetS(); break;
            case 70: nFlags=rDGR.GetI(); break;
            case 10: aBasePoint.fx=rDGR.GetF(); break;
            case 20: aBasePoint.fy=rDGR.GetF(); break;
            case 30: aBasePoint.fz=rDGR.GetF(); break;
            case  1: m_sXRef = rDGR.GetS(); break;
        }
    }
    DXFEntities::Read(rDGR);
}



//---------------- DXFBlocks -------------------------------------------------



DXFBlocks::DXFBlocks()
{
    pFirst=nullptr;
}


DXFBlocks::~DXFBlocks()
{
    Clear();
}


void DXFBlocks::Read(DXFGroupReader & rDGR)
{
    DXFBlock * pB, * * ppSucc;

    ppSucc=&pFirst;
    while (*ppSucc!=nullptr) ppSucc=&((*ppSucc)->pSucc);

    for (;;) {
        while (rDGR.GetG()!=0) rDGR.Read();
        if (rDGR.GetS() == "ENDSEC" ||
            rDGR.GetS() == "EOF") break;
        if (rDGR.GetS() == "BLOCK") {
            pB=new DXFBlock;
            pB->Read(rDGR);
            *ppSucc=pB;
            ppSucc=&(pB->pSucc);
        }
        else rDGR.Read();
    }
}


DXFBlock * DXFBlocks::Search(OString const& rName) const
{
    DXFBlock * pB;
    for (pB=pFirst; pB!=nullptr; pB=pB->pSucc) {
        if (rName == pB->m_sName) break;
    }
    return pB;
}


void DXFBlocks::Clear()
{
    DXFBlock * ptmp;

    while (pFirst!=nullptr) {
        ptmp=pFirst;
        pFirst=ptmp->pSucc;
        delete ptmp;
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
