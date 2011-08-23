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

#ifndef _SCH_MEMCHRT_HXX
#define _SCH_MEMCHRT_HXX

#include <bf_sfx2/sfxdefs.hxx>
#include <bf_sfx2/module.hxx>
#include <tools/color.hxx>
#include <tools/link.hxx>
#include <tools/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <bf_sch/cellranges.hxx>

#include <com/sun/star/chart/ChartSeriesAddress.hpp>
class SvStream;

namespace binfilter {

class SfxItemSet;
class SvNumberFormatter;
class ChartModel;

#define SCH_COPY_HACK

enum ChartDataId
{
    CHDATAID_NONE,
    CHDATAID_MEMCHART,
    CHDATAID_DYNCHART,
    CHDATAID_MEMCHART_PLUS
};

// This struct contains information about selection in the chart, so that
// other apps (esp. calc) know what is currently selected

struct ChartSelectionInfo
{
    ChartSelectionInfo() : nRow(0), nCol(0), fValue(0.0), nValue(0), nSelection(0) {}
    ChartSelectionInfo(long sel) : nRow(0), nCol(0), fValue(0.0), nValue(0), nSelection(sel) {}
    BOOL operator == (const ChartSelectionInfo& s) const
    {
        return nRow				== s.nRow
            && nCol				== s.nCol
            && nSelection		== s.nSelection
            && fValue			== s.fValue
            && nValue			== s.nValue
            && aSelectionColor	== s.aSelectionColor;
    }
    long	nRow;			 // Datenzeilen-Koordinate
    long	nCol;			 // Datenspaltenkoordinate
    double	fValue;			 // neuer Wert fuer die Zelle (nCol,nRow)
    long	nValue;			 // reserviert, evtl. fuer Numberformat und andere Informationen
    long	nSelection;      // s.u. , 31-Bit-Register (also besser Int32? -> return ist long)
    Color	aSelectionColor; // Farbe des Selektions-Rahmens
};

// Flags fuer nSelection UND den Rueckgabewert der CallBackFunktion
// long ChartSelectionCallback(struct ChartSelectionInfo *pInfo), die mittels
// SchMemChart::SetSelectionCallback(...) am Chart angemeldet wird

#define CHART_SEL_NONE			1	// bestehende Selektion aufheben (derzeit default, da im Chart keine Multiselektion besteht)
#define CHART_SEL_ALL			2	// den gesamten MemChart-Datenbereich selektieren
#define CHART_SEL_ROW			4	// die DatenReihe nRow selektieren
#define CHART_SEL_COL			8	// die DatenSpalte nCol Selektieren
#define CHART_SEL_POINT			16	// den Datenpunkt selektieren
#define CHART_SEL_SETCURSOR		32	// Cursor an die Stelle (nCol,nRow) setzen
#define CHART_SEL_CHANGEVALUE	64	// Wert in der Tabelle aendern (auf fValue)
#define CHART_SEL_COLOR			128	// Farbinformation für farbigen Rahmen um den Selektionsbereich herum ist gesetzt
#define CHART_SEL_QUERYSUPPORT  256
#define CHART_SEL_QUERYPOSSIBLE 512

/*
 In der Callback-routine soll wie folgt verfahren werden:
 Kann eine Aktion (z.B. Aenderung des Wertes) ausgefuehrt werden, so MUSS das
 entsprechende Flag im Rueckgabewert gesetzt werden, kann die Aktion nicht ausgefuehrt
 werden, so DARF das Flag NICHT gesetzt werden.

Prioritaet der Aktionen:

 Prio 0 : CHART_SEL_QUERYSUPPORT
 Prio 1 : CHART_SEL_NONE   ; wichtig fuer (spaetere) Multiselektion.
 Prio 2 : CHART_SEL_BORDERONLY ; Selektionsmodus, wird vorerst immer gesetzt sein
 Prio n : der Rest....

CHART_SEL_QUERYSUPPORT:
 Ist dieses Flag gesetzt, DARF die Aktion NICHT ausgefuehrt werden, sondern nur
 zurueckgegeben werden, ob es moeglich waere!

CHART_SEL_NONE:

  Aufheben der Selektion, bei Multiselektion ist dieses Flag NICHT gesetzt, die neue Selektion
  wird zusaetzlich angewand. (derzeit im Chart nicht implementiert, daher immer gesetzt)

*/

// ============================================================
// ============================================================
//
//              SchMemChart: class for chart data
//
// ============================================================
// ============================================================

#define TRANS_NONE	0
#define TRANS_COL	1
#define TRANS_ROW	2
#define TRANS_ERROR 3

class SchMemChart
{
private:
    long			nLastSelInfoReturn;
    ULONG           nRefCount;
    mutable String*	mpColNameBuffer;
    mutable String*	mpRowNameBuffer;

protected :
    long			nTranslated;
    short			nRowCnt;
    short			nColCnt;
    String			aMainTitle;
    String			aSubTitle;
    String			aXAxisTitle;
    String			aYAxisTitle;
    String			aZAxisTitle;
    short			eDataType;
    String          aSomeData1;
    String          aSomeData2;
    String          aSomeData3;
    String          aSomeData4;
    double          *pData;
    String          *pColText;
    String          *pRowText;
    ChartDataId     myID;


