/*************************************************************************
 *
 *  $RCSfile: pivot.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:49 $
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
/*
    WICHTIG:
    Folgende Reihenfolge beim Aufbau der Pivot-Tabelle unbedingt einzuhalten:

    pPivot->SetColFields(aColArr, aColCount)
    pPivot->SetRowFields(aRowArr, aRowCount)
    pPivot->SetDataFields(aDataArr, aDataCount)
    if (pPivot->CreateData())
    {
        pPivotDrawData();
        pPivotReleaseData();
    }

    ausserdem ist sicherzustellen, dass entweder das ColArr oder das RowArr
    einen PivotDataField Eintrag enthalten

*/


#ifndef SC_PIVOT_HXX
#define SC_PIVOT_HXX

#include <global.hxx>

class SubTotal;

#ifndef SC_COLLECT_HXX
#include "collect.hxx"
#endif

#define PIVOT_DATA_FIELD        (MAXCOL + 1)
#define PIVOT_FUNC_REF          (MAXCOL + 1)

#define PIVOT_STYLE_INNER       0
#define PIVOT_STYLE_RESULT      1
#define PIVOT_STYLE_CATEGORY    2
#define PIVOT_STYLE_TITLE       3
#define PIVOT_STYLE_FIELDNAME   4
#define PIVOT_STYLE_TOP         5

class SvStream;
class ScDocument;
class ScUserListData;
class ScMultipleReadHeader;
class ScMultipleWriteHeader;
class ScProgress;

struct PivotColRef
{
    USHORT nDataIndex;
    USHORT nRecCount;
    USHORT nFuncMask;
    USHORT nIndex;

    PivotColRef()
    {
        nDataIndex = nRecCount = nIndex = 0;
        nFuncMask = PIVOT_FUNC_NONE;
    }
};

typedef PivotField          PivotFieldArr[PIVOT_MAXFIELD];

class PivotStrCollection : public StrCollection
{
    ScUserListData* pUserData;
public:
    PivotStrCollection(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE) :
                        StrCollection   ( nLim, nDel, bDup ),
                        pUserData       (NULL) { }
    PivotStrCollection(const PivotStrCollection& rPivotStrCollection) :
                        StrCollection   ( rPivotStrCollection ),
                        pUserData       ( rPivotStrCollection.pUserData) {}

    virtual DataObject* Clone() const;
    virtual short       Compare(DataObject* pKey1, DataObject* pKey2) const;

    TypedStrData*       operator[]( const USHORT nIndex) const
                            { return (TypedStrData*)At(nIndex); }
            void        SetUserData(ScUserListData* pData)
                            { pUserData = pData; }
    const   String&     GetString(USHORT nIndex)
                            { return ((TypedStrData*)At(nIndex))->GetString(); }
            short       GetIndex(TypedStrData* pData) const;
};

class ScPivot : public DataObject
{
    ScDocument*         pDoc;
    ScQueryParam        aQuery;
    BOOL                bHasHeader;

    BOOL                bIgnoreEmpty;           // Flags aus Dialog
    BOOL                bDetectCat;
    BOOL                bMakeTotalCol;
    BOOL                bMakeTotalRow;

    String              aName;
    String              aTag;
    USHORT              nColNameCount;
    String*             pColNames;              // Array

    USHORT              nSrcCol1;
    USHORT              nSrcRow1;
    USHORT              nSrcCol2;
    USHORT              nSrcRow2;
    USHORT              nSrcTab;

    USHORT              nDestCol1;
    USHORT              nDestRow1;
    USHORT              nDestCol2;
    USHORT              nDestRow2;
    USHORT              nDestTab;

    USHORT              nDataStartCol;
    USHORT              nDataStartRow;

    short               nColCount;
    short               nRowCount;
    short               nDataCount;

    PivotFieldArr       aColArr;
    PivotFieldArr       aRowArr;
    PivotFieldArr       aDataArr;

    PivotStrCollection* pColList[PIVOT_MAXFIELD];       // pro Zeile alle Eintraege
    PivotStrCollection* pRowList[PIVOT_MAXFIELD];
    PivotStrCollection* pDataList;                      // Shortcut auf Col/RowList mit Daten

    SubTotal**          ppDataArr;
    short               nDataColCount;
    short               nDataRowCount;
    short               nRowIndex;
    short               nColIndex;
    short               nDataIndex;
    short               nRecCount;

    PivotColRef*        pColRef;

    BOOL                bValidArea;
    BOOL                bDataAtCol;

public:
    ScPivot(ScDocument* pDocument);
    ScPivot(const ScPivot& rPivot);
    ~ScPivot();

    virtual DataObject* Clone() const;

    ScPivot*    CreateNew() const;

    BOOL        Load(SvStream& rStream, ScMultipleReadHeader& rHdr );
    BOOL        Store(SvStream& rStream, ScMultipleWriteHeader& rHdr ) const;

    void        SetQuery(const ScQueryParam& rQuery);
    void        GetQuery(ScQueryParam& rQuery) const;

    void        SetHeader(BOOL bHeader);
    BOOL        GetHeader() const;
    void        SetIgnoreEmpty(BOOL bIgnore);
    BOOL        GetIgnoreEmpty() const;
    void        SetDetectCat(BOOL bDetect);
    BOOL        GetDetectCat() const;
    void        SetMakeTotalCol(BOOL bSet);
    BOOL        GetMakeTotalCol() const;
    void        SetMakeTotalRow(BOOL bSet);
    BOOL        GetMakeTotalRow() const;

    void            SetName(const String& rNew);
    const String&   GetName() const;
    void            SetTag(const String& rNew);
    const String&   GetTag() const;

