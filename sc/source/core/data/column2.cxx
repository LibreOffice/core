/*************************************************************************
 *
 *  $RCSfile: column2.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:54:06 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/algitem.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#include <svx/emphitem.hxx>
#include <svx/fhgtitem.hxx>
#include <svx/forbiddencharacterstable.hxx>
#include <svx/rotmodit.hxx>
#include <svx/scripttypeitem.hxx>
#include <svx/unolingu.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/broadcast.hxx>
#include <svtools/listeneriter.hxx>
#include <vcl/outdev.hxx>
#include <math.h>

#include "column.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "attarray.hxx"
#include "patattr.hxx"
#include "cellform.hxx"
#include "collect.hxx"
#include "stlsheet.hxx"
#include "rechead.hxx"
#include "brdcst.hxx"
#include "editutil.hxx"
#include "subtotal.hxx"
#include "markdata.hxx"
#include "compiler.hxx"         // ScTokenArray GetCodeLen
#include "dbcolect.hxx"
#include "fillinfo.hxx"

// -----------------------------------------------------------------------

// factor from font size to optimal cell height (text width)
#define SC_ROT_BREAK_FACTOR     6

// -----------------------------------------------------------------------

inline BOOL CellVisible( const ScBaseCell* pCell )
{
    return ( pCell->GetCellType() != CELLTYPE_NOTE || pCell->GetNotePtr() );
}

inline BOOL IsAmbiguousScript( BYTE nScript )
{
    //! move to a header file
    return ( nScript != SCRIPTTYPE_LATIN &&
             nScript != SCRIPTTYPE_ASIAN &&
             nScript != SCRIPTTYPE_COMPLEX );
}

// -----------------------------------------------------------------------------------------

//
//  Datei-Operationen
//

// -----------------------------------------------------------------------------------------

// special handling for non-convertable characters is no longer needed
#if 0

//  read string from a string cell in original CharSet

String lcl_ReadOriginalStringCell( SvStream& rStream, USHORT nVer, CharSet eSystemCharSet )
{
    if( nVer >= SC_DATABYTES2 )
    {
        BYTE cData;
        rStream >> cData;
        if( cData & 0x0F )
            rStream.SeekRel( cData & 0x0F );
    }

    CharSet eOld = rStream.GetStreamCharSet();
    rStream.SetStreamCharSet( eSystemCharSet );     // no conversion

    String aString;
    rStream >> aString;

    rStream.SetStreamCharSet( eOld );

    return aString;
}

#endif

// -----------------------------------------------------------------------------------------

void ScColumn::LoadData( SvStream& rStream )
{
    SCSIZE      nNewCount;
    SCROW       nNewRow;
    BYTE        nByte;
    USHORT      nVer = (USHORT) pDocument->GetSrcVersion();

    ScMultipleReadHeader aHdr( rStream );

#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
    rStream >> nNewCount;
#endif
    if ( nNewCount > static_cast<SCSIZE>(MAXROW+1) )                        // wuerde das Array zu gross?
    {
        pDocument->SetLostData();
        rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
        return;
    }

    Resize( nNewCount );                            // veraendert nCount nicht
    for (SCSIZE i=0; i<nNewCount; i++)
    {
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
        rStream >> nNewRow;
#endif
        rStream >> nByte;

        if ( nNewRow > MAXROW )                 // Zeilennummer zu gross?
        {
            pDocument->SetLostData();
            rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
            return;
        }

        switch ((CellType) nByte)
        {
            case CELLTYPE_VALUE:
                {
                    ScValueCell* pCell = new ScValueCell( rStream, nVer );
                    Append( nNewRow, pCell );
                }
                break;
            case CELLTYPE_STRING:
                {
                    ScStringCell* pCell = new ScStringCell( rStream, nVer );
                    Append( nNewRow, pCell );
                }
                break;
            case CELLTYPE_SYMBOLS:
                {
                    CharSet eOld = rStream.GetStreamCharSet();
                    //  convert into true symbol characters
                    rStream.SetStreamCharSet( RTL_TEXTENCODING_SYMBOL );
                    ScStringCell* pCell = new ScStringCell( rStream, nVer );
                    Append( nNewRow, pCell );
                    rStream.SetStreamCharSet( eOld );
                    ScSymbolStringCellEntry * pEntry = new ScSymbolStringCellEntry;
                    pEntry->pCell = pCell;
                    pEntry->nRow = nNewRow;
                    pDocument->GetLoadedSymbolStringCellsList().Insert(
                        pEntry, LIST_APPEND );
                }
                break;
            case CELLTYPE_EDIT:
                {
                    ScEditCell* pCell = new ScEditCell( rStream, nVer, pDocument );
                    Append( nNewRow, pCell );
                }
                break;
            case CELLTYPE_FORMULA:
                {
                    ScFormulaCell* pCell = new ScFormulaCell(
                                            pDocument, ScAddress( nCol, nNewRow, nTab ), rStream, aHdr );
                    Append( nNewRow, pCell);
                }
                break;
            case CELLTYPE_NOTE:
                {
                    ScNoteCell *pCell = new ScNoteCell( rStream, nVer );
                    Append( nNewRow, pCell);
                }
                break;
            default:
                DBG_ERROR( "Falscher Zellentyp" );
                rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
                return;
        }
    }
}

BOOL lcl_RemoveAny( ScDocument* pDocument, SCCOL nCol, SCTAB nTab )
{
    ScDBCollection* pDBColl = pDocument->GetDBCollection();
    if ( pDBColl )
    {
        USHORT nCount = pDBColl->GetCount();
        for (USHORT i=0; i<nCount; i++)
        {
            ScDBData* pData = (*pDBColl)[i];
            if ( pData->IsStripData() &&
                    pData->HasImportParam() && !pData->HasImportSelection() )
            {
                ScRange aDBRange;
                pData->GetArea(aDBRange);
                if ( nTab == aDBRange.aStart.Tab() &&
                     nCol >= aDBRange.aStart.Col() && nCol <= aDBRange.aEnd.Col() )
                    return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL lcl_RemoveThis( ScDocument* pDocument, SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    ScDBCollection* pDBColl = pDocument->GetDBCollection();
    if ( pDBColl )
    {
        USHORT nCount = pDBColl->GetCount();
        for (USHORT i=0; i<nCount; i++)
        {
            ScDBData* pData = (*pDBColl)[i];
            if ( pData->IsStripData() &&
                    pData->HasImportParam() && !pData->HasImportSelection() )
            {
                ScRange aDBRange;
                pData->GetArea(aDBRange);
                if ( nTab == aDBRange.aStart.Tab() &&
                     nCol >= aDBRange.aStart.Col() && nCol <= aDBRange.aEnd.Col() &&
                     nRow >= aDBRange.aStart.Row() && nRow <= aDBRange.aEnd.Row() )
                    return TRUE;
            }
        }
    }

    return FALSE;
}

void ScColumn::SaveData( SvStream& rStream ) const
{
    CellType eCellType;
    ScBaseCell* pCell;
    SCSIZE i;
    ScFontToSubsFontConverter_AutoPtr xFontConverter;
    const ULONG nFontConverterFlags = FONTTOSUBSFONT_EXPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS;

    ScMultipleWriteHeader aHdr( rStream );

    SCSIZE nSaveCount = nCount;

    //  Zeilen hinter MAXROW abziehen
    SCROW nSaveMaxRow = pDocument->GetSrcMaxRow();
    if ( nSaveMaxRow != MAXROW )
    {
        if ( nSaveCount && pItems[nSaveCount-1].nRow > nSaveMaxRow )
        {
            pDocument->SetLostData();           // Warnung ausgeben
            do
                --nSaveCount;
            while ( nSaveCount && pItems[nSaveCount-1].nRow > nSaveMaxRow );
        }
    }

    //  Zellen abziehen, die wegen Import nicht gespeichert werden
    BOOL bRemoveAny = lcl_RemoveAny( pDocument, nCol, nTab );
    SCSIZE nEffCount = nSaveCount;
    if ( bRemoveAny )
    {
        for (i=0; i<nSaveCount; i++)
            if ( lcl_RemoveThis( pDocument, nCol, pItems[i].nRow, nTab ) )
                --nEffCount;

//      String aDbg("Tab ");aDbg+=nTab;aDbg+=" Col ";aDbg+=nCol;
//      aDbg+=" Remove ";aDbg+=nSaveCount-nEffCount; DBG_ERROR(aDbg);
    }

#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
    rStream << nEffCount;           // nEffCount: Zellen, die wirklich gespeichert werden
#endif

    ScAttrIterator aIter( pAttrArray, 0, MAXROW );
    SCROW nStt, nEnd;
    const ScPatternAttr* pAttr;
    do
    {
        pAttr = aIter.Next( nStt, nEnd );
    }
    while( pAttr && !(
        (xFontConverter = pAttr->GetSubsFontConverter( nFontConverterFlags ))
        || pAttr->IsSymbolFont()) );

    for (i=0; i<nSaveCount; i++)        // nSaveCount: Ende auf MAXROW angepasst
    {
        SCROW nRow = pItems[i].nRow;

        if ( !bRemoveAny || !lcl_RemoveThis( pDocument, nCol, nRow, nTab ) )
        {
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
            rStream << nRow;
#endif

            pCell = pItems[i].pCell;
            eCellType = pCell->GetCellType();

            switch( eCellType )
            {
                case CELLTYPE_VALUE:
                    rStream << (BYTE) eCellType;
                    ((ScValueCell*)pCell)->Save( rStream );
                    break;
                case CELLTYPE_STRING:
                    if( pAttr )
                    {
                        if( nRow > nEnd )
                        {
                            do
                            {
                                do
                                {
                                    pAttr = aIter.Next( nStt, nEnd );
                                }
                                while ( pAttr && nRow > nEnd );     // #99139# skip all formats before this cell
                            }
                            while( pAttr && !(
                                (xFontConverter = pAttr->GetSubsFontConverter( nFontConverterFlags ))
                                || pAttr->IsSymbolFont()) );
                        }
                        if( pAttr && nRow >= nStt && nRow <= nEnd )
                            eCellType = CELLTYPE_SYMBOLS;
                    }
                    rStream << (BYTE) eCellType;
                    if ( eCellType == CELLTYPE_SYMBOLS )
                    {
                        //  cell string contains true symbol characters
                        CharSet eOld = rStream.GetStreamCharSet();
                        rStream.SetStreamCharSet( RTL_TEXTENCODING_SYMBOL );
                        ((ScStringCell*)pCell)->Save( rStream, xFontConverter );
                        rStream.SetStreamCharSet( eOld );
                    }
                    else
                        ((ScStringCell*)pCell)->Save( rStream );
                    break;
                case CELLTYPE_EDIT:
                    rStream << (BYTE) eCellType;
                    ((ScEditCell*)pCell)->Save( rStream );
                    break;
                case CELLTYPE_FORMULA:
                    rStream << (BYTE) eCellType;
                    ((ScFormulaCell*)pCell)->Save( rStream, aHdr );
                    break;
                case CELLTYPE_NOTE:
                    rStream << (BYTE) eCellType;
                    ((ScNoteCell*)pCell)->Save( rStream );
                    break;
                default:
                    {
                        //  #53846# soll zwar nicht vorkommen, aber falls doch,
                        //  eine leere NoteCell speichern, damit das Dokument
                        //  ueberhaupt wieder geladen werden kann.
                        rStream << (BYTE) CELLTYPE_NOTE;
                        ScNoteCell aDummyCell;
                        aDummyCell.Save( rStream );
                        DBG_ERROR( "Falscher Zellentyp" );
                    }
                    break;
            }
        }
    }
}

// -----------------------------------------------------------------------------------------

void ScColumn::LoadNotes( SvStream& rStream )
{
    ScReadHeader aHdr(rStream);

    SCSIZE nNoteCount;
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
    rStream >> nNoteCount;
#endif
    for (SCSIZE i=0; i<nNoteCount && rStream.GetError() == SVSTREAM_OK; i++)
    {
        SCSIZE nPos;
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
        rStream >> nPos;
#endif
        if (nPos < nCount)
            pItems[nPos].pCell->LoadNote(rStream);
        else
        {
            DBG_ERROR("falsche Pos in ScColumn::LoadNotes");
            rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
        }
    }
}

SCROW ScColumn::NoteCount( SCROW nMaxRow ) const
{
    SCROW nNoteCount = 0;
    SCSIZE i;

    for (i=0; i<nCount; i++)
        if ( pItems[i].pCell->GetNotePtr() && pItems[i].nRow<=nMaxRow )
            ++nNoteCount;

    return nNoteCount;
}

void ScColumn::SaveNotes( SvStream& rStream ) const
{
    SCROW nSaveMaxRow = pDocument->GetSrcMaxRow();
    SCSIZE i;

    BOOL bRemoveAny = lcl_RemoveAny( pDocument, nCol, nTab );
    SCROW nNoteCount;
    if ( bRemoveAny )
    {
        //  vorher zaehlen, wieviele Notizen es werden

        nNoteCount = 0;
        for (i=0; i<nCount; i++)
            if ( pItems[i].pCell->GetNotePtr() && pItems[i].nRow<=nSaveMaxRow &&
                    !lcl_RemoveThis( pDocument, nCol, pItems[i].nRow, nTab ) )
                ++nNoteCount;
    }
    else
        nNoteCount = NoteCount(nSaveMaxRow);

    //  Speichern
    //  Als Positionen muessen die Indizes gespeichert werden, die beim Laden entstehen,
    //  also ohne die weggelassenen Zellen mitzuzaehlen.

    ScWriteHeader aHdr(rStream);
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
    rStream << nNoteCount;
#endif

    USHORT nDestPos = 0;
    for (i=0; i<nCount && rStream.GetError() == SVSTREAM_OK; i++)
    {
        SCROW nRow = pItems[i].nRow;
        if ( !bRemoveAny || !lcl_RemoveThis( pDocument, nCol, nRow, nTab ) )
        {
            const ScPostIt* pNote = pItems[i].pCell->GetNotePtr();
            if ( pNote && nRow <= nSaveMaxRow )
            {
                rStream << nDestPos;
                rStream << *pNote;
            }
            ++nDestPos;         // nDestPos zaehlt die in SaveData gespeicherten Zellen
        }
    }

    //  SetLostData ist schon in SaveData passiert, wenn noetig
}

// -----------------------------------------------------------------------------------------

void ScColumn::CorrectSymbolCells( CharSet eStreamCharSet )
{
    //  #99139# find and correct string cells that are formatted with a symbol font,
    //  but are not in the LoadedSymbolStringCellsList
    //  (because CELLTYPE_SYMBOLS wasn't written in the file)

    ScFontToSubsFontConverter_AutoPtr xFontConverter;
    const ULONG nFontConverterFlags = FONTTOSUBSFONT_EXPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS;

    BOOL bListInitialized = FALSE;
    ScSymbolStringCellEntry* pCurrentEntry = NULL;

    ScAttrIterator aAttrIter( pAttrArray, 0, MAXROW );
    SCROW nStt, nEnd;
    const ScPatternAttr* pAttr = aAttrIter.Next( nStt, nEnd );
    while ( pAttr )
    {
        if ( (xFontConverter = pAttr->GetSubsFontConverter( nFontConverterFlags )) ||
                pAttr->IsSymbolFont() )
        {
            ScColumnIterator aCellIter( this, nStt, nEnd );
            SCROW nRow;
            ScBaseCell* pCell;
            while ( aCellIter.Next( nRow, pCell ) )
            {
                if ( pCell->GetCellType() == CELLTYPE_STRING )
                {
                    List& rList = pDocument->GetLoadedSymbolStringCellsList();
                    if (!bListInitialized)
                    {
                        pCurrentEntry = (ScSymbolStringCellEntry*)rList.First();
                        bListInitialized = TRUE;
                    }

                    while ( pCurrentEntry && pCurrentEntry->nRow < nRow )
                        pCurrentEntry = (ScSymbolStringCellEntry*)rList.Next();

                    if ( pCurrentEntry && pCurrentEntry->nRow == nRow )
                    {
                        //  found
                    }
                    else
                    {
                        //  not in list -> convert and put into list

                        ScStringCell* pStrCell = (ScStringCell*)pCell;
                        String aOldStr;
                        pStrCell->GetString( aOldStr );

                        //  convert back to stream character set (get original data)
                        ByteString aByteStr( aOldStr, eStreamCharSet );

                        //  convert using symbol encoding, as for CELLTYPE_SYMBOLS cells
                        String aNewStr( aByteStr, RTL_TEXTENCODING_SYMBOL );
                        pStrCell->SetString( aNewStr );

                        ScSymbolStringCellEntry * pEntry = new ScSymbolStringCellEntry;
                        pEntry->pCell = pStrCell;
                        pEntry->nRow = nRow;

                        if ( pCurrentEntry )
                            rList.Insert( pEntry );     // before current entry - pCurrentEntry stays valid
                        else
                            rList.Insert( pEntry, LIST_APPEND );    // append if already behind last entry
                    }
                }
            }
        }

        pAttr = aAttrIter.Next( nStt, nEnd );
    }
}

BOOL ScColumn::Load( SvStream& rStream, ScMultipleReadHeader& rHdr )
{
    rHdr.StartEntry();
    while (rHdr.BytesLeft() && rStream.GetError() == SVSTREAM_OK)
    {
        USHORT nID;
        rStream >> nID;
        switch (nID)
        {
            case SCID_COLDATA:
                LoadData( rStream );
                break;
            case SCID_COLNOTES:
                LoadNotes( rStream );
                break;
            case SCID_COLATTRIB:
                pAttrArray->Load( rStream );
                break;
            default:
                {
                    DBG_ERROR("unbekannter Sub-Record in ScColumn::Load");
                    ScReadHeader aDummyHeader( rStream );
                }
        }
    }
    rHdr.EndEntry();

    //  #99139# old versions didn't always write CELLTYPE_SYMBOLS for symbol string cells,
    //  so we have to look for remaining string cells in areas that are formatted with
    //  symbol font:
    CorrectSymbolCells( rStream.GetStreamCharSet() );

    if ( pDocument->SymbolStringCellsPending() )
    {
        ScFontToSubsFontConverter_AutoPtr xFontConverter;
        const ULONG nFontConverterFlags = FONTTOSUBSFONT_IMPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS;
        ScSymbolStringCellEntry* pE;
        SCROW nStt, nEnd;

        ScAttrIterator aIter( pAttrArray, 0, MAXROW );
        const ScPatternAttr* pAttr = aIter.Next( nStt, nEnd );
        xFontConverter = pAttr->GetSubsFontConverter( nFontConverterFlags );

        List& rList = pDocument->GetLoadedSymbolStringCellsList();
        for ( pE = (ScSymbolStringCellEntry*) rList.First(); pE;
                pE = (ScSymbolStringCellEntry*) rList.Next() )
        {
            const ScPatternAttr* pLastAttr = pAttr;
            while ( nEnd < pE->nRow )
            {
                pAttr = aIter.Next( nStt, nEnd );
            }
            if ( pAttr != pLastAttr )
                xFontConverter = pAttr->GetSubsFontConverter( nFontConverterFlags );
            pE->pCell->ConvertFont( xFontConverter );
            delete pE;
        }
        rList.Clear();
    }
    pAttrArray->ConvertFontsAfterLoad();

    return TRUE;
}

BOOL ScColumn::Save( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const
{
    rHdr.StartEntry();

    if (!IsEmptyData())             //! Test, ob alles weggelassen wird?
    {
        rStream << (USHORT) SCID_COLDATA;
        SaveData( rStream );
    }
    SCROW nNotes = NoteCount(); //! Test, ob alles weggelassen wird?
    if (nNotes > 0)
    {
        rStream << (USHORT) SCID_COLNOTES;
        SaveNotes( rStream );
    }
    if (!IsEmptyAttr())
    {
        rStream << (USHORT) SCID_COLATTRIB;
        pAttrArray->Save( rStream );
    }

    rHdr.EndEntry();

    return TRUE;
}

// -----------------------------------------------------------------------------------------

                                    //  GetNeededSize: optimale Hoehe / Breite in Pixeln

long ScColumn::GetNeededSize( SCROW nRow, OutputDevice* pDev,
                              double nPPTX, double nPPTY,
                              const Fraction& rZoomX, const Fraction& rZoomY,
                              BOOL bWidth, const ScNeededSizeOptions& rOptions )
{
    long nValue=0;
    SCSIZE nIndex;
    double nPPT = bWidth ? nPPTX : nPPTY;
    if (Search(nRow,nIndex))
    {
        const ScPatternAttr* pPattern = rOptions.pPattern;
        if (!pPattern)
            pPattern = pAttrArray->GetPattern( nRow );

        //      zusammengefasst?
        //      Merge nicht in bedingter Formatierung

        const ScMergeAttr*      pMerge = (const ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
        const ScMergeFlagAttr*  pFlag = (const ScMergeFlagAttr*)&pPattern->GetItem(ATTR_MERGE_FLAG);

        if ( bWidth )
        {
            if ( pFlag->IsHorOverlapped() )
                return 0;
            if ( rOptions.bSkipMerged && pMerge->GetColMerge() > 1 )
                return 0;
        }
        else
        {
            if ( pFlag->IsVerOverlapped() )
                return 0;
            if ( rOptions.bSkipMerged && pMerge->GetRowMerge() > 1 )
                return 0;
        }

        //      bedingte Formatierung
        const SfxItemSet* pCondSet = NULL;
        if ( ((const SfxUInt32Item&)pPattern->GetItem(ATTR_CONDITIONAL)).GetValue() )
            pCondSet = pDocument->GetCondResult( nCol, nRow, nTab );

        //  Zeilenumbruch?

        const SfxPoolItem* pCondItem;
        SvxCellHorJustify eHorJust;
        if (pCondSet &&
                pCondSet->GetItemState(ATTR_HOR_JUSTIFY, TRUE, &pCondItem) == SFX_ITEM_SET)
            eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem*)pCondItem)->GetValue();
        else
            eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)
                                            pPattern->GetItem( ATTR_HOR_JUSTIFY )).GetValue();
        BOOL bBreak;
        if ( eHorJust == SVX_HOR_JUSTIFY_BLOCK )
            bBreak = TRUE;
        else if ( pCondSet &&
                    pCondSet->GetItemState(ATTR_LINEBREAK, TRUE, &pCondItem) == SFX_ITEM_SET)
            bBreak = ((const SfxBoolItem*)pCondItem)->GetValue();
        else
            bBreak = ((const SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK)).GetValue();

        //  get other attributes from pattern and conditional formatting

        SvxCellOrientation eOrient = pPattern->GetCellOrientation( pCondSet );
        BOOL bAsianVertical = ( eOrient == SVX_ORIENTATION_STACKED &&
                ((const SfxBoolItem&)pPattern->GetItem( ATTR_VERTICAL_ASIAN, pCondSet )).GetValue() );
        if ( bAsianVertical )
            bBreak = FALSE;

        if ( bWidth && bBreak )     // after determining bAsianVertical (bBreak may be reset)
            return 0;

        long nRotate = 0;
        SvxRotateMode eRotMode = SVX_ROTATE_MODE_STANDARD;
        if ( eOrient == SVX_ORIENTATION_STANDARD )
        {
            if (pCondSet &&
                    pCondSet->GetItemState(ATTR_ROTATE_VALUE, TRUE, &pCondItem) == SFX_ITEM_SET)
                nRotate = ((const SfxInt32Item*)pCondItem)->GetValue();
            else
                nRotate = ((const SfxInt32Item&)pPattern->GetItem(ATTR_ROTATE_VALUE)).GetValue();
            if ( nRotate )
            {
                if (pCondSet &&
                        pCondSet->GetItemState(ATTR_ROTATE_MODE, TRUE, &pCondItem) == SFX_ITEM_SET)
                    eRotMode = (SvxRotateMode)((const SvxRotateModeItem*)pCondItem)->GetValue();
                else
                    eRotMode = (SvxRotateMode)((const SvxRotateModeItem&)
                                                pPattern->GetItem(ATTR_ROTATE_MODE)).GetValue();

                if ( nRotate == 18000 )
                    eRotMode = SVX_ROTATE_MODE_STANDARD;    // keinen Ueberlauf
            }
        }

        if ( eHorJust == SVX_HOR_JUSTIFY_REPEAT )
        {
            // ignore orientation/rotation if "repeat" is active
            eOrient = SVX_ORIENTATION_STANDARD;
            nRotate = 0;
            bAsianVertical = FALSE;
        }

        const SvxMarginItem* pMargin;
        if (pCondSet &&
                pCondSet->GetItemState(ATTR_MARGIN, TRUE, &pCondItem) == SFX_ITEM_SET)
            pMargin = (const SvxMarginItem*) pCondItem;
        else
            pMargin = (const SvxMarginItem*) &pPattern->GetItem(ATTR_MARGIN);
        USHORT nIndent = 0;
        if ( eHorJust == SVX_HOR_JUSTIFY_LEFT )
        {
            if (pCondSet &&
                    pCondSet->GetItemState(ATTR_INDENT, TRUE, &pCondItem) == SFX_ITEM_SET)
                nIndent = ((const SfxUInt16Item*)pCondItem)->GetValue();
            else
                nIndent = ((const SfxUInt16Item&)pPattern->GetItem(ATTR_INDENT)).GetValue();
        }

        ScBaseCell* pCell = pItems[nIndex].pCell;
        BYTE nScript = pDocument->GetScriptType( nCol, nRow, nTab, pCell );
        if (nScript == 0) nScript = ScGlobal::GetDefaultScriptType();

        //  also call SetFont for edit cells, because bGetFont may be set only once
        //  bGetFont is set also if script type changes
        if (rOptions.bGetFont)
        {
            Fraction aFontZoom = ( eOrient == SVX_ORIENTATION_STANDARD ) ? rZoomX : rZoomY;
            Font aFont;
            // font color doesn't matter here
            pPattern->GetFont( aFont, SC_AUTOCOL_BLACK, pDev, &aFontZoom, pCondSet, nScript );
            pDev->SetFont(aFont);
        }

        BOOL bAddMargin = TRUE;
        BOOL bEditEngine = ( pCell->GetCellType() == CELLTYPE_EDIT ||
                                eOrient == SVX_ORIENTATION_STACKED ||
                                IsAmbiguousScript( nScript ) );

        if (!bEditEngine)                                   // direkte Ausgabe
        {
            String aValStr;
            Color* pColor;
            SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
            ULONG nFormat = pPattern->GetNumberFormat( pFormatter, pCondSet );
            ScCellFormat::GetString( pCell, nFormat, aValStr, &pColor,
                                        *pFormatter,
                                        TRUE, rOptions.bFormula, ftCheck );
            if (aValStr.Len())
            {
                //  SetFont ist nach oben verschoben

                Size aSize( pDev->GetTextWidth( aValStr ), pDev->GetTextHeight() );
                if ( eOrient != SVX_ORIENTATION_STANDARD )
                {
                    long nTemp = aSize.Width();
                    aSize.Width() = aSize.Height();
                    aSize.Height() = nTemp;
                }
                else if ( nRotate )
                {
                    //! unterschiedliche Skalierung X/Y beruecksichtigen

                    double nRealOrient = nRotate * F_PI18000;   // nRotate sind 1/100 Grad
                    double nCosAbs = fabs( cos( nRealOrient ) );
                    double nSinAbs = fabs( sin( nRealOrient ) );
                    long nHeight = (long)( aSize.Height() * nCosAbs + aSize.Width() * nSinAbs );
                    long nWidth;
                    if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
                        nWidth  = (long)( aSize.Width() * nCosAbs + aSize.Height() * nSinAbs );
                    else if ( rOptions.bTotalSize )
                    {
                        nWidth = (long) ( pDocument->GetColWidth( nCol,nTab ) * nPPT );
                        bAddMargin = FALSE;
                        //  nur nach rechts:
                        //! unterscheiden nach Ausrichtung oben/unten (nur Text/ganze Hoehe)
                        if ( pPattern->GetRotateDir( pCondSet ) == SC_ROTDIR_RIGHT )
                            nWidth += (long)( pDocument->GetRowHeight( nRow,nTab ) *
                                                nPPT * nCosAbs / nSinAbs );
                    }
                    else
                        nWidth  = (long)( aSize.Height() / nSinAbs );   //! begrenzen?

                    if ( bBreak && !rOptions.bTotalSize )
                    {
                        //  #47744# limit size for line break
                        long nCmp = pDev->GetFont().GetSize().Height() * SC_ROT_BREAK_FACTOR;
                        if ( nHeight > nCmp )
                            nHeight = nCmp;
                    }

                    aSize = Size( nWidth, nHeight );
                }
                nValue = bWidth ? aSize.Width() : aSize.Height();

                if ( bAddMargin )
                {
                    if (bWidth)
                    {
                        nValue += (long) ( pMargin->GetLeftMargin() * nPPT ) +
                                  (long) ( pMargin->GetRightMargin() * nPPT );
                        if ( nIndent )
                            nValue += (long) ( nIndent * nPPT );
                    }
                    else
                        nValue += (long) ( pMargin->GetTopMargin() * nPPT ) +
                                  (long) ( pMargin->GetBottomMargin() * nPPT );
                }

                                                //  Zeilenumbruch ausgefuehrt ?

                if ( bBreak && !bWidth )
                {
                    //  Test mit EditEngine zur Sicherheit schon bei 90%
                    //  (wegen Rundungsfehlern und weil EditEngine teilweise anders formatiert)

                    long nDocPixel = (long) ( ( pDocument->GetColWidth( nCol,nTab ) -
                                        pMargin->GetLeftMargin() - pMargin->GetRightMargin() -
                                        nIndent )
                                        * nPPT );
                    nDocPixel = (nDocPixel * 9) / 10;           // zur Sicherheit
                    if ( aSize.Width() > nDocPixel )
                        bEditEngine = TRUE;
                }
            }
        }

        if (bEditEngine)
        {
            //  der Font wird bei !bEditEngine nicht jedesmal neu gesetzt
            Font aOldFont = pDev->GetFont();

            MapMode aHMMMode( MAP_100TH_MM, Point(), rZoomX, rZoomY );

            // am Dokument speichern ?
            ScFieldEditEngine* pEngine = pDocument->CreateFieldEditEngine();

            pEngine->SetUpdateMode( FALSE );
            MapMode aOld = pDev->GetMapMode();
            pDev->SetMapMode( aHMMMode );
            pEngine->SetRefDevice( pDev );
            pEngine->SetForbiddenCharsTable( pDocument->GetForbiddenCharacters() );
            pEngine->SetAsianCompressionMode( pDocument->GetAsianCompression() );
            pEngine->SetKernAsianPunctuation( pDocument->GetAsianKerning() );
            SfxItemSet* pSet = new SfxItemSet( pEngine->GetEmptyItemSet() );
            pPattern->FillEditItemSet( pSet, pCondSet );

//          no longer needed, are setted with the text (is faster)
//          pEngine->SetDefaults( pSet );

            if ( ((const SfxBoolItem&)pSet->Get(EE_PARA_HYPHENATE)).GetValue() ) {

                com::sun::star::uno::Reference<com::sun::star::linguistic2::XHyphenator> xXHyphenator( LinguMgr::GetHyphenator() );
                pEngine->SetHyphenator( xXHyphenator );
            }

            Size aPaper = Size( 1000000, 1000000 );
            if ( eOrient==SVX_ORIENTATION_STACKED && !bAsianVertical )
                aPaper.Width() = 1;
            else if (bBreak)
            {
                double fWidthFactor = nPPTX;
                BOOL bTextWysiwyg = ( pDev->GetOutDevType() == OUTDEV_PRINTER );
                if ( bTextWysiwyg )
                {
                    //  #95593# if text is formatted for printer, don't use PixelToLogic,
                    //  to ensure the exact same paper width (and same line breaks) as in
                    //  ScEditUtil::GetEditArea, used for output.

                    fWidthFactor = HMM_PER_TWIPS;
                }

                // use original width for hidden columns:
                long nDocWidth = (long) ( pDocument->GetOriginalWidth(nCol,nTab) * fWidthFactor );
                SCCOL nColMerge = pMerge->GetColMerge();
                if (nColMerge > 1)
                    for (SCCOL nColAdd=1; nColAdd<nColMerge; nColAdd++)
                        nDocWidth += (long) ( pDocument->GetColWidth(nCol+nColAdd,nTab) * fWidthFactor );
                nDocWidth -= (long) ( pMargin->GetLeftMargin() * fWidthFactor )
                           + (long) ( pMargin->GetRightMargin() * fWidthFactor )
                           + 1;     // Ausgabebereich ist Breite-1 Pixel (wegen Gitterlinien)
                if ( nIndent )
                    nDocWidth -= (long) ( nIndent * fWidthFactor );

                // space for AutoFilter button:  20 * nZoom/100
                if ( pFlag->HasAutoFilter() && !bTextWysiwyg )
                    nDocWidth -= (rZoomX.GetNumerator()*20)/rZoomX.GetDenominator();

                aPaper.Width() = nDocWidth;

                if ( !bTextWysiwyg )
                    aPaper = pDev->PixelToLogic( aPaper, aHMMMode );
            }
            pEngine->SetPaperSize(aPaper);

            if ( pCell->GetCellType() == CELLTYPE_EDIT )
            {
                const EditTextObject* pData;
                ((ScEditCell*)pCell)->GetData(pData);
                pEngine->SetTextNewDefaults(*pData, pSet);
            }
            else
            {
                Color* pColor;
                SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
                ULONG nFormat = pPattern->GetNumberFormat( pFormatter, pCondSet );
                String aString;
                ScCellFormat::GetString( pCell, nFormat, aString, &pColor,
                                            *pFormatter,
                                            TRUE, rOptions.bFormula, ftCheck );
                if (aString.Len())
                    pEngine->SetTextNewDefaults(aString, pSet);
                else
                    pEngine->SetDefaults(pSet);
            }

            BOOL bEngineVertical = pEngine->IsVertical();
            pEngine->SetVertical( bAsianVertical );
            pEngine->SetUpdateMode( TRUE );

            BOOL bEdWidth = bWidth;
            if ( eOrient != SVX_ORIENTATION_STANDARD && eOrient != SVX_ORIENTATION_STACKED )
                bEdWidth = !bEdWidth;
            if ( nRotate )
            {
                //! unterschiedliche Skalierung X/Y beruecksichtigen

                Size aSize( pEngine->CalcTextWidth(), pEngine->GetTextHeight() );
                double nRealOrient = nRotate * F_PI18000;   // nRotate sind 1/100 Grad
                double nCosAbs = fabs( cos( nRealOrient ) );
                double nSinAbs = fabs( sin( nRealOrient ) );
                long nHeight = (long)( aSize.Height() * nCosAbs + aSize.Width() * nSinAbs );
                long nWidth;
                if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
                    nWidth  = (long)( aSize.Width() * nCosAbs + aSize.Height() * nSinAbs );
                else if ( rOptions.bTotalSize )
                {
                    nWidth = (long) ( pDocument->GetColWidth( nCol,nTab ) * nPPT );
                    bAddMargin = FALSE;
                    if ( pPattern->GetRotateDir( pCondSet ) == SC_ROTDIR_RIGHT )
                        nWidth += (long)( pDocument->GetRowHeight( nRow,nTab ) *
                                            nPPT * nCosAbs / nSinAbs );
                }
                else
                    nWidth  = (long)( aSize.Height() / nSinAbs );   //! begrenzen?
                aSize = Size( nWidth, nHeight );

                Size aPixSize = pDev->LogicToPixel( aSize, aHMMMode );
                if ( bEdWidth )
                    nValue = aPixSize.Width();
                else
                {
                    nValue = aPixSize.Height();

                    if ( bBreak && !rOptions.bTotalSize )
                    {
                        //  #47744# limit size for line break
                        long nCmp = aOldFont.GetSize().Height() * SC_ROT_BREAK_FACTOR;
                        if ( nValue > nCmp )
                            nValue = nCmp;
                    }
                }
            }
            else if ( bEdWidth )
            {
                if (bBreak)
                    nValue = 0;
                else
                    nValue = pDev->LogicToPixel(Size( pEngine->CalcTextWidth(), 0 ),
                                        aHMMMode).Width();
            }
            else            // Hoehe
            {
                nValue = pDev->LogicToPixel(Size( 0, pEngine->GetTextHeight() ),
                                    aHMMMode).Height();
            }

            if ( nValue && bAddMargin )
            {
                if (bWidth)
                {
                    nValue += (long) ( pMargin->GetLeftMargin() * nPPT ) +
                              (long) ( pMargin->GetRightMargin() * nPPT );
                    if (nIndent)
                        nValue += (long) ( nIndent * nPPT );
                }
                else
                {
                    nValue += (long) ( pMargin->GetTopMargin() * nPPT ) +
                              (long) ( pMargin->GetBottomMargin() * nPPT );

                    if ( bAsianVertical && pDev->GetOutDevType() != OUTDEV_PRINTER )
                    {
                        //  add 1pt extra (default margin value) for line breaks with SetVertical
                        nValue += (long) ( 20 * nPPT );
                    }
                }
            }

            //  EditEngine is cached and re-used, so the old vertical flag must be restored
            pEngine->SetVertical( bEngineVertical );

            pDocument->DisposeFieldEditEngine(pEngine);

            pDev->SetMapMode( aOld );
            pDev->SetFont( aOldFont );
        }

        if (bWidth)
        {
            //      Platz fuer Autofilter-Button
            //      20 * nZoom/100
            //      bedingte Formatierung hier nicht interessant

            INT16 nFlags = ((const ScMergeFlagAttr&)pPattern->GetItem(ATTR_MERGE_FLAG)).GetValue();
            if (nFlags & SC_MF_AUTO)
                nValue += (rZoomX.GetNumerator()*20)/rZoomX.GetDenominator();
        }
    }
    return nValue;
}

long ScColumn::GetSimpleTextNeededSize( SCSIZE nIndex, OutputDevice* pDev,
        BOOL bWidth )
{
    long nValue=0;
    if ( nIndex < nCount )
    {
        SCROW nRow = pItems[nIndex].nRow;
        const ScPatternAttr* pPattern = pAttrArray->GetPattern( nRow );
        ScBaseCell* pCell = pItems[nIndex].pCell;
        String aValStr;
        Color* pColor;
        SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
        ULONG nFormat = pPattern->GetNumberFormat( pFormatter );
        ScCellFormat::GetString( pCell, nFormat, aValStr, &pColor,
                                    *pFormatter, TRUE, FALSE, ftCheck );
        if ( aValStr.Len() )
        {
            if ( bWidth )
                nValue = pDev->GetTextWidth( aValStr );
            else
                nValue = pDev->GetTextHeight();
        }
    }
    return nValue;
}

USHORT ScColumn::GetOptimalColWidth( OutputDevice* pDev, double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        BOOL bFormula, USHORT nOldWidth,
                                        const ScMarkData* pMarkData,
                                        BOOL bSimpleTextImport )
{
    if (nCount == 0)
        return nOldWidth;

    USHORT  nWidth = (USHORT) (nOldWidth * nPPTX);
    BOOL    bFound = FALSE;

    SCSIZE nIndex;
    ScMarkedDataIter aDataIter(this, pMarkData, TRUE);
    if ( bSimpleTextImport )
    {   // alles eins bis auf NumberFormate
        const ScPatternAttr* pPattern = GetPattern( 0 );
        Font aFont;
        // font color doesn't matter here
        pPattern->GetFont( aFont, SC_AUTOCOL_BLACK, pDev, &rZoomX, NULL );
        pDev->SetFont( aFont );
        const SvxMarginItem* pMargin = (const SvxMarginItem*) &pPattern->GetItem(ATTR_MARGIN);
        long nMargin = (long) ( pMargin->GetLeftMargin() * nPPTX ) +
                        (long) ( pMargin->GetRightMargin() * nPPTX );

        while (aDataIter.Next( nIndex ))
        {
            USHORT nThis = (USHORT) (GetSimpleTextNeededSize( nIndex, pDev,
                TRUE ) + nMargin);
            if (nThis)
            {
                if (nThis>nWidth || !bFound)
                {
                    nWidth = nThis;
                    bFound = TRUE;
                }
            }
        }
    }
    else
    {
        ScNeededSizeOptions aOptions;
        aOptions.bFormula = bFormula;
        const ScPatternAttr* pOldPattern = NULL;
        BYTE nOldScript = 0;

        while (aDataIter.Next( nIndex ))
        {
            SCROW nRow = pItems[nIndex].nRow;

            BYTE nScript = pDocument->GetScriptType( nCol, nRow, nTab, pItems[nIndex].pCell );
            if (nScript == 0) nScript = ScGlobal::GetDefaultScriptType();

            const ScPatternAttr* pPattern = GetPattern( nRow );
            aOptions.pPattern = pPattern;
            aOptions.bGetFont = (pPattern != pOldPattern || nScript != nOldScript);
            USHORT nThis = (USHORT) GetNeededSize( nRow, pDev, nPPTX, nPPTY,
                rZoomX, rZoomY, TRUE, aOptions );
            pOldPattern = pPattern;
            if (nThis)
            {
                if (nThis>nWidth || !bFound)
                {
                    nWidth = nThis;
                    bFound = TRUE;
                }
            }
        }
    }

    if (bFound)
    {
        nWidth += 2;
        USHORT nTwips = (USHORT) (nWidth / nPPTX);
        return nTwips;
    }
    else
        return nOldWidth;
}

USHORT lcl_GetAttribHeight( const ScPatternAttr& rPattern, USHORT nFontHeightId )
{
    USHORT nHeight = (USHORT) ((const SvxFontHeightItem&) rPattern.GetItem(nFontHeightId)).GetHeight();
    const SvxMarginItem* pMargin = (const SvxMarginItem*) &rPattern.GetItem(ATTR_MARGIN);
    nHeight += nHeight / 5;
    //  gibt bei 10pt 240

    if ( ((const SvxEmphasisMarkItem&)rPattern.
            GetItem(ATTR_FONT_EMPHASISMARK)).GetEmphasisMark() != EMPHASISMARK_NONE )
    {
        //  add height for emphasis marks
        //! font metrics should be used instead
        nHeight += nHeight / 4;
    }

    if ( nHeight + 240 > ScGlobal::nDefFontHeight )
    {
        nHeight += ScGlobal::nDefFontHeight;
        nHeight -= 240;
    }

    //  Standard-Hoehe: TextHeight + Raender - 23
    //  -> 257 unter Windows

    if (nHeight > STD_ROWHEIGHT_DIFF)
        nHeight -= STD_ROWHEIGHT_DIFF;

    nHeight += pMargin->GetTopMargin() + pMargin->GetBottomMargin();

    return nHeight;
}

//  pHeight in Twips
//  nMinHeight, nMinStart zur Optimierung: ab nRow >= nMinStart ist mindestens nMinHeight
//  (wird nur bei bStdAllowed ausgewertet)

void ScColumn::GetOptimalHeight( SCROW nStartRow, SCROW nEndRow, USHORT* pHeight,
                                OutputDevice* pDev,
                                double nPPTX, double nPPTY,
                                const Fraction& rZoomX, const Fraction& rZoomY,
                                BOOL bShrink, USHORT nMinHeight, SCROW nMinStart )
{
    ScAttrIterator aIter( pAttrArray, nStartRow, nEndRow );

    SCROW nStart;
    SCROW nEnd;
    SCROW nEditPos = 0;
    SCROW nNextEnd = 0;

    //  bei bedingter Formatierung werden immer die einzelnen Zellen angesehen

    const ScPatternAttr* pPattern = aIter.Next(nStart,nEnd);
    while ( pPattern )
    {
        const ScMergeAttr*      pMerge = (const ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
        const ScMergeFlagAttr*  pFlag = (const ScMergeFlagAttr*)&pPattern->GetItem(ATTR_MERGE_FLAG);
        if ( pMerge->GetRowMerge() > 1 || pFlag->IsOverlapped() )
        {
            //  nix - vertikal bei der zusammengefassten und den ueberdeckten,
            //        horizontal nur bei den ueberdeckten (unsichtbaren) -
            //        eine nur horizontal zusammengefasste wird aber beruecksichtigt
        }
        else
        {
            SCROW nRow;
            BOOL bStdAllowed = (pPattern->GetCellOrientation() == SVX_ORIENTATION_STANDARD);
            BOOL bStdOnly = FALSE;
            if (bStdAllowed)
            {
                BOOL bBreak = ((SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK)).GetValue() ||
                                ((SvxCellHorJustify)((const SvxHorJustifyItem&)pPattern->
                                    GetItem( ATTR_HOR_JUSTIFY )).GetValue() ==
                                    SVX_HOR_JUSTIFY_BLOCK);
                bStdOnly = !bBreak;

                // bedingte Formatierung: Zellen durchgehen
                if ( bStdOnly && ((const SfxUInt32Item&)pPattern->
                                    GetItem(ATTR_CONDITIONAL)).GetValue() )
                    bStdOnly = FALSE;

                // gedrehter Text: Zellen durchgehen
                if ( bStdOnly && ((const SfxInt32Item&)pPattern->
                                    GetItem(ATTR_ROTATE_VALUE)).GetValue() )
                    bStdOnly = FALSE;
            }

            if (bStdOnly)
                if (HasEditCells(nStart,nEnd,nEditPos))     // includes mixed script types
                {
                    if (nEditPos == nStart)
                    {
                        bStdOnly = FALSE;
                        if (nEnd > nEditPos)
                            nNextEnd = nEnd;
                        nEnd = nEditPos;                // einzeln ausrechnen
                        bStdAllowed = FALSE;            // wird auf jeden Fall per Zelle berechnet
                    }
                    else
                    {
                        nNextEnd = nEnd;
                        nEnd = nEditPos - 1;            // Standard - Teil
                    }
                }

            if (bStdAllowed)
            {
                USHORT nLatHeight = 0;
                USHORT nCjkHeight = 0;
                USHORT nCtlHeight = 0;
                USHORT nDefHeight;
                BYTE nDefScript = ScGlobal::GetDefaultScriptType();
                if ( nDefScript == SCRIPTTYPE_ASIAN )
                    nDefHeight = nCjkHeight = lcl_GetAttribHeight( *pPattern, ATTR_CJK_FONT_HEIGHT );
                else if ( nDefScript == SCRIPTTYPE_COMPLEX )
                    nDefHeight = nCtlHeight = lcl_GetAttribHeight( *pPattern, ATTR_CTL_FONT_HEIGHT );
                else
                    nDefHeight = nLatHeight = lcl_GetAttribHeight( *pPattern, ATTR_FONT_HEIGHT );

                //  if everything below is already larger, the loop doesn't have to
                //  be run again
                SCROW nStdEnd = nEnd;
                if ( nDefHeight <= nMinHeight && nStdEnd >= nMinStart )
                    nStdEnd = (nMinStart>0) ? nMinStart-1 : 0;

                for (nRow=nStart; nRow<=nStdEnd; nRow++)
                    if (nDefHeight > pHeight[nRow-nStartRow])
                        pHeight[nRow-nStartRow] = nDefHeight;

                if ( bStdOnly )
                {
                    //  if cells are not handled individually below,
                    //  check for cells with different script type

                    SCSIZE nIndex;
                    Search(nStart,nIndex);
                    while ( nIndex < nCount && (nRow=pItems[nIndex].nRow) <= nEnd )
                    {
                        BYTE nScript = pDocument->GetScriptType( nCol, nRow, nTab, pItems[nIndex].pCell );
                        if ( nScript != nDefScript )
                        {
                            if ( nScript == SCRIPTTYPE_ASIAN )
                            {
                                if ( nCjkHeight == 0 )
                                    nCjkHeight = lcl_GetAttribHeight( *pPattern, ATTR_CJK_FONT_HEIGHT );
                                if (nCjkHeight > pHeight[nRow-nStartRow])
                                    pHeight[nRow-nStartRow] = nCjkHeight;
                            }
                            else if ( nScript == SCRIPTTYPE_COMPLEX )
                            {
                                if ( nCtlHeight == 0 )
                                    nCtlHeight = lcl_GetAttribHeight( *pPattern, ATTR_CTL_FONT_HEIGHT );
                                if (nCtlHeight > pHeight[nRow-nStartRow])
                                    pHeight[nRow-nStartRow] = nCtlHeight;
                            }
                            else
                            {
                                if ( nLatHeight == 0 )
                                    nLatHeight = lcl_GetAttribHeight( *pPattern, ATTR_FONT_HEIGHT );
                                if (nLatHeight > pHeight[nRow-nStartRow])
                                    pHeight[nRow-nStartRow] = nLatHeight;
                            }
                        }
                        ++nIndex;
                    }
                }
            }

            if (!bStdOnly)                      // belegte Zellen suchen
            {
                ScNeededSizeOptions aOptions;

                SCSIZE nIndex;
                Search(nStart,nIndex);
                while ( (nIndex < nCount) ? ((nRow=pItems[nIndex].nRow) <= nEnd) : FALSE )
                {
                    //  Zellhoehe nur berechnen, wenn sie spaeter auch gebraucht wird (#37928#)

                    if ( bShrink || !(pDocument->GetRowFlags(nRow, nTab) & CR_MANUALSIZE) )
                    {
                        aOptions.pPattern = pPattern;
                        USHORT nHeight = (USHORT)
                                ( GetNeededSize( nRow, pDev, nPPTX, nPPTY,
                                                    rZoomX, rZoomY, FALSE, aOptions ) / nPPTY );
                        if (nHeight > pHeight[nRow-nStartRow])
                            pHeight[nRow-nStartRow] = nHeight;
                    }
                    ++nIndex;
                }
            }
        }

        if (nNextEnd > 0)
        {
            nStart = nEnd + 1;
            nEnd = nNextEnd;
            nNextEnd = 0;
        }
        else
            pPattern = aIter.Next(nStart,nEnd);
    }
}

BOOL ScColumn::GetNextSpellingCell(SCROW& nRow, BOOL bInSel, const ScMarkData& rData) const
{
    BOOL bStop = FALSE;
    CellType eCellType;
    SCSIZE nIndex;
    if (!bInSel && Search(nRow, nIndex))
    {
        eCellType = GetCellType(nRow);
        if ( (eCellType == CELLTYPE_STRING || eCellType == CELLTYPE_EDIT) &&
             !(HasAttrib( nRow, nRow, HASATTR_PROTECTED) &&
               pDocument->IsTabProtected(nTab)) )
                return TRUE;
    }
    while (!bStop)
    {
        if (bInSel)
        {
            nRow = rData.GetNextMarked(nCol, nRow, FALSE);
            if (!ValidRow(nRow))
            {
                nRow = MAXROW+1;
                bStop = TRUE;
            }
            else
            {
                eCellType = GetCellType(nRow);
                if ( (eCellType == CELLTYPE_STRING || eCellType == CELLTYPE_EDIT) &&
                     !(HasAttrib( nRow, nRow, HASATTR_PROTECTED) &&
                       pDocument->IsTabProtected(nTab)) )
                        return TRUE;
                else
                    nRow++;
            }
        }
        else if (GetNextDataPos(nRow))
        {
            eCellType = GetCellType(nRow);
            if ( (eCellType == CELLTYPE_STRING || eCellType == CELLTYPE_EDIT) &&
                 !(HasAttrib( nRow, nRow, HASATTR_PROTECTED) &&
                   pDocument->IsTabProtected(nTab)) )
                    return TRUE;
            else
                nRow++;
        }
        else
        {
            nRow = MAXROW+1;
            bStop = TRUE;
        }
    }
    return FALSE;
}

// =========================================================================================

void ScColumn::RemoveAutoSpellObj()
{
    ScTabEditEngine* pEngine = NULL;

    for (USHORT i=0; i<nCount; i++)
        if ( pItems[i].pCell->GetCellType() == CELLTYPE_EDIT )
        {
            ScEditCell* pOldCell = (ScEditCell*) pItems[i].pCell;
            const EditTextObject* pData = pOldCell->GetData();
            //  keine Abfrage auf HasOnlineSpellErrors, damit es auch
            //  nach dem Laden funktioniert

            //  Fuer den Test auf harte Formatierung (ScEditAttrTester) sind die Defaults
            //  in der EditEngine unwichtig. Wenn der Tester spaeter einmal gleiche
            //  Attribute in Default und harter Formatierung erkennen und weglassen sollte,
            //  muessten an der EditEngine zu jeder Zelle die richtigen Defaults gesetzt
            //  werden!

            //  auf Attribute testen
            if ( !pEngine )
                pEngine = new ScTabEditEngine(pDocument);
            pEngine->SetText( *pData );
            ScEditAttrTester aTester( pEngine );
            if ( aTester.NeedsObject() )                    // nur Spell-Errors entfernen
            {
                EditTextObject* pNewData = pEngine->CreateTextObject(); // ohne BIGOBJ
                pOldCell->SetData( pNewData, pEngine->GetEditTextObjectPool() );
                delete pNewData;
            }
            else                                            // String erzeugen
            {
                String aText = ScEditUtil::GetSpaceDelimitedString( *pEngine );
                ScBaseCell* pNewCell = new ScStringCell( aText );
                SvtBroadcaster* pBC = pOldCell->GetBroadcaster();
                pNewCell->SetBroadcaster( pBC );
                pOldCell->ForgetBroadcaster();
                if (pOldCell->GetNotePtr())
                    pNewCell->SetNote( *pOldCell->GetNotePtr() );
                pItems[i].pCell = pNewCell;
                delete pOldCell;
            }
        }

    delete pEngine;
}

void ScColumn::RemoveEditAttribs( SCROW nStartRow, SCROW nEndRow )
{
    ScFieldEditEngine* pEngine = NULL;

    SCSIZE i;
    Search( nStartRow, i );
    for (; i<nCount && pItems[i].nRow <= nEndRow; i++)
        if ( pItems[i].pCell->GetCellType() == CELLTYPE_EDIT )
        {
            ScEditCell* pOldCell = (ScEditCell*) pItems[i].pCell;
            const EditTextObject* pData = pOldCell->GetData();

            //  Fuer den Test auf harte Formatierung (ScEditAttrTester) sind die Defaults
            //  in der EditEngine unwichtig. Wenn der Tester spaeter einmal gleiche
            //  Attribute in Default und harter Formatierung erkennen und weglassen sollte,
            //  muessten an der EditEngine zu jeder Zelle die richtigen Defaults gesetzt
            //  werden!

            //  auf Attribute testen
            if ( !pEngine )
            {
                //pEngine = new ScTabEditEngine(pDocument);
                pEngine = new ScFieldEditEngine( pDocument->GetEditPool() );
                //  EE_CNTRL_ONLINESPELLING falls schon Fehler drin sind
                pEngine->SetControlWord( pEngine->GetControlWord() | EE_CNTRL_ONLINESPELLING );
                pEngine->SetForbiddenCharsTable( pDocument->GetForbiddenCharacters() );
                pEngine->SetAsianCompressionMode( pDocument->GetAsianCompression() );
                pEngine->SetKernAsianPunctuation( pDocument->GetAsianKerning() );
            }
            pEngine->SetText( *pData );
            USHORT nParCount = pEngine->GetParagraphCount();
            for (USHORT nPar=0; nPar<nParCount; nPar++)
            {
                pEngine->QuickRemoveCharAttribs( nPar );
                const SfxItemSet& rOld = pEngine->GetParaAttribs( nPar );
                if ( rOld.Count() )
                {
                    SfxItemSet aNew( *rOld.GetPool(), rOld.GetRanges() );   // leer
                    pEngine->SetParaAttribs( nPar, aNew );
                }
            }
            //  URL-Felder in Text wandeln (andere gibt's nicht, darum pType=0)
            pEngine->RemoveFields( TRUE );

            BOOL bSpellErrors = pEngine->HasOnlineSpellErrors();
            BOOL bNeedObject = bSpellErrors || nParCount>1;         // Errors/Absaetze behalten
            //  ScEditAttrTester nicht mehr noetig, Felder sind raus

            if ( bNeedObject )                                      // bleibt Edit-Zelle
            {
                ULONG nCtrl = pEngine->GetControlWord();
                ULONG nWantBig = bSpellErrors ? EE_CNTRL_ALLOWBIGOBJS : 0;
                if ( ( nCtrl & EE_CNTRL_ALLOWBIGOBJS ) != nWantBig )
                    pEngine->SetControlWord( (nCtrl & ~EE_CNTRL_ALLOWBIGOBJS) | nWantBig );
                EditTextObject* pNewData = pEngine->CreateTextObject();
                pOldCell->SetData( pNewData, pEngine->GetEditTextObjectPool() );
                delete pNewData;
            }
            else                                            // String erzeugen
            {
                String aText = ScEditUtil::GetSpaceDelimitedString( *pEngine );
                ScBaseCell* pNewCell = new ScStringCell( aText );
                SvtBroadcaster* pBC = pOldCell->GetBroadcaster();
                pNewCell->SetBroadcaster( pBC );
                pOldCell->ForgetBroadcaster();
                if (pOldCell->GetNotePtr())
                    pNewCell->SetNote( *pOldCell->GetNotePtr() );
                pItems[i].pCell = pNewCell;
                delete pOldCell;
            }
        }

    delete pEngine;
}

// =========================================================================================

BOOL ScColumn::TestTabRefAbs(SCTAB nTable)
{
    BOOL bRet = FALSE;
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
            if ( pItems[i].pCell->GetCellType() == CELLTYPE_FORMULA )
                if (((ScFormulaCell*)pItems[i].pCell)->TestTabRefAbs(nTable))
                    bRet = TRUE;
    return bRet;
}

// =========================================================================================

ScColumnIterator::ScColumnIterator( const ScColumn* pCol, SCROW nStart, SCROW nEnd ) :
    pColumn( pCol ),
    nTop( nStart ),
    nBottom( nEnd )
{
    pColumn->Search( nTop, nPos );
}

ScColumnIterator::~ScColumnIterator()
{
}

BOOL ScColumnIterator::Next( SCROW& rRow, ScBaseCell*& rpCell )
{
    if ( nPos < pColumn->nCount )
    {
        rRow = pColumn->pItems[nPos].nRow;
        if ( rRow <= nBottom )
        {
            rpCell = pColumn->pItems[nPos].pCell;
            ++nPos;
            return TRUE;
        }
    }

    rRow = 0;
    rpCell = NULL;
    return FALSE;
}

SCSIZE ScColumnIterator::GetIndex() const           // Index zur letzen abgefragten Zelle
{
    return nPos - 1;        // bei Next ist Pos hochgezaehlt worden
}

// -----------------------------------------------------------------------------------------

ScMarkedDataIter::ScMarkedDataIter( const ScColumn* pCol, const ScMarkData* pMarkData,
                                    BOOL bAllIfNone ) :
    pColumn( pCol ),
    pMarkIter( NULL ),
    bNext( TRUE ),
    bAll( bAllIfNone )
{
    if (pMarkData && pMarkData->IsMultiMarked())
        pMarkIter = new ScMarkArrayIter( pMarkData->GetArray() + pCol->GetCol() );
}

ScMarkedDataIter::~ScMarkedDataIter()
{
    delete pMarkIter;
}

BOOL ScMarkedDataIter::Next( SCSIZE& rIndex )
{
    BOOL bFound = FALSE;
    do
    {
        if (bNext)
        {
            if (!pMarkIter || !pMarkIter->Next( nTop, nBottom ))
            {
                if (bAll)                   // ganze Spalte
                {
                    nTop    = 0;
                    nBottom = MAXROW;
                }
                else
                    return FALSE;
            }
            pColumn->Search( nTop, nPos );
            bNext = FALSE;
            bAll  = FALSE;                  // nur beim ersten Versuch
        }

        if ( nPos >= pColumn->nCount )
            return FALSE;

        if ( pColumn->pItems[nPos].nRow <= nBottom )
            bFound = TRUE;
        else
            bNext = TRUE;
    }
    while (!bFound);

    rIndex = nPos++;
    return TRUE;
}

USHORT ScColumn::GetErrorData( SCROW nRow ) const
{
    SCSIZE  nIndex;
    if (Search(nRow, nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        switch (pCell->GetCellType())
        {
            case CELLTYPE_FORMULA :
                return ((ScFormulaCell*)pCell)->GetErrCode();
            break;
            default:
            return 0;
        }
    }
    return 0;
}

//------------

BOOL ScColumn::IsEmptyData() const
{
    return (nCount == 0);
}

BOOL ScColumn::IsEmptyVisData(BOOL bNotes) const
{
    if (!pItems || nCount == 0)
        return TRUE;
    else
    {
        BOOL bVisData = FALSE;
        USHORT i;
        for (i=0; i<nCount && !bVisData; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() != CELLTYPE_NOTE || (bNotes && pCell->GetNotePtr()) )
                bVisData = TRUE;
        }
        return !bVisData;
    }
}

SCSIZE ScColumn::VisibleCount( SCROW nStartRow, SCROW nEndRow ) const
{
    //  Notizen werden nicht mitgezaehlt

    SCSIZE nVisCount = 0;
    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    while ( nIndex < nCount && pItems[nIndex].nRow <= nEndRow )
    {
        if ( pItems[nIndex].nRow >= nStartRow &&
             pItems[nIndex].pCell->GetCellType() != CELLTYPE_NOTE )
        {
            ++nVisCount;
        }
        ++nIndex;
    }
    return nVisCount;
}

SCROW ScColumn::GetLastVisDataPos(BOOL bNotes) const
{
    SCROW nRet = 0;
    if (pItems)
    {
        SCSIZE i;
        BOOL bFound = FALSE;
        for (i=nCount; i>0 && !bFound; )
        {
            --i;
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() != CELLTYPE_NOTE || (bNotes && pCell->GetNotePtr()) )
            {
                bFound = TRUE;
                nRet = pItems[i].nRow;
            }
        }
    }
    return nRet;
}

SCROW ScColumn::GetFirstVisDataPos(BOOL bNotes) const
{
    SCROW nRet = 0;
    if (pItems)
    {
        SCSIZE i;
        BOOL bFound = FALSE;
        for (i=0; i<nCount && !bFound; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() != CELLTYPE_NOTE || (bNotes && pCell->GetNotePtr()) )
            {
                bFound = TRUE;
                nRet = pItems[i].nRow;
            }
        }
    }
    return nRet;
}

BOOL ScColumn::HasVisibleDataAt(SCROW nRow) const
{
    SCSIZE nIndex;
    if (Search(nRow, nIndex))
        if (CellVisible(pItems[nIndex].pCell))
            return TRUE;

    return FALSE;
}

BOOL ScColumn::IsEmptyAttr() const
{
    if (pAttrArray)
        return pAttrArray->IsEmpty();
    else
        return TRUE;
}

BOOL ScColumn::IsEmpty() const
{
    return (IsEmptyData() && IsEmptyAttr());
}

BOOL ScColumn::IsEmptyBlock(SCROW nStartRow, SCROW nEndRow) const
{
    if ( nCount == 0 || !pItems )
        return TRUE;

    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    while ( nIndex < nCount && pItems[nIndex].nRow <= nEndRow )
    {
        if ( CellVisible(pItems[nIndex].pCell) )    // found a cell
            return FALSE;                           // not empty
        ++nIndex;
    }
    return TRUE;                                    // no cell found
}

SCSIZE ScColumn::GetEmptyLinesInBlock( SCROW nStartRow, SCROW nEndRow, ScDirection eDir ) const
{
    SCSIZE nLines = 0;
    BOOL bFound = FALSE;
    SCSIZE i;
    if (pItems && (nCount > 0))
    {
        if (eDir == DIR_BOTTOM)
        {
            i = nCount;
            while (!bFound && (i > 0))
            {
                i--;
                if ( pItems[i].nRow < nStartRow )
                    break;
                bFound = pItems[i].nRow <= nEndRow && CellVisible(pItems[i].pCell);
            }
            if (bFound)
                nLines = static_cast<SCSIZE>(nEndRow - pItems[i].nRow);
            else
                nLines = static_cast<SCSIZE>(nEndRow - nStartRow);
        }
        else if (eDir == DIR_TOP)
        {
            i = 0;
            while (!bFound && (i < nCount))
            {
                if ( pItems[i].nRow > nEndRow )
                    break;
                bFound = pItems[i].nRow >= nStartRow && CellVisible(pItems[i].pCell);
                i++;
            }
            if (bFound)
                nLines = static_cast<SCSIZE>(pItems[i-1].nRow - nStartRow);
            else
                nLines = static_cast<SCSIZE>(nEndRow - nStartRow);
        }
    }
    else
        nLines = static_cast<SCSIZE>(nEndRow - nStartRow);
    return nLines;
}

SCROW ScColumn::GetFirstDataPos() const
{
    if (nCount)
        return pItems[0].nRow;
    else
        return 0;
}

SCROW ScColumn::GetLastDataPos() const
{
    if (nCount)
        return pItems[nCount-1].nRow;
    else
        return 0;
}

BOOL ScColumn::GetPrevDataPos(SCROW& rRow) const
{
    BOOL bFound = FALSE;
    SCSIZE i = nCount;
    while (!bFound && (i > 0))
    {
        --i;
        bFound = (pItems[i].nRow < rRow);
        if (bFound)
            rRow = pItems[i].nRow;
    }
    return bFound;
}

BOOL ScColumn::GetNextDataPos(SCROW& rRow) const        // groesser als rRow
{
    SCSIZE nIndex;
    if (Search( rRow, nIndex ))
        ++nIndex;                   // naechste Zelle

    BOOL bMore = ( nIndex < nCount );
    if ( bMore )
        rRow = pItems[nIndex].nRow;
    return bMore;
}

void ScColumn::FindDataAreaPos(SCROW& rRow, long nMovY) const
{
    if (!nMovY) return;
    BOOL bForward = (nMovY>0);

    SCSIZE nIndex;
    BOOL bThere = Search(rRow, nIndex);
    if (bThere && !CellVisible(pItems[nIndex].pCell))
        bThere = FALSE;

    if (bThere)
    {
        SCROW nLast = rRow;
        SCSIZE nOldIndex = nIndex;
        if (bForward)
        {
            if (nIndex<nCount-1)
            {
                ++nIndex;
                while (nIndex<nCount-1 && pItems[nIndex].nRow==nLast+1
                                        && CellVisible(pItems[nIndex].pCell))
                {
                    ++nIndex;
                    ++nLast;
                }
                if (nIndex==nCount-1)
                    if (pItems[nIndex].nRow==nLast+1 && CellVisible(pItems[nIndex].pCell))
                        ++nLast;
            }
        }
        else
        {
            if (nIndex>0)
            {
                --nIndex;
                while (nIndex>0 && pItems[nIndex].nRow+1==nLast
                                        && CellVisible(pItems[nIndex].pCell))
                {
                    --nIndex;
                    --nLast;
                }
                if (nIndex==0)
                    if (pItems[nIndex].nRow+1==nLast && CellVisible(pItems[nIndex].pCell))
                        --nLast;
            }
        }
        if (nLast==rRow)
        {
            bThere = FALSE;
            nIndex = bForward ? nOldIndex+1 : nOldIndex;
        }
        else
            rRow = nLast;
    }

    if (!bThere)
    {
        if (bForward)
        {
            while (nIndex<nCount && !CellVisible(pItems[nIndex].pCell))
                ++nIndex;
            if (nIndex<nCount)
                rRow = pItems[nIndex].nRow;
            else
                rRow = MAXROW;
        }
        else
        {
            while (nIndex>0 && !CellVisible(pItems[nIndex-1].pCell))
                --nIndex;
            if (nIndex>0)
                rRow = pItems[nIndex-1].nRow;
            else
                rRow = 0;
        }
    }
}

BOOL ScColumn::HasDataAt(SCROW nRow) const
{
/*  SCSIZE nIndex;
    return Search( nRow, nIndex );
*/
        //  immer nur sichtbare interessant ?
        //! dann HasVisibleDataAt raus

    SCSIZE nIndex;
    if (Search(nRow, nIndex))
        if (CellVisible(pItems[nIndex].pCell))
            return TRUE;

    return FALSE;

}