    // number formatter and format id
    SvNumberFormatter*  mpNumFormatter;
    sal_Int32 *pRowNumFmtId;
    sal_Int32 *pColNumFmtId;

    // translation table for row-/column reordering (internal)
    sal_Int32 *pRowTable;
    sal_Int32 *pColTable;
    ChartSelectionInfo aSelectionInfo;

    // is not copied in copy contrustor (?)
    Link aAppLink;
    BOOL bReadOnly;

    ::rtl::OUString maCategoriesRangeAddress;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::chart::ChartSeriesAddress > maSeriesAddresses;

    /// contains all information necessary to write the source data ranges
    SchChartRange maChartRange;

    ::rtl::OUStringBuffer getXMLStringForCellAddress( const SchCellAddress& rCell );
     void getSingleCellAddressFromXMLString( const ::rtl::OUString& rXMLString,
                                             sal_Int32 nStartPos, sal_Int32 nEndPos,
                                             SchSingleCell& rSingleCell );
     bool getCellAddressFromXMLString( const ::rtl::OUString& rXMLString,
                                      sal_Int32 nStartPos, sal_Int32 nEndPos,
                                      SchCellAddress& rOutCell,
                                      ::rtl::OUString& rOutTableName );
    bool getCellRangeAddressFromXMLString( const ::rtl::OUString& rXMLString,
                                           sal_Int32 nStartPos, sal_Int32 nEndPos,
                                           SchCellRangeAddress& rOutRange );

public:

    inline void SetReadOnly( BOOL bNewValue );
    BOOL IsReadOnly() { return bReadOnly; }

    SchMemChart(ChartDataId myID);
    SchMemChart(short nCols, short nRows);
    SchMemChart(const SchMemChart& rMemChart);
    ~SchMemChart()
    {
        delete[] pRowText;
        delete[] pColText;
        delete[] pData;

        if( pRowNumFmtId )
        delete[] pRowNumFmtId;
        if( pColNumFmtId )
        delete[] pColNumFmtId;
        if( pRowTable )
        delete[] pRowTable;
        if( pColTable )
        delete[] pColTable;
        delete[] mpRowNameBuffer;
        delete[] mpColNameBuffer;
    }

    // initialize number format, -1 means unset
    void InitNumFmt();

    // transfer chart selection to calc or writer
    const ChartSelectionInfo& GetSelectionInfo() const { return aSelectionInfo; }
    // methods for translation of data
    inline void ResetTranslation(sal_Int32 *pTable,long nCnt);
#if OSL_DEBUG_LEVEL > 0
    BOOL VerifyTranslation();
#endif
    long GetTranslation() const { return nTranslated; }
    inline void UpdateTranslation(sal_Int32 *pTable,long nCnt);
    // for acces always use these Get-methods !
    double GetTransData(long nCol,long nRow);
    double GetTransDataInPercent(long nCol ,long nRow,BOOL bRowData) const;
    const String& GetTransColText(long nCol) const;
    const String& GetTransRowText(long nRow) const;

    const sal_Int32 *GetRowTranslation() const { return pRowTable; }
    const sal_Int32 *GetColTranslation() const { return pColTable; }

#ifdef SCH_COPY_HACK
    const sal_Int32* GetRowNumFmtTable() const { return pRowNumFmtId; }
    const sal_Int32* GetColNumFmtTable() const { return pColNumFmtId; }