    void        SetSrcArea(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, USHORT nTab);
    void        GetSrcArea(USHORT& rCol1, USHORT& rRow1, USHORT& rCol2, USHORT& rRow2, USHORT& rTab) const;
    ScRange     GetSrcArea() const;

    void        SetDestPos(USHORT nCol, USHORT nRow, USHORT nTab);
    void        GetDestArea(USHORT& rCol1, USHORT& rRow1, USHORT& rCol2, USHORT& rRow2, USHORT& rTab) const;
    ScRange     GetDestArea() const;

    void        SetColFields(const PivotField* pFieldArr, short nCount);
    void        GetColFields(PivotField* pFieldArr, short& rCount) const;
    short       GetColFieldCount() const    { return nColCount; }

    void        SetRowFields(const PivotField* pFieldArr, short nCount);
    void        GetRowFields(PivotField* pFieldArr, short& rCount) const;
    short       GetRowFieldCount() const    { return nRowCount; }

    void        SetDataFields(const PivotField* pFieldArr, short nCount);
    void        GetDataFields(PivotField* pFieldArr, short& rCount) const;

    void        GetParam( ScPivotParam& rParam, ScQueryParam& rQuery, ScArea& rSrcArea ) const;
    void        SetParam( const ScPivotParam& rParam, const ScQueryParam& rQuery,
                            const ScArea& rSrcArea );

    BOOL        CreateData(BOOL bKeepDest = FALSE);
    void        DrawData();
    void        ReleaseData();

    BOOL        IsPivotAtCursor(USHORT nCol, USHORT nRow, USHORT nTab) const;
    BOOL        IsFilterAtCursor(USHORT nCol, USHORT nRow, USHORT nTab) const;
    BOOL        GetColFieldAtCursor(USHORT nCol, USHORT nRow, USHORT nTab, USHORT& rField) const;
    BOOL        GetRowFieldAtCursor(USHORT nCol, USHORT nRow, USHORT nTab, USHORT& rField) const;

                //  Referenz-Anpassung:

    void        MoveSrcArea( USHORT nNewCol, USHORT nNewRow, USHORT nNewTab );
    void        MoveDestArea( USHORT nNewCol, USHORT nNewRow, USHORT nNewTab );
    void        ExtendSrcArea( USHORT nNewEndCol, USHORT nNewEndRow );

private:
    BOOL    CreateFields();
    void    CreateFieldData();
    void    CalcArea();

    void    SetDataLine(USHORT nCol, USHORT nRow, USHORT nTab, USHORT nRIndex);
    void    SetFuncLine(USHORT nCol, USHORT nRow, USHORT nTab, USHORT nFunc, USHORT nIndex, USHORT nStartRIndex, USHORT nEndRIndex);
    void    ColToTable(short nField, USHORT& nRow, ScProgress& rProgress);
    void    RowToTable(short nField, USHORT& nCol);
    void    SetFrame(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, USHORT nWidth = 20);
    void    SetFrameHor(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2);
    void    SetFrameVer(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2);
    void    SetFontBold(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2);
    void    SetJustifyLeft(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2);
    void    SetJustifyRight(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2);
    void    SetStyle(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, USHORT nId);
    void    SetButton(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2);
    void    SetValue(USHORT nCol, USHORT nRow, const SubTotal& rTotal, USHORT nFunc);
    USHORT  GetCategoryRow( USHORT nCol, USHORT nRow );
};

//------------------------------------------------------------------------
class ScPivotCollection : public Collection
{

private:
    ScDocument* pDoc;
public:
    ScPivotCollection(USHORT nLim = 4, USHORT nDel = 4, ScDocument* pDocument = NULL) :
                    Collection  ( nLim, nDel),
                    pDoc        ( pDocument ) {}
    ScPivotCollection(const ScPivotCollection& rScPivotCollection) :
                    Collection  ( rScPivotCollection ),
                    pDoc        ( rScPivotCollection.pDoc ) {}

    virtual DataObject* Clone() const;
            ScPivot*    operator[]( const USHORT nIndex) const {return (ScPivot*)At(nIndex);}
            ScPivot*    GetPivotAtCursor(USHORT nCol, USHORT nRow, USHORT nTab) const;

    BOOL    Load( SvStream& rStream );
    BOOL    Store( SvStream& rStream ) const;

    void    UpdateReference(UpdateRefMode eUpdateRefMode,
                                USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                short nDx, short nDy, short nDz );
    void    UpdateGrow( const ScRange& rArea, USHORT nGrowX, USHORT nGrowY );

    BOOL    operator==(const ScPivotCollection& rCmp) const;

    String  CreateNewName( USHORT nMin = 1 ) const;
};

//------------------------------------------------------------------------
struct LabelData
{
    String* pStrColName;
    short   nCol;
    BOOL    bIsValue; // Summe oder Anzahl im Data-Feld
    USHORT  nFuncMask;


        LabelData( const String&    rColName,
                   short            nColumn,
                   BOOL             bVal,
                   USHORT           nMask = PIVOT_FUNC_NONE )
            :   nCol        (nColumn),
                bIsValue    (bVal),
                nFuncMask   (nMask)
            { pStrColName = new String( rColName ); }

        LabelData( const LabelData& rCpy )
            :   nCol        (rCpy.nCol),
                bIsValue    (rCpy.bIsValue),
                nFuncMask   (rCpy.nFuncMask)
            { pStrColName = new String( *(rCpy.pStrColName) ); }

        ~LabelData()
            { delete pStrColName; }

    LabelData& operator=( const LabelData& r )
        {
            nCol        = r.nCol;
            bIsValue    = r.bIsValue;
            nFuncMask   = r.nFuncMask;
            pStrColName = new String( *(r.pStrColName) );

            return *this;
        }
};


#endif
