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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifdef _MSC_VER
#pragma optimize("",off)
#endif

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/boxitem.hxx>
#include <editeng/wghtitem.hxx>
#include <svx/algitem.hxx>
#include <unotools/transliterationwrapper.hxx>

#include "globstr.hrc"
#include "subtotal.hxx"
#include "rangeutl.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "document.hxx"
#include "userlist.hxx"
#include "pivot.hxx"
#include "rechead.hxx"
#include "formula/errorcodes.hxx"                           // fuer errNoValue
#include "refupdat.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"

using ::com::sun::star::sheet::DataPilotFieldReference;
using ::rtl::OUString;

// ============================================================================

ScDPLabelData::Member::Member() :
    mbVisible(true),
    mbShowDetails(true)
{
}

OUString ScDPLabelData::Member::getDisplayName() const
{
    if (maLayoutName.getLength())
        return maLayoutName;

    return maName;
}

ScDPLabelData::ScDPLabelData( const String& rName, SCCOL nCol, bool bIsValue ) :
    maName( rName ),
    mnCol( nCol ),
    mnFuncMask( PIVOT_FUNC_NONE ),
    mnUsedHier( 0 ),
    mnFlags( 0 ),
    mbShowAll( false ),
    mbIsValue( bIsValue )
{
}

OUString ScDPLabelData::getDisplayName() const
{
    if (maLayoutName.getLength())
        return maLayoutName;

    return maName;
}

// ============================================================================

ScPivotField::ScPivotField( SCCOL nNewCol, sal_uInt16 nNewFuncMask ) :
    nCol( nNewCol ),
    nFuncMask( nNewFuncMask ),
    nFuncCount( 0 )
{
}

bool ScPivotField::operator==( const ScPivotField& r ) const
{
    return (nCol                            == r.nCol)
        && (nFuncMask                       == r.nFuncMask)
        && (nFuncCount                      == r.nFuncCount)
        && (maFieldRef.ReferenceType        == r.maFieldRef.ReferenceType)
        && (maFieldRef.ReferenceField       == r.maFieldRef.ReferenceField)
        && (maFieldRef.ReferenceItemType    == r.maFieldRef.ReferenceItemType)
        && (maFieldRef.ReferenceItemName    == r.maFieldRef.ReferenceItemName);
}

// ============================================================================

ScPivotParam::ScPivotParam()
    :   nCol( 0 ), nRow( 0 ), nTab( 0 ),
        bIgnoreEmptyRows( false ), bDetectCategories( false ),
        bMakeTotalCol( true ), bMakeTotalRow( true )
{
}

bool ScPivotParam::operator==( const ScPivotParam& r ) const
{
    return
        (nCol == r.nCol)
     && (nRow == r.nRow)
     && (nTab == r.nTab)
     && (bIgnoreEmptyRows  == r.bIgnoreEmptyRows)
     && (bDetectCategories == r.bDetectCategories)
     && (bMakeTotalCol == r.bMakeTotalCol)
     && (bMakeTotalRow == r.bMakeTotalRow)
     && (maLabelArray.size() == r.maLabelArray.size())  // ! only size??
     && (maPageArr == r.maPageArr)
     && (maColArr == r.maColArr)
     && (maRowArr == r.maRowArr)
     && (maDataArr == r.maDataArr);
}

// ============================================================================

ScPivotFuncData::ScPivotFuncData( SCCOL nCol, sal_uInt16 nFuncMask ) :
    mnCol( nCol ),
    mnFuncMask( nFuncMask )
{
}

ScPivotFuncData::ScPivotFuncData( SCCOL nCol, sal_uInt16 nFuncMask, const DataPilotFieldReference& rFieldRef ) :
    mnCol( nCol ),
    mnFuncMask( nFuncMask ),
    maFieldRef( rFieldRef )
{
}

// ============================================================================

