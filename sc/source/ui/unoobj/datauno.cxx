/*************************************************************************
 *
 *  $RCSfile: datauno.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-26 18:58:52 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <tools/debug.hxx>
#include <svtools/smplhint.hxx>
#include <svtools/zforlist.hxx>
#include <rtl/uuid.h>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/util/SortField.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/table/TableOrientation.hpp>
#include <com/sun/star/sheet/DataImportMode.hpp>

#include "datauno.hxx"
#include "dapiuno.hxx"
#include "cellsuno.hxx"
#include "miscuno.hxx"
#include "targuno.hxx"
#include "rangeutl.hxx"
#include "dbcolect.hxx"
#include "docsh.hxx"
#include "dbdocfun.hxx"
#include "unoguard.hxx"
#include "unonames.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

//  alles ohne Which-ID, Map nur fuer PropertySetInfo

const SfxItemPropertyMap* lcl_GetSubTotalPropertyMap()
{
    static SfxItemPropertyMap aSubTotalPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_CASE),     0,  &getBooleanCppuType(),       0},
        {MAP_CHAR_LEN(SC_UNONAME_FORMATS),  0,  &getBooleanCppuType(),       0},
        {MAP_CHAR_LEN(SC_UNONAME_INSBRK),   0,  &getBooleanCppuType(),       0},
        {MAP_CHAR_LEN(SC_UNONAME_ULIST),    0,  &getBooleanCppuType(),       0},
        {MAP_CHAR_LEN(SC_UNONAME_UINDEX),   0,  &getCppuType((sal_Int32*)0), 0},
        {0,0,0,0}
    };
    return aSubTotalPropertyMap_Impl;
}

const SfxItemPropertyMap* lcl_GetFilterPropertyMap()
{
    static SfxItemPropertyMap aFilterPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_CONTHDR),  0,  &getBooleanCppuType(),                      0},
        {MAP_CHAR_LEN(SC_UNONAME_COPYOUT),  0,  &getBooleanCppuType(),                      0},
        {MAP_CHAR_LEN(SC_UNONAME_ISCASE),   0,  &getBooleanCppuType(),                      0},
        {MAP_CHAR_LEN(SC_UNONAME_MAXFLD),   0,  &getCppuType((sal_Int32*)0),                beans::PropertyAttribute::READONLY},
        {MAP_CHAR_LEN(SC_UNONAME_ORIENT),   0,  &getCppuType((table::TableOrientation*)0),  0},
        {MAP_CHAR_LEN(SC_UNONAME_OUTPOS),   0,  &getCppuType((table::CellAddress*)0),       0},
        {MAP_CHAR_LEN(SC_UNONAME_SAVEOUT),  0,  &getBooleanCppuType(),                      0},
        {MAP_CHAR_LEN(SC_UNONAME_SKIPDUP),  0,  &getBooleanCppuType(),                      0},
        {MAP_CHAR_LEN(SC_UNONAME_USEREGEX), 0,  &getBooleanCppuType(),                      0},
        {0,0,0,0}
    };
    return aFilterPropertyMap_Impl;
}

const SfxItemPropertyMap* lcl_GetDBRangePropertyMap()
{
    static SfxItemPropertyMap aDBRangePropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_KEEPFORM), 0,  &getBooleanCppuType(),           0},
        {MAP_CHAR_LEN(SC_UNO_LINKDISPBIT),  0,  &getCppuType((uno::Reference<awt::XBitmap>*)0), beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNO_LINKDISPNAME), 0,  &getCppuType((rtl::OUString*)0), beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_MOVCELLS), 0,  &getBooleanCppuType(),           0},
        {MAP_CHAR_LEN(SC_UNONAME_STRIPDAT), 0,  &getBooleanCppuType(),           0},
        {0,0,0,0}
    };
    return aDBRangePropertyMap_Impl;
}


//------------------------------------------------------------------------

#define SCDATABASERANGEOBJ_SERVICE      "com.sun.star.sheet.DatabaseRange"

SC_SIMPLE_SERVICE_INFO( ScConsolidationDescriptor, "ScConsolidationDescriptor", "com.sun.star.sheet.ConsolidationDescriptor" )
SC_SIMPLE_SERVICE_INFO( ScDatabaseRangesObj, "ScDatabaseRangesObj", "com.sun.star.sheet.DatabaseRanges" )
SC_SIMPLE_SERVICE_INFO( ScFilterDescriptorBase, "ScFilterDescriptorBase", "com.sun.star.sheet.SheetFilterDescriptor" )
SC_SIMPLE_SERVICE_INFO( ScSubTotalDescriptorBase, "ScSubTotalDescriptorBase", "com.sun.star.sheet.SubTotalDescriptor" )
SC_SIMPLE_SERVICE_INFO( ScSubTotalFieldObj, "ScSubTotalFieldObj", "com.sun.star.sheet.SubTotalField" )


//------------------------------------------------------------------------

// static
ScSubTotalFunc ScDataUnoConversion::GeneralToSubTotal( sheet::GeneralFunction eSummary )
{
    ScSubTotalFunc eSubTotal;
    switch (eSummary)
    {
        case sheet::GeneralFunction_NONE:       eSubTotal = SUBTOTAL_FUNC_NONE; break;
        case sheet::GeneralFunction_SUM:        eSubTotal = SUBTOTAL_FUNC_SUM;  break;
        case sheet::GeneralFunction_COUNT:      eSubTotal = SUBTOTAL_FUNC_CNT2; break;
        case sheet::GeneralFunction_AVERAGE:    eSubTotal = SUBTOTAL_FUNC_AVE;  break;
        case sheet::GeneralFunction_MAX:        eSubTotal = SUBTOTAL_FUNC_MAX;  break;
        case sheet::GeneralFunction_MIN:        eSubTotal = SUBTOTAL_FUNC_MIN;  break;
        case sheet::GeneralFunction_PRODUCT:    eSubTotal = SUBTOTAL_FUNC_PROD; break;
        case sheet::GeneralFunction_COUNTNUMS:  eSubTotal = SUBTOTAL_FUNC_CNT;  break;
        case sheet::GeneralFunction_STDEV:      eSubTotal = SUBTOTAL_FUNC_STD;  break;
        case sheet::GeneralFunction_STDEVP:     eSubTotal = SUBTOTAL_FUNC_STDP; break;
        case sheet::GeneralFunction_VAR:        eSubTotal = SUBTOTAL_FUNC_VAR;  break;
        case sheet::GeneralFunction_VARP:       eSubTotal = SUBTOTAL_FUNC_VARP; break;
        case sheet::GeneralFunction_AUTO:
        default:
            DBG_ERROR("GeneralToSubTotal: falscher enum");
            eSubTotal = SUBTOTAL_FUNC_NONE;
    }
    return eSubTotal;
}

// static
sheet::GeneralFunction  ScDataUnoConversion::SubTotalToGeneral( ScSubTotalFunc eSubTotal )
{
    sheet::GeneralFunction eGeneral;
    switch (eSubTotal)
    {
        case SUBTOTAL_FUNC_NONE: eGeneral = sheet::GeneralFunction_NONE;      break;
        case SUBTOTAL_FUNC_AVE:  eGeneral = sheet::GeneralFunction_AVERAGE;   break;
        case SUBTOTAL_FUNC_CNT:  eGeneral = sheet::GeneralFunction_COUNTNUMS; break;
        case SUBTOTAL_FUNC_CNT2: eGeneral = sheet::GeneralFunction_COUNT;     break;
        case SUBTOTAL_FUNC_MAX:  eGeneral = sheet::GeneralFunction_MAX;       break;
        case SUBTOTAL_FUNC_MIN:  eGeneral = sheet::GeneralFunction_MIN;       break;
        case SUBTOTAL_FUNC_PROD: eGeneral = sheet::GeneralFunction_PRODUCT;   break;
        case SUBTOTAL_FUNC_STD:  eGeneral = sheet::GeneralFunction_STDEV;     break;
        case SUBTOTAL_FUNC_STDP: eGeneral = sheet::GeneralFunction_STDEVP;    break;
        case SUBTOTAL_FUNC_SUM:  eGeneral = sheet::GeneralFunction_SUM;       break;
        case SUBTOTAL_FUNC_VAR:  eGeneral = sheet::GeneralFunction_VAR;       break;
        case SUBTOTAL_FUNC_VARP: eGeneral = sheet::GeneralFunction_VARP;      break;
        default:
            DBG_ERROR("SubTotalToGeneral: falscher enum");
            eGeneral = sheet::GeneralFunction_NONE;
            break;
    }
    return eGeneral;
}

//------------------------------------------------------------------------

//  ScImportDescriptor: alles static

long ScImportDescriptor::GetPropertyCount()
{
    return 4;
}

void ScImportDescriptor::FillProperties( uno::Sequence<beans::PropertyValue>& rSeq, const ScImportParam& rParam )
{
    DBG_ASSERT( rSeq.getLength() == GetPropertyCount(), "falscher Count" );

    beans::PropertyValue* pArray = rSeq.getArray();

    sheet::DataImportMode eMode = sheet::DataImportMode_NONE;
    if ( rParam.bImport )
    {
        if ( rParam.bSql )
            eMode = sheet::DataImportMode_SQL;
        else if ( rParam.nType == ScDbQuery )
            eMode = sheet::DataImportMode_QUERY;
        else
            eMode = sheet::DataImportMode_TABLE;        // Type ist immer ScDbQuery oder ScDbTable
    }

    pArray[0].Name = rtl::OUString::createFromAscii( SC_UNONAME_DBNAME );
    pArray[0].Value <<= rtl::OUString( rParam.aDBName );

    pArray[1].Name = rtl::OUString::createFromAscii( SC_UNONAME_SRCTYPE );
    pArray[1].Value <<= eMode;

    pArray[2].Name = rtl::OUString::createFromAscii( SC_UNONAME_SRCOBJ );
    pArray[2].Value <<= rtl::OUString( rParam.aStatement );

    pArray[3].Name = rtl::OUString::createFromAscii( SC_UNONAME_NATIVE );
    ScUnoHelpFunctions::SetBoolInAny( pArray[3].Value, rParam.bNative );
}

void ScImportDescriptor::FillImportParam( ScImportParam& rParam, const uno::Sequence<beans::PropertyValue>& rSeq )
{
    rtl::OUString aStrVal;
    const beans::PropertyValue* pPropArray = rSeq.getConstArray();
    long nPropCount = rSeq.getLength();
    for (long i = 0; i < nPropCount; i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        String aPropName = rProp.Name;

        if (aPropName.EqualsAscii( SC_UNONAME_NATIVE ))
            rParam.bNative = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
        else if (aPropName.EqualsAscii( SC_UNONAME_DBNAME ))
        {
            if ( rProp.Value >>= aStrVal )
                rParam.aDBName = String( aStrVal );
        }
        else if (aPropName.EqualsAscii( SC_UNONAME_SRCOBJ ))
        {
            if ( rProp.Value >>= aStrVal )
                rParam.aStatement = String( aStrVal );
        }
        else if (aPropName.EqualsAscii( SC_UNONAME_SRCTYPE ))
        {
            //! test for correct enum type?
            sheet::DataImportMode eMode = (sheet::DataImportMode)
                                ScUnoHelpFunctions::GetEnumFromAny( rProp.Value );
            switch (eMode)
            {
                case sheet::DataImportMode_NONE:
                    rParam.bImport = FALSE;
                    break;
                case sheet::DataImportMode_SQL:
                    rParam.bImport = TRUE;
                    rParam.bSql    = TRUE;
                    break;
                case sheet::DataImportMode_TABLE:
                    rParam.bImport = TRUE;
                    rParam.bSql    = FALSE;
                    rParam.nType   = ScDbTable;
                    break;
                case sheet::DataImportMode_QUERY:
                    rParam.bImport = TRUE;
                    rParam.bSql    = FALSE;
                    rParam.nType   = ScDbQuery;
                    break;
                default:
                    DBG_ERROR("falscher Mode");
                    rParam.bImport = FALSE;
            }
        }
    }
}

//------------------------------------------------------------------------

//  ScSortDescriptor: alles static

//! SortAscending muss aus der SheetSortDescriptor service-Beschreibung raus

long ScSortDescriptor::GetPropertyCount()
{
    return 10;      // TableSortDescriptor und SheetSortDescriptor
}

void ScSortDescriptor::FillProperties( uno::Sequence<beans::PropertyValue>& rSeq, const ScSortParam& rParam )
{
    DBG_ASSERT( rSeq.getLength() == GetPropertyCount(), "falscher Count" );

    beans::PropertyValue* pArray = rSeq.getArray();

    //  Uno-Werte zusammensuchen

    table::TableOrientation eOrient =
        rParam.bByRow ? table::TableOrientation_ROWS : table::TableOrientation_COLUMNS;

    table::CellAddress aOutPos;
    aOutPos.Sheet  = rParam.nDestTab;
    aOutPos.Column = rParam.nDestCol;
    aOutPos.Row    = rParam.nDestRow;

    USHORT nSortCount = 0;
    while ( nSortCount < MAXSORT && rParam.bDoSort[nSortCount] )
        ++nSortCount;

    uno::Sequence<util::SortField> aFields(nSortCount);
    if (nSortCount)
    {
        util::SortField* pFieldArray = aFields.getArray();
        for (USHORT i=0; i<nSortCount; i++)
        {
            pFieldArray[i].Field         = rParam.nField[i];
            pFieldArray[i].SortAscending = rParam.bAscending[i];
            pFieldArray[i].FieldType     = util::SortFieldType_AUTOMATIC;       // immer Automatic
        }
    }

    //  Sequence fuellen

    pArray[0].Name = rtl::OUString::createFromAscii( SC_UNONAME_ORIENT );
    pArray[0].Value <<= eOrient;

    pArray[1].Name = rtl::OUString::createFromAscii( SC_UNONAME_CONTHDR );
    ScUnoHelpFunctions::SetBoolInAny( pArray[1].Value, rParam.bHasHeader );

    pArray[2].Name = rtl::OUString::createFromAscii( SC_UNONAME_MAXFLD );
    pArray[2].Value <<= (sal_Int32) MAXSORT;

    pArray[3].Name = rtl::OUString::createFromAscii( SC_UNONAME_SORTFLD );
    pArray[3].Value <<= aFields;

    pArray[4].Name = rtl::OUString::createFromAscii( SC_UNONAME_ISCASE );
    ScUnoHelpFunctions::SetBoolInAny( pArray[4].Value, rParam.bCaseSens );

    pArray[5].Name = rtl::OUString::createFromAscii( SC_UNONAME_BINDFMT );
    ScUnoHelpFunctions::SetBoolInAny( pArray[5].Value, rParam.bIncludePattern );

    pArray[6].Name = rtl::OUString::createFromAscii( SC_UNONAME_COPYOUT );
    ScUnoHelpFunctions::SetBoolInAny( pArray[6].Value, !rParam.bInplace );

    pArray[7].Name = rtl::OUString::createFromAscii( SC_UNONAME_OUTPOS );
    pArray[7].Value <<= aOutPos;

    pArray[8].Name = rtl::OUString::createFromAscii( SC_UNONAME_ISULIST );
    ScUnoHelpFunctions::SetBoolInAny( pArray[8].Value, rParam.bUserDef );

    pArray[9].Name = rtl::OUString::createFromAscii( SC_UNONAME_UINDEX );
    pArray[9].Value <<= (sal_Int32) rParam.nUserIndex;
}

void ScSortDescriptor::FillSortParam( ScSortParam& rParam, const uno::Sequence<beans::PropertyValue>& rSeq )
{
    const beans::PropertyValue* pPropArray = rSeq.getConstArray();
    long nPropCount = rSeq.getLength();
    for (long i = 0; i < nPropCount; i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        String aPropName = rProp.Name;

        if (aPropName.EqualsAscii( SC_UNONAME_ORIENT ))
        {
            //! test for correct enum type?
            table::TableOrientation eOrient = (table::TableOrientation)
                                ScUnoHelpFunctions::GetEnumFromAny( rProp.Value );
            rParam.bByRow = ( eOrient != table::TableOrientation_COLUMNS );
        }
        else if (aPropName.EqualsAscii( SC_UNONAME_CONTHDR ))
            rParam.bHasHeader = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
        else if (aPropName.EqualsAscii( SC_UNONAME_MAXFLD ))
        {
            sal_Int32 nVal;
            if ( (rProp.Value >>= nVal) && nVal > MAXSORT )
            {
                //! specify exceptions
                //! throw lang::IllegalArgumentException();
            }
        }
        else if (aPropName.EqualsAscii( SC_UNONAME_SORTFLD ))
        {
            uno::Sequence<util::SortField> aSeq;
            if ( rProp.Value >>= aSeq )
            {
                INT32 nCount = aSeq.getLength();
                INT32 i;
                if ( nCount > MAXSORT )
                {
                    DBG_ERROR("Zu viele Sortierfelder");
                    nCount = MAXSORT;
                }
                const util::SortField* pFieldArray = aSeq.getConstArray();
                for (i=0; i<nCount; i++)
                {
                    rParam.nField[i]     = pFieldArray[i].Field;
                    rParam.bAscending[i] = pFieldArray[i].SortAscending;
                    // FieldType wird ignoriert
                    rParam.bDoSort[i] = TRUE;
                }
                for (i=nCount; i<MAXSORT; i++)
                    rParam.bDoSort[i] = FALSE;
            }
        }
        else if (aPropName.EqualsAscii( SC_UNONAME_ISCASE ))
            rParam.bCaseSens = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
        else if (aPropName.EqualsAscii( SC_UNONAME_BINDFMT ))
            rParam.bIncludePattern = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
        else if (aPropName.EqualsAscii( SC_UNONAME_COPYOUT ))
            rParam.bInplace = !ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
        else if (aPropName.EqualsAscii( SC_UNONAME_OUTPOS ))
        {
            table::CellAddress aAddress;
            if ( rProp.Value >>= aAddress )
            {
                rParam.nDestTab = aAddress.Sheet;
                rParam.nDestCol = aAddress.Column;
                rParam.nDestRow = aAddress.Row;
            }
        }
        else if (aPropName.EqualsAscii( SC_UNONAME_ISULIST ))
            rParam.bUserDef = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
        else if (aPropName.EqualsAscii( SC_UNONAME_UINDEX ))
        {
            sal_Int32 nVal;
            if ( rProp.Value >>= nVal )
                rParam.nUserIndex = nVal;
        }
    }
}

//------------------------------------------------------------------------

ScSubTotalFieldObj::ScSubTotalFieldObj( ScSubTotalDescriptorBase* pDesc, USHORT nP ) :
    rParent( *pDesc ),
    xRef( pDesc ),          // Objekt festhalten
    nPos( nP )
{
    DBG_ASSERT(pDesc, "ScSubTotalFieldObj: Parent ist 0");
}

ScSubTotalFieldObj::~ScSubTotalFieldObj()
{
}

// XSubTotalField

sal_Int32 SAL_CALL ScSubTotalFieldObj::getGroupColumn() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScSubTotalParam aParam;
    rParent.GetData(aParam);

    return aParam.nField[nPos];
}

void SAL_CALL ScSubTotalFieldObj::setGroupColumn( sal_Int32 nGroupColumn ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScSubTotalParam aParam;
    rParent.GetData(aParam);

    aParam.nField[nPos] = nGroupColumn;

    rParent.PutData(aParam);
}

uno::Sequence<sheet::SubTotalColumn> SAL_CALL ScSubTotalFieldObj::getSubTotalColumns()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScSubTotalParam aParam;
    rParent.GetData(aParam);

    USHORT nCount = aParam.nSubTotals[nPos];
    uno::Sequence<sheet::SubTotalColumn> aSeq(nCount);
    sheet::SubTotalColumn* pAry = aSeq.getArray();
    for (USHORT i=0; i<nCount; i++)
    {
        pAry[i].Column = aParam.pSubTotals[nPos][i];
        pAry[i].Function = ScDataUnoConversion::SubTotalToGeneral(
                                        aParam.pFunctions[nPos][i] );
    }
    return aSeq;
}

void SAL_CALL ScSubTotalFieldObj::setSubTotalColumns(
                            const uno::Sequence<sheet::SubTotalColumn>& aSubTotalColumns )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScSubTotalParam aParam;
    rParent.GetData(aParam);

    UINT32 nColCount = aSubTotalColumns.getLength();
    if ( nColCount <= USHRT_MAX )
    {
        USHORT nCount = (USHORT)nColCount;
        aParam.nSubTotals[nPos] = nCount;
        if (nCount)
        {
            aParam.pSubTotals[nPos] = new USHORT[nCount];
            aParam.pFunctions[nPos] = new ScSubTotalFunc[nCount];

            const sheet::SubTotalColumn* pAry = aSubTotalColumns.getConstArray();
            for (USHORT i=0; i<nCount; i++)
            {
                aParam.pSubTotals[nPos][i] = pAry[i].Column;
                aParam.pFunctions[nPos][i] =
                            ScDataUnoConversion::GeneralToSubTotal( pAry[i].Function );
            }
        }
        else
        {
            aParam.pSubTotals[nPos] = NULL;
            aParam.pFunctions[nPos] = NULL;
        }
    }
    //! sonst Exception oder so? (zuviele Spalten)

    rParent.PutData(aParam);
}

//------------------------------------------------------------------------

ScSubTotalDescriptorBase::ScSubTotalDescriptorBase() :
    aPropSet( lcl_GetSubTotalPropertyMap() )
{
}

ScSubTotalDescriptorBase::~ScSubTotalDescriptorBase()
{
}

// GetData/PutData hier nur wegen NewInstance-Krempel implementiert...

void ScSubTotalDescriptorBase::GetData( ScSubTotalParam& rParam ) const
{
    DBG_ERROR("ScSubTotalDescriptorBase::GetData soll nicht gerufen werden");
}

void ScSubTotalDescriptorBase::PutData( const ScSubTotalParam& rParam )
{
    DBG_ERROR("ScSubTotalDescriptorBase::PutData soll nicht gerufen werden");
}

// XSubTotalDesctiptor

ScSubTotalFieldObj* ScSubTotalDescriptorBase::GetObjectByIndex_Impl(USHORT nIndex)
{
    if ( nIndex < getCount() )
        return new ScSubTotalFieldObj( this, nIndex );
    return NULL;
}

void SAL_CALL ScSubTotalDescriptorBase::clear() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScSubTotalParam aParam;
    GetData(aParam);

    for (USHORT i=0; i<MAXSUBTOTAL; i++)
        aParam.bGroupActive[i] = FALSE;

    //! Notify oder so fuer die Field-Objekte???

    PutData(aParam);
}

void SAL_CALL ScSubTotalDescriptorBase::addNew(
                        const uno::Sequence<sheet::SubTotalColumn>& aSubTotalColumns,
                        sal_Int32 nGroupColumn ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScSubTotalParam aParam;
    GetData(aParam);

    USHORT nPos = 0;
    while ( nPos < MAXSUBTOTAL && aParam.bGroupActive[nPos] )
        ++nPos;

    UINT32 nColCount = aSubTotalColumns.getLength();

    if ( nPos < MAXSUBTOTAL && nColCount <= USHRT_MAX )
    {
        aParam.bGroupActive[nPos] = TRUE;
        aParam.nField[nPos] = nGroupColumn;

        delete aParam.pSubTotals[nPos];
        delete aParam.pFunctions[nPos];

        USHORT nCount = (USHORT)nColCount;
        aParam.nSubTotals[nPos] = nCount;
        if (nCount)
        {
            aParam.pSubTotals[nPos] = new USHORT[nCount];
            aParam.pFunctions[nPos] = new ScSubTotalFunc[nCount];

            const sheet::SubTotalColumn* pAry = aSubTotalColumns.getConstArray();
            for (USHORT i=0; i<nCount; i++)
            {
                aParam.pSubTotals[nPos][i] = pAry[i].Column;
                aParam.pFunctions[nPos][i] =
                            ScDataUnoConversion::GeneralToSubTotal( pAry[i].Function );
            }
        }
        else
        {
            aParam.pSubTotals[nPos] = NULL;
            aParam.pFunctions[nPos] = NULL;
        }
    }
    else                                    // too many fields / columns
        throw uno::RuntimeException();      // no other exceptions specified

    PutData(aParam);
}

//  Flags/Einstellungen als Properties

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScSubTotalDescriptorBase::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this);
}

// XIndexAccess

sal_Int32 SAL_CALL ScSubTotalDescriptorBase::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScSubTotalParam aParam;
    GetData(aParam);

    USHORT nCount = 0;
    while ( nCount < MAXSUBTOTAL && aParam.bGroupActive[nCount] )
        ++nCount;
    return nCount;
}

uno::Any SAL_CALL ScSubTotalDescriptorBase::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<sheet::XSubTotalField> xField = GetObjectByIndex_Impl(nIndex);
    uno::Any aAny;
    if (xField.is())
        aAny <<= xField;
    else
        throw lang::IndexOutOfBoundsException();
    return aAny;
}

uno::Type SAL_CALL ScSubTotalDescriptorBase::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<sheet::XSubTotalField>*)0);
}

sal_Bool SAL_CALL ScSubTotalDescriptorBase::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScSubTotalDescriptorBase::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScSubTotalDescriptorBase::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScSubTotalParam aParam;
    GetData(aParam);

    String aString = aPropertyName;

    if (aString.EqualsAscii( SC_UNONAME_CASE ))
        aParam.bCaseSens = ScUnoHelpFunctions::GetBoolFromAny( aValue );
    else if (aString.EqualsAscii( SC_UNONAME_FORMATS ))
        aParam.bIncludePattern = ScUnoHelpFunctions::GetBoolFromAny( aValue );
    else if (aString.EqualsAscii( SC_UNONAME_INSBRK ))
        aParam.bPagebreak = ScUnoHelpFunctions::GetBoolFromAny( aValue );
    else if (aString.EqualsAscii( SC_UNONAME_ULIST ))
        aParam.bUserDef = ScUnoHelpFunctions::GetBoolFromAny( aValue );
    else if (aString.EqualsAscii( SC_UNONAME_UINDEX ))
    {
        sal_Int32 nVal;
        if ( aValue >>= nVal )
            aParam.nUserIndex = nVal;
    }

    PutData(aParam);
}

uno::Any SAL_CALL ScSubTotalDescriptorBase::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScSubTotalParam aParam;
    GetData(aParam);

    String aString = aPropertyName;
    uno::Any aRet;

    if (aString.EqualsAscii( SC_UNONAME_CASE ))
        ScUnoHelpFunctions::SetBoolInAny( aRet, aParam.bCaseSens );
    else if (aString.EqualsAscii( SC_UNONAME_FORMATS ))
        ScUnoHelpFunctions::SetBoolInAny( aRet, aParam.bIncludePattern );
    else if (aString.EqualsAscii( SC_UNONAME_INSBRK ))
        ScUnoHelpFunctions::SetBoolInAny( aRet, aParam.bPagebreak );
    else if (aString.EqualsAscii( SC_UNONAME_ULIST ))
        ScUnoHelpFunctions::SetBoolInAny( aRet, aParam.bUserDef );
    else if (aString.EqualsAscii( SC_UNONAME_UINDEX ))
        aRet <<= (sal_Int32) aParam.nUserIndex;

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScSubTotalDescriptorBase )

// XUnoTunnel

sal_Int64 SAL_CALL ScSubTotalDescriptorBase::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

// static
const uno::Sequence<sal_Int8>& ScSubTotalDescriptorBase::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

// static
ScSubTotalDescriptorBase* ScSubTotalDescriptorBase::getImplementation(
                                const uno::Reference<sheet::XSubTotalDescriptor> xObj )
{
    ScSubTotalDescriptorBase* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScSubTotalDescriptorBase*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}

//------------------------------------------------------------------------

ScSubTotalDescriptor::ScSubTotalDescriptor()
{
}

ScSubTotalDescriptor::~ScSubTotalDescriptor()
{
}

void ScSubTotalDescriptor::GetData( ScSubTotalParam& rParam ) const
{
    rParam = aStoredParam;          // Abfrage fuer Interface
}

void ScSubTotalDescriptor::PutData( const ScSubTotalParam& rParam )
{
    aStoredParam = rParam;          // vom Interface gesetzt
}

void ScSubTotalDescriptor::SetParam( const ScSubTotalParam& rNew )
{
    aStoredParam = rNew;            // von aussen gesetzt
}

//------------------------------------------------------------------------

ScRangeSubTotalDescriptor::ScRangeSubTotalDescriptor(ScDatabaseRangeObj* pPar) :
    pParent(pPar)
{
    if (pParent)
        pParent->acquire();
}

ScRangeSubTotalDescriptor::~ScRangeSubTotalDescriptor()
{
    if (pParent)
        pParent->release();
}

void ScRangeSubTotalDescriptor::GetData( ScSubTotalParam& rParam ) const
{
    if (pParent)
        pParent->GetSubTotalParam( rParam );
}

void ScRangeSubTotalDescriptor::PutData( const ScSubTotalParam& rParam )
{
    if (pParent)
        pParent->SetSubTotalParam( rParam );
}

//------------------------------------------------------------------------

ScConsolidationDescriptor::ScConsolidationDescriptor()
{
}

ScConsolidationDescriptor::~ScConsolidationDescriptor()
{
}

void ScConsolidationDescriptor::SetParam( const ScConsolidateParam& rNew )
{
    aParam = rNew;
}

// XConsolidationDescriptor

sheet::GeneralFunction SAL_CALL ScConsolidationDescriptor::getFunction() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ScDataUnoConversion::SubTotalToGeneral(aParam.eFunction);
}

void SAL_CALL ScConsolidationDescriptor::setFunction( sheet::GeneralFunction nFunction )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    aParam.eFunction = ScDataUnoConversion::GeneralToSubTotal(nFunction);
}

uno::Sequence<table::CellRangeAddress> SAL_CALL ScConsolidationDescriptor::getSources()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    USHORT nCount = aParam.nDataAreaCount;
    if (!aParam.ppDataAreas)
        nCount = 0;
    table::CellRangeAddress aRange;
    uno::Sequence<table::CellRangeAddress> aSeq(nCount);
    table::CellRangeAddress* pAry = aSeq.getArray();
    for (USHORT i=0; i<nCount; i++)
    {
        ScArea* pArea = aParam.ppDataAreas[i];
        if (pArea)
        {
            aRange.Sheet        = pArea->nTab;
            aRange.StartColumn  = pArea->nColStart;
            aRange.StartRow     = pArea->nRowStart;
            aRange.EndColumn    = pArea->nColEnd;
            aRange.EndRow       = pArea->nRowEnd;
        }
        pAry[i] = aRange;
    }
    return aSeq;
}

void SAL_CALL ScConsolidationDescriptor::setSources(
                    const uno::Sequence<table::CellRangeAddress>& aSources )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    USHORT nCount = (USHORT)aSources.getLength();
    if (nCount)
    {
        const table::CellRangeAddress* pAry = aSources.getConstArray();
        ScArea** pNew = new ScArea*[nCount];
        USHORT i;
        for (i=0; i<nCount; i++)
            pNew[i] = new ScArea( pAry[i].Sheet,
                                    pAry[i].StartColumn, pAry[i].StartRow,
                                    pAry[i].EndColumn,   pAry[i].EndRow );

        aParam.SetAreas( pNew, nCount );    // kopiert alles

        for (i=0; i<nCount; i++)
            delete pNew[i];
        delete[] pNew;
    }
    else
        aParam.ClearDataAreas();
}

table::CellAddress SAL_CALL ScConsolidationDescriptor::getStartOutputPosition()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellAddress aPos;
    aPos.Column = aParam.nCol;
    aPos.Row    = aParam.nRow;
    aPos.Sheet  = aParam.nTab;
    return aPos;
}

void SAL_CALL ScConsolidationDescriptor::setStartOutputPosition(
                                const table::CellAddress& aStartOutputPosition )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    aParam.nCol = aStartOutputPosition.Column;
    aParam.nRow = aStartOutputPosition.Row;
    aParam.nTab = aStartOutputPosition.Sheet;
}

sal_Bool SAL_CALL ScConsolidationDescriptor::getUseColumnHeaders() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return aParam.bByCol;
}

void SAL_CALL ScConsolidationDescriptor::setUseColumnHeaders( sal_Bool bUseColumnHeaders )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    aParam.bByCol = bUseColumnHeaders;
}

sal_Bool SAL_CALL ScConsolidationDescriptor::getUseRowHeaders() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return aParam.bByRow;
}

void SAL_CALL ScConsolidationDescriptor::setUseRowHeaders( sal_Bool bUseRowHeaders )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    aParam.bByRow = bUseRowHeaders;
}

sal_Bool SAL_CALL ScConsolidationDescriptor::getInsertLinks() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return aParam.bReferenceData;
}

void SAL_CALL ScConsolidationDescriptor::setInsertLinks( sal_Bool bInsertLinks )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    aParam.bReferenceData = bInsertLinks;
}

//------------------------------------------------------------------------

ScFilterDescriptorBase::ScFilterDescriptorBase() :
    aPropSet( lcl_GetFilterPropertyMap() )
{
}

ScFilterDescriptorBase::~ScFilterDescriptorBase()
{
}

// XSheetFilterDescriptor

uno::Sequence<sheet::TableFilterField> SAL_CALL ScFilterDescriptorBase::getFilterFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScQueryParam aParam;
    GetData(aParam);

    USHORT nEntries = aParam.GetEntryCount();   // allozierte Eintraege im Param
    USHORT nCount = 0;                          // aktive
    while ( nCount < nEntries &&
            aParam.GetEntry(nCount).bDoQuery )
        ++nCount;

    sheet::TableFilterField aField;
    uno::Sequence<sheet::TableFilterField> aSeq(nCount);
    sheet::TableFilterField* pAry = aSeq.getArray();
    for (USHORT i=0; i<nCount; i++)
    {
        const ScQueryEntry& rEntry = aParam.GetEntry(i);
        sheet::FilterOperator eFilterOp;
        switch (rEntry.eOp)             // ScQueryOp
        {
            case SC_EQUAL:          eFilterOp = sheet::FilterOperator_EQUAL;          break;
            case SC_LESS:           eFilterOp = sheet::FilterOperator_LESS;           break;
            case SC_GREATER:        eFilterOp = sheet::FilterOperator_GREATER;        break;
            case SC_LESS_EQUAL:     eFilterOp = sheet::FilterOperator_LESS_EQUAL;     break;
            case SC_GREATER_EQUAL:  eFilterOp = sheet::FilterOperator_GREATER_EQUAL;  break;
            case SC_NOT_EQUAL:      eFilterOp = sheet::FilterOperator_NOT_EQUAL;      break;
            case SC_TOPVAL:         eFilterOp = sheet::FilterOperator_TOP_VALUES;     break;
            case SC_BOTVAL:         eFilterOp = sheet::FilterOperator_BOTTOM_VALUES;  break;
            case SC_TOPPERC:        eFilterOp = sheet::FilterOperator_TOP_PERCENT;    break;
            case SC_BOTPERC:        eFilterOp = sheet::FilterOperator_BOTTOM_PERCENT; break;
            default:
                DBG_ERROR("Falscher Filter-enum");
                eFilterOp = sheet::FilterOperator_EMPTY;
        }
        //! FILTER_EMPTY / FILTER_NOTEMPTY umsetzen !!!!!!!!!!!!!!!!!!!!!

        rtl::OUString aStringValue;
        if (rEntry.pStr)
            aStringValue = *rEntry.pStr;

        aField.Connection    = (rEntry.eConnect == SC_AND) ? sheet::FilterConnection_AND :
                                                             sheet::FilterConnection_OR;
        aField.Field         = rEntry.nField;
        aField.Operator      = eFilterOp;
        aField.IsNumeric     = !rEntry.bQueryByString;
        aField.StringValue   = aStringValue;
        aField.NumericValue  = rEntry.nVal;
        pAry[i] = aField;
    }
    return aSeq;
}

void SAL_CALL ScFilterDescriptorBase::setFilterFields(
                const uno::Sequence<sheet::TableFilterField>& aFilterFields )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScQueryParam aParam;
    GetData(aParam);

    USHORT nCount = (USHORT)aFilterFields.getLength();
    DBG_ASSERT( nCount <= MAXQUERY, "setFilterFields: zu viele" );

    aParam.Resize( nCount );

    const sheet::TableFilterField* pAry = aFilterFields.getConstArray();
    USHORT i;
    for (i=0; i<nCount; i++)
    {
        ScQueryEntry& rEntry = aParam.GetEntry(i);
        if (!rEntry.pStr)
            rEntry.pStr = new String;       // sollte nicht sein (soll immer initialisiert sein)

        ScQueryOp eQueryOp;
        switch (pAry[i].Operator)           // FilterOperator
        {
            case sheet::FilterOperator_EQUAL:           eQueryOp = SC_EQUAL;         break;
            case sheet::FilterOperator_LESS:            eQueryOp = SC_LESS;          break;
            case sheet::FilterOperator_GREATER:         eQueryOp = SC_GREATER;       break;
            case sheet::FilterOperator_LESS_EQUAL:      eQueryOp = SC_LESS_EQUAL;    break;
            case sheet::FilterOperator_GREATER_EQUAL:   eQueryOp = SC_GREATER_EQUAL; break;
            case sheet::FilterOperator_NOT_EQUAL:       eQueryOp = SC_NOT_EQUAL;     break;
            case sheet::FilterOperator_TOP_VALUES:      eQueryOp = SC_TOPVAL;        break;
            case sheet::FilterOperator_BOTTOM_VALUES:   eQueryOp = SC_BOTVAL;        break;
            case sheet::FilterOperator_TOP_PERCENT:     eQueryOp = SC_TOPPERC;       break;
            case sheet::FilterOperator_BOTTOM_PERCENT:  eQueryOp = SC_BOTPERC;       break;
            case sheet::FilterOperator_EMPTY:
            case sheet::FilterOperator_NOT_EMPTY:
                //! set SC_EMPTYFIELDS / SC_NONEMPTYFIELDS values !!!!!
                eQueryOp = SC_EQUAL;
                break;
            default:
                DBG_ERROR("Falscher Query-enum");
                eQueryOp = SC_EQUAL;
        }

        rEntry.bDoQuery         = TRUE;
        rEntry.eConnect         = (pAry[i].Connection == sheet::FilterConnection_AND) ? SC_AND : SC_OR;
        rEntry.nField           = pAry[i].Field;
        rEntry.eOp              = eQueryOp;
        rEntry.bQueryByString   = !pAry[i].IsNumeric;
        *rEntry.pStr            = String( pAry[i].StringValue );
        rEntry.nVal             = pAry[i].NumericValue;
    }

    USHORT nParamCount = aParam.GetEntryCount();    // Param wird nicht unter 8 resized
    for (i=nCount; i<nParamCount; i++)
        aParam.GetEntry(i).bDoQuery = FALSE;        // ueberzaehlige Felder zuruecksetzen

    PutData(aParam);
}

// Rest sind Properties

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScFilterDescriptorBase::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScFilterDescriptorBase::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScQueryParam aParam;
    GetData(aParam);

    String aString = aPropertyName;
    if (aString.EqualsAscii( SC_UNONAME_CONTHDR ))
        aParam.bHasHeader = ScUnoHelpFunctions::GetBoolFromAny( aValue );
    else if (aString.EqualsAscii( SC_UNONAME_COPYOUT ))
        aParam.bInplace = !(ScUnoHelpFunctions::GetBoolFromAny( aValue ));
    else if (aString.EqualsAscii( SC_UNONAME_ISCASE ))
        aParam.bCaseSens = ScUnoHelpFunctions::GetBoolFromAny( aValue );
    else if (aString.EqualsAscii( SC_UNONAME_MAXFLD ))
    {
        sal_Int32 nVal;
        if ( (aValue >>= nVal) && nVal > MAXQUERY )
        {
            throw lang::IllegalArgumentException();
        }
    }
    else if (aString.EqualsAscii( SC_UNONAME_ORIENT ))
    {
        //! test for correct enum type?
        table::TableOrientation eOrient = (table::TableOrientation)
                                ScUnoHelpFunctions::GetEnumFromAny( aValue );
        aParam.bByRow = ( eOrient != table::TableOrientation_COLUMNS );
    }
    else if (aString.EqualsAscii( SC_UNONAME_OUTPOS ))
    {
        table::CellAddress aAddress;
        if ( aValue >>= aAddress )
        {
            aParam.nDestTab = aAddress.Sheet;
            aParam.nDestCol = aAddress.Column;
            aParam.nDestRow = aAddress.Row;
        }
    }
    else if (aString.EqualsAscii( SC_UNONAME_SAVEOUT ))
        aParam.bDestPers = ScUnoHelpFunctions::GetBoolFromAny( aValue );
    else if (aString.EqualsAscii( SC_UNONAME_SKIPDUP ))
        aParam.bDuplicate = !(ScUnoHelpFunctions::GetBoolFromAny( aValue ));
    else if (aString.EqualsAscii( SC_UNONAME_USEREGEX ))
        aParam.bRegExp = ScUnoHelpFunctions::GetBoolFromAny( aValue );

    PutData(aParam);
}

uno::Any SAL_CALL ScFilterDescriptorBase::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScQueryParam aParam;
    GetData(aParam);

    String aString = aPropertyName;
    uno::Any aRet;

    if (aString.EqualsAscii( SC_UNONAME_CONTHDR ))
        ScUnoHelpFunctions::SetBoolInAny( aRet, aParam.bHasHeader );
    else if (aString.EqualsAscii( SC_UNONAME_COPYOUT ))
        ScUnoHelpFunctions::SetBoolInAny( aRet, !(aParam.bInplace) );
    else if (aString.EqualsAscii( SC_UNONAME_ISCASE ))
        ScUnoHelpFunctions::SetBoolInAny( aRet, aParam.bCaseSens );
    else if (aString.EqualsAscii( SC_UNONAME_MAXFLD ))
        aRet <<= (sal_Int32) MAXQUERY;
    else if (aString.EqualsAscii( SC_UNONAME_ORIENT ))
    {
        table::TableOrientation eOrient = aParam.bByRow ? table::TableOrientation_ROWS :
                                                          table::TableOrientation_COLUMNS;
        aRet <<= eOrient;
    }
    else if (aString.EqualsAscii( SC_UNONAME_OUTPOS ))
    {
        table::CellAddress aOutPos;
        aOutPos.Sheet  = aParam.nDestTab;
        aOutPos.Column = aParam.nDestCol;
        aOutPos.Row    = aParam.nDestRow;
        aRet <<= aOutPos;
    }
    else if (aString.EqualsAscii( SC_UNONAME_SAVEOUT ))
        ScUnoHelpFunctions::SetBoolInAny( aRet, aParam.bDestPers );
    else if (aString.EqualsAscii( SC_UNONAME_SKIPDUP ))
        ScUnoHelpFunctions::SetBoolInAny( aRet, !(aParam.bDuplicate) );
    else if (aString.EqualsAscii( SC_UNONAME_USEREGEX ))
        ScUnoHelpFunctions::SetBoolInAny( aRet, aParam.bRegExp );

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScFilterDescriptorBase )

//------------------------------------------------------------------------

ScFilterDescriptor::ScFilterDescriptor()
{
}

ScFilterDescriptor::~ScFilterDescriptor()
{
}

void ScFilterDescriptor::GetData( ScQueryParam& rParam ) const
{
    rParam = aStoredParam;          // Abfrage fuer Interface
}

void ScFilterDescriptor::PutData( const ScQueryParam& rParam )
{
    aStoredParam = rParam;          // vom Interface gesetzt
}

void ScFilterDescriptor::SetParam( const ScQueryParam& rNew )
{
    aStoredParam = rNew;            // von aussen gesetzt
}

//------------------------------------------------------------------------

ScRangeFilterDescriptor::ScRangeFilterDescriptor(ScDatabaseRangeObj* pPar) :
    pParent(pPar)
{
    if (pParent)
        pParent->acquire();
}

ScRangeFilterDescriptor::~ScRangeFilterDescriptor()
{
    if (pParent)
        pParent->release();
}

void ScRangeFilterDescriptor::GetData( ScQueryParam& rParam ) const
{
    if (pParent)
        pParent->GetQueryParam( rParam );
}

void ScRangeFilterDescriptor::PutData( const ScQueryParam& rParam )
{
    if (pParent)
        pParent->SetQueryParam( rParam );
}

//------------------------------------------------------------------------

ScDataPilotFilterDescriptor::ScDataPilotFilterDescriptor(ScDataPilotDescriptorBase* pPar) :
    pParent(pPar)
{
    if (pParent)
        pParent->acquire();
}

ScDataPilotFilterDescriptor::~ScDataPilotFilterDescriptor()
{
    if (pParent)
        pParent->release();
}

void ScDataPilotFilterDescriptor::GetData( ScQueryParam& rParam ) const
{
    if (pParent)
    {
        ScPivotParam aPivot;
        ScArea aArea;
        pParent->GetParam( aPivot, rParam, aArea );
    }
}

void ScDataPilotFilterDescriptor::PutData( const ScQueryParam& rParam )
{
    if (pParent)
    {
        ScPivotParam aPivot;
        ScArea       aArea;
        ScQueryParam aOldQuery;
        pParent->GetParam( aPivot, aOldQuery, aArea );
        pParent->SetParam( aPivot, rParam,    aArea );
    }
}

//------------------------------------------------------------------------

ScDatabaseRangeObj::ScDatabaseRangeObj(ScDocShell* pDocSh, const String& rNm) :
    pDocShell( pDocSh ),
    aName( rNm ),
    aPropSet( lcl_GetDBRangePropertyMap() )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScDatabaseRangeObj::~ScDatabaseRangeObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScDatabaseRangeObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //  Ref-Update interessiert nicht

    if ( rHint.ISA( SfxSimpleHint ) && ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
        pDocShell = NULL;       // ungueltig geworden
}

// Hilfsfuntionen

ScDBData* ScDatabaseRangeObj::GetDBData_Impl() const
{
    ScDBData* pRet = NULL;
    if (pDocShell)
    {
        ScDBCollection* pNames = pDocShell->GetDocument()->GetDBCollection();
        if (pNames)
        {
            USHORT nPos = 0;
            if (pNames->SearchName( aName, nPos ))
                pRet = (*pNames)[nPos];
        }
    }
    return pRet;
}

// XNamed

rtl::OUString SAL_CALL ScDatabaseRangeObj::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return aName;
}

void SAL_CALL ScDatabaseRangeObj::setName( const rtl::OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
    {
        ScDBDocFunc aFunc(*pDocShell);
        String aNewStr = aNewName;
        BOOL bOk = aFunc.RenameDBRange( aName, aNewStr, TRUE );
        if (bOk)
            aName = aNewStr;
    }
}

// XDatabaseRange

table::CellRangeAddress SAL_CALL ScDatabaseRangeObj::getDataArea() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellRangeAddress aAddress;
    ScDBData* pData = GetDBData_Impl();
    if (pData)
    {
        ScRange aRange;
        pData->GetArea(aRange);
        aAddress.Sheet       = aRange.aStart.Tab();
        aAddress.StartColumn = aRange.aStart.Col();
        aAddress.StartRow    = aRange.aStart.Row();
        aAddress.EndColumn   = aRange.aEnd.Col();
        aAddress.EndRow      = aRange.aEnd.Row();
    }
    return aAddress;
}

void SAL_CALL ScDatabaseRangeObj::setDataArea( const table::CellRangeAddress& aDataArea )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDBData* pData = GetDBData_Impl();
    if ( pDocShell && pData )
    {
        ScDBData aNewData( *pData );
        //! MoveTo ???
        aNewData.SetArea( aDataArea.Sheet, aDataArea.StartColumn, aDataArea.StartRow,
                                           aDataArea.EndColumn, aDataArea.EndRow );
        ScDBDocFunc aFunc(*pDocShell);
        aFunc.ModifyDBData(aNewData, TRUE);
    }
}

void ScDatabaseRangeObj::GetSortParam(ScSortParam& rSortParam) const
{
    const ScDBData* pData = GetDBData_Impl();
    if (pData)
    {
        pData->GetSortParam(rSortParam);

        //  im SortDescriptor sind die Fields innerhalb des Bereichs gezaehlt
        ScRange aDBRange;
        pData->GetArea(aDBRange);
        USHORT nFieldStart = rSortParam.bByRow ? aDBRange.aStart.Col() : aDBRange.aStart.Row();
        for (USHORT i=0; i<MAXSORT; i++)
            if (rSortParam.bDoSort[i] && rSortParam.nField[i] >= nFieldStart)
                rSortParam.nField[i] -= nFieldStart;
    }
}

void ScDatabaseRangeObj::SetSortParam(const ScSortParam& rSortParam)
{
    const ScDBData* pData = GetDBData_Impl();
    if (pData)
    {
        //  im SortDescriptor sind die Fields innerhalb des Bereichs gezaehlt
        ScSortParam aParam = rSortParam;
        ScRange aDBRange;
        pData->GetArea(aDBRange);
        USHORT nFieldStart = aParam.bByRow ? aDBRange.aStart.Col() : aDBRange.aStart.Row();
        for (USHORT i=0; i<MAXSORT; i++)
            if (aParam.bDoSort[i])
                aParam.nField[i] += nFieldStart;

        ScDBData aNewData( *pData );
        aNewData.SetSortParam(aParam);
        ScDBDocFunc aFunc(*pDocShell);
        aFunc.ModifyDBData(aNewData, TRUE);
    }
}

uno::Sequence<beans::PropertyValue> SAL_CALL ScDatabaseRangeObj::getSortDescriptor()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScSortParam aParam;
    const ScDBData* pData = GetDBData_Impl();
    if (pData)
    {
        pData->GetSortParam(aParam);

        //  im SortDescriptor sind die Fields innerhalb des Bereichs gezaehlt
        ScRange aDBRange;
        pData->GetArea(aDBRange);
        USHORT nFieldStart = aParam.bByRow ? aDBRange.aStart.Col() : aDBRange.aStart.Row();
        for (USHORT i=0; i<MAXSORT; i++)
            if ( aParam.bDoSort[i] && aParam.nField[i] >= nFieldStart )
                aParam.nField[i] -= nFieldStart;
    }

    uno::Sequence<beans::PropertyValue> aSeq( ScSortDescriptor::GetPropertyCount() );
    ScSortDescriptor::FillProperties( aSeq, aParam );
    return aSeq;
}

void ScDatabaseRangeObj::GetQueryParam(ScQueryParam& rQueryParam) const
{
    const ScDBData* pData = GetDBData_Impl();
    if (pData)
    {
        pData->GetQueryParam(rQueryParam);

        //  im FilterDescriptor sind die Fields innerhalb des Bereichs gezaehlt
        ScRange aDBRange;
        pData->GetArea(aDBRange);
        USHORT nFieldStart = rQueryParam.bByRow ? aDBRange.aStart.Col() : aDBRange.aStart.Row();
        USHORT nCount = rQueryParam.GetEntryCount();
        for (USHORT i=0; i<nCount; i++)
        {
            ScQueryEntry& rEntry = rQueryParam.GetEntry(i);
            if (rEntry.bDoQuery && rEntry.nField >= nFieldStart)
                rEntry.nField -= nFieldStart;
        }
    }
}

void ScDatabaseRangeObj::SetQueryParam(const ScQueryParam& rQueryParam)
{
    const ScDBData* pData = GetDBData_Impl();
    if (pData)
    {
        //  im FilterDescriptor sind die Fields innerhalb des Bereichs gezaehlt
        ScQueryParam aParam = rQueryParam;
        ScRange aDBRange;
        pData->GetArea(aDBRange);
        USHORT nFieldStart = aParam.bByRow ? aDBRange.aStart.Col() : aDBRange.aStart.Row();
        USHORT nCount = aParam.GetEntryCount();
        for (USHORT i=0; i<nCount; i++)
        {
            ScQueryEntry& rEntry = aParam.GetEntry(i);
            if (rEntry.bDoQuery)
            {
                rEntry.nField += nFieldStart;
                //  Im Dialog wird immer der String angezeigt -> muss zum Wert passen
                if ( !rEntry.bQueryByString )
                    pDocShell->GetDocument()->GetFormatTable()->
                        GetInputLineString( rEntry.nVal, 0, *rEntry.pStr );
            }
        }

        ScDBData aNewData( *pData );
        aNewData.SetQueryParam(aParam);
        ScDBDocFunc aFunc(*pDocShell);
        aFunc.ModifyDBData(aNewData, TRUE);
    }
}

uno::Reference<sheet::XSheetFilterDescriptor> SAL_CALL ScDatabaseRangeObj::getFilterDescriptor()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScRangeFilterDescriptor(this);
}

void ScDatabaseRangeObj::GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const
{
    const ScDBData* pData = GetDBData_Impl();
    if (pData)
    {
        pData->GetSubTotalParam(rSubTotalParam);

        //  im FilterDescriptor sind die Fields innerhalb des Bereichs gezaehlt
        ScRange aDBRange;
        pData->GetArea(aDBRange);
        USHORT nFieldStart = aDBRange.aStart.Col();
        for (USHORT i=0; i<MAXSUBTOTAL; i++)
        {
            if ( rSubTotalParam.bGroupActive[i] )
            {
                if ( rSubTotalParam.nField[i] >= nFieldStart )
                    rSubTotalParam.nField[i] -= nFieldStart;
                for (USHORT j=0; j<rSubTotalParam.nSubTotals[i]; j++)
                    if ( rSubTotalParam.pSubTotals[i][j] >= nFieldStart )
                        rSubTotalParam.pSubTotals[i][j] -= nFieldStart;
            }
        }
    }
}

void ScDatabaseRangeObj::SetSubTotalParam(const ScSubTotalParam& rSubTotalParam)
{
    const ScDBData* pData = GetDBData_Impl();
    if (pData)
    {
        //  im FilterDescriptor sind die Fields innerhalb des Bereichs gezaehlt
        ScSubTotalParam aParam = rSubTotalParam;
        ScRange aDBRange;
        pData->GetArea(aDBRange);
        USHORT nFieldStart = aDBRange.aStart.Col();
        for (USHORT i=0; i<MAXSUBTOTAL; i++)
        {
            if ( aParam.bGroupActive[i] )
            {
                aParam.nField[i] += nFieldStart;
                for (USHORT j=0; j<aParam.nSubTotals[i]; j++)
                    aParam.pSubTotals[i][j] += nFieldStart;
            }
        }

        ScDBData aNewData( *pData );
        aNewData.SetSubTotalParam(aParam);
        ScDBDocFunc aFunc(*pDocShell);
        aFunc.ModifyDBData(aNewData, TRUE);
    }
}

uno::Reference<sheet::XSubTotalDescriptor> SAL_CALL ScDatabaseRangeObj::getSubTotalDescriptor()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScRangeSubTotalDescriptor(this);
}

void ScDatabaseRangeObj::GetImportParam(ScImportParam& rImportParam) const
{
    const ScDBData* pData = GetDBData_Impl();
    if (pData)
    {
        pData->GetImportParam(rImportParam);
        //  Fields gibt's hier nicht anzupassen
    }
}

void ScDatabaseRangeObj::SetImportParam(const ScImportParam& rImportParam)
{
    const ScDBData* pData = GetDBData_Impl();
    if (pData)
    {
        //  Fields gibt's hier nicht anzupassen

        ScDBData aNewData( *pData );
        aNewData.SetImportParam(rImportParam);
        ScDBDocFunc aFunc(*pDocShell);
        aFunc.ModifyDBData(aNewData, TRUE);
    }
}

uno::Sequence<beans::PropertyValue> SAL_CALL ScDatabaseRangeObj::getImportDescriptor()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScImportParam aParam;
    const ScDBData* pData = GetDBData_Impl();
    if (pData)
        pData->GetImportParam(aParam);

    uno::Sequence<beans::PropertyValue> aSeq( ScImportDescriptor::GetPropertyCount() );
    ScImportDescriptor::FillProperties( aSeq, aParam );
    return aSeq;
}

void SAL_CALL ScDatabaseRangeObj::refresh() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDBData* pData = GetDBData_Impl();
    if ( pDocShell && pData )
    {
        ScDBDocFunc aFunc(*pDocShell);

        // Import zu wiederholen?
        BOOL bContinue = TRUE;
        ScImportParam aImportParam;
        pData->GetImportParam( aImportParam );
        if (aImportParam.bImport && !pData->HasImportSelection())
        {
            USHORT nTab, nDummy;
            pData->GetArea( nTab, nDummy,nDummy,nDummy,nDummy );
            bContinue = aFunc.DoImport( nTab, aImportParam, NULL, TRUE, FALSE );    //! Api-Flag als Parameter
        }

        // interne Operationen (sort, query, subtotal) nur, wenn kein Fehler
        if (bContinue)
            aFunc.RepeatDB( pData->GetName(), TRUE, TRUE );
    }
}

// XCellRangeSource

uno::Reference<table::XCellRange> SAL_CALL ScDatabaseRangeObj::getReferredCells()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScRange aRange;
    ScDBData* pData = GetDBData_Impl();
    if ( pData )
    {
        //! static Funktion um ScCellObj/ScCellRangeObj zu erzeugen am ScCellRangeObj ???

        pData->GetArea(aRange);
        if ( aRange.aStart == aRange.aEnd )
            return new ScCellObj( pDocShell, aRange.aStart );
        else
            return new ScCellRangeObj( pDocShell, aRange );
    }
    return NULL;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDatabaseRangeObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScDatabaseRangeObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDBData* pData = GetDBData_Impl();
    if ( pDocShell && pData )
    {
        ScDBData aNewData( *pData );
        BOOL bDo = TRUE;

        String aString = aPropertyName;
        if ( aString.EqualsAscii( SC_UNONAME_KEEPFORM ) )
            aNewData.SetKeepFmt( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNONAME_MOVCELLS ) )
            aNewData.SetDoSize( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNONAME_STRIPDAT ) )
            aNewData.SetStripData( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else
            bDo = FALSE;

        if (bDo)
        {
            ScDBDocFunc aFunc(*pDocShell);
            aFunc.ModifyDBData(aNewData, TRUE);
        }
    }
}

uno::Any SAL_CALL ScDatabaseRangeObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Any aRet;
    ScDBData* pData = GetDBData_Impl();
    if ( pData )
    {
        String aString = aPropertyName;
        if ( aString.EqualsAscii( SC_UNONAME_KEEPFORM ) )
            ScUnoHelpFunctions::SetBoolInAny( aRet, pData->IsKeepFmt() );
        else if ( aString.EqualsAscii( SC_UNONAME_MOVCELLS ) )
            ScUnoHelpFunctions::SetBoolInAny( aRet, pData->IsDoSize() );
        else if ( aString.EqualsAscii( SC_UNONAME_STRIPDAT ) )
            ScUnoHelpFunctions::SetBoolInAny( aRet, pData->IsStripData() );
        else if ( aString.EqualsAscii( SC_UNO_LINKDISPBIT ) )
        {
            //  no target bitmaps for individual entries (would be all equal)
            // ScLinkTargetTypeObj::SetLinkTargetBitmap( aRet, SC_LINKTARGETTYPE_DBAREA );
        }
        else if ( aString.EqualsAscii( SC_UNO_LINKDISPNAME ) )
            aRet <<= rtl::OUString( aName );
    }
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDatabaseRangeObj )

// XServiceInfo

rtl::OUString SAL_CALL ScDatabaseRangeObj::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString::createFromAscii( "ScDatabaseRangeObj" );
}

sal_Bool SAL_CALL ScDatabaseRangeObj::supportsService( const rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr( rServiceName );
    return aServiceStr.EqualsAscii( SCDATABASERANGEOBJ_SERVICE ) ||
           aServiceStr.EqualsAscii( SCLINKTARGET_SERVICE );
}

uno::Sequence<rtl::OUString> SAL_CALL ScDatabaseRangeObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(2);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString::createFromAscii( SCDATABASERANGEOBJ_SERVICE );
    pArray[1] = rtl::OUString::createFromAscii( SCLINKTARGET_SERVICE );
    return aRet;
}

//------------------------------------------------------------------------

ScDatabaseRangesObj::ScDatabaseRangesObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScDatabaseRangesObj::~ScDatabaseRangesObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScDatabaseRangesObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //  Referenz-Update interessiert hier nicht

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

// XDatabaseRanges

ScDatabaseRangeObj* ScDatabaseRangesObj::GetObjectByIndex_Impl(USHORT nIndex)
{
    if (pDocShell)
    {
        ScDBCollection* pNames = pDocShell->GetDocument()->GetDBCollection();
        if (pNames && nIndex < pNames->GetCount())
            return new ScDatabaseRangeObj( pDocShell, (*pNames)[nIndex]->GetName() );
    }
    return NULL;
}

ScDatabaseRangeObj* ScDatabaseRangesObj::GetObjectByName_Impl(const rtl::OUString& aName)
{
    if ( pDocShell && hasByName(aName) )
    {
        String aString = aName;
        return new ScDatabaseRangeObj( pDocShell, aString );
    }
    return NULL;
}


void SAL_CALL ScDatabaseRangesObj::addNewByName( const rtl::OUString& aName,
                                        const table::CellRangeAddress& aRange )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    BOOL bDone = FALSE;
    if (pDocShell)
    {
        ScDBDocFunc aFunc(*pDocShell);

        String aString = aName;
        ScRange aNameRange( aRange.StartColumn, aRange.StartRow, aRange.Sheet,
                            aRange.EndColumn,   aRange.EndRow,   aRange.Sheet );
        bDone = aFunc.AddDBRange( aString, aNameRange, TRUE );
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScDatabaseRangesObj::removeByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    BOOL bDone = FALSE;
    if (pDocShell)
    {
        ScDBDocFunc aFunc(*pDocShell);
        String aString = aName;
        bDone = aFunc.DeleteDBRange( aString, TRUE );
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScDatabaseRangesObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this);
}

// XIndexAccess

sal_Int32 SAL_CALL ScDatabaseRangesObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //! "unbenannt" weglassen ?

    if (pDocShell)
    {
        ScDBCollection* pNames = pDocShell->GetDocument()->GetDBCollection();
        if (pNames)
            return pNames->GetCount();
    }

    return NULL;
}

uno::Any SAL_CALL ScDatabaseRangesObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<sheet::XDatabaseRange> xRange = GetObjectByIndex_Impl(nIndex);
    uno::Any aAny;
    if (xRange.is())
        aAny <<= xRange;
    else
        throw lang::IndexOutOfBoundsException();
    return aAny;
}

uno::Type SAL_CALL ScDatabaseRangesObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<sheet::XDatabaseRange>*)0);
}

sal_Bool SAL_CALL ScDatabaseRangesObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

// XNameAccess

uno::Any SAL_CALL ScDatabaseRangesObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<sheet::XDatabaseRange> xRange = GetObjectByName_Impl(aName);
    uno::Any aAny;
    if (xRange.is())
        aAny <<= xRange;
    else
        throw container::NoSuchElementException();
    return aAny;
}

uno::Sequence<rtl::OUString> SAL_CALL ScDatabaseRangesObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //! "unbenannt" weglassen ?

    if (pDocShell)
    {
        ScDBCollection* pNames = pDocShell->GetDocument()->GetDBCollection();
        if (pNames)
        {
            USHORT nCount = pNames->GetCount();
            String aName;
            uno::Sequence<rtl::OUString> aSeq(nCount);
            rtl::OUString* pAry = aSeq.getArray();
            for (USHORT i=0; i<nCount; i++)
                pAry[i] = (*pNames)[i]->GetName();

            return aSeq;
        }
    }
    return uno::Sequence<rtl::OUString>(0);
}

sal_Bool SAL_CALL ScDatabaseRangesObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //! "unbenannt" weglassen ?

    if (pDocShell)
    {
        ScDBCollection* pNames = pDocShell->GetDocument()->GetDBCollection();
        if (pNames)
        {
            String aString = aName;
            USHORT nPos = 0;
            if (pNames->SearchName( aString, nPos ))
                return TRUE;
        }
    }
    return FALSE;
}

//------------------------------------------------------------------------





