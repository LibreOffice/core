/*************************************************************************
 *
 *  $RCSfile: consoli.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:48 $
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

#ifndef SC_CONSOLI_HXX
#define SC_CONSOLI_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

class ScDocument;

// -----------------------------------------------------------------------

struct ScReferenceEntry             // ohne Constructor !
{
    USHORT  nCol;
    USHORT  nRow;
    USHORT  nTab;
};


//!     Delta-Wert fuer Daten benutzen?

class ScReferenceList           // ohne Constructor !
{
private:
    USHORT              nCount;
    USHORT              nFullSize;          // inkl. Fuell-Eintraege
    ScReferenceEntry*   pData;

public:
    void                    Init()                      { nCount=0; nFullSize=0; pData=NULL; }
    void                    Clear()                     { delete[] pData; }

    USHORT                  GetCount()                  { return nCount; }
    const ScReferenceEntry& GetEntry( USHORT nPos )     { return pData[nPos]; }
    void                    SetFullSize( USHORT nNew )  { nFullSize = nNew; }

    void                    AddEntry( USHORT nCol, USHORT nRow, USHORT nTab );
};

// -----------------------------------------------------------------------


//
//  Reihenfolge:
//      1)  ScConsData anlegen
//      2)  Parameter (Size/Flags)
//      3)  AddFields fuer alle Bereiche (nur noetig bei bColByName oder bRowByName)
//      4)  DoneFields                   (      "                       "          )
//      5)  AddData fuer alle Bereiche
//          evtl. AddName nach jedem Bereich
//      6)  OutputToDocument
//

//! ab bestimmter Groesse ScDocument Struktur benutzen?


class ScConsData
{
private:
    ScSubTotalFunc      eFunction;
    BOOL                bReference;
    BOOL                bColByName;
    BOOL                bRowByName;
    BOOL                bSubTitles;
    USHORT              nColCount;
    USHORT              nRowCount;
    BOOL**              ppUsed;
    double**            ppSum;
    double**            ppCount;
    double**            ppSumSqr;
    ScReferenceList**   ppRefs;
    String**            ppColHeaders;
    String**            ppRowHeaders;
    USHORT              nDataCount;
    USHORT              nTitleCount;
    String**            ppTitles;
    USHORT**            ppTitlePos;
    BOOL                bCornerUsed;
    String              aCornerText;        // nur bei bColByName && bRowByName

public:
                ScConsData();
                ~ScConsData();

    void        SetSize( USHORT nCols, USHORT nRows );
    void        SetFlags( ScSubTotalFunc eFunc, BOOL bColName, BOOL bRowName, BOOL bRef );

    void        InitData(BOOL bDelete=TRUE);
    void        DeleteData();

    void        AddFields( ScDocument* pSrcDoc, USHORT nTab,
                            USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 );
    void        DoneFields();

    void        AddData( ScDocument* pSrcDoc, USHORT nTab,
                            USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 );
    void        AddName( const String& rName );

    void        OutputToDocument( ScDocument* pDestDoc, USHORT nCol, USHORT nRow, USHORT nTab );

    void        GetSize( USHORT& rCols, USHORT& rRows ) const;
    USHORT      GetInsertCount() const;
};


#endif


