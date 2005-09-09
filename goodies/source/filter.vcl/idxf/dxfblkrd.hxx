/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dxfblkrd.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:54:34 $
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

#ifndef _DXFBLKRD_HXX
#define _DXFBLKRD_HXX

#ifndef _DXFENTRD_HXX
#include <dxfentrd.hxx>
#endif

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


