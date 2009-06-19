/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pivot.hxx,v $
 * $Revision: 1.8.32.3 $
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

#include "global.hxx"
#include "address.hxx"

#include <vector>
#include <boost/shared_ptr.hpp>

class SubTotal;
#include "collect.hxx"

#define PIVOT_DATA_FIELD        (MAXCOLCOUNT)
#define PIVOT_FUNC_REF          (MAXCOLCOUNT)
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>

#if OLD_PIVOT_IMPLEMENTATION
#define PIVOT_STYLE_INNER       0
#define PIVOT_STYLE_RESULT      1
#define PIVOT_STYLE_CATEGORY    2
#define PIVOT_STYLE_TITLE       3
#define PIVOT_STYLE_FIELDNAME   4
#define PIVOT_STYLE_TOP         5
#endif

class SvStream;
class ScDocument;
class ScUserListData;
class ScMultipleReadHeader;
class ScMultipleWriteHeader;
class ScProgress;
struct ScDPLabelData;

typedef ::boost::shared_ptr<ScDPLabelData> ScDPLabelDataRef;

// -----------------------------------------------------------------------

struct PivotField
{
    SCsCOL               nCol;
    USHORT              nFuncMask;
    USHORT              nFuncCount;
    ::com::sun::star::sheet::DataPilotFieldReference maFieldRef;

    explicit            PivotField( SCsCOL nNewCol = 0, USHORT nNewFuncMask = PIVOT_FUNC_NONE );

    bool                operator==( const PivotField& r ) const;
};

// -----------------------------------------------------------------------

// implementation still in global2.cxx
struct ScPivotParam
{
    SCCOL           nCol;           // Cursor Position /
    SCROW           nRow;           // bzw. Anfang des Zielbereiches
    SCTAB           nTab;
    ::std::vector<ScDPLabelDataRef> maLabelArray;
    PivotField      aPageArr[PIVOT_MAXPAGEFIELD];
    PivotField      aColArr[PIVOT_MAXFIELD];
    PivotField      aRowArr[PIVOT_MAXFIELD];
    PivotField      aDataArr[PIVOT_MAXFIELD];
    SCSIZE          nPageCount;
    SCSIZE          nColCount;
    SCSIZE          nRowCount;
    SCSIZE          nDataCount;
    BOOL            bIgnoreEmptyRows;
    BOOL            bDetectCategories;
    BOOL            bMakeTotalCol;
    BOOL            bMakeTotalRow;

    ScPivotParam();
    ScPivotParam( const ScPivotParam& r );
    ~ScPivotParam();

    ScPivotParam&   operator=       ( const ScPivotParam& r );
    BOOL            operator==      ( const ScPivotParam& r ) const;
    void            Clear           ();
    void            ClearPivotArrays();
    void            SetLabelData    (const ::std::vector<ScDPLabelDataRef>& r);
    void            SetPivotArrays  ( const PivotField* pPageArr,
                                      const PivotField* pColArr,
                                      const PivotField* pRowArr,
                                      const PivotField* pDataArr,
                                      SCSIZE            nPageCnt,
                                      SCSIZE            nColCnt,
                                      SCSIZE            nRowCnt,
                                      SCSIZE            nDataCnt );
};

// -----------------------------------------------------------------------

#if OLD_PIVOT_IMPLEMENTATION
struct PivotColRef
{
    SCSIZE nDataIndex;
    SCSIZE nRecCount;
    USHORT nFuncMask;
    SCSIZE nIndex;

    PivotColRef()
    {
        nDataIndex = nRecCount = nIndex = 0;
        nFuncMask = PIVOT_FUNC_NONE;
    }
};
#endif

typedef PivotField          PivotFieldArr[PIVOT_MAXFIELD];
typedef PivotField          PivotPageFieldArr[PIVOT_MAXPAGEFIELD];

#if OLD_PIVOT_IMPLEMENTATION
class PivotScStrCollection : public ScStrCollection
{
    ScUserListData* pUserData;
public:
    PivotScStrCollection(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE) :
                        ScStrCollection ( nLim, nDel, bDup ),
                        pUserData       (NULL) { }
    PivotScStrCollection(const PivotScStrCollection& rPivotScStrCollection) :
                        ScStrCollection ( rPivotScStrCollection ),
                        pUserData       ( rPivotScStrCollection.pUserData) {}

