/*************************************************************************
 *
 *  $RCSfile: impcont.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:01 $
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

#ifndef _IMPCONT_HXX
#define _IMPCONT_HXX

#ifndef _TOOLS_H
#include <tools.h>
#endif

#ifndef _CONTNR_HXX
#include <contnr.hxx>
#endif

typedef void* PVOID;

// ----------
// - CBlock -
// ----------

class CBlock
{
private:
    CBlock*         pPrev;              // Vorheriger Block
    CBlock*         pNext;              // Naechster Block
    USHORT          nSize;              // Groesse des Blocks
    USHORT          nCount;             // Anzahl Pointer
    void**          pNodes;             // Pointer auf die Daten

public:
                    // Fuer List-Container
                    CBlock( USHORT nSize, CBlock* pPrev, CBlock* pNext );
                    // Fuer Array-Container
                    CBlock( USHORT nSize, CBlock* pPrev );
                    // Copy-Ctor
                    CBlock( const CBlock& r, CBlock* pPrev );
                    ~CBlock();

    void            Insert( void* p, USHORT nIndex, USHORT nReSize );
    CBlock*         Split( void* p, USHORT nIndex, USHORT nReSize );
    void*           Remove( USHORT nIndex, USHORT nReSize );
    void*           Replace( void* pNew, USHORT nIndex );

    void**          GetNodes() const { return pNodes; }
    void**          GetObjectPtr( USHORT nIndex );
    void*           GetObject( USHORT nIndex ) const;

    void            SetSize( USHORT nNewSize );

    USHORT          GetSize() const               { return nCount; }
    USHORT          Count() const                 { return nCount; }
    void            SetPrevBlock( CBlock* p )     { pPrev = p;     }
    void            SetNextBlock( CBlock* p )     { pNext = p;     }
    CBlock*         GetPrevBlock() const          { return pPrev;  }
    CBlock*         GetNextBlock() const          { return pNext;  }
    void            Reset()                       { nCount = 0;    }

private:
                    CBlock( const CBlock& r );
};

/*************************************************************************
|*
|*    CBlock::GetObject()
|*
|*    Beschreibung      Gibt einen Pointer aus dem Block zurueck
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

inline void* CBlock::GetObject( USHORT nIndex ) const
{
    return pNodes[nIndex];
}

/*************************************************************************
|*
|*    Container::ImpGetObject()
|*
|*    Beschreibung      Wir gehen davon aus, das Pointer in der Regel
|*                      sich im ersten Block befindet und schalten
|*                      deshalb eine Inline-Methode davor
|*    Ersterstellung    TH 02.07.93
|*    Letzte Aenderung  TH 02.07.93
|*
*************************************************************************/

inline void* Container::ImpGetObject( ULONG nIndex ) const
{
    if ( pFirstBlock && (nIndex < pFirstBlock->Count()) )
        // Item innerhalb des gefundenen Blocks zurueckgeben
        return pFirstBlock->GetObject( (USHORT)nIndex );
    else
        return GetObject( nIndex );
}

/*************************************************************************
|*
|*    Container::ImpGetOnlyNodes()
|*
|*    Beschreibung      Wenn es nur einen Block gibt, wird davon
|*                      das Daten-Array zurueckgegeben
|*    Ersterstellung    TH 24.01.96
|*    Letzte Aenderung  TH 24.01.96
|*
*************************************************************************/

inline void** Container::ImpGetOnlyNodes() const
{
    if ( (pFirstBlock == pLastBlock) && pFirstBlock )
        return pFirstBlock->GetNodes();
    else
        return NULL;
}

#endif // _IMPCONT_HXX
