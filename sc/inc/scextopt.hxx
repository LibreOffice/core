/*************************************************************************
 *
 *  $RCSfile: scextopt.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 08:57:38 $
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

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

/******************************************************************************
 *
 * ATTENTION:
 * All col/row/tab members here are dedicated UINT16/INT16 types for alien
 * binary file import/export. May have to be casted/converted when used in Calc
 * context.
 *
 *****************************************************************************/

class ColRowSettings;

// ACHTUNG1: einzelne Einstellungen sollten stimmen
//              -> Absicherung nur ueber Assertions
// ACHTUNG2: Plausibilitaet der Einstellungen untereinander ist nicht
//              gewaehrleistet

struct ScExtTabOptions
{
    // Split -Info
    UINT16                      nTabNum;
    UINT16                      nSplitX;            // horiz. pos. in twips, 0 = no split
    UINT16                      nSplitY;            // vert. pos. ~
    UINT16                      nLeftCol;           // leftmost column visible
    UINT16                      nTopRow;            // topmost row visible
    UINT16                      nLeftSplitCol;      // leftmost column after horizontal split
    UINT16                      nTopSplitRow;       // topmost row after vertical split
    UINT16                      nActPane;           // 0: br, 1: tr, 2: bl, 3: tl

    ScRange                     aLastSel;           // last selection
    BOOL                        bValidSel;
    ScRange                     aDim;               // original Excel size
    BOOL                        bValidDim;

    BOOL                        bSelected;
    BOOL                        bFrozen;            // = TRUE -> nSplitX / nSplitY contain
                                                    // count of visible columns/rows

    explicit                    ScExtTabOptions();

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
    friend class ColRowSettings;
    // Window -Info
    ScExtTabOptions**       ppExtTabOpts;

    String*                 pCodenameWB;
    CodenameList*           pCodenames;

    BOOL                    bChanged;       // for import: copy data only first time to doc
    bool                    bWinProtection;  // Excel Workbook Windows protection flag
    bool                    bWinEncryption;  // Excel Workbook Windows encryption flag

    void                    Reset();

public:
    UINT32                  nLinkCnt;       // Zaehlt die Rekursionstufe beim Laden
                                            //  von externen Dokumenten
    UINT16                  nActTab;        // aktuelle Tabelle
    ScRange*                pOleSize;       // visible range if embedded
    UINT16                  nSelTabs;       // count of selected sheets
    Color*                  pGridCol;       // Farbe Grid und Row-/Col-Heading
    UINT16                  nZoom;          // in %
    // Cursor
    UINT16                  nCurCol;        // aktuelle Cursor-Position
    UINT16                  nCurRow;
    // -------------------------------------------------------------------
                            ScExtDocOptions( void );
                            ScExtDocOptions( const ScExtDocOptions& rCpy );
                            ~ScExtDocOptions();

    ScExtDocOptions&        operator =( const ScExtDocOptions& rCpy );

    void                    SetExtTabOptions( SCTAB nTabNum, ScExtTabOptions* pTabOpt );

    void                    SetGridCol( const Color& rColor );
    void                    SetActTab( UINT16 nTab );
    void                    SetOleSize( SCCOL nFirstCol, SCROW nFirstRow, SCCOL nLastCol, SCROW nLastRow );
    void                    SetCursor( UINT16 nCol, UINT16 nRow );
    void                    SetZoom( UINT16 nZaehler, UINT16 nNenner );
    inline void             SetChanged( BOOL bChg )     { bChanged = bChg; }
    inline BOOL             IsChanged() const           { return bChanged; }

    void                    Add( const ColRowSettings& rCRS );

    inline const ScExtTabOptions*   GetExtTabOptions( const SCTAB nTabNum ) const;
    inline ScExtTabOptions* GetExtTabOptions( const SCTAB nTabNum );
    inline const ScRange*   GetOleSize() const  { return pOleSize; }

    inline const String*    GetCodename( void ) const;      // for Workbook globals
    inline CodenameList*    GetCodenames( void );           // for tables

    void                    SetCodename( const String& );   // -> Workbook globals
    void                    AddCodename( const String& );   // -> tables
    inline void             SetWinProtection(bool bImportWinProtection) {bWinProtection = bImportWinProtection; }
    inline bool             IsWinProtected()         { return bWinProtection; }
    inline void             SetWinEncryption(bool bImportWinEncryption) {bWinEncryption = bImportWinEncryption; }
    inline bool             IsWinEncrypted()         { return bWinEncryption; }
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




inline const ScExtTabOptions* ScExtDocOptions::GetExtTabOptions( const SCTAB nTab ) const
{
    return ValidTab(nTab) ? ppExtTabOpts[ nTab ] : NULL;
}


inline ScExtTabOptions* ScExtDocOptions::GetExtTabOptions( const SCTAB nTab )
{
    return ValidTab(nTab) ? ppExtTabOpts[ nTab ] : NULL;
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

