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

#ifndef _DXFBLKRD_HXX
#define _DXFBLKRD_HXX

#include <dxfentrd.hxx>

//----------------------------------------------------------------------------
//---------------- Ein Block (= Menge von Entities) --------------------------
//----------------------------------------------------------------------------

class DXFBlock : public DXFEntities {

public:

    DXFBlock * pSucc;
        // pointer to the nex block in the list DXFBlocks::pFirst

    // properties of blocks; commented with group codes:
    char sName[DXF_MAX_STRING_LEN+1];     //  2
    char sAlsoName[DXF_MAX_STRING_LEN+1]; //  3
    long nFlags;                          // 70
    DXFVector aBasePoint;                 // 10,20,30
    char sXRef[DXF_MAX_STRING_LEN+1];     //  1

    DXFBlock();
    ~DXFBlock();

    void Read(DXFGroupReader & rDGR);
        // reads the block (including entities) from a dxf file
        // by rGDR until a ENDBLK, ENDSEC oder EOF.
};


//----------------------------------------------------------------------------
//---------------- Eine Menge von Bloecken -----------------------------------
//----------------------------------------------------------------------------

class DXFBlocks {

public:

    DXFBlock * pFirst;
        // list of blocks, READ ONLY!

    DXFBlocks();
    ~DXFBlocks();

    void Read(DXFGroupReader & rDGR);
        // reads all block per rDGR until a ENDSEC oder EOF.

    DXFBlock * Search(const char * sName) const;
        // looks for a block with the name, return NULL if not successful

    void Clear();
        // deletes all blocks

};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
