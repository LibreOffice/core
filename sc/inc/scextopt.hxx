/*************************************************************************
 *
 *  $RCSfile: scextopt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:50 $
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

#ifndef _SCEXTOPT_HXX
#define _SCEXTOPT_HXX


#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif



class ColRowSettings;

// ACHTUNG1: einzelne Einstellungen sollten stimmen
//              -> Absicherung nur ueber Assertions
// ACHTUNG2: Plausibilitaet der Einstellungen untereinander ist nicht
//              gewaehrleistet

struct ScExtTabOptions
{
    // Split -Info
    UINT16                      nTabNum;
    UINT16                      nSplitX;            // horiz. Pos. in Twips, 0 = kein Split
    UINT16                      nSplitY;            // vert. Pos. ~
    UINT16                      nLeftSplitCol;      // linke sichtbare Col im rechten Teil
    UINT16                      nTopSplitRow;       // obere sichtbare Row im unteren Teil
    UINT16                      nActPane;           // 0: ur, 1: or, 2: ul, 3: ol

    ScRange                     aLastSel;           // letzte Selektion
    BOOL                        bValidSel;
    ScRange                     aDim;               // original Excel-Groesse
    BOOL                        bValidDim;

    BOOL                        bFrozen;            // = TRUE -> nSplitX / nSplitY Anzahl
                                                    //  sichtbarer Col/Row links bzw. oben

    inline                      ScExtTabOptions( void );
    inline                      ScExtTabOptions( const ScRange& rLastSel );
    inline                      ScExtTabOptions( const ScExtTabOptions& rCpy );
    inline void                 operator =( const ScExtTabOptions& rCpy );

    void                        SetSelection( const ScRange& rSelection );
    void                        SetDimension( const ScRange& rDim );
};




class CodenameList : protected List
{
    // Code: colrowst.cxx
private:
protected:
public:
    inline                      CodenameList( void );
                                CodenameList( const CodenameList& );
    virtual                     ~CodenameList();

    inline void                 Append( const String& );

    inline const String*        First( void );
    inline const String*        Next( void );
    inline const String*        Act( void ) const;

    List::Count;
};




class ScExtDocOptions
{
// Code: colrowst.cxx
private:
    friend ColRowSettings;
    // Window -Info
    ScExtTabOptions**       ppExtTabOpts;

    String*                 pCodenameWB;
    CodenameList*           pCodenames;
public:
    UINT32                  nLinkCnt;       // Zaehlt die Rekursionstufe beim Laden
                                            //  von externen Dokumenten
    UINT16                  nActTab;        // aktuelle Tabelle
    UINT16                  nVisLeftCol;    // linke Col des sichtbaren Tabellenteils
    UINT16                  nVisTopRow;     // rechte Row des sichtbaren Tabellenteils
    Color*                  pGridCol;       // Farbe Grid und Row-/Col-Heading
    UINT16                  nZoom;          // in %
    // Cursor
    UINT16                  nCurCol;        // aktuelle Cursor-Position
    UINT16                  nCurRow;
    // -------------------------------------------------------------------
                            ScExtDocOptions( void );
                            ~ScExtDocOptions();

    ScExtDocOptions&        operator =( const ScExtDocOptions& rCpy );

    void                    SetGridCol( BYTE nR, BYTE nG, BYTE nB );
    void                    SetActTab( UINT16 nTab );
    void                    SetVisCorner( UINT16 nCol, UINT16 nRow );
    void                    SetCursor( UINT16 nCol, UINT16 nRow );
    void                    SetZoom( UINT16 nZaehler, UINT16 nNenner );

    void                    Add( const ColRowSettings& rCRS );

    inline const ScExtTabOptions*   GetExtTabOptions( const UINT16 nTabNum ) const;

    inline const String*    GetCodename( void ) const;      // for Workbook globals
    inline CodenameList*    GetCodenames( void );           // for tables

    void                    SetCodename( const String& );   // -> Workbook globals
    void                    AddCodename( const String& );   // -> tables
};




inline CodenameList::CodenameList( void )
{
}


inline void CodenameList::Append( const String& r )
{
    List::Insert( new String( r ), LIST_APPEND );
}


inline const String* CodenameList::First( void )
{
    return ( const String* ) List::First();
}


inline const String* CodenameList::Next( void )
{
    return ( const String* ) List::Next();
}


inline const String* CodenameList::Act( void ) const
{
    return ( const String* ) List::GetCurObject();
}




inline ScExtTabOptions::ScExtTabOptions( void )
{
    nSplitX = nSplitY = nLeftSplitCol = nTopSplitRow = nActPane = 0;
    bFrozen = bValidSel = bValidDim = FALSE;
}


inline ScExtTabOptions::ScExtTabOptions( const ScRange& rLastSel ) : aLastSel( rLastSel )
{
    nTabNum = nSplitX = nSplitY = nLeftSplitCol = nTopSplitRow = nActPane = 0;
    bFrozen = bValidDim = FALSE;
    bValidSel = TRUE;
}


inline ScExtTabOptions::ScExtTabOptions( const ScExtTabOptions& rCpy )
{
    nTabNum = rCpy.nTabNum;
    nSplitX = rCpy.nSplitX;
    nSplitY = rCpy.nSplitY;
    nLeftSplitCol = rCpy.nLeftSplitCol;
    nTopSplitRow = rCpy.nTopSplitRow;
    nActPane = rCpy.nActPane;
    aLastSel = rCpy.aLastSel;
    aDim = rCpy.aDim;
    bFrozen = rCpy.bFrozen;
    bValidSel = rCpy.bValidSel;
    bValidDim = rCpy.bValidDim;
}


inline void ScExtTabOptions::operator =( const ScExtTabOptions& rCpy )
{
    nTabNum = rCpy.nTabNum;
    nSplitX = rCpy.nSplitX;
    nSplitY = rCpy.nSplitY;
    nLeftSplitCol = rCpy.nLeftSplitCol;
    nTopSplitRow = rCpy.nTopSplitRow;
    nActPane = rCpy.nActPane;
    aLastSel = rCpy.aLastSel;
    aDim = rCpy.aDim;
    bFrozen = rCpy.bFrozen;
    bValidSel = rCpy.bValidSel;
    bValidDim = rCpy.bValidDim;
}




inline const ScExtTabOptions* ScExtDocOptions::GetExtTabOptions( const UINT16 nTab ) const
{
    if( nTab <= MAXTAB )
        return ppExtTabOpts[ nTab ];
    else
        return NULL;
}


inline const String* ScExtDocOptions::GetCodename( void ) const
{
    return pCodenameWB;
}


inline CodenameList* ScExtDocOptions::GetCodenames( void )
{
    return pCodenames;
}


#endif