SCROW ScColumn::GetFirstEntryPos() const
{
    if (pAttrArray)
        return Min( GetFirstDataPos(), pAttrArray->GetFirstEntryPos() );
    else
        return GetFirstDataPos();
}

SCROW ScColumn::GetLastEntryPos() const
{
    if (pAttrArray)
        return Max( GetLastDataPos(), pAttrArray->GetLastEntryPos(TRUE) );
    else
        return GetLastDataPos();
}

SCROW ScColumn::GetLastAttrPos() const
{
    if (pAttrArray)
        return pAttrArray->GetLastEntryPos(FALSE);
    else
        return 0;
}

BOOL ScColumn::IsAllAttrEqual( const ScColumn& rCol, SCROW nStartRow, SCROW nEndRow ) const
{
    if (pAttrArray && rCol.pAttrArray)
        return pAttrArray->IsAllEqual( *rCol.pAttrArray, nStartRow, nEndRow );
    else
        return !pAttrArray && !rCol.pAttrArray;
}

BOOL ScColumn::IsVisibleAttrEqual( const ScColumn& rCol, SCROW nStartRow, SCROW nEndRow ) const
{
    if (pAttrArray && rCol.pAttrArray)
        return pAttrArray->IsVisibleEqual( *rCol.pAttrArray, nStartRow, nEndRow );
    else
        return !pAttrArray && !rCol.pAttrArray;
}

