/*************************************************************************
 *
 *  $RCSfile: pivot.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-08 09:01:30 $
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

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#include <vector>

class SubTotal;

#ifndef SC_COLLECT_HXX
#include "collect.hxx"
#endif

#define PIVOT_DATA_FIELD        (MAXCOLCOUNT)
#define PIVOT_FUNC_REF          (MAXCOLCOUNT)

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDREFERENCE_HPP_
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDSORTINFO_HPP_
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDLAYOUTINFO_HPP_
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDAUTOSHOWINFO_HPP_
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#endif

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
struct LabelData;

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
    LabelData**     ppLabelArr;
    SCSIZE          nLabels;
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
    void            ClearLabelData  ();
    void            ClearPivotArrays();
    void            SetLabelData    ( LabelData**   ppLabArr,
                                      SCSIZE        nLab );
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

typedef PivotField          PivotFieldArr[PIVOT_MAXFIELD];
typedef PivotField          PivotPageFieldArr[PIVOT_MAXPAGEFIELD];

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
            USHORT      GetIndex(TypedStrData* pData) const;
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

    PivotStrCollection* pColList[PIVOT_MAXFIELD];       // pro Zeile alle Eintraege
    PivotStrCollection* pRowList[PIVOT_MAXFIELD];
    PivotStrCollection* pDataList;                      // Shortcut auf Col/RowList mit Daten

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

//------------------------------------------------------------------------

struct LabelData
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
    ::com::sun::star::sheet::DataPilotFieldSortInfo     maSortInfo;     /// Sorting info.
    ::com::sun::star::sheet::DataPilotFieldLayoutInfo   maLayoutInfo;   /// Layout info.
    ::com::sun::star::sheet::DataPilotFieldAutoShowInfo maShowInfo;     /// AutoShow info.

    explicit            LabelData( const String& rName, short nCol, bool bIsValue );
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

typedef LabelData ScDPLabelData;
typedef std::vector< ScDPLabelData > ScDPLabelDataVec;
typedef std::vector< String > ScDPNameVec;

// ============================================================================

#endif