    // ********************
    // BM: Fix for #68864#
    // Use these four methods with care! The arrays MUST have the correct size!
    // ********************
    void SetRowTranslation( const sal_Int32* pTransTable )
    {
        if( !pRowTable ) return;
        for( sal_Int32 i=0; i<nRowCnt; i++ )
            pRowTable[ i ] = pTransTable[ i ];
    }
    void SetColTranslation( const sal_Int32* pTransTable )
    {
        if( !pColTable ) return;
        for( sal_Int32 i=0; i<nColCnt; i++ )
            pColTable[ i ] = pTransTable[ i ];
    }
    void SetRowNumFmtTable( const sal_Int32* pNumFmtTable )
    {
        if( !pRowNumFmtId ) return;
        for( sal_Int32 i=0; i<nRowCnt; i++ )
            pRowNumFmtId[ i ] = pNumFmtTable[ i ];
    }
    void SetColNumFmtTable( const sal_Int32* pNumFmtTable )
    {
        if( !pColNumFmtId ) return;
        for( sal_Int32 i=0; i<nColCnt; i++ )
            pColNumFmtId[ i ] = pNumFmtTable[ i ];
    }
    // this is only valid if the corresponding translation table was set correctly!
    void SetTranslation( long nTrans ) { nTranslated = nTrans; }
    // ********************
#endif



    // number format

    void SetNumberFormatter(SvNumberFormatter* pNumFormatter) { mpNumFormatter = pNumFormatter; }
    SvNumberFormatter*  GetNumberFormatter() const { return mpNumFormatter;	}

    void SetNumFormatIdRow( const long nRow, const long nFmtId )	{ if( pRowNumFmtId && nRow < nRowCnt ) pRowNumFmtId[ nRow ] = nFmtId; }
    void SetNumFormatIdCol( const long nCol, const long nFmtId )	{ if( pColNumFmtId && nCol < nColCnt ) pColNumFmtId[ nCol ] = nFmtId; }
    long GetNumFormatIdRow( const long nRow ) const					{ return ( pRowNumFmtId && nRow < nRowCnt )? pRowNumFmtId[ nRow ]: -1; }
    long GetNumFormatIdCol( const long nCol ) const					{ return ( pColNumFmtId && nCol < nColCnt )? pColNumFmtId[ nCol ]: -1; }
    long GetTransNumFormatIdRow( const long nRow ) const;
    long GetTransNumFormatIdCol( const long nCol ) const;

    void SetSelectionHdl(const Link& rLink) { aAppLink=rLink; }

    void SetNonNumericData(const SchMemChart &rMemChart);

    void SetDataType(short eType) { eDataType = eType; }
    short GetDataType() const { return eDataType; }

    short GetColCount() const { return nColCnt;	}
    short GetRowCount() const {	return nRowCnt;	}

    const String& GetMainTitle() const	{ return aMainTitle; }
    const String& GetSubTitle() const	{ return aSubTitle; }

    void SetMainTitle(const String& rText) { aMainTitle = rText; }
    void SetSubTitle(const String& rText) { aSubTitle = rText; }

    const String& GetXAxisTitle() const	{ return aXAxisTitle; }
    const String& GetYAxisTitle() const	{ return aYAxisTitle; }
    const String& GetZAxisTitle() const	{ return aZAxisTitle; }

    void SetXAxisTitle(const String& rText) { aXAxisTitle = rText; }
    void SetYAxisTitle(const String& rText) { aYAxisTitle = rText; }
    void SetZAxisTitle(const String& rText) { aZAxisTitle = rText; }

    friend SvStream& operator << (SvStream& rOut, const SchMemChart& rMemChart);
    friend SvStream& operator >> (SvStream& rIn, SchMemChart& rMemChart);

    const String& GetColText(short nCol) const { return pColText[nCol]; }
    const String& GetRowText(short nRow) const { return pRowText[nRow]; }

    ChartDataId GetId() const { return myID; }
    double GetData(short nCol, short nRow) const { return pData[nCol * nRowCnt + nRow]; }
    double GetDataInPercent(const short nCol , const short nRow, const BOOL bRowData) const;