BOOL ScColumn::HasVisibleAttr( SCROW& rFirstRow, SCROW& rLastRow, BOOL bSkipFirst ) const
{
    if (pAttrArray)
        return pAttrArray->HasVisibleAttr(rFirstRow,rLastRow,bSkipFirst);
    else
        return FALSE;
}

BOOL ScColumn::HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const
{
    if (pAttrArray)
        return pAttrArray->HasVisibleAttrIn( nStartRow, nEndRow );
    else
        return FALSE;
}

void ScColumn::FindUsed( SCROW nStartRow, SCROW nEndRow, BOOL* pUsed ) const
{
    SCROW nRow;
    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    while ( (nIndex < nCount) ? ((nRow=pItems[nIndex].nRow) <= nEndRow) : FALSE )
    {
        pUsed[nRow-nStartRow] = TRUE;
        ++nIndex;
    }
}

void ScColumn::StartListening( SvtListener& rLst, SCROW nRow )
{
    SvtBroadcaster* pBC = NULL;
    ScBaseCell* pCell;

    SCSIZE nIndex;
    if (Search(nRow,nIndex))
    {
        pCell = pItems[nIndex].pCell;
        pBC = pCell->GetBroadcaster();
    }
    else
    {
        pCell = new ScNoteCell;
        Insert(nRow, pCell);
    }

    if (!pBC)
    {
        pBC = new SvtBroadcaster;
        pCell->SetBroadcaster(pBC);
    }
    rLst.StartListening(*pBC);
}

