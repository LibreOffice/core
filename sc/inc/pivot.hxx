/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
#include "dpglobal.hxx"

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

class SvStream;
class ScDocument;
class ScUserListData;
class ScProgress;

// ============================================================================

struct ScDPLabelData
{
    ::rtl::OUString     maName;         /// Original name of the dimension.
    ::rtl::OUString     maLayoutName;   /// Layout name (display name)
    SCCOL               mnCol;
    sal_uInt16          mnFuncMask;     /// Page/Column/Row subtotal function.
    sal_Int32           mnUsedHier;     /// Used hierarchy.
    sal_Int32           mnFlags;        /// Flags from the DataPilotSource dimension
    bool                mbShowAll;      /// true = Show all (also empty) results.
    bool                mbIsValue;      /// true = Sum or count in data field.

    struct Member
    {
        ::rtl::OUString maName;
        ::rtl::OUString maLayoutName;
        bool mbVisible;
        bool mbShowDetails;

        Member();

        /**
         * return the name that should be displayed in the dp dialogs i.e.
         * when the layout name is present, use it, or else use the original
         * name.
         */
        ::rtl::OUString SC_DLLPUBLIC getDisplayName() const;
    };
    ::std::vector<Member>                               maMembers;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  maHiers;        /// Hierarchies.
    ::com::sun::star::sheet::DataPilotFieldSortInfo     maSortInfo;     /// Sorting info.
    ::com::sun::star::sheet::DataPilotFieldLayoutInfo   maLayoutInfo;   /// Layout info.
    ::com::sun::star::sheet::DataPilotFieldAutoShowInfo maShowInfo;     /// AutoShow info.

    explicit            ScDPLabelData( const String& rName, SCCOL nCol, bool bIsValue );

    /**
     * return the name that should be displayed in the dp dialogs i.e. when
     * the layout name is present, use it, or else use the original name.
     */
    ::rtl::OUString SC_DLLPUBLIC getDisplayName() const;
};

typedef std::vector< ScDPLabelData > ScDPLabelDataVector;

// ============================================================================

struct ScPivotField
{
    SCCOL               nCol;
    sal_uInt16          nFuncMask;
    sal_uInt16          nFuncCount;
    ::com::sun::star::sheet::DataPilotFieldReference maFieldRef;

    explicit            ScPivotField( SCCOL nNewCol = 0, sal_uInt16 nNewFuncMask = PIVOT_FUNC_NONE );

    bool                operator==( const ScPivotField& r ) const;
};

typedef ::std::vector< ScPivotField > ScPivotFieldVector;

// ============================================================================

struct ScPivotParam
{
    SCCOL           nCol;           // Cursor Position /
    SCROW           nRow;           // bzw. Anfang des Zielbereiches
    SCTAB           nTab;
    ScDPLabelDataVector maLabelArray;
    ScPivotFieldVector maPageArr;
    ScPivotFieldVector maColArr;
    ScPivotFieldVector maRowArr;
    ScPivotFieldVector maDataArr;
    bool            bIgnoreEmptyRows;
    bool            bDetectCategories;
    bool            bMakeTotalCol;
    bool            bMakeTotalRow;

    ScPivotParam();

    bool            operator==( const ScPivotParam& r ) const;
};

// ============================================================================

struct ScPivotFuncData
{
    SCCOL               mnCol;
    sal_uInt16          mnFuncMask;
    ::com::sun::star::sheet::DataPilotFieldReference maFieldRef;

    explicit            ScPivotFuncData( SCCOL nCol, sal_uInt16 nFuncMask );
    explicit            ScPivotFuncData( SCCOL nCol, sal_uInt16 nFuncMask,
                            const ::com::sun::star::sheet::DataPilotFieldReference& rFieldRef );
};

typedef ::std::vector< ScPivotFuncData > ScPivotFuncDataVector;

// ============================================================================

typedef std::vector< String > ScDPNameVec;

// ============================================================================

#endif
