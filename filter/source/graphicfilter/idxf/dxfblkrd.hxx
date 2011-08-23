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

#ifndef _DXFBLKRD_HXX
#define _DXFBLKRD_HXX

#include <dxfentrd.hxx>

//----------------------------------------------------------------------------
//---------------- Ein Block (= Menge von Entities) --------------------------
//----------------------------------------------------------------------------

class DXFBlock : public DXFEntities {

public:

    DXFBlock * pSucc;
        // Zeiger auf naechsten Block in der Liste DXFBlocks::pFirst

    // Eigenschaften des Blocks, durch Gruppencodes kommentiert:
    char sName[DXF_MAX_STRING_LEN+1];     //  2
    char sAlsoName[DXF_MAX_STRING_LEN+1]; //  3
    long nFlags;                          // 70
    DXFVector aBasePoint;                 // 10,20,30
    char sXRef[DXF_MAX_STRING_LEN+1];     //  1

    DXFBlock();
    ~DXFBlock();

    void Read(DXFGroupReader & rDGR);
        // Liest den Block (einschliesslich der Entities) per rGDR
        // aus einer DXF-Datei bis zu einem ENDBLK, ENDSEC oder EOF.
};


//----------------------------------------------------------------------------
//---------------- Eine Menge von Bloecken -----------------------------------
//----------------------------------------------------------------------------

class DXFBlocks {

public:

    DXFBlock * pFirst;
        // Liste der Bloecke, READ ONLY!

    DXFBlocks();
    ~DXFBlocks();

    void Read(DXFGroupReader & rDGR);
        // Liesst alle Bloecke per rDGR bis zu einem ENDSEC oder EOF.

    DXFBlock * Search(const char * sName) const;
        // Sucht einen Block mit dem Namen, liefert NULL bei Misserfolg.

    void Clear();
        // Loescht alle Bloecke;

};

#endif