    inline void InsertCols(short nAtCol, short nCount);
    inline void RemoveCols(short nAtCol, short nCount);
    inline void InsertRows(short nAtRow, short nCount);
    inline void RemoveRows(short nAtRow, short nCount);
    inline void SwapCols(int nAtCol1, int nAtCol2);
    inline void SwapRows(int nAtRow1, int nAtRow2);

    void SetData(short nCol, short nRow, const double& rVal)
    { pData[nCol * nRowCnt + nRow] = rVal; }

    String& SomeData1() { return aSomeData1; }
    String& SomeData2() { return aSomeData2; }
    String& SomeData3() { return aSomeData3; }
    String& SomeData4() { return aSomeData4; }

    const String& SomeData1() const { return aSomeData1; }
    const String& SomeData2() const { return aSomeData2; }
    const String& SomeData3() const { return aSomeData3; }
    const String& SomeData4() const { return aSomeData4; }

    void SetColText(short nCol, const String& rText) { pColText[nCol] = rText; }
    void SetRowText(short nRow, const String& rText) { pRowText[nRow] = rText; }





    friend class ChartModel;

    void IncreaseRefCount() { nRefCount++; }
    BOOL DecreaseRefCount()
    {
        DBG_ASSERT( nRefCount, "SchMemChart::DecreaseRefCount: illegal decrement of RefCount");
        if(nRefCount) nRefCount--;
        return (nRefCount==0);	// true iff object can be deleted
    }

    // get a string for default (column|row) labels
    // the indexes start at 0, but the text starts with "(Column|Row) 1"
    String GetDefaultColumnText( sal_Int32 nCol ) const;
    String GetDefaultRowText( sal_Int32 nRow ) const;

    // ====================

    /// @deprecated
    void SetCategoriesRangeAddress( const ::rtl::OUString& rCellRangeAddress )
        { maCategoriesRangeAddress = rCellRangeAddress; }

    /// @deprecated
    void SetSeriesAddresses( const ::com::sun::star::uno::Sequence< ::com::sun::star::chart::ChartSeriesAddress >& rCellRangeAddresses )
        { maSeriesAddresses = rCellRangeAddresses; }

    /// @deprecated
    const ::rtl::OUString&
        GetCategoriesRangeAddress()
        { return maCategoriesRangeAddress; }

    /// @deprecated
    const ::com::sun::star::uno::Sequence< ::com::sun::star::chart::ChartSeriesAddress >&
        GetSeriesAddresses()
        { return maSeriesAddresses; }

    // --------------------

    /// get table ranges for entire chart
    const SchChartRange& GetChartRange() const
        { return maChartRange; }
    /// set table ranges for entire chart
    void SetChartRange( const SchChartRange& rRange )
        { maChartRange = rRange; }

    /** convert SomeData string(s) to SchChartRange and vice versa for Writer
        @return <TRUE/>, iff the conversion was successfully done.
    */
    bool ConvertChartRangeForWriter( BOOL bOldToNew = TRUE );

    /** convert SomeData string(s) to SchChartRange and vice versa for Calc
        @return <TRUE/>, iff the conversion was successfully done.
     */
    bool ConvertChartRangeForCalc( BOOL bOldToNew = FALSE );

    ::rtl::OUString getXMLStringForChartRange();
    void getChartRangeForXMLString( const ::rtl::OUString& rXMLString );

