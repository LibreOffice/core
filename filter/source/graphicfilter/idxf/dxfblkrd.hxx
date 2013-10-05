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
        // Zeiger auf naechsten Block in der Liste DXFBlocks::pFirst

    // Eigenschaften des Blocks, durch Gruppencodes kommentiert:
    OString m_sName;                      //  2
    OString m_sAlsoName;                  //  3
    long nFlags;                          // 70
    DXFVector aBasePoint;                 // 10,20,30
    OString m_sXRef;                      //  1

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

    DXFBlock * Search(OString const& rName) const;
        // Sucht einen Block mit dem Namen, liefert NULL bei Misserfolg.

    void Clear();
        // Loescht alle Bloecke;

};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
