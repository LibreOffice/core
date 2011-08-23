/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef _MSC_VER
#pragma hdrstop
#endif

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include "hintids.hxx"

#include <bf_svx/boxitem.hxx>
#include <bf_svx/fontitem.hxx>
#include <bf_svx/fhgtitem.hxx>
#include <bf_svx/colritem.hxx>
#include <bf_svx/wghtitem.hxx>
#include <bf_svx/postitem.hxx>
#include <bf_svx/udlnitem.hxx>
#include <bf_svx/crsditem.hxx>
#include <bf_svx/cntritem.hxx>
#include <bf_svx/shdditem.hxx>
#include <bf_svx/brshitem.hxx>
#include <bf_svx/adjitem.hxx>
#include <bf_svtools/zforlist.hxx>

#include <fmtfsize.hxx>
#include <swtblfmt.hxx>
#include <node.hxx>
#include "doc.hxx"
#include "pam.hxx"
#include "swtypes.hxx"
#include "swtable.hxx"
#include "fltglbls.hxx"
#include "exccrts.hxx"
#include "excxfbuf.hxx"
#include "excfntbf.hxx"
#include "excdocum.hxx"
#include "excvfbff.hxx"
#include "fltini.hxx"
namespace binfilter {


// -------------------------------------------------- FltColumn::statics -
const UINT16 FltColumn::nDefSize = 128;
//const UINT16 FltColumn::nLevelSize = ( UINT16 ) ( 0.9 * MAXROW );
const UINT16 FltColumn::nLevelSize = ( UINT16 ) ( 0.9 * 8191 );
UINT32 FltColumn::nDefCleared = 0xFFFFFFFFL;
UINT16 FltColumn::nTab;




#ifdef USED
// ------------------------------------------------------ class SpString -
// Kopiert aus: spstring.cxx

// ---------------------------------------- SpString::SpString( UINT16 ) -
SpString::SpString( UINT16 nInitSize )
    {
    nSize = nInitSize;
    pData = new sal_Char[ nSize ];
    pLimit = pData + nSize;
    Clear();	// pNullTerm auf Anfang und mit Nullbyte abschliessen
    }

// ----------------------------------------------- SpString::~SpString() -
SpString::~SpString()
    {
    delete[] pData;
    }

// ------------------------------- SpString::operator +=( const sal_Char * ) -
void SpString::operator +=( const sal_Char *pCString )
    {
    // PREC:	pCString	=	Pointer auf C-String
    // POST:	C-String wird an Inhalt in pData angehaengt
    while( *pCString != 0 )
        {
        if( pNullTerm < pLimit )
            *( pNullTerm++ ) = *( pCString++ );
        else
            Grow();
        }

    if( pNullTerm >= pLimit ) Grow();

    *pNullTerm = 0;
    }

// --------------------------------- SpString::operator +=( const sal_Char ) -
void SpString::operator +=( const sal_Char cChar )
    {
    // POST:	cChar wird an Inhalt in pData angehaengt, ausser cChar ist 0
    if( pNullTerm + 2 >= pLimit )	// noch Platz fuer Charakter
        Grow();
    if( cChar != 0 )
        *( pNullTerm++ ) = cChar;

    *pNullTerm = 0;
    }

// ------------------------------------------------- SpString::ExcScName -
void SpString::MakeScName()
    {
    // POST: ersetzt alle Zeichen <> {[a..z],[A..Z],[0..9],ä,Ä,ö,Ö,ü,Ü,ß}
    //		durch '_' (kann man sicher schneller implementieren)
    //PLATTFORMABHAENGIG//
    sal_Char *pAkt;
    register cAkt;
    for( pAkt = pData ; pAkt < pNullTerm ; pAkt++ )
        {
        cAkt = *pAkt;
        if( ( cAkt < 'a' || cAkt > 'z' ) &&
            ( cAkt < 'A' || cAkt > 'Z' ) &&
            ( cAkt < '0' || cAkt > '9' ) &&
            cAkt != '\xE4' && cAkt != '\xC4' && cAkt != '\xF6' && cAkt != '\xD6' &&
            cAkt != '\xFC' && cAkt != '\xDC' && cAkt != '\xDF' )
            {
            *pAkt = '_';
            }
        }
    }

// ---------------------------------------------------- SpString::Grow() -
void SpString::Grow()
    {
    // PREC:	Verdoppelt die Groesse des pData-Arrays
    sal_Char *pNewData;
    register sal_Char *pFrom, *pTo;

    nSize *= 2;
    pNewData = new sal_Char[ nSize ];

    for( pFrom = pData, pTo = pNewData ; pFrom < pLimit ; pFrom++, pTo++ )
        *pTo = *pFrom;
    pNullTerm = pNullTerm - pData + pNewData;
    delete[] pData;
    pData = pNewData;
    pLimit = pData + nSize;
    }

// -------------------------------------------- SpString::Grow( UINT16 ) -
void SpString::Grow( UINT16 nNewSize )
    {
    // PREC:	Verdoppelt die Groesse des pData-Arrays
    sal_Char *pNewData;
    register sal_Char *pFrom, *pTo;

    if( nNewSize > nSize )
        nSize = nNewSize;

    pNewData = new sal_Char[ nSize ];

    for( pFrom = pData, pTo = pNewData ; pFrom <= pNullTerm ; pFrom++, pTo++ )
        *pTo = *pFrom;
    pNullTerm = pNullTerm - pData + pNewData;
    delete[] pData;
    pData = pNewData;
    pLimit = pData + nSize;
    }

// ------------------------------------------------- SpString::Convert() -
SpString &SpString::Convert( CharSet eSource, CharSet eTarget )
    {
    register const sal_Char *pCString;
    register sal_Char *pTempNullTerm;
    register sal_Char cAkt;

    String aTempString( pData );
    aTempString.Convert( eSource, eTarget );

    // String jetzt groesser?
    if( ( INT32 ) ( pNullTerm - pData ) < ( INT32 ) aTempString.Len() )
        Grow( aTempString.Len() + 2 );

    pTempNullTerm = pNullTerm = pData;		// String loeschen
    *pTempNullTerm = 0;

    pCString = ( const sal_Char * ) aTempString;

    do	{
        cAkt =  *( pCString++ );
        *( pTempNullTerm++ ) = cAkt;
        }
    while( cAkt != 0x00 );
    pTempNullTerm--;
    pNullTerm = pTempNullTerm;

    return *this;
    }

// -------------------------------------------------- SpString::Shrink() -
void SpString::Shrink()
    {
    }

#endif
    // used


#ifdef USED
// --------------------------------------------------- class ExcDataPool -

// ------------------------------------------ ExcDataPool::ExcDataPool() -
ExcDataPool::ExcDataPool()
    {
    pUiBack = NULL;
    }

// ----------------------------------------- ExcDataPool::~ExcDataPool() -
ExcDataPool::~ExcDataPool()
    {
    if( pUiBack ) delete pUiBack;
    }

// --------------------------------------- ExcDataPool::InsertEntry(...) -
void ExcDataPool::InsertEntry( const String &rName, ExcEntryType eType,
    const String &rTabName, UINT16 nTab, UINT16 nCS, UINT16 nCE,
    UINT16 nRS, UINT16 nRE/*, UINT32 nStrPos*/ )
    {
    ExcUiEntry aNew;

    aNew.aName = rName;
    aNew.aTabName = rTabName;
    aNew.nTab = nTab;
    aNew.nColStart = nCS;
    aNew.nColEnd = nCE;
    aNew.nRowStart = nRS;
    aNew.nRowEnd = nRE;
    aNew.eType = eType;
    //aNew.nStrPos = nStrPos;

    aInit.Insert( aNew );
    }

// ---------------------------------- ExcDataPool::GetNextSelection(...) -
BOOL ExcDataPool::GetNextSelection( String &rName, UINT16 &rTab,
    UINT16 &rCS,UINT16 &rCE, UINT16 &rRS, UINT16 &rRE/*, UINT32 &rStrPos*/ )
    {
    // POST:	wenn pUiBack == NULL:	Dlg starten und erste Selektion
    //									zurueckliefern
    //			sonst					Daten aus naechsten in UiBack-Liste
    //			return	= TRUE, wenn	Daten gefunden
    //					= FALSE, sonst
    const ExcUiEntry *pNext;
    if( pUiBack )
        {// n-ter Aufruf
        pNext = pUiBack->GetNext();
        }
    else
        {// erster Aufruf
        pUiBack = new ExcUiList;
        // Dialog-Aufruf
        // Dlg( const ExcUiList &rInit, ExcUiList &rUiBack );
        pNext = pUiBack->GetFirst();
        }

    if( pNext )
        {
        rName = pNext->aName;
        rTab = pNext->nTab;
        rCS = pNext->nColStart;
        rCE = pNext->nColEnd;
        rRS = pNext->nRowStart;
        rRE = pNext->nRowEnd;
        //rStrPos = pNext->nStrPos;
        }

    return ( pNext != NULL );
    }

// ----------------------- ExcDataPool::operator<<( const ExcDokument& ) -
ExcDataPool	&ExcDataPool::operator<<( ExcDokument &rExcDok )
    {
    UINT16 nCS, nCE, nRS, nRE, nTab;
    const ExcTabelle *pAktTab = rExcDok.GetFirstTab();

    nTab = 0;

    while( pAktTab )
        {
        pAktTab->GetRange( nCS, nCE, nRS, nRE );

        InsertEntry( pAktTab->GetName(), EET_Table, pAktTab->GetName(),
            nTab, nCS, nCE, nRS, nRE );

        nTab++;
        }

    return *this;
    }


#endif		// #ifdef USED

// ------------------------------------------------ class ColRowSettings -
//
// Kopiert aus: colrowst.cxx


// -------------------------------------- ColRowSettings::ColRowSettings -
ColRowSettings::ColRowSettings()
    {
    nColLimit = 256;
    nRowLimit = 8192;

    nDefWidth = nDefHeight = 0;

    pWidth = new INT32 [ nColLimit ];
    pHeight = new INT32 [ nRowLimit ];
    pColHidden = new BOOL [ nColLimit ];
    pRowHidden = new BOOL [ nRowLimit ];
    pColUsed = new BOOL [ nColLimit ];
    pRowUsed = new BOOL [ nRowLimit ];

    NewTab( 0 );
    }

// ------------------------------------- ColRowSettings::~ColRowSettings -
ColRowSettings::~ColRowSettings()
    {
    delete[] pWidth;
    delete[] pHeight;
    delete[] pColHidden;
    delete[] pRowHidden;
    delete[] pColUsed;
    delete[] pRowUsed;
    }

// ---------------------------------------------- ColRowSettings::NewTab -
void ColRowSettings::NewTab( UINT16 nNew )
    {
    UINT16 nC;
    nAktTab = nNew;

    // FALSE / -1 ist Default
    for( nC = 0 ; nC < nColLimit ; nC++ )
        {
        pColHidden[ nC ] = pColUsed[ nC ] = FALSE;
        pWidth[ nC ] = -1;
        }
    for( nC = 0 ; nC < nRowLimit ; nC++ )
        {
        pRowHidden[ nC ] = pRowUsed[ nC ] = FALSE;
        pHeight[ nC ] = -1;
        }
    }

// ----------------------------------------------- ColRowSettings::Apply -
void ColRowSettings::Apply()
    {
    UINT16 nC, nAnzCols, nAnzRows;
    UINT32 nWidthSum;
    double fFaktor;

    SwFmtFrmSize aSize( ATT_FIX_SIZE, nDefWidth );

    // Column-Bemachung
    nAnzCols = pExcGlob->AnzCols();
    nAnzRows = pExcGlob->AnzRows();
    nWidthSum = 0;
    for( nC = 0 ; nC < nAnzCols ; nC++ )
        {
        if( pWidth[ nC ] < 0 )
            pWidth[ nC ] = nDefWidth;
        else if( pWidth[ nC ] < MINLAY )
            pWidth[ nC ] = MINLAY;

        nWidthSum += pWidth[ nC ];
        }
    fFaktor = 65535.0 / ( double ) nWidthSum;

    for( nC = 0 ; nC < nAnzCols ; nC++ )
        {
        aSize.SetWidth( ( UINT16 ) ( fFaktor * ( double ) pWidth[ nC ] ) );
        SwTableBox *pTBox = pExcGlob->pTable->GetTabLines()[ 0 ]->GetTabBoxes()[ nC ];
        SwTableBoxFmt* pBoxFmt = (SwTableBoxFmt*)pTBox->ClaimFrmFmt();		// Frm-Format eindeutig
        pBoxFmt->SetAttr( aSize );
        for( UINT16 nRow = 1 ; nRow < nAnzRows ; nRow++ )
            pExcGlob->pTable->GetTabLines()[ nRow ]->
                        GetTabBoxes()[ nC ]->ChgFrmFmt( pBoxFmt );
        }
    }


#ifdef USED
// ---------------------------------------- ColRowSettings::HideColRange -
void ColRowSettings::HideColRange( UINT16 nColFirst, UINT16 nColLast )
    {
    DBG_ASSERT( nColFirst <= nColLast,
        "+ColRowSettings::HideColRange(): First > Last?!" );
    }

// ---------------------------------------- ColRowSettings::HideRowRange -
void ColRowSettings::HideRowRange( UINT16 nRowFirst, UINT16 nRowLast )
    {
    DBG_ASSERT( nRowFirst <= nRowLast,
        "+RowRowSettings::HideRowRange(): First > Last?!" );
    }
#endif


// ------------------------------------ ColRowSettings::SetColWidthRange -
void ColRowSettings::SetWidthRange( UINT16 nColFirst, UINT16 nColLast,
    UINT16 nNew )
    {
    DBG_ASSERT( nColFirst <= nColLast,
        "-ColRowSettings::SetColWidthRange(): First > Last?!" );

    if( pExcGlob->ColRangeLimitter( nColFirst, nColLast ) )
        {
        pExcGlob->NormalizeCol( nColFirst );
        pExcGlob->NormalizeCol( nColLast );

        register BOOL *pUsed;
        register INT32 *pWidthCount, *pFinish;
        pWidthCount = &pWidth[ nColFirst ];
        pFinish = &pWidth[ nColLast ];
        pUsed = &pColUsed[ nColFirst ];
        while( pWidthCount <= pFinish )
            {
            *( pUsed++ ) = TRUE;
            *( pWidthCount++ ) = nNew;
            }
        }
    }

// -- class FltColumn ----------------------------------------------------

// ------------------------------------------------ FltColumn::FltColumn -
FltColumn::FltColumn( UINT16 nNewCol )
    {
    // ACHTUNG: interne Nutzung von SW-KOORDINATEN
    pData = new UINT16[ nDefSize ];
    nSize = nDefSize;
    nCol = nNewCol;
    nLastRow = nSize - 1;	// wird in Reset zurueckgesetzt
    Reset();
    }


#ifdef USED
// ------------------------------------------------ FltColumn::FltColumn -
FltColumn::FltColumn( UINT16 nNewCol, UINT16 nNewSize )
    {
    // ACHTUNG: interne Nutzung von SW-KOORDINATEN
    DBG_ASSERT( nNewSize > 0,
        "-FltColumn::Ctor: NewSize == 0! VERBOTEN - WEIL TOEDLICH!" );
    nNewSize += 1;
    nNewSize &= 0xFFFE;
    // jetzt ist nNewSize' mod 2 = 0 und nNewSize' >= nNewSize
    nSize = nNewSize;
    pData = new UINT16[ nSize ];
    nCol = nNewCol;
    nLastRow = nSize - 1;	// wird in Reset zurueckgesetzt
    Reset();
    }

// ------------------------------------------ FltColumn::Reset( UINT16 ) -
void FltColumn::Reset( UINT16 nNewTab )
    {
    // ACHTUNG: interne Nutzung von SW-KOORDINATEN
    nTab = nNewTab;
    Reset();
    }
#endif


// -------------------------------------------------- FltColumn::Reset() -
void FltColumn::Reset()
    {
    // ACHTUNG: interne Nutzung von SW-KOORDINATEN
    UINT16 nClear = ( UINT16 ) nDefCleared;

    for( UINT16 nC = 0 ; nC <= nLastRow ; nC++ )
        pData[ nC ] = nClear;
    }

// ----------------------------------------------------- FltColumn::Grow -
void FltColumn::Grow( UINT16 nRecIndex )
    {
    // ACHTUNG: interne Nutzung von SW-KOORDINATEN
    // PREC:	nRecIndex: Index, der noch in das neue Array passen soll

    DBG_ASSERT( nRecIndex >= nSize,
        "-FltColumn::Grow(): geforderte Groesse <= alte Groesse" );

    UINT16 nNewSize;

    if( nRecIndex >= nLevelSize )
//		nNewSize = MAXROW + 1;
        nNewSize = 8191 + 1;
    else
        {
        nRecIndex++;
        // ab hier ist RecIndex gleichbedeutend mit RecSize
        nNewSize = nSize * 2;		// neue Groesse bestimmen
        // Test auf Peak
        if( nRecIndex > nNewSize )
            {// grosser Index -> nicht wesentlich groesser werden als Index
            nRecIndex += 5;	// 4 groesser, 1 fuer gerades nSize
            nNewSize = nRecIndex & 0xFFFE;
            }
        }

    // jetzt kommt erst das Growen
    UINT16 *pNewData;
    pNewData = new UINT16[ nNewSize ];

    {// alte Daten kopieren, neue resetten				----------------
    // nSize -> nLastRow + 1
    //register UINT16 nCount = nSize / 2;
    register UINT16 nCount = ( nLastRow + 1 ) / 2;
    register UINT32 *pWrite = ( UINT32 * ) pNewData;
    {//													-----------
    register const UINT32 *pRead = ( UINT32 * ) pData;	// kopieren
    do	{
        *( pWrite++ ) = *( pRead++ );
        }
        while( --nCount > 0 );							//
    }//													-----------
    {//													-----------
    //nCount = ( nNewSize - nSize ) / 2;				// resetten
    nCount = ( nNewSize - nLastRow - 1 ) / 2;
    register UINT32 nCleared = nDefCleared;
    do	{
        *( pWrite++ ) = nCleared;
        }
        while( --nCount > 0 );							//
    }//													-----------
    }//													----------------

    nSize = nNewSize;
    pData = pNewData;
    }

// ------------------------------------------------ FltColumn::Apply_int -
void FltColumn::Apply()
    {
    // ACHTUNG: interne Nutzung von SW-KOORDINATEN
    UINT16 nCount, nAnzRows;
    UINT16 nLastXF, nAktXF, nLastCount;

    nLastCount = 0;
    nLastXF = pData[ nLastCount ];

    nAnzRows = pExcGlob->AnzRows();
    for( nCount = 1 ; nCount < nAnzRows ; nCount++ )
        {
        nAktXF = pData[ nCount ];
        if( nAktXF != nLastXF )
            {
            if( nLastXF != ( UINT16 ) nDefCleared )
                {
                pExcGlob->pXF_Buff->SetItemSets(
                    nCol, nLastCount, nCount - 1, nLastXF );
                }
            nLastCount = nCount;
            nLastXF = nAktXF;
            }
        }
    // ...und den Rest applyen
    if( nLastXF != ( UINT16 ) nDefCleared )
        {
//		pD->ApplyPatternAreaTab( nCol, nLastCount, nCol, nCount - 1, nTab,
//			pXF_Buff->GetPattern( nLastXF ) );
        pExcGlob->pXF_Buff->SetItemSets(
            nCol, nLastCount, nCount - 1, nLastXF );
        }
    }

// -- class FltTabelle ---------------------------------------------------

// ---------------------------------------------- FltTabelle::FltTabelle -
FltTabelle::FltTabelle()
    {
    Settings( 0 );
    }

// --------------------------------------------- FltTabelle::~FltTabelle -
FltTabelle::~FltTabelle()
    {
    FltColumn **pDel = pData;
    for( UINT16 nC = 0 ; nC <= nLastCol ; nC++, pDel++ )
        if( *pDel ) delete *pDel;
    }

// ------------------------------------------------ FltTabelle::Settings -
void FltTabelle::Settings( UINT16 nTab )
    {
    nSize = 8191 + 1;
    nLastCol = 0;
    pData = new FltColumn *[ nSize ];

    pData[ 0 ] = new FltColumn( 0 );	// Column 0 gibt's per Default
    pData[ 0 ]->Settings( nTab );

    for( UINT16 nC = 1 ; nC < nSize ; nC++ ) pData[ nC ] = NULL;
    }

// --------------------------------------------------- FltTabelle::SetXF -
void FltTabelle::SetXF( UINT16 nCol, UINT16 nRow, UINT16 nNewXF )
    {
    // nCol / nRow : Excel-Koordinaten -> SW-Koordinaten
    if( pExcGlob->IsInRange( nCol, nRow ) )
        {
        pExcGlob->Normalize( nCol, nRow );
        if( !pData[ nCol ] )
            {
            pData[ nCol ] = new FltColumn( nCol );
            if( nCol > nLastCol ) nLastCol = nCol;
            }
        pData[ nCol ]->SetXF( nRow, nNewXF );
        }
    }


#ifdef USED
// --------------------------------------------------- FltTabelle::Reset -
void FltTabelle::Reset( UINT16 nTab )
    {
    for( UINT16 nC = 0 ; nC <= nLastCol ; nC++ )
        if( pData[ nC ] ) pData[ nC ]->Reset();
    pData[ 0 ]->SetTab( nTab );
    }
#endif


// --------------------------------------------------- FltTabelle::Apply -
void FltTabelle::Apply()
    {
    for( UINT16 nC = 0 ; nC < pExcGlob->AnzCols() ; nC++ )
        if( pData[ nC ] ) pData[ nC ]->Apply();
    }


// -- class XF_Data ------------------------------------------------------
//
// -------------------------------------------------- XF_Data::XF_Data() -
XF_Data::XF_Data()
    {
    pFill = NULL;
    pBord = NULL;

    bFontValid = FALSE;
    bFormValid = FALSE;

    nParent = nFont = 0;
    eHoriz = EHA_Parent;
    }

// ------------------------------------------------- XF_Data::~XF_Data() -
XF_Data::~XF_Data()
    {
    if( pFill != NULL ) delete pFill;
    if( pBord != NULL ) delete pBord;
    }

// ----------------------------- XF_Data::SetBorder( const CellBorder& ) -
void XF_Data::SetBorder( const CellBorder &rNew )
    {
    if( pBord == NULL ) pBord = new CellBorder;
    DBG_ASSERT( pBord != NULL,
        "-XF_Data::SetBorder(): pBord == NULL!" );
    pBord->nTopLine = rNew.nTopLine;
    pBord->nLeftLine = rNew.nLeftLine;
    pBord->nBottomLine = rNew.nBottomLine;
    pBord->nRightLine = rNew.nRightLine;
    pBord->nTopColor = rNew.nTopColor;
    pBord->nLeftColor = rNew.nLeftColor;
    pBord->nBottomColor = rNew.nBottomColor;
    pBord->nRightColor = rNew.nRightColor;
    }

#if 0
// ------------------------- XF_Data::SetBorder( BYTE, BYTE, BYTE,...  ) -
void XF_Data::SetBorder( BYTE nTopLine, BYTE nLeftLine, BYTE nBottomLine,
    BYTE nRightLine, UINT16 nTopColor, UINT16 nLeftColor,
    UINT16 nBottomColor, UINT16 nRightColor )
    {
    if( pBord == NULL ) pBord = new CellBorder;
    DBG_ASSERT( pBord != NULL,
        "-XF_Data::SetBorder(): pBord == NULL!" );
    pBord->nTopLine = nTopLine;
    pBord->nLeftLine = nLeftLine;
    pBord->nBottomLine = nBottomLine;
    pBord->nRightLine = nRightLine;
    pBord->nTopColor = nTopColor;
    pBord->nLeftColor = nLeftColor;
    pBord->nBottomColor = nBottomColor;
    pBord->nRightColor = nRightColor;
    }
#endif

// -- class XF_Buffer ----------------------------------------------------
//
// Kopiert aus: xfbuff.cxx

// ------------------------------------------------ XF_Buffer::XF_Buffer -
XF_Buffer::XF_Buffer( UINT16 nNewMax )
    {// default: 4K Pattern
    nMax = nNewMax;
    nCount = 0;

    ppTxtAttr = new SfxItemSet *[ nNewMax ];
    ppBoxAttr = new SfxItemSet *[ nNewMax ];

    ppData = new XF_Data *[ nNewMax ];

    pDefaultData = new XF_Data;

    pDefTxtAttr = new SfxItemSet( pExcGlob->pD->GetAttrPool(),
                                    RES_CHRATR_BEGIN, RES_CHRATR_END - 1 );
    pDefBoxAttr = new SfxItemSet( pExcGlob->pD->GetAttrPool(),
                                    RES_FRMATR_BEGIN, RES_FRMATR_END - 1 );

    // alle NULL
    for( UINT16 nC = 0 ; nC < nMax ; nC++ )
        {
        ppTxtAttr[ nC ] = NULL;
        ppBoxAttr[ nC ] = NULL;
        ppData[ nC ] = NULL;
        }
    }

// ----------------------------------------------- XF_Buffer::~XF_Buffer -
XF_Buffer::~XF_Buffer()
    {
    for( UINT16 nC = 0 ; nC < nCount ; nC++ )
        {
        if( ppTxtAttr[ nC ] != NULL )
            {
            delete ppTxtAttr[ nC ];
            delete ppBoxAttr[ nC ];
            }
        if( ppData[ nC ] != NULL ) delete ppData[ nC ];
        }

    delete[] ppData;
    delete[] ppTxtAttr;
    delete[] ppBoxAttr;
    delete pDefaultData;
    delete pDefTxtAttr;
    delete pDefBoxAttr;
    }

// ---------------------------------------- XF_Buffer::ExcToSwBorderLine -
void XF_Buffer::ExcToSwBorderLine( BYTE nLine, UINT16 nColor,
    SvxBorderLine *&rpBorderLine )
    {
    // POST: rpBorderLine = NULL, wenn nLine == 0
    //						eine Linie, sonst
    if( nLine == 0 )
        {
        rpBorderLine = NULL;
        return;
        }

    DBG_ASSERT( nLine < 8,
        "+XF_Buffer::ExcToSwBorderLine(): Line-# groesser als 7!?" );

    const static UINT16 nPara[ 8 ][ 3 ] = {
        // OutWidth, InWidth, Distance
        { 0, 0, 0 },		// (0)
        { DEF_LINE_WIDTH_1, 0, 0 },		// 1, duenn
        { DEF_LINE_WIDTH_2, 0, 0 },		// 2, mittel
        { DEF_LINE_WIDTH_1, 0, 0 },		// 3, gestrichelt lang
        { DEF_LINE_WIDTH_0, 0, 0 },		// 4, gestrichelt kurz
        { DEF_LINE_WIDTH_3, 0, 0 },		// 5, dick
        { DEF_LINE_WIDTH_1, DEF_LINE_WIDTH_1,
                DEF_LINE_WIDTH_1 },		// 6, doppelt
        { DEF_LINE_WIDTH_0, 0, 0 } };	// 7, gepunktet

    rpBorderLine = new SvxBorderLine;	// ACHTUNG: delete im Caller!!!!!!

    rpBorderLine->SetColor(
        ( *pExcGlob->pColorBuff->GetColor( nColor ) ).GetValue() );

    if( nLine < 8 )
        {
        rpBorderLine->SetOutWidth( nPara[ nLine ][ 0 ] );
        rpBorderLine->SetInWidth( nPara[ nLine ][ 1 ] );
        rpBorderLine->SetDistance( nPara[ nLine ][ 2 ] );
        }
    }


#ifdef USED
// ---------------------------------------------------- XF_Buffer::Reset -
void XF_Buffer::Reset()
    {
    for( UINT16 nC = 0 ; nC < nCount ; nC++ )
        if( ppTxtAttr[ nC ] != NULL )
            {
            delete ppTxtAttr[ nC ];
            delete ppBoxAttr[ nC ];
            }

    nCount = 0;
    }
#endif


// ------------------------------------------- XF_Buffer::CreateItemSets -
void XF_Buffer::CreateItemSets( UINT16 nIndex )
    {
    XF_Data *pD;//, *pP;	// Pointer auf eigene und Parent-Daten
    SfxItemSet *pTxtAttr, *pBoxAttr;

    DBG_ASSERT( nIndex < nCount,
        "+XF_Buffer::CreatePattern(): XF nicht im Puffer!" );

    if( nIndex >= nMax )
        {
        pTxtAttr = pDefTxtAttr;	// Defaults
        pBoxAttr = pDefBoxAttr;
        return;
        }

    DBG_ASSERT( ppData[ nIndex ] != NULL,
        "-XF_Buffer::CreatePattern(): Wiiiiiiiiiiiiiie bitte?!" );

    pTxtAttr = new SfxItemSet( pExcGlob->pD->GetAttrPool(),
                                RES_CHRATR_BEGIN, RES_PARATR_ADJUST );
    pBoxAttr = new SfxItemSet( pExcGlob->pD->GetAttrPool(),
                                RES_FRMATR_BEGIN, RES_FRMATR_END - 1 );
    pD = ppData[ nIndex ];
    ppTxtAttr[ nIndex ] = pTxtAttr;
    ppBoxAttr[ nIndex ] = pBoxAttr;

//	DBG_ASSERT( pD->Parent() < nCount,
//		"+XF_Buffer::CreatePattern(): Parent-XF nicht im Puffer!" );
//	if( pD->Parent() < nCount )
//		pP = ppData[ pD->Parent() ];
//	else
//		pP = pDefaultData;

    DBG_ASSERT( pD->IsCellXF(),
        "+XF_Buffer::CreatePattern(): Und was soll ich mit 'nem Style?" );
    if( !pD->IsCellXF() )
        {
        pTxtAttr = pDefTxtAttr;	// Defaults
        pBoxAttr = pDefBoxAttr;
        return;
        }

    // Font ---------------------------------------------------------
    const ExcFont *pFont;

//	if( pD->HasFont() )
        pFont = &pExcGlob->pFontBuff->GetFont( pD->Font() );
//	else
//		pFont = &pExcGlob->pFontBuff->GetFont( pP->Font() );

    pTxtAttr->Put( *pFont->pFont );
    pTxtAttr->Put( *pFont->pHeight );
    pTxtAttr->Put( *pExcGlob->pColorBuff->GetColor( pFont->nColor ) );

    if( pFont->nWeight )
        {// bold
        SvxWeightItem aWeightItem( WEIGHT_BOLD );
        register UINT16 nWeight = pFont->nWeight;
        if( !nWeight )
            aWeightItem = WEIGHT_DONTKNOW;
        else if( nWeight < 150 )
            aWeightItem = WEIGHT_THIN;
        else if( nWeight < 250 )
            aWeightItem = WEIGHT_ULTRALIGHT;
        else if( nWeight < 325 )
            aWeightItem = WEIGHT_LIGHT;
        else if( nWeight < 375 )
            aWeightItem = WEIGHT_SEMILIGHT;
        else if( nWeight < 450 )
            aWeightItem = WEIGHT_NORMAL;
        else if( nWeight < 550 )
            aWeightItem = WEIGHT_MEDIUM;
        else if( nWeight < 650 )
            aWeightItem = WEIGHT_SEMIBOLD;
        else if( nWeight < 750 )
            aWeightItem = WEIGHT_BOLD;
        else if( nWeight < 850 )
            aWeightItem = WEIGHT_ULTRABOLD;
        else
            aWeightItem = WEIGHT_BLACK;
        pTxtAttr->Put( aWeightItem );
        }

    if( pFont->bItalic )
        {// italic
        SvxPostureItem aAttr( ITALIC_NORMAL );
        pTxtAttr->Put( aAttr );
        }

    switch( pFont->eUnderline )
        {
        case EXCUNDER_Single:
            {
            SvxUnderlineItem aAttr( UNDERLINE_SINGLE );
            pTxtAttr->Put( aAttr );
            }
            break;
        case EXCUNDER_Double:
            {
            SvxUnderlineItem aAttr( UNDERLINE_DOUBLE );
            pTxtAttr->Put( aAttr );
            }
            break;
        case EXCUNDER_SingleAccount:
            {
            SvxUnderlineItem aAttr( UNDERLINE_DOTTED );
            pTxtAttr->Put( aAttr );
            }
            break;
        case EXCUNDER_DoubleAccount:
            {
            SvxUnderlineItem aAttr( UNDERLINE_DOTTED );
            pTxtAttr->Put( aAttr );
            }
            break;
        }

/* FOR FUTURE USE
    switch( pFont->eScript )
        {
        case EXCSCR_Super:
            break;
        case EXCSCR_Sub:
            break;
        }
*/
    if( pFont->bStrikeout )
        {// strikeout
        SvxCrossedOutItem aAttr( STRIKEOUT_SINGLE );
        pTxtAttr->Put( aAttr );
        }

    if( pFont->bOutline )
        {// outline
        SvxContourItem aAttr( TRUE );
        pTxtAttr->Put( aAttr );
        }

    if( pFont->bShadow )
        {// shadow
        SvxShadowedItem aAttr( TRUE );
        pTxtAttr->Put( aAttr );
        }

    // Bordereinstellungen ------------------------------------------
    const CellBorder *pBorder;

//	if( pD->HasBorder() )
        pBorder = pD->Border();
//	else
//		pBorder = pP->Border();

    if( pBorder )
        {
        SvxBoxItem aBox;
        SvxBorderLine *pTop, *pLeft, *pBottom, *pRight;

        ExcToSwBorderLine( pBorder->nTopLine, pBorder->nTopColor, pTop );
        ExcToSwBorderLine( pBorder->nLeftLine, pBorder->nLeftColor, pLeft );
        ExcToSwBorderLine( pBorder->nBottomLine, pBorder->nBottomColor,
            pBottom );
        ExcToSwBorderLine( pBorder->nRightLine, pBorder->nRightColor, pRight );

        aBox.SetLine( pTop, BOX_LINE_TOP );
        aBox.SetLine( pLeft, BOX_LINE_LEFT );
        aBox.SetLine( pBottom, BOX_LINE_BOTTOM );
        aBox.SetLine( pRight, BOX_LINE_RIGHT );
        aBox.SetDistance( MIN_BORDER_DIST );

        pBoxAttr->Put( aBox );
        delete pTop;
        delete pLeft;
        delete pBottom;
        delete pRight;
        }

    // Hintergrund- / Fill-Einstellungen ----------------------------
    const CellFill *pFill;

//	if( pD->HasFill() )
        pFill = pD->Fill();
//	else
//		pFill = pP->Fill();

    if( pFill )
    {
        const static BYTE eStyle[ 19 ] = {					// Exc-#
            SW_SV_BRUSH_NULL, 	SW_SV_BRUSH_SOLID, 	SW_SV_BRUSH_50, 	SW_SV_BRUSH_75,					// 00-03
            SW_SV_BRUSH_25, 	SW_SV_BRUSH_INVALID,SW_SV_BRUSH_INVALID,SW_SV_BRUSH_INVALID,	// 04-07
            SW_SV_BRUSH_INVALID,SW_SV_BRUSH_50, 	SW_SV_BRUSH_75, 	SW_SV_BRUSH_INVALID,					// 08-0B
            SW_SV_BRUSH_INVALID,SW_SV_BRUSH_INVALID,SW_SV_BRUSH_INVALID,SW_SV_BRUSH_INVALID,	// 0C-0F
            SW_SV_BRUSH_50, 	SW_SV_BRUSH_25, 	SW_SV_BRUSH_25 };													// 10-12

        // !kein Hintergrund -> set nothing!
        if( pFill->nPattern != 0 )
        {
            Color aBack(
                ( *pExcGlob->pColorBuff->GetColor( pFill->nBackColor ) ).GetValue() );

            if( pExcGlob->pColorBuff->Auto() )	// Auto-Color fuer Background?
                aBack = COL_WHITE;

                Color aColor((*pExcGlob->pColorBuff->GetColor( pFill->nForeColor ) ).GetValue());

                if( pFill->nPattern < 19 )
                    aColor = ConvertBrushStyle(aColor, aBack, eStyle[ pFill->nPattern ] );

                pBoxAttr->Put( SvxBrushItem( aColor ) );
            }
        }

    // Ausrichtungsattribute ----------------------------------------
    ExcHorizAlign eHorizAlign;

//	if( pD->HasHorizAlign() )
        eHorizAlign = pD->HorizAlign();
//	else
//		eHorizAlign = pP->HorizAlign();

    SvxAdjust eAdjust = SVX_ADJUST_LEFT;

    switch( eHorizAlign )
        {
//		case EHA_Parent:
//		case EHA_General:
//			eAdjust = SVX_ADJUST_LEFT;
//			break;
//		case EHA_Left:
//			eAdjust = SVX_ADJUST_LEFT;
//			break;
        case EHA_Center:
            eAdjust = SVX_ADJUST_CENTER;
            break;
        case EHA_Right:
            eAdjust = SVX_ADJUST_RIGHT;
            break;
//		case EHA_Fill:
//			eAdjust = SVX_ADJUST_LEFT;
//			break;
        case EHA_Justify:
            eAdjust = SVX_ADJUST_BLOCK;
            break;
//		case EHA_CentAcrSel:
//			eAdjust = SVX_ADJUST_LEFT;
//			break;
        }
    pTxtAttr->Put( SvxAdjustItem( eAdjust ) );
    }

// ---------------------------------------------- XF_Buffer::SetItemSets -
void XF_Buffer::SetItemSets( UINT16 nCol, UINT16 nStartRow, UINT16 nEndRow,
                            UINT16 nXF )
    {
    const SfxItemSet *pTxtAttr, *pBoxAttr;

//	DBG_ASSERT( nXF < nCount ,
//		"+XF_Buffer::SetItemSets(): das ist zuviel des Guten!" );
    if( nXF >= nCount )
        {
        pTxtAttr = pDefTxtAttr;			// nicht im Puffer
        pBoxAttr = pDefBoxAttr;
        }
    else
        {
        if( ppTxtAttr[ nXF ] == NULL )
            CreateItemSets( nXF );		// erste Nutzung

        pTxtAttr = ppTxtAttr[ nXF ];
        pBoxAttr = ppBoxAttr[ nXF ];
        }

    // Ersatz fuer ApplyPatternAreaTab()
    SwTableBox* pSttBox = pExcGlob->pTable->GetTabLines()[ nStartRow ]->
                            GetTabBoxes()[ nCol ];

    // BoxAttributierung
    SwTableBoxFmt* pBoxFmt = (SwTableBoxFmt*)pSttBox->ClaimFrmFmt();
    pBoxFmt->SetAttr( *pBoxAttr );

    SwTableBox* pEndBox;
    if( nStartRow != nEndRow )
    {
        for( UINT16 nRow = nStartRow ; nRow <= nEndRow ; nRow++ )
        {
            pEndBox = pExcGlob->pTable->GetTabLines()[ nRow ]->
                                    GetTabBoxes()[ nCol ];
            pEndBox->ChgFrmFmt( pBoxFmt );
        }
    }
    else
        pEndBox = pSttBox;

    // TextAttributierung:
    pExcGlob->pPam->GetPoint()->nNode = *pSttBox->GetSttNd();
    pExcGlob->pPam->Move( fnMoveForward, fnGoCntnt );
    pExcGlob->pPam->SetMark();
    pExcGlob->pPam->GetPoint()->nNode = *pEndBox->GetSttNd()->EndOfSectionNode();
    pExcGlob->pPam->Move( fnMoveBackward, fnGoCntnt );

    pExcGlob->pD->Insert( *pExcGlob->pPam, *pTxtAttr );

    }

// --------------------------------------------- XF_Buffer::GetNumFormat -
sal_uInt32 XF_Buffer::GetNumFormat( UINT16 nIndex )
    {
//	DBG_ASSERT( nIndex < nCount ,
//		"+XF_Buffer::GetNumFormat(): das ist zuviel des Guten!" );
    if( nIndex < nCount )
        {// im Puffer
        XF_Data *pD = ppData[ nIndex ];

        if( pD->HasValueFormat() )
            // eigenes Format
            return pD->ValueFormat();
        else
            {// Format vom Parent
            if( pD->Parent() < nCount )
                {
                XF_Data *pP = ppData[ pD->Parent() ];
                if( pP->HasValueFormat() )
                    return pP->ValueFormat();
                }
            }
        }

    return pExcGlob->nStandard;	// nicht im Puffer oder kein Value-Format
    }


// --------------------------------------------------- class ColorBuffer -
// Kopiert aus: fontbuff.cxx


const UINT16 nIndCorrect = 8;

//------------------------------------------- ColorBuffer::ColorBuffer() -

ColorBuffer::ColorBuffer( void  )
    {
    UINT16 nSize = 256;

    pArray = new SvxColorItem *[ nSize ];
    for( UINT16 nC = 0 ; nC < nSize ; nC++ ) pArray[ nC ] = NULL;

    nCount = 0;
    nMax = nSize;
    bAuto = FALSE;

    pDefault	= new SvxColorItem( Color( COL_BLACK ) );
    }

//------------------------------------------ ColorBuffer::~ColorBuffer() -

ColorBuffer::~ColorBuffer()
    {
    for( UINT16 nC = 0 ; nC < nCount ; nC++ )
        if( pArray[ nC ] ) delete pArray[ nC ];
    delete[] pArray;

    delete pDefault;
    }


#ifdef USED
//--------------------------------------------------- ColorBuffer::Reset -

void ColorBuffer::Reset( void )
    {
    for( UINT16 nC = 0 ; nC < nCount ; nC++ )
        if( pArray[ nC ] )
            {
            delete pArray[ nC ];
            pArray[ nC ] = NULL;
            }
    nCount = 0;
    }
#endif


//----------------------------------------- ColorBuffer::NewColor( ... ) -

BOOL ColorBuffer::NewColor( UINT16 nR, UINT16 nG, UINT16 nB )
    {
    if( nCount < nMax )
        {
        pArray[ nCount ] = new SvxColorItem( Color(
            nR, nG, nB
            ) );
        nCount++;
        return TRUE;
        }

    return FALSE;
    }

//---------------------------------------- ColorBuffer::GetColor5( ... ) -

#define DefColorSize 56

const SvxColorItem *ColorBuffer::GetColor( UINT16 nIndex )
    {
    if( nIndex >= nIndCorrect )
        nIndex -= nIndCorrect;

    if( nIndex < nMax )
        {
        bAuto = FALSE;
        if( pArray[ nIndex ] )
            return pArray[ nIndex ];	// schon Generiert
        if( nIndex >= nCount )
        {							// ausserhalb der Definierten
            if( nIndex < DefColorSize )
            {							// ... aber innerhalb der Defaults
                static BYTE __READONLY_DATA pDefArray[ DefColorSize ][ 3 ] = {
                    0x00, 0x00, 0x00,
                    0xff, 0xff, 0xff,
                    0xff, 0x00, 0x00,
                    0x00, 0xff, 0x00,
                    0x00, 0x00, 0xff,
                    0xff, 0xff, 0x00,
                    0xff, 0x00, 0xff,
                    0x00, 0xff, 0xff,
                    0x80, 0x00, 0x00,
                    0x00, 0x80, 0x00,
                    0x00, 0x00, 0x80,
                    0x80, 0x80, 0x00,
                    0x80, 0x00, 0x80,
                    0x00, 0x80, 0x80,
                    0xc0, 0xc0, 0xc0,
                    0x80, 0x80, 0x80,
                    0x80, 0x80, 0xff,
                    0x80, 0x20, 0x60,
                    0xff, 0xff, 0xc0,
                    0xa0, 0xe0, 0xe0,
                    0x60, 0x00, 0x80,
                    0xff, 0x80, 0x80,
                    0x00, 0x80, 0xc0,
                    0xc0, 0xc0, 0xff,
                    0x00, 0x00, 0x80,
                    0xff, 0x00, 0xff,
                    0xff, 0xff, 0x00,
                    0x00, 0xff, 0xff,
                    0x80, 0x00, 0x80,
                    0x80, 0x00, 0x00,
                    0x00, 0x80, 0x80,
                    0x00, 0x00, 0xff,
                    0x00, 0xcf, 0xff,
                    0x69, 0xff, 0xff,
                    0xe0, 0xff, 0xe0,
                    0xff, 0xff, 0x80,
                    0xa6, 0xca, 0xf0,
                    0xdd, 0x9c, 0xb3,
                    0xb3, 0x8f, 0xee,
                    0xe3, 0xe3, 0xe3,
                    0x2a, 0x6f, 0xf9,
                    0x3f, 0xb8, 0xcd,
                    0x48, 0x84, 0x36,
                    0x95, 0x8c, 0x41,
                    0x8e, 0x5e, 0x42,
                    0xa0, 0x62, 0x7a,
                    0x62, 0x4f, 0xac,
                    0x96, 0x96, 0x96,
                    0x1d, 0x2f, 0xbe,
                    0x28, 0x66, 0x76,
                    0x00, 0x45, 0x00,
                    0x45, 0x3e, 0x01,
                    0x6a, 0x28, 0x13,
                    0x85, 0x39, 0x6a,
                    0x4a, 0x32, 0x85,
                    0x42, 0x42, 0x42
                    };

                pArray[ nIndex ] = new SvxColorItem( Color(
                    pDefArray[ nIndex ][ 0 ],		// R
                    pDefArray[ nIndex ][ 1 ],		// G
                    pDefArray[ nIndex ][ 2 ]		// B
                    ));
                }
            else
                {							// ... und ausserhalb der Defaults
                return pDefault;
                }
            }
        return pArray[ nIndex ];
        }
    else
        {								// ueber Array-Kapazitaet bzw. Auto
        bAuto = TRUE;
        return pDefault;
        }
    }

// ---------------------------------------------------- class FontBuffer -

// ---------------------------------------------- FontBuffer::FontBuffer -
FontBuffer::FontBuffer( UINT16 nNewMax )
    {// default: 256 Fonts
    DBG_ASSERT( nNewMax, "-FontBuffer::FontBuffer(): Groesse 0!" );

    nMax = nNewMax;
    nCount = 0;

    aDefaultFont.pFont = new SvxFontItem;

    aDefaultFont.pHeight = new SvxFontHeightItem;

    aDefaultFont.bItalic = aDefaultFont.bStrikeout = aDefaultFont.bOutline =
        aDefaultFont.bShadow = FALSE;
    aDefaultFont.eUnderline = EXCUNDER_None;
    aDefaultFont.nWeight = 400;	// NORMAL
    aDefaultFont.nColor = 0xFFFF;

    ppFonts = new ExcFont *[ nNewMax ];

    // alle NULL
    for( UINT16 nC = 0 ; nC < nMax ; nC++ )
        ppFonts[ nC ] = NULL;

    // ...ausser No. 4
    ppFonts[ 4 ] = &aDefaultFont;
    }

// --------------------------------------------- FontBuffer::~FontBuffer -
FontBuffer::~FontBuffer()
    {
    for( UINT16 nC = 0 ; nC < nCount ; nC++ )
        if( nC != 4  )
            {
            delete ppFonts[ nC ]->pFont;
            delete ppFonts[ nC ]->pHeight;
            }

    delete[] ppFonts;

    delete aDefaultFont.pFont;
    delete aDefaultFont.pHeight;
    }

//-------------------------------------------------- FontBuffer::NewFont -
void FontBuffer::NewFont( UINT16 nHeight, BYTE nAttr0, UINT16 nIndexCol,
    const String &rName )
    {
    // fuer Biff2-4
    BYTE	nUnderline;
    UINT16	nBoldness;

    if( nAttr0 &0x04 )
        // underline
        nUnderline = ( BYTE ) EXCUNDER_Single;
    else
        nUnderline = ( BYTE ) EXCUNDER_None;

    if( nAttr0 & 0x01 )
        // bold
        //rExcFont.bWeight = TRUE;
        nBoldness = 700;		// BOLD
    else
        //rExcFont.bWeight = FALSE;
        nBoldness = 400;		// NORMAL

    NewFont(
        nHeight, nAttr0, nUnderline, nIndexCol, nBoldness, 0x00, 0x01, rName );
        // -> nFamily = DONTKNOW, nCharSet = DONTKNOW
    }

// ------------------------------------------------- FontBuffer::NewFont -
void FontBuffer::NewFont( UINT16 nHeight, BYTE nAttr0, BYTE nUnderline,
    UINT16 nIndexCol, UINT16 nBoldness, BYTE nFamily, BYTE nCharSet,
    const String &rName )
    {
    // fuer Biff5

    DBG_ASSERT( nCount < nMax,
        "+FontBuffer::NewFont(): Puffer fuer Fonts voll!" );
    if( nCount >= nMax ) return;

    if( nCount == 4 ) nCount++;	// 4 darf nicht vorkommen

    if( !ppFonts[ nCount ] )
        ppFonts[ nCount ] = new ExcFont;	// bei Bedarf neu anlegen


    ExcFont &rExcFont = *ppFonts[ nCount ];

    // Umwandlung Windows-Font in SV-Font
    CharSet		eCharSet;
    switch ( nCharSet )
        {
        case 0:							// ANSI_CHARSET
            eCharSet = RTL_TEXTENCODING_MS_1252;
            break;
        case 255:						// OEM_CHARSET
            eCharSet = RTL_TEXTENCODING_IBM_850;
            break;
        case 2:							// SYMBOL_CHARSET
            eCharSet = RTL_TEXTENCODING_SYMBOL;
            break;
        default:
            eCharSet = RTL_TEXTENCODING_DONTKNOW;
        }

    FontFamily	eFamily;
    // !ACHTUNG!: anders als in Windows-Doku scheint der Font im unteren
    // Nible des Bytes zu stehen -> Pitch unbekannt!
    switch( nFamily & 0x0F )	// ...eben nicht!
        {
        case 0x01:						// FF_ROMAN
            eFamily = FAMILY_ROMAN;
            break;
        case 0x02:						// FF_SWISS
            eFamily = FAMILY_SWISS;
            break;
        case 0x03:						// FF_MODERN
            eFamily = FAMILY_MODERN;
            break;
        case 0x04:						// FF_SCRIPT
            eFamily = FAMILY_SCRIPT;
            break;
        case 0x05:						// FF_DECORATIVE
            eFamily = FAMILY_DECORATIVE;
            break;
        default:
            eFamily = FAMILY_DONTKNOW;
        }

    rExcFont.pFont = new SvxFontItem( eFamily, rName,
        aEmptyStr, PITCH_DONTKNOW, eCharSet );

    DBG_ASSERT( nHeight <  32767,
        "+FontList::NewFont(): Height > 32767 - Pech..." );

    rExcFont.pHeight = new SvxFontHeightItem( ( UINT32 ) nHeight );

    rExcFont.nColor = nIndexCol;

    // Aufdroeseln der Fontattribute
    rExcFont.nWeight = nBoldness;

    if( nAttr0 & 0x02 )
        // italic
        rExcFont.bItalic = TRUE;
    else
        rExcFont.bItalic = FALSE;

    rExcFont.eUnderline = ( ExcUnderline ) nUnderline;

    if( nAttr0 &0x08 )
        // strikeout
        rExcFont.bStrikeout = TRUE;
    else
        rExcFont.bStrikeout = FALSE;

    if( nAttr0 &0x10 )
        // outline
        rExcFont.bOutline = TRUE;
    else
        rExcFont.bOutline = FALSE;

    if( nAttr0 &0x20 )
        // shadow
        rExcFont.bShadow = TRUE;
    else
        rExcFont.bShadow = FALSE;

    nCount++;
    //...und schon fertig...
    }

// ------------------------------------------------- FontBuffer::GetFont -
const ExcFont &FontBuffer::GetFont( UINT16 nIndex )
    {
    // Index 4 ist per Definition nicht vorhanden!
    DBG_ASSERT( nIndex != 4,
        "+FontBuffer::GetFont(): Nr. 4 gibt's nicht!");

    DBG_ASSERT( nIndex < nCount ,
        "+FontBuffer::GetPattern(): Hab'n mer nich' und krieg'n mer nich'" );
    if( nIndex >= nCount ) return aDefaultFont;

    return *ppFonts[ nIndex ];
    }


#ifdef USED
// --------------------------------------------------- FontBuffer::Reset -
void FontBuffer::Reset( void )
    {
    for( UINT16 nC = 0 ; nC < nCount ; nC++ )
        {
        if( nC != 4 )
            {
            delete ppFonts[ nC ]->pFont;
            delete ppFonts[ nC ]->pHeight;
            ppFonts[ nC ]->pFont = NULL;
            ppFonts[ nC ]->pHeight = NULL;
            }
        }

    nCount = 0;
    }
#endif


#ifdef USED
// ---------------------------------------------------- class ExcTabelle -

// -------------------------------------------- ExcTabelle::ExcTabelle() -
ExcTabelle::ExcTabelle()
    {
    nStrPos = 0xFFFFFFFF;
    nCS = nRS = 1;
    nCE = nRE = 0;
    }

// ------------------------------------- ExcTabelle::ExcTabelle( UINT32 ) -
ExcTabelle::ExcTabelle( UINT32 nNewStrPos )
    {
    nStrPos = nNewStrPos;
    nCS = nRS = 1;
    nCE = nRE = 0;
    }

// ----------------------------- ExcTabelle::ExcTabelle( const String& ) -
ExcTabelle::ExcTabelle( const String &rName )
    {
    aName = rName;
    nCS = nRS = 1;
    nCE = nRE = 0;
    }

// ------------ ExcTabelle::ExcTabelle( UINT16, UINT16, UINT16, UINT16 ) -
ExcTabelle::ExcTabelle( UINT16 nNewCS, UINT16 nNewCE, UINT16 nNewRS,
    UINT16 nNewRE )
    {
    nCS = nNewCS;
    nRS = nNewRS;
    nCE = nNewCE;
    nRE = nNewRE;
    }

// ------------------------------------------- ExcTabelle::~ExcTabelle() -
ExcTabelle::~ExcTabelle()
    {
    }


// --------------------------------------------------- class ExcDokument -

// ------------------------------------------ ExcDokument::ExcDokument() -
ExcDokument::ExcDokument()
    {
    ppTabellen = new ExcTabelle *[ MAX_TABS_EXCDOK ];

    for( UINT16 nC = 0 ; nC < MAX_TABS_EXCDOK ; nC++ )
        ppTabellen[ nC ] = NULL;

    nAktTabIndex = 0;
    nAktGetNext = -1;	// -> GetNext enzspricht GetFirst
    }

// ----------------------------------------- ExcDokument::~ExcDokument() -
ExcDokument::~ExcDokument()
    {
    for( UINT16 nC = 0 ; nC < MAX_TABS_EXCDOK ; nC++ )
        if( ppTabellen[ nC ] ) delete ppTabellen[ nC ];
    delete[] ppTabellen;
    }


// ----------------------------------------- ExcDokument::GetLastTabNr() -
UINT16 ExcDokument::GetAnzTabs( void ) const
    {
    if( ppTabellen[ nAktTabIndex ] )
        return nAktTabIndex + 1;
    else
        return nAktTabIndex;
    }

// ----------------------------------- ExcDokument::operator[]( UINT16 ) -
ExcTabelle *ExcDokument::operator[]( UINT16 nIndex )
    {
    if( nIndex < MAX_TABS_EXCDOK )
        return ppTabellen[ nIndex ];
    else
        return NULL;
    }

// ------------------------- ExcDokument::NewTab( const String&, UINT32 ) -
void ExcDokument::NewTab( const String &rName, UINT32 nStrPos )
    {
    if( ppTabellen[ nAktTabIndex ] )
        nAktTabIndex++;

    if( nAktTabIndex < MAX_TABS_EXCDOK )
        {
        ExcTabelle *pAkt = ppTabellen[ nAktTabIndex ] = new ExcTabelle;

        pAkt->aName = rName;
        pAkt->nStrPos = nStrPos;
        }
    }

// --------------------------------------- ExcDokument::Set( UINT16,...) -
void ExcDokument::Set( UINT16 nCS, UINT16 nCE, UINT16 nRS,
    UINT16 nRE, UINT16 nIndex )
    {
    if( nIndex == 0xFFFF )
        nIndex = nAktTabIndex;

    if( nIndex >= MAX_TABS_EXCDOK ) return;

    ExcTabelle *pAkt = ppTabellen[ nIndex ];

    if( !pAkt )
        pAkt = ppTabellen[ nIndex ] = new ExcTabelle( nCS, nCE, nRS, nRE );
    }


// ----------------------------------- ExcDokument::Set( UINT32, UINT16 ) -
void ExcDokument::Set( UINT32 nStrPos, UINT16 nIndex )
    {
    if( nIndex == 0xFFFF )
        nIndex = nAktTabIndex;

    if( nIndex >= MAX_TABS_EXCDOK ) return;

    ExcTabelle *pAkt = ppTabellen[ nIndex ];

    if( !pAkt )
        pAkt = ppTabellen[ nIndex ] = new ExcTabelle( nStrPos );
    }

// ----------------------------------- ExcDokument::Set( UINT32, UINT16 ) -
void ExcDokument::Set( const String &rName, UINT16 nIndex )
    {
    if( nIndex == 0xFFFF )
        nIndex = nAktTabIndex;

    if( nIndex >= MAX_TABS_EXCDOK ) return;

    ExcTabelle *pAkt = ppTabellen[ nIndex ];

    if( !pAkt )
        pAkt = ppTabellen[ nIndex ] = new ExcTabelle( rName );
    }

// ------------------------------------------- ExcDokument::GetNextTab() -
ExcTabelle *ExcDokument::GetNextTab( void )
    {
    nAktGetNext++;
    DBG_ASSERT( nAktGetNext >= 0,
        "-ExcDokument::GetNextTab(): nAktGetNext totaler Murks!" );
    while( nAktGetNext < MAX_TABS_EXCDOK && !ppTabellen[ nAktGetNext ] )
        nAktGetNext++;	// solange, bis eine Tabelle gefunden

    if( nAktGetNext >= MAX_TABS_EXCDOK )
        return NULL;
    else
        return ppTabellen[ nAktGetNext ];
    }

// ------------------------------------------ ExcDokument::GetFirstTab() -
ExcTabelle *ExcDokument::GetFirstTab( void )
    {
    nAktGetNext = 0;
    return ppTabellen[ nAktGetNext ];
    }
#endif

// ----------------------------------------------- class ValueFormBuffer -

const sal_Char *ValueFormBuffer::pBuiltinFormats[] = {
    "General",								// 0
    "0",									// 1
    "0.00",									// 2
    "#,##0",								// 3
    "#,##0.00",								// 4
    "",										// 5	-> Defined 0
    "",										// 6	-> Defined 1
    "",										// 7	-> Defined 2
    "",										// 8	-> Defined 3
    "0%",									// 9
    "0.00%",								// 10
    "0.00E+00",								// 11
    "# ?/?",								// 12
    "# ?""?/??",							// 13
    "DD.MM.YYYY",							// 14
    "DD. MMM YY",							// 15
    "DD. MMM",								// 16
    "MMM YY",								// 17
    "h:mm AM/PM",							// 18
    "h:mm:ss AM/PM",						// 19
    "hh:mm",								// 20
    "hh:mm:ss",								// 21
    "DD.MM.YYYY hh:mm",						// 22
    "",									// 23	?
    "",									// 24	?
    "",									// 25	?
    "",									// 26	?
    "",									// 27	?
    "",									// 28	?
    "",									// 29	?
    "",									// 30	?
    "",									// 31	?
    "",									// 32	?
    "",									// 33	?
    "",									// 34	?
    "",									// 35	?
    "",									// 36	?
    "#,##0 _$;-#,##0 _$",					// 37
    "#,##0 _$;[Red]-#,##0 _$",				// 38
    "#,##0.00 _$;-#,##0.00 _$",				// 39
    "#,##0.00 _$;[Red]-#,##0.00 _$",		// 40
    "",										// 41	-> Defined 5
    "",										// 42	-> Defined 4
    "",										// 43	-> Defined 7
    "",										// 44	-> Defined 6
    "mm:ss",								// 45
    "[h]:mm:ss",							// 46
    "mm:ss,0",								// 47
    "##0.0E+0",								// 48
    "@"										// 49
    };

const UINT16 ValueFormBuffer::nAnzBuiltin = 50;
const UINT16 ValueFormBuffer::nNewFormats = 164;

//------------------------------------------------------------------------
ValueFormBuffer::ValueFormBuffer( const UINT16 nSize )
    {
    nMax = nSize;

    pHandles = new UINT32[ nSize ];

    // ACHTUNG: nCount wird in Init() sinnvoll gesetzt!

    _NewValueFormat = &ValueFormBuffer::__NewValueFormat;
    _GetValueFormat = &ValueFormBuffer::__GetValueFormat;
    }

//------------------------------------------------------------------------
ValueFormBuffer::~ValueFormBuffer()
    {
    delete[] pHandles;
    }

//------------------------------------------------------------------------
void ValueFormBuffer::__NewValueFormat( String &rFormString )
    {
    Init();
    ( this->*_NewValueFormat )( rFormString );
    }

//------------------------------------------------------------------------
void ValueFormBuffer::_NewValueFormatX( String &rFormString )
    {
    DBG_ASSERT( nCount < nMax,
        "+ValueFormBuffer::NewValueFormat(): Value-Form-Puffer voll!" );
    if( nCount >= nMax ) return;

    xub_StrLen nDummy;
    INT16 nTyp = NUMBERFORMAT_DEFINED;

    if( rFormString == pExcGlob->aStandard )
        pHandles[ nCount ] = pExcGlob->pNumFormatter->GetStandardIndex
            ( pExcGlob->eDefLanguage );
    else
        pExcGlob->pNumFormatter->PutandConvertEntry( rFormString, nDummy, nTyp,
            pHandles[ nCount ], LANGUAGE_ENGLISH_US, pExcGlob->eDefLanguage );

    nCount++;
    }

//------------------------------------------------------------------------
void ValueFormBuffer::_NewValueFormat5( String &rFormString )
    {
    DBG_ASSERT( nCount < nMax,
        "+ValueFormBuffer::NewValueFormat(): Value-Form-Puffer voll!" );
    if( nCount >= nMax ) return;

    xub_StrLen nDummy;
    INT16 nTyp = NUMBERFORMAT_DEFINED;

    pExcGlob->pNumFormatter->PutandConvertEntry( rFormString, nDummy, nTyp,
        pHandles[ nCount ], LANGUAGE_ENGLISH_US, pExcGlob->eDefLanguage );

    // jetzt wird's merkwuerdig!
    switch( nCount )
        {
        case 5:		nCount++;				break;	// 0 |
        case 6:		nCount++;				break;	// 1 |--|
        case 7:		nCount++;				break;	// 2    |--|
        case 8:		nCount = 42;			break;	// 3       |--|
        case 41:	nCount = 44;			break;	// 5          |  |--|
        case 42:	nCount = 41;			break;	// 4          |--|  |
        case 43:	nCount = nNewFormats;	break;	// 7                |  |--|
        case 44:	nCount = 43;			break;	// 6                |--|  |
        default:	nCount++;						//                        |-...
        }
    }

//------------------------------------------------------------------------
UINT32 ValueFormBuffer::__GetValueFormat( UINT16 nExcIndex )
    {
    Init();
    return ( this->*_GetValueFormat )( nExcIndex );
    }

//------------------------------------------------------------------------
UINT32 ValueFormBuffer::_GetValueFormatX5( UINT16 nExcIndex )
    {
    DBG_ASSERT( nExcIndex < nCount,
        "+ValueFormBuffer::GetValueFormat(): Da kann ich auch nichts fuer!" );
    if( nExcIndex >= nCount )
        return nDefaultHandle;

    return pHandles[ nExcIndex ];
    }


#ifdef USED
//------------------------------------------------------------------------
void ValueFormBuffer::Reset( void )
    {
    nCount = 0;
    _NewValueFormat = &ValueFormBuffer::__NewValueFormat;
    _GetValueFormat = &ValueFormBuffer::__GetValueFormat;
    }
#endif


//------------------------------------------------------------------------
void ValueFormBuffer::Init( void )
    {
    if( pExcGlob->eHauptDateiTyp == ERT_Biff5 )
        {// Excel5 kriegt 'ne Sonderwurst!
        UINT16 nC;
        xub_StrLen nDummy;
        INT16 nTyp = NUMBERFORMAT_DEFINED;
        pExcGlob->pNumFormatter->PutEntry( aEmptyStr, nDummy, nTyp,
            nDefaultHandle,	pExcGlob->eDefLanguage );

        // Builtin belegen
        pHandles[ 0 ] = pExcGlob->pNumFormatter->
            GetStandardIndex( pExcGlob->eDefLanguage );

        for( nC = 1 ; nC < nAnzBuiltin ; nC++ )
            {
                String aString( String::CreateFromAscii(pBuiltinFormats[ nC ]) );
            pExcGlob->pNumFormatter->PutandConvertEntry(
                aString, nDummy, nTyp,
                pHandles[ nC ], LANGUAGE_ENGLISH_US, pExcGlob->eDefLanguage );
            }

        // Rest defaulten
        for( nC = nAnzBuiltin ; nC < nMax ; nC++ )
            pHandles[ nC ] = nDefaultHandle;

        _NewValueFormat = &ValueFormBuffer::_NewValueFormat5;
        nCount = 5;			// !!!ACHTUNG!!! erster aus Format-Record!
        }
    else
        {
        _NewValueFormat = &ValueFormBuffer::_NewValueFormatX;
        nCount = 0;
        }

    _GetValueFormat = &ValueFormBuffer::_GetValueFormatX5;
    }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
