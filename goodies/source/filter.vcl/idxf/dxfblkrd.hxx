/*************************************************************************
 *
 *  $RCSfile: dxfblkrd.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:14 $
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
 *  Copyright 2000 by Sun Microsystems, Inc.
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