void ScColumn::MoveListeners( SvtBroadcaster& rSource, SCROW nDestRow )
{
    SvtBroadcaster* pBC = NULL;
    ScBaseCell* pCell;

    SCSIZE nIndex;
    if (Search(nDestRow,nIndex))
    {
        pCell = pItems[nIndex].pCell;
        pBC = pCell->GetBroadcaster();
    }
    else
    {
        pCell = new ScNoteCell;
        Insert(nDestRow, pCell);
    }

    if (!pBC)
    {
        pBC = new SvtBroadcaster;
        pCell->SetBroadcaster(pBC);
    }

    if (rSource.HasListeners())
    {
        SvtListenerIter aIter( rSource);
        for (SvtListener* pLst = aIter.GoStart(); pLst; pLst = aIter.GoNext())
        {
            pLst->StartListening( *pBC);
            pLst->EndListening( rSource);
        }
    }
}

void ScColumn::EndListening( SvtListener& rLst, SCROW nRow )
{
    SCSIZE nIndex;
    if (Search(nRow,nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        SvtBroadcaster* pBC = pCell->GetBroadcaster();
        if (pBC)
        {
            rLst.EndListening(*pBC);

            if (!pBC->HasListeners())
            {
                if (pCell->GetCellType() == CELLTYPE_NOTE && !pCell->GetNotePtr())
                    DeleteAtIndex(nIndex);
                else
                    pCell->SetBroadcaster(NULL);
            }
        }
//      else
//          DBG_ERROR("ScColumn::EndListening - kein Broadcaster");
    }
//  else
//      DBG_ERROR("ScColumn::EndListening - keine Zelle");
}

void ScColumn::CompileDBFormula()
{
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileDBFormula();
        }
}

