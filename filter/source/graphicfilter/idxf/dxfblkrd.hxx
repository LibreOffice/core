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