    /** create a string of form "1 2 3 4" containing a list of table numbers
        in SchChartRange maChartRange to create SomeData for calc
     */
    ::rtl::OUString createTableNumberList();
    /** prerequisite: maChartRange must be set and have the correct dimension
        for all table numbers to fit in
    */
    void parseTableNumberList( const ::rtl::OUString& rList );
};

// ==================== Inline Implementations ====================

inline void SchMemChart::ResetTranslation(sal_Int32 *pTable,long nCnt)
{
    long i;
    if(pTable)
        for(i=0;i<nCnt;i++)
            pTable[i]=i;

    if(pTable==pRowTable && nTranslated==TRANS_ROW)
        nTranslated=TRANS_NONE;
    if(pTable==pColTable && nTranslated==TRANS_COL)
        nTranslated=TRANS_NONE;
}

inline void SchMemChart::UpdateTranslation(sal_Int32 *pTable,long nCnt)
{
    if( (pTable==pRowTable && nTranslated==TRANS_ROW)
      ||(pTable==pColTable && nTranslated==TRANS_COL))
    {
        long i;
        sal_Int32 nMax=0;
        for(i=0;i<nCnt;i++)
            nMax=Max(nMax,pTable[i]);

        if(nMax>=nCnt)//something added
        {
            long nStart=-1,nEnd=-1;
            for(i=0;i<nCnt;i++)
            {
                if(pTable[i]==-1)//markierung gefunden
                {
                    if(nStart!=-1)
                        nEnd=i;
                    else
                        nStart=i;
                }
            }

            long nDiff=nEnd-nStart;//soviele sind neu

            DBG_ASSERT( (nDiff>0) && ((nEnd-nStart) == (nCnt-nMax)),"MemChart::Something went wrong!");

            if( (nDiff<=0) || ((nEnd-nStart)!=(nCnt-nMax)))
                return;

            long nNew = nStart;

            for(i=0;i<nCnt;i++)
            {
                if(pTable[i]>nStart)
                {
                    pTable[i]+=nDiff;   //nach hinten verschieben
                }
                else if (pTable[i]==-1)
                {
                    pTable[i]=nNew;     //"einfuegen"
                    nNew++;
                }
            }
        }
        else //something removed
        {
            ResetTranslation(pTable,nCnt); //keine schoene Loesung....(ToDo:), Aufwand recht hoch...
        }
    }
    else
    {
        ResetTranslation(pTable,nCnt); //Normale Reihenfolge, da nicht die Sortierte Tabelll geupdated wird
    }
}

inline void SchMemChart::InsertCols(short nAtCol, short nCount)
{
    double* pOldData = pData;

    short nNewColCnt = nColCnt + nCount;

    pData			= new double[nNewColCnt * nRowCnt];

    short i, j, nOld, nMax;

    for (i = 0, nOld = 0; i < nAtCol; i++, nOld++)
    {
        for (j = 0; j < nRowCnt; j++)
        {
            pData[i * nRowCnt + j] = pOldData[nOld * nRowCnt + j];
        }
    }

    nMax = nAtCol + nCount;

    for (; i < nMax; i++)
    {
        for (j = 0; j < nRowCnt; j++)
        {
            pData[i * nRowCnt + j] = 0.0;
        }
    }

    for (; i < nNewColCnt; i++, nOld++)
    {
        for (j = 0; j < nRowCnt; j++)
        {
            pData[i * nRowCnt + j] = pOldData[nOld * nRowCnt + j];
        }
    }
    delete[] pOldData;

    String *pOldColText     = pColText;
    sal_Int32 *pOldColNumFmtId = pColNumFmtId;
    sal_Int32 *pOldColTable    = pColTable;

    pColNumFmtId	= new sal_Int32 [nNewColCnt];
    pColTable		= new sal_Int32 [nNewColCnt];
    pColText		= new String[nNewColCnt];

    long nC=nNewColCnt;
    while(nC--)
    {
        pColTable[nC]    = -1; //init
        pColNumFmtId[nC] = -1;
    }

    for (i = 0, nOld = 0;; i++, nOld++)
    {
        if (i == nAtCol)
            i += nCount;
        if (i >= nNewColCnt)
            break;

        pColText[i]     = pOldColText[nOld];
        pColTable[i]    = pOldColTable[nOld];
        pColNumFmtId[i] = pOldColNumFmtId[nOld];
    }

    delete[] pOldColNumFmtId;
    delete[] pOldColTable;
    delete[] pOldColText;

    nColCnt = nNewColCnt;

    UpdateTranslation(pColTable,nColCnt);
}

inline void SchMemChart::RemoveCols(short nAtCol, short nCount)
{
    DBG_ASSERT( nAtCol < nColCnt, "column index overflow");

    if (nAtCol + nCount > nColCnt)
        nCount = nColCnt - nAtCol;

    short nNewColCnt = nColCnt - nCount;

    double* pOldData = pData;
    pData = new double[nNewColCnt * nRowCnt];

    String* pOldColText		= pColText;
    sal_Int32* pOldColNumFmtId	= pColNumFmtId;
    sal_Int32* pOldColTable	= pColTable;

    pColText		= new String[nNewColCnt];
    pColNumFmtId	= new sal_Int32 [nNewColCnt];
    pColTable		= new sal_Int32 [nNewColCnt];

    short i, j, nOld;

    for (i = 0, nOld = 0;; i++, nOld++)
    {
        if (nOld == nAtCol)
            nOld += nCount;
        if (nOld >= nColCnt)
            break;

        for (j = 0; j < nRowCnt; j++)
            pData[i * nRowCnt + j] = pOldData[nOld * nRowCnt + j];

        pColText[i]		= pOldColText[nOld];
        pColTable[i]    = pOldColTable[nOld];
        pColNumFmtId[i] = pOldColNumFmtId[nOld];
    }

    delete[] pOldData;
    delete[] pOldColText;
    delete[] pOldColTable;
    delete[] pOldColNumFmtId;

    nColCnt = nNewColCnt;

    UpdateTranslation(pColTable,nColCnt);
}

inline void SchMemChart::InsertRows(short nAtRow, short nCount)
{
    double* pOldData = pData;

    short nNewRowCnt = nRowCnt + nCount;

    pData = new double[nColCnt * nNewRowCnt];

    short i, j, nOld, nMax, nGapEnd;

    for (i = 0; i < nColCnt; i++)
    {
        for (j = 0, nOld= 0; j < nAtRow; j++, nOld++)
        {
            pData[i * nNewRowCnt + j] = pOldData[i * nRowCnt + nOld];
        }
    }

    nMax = nAtRow + nCount;

    for (i = 0; i < nColCnt; i++)
    {
        for (j = nAtRow; j < nMax; j++)
        {
            pData[i * nNewRowCnt + j] = 0.0;
        }
    }

    nGapEnd = nMax;

    for (i = 0; i < nColCnt; i++)
    {
        for (j = nGapEnd, nOld = nAtRow; j < nNewRowCnt; j++, nOld++)
        {
            pData[i * nNewRowCnt + j] = pOldData[i * nRowCnt + nOld];
        }
    }
    delete[] pOldData;

    String *pOldRowText     =pRowText;
    sal_Int32 *pOldRowNumFmtId =pRowNumFmtId;
    sal_Int32 *pOldRowTable    =pRowTable;

    pRowNumFmtId	= new sal_Int32 [nNewRowCnt];
    pRowTable		= new sal_Int32 [nNewRowCnt];
    pRowText        = new String[nNewRowCnt];

    long nC=nNewRowCnt;
    while(nC--) //init
    {
        pRowNumFmtId[nC]	= -1;
        pRowTable[nC]	= -1;
    }

    for (i = 0, nOld = 0;; i++, nOld++)
    {
        if (i == nAtRow)
            i += nCount;
        if (i >= nNewRowCnt)
            break;

        pRowNumFmtId[i]	= pOldRowNumFmtId[nOld];
        pRowTable[i]	= pOldRowTable[nOld];
        pRowText[i]     = pOldRowText[nOld];
    }

    delete[] pOldRowText;
    delete[] pOldRowTable;
    delete[] pOldRowNumFmtId;

    nRowCnt = nNewRowCnt;

    UpdateTranslation(pRowTable,nRowCnt);
}

inline void SchMemChart::RemoveRows(short nAtRow, short nCount)
{
    DBG_ASSERT( nAtRow < nRowCnt, "row  index overflow" );

    if (nAtRow + nCount > nRowCnt)
        nCount = nRowCnt - nAtRow;

    short nNewRowCnt = nRowCnt - nCount;

    double* pOldData = pData;
    pData = new double[nColCnt * nNewRowCnt];

    short i, j, nOld;

    for (i = 0; i < nColCnt; i++)
        for (j = 0, nOld = 0;; j++, nOld++)
        {
            if (nOld == nAtRow)
                nOld += nCount;
            if (nOld >= nRowCnt)
                break;

            pData[i * nNewRowCnt + j] = pOldData[i * nRowCnt + nOld];
        }

    delete[] pOldData;

    String* pOldRowText		= pRowText;
    sal_Int32*   pOldRowNumFmtId	= pRowNumFmtId;
    sal_Int32*   pOldRowTable	= pRowTable;

    pRowText		= new String[nNewRowCnt];
    pRowNumFmtId	= new sal_Int32 [nNewRowCnt];
    pRowTable		= new sal_Int32 [nNewRowCnt];

    for (i = 0, nOld = 0;; i++, nOld++)
    {
        if (nOld == nAtRow)
            nOld += nCount;
        if (nOld >= nRowCnt)
            break;

        pRowText[i] = pOldRowText[nOld];
        pRowTable[i] = pOldRowTable[nOld];
        pRowNumFmtId[i] = pOldRowNumFmtId[nOld];
    }

    delete[] pOldRowText;
    delete[] pOldRowTable;
    delete[] pOldRowNumFmtId;

    nRowCnt = nNewRowCnt;

    UpdateTranslation(pRowTable,nRowCnt);
}

inline void SchMemChart::SwapCols(int nAtCol1, int nAtCol2)
{
    // Wenn hier Aenderungen Stattfinden, mu?auch im Logbuch
    // angepasst werden
    if (nAtCol1 > nAtCol2)
    {
        long nTemp = nAtCol1;

        nAtCol1 = nAtCol2;
        nAtCol2 = nTemp;
    }

    if (nAtCol1 >= nColCnt - 1) nAtCol1 = nColCnt - 2;
    if (nAtCol2 >= nColCnt) nAtCol2 = nColCnt - 1;

    nAtCol1 = Max(nAtCol1, 0);
    nAtCol2 = Max(nAtCol2, 0);
    //Insbesondere bis hier,, da dieses Handling autokorrigierend ist

    double *pSrc  = pData + nAtCol1 * nRowCnt;
    double *pDest = pData + nAtCol2 * nRowCnt;

    for (long nSwaps = 0;
              nSwaps < nRowCnt;
              nSwaps ++)
    {
        double fSwap = *pSrc;

        *pSrc  = *pDest;
        *pDest = fSwap;

        pSrc ++;
        pDest ++;
    }

    String aTemp = pColText [nAtCol1];
    pColText [nAtCol1] = pColText [nAtCol2];
    pColText [nAtCol2] = aTemp;

    long nTmp = pColTable [nAtCol1];
    pColTable [nAtCol1] = pColTable [nAtCol2];
    pColTable [nAtCol2] = nTmp;

    nTmp = pColNumFmtId[nAtCol1];
    pColNumFmtId [nAtCol1] = pColNumFmtId [nAtCol2];
    pColNumFmtId [nAtCol2] = nTmp;

    ResetTranslation(pColTable,nColCnt); //ToDo:???
}

inline void SchMemChart::SwapRows(int nAtRow1,int nAtRow2)
{

    // Wenn hier Aenderungen Stattfinden, mu?auch im Logbuch
    // angepasst werden
    if (nAtRow1 > nAtRow2)
    {
        long nTemp = nAtRow1;

        nAtRow1 = nAtRow2;
        nAtRow2 = nTemp;
    }

    if (nAtRow1 >= nRowCnt - 1) nAtRow1 = nRowCnt - 2;
    if (nAtRow2 >= nRowCnt) nAtRow2 = nRowCnt - 1;

    nAtRow1 = Max(nAtRow1, 0);
    nAtRow2 = Max(nAtRow2, 0);
    //Insbesondere bis hier,, da dieses Handling autokorrigierend ist

    double *pSrc  = pData + nAtRow1;
    double *pDest = pData + nAtRow2;

    for (long nSwaps = 0;
              nSwaps < nColCnt;
              nSwaps ++)
    {
        double fSwap = *pSrc;

        *pSrc  = *pDest;
        *pDest = fSwap;

        pSrc  += nRowCnt;
        pDest += nRowCnt;
    }

    String aTemp = pRowText [nAtRow1];
    pRowText [nAtRow1] = pRowText [nAtRow2];
    pRowText [nAtRow2] = aTemp;

    long nTmp = pRowTable [nAtRow1];
    pRowTable [nAtRow1] = pRowTable [nAtRow2];
    pRowTable [nAtRow2] = nTmp;

    nTmp = pRowNumFmtId[nAtRow1];
    pRowNumFmtId [nAtRow1] = pRowNumFmtId [nAtRow2];
    pRowNumFmtId [nAtRow2] = nTmp;

    ResetTranslation(pRowTable,nRowCnt);//ToDo:???
}

inline void SchMemChart::SetReadOnly( BOOL bNewValue )
{
    // do not set ReadOnly, if there is no external data set
    if( bNewValue && maChartRange.maRanges.size() == 0 )
        return;
    else
        bReadOnly = bNewValue;
}

} //namespace binfilter
#endif	// _SCH_MEMCHRT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