void ScColumn::CompileDBFormula( BOOL bCreateFormulaString )
{
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileDBFormula( bCreateFormulaString );
        }
}

void ScColumn::CompileNameFormula( BOOL bCreateFormulaString )
{
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileNameFormula( bCreateFormulaString );
        }
}

void ScColumn::CompileColRowNameFormula()
{
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileColRowNameFormula();
        }
}

void lcl_UpdateSubTotal( ScFunctionData& rData, ScBaseCell* pCell )
{
    double nValue;
    BOOL bVal = FALSE;
    BOOL bCell = TRUE;
    switch (pCell->GetCellType())
    {
        case CELLTYPE_VALUE:
            nValue = ((ScValueCell*)pCell)->GetValue();
            bVal = TRUE;
            break;
        case CELLTYPE_FORMULA:
            {
                if ( rData.eFunc != SUBTOTAL_FUNC_CNT2 )        // da interessiert's nicht
                {
                    ScFormulaCell* pFC = (ScFormulaCell*)pCell;
                    if ( pFC->GetErrCode() )
                    {
                        if ( rData.eFunc != SUBTOTAL_FUNC_CNT ) // fuer Anzahl einfach weglassen
                            rData.bError = TRUE;
                    }
                    else if (pFC->IsValue())
                    {
                        nValue = pFC->GetValue();
                        bVal = TRUE;
                    }
                    // sonst Text
                }
            }
            break;
        case CELLTYPE_NOTE:
            bCell = FALSE;
            break;
        // bei Strings nichts
    }

    if (!rData.bError)
    {
        switch (rData.eFunc)
        {
            case SUBTOTAL_FUNC_SUM:
            case SUBTOTAL_FUNC_AVE:
                if (bVal)
                {
                    ++rData.nCount;
                    if (!SubTotal::SafePlus( rData.nVal, nValue ))
                        rData.bError = TRUE;
                }
                break;
            case SUBTOTAL_FUNC_CNT:             // nur Werte
                if (bVal)
                    ++rData.nCount;
                break;
            case SUBTOTAL_FUNC_CNT2:            // alle
                if (bCell)
                    ++rData.nCount;
                break;
            case SUBTOTAL_FUNC_MAX:
                if (bVal)
                    if (++rData.nCount == 1 || nValue > rData.nVal )
                        rData.nVal = nValue;
                break;
            case SUBTOTAL_FUNC_MIN:
                if (bVal)
                    if (++rData.nCount == 1 || nValue < rData.nVal )
                        rData.nVal = nValue;
                break;
        }
    }
}

