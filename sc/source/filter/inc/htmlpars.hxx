/*************************************************************************
 *
 *  $RCSfile: htmlpars.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:13 $
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

#ifndef SC_HTMLPARS_HXX
#define SC_HTMLPARS_HXX

#ifndef _STACK_HXX //autogen
#include <tools/stack.hxx>
#endif

#include "rangelst.hxx"
#include "eeparser.hxx"

#define SC_HTML_FONTSIZES 7     // wie Export, HTML-Options
#define SC_HTML_OFFSET_TOL 10   // Pixeltoleranz fuer SeekOffset

SV_DECL_VARARR_SORT( ScHTMLColOffset, ULONG, 16, 4);

struct ScHTMLTableStackEntry
{
    ScRangeListRef      xLockedList;
    ScEEParseEntry*     pCellEntry;
    ScHTMLColOffset*    pLocalColOffset;
    ULONG               nFirstTableCell;
    USHORT              nColCnt;
    USHORT              nRowCnt;
    USHORT              nColCntStart;
    USHORT              nMaxCol;
    USHORT              nTable;
    USHORT              nTableWidth;
    USHORT              nColOffset;
    USHORT              nColOffsetStart;
    BOOL                bFirstRow;
                        ScHTMLTableStackEntry( ScEEParseEntry* pE,
                                const ScRangeListRef& rL, ScHTMLColOffset* pTO,
                                ULONG nFTC,
                                USHORT nCol, USHORT nRow,
                                USHORT nStart, USHORT nMax, USHORT nTab,
                                USHORT nTW, USHORT nCO, USHORT nCOS,
                                BOOL bFR )
                            : pCellEntry( pE ), xLockedList( rL ),
                            pLocalColOffset( pTO ),
                            nFirstTableCell( nFTC ),
                            nColCnt( nCol ), nRowCnt( nRow ),
                            nColCntStart( nStart ), nMaxCol( nMax ),
                            nTable( nTab ), nTableWidth( nTW ),
                            nColOffset( nCO ), nColOffsetStart( nCOS ),
                            bFirstRow( bFR )
                            {}
                        ~ScHTMLTableStackEntry() {}
};
DECLARE_STACK( ScHTMLTableStack, ScHTMLTableStackEntry* );

struct ScHTMLAdjustStackEntry
{
    USHORT              nLastCol;
    USHORT              nNextRow;
    USHORT              nCurRow;
                        ScHTMLAdjustStackEntry( USHORT nLCol, USHORT nNRow,
                                USHORT nCRow )
                            : nLastCol( nLCol ), nNextRow( nNRow ),
                            nCurRow( nCRow )
                            {}
};
DECLARE_STACK( ScHTMLAdjustStack, ScHTMLAdjustStackEntry* );

class EditEngine;
class ScDocument;
class HTMLOption;

class ScHTMLParser : public ScEEParser
{
private:
    ULONG               aFontHeights[SC_HTML_FONTSIZES];
    ScHTMLTableStack    aTableStack;
    Size                aPageSize;          // in Pixeln
    String              aString;
    ScRangeListRef      xLockedList;        // je Table
    ScDocument*         pDoc;
    Table*              pTables;
    ScHTMLColOffset*    pColOffset;
    ScHTMLColOffset*    pLocalColOffset;    // je Table
    ULONG               nFirstTableCell;    // je Table
    short               nTableLevel;
    USHORT              nTable;
    USHORT              nMaxTable;
    USHORT              nColCntStart;       // erste Col je Table
    USHORT              nMaxCol;            // je Table
    USHORT              nTableWidth;        // je Table
    USHORT              nColOffset;         // aktuell, Pixel
    USHORT              nColOffsetStart;    // Startwert je Table, in Pixel
    USHORT              nMetaCnt;           // fuer ParseMetaOptions
    BOOL                bTabInTabCell;
    BOOL                bFirstRow;          // je Table, ob in erster Zeile
    BOOL                bInCell;
    BOOL                bInTitle;

    DECL_LINK( HTMLImportHdl, ImportInfo* );
    void                NewActEntry( ScEEParseEntry* );
    void                EntryEnd( ScEEParseEntry*, const ESelection& );
    void                ProcToken( ImportInfo* );
    void                CloseEntry( ImportInfo* );
    void                NextRow(  ImportInfo*  );
    void                SkipLocked( ScEEParseEntry*, BOOL bJoin = TRUE );
    static BOOL         SeekOffset( ScHTMLColOffset*, USHORT nOffset,
                                    USHORT* pCol,
                                    USHORT nOffsetTol = SC_HTML_OFFSET_TOL );
    static void         MakeCol( ScHTMLColOffset*, USHORT& nOffset,
                                USHORT& nWidth,
                                USHORT nOffsetTol = SC_HTML_OFFSET_TOL,
                                USHORT nWidthTol = SC_HTML_OFFSET_TOL );
    static void         MakeColNoRef( ScHTMLColOffset*, USHORT nOffset,
                                USHORT nWidth,
                                USHORT nOffsetTol = SC_HTML_OFFSET_TOL,
                                USHORT nWidthTol = SC_HTML_OFFSET_TOL );
    static void         ModifyOffset( ScHTMLColOffset*, USHORT& nOldOffset,
                                    USHORT& nNewOffset,
                                    USHORT nOffsetTol = SC_HTML_OFFSET_TOL );
    void                Colonize( ScEEParseEntry* );
    USHORT              GetWidth( ScEEParseEntry* );
    void                SetWidths();
    void                Adjust();

    USHORT              GetWidthPixel( const HTMLOption* );
    BOOL                IsAtBeginningOfText( ImportInfo* );

    void                TableOn( ImportInfo* );
    void                ColOn( ImportInfo* );
    void                TableDataOn( ImportInfo* );
    void                TableOff( ImportInfo* );
    void                Image( ImportInfo* );
    void                AnchorOn( ImportInfo* );
    void                FontOn( ImportInfo* );

public:
                        ScHTMLParser( EditEngine*, const Size& aPageSize,
                            ScDocument* );
    virtual             ~ScHTMLParser();
    virtual ULONG       Read( SvStream& );
};


#endif