    virtual ScDataObject*   Clone() const;
    virtual short       Compare(ScDataObject* pKey1, ScDataObject* pKey2) const;

    TypedStrData*       operator[]( const USHORT nIndex) const
                            { return (TypedStrData*)At(nIndex); }
            void        SetUserData(ScUserListData* pData)
                            { pUserData = pData; }
    const   String&     GetString(USHORT nIndex)
                            { return ((TypedStrData*)At(nIndex))->GetString(); }
            USHORT      GetIndex(TypedStrData* pData) const;
};

class ScPivot : public ScDataObject
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
    SCSIZE              nColNameCount;
    String*             pColNames;              // Array

    SCCOL               nSrcCol1;
    SCROW               nSrcRow1;
    SCCOL               nSrcCol2;
    SCROW               nSrcRow2;
    SCTAB               nSrcTab;

    SCCOL               nDestCol1;
    SCROW               nDestRow1;
    SCCOL               nDestCol2;
    SCROW               nDestRow2;
    SCTAB               nDestTab;

    SCCOL               nDataStartCol;
    SCROW               nDataStartRow;

    SCSIZE              nColCount;
    SCSIZE              nRowCount;
    SCSIZE              nDataCount;

    PivotFieldArr       aColArr;
    PivotFieldArr       aRowArr;
    PivotFieldArr       aDataArr;

    PivotScStrCollection*   pColList[PIVOT_MAXFIELD];       // pro Zeile alle Eintraege
    PivotScStrCollection*   pRowList[PIVOT_MAXFIELD];
    PivotScStrCollection*   pDataList;                      // Shortcut auf Col/RowList mit Daten

    SubTotal**          ppDataArr;
    SCSIZE              nDataColCount;
    SCSIZE              nDataRowCount;
    SCSIZE              nRowIndex;
    SCSIZE              nColIndex;
    SCSIZE              nDataIndex;
    SCSIZE              nRecCount;

    PivotColRef*        pColRef;

    BOOL                bValidArea;
    BOOL                bDataAtCol;

public:
    ScPivot(ScDocument* pDocument);
    ScPivot(const ScPivot& rPivot);
    ~ScPivot();

    virtual ScDataObject*   Clone() const;

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

    void        SetSrcArea(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, SCTAB nTab);
    void        GetSrcArea(SCCOL& rCol1, SCROW& rRow1, SCCOL& rCol2, SCROW& rRow2, SCTAB& rTab) const;
    ScRange     GetSrcArea() const;

    void        SetDestPos(SCCOL nCol, SCROW nRow, SCTAB nTab);
    void        GetDestArea(SCCOL& rCol1, SCROW& rRow1, SCCOL& rCol2, SCROW& rRow2, SCTAB& rTab) const;
    ScRange     GetDestArea() const;

    void        SetColFields(const PivotField* pFieldArr, SCSIZE nCount);
    void        GetColFields(PivotField* pFieldArr, SCSIZE& rCount) const;
    SCSIZE      GetColFieldCount() const    { return nColCount; }

    void        SetRowFields(const PivotField* pFieldArr, SCSIZE nCount);
    void        GetRowFields(PivotField* pFieldArr, SCSIZE& rCount) const;
    SCSIZE      GetRowFieldCount() const    { return nRowCount; }

    void        SetDataFields(const PivotField* pFieldArr, SCSIZE nCount);
    void        GetDataFields(PivotField* pFieldArr, SCSIZE& rCount) const;

    void        GetParam( ScPivotParam& rParam, ScQueryParam& rQuery, ScArea& rSrcArea ) const;
    void        SetParam( const ScPivotParam& rParam, const ScQueryParam& rQuery,
                            const ScArea& rSrcArea );

    BOOL        CreateData(BOOL bKeepDest = FALSE);
    void        DrawData();
    void        ReleaseData();

    BOOL        IsPivotAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab) const;
    BOOL        IsFilterAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab) const;
    BOOL        GetColFieldAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, SCCOL& rField) const;
    BOOL        GetRowFieldAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, SCCOL& rField) const;

                //  Referenz-Anpassung:

    void        MoveSrcArea( SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab );
    void        MoveDestArea( SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab );
    void        ExtendSrcArea( SCCOL nNewEndCol, SCROW nNewEndRow );