//  Mehrfachselektion:
void ScColumn::UpdateSelectionFunction( const ScMarkData& rMark,
                                        ScFunctionData& rData, const BYTE* pRowFlags,
                                        BOOL bDoExclude, SCROW nExStartRow, SCROW nExEndRow )
{
    SCSIZE nIndex;
    ScMarkedDataIter aDataIter(this, &rMark, FALSE);
    while (aDataIter.Next( nIndex ))
    {
        SCROW nRow = pItems[nIndex].nRow;
        if ( !pRowFlags || !( pRowFlags[nRow] & CR_HIDDEN ) )
            if ( !bDoExclude || nRow < nExStartRow || nRow > nExEndRow )
                lcl_UpdateSubTotal( rData, pItems[nIndex].pCell );
    }
}

//  bei bNoMarked die Mehrfachselektion weglassen
void ScColumn::UpdateAreaFunction( ScFunctionData& rData, BYTE* pRowFlags,
                                    SCROW nStartRow, SCROW nEndRow )
{
    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    while ( nIndex<nCount && pItems[nIndex].nRow<=nEndRow )
    {
        SCROW nRow = pItems[nIndex].nRow;
        if ( !pRowFlags || !( pRowFlags[nRow] & CR_HIDDEN ) )
            lcl_UpdateSubTotal( rData, pItems[nIndex].pCell );
        ++nIndex;
    }
}

ULONG ScColumn::GetWeightedCount() const
{
    ULONG nTotal = 0;

    //  Notizen werden nicht gezaehlt

    for (SCSIZE i=0; i<nCount; i++)
    {
        ScBaseCell* pCell = pItems[i].pCell;
        switch ( pCell->GetCellType() )
        {
            case CELLTYPE_VALUE:
            case CELLTYPE_STRING:
                ++nTotal;
                break;
            case CELLTYPE_FORMULA:
                nTotal += 5 + ((ScFormulaCell*)pCell)->GetCode()->GetCodeLen();
                break;
            case CELLTYPE_EDIT:
                nTotal += 50;
                break;
        }
    }

    return nTotal;
}

ULONG ScColumn::GetCodeCount() const
{
    ULONG nCodeCount = 0;

    for (USHORT i=0; i<nCount; i++)
    {
        ScBaseCell* pCell = pItems[i].pCell;
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            nCodeCount += ((ScFormulaCell*)pCell)->GetCode()->GetCodeLen();
    }

    return nCodeCount;
}