private:
    BOOL    CreateFields();
    void    CreateFieldData();
    void    CalcArea();

    void    SetDataLine(SCCOL nCol, SCROW nRow, SCTAB nTab, SCSIZE nRIndex);
    void    SetFuncLine(SCCOL nCol, SCROW nRow, SCTAB nTab, USHORT nFunc, SCSIZE nIndex, SCSIZE nStartRIndex, SCSIZE nEndRIndex);
    void    ColToTable(SCSIZE nField, SCROW& nRow, ScProgress& rProgress);
    void    RowToTable(SCSIZE nField, SCCOL& nCol);
    void    SetFrame(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, USHORT nWidth = 20);
    void    SetFrameHor(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
    void    SetFrameVer(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
    void    SetFontBold(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
    void    SetJustifyLeft(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
    void    SetJustifyRight(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
    void    SetStyle(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, USHORT nId);
    void    SetButton(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
    void    SetValue(SCCOL nCol, SCROW nRow, const SubTotal& rTotal, USHORT nFunc);
    SCROW   GetCategoryRow( SCCOL nCol, SCROW nRow );
};

//------------------------------------------------------------------------
class ScPivotCollection : public ScCollection
{

private:
    ScDocument* pDoc;
public:
    ScPivotCollection(USHORT nLim = 4, USHORT nDel = 4, ScDocument* pDocument = NULL) :
                    ScCollection    ( nLim, nDel),
                    pDoc        ( pDocument ) {}
    ScPivotCollection(const ScPivotCollection& rScPivotCollection) :
                    ScCollection    ( rScPivotCollection ),
                    pDoc        ( rScPivotCollection.pDoc ) {}

    virtual ScDataObject*   Clone() const;
            ScPivot*    operator[]( const USHORT nIndex) const {return (ScPivot*)At(nIndex);}
            ScPivot*    GetPivotAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab) const;

    BOOL    Load( SvStream& rStream );
    BOOL    Store( SvStream& rStream ) const;

    void    UpdateReference(UpdateRefMode eUpdateRefMode,
                                SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void    UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    BOOL    operator==(const ScPivotCollection& rCmp) const;

    String  CreateNewName( USHORT nMin = 1 ) const;
};
#endif

//------------------------------------------------------------------------

struct ScDPLabelData
{
    String              maName;         /// Visible name of the dimension.
    SCsCOL              mnCol;
    USHORT              mnFuncMask;     /// Page/Column/Row subtotal function.
    sal_Int32           mnUsedHier;     /// Used hierarchy.
    bool                mbShowAll;      /// true = Show all (also empty) results.
    bool                mbIsValue;      /// true = Sum or count in data field.

    ::com::sun::star::uno::Sequence< ::rtl::OUString >  maHiers;        /// Hierarchies.
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  maMembers;      /// Members.
    ::com::sun::star::uno::Sequence< sal_Bool >         maVisible;      /// Visibility of members.
    ::com::sun::star::uno::Sequence< sal_Bool >         maShowDet;      /// Show details of members.
    ::com::sun::star::sheet::DataPilotFieldSortInfo     maSortInfo;     /// Sorting info.
    ::com::sun::star::sheet::DataPilotFieldLayoutInfo   maLayoutInfo;   /// Layout info.
    ::com::sun::star::sheet::DataPilotFieldAutoShowInfo maShowInfo;     /// AutoShow info.

    explicit            ScDPLabelData( const String& rName, short nCol, bool bIsValue );
};

// ============================================================================

struct ScDPFuncData
{
    short               mnCol;
    USHORT              mnFuncMask;
    ::com::sun::star::sheet::DataPilotFieldReference maFieldRef;

    explicit            ScDPFuncData( short nNewCol, USHORT nNewFuncMask );
    explicit            ScDPFuncData( short nNewCol, USHORT nNewFuncMask,
                            const ::com::sun::star::sheet::DataPilotFieldReference& rFieldRef );
};

// ============================================================================

typedef std::vector< ScDPLabelData > ScDPLabelDataVec;
typedef std::vector< String > ScDPNameVec;

// ============================================================================

#endif
