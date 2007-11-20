/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dapiuno.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 17:42:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include <svtools/smplhint.hxx>
#include <rtl/uuid.h>

#include "dapiuno.hxx"
#include "datauno.hxx"
#include "miscuno.hxx"
#include "docsh.hxx"
#include "pivot.hxx"
#include "rangeutl.hxx"
#include "unoguard.hxx"
#include "dpobject.hxx"
#include "dpshttab.hxx"
#include "dpsave.hxx"
#include "dbdocfun.hxx"
#include "unonames.hxx"
#ifndef SC_DPGROUP_HXX
#include "dpgroup.hxx"
#endif
#ifndef SC_DPDIMSAVE_HXX
#include "dpdimsave.hxx"
#endif

#ifndef _COM_SUN_STAR_SHEET_XHIERARCHIESSUPPLIER_HPP_
#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XLEVELSSUPPLIER_HPP_
#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XMEMBERSSUPPLIER_HPP_
#include <com/sun/star/sheet/XMembersSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDGROUPBY_HPP_
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#endif

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

using namespace com::sun::star;


//------------------------------------------------------------------------

const SfxItemPropertyMap* lcl_GetDataPilotDescriptorBaseMap()
{
    static SfxItemPropertyMap aDataPilotDescriptorBaseMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNO_COLGRAND), 0,  &getBooleanCppuType(),  0, 0 },
        {MAP_CHAR_LEN(SC_UNO_DRILLDOWN),0,  &getBooleanCppuType(),  0, 0 },
        {MAP_CHAR_LEN(SC_UNO_IGNEMPROWS),0, &getBooleanCppuType(),  0, 0 },
        {MAP_CHAR_LEN(SC_UNO_RPTEMPTY), 0,  &getBooleanCppuType(),  0, 0 },
        {MAP_CHAR_LEN(SC_UNO_ROWGRAND), 0,  &getBooleanCppuType(),  0, 0 },
        {MAP_CHAR_LEN(SC_UNO_SHOWFILT), 0,  &getBooleanCppuType(),  0, 0 },
        {0,0,0,0,0,0}
    };
    return aDataPilotDescriptorBaseMap_Impl;
}

//------------------------------------------------------------------------

const SfxItemPropertyMap* lcl_GetDataPilotFieldMap()
{
    static SfxItemPropertyMap aDataPilotFieldMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_AUTOSHOW),     0,  &getCppuType((sheet::DataPilotFieldAutoShowInfo*)0),0 | beans::PropertyAttribute::MAYBEVOID, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_FUNCTION),     0,  &getCppuType((sheet::GeneralFunction*)0),           0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_GROUPINFO),    0,  &getCppuType((sheet::DataPilotFieldGroupInfo*)0),   0 | beans::PropertyAttribute::MAYBEVOID, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_HASAUTOSHOW),  0,  &getBooleanCppuType(),                              0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_HASLAYOUTINFO),0,  &getBooleanCppuType(),                              0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_HASREFERENCE), 0,  &getBooleanCppuType(),                              0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_HASSORTINFO),  0,  &getBooleanCppuType(),                              0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ISGROUP),      0,  &getBooleanCppuType(),                              0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LAYOUTINFO),   0,  &getCppuType((sheet::DataPilotFieldLayoutInfo*)0),  0 | beans::PropertyAttribute::MAYBEVOID, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ORIENT),       0,  &getCppuType((sheet::DataPilotFieldOrientation*)0), 0 | beans::PropertyAttribute::MAYBEVOID, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_REFERENCE),    0,  &getCppuType((sheet::DataPilotFieldReference*)0),   0 | beans::PropertyAttribute::MAYBEVOID, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SELPAGE),      0,  &getCppuType((rtl::OUString*)0),                    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SHOWEMPTY),    0,  &getBooleanCppuType(),                              0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SORTINFO),     0,  &getCppuType((sheet::DataPilotFieldSortInfo*)0),    0 | beans::PropertyAttribute::MAYBEVOID, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_USESELPAGE),   0,  &getBooleanCppuType(),                              0, 0 },
        {0,0,0,0,0,0}
    };
    return aDataPilotFieldMap_Impl;
}

const SfxItemPropertyMap* lcl_GetDataPilotItemMap()
{
    static SfxItemPropertyMap aDataPilotItemMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_ISHIDDEN),     0,  &getBooleanCppuType(),                              0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_POS),          0,  &getCppuType((sal_Int32*)0),                        0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SHOWDETAIL),   0,  &getBooleanCppuType(),                              0, 0 },
        {0,0,0,0,0,0}
    };
    return aDataPilotItemMap_Impl;
}

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScDataPilotDescriptor, "ScDataPilotDescriptor", "stardiv::one::sheet::DataPilotDescriptor" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotFieldObj, "ScDataPilotFieldObj", "com.sun.star.sheet.DataPilotField" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotFieldsObj, "ScDataPilotFieldsObj", "com.sun.star.sheet.DataPilotFields" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotTableObj, "ScDataPilotTableObj", "com.sun.star.sheet.DataPilotTable" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotTablesObj, "ScDataPilotTablesObj", "com.sun.star.sheet.DataPilotTables" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotItemsObj, "ScDataPilotItemsObj", "com.sun.star.sheet.DataPilotItems" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotItemObj, "ScDataPilotItemObj", "com.sun.star.sheet.DataPilotItem" )

SC_SIMPLE_SERVICE_INFO( ScDataPilotFieldGroupsObj, "ScDataPilotFieldGroupsObj", "com.sun.star.sheet.DataPilotFieldGroups" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotFieldGroupObj, "ScDataPilotFieldGroupObj", "com.sun.star.sheet.DataPilotFieldGroup" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotFieldGroupItemObj, "ScDataPilotFieldGroupItemObj", "com.sun.star.sheet.DataPilotFieldGroupItem" )

//------------------------------------------------------------------------

//! irgendwann ueberall die neuen enum-Werte benutzen
#define DATA_PILOT_HIDDEN   sheet::DataPilotFieldOrientation_HIDDEN
#define DATA_PILOT_COLUMN   sheet::DataPilotFieldOrientation_COLUMN
#define DATA_PILOT_ROW      sheet::DataPilotFieldOrientation_ROW
#define DATA_PILOT_PAGE     sheet::DataPilotFieldOrientation_PAGE
#define DATA_PILOT_DATA     sheet::DataPilotFieldOrientation_DATA

//------------------------------------------------------------------------

// name that is used in the API for the data layout field
#define SC_DATALAYOUT_NAME  "Data"

//------------------------------------------------------------------------

sheet::GeneralFunction ScDataPilotConversion::FirstFunc( USHORT nBits )
{
    if ( nBits & PIVOT_FUNC_SUM )       return sheet::GeneralFunction_SUM;
    if ( nBits & PIVOT_FUNC_COUNT )     return sheet::GeneralFunction_COUNT;
    if ( nBits & PIVOT_FUNC_AVERAGE )   return sheet::GeneralFunction_AVERAGE;
    if ( nBits & PIVOT_FUNC_MAX )       return sheet::GeneralFunction_MAX;
    if ( nBits & PIVOT_FUNC_MIN )       return sheet::GeneralFunction_MIN;
    if ( nBits & PIVOT_FUNC_PRODUCT )   return sheet::GeneralFunction_PRODUCT;
    if ( nBits & PIVOT_FUNC_COUNT_NUM ) return sheet::GeneralFunction_COUNTNUMS;
    if ( nBits & PIVOT_FUNC_STD_DEV )   return sheet::GeneralFunction_STDEV;
    if ( nBits & PIVOT_FUNC_STD_DEVP )  return sheet::GeneralFunction_STDEVP;
    if ( nBits & PIVOT_FUNC_STD_VAR )   return sheet::GeneralFunction_VAR;
    if ( nBits & PIVOT_FUNC_STD_VARP )  return sheet::GeneralFunction_VARP;
    if ( nBits & PIVOT_FUNC_AUTO )      return sheet::GeneralFunction_AUTO;
    return sheet::GeneralFunction_NONE;
}

USHORT ScDataPilotConversion::FunctionBit( sheet::GeneralFunction eFunc )
{
    USHORT nRet = PIVOT_FUNC_NONE;  // 0
    switch (eFunc)
    {
        case sheet::GeneralFunction_SUM:        nRet = PIVOT_FUNC_SUM;       break;
        case sheet::GeneralFunction_COUNT:      nRet = PIVOT_FUNC_COUNT;     break;
        case sheet::GeneralFunction_AVERAGE:    nRet = PIVOT_FUNC_AVERAGE;   break;
        case sheet::GeneralFunction_MAX:        nRet = PIVOT_FUNC_MAX;       break;
        case sheet::GeneralFunction_MIN:        nRet = PIVOT_FUNC_MIN;       break;
        case sheet::GeneralFunction_PRODUCT:    nRet = PIVOT_FUNC_PRODUCT;   break;
        case sheet::GeneralFunction_COUNTNUMS:  nRet = PIVOT_FUNC_COUNT_NUM; break;
        case sheet::GeneralFunction_STDEV:      nRet = PIVOT_FUNC_STD_DEV;   break;
        case sheet::GeneralFunction_STDEVP:     nRet = PIVOT_FUNC_STD_DEVP;  break;
        case sheet::GeneralFunction_VAR:        nRet = PIVOT_FUNC_STD_VAR;   break;
        case sheet::GeneralFunction_VARP:       nRet = PIVOT_FUNC_STD_VARP;  break;
        case sheet::GeneralFunction_AUTO:       nRet = PIVOT_FUNC_AUTO;      break;
        default:
        {
            // added to avoid warnings
        }
    }
    return nRet;
}

//------------------------------------------------------------------------

ScDPObject* lcl_GetDPObject( ScDocShell* pDocShell, SCTAB nTab, const String& rName )
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
        {
            USHORT nCount = pColl->GetCount();
            for (USHORT i=0; i<nCount; i++)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->IsSheetData() &&
                     pDPObj->GetOutRange().aStart.Tab() == nTab &&
                     pDPObj->GetName() == rName )
                    return pDPObj;
            }
        }
    }
    return NULL;    // nicht gefunden
}

String lcl_ColumnTitle( ScDocument* pDoc, USHORT nCol, USHORT nRow, USHORT nTab )
{
    //  Spaltennamen, wie sie in der Pivottabelle angezeigt werden
    String aStr;
    pDoc->GetString(nCol, nRow, nTab, aStr);
    if (aStr.Len() == 0)
        aStr = ColToAlpha( nCol );
    return aStr;
}

String lcl_CreatePivotName( ScDocShell* pDocShell )
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
            return pColl->CreateNewName();
    }
    return String();                    // sollte nicht vorkommen
}

sal_Int32 lcl_GetObjectIndex( ScDPObject* pDPObj, const ScFieldIdentifier& rIdent )
{
    // used for items - nRepeat in identifier can be ignored

    if ( pDPObj )
    {
        sal_Int32 nCount = pDPObj->GetDimCount();
        for ( sal_Int32 nDim = 0; nDim < nCount; nDim++ )
        {
            BOOL bIsDataLayout = FALSE;
            String aDimName( pDPObj->GetDimName( nDim, bIsDataLayout ) );
            if ( rIdent.bDataLayoutField ? bIsDataLayout : ( aDimName == rIdent.sFieldName ) )
                return nDim;
        }
    }
    return -1;  // none
}

BOOL lcl_GetMembers( ScDataPilotDescriptorBase* pParent, const ScFieldIdentifier& rIdent, uno::Reference<container::XNameAccess>& xMembers )
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    sal_Int32 nObjIndex = lcl_GetObjectIndex( pDPObj, rIdent );
    return pDPObj && pDPObj->GetMembersNA( nObjIndex, xMembers );
}

//------------------------------------------------------------------------

ScDataPilotTablesObj::ScDataPilotTablesObj(ScDocShell* pDocSh, SCTAB nT) :
    pDocShell( pDocSh ),
    nTab( nT )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScDataPilotTablesObj::~ScDataPilotTablesObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScDataPilotTablesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //! Referenz-Update

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

// XDataPilotTables

ScDataPilotTableObj* ScDataPilotTablesObj::GetObjectByIndex_Impl(SCSIZE nIndex)
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
        {
            //  count tables on this sheet
            //  api only handles sheet data at this time
            //! allow all data sources!!!
            SCSIZE nFound = 0;
            USHORT nCount = pColl->GetCount();
            for (USHORT i=0; i<nCount; i++)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->IsSheetData() && pDPObj->GetOutRange().aStart.Tab() == nTab )
                {
                    if ( nFound == nIndex )
                    {
                        String aName = pDPObj->GetName();
                        return new ScDataPilotTableObj( pDocShell, nTab, aName );
                    }
                    ++nFound;
                }
            }
        }
    }
    return NULL;
}

ScDataPilotTableObj* ScDataPilotTablesObj::GetObjectByName_Impl(const rtl::OUString& aName)
{
    if (hasByName(aName))
    {
        String aNamStr(aName);
        return new ScDataPilotTableObj( pDocShell, nTab, aNamStr );
    }
    return NULL;
}

uno::Reference<sheet::XDataPilotDescriptor> SAL_CALL ScDataPilotTablesObj::createDataPilotDescriptor()
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
        return new ScDataPilotDescriptor(pDocShell);
    return NULL;
}

bool lcl_IsDuplicated( const uno::Reference<beans::XPropertySet> xDimProps )
{
    bool bRet = false;

    try
    {
        uno::Any aAny = xDimProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ORIGINAL)));
        uno::Reference<container::XNamed> xOriginal( aAny, uno::UNO_QUERY );
        if ( xOriginal.is() )
            bRet = true;
    }
    catch(uno::Exception&)
    {
    }

    return bRet;
}

rtl::OUString lcl_GetOriginalName( const uno::Reference<container::XNamed> xDim )
{
    uno::Reference<container::XNamed> xOriginal;

    uno::Reference<beans::XPropertySet> xDimProps( xDim, uno::UNO_QUERY );
    if ( xDimProps.is() )
    {
        try
        {
            uno::Any aAny = xDimProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ORIGINAL)));
            aAny >>= xOriginal;
        }
        catch(uno::Exception&)
        {
        }
    }

    if ( !xOriginal.is() )
        xOriginal = xDim;

    return xOriginal->getName();
}

void lcl_SetSaveData(const uno::Reference<container::XIndexAccess>& xFields, ScDPSaveData* pSaveData)
{
    if (xFields.is() && pSaveData)
    {
        sal_Int32 nFieldsCount(xFields->getCount());
        for (sal_Int32 i = 0; i < nFieldsCount; ++i)
        {
            uno::Any aDim = xFields->getByIndex(i);
            uno::Reference<container::XNamed> xDim;
            uno::Reference<beans::XPropertySet> xDimProps;
            if ((aDim >>= xDim) && (aDim >>= xDimProps))
            {
                //rtl::OUString sName(xDim->getName());
                rtl::OUString sName( lcl_GetOriginalName(xDim) );
                ScDPSaveDimension* pDim = sName.getLength() ? pSaveData->GetDimensionByName(sName) : 0;
                if (pDim)
                {
                    uno::Any aAny = xDimProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ORIENT)));
                    sheet::DataPilotFieldOrientation eOrient;
                    if (aAny >>= eOrient)
                        pDim->SetOrientation( sal::static_int_cast<USHORT>( eOrient ) );

                    aAny = xDimProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_FUNCTION)));
                    sheet::GeneralFunction eFunc;
                    if (aAny >>= eFunc)
                        pDim->SetFunction( sal::static_int_cast<USHORT>( eFunc ) );
                }
            }
        }
    }
}

void SAL_CALL ScDataPilotTablesObj::insertNewByName( const rtl::OUString& aNewName,
                                    const table::CellAddress& aOutputAddress,
                                    const uno::Reference<sheet::XDataPilotDescriptor>& xDescriptor )
                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (!xDescriptor.is()) return;

    // inserting with already existing name?
    if ( aNewName.getLength() && hasByName( aNewName ) )
        throw uno::RuntimeException();      // no other exceptions specified

    BOOL bDone = FALSE;
    ScDataPilotDescriptorBase* pImp = ScDataPilotDescriptorBase::getImplementation( xDescriptor );
    if ( pDocShell && pImp )
    {
        ScDPObject* pNewObj = pImp->GetDPObject();

        if (pNewObj)
        {
            ScRange aOutputRange((SCCOL)aOutputAddress.Column, (SCROW)aOutputAddress.Row, (SCTAB)aOutputAddress.Sheet,
                                (SCCOL)aOutputAddress.Column, (SCROW)aOutputAddress.Row, (SCTAB)aOutputAddress.Sheet);
            pNewObj->SetOutRange(aOutputRange);
            String aName = aNewName;
            if (!aName.Len())
                aName = lcl_CreatePivotName( pDocShell );
            pNewObj->SetName(aName);
            String aTag = xDescriptor->getTag();
            pNewObj->SetTag(aTag);

    // todo: handle double fields (for more information see ScDPObject

            ScDBDocFunc aFunc(*pDocShell);
            bDone = aFunc.DataPilotUpdate( NULL, pNewObj, TRUE, TRUE );
        }
    }

    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScDataPilotTablesObj::removeByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameStr(aName);
    ScDPObject* pDPObj = lcl_GetDPObject( pDocShell, nTab, aNameStr );
    if (pDPObj && pDocShell)
    {
        ScDBDocFunc aFunc(*pDocShell);
        aFunc.DataPilotUpdate( pDPObj, NULL, TRUE, TRUE );  // remove - incl. undo etc.
    }
    else
        throw uno::RuntimeException();      // no other exceptions specified
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScDataPilotTablesObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.DataPilotTablesEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScDataPilotTablesObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
        {
            //  count tables on this sheet
            //  api only handles sheet data at this time
            //! allow all data sources!!!

            USHORT nFound = 0;
            USHORT nCount = pColl->GetCount();
            for (USHORT i=0; i<nCount; i++)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->IsSheetData() && pDPObj->GetOutRange().aStart.Tab() == nTab )
                    ++nFound;
            }
            return nFound;
        }
    }

    return 0;
}

uno::Any SAL_CALL ScDataPilotTablesObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<sheet::XDataPilotTable> xTable(GetObjectByIndex_Impl(static_cast<SCSIZE>(nIndex)));
    if (xTable.is())
        return uno::makeAny(xTable);
    else
        throw lang::IndexOutOfBoundsException();
//    return uno::Any();
}

uno::Type SAL_CALL ScDataPilotTablesObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<sheet::XDataPilotTable>*)0);
}

sal_Bool SAL_CALL ScDataPilotTablesObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

// XNameAccess

uno::Any SAL_CALL ScDataPilotTablesObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<sheet::XDataPilotTable> xTable(GetObjectByName_Impl(aName));
    if (xTable.is())
        return uno::makeAny(xTable);
    else
        throw container::NoSuchElementException();
//    return uno::Any();
}

uno::Sequence<rtl::OUString> SAL_CALL ScDataPilotTablesObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
        {
            //  count tables on this sheet
            //  api only handles sheet data at this time
            //! allow all data sources!!!

            USHORT nFound = 0;
            USHORT nCount = pColl->GetCount();
            USHORT i;
            for (i=0; i<nCount; i++)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->IsSheetData() && pDPObj->GetOutRange().aStart.Tab() == nTab )
                    ++nFound;
            }

            USHORT nPos = 0;
            uno::Sequence<rtl::OUString> aSeq(nFound);
            rtl::OUString* pAry = aSeq.getArray();
            for (i=0; i<nCount; i++)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->IsSheetData() && pDPObj->GetOutRange().aStart.Tab() == nTab )
                    pAry[nPos++] = pDPObj->GetName();
            }

            return aSeq;
        }
    }
    return uno::Sequence<rtl::OUString>(0);
}

sal_Bool SAL_CALL ScDataPilotTablesObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
        {
            String aNamStr(aName);
            USHORT nCount = pColl->GetCount();
            for (USHORT i=0; i<nCount; i++)
            {
                //  api only handles sheet data at this time
                //! allow all data sources!!!

                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->IsSheetData() &&
                     pDPObj->GetOutRange().aStart.Tab() == nTab &&
                     pDPObj->GetName() == aNamStr )
                    return TRUE;
            }
        }
    }
    return FALSE;
}

//------------------------------------------------------------------------

ScDataPilotDescriptorBase::ScDataPilotDescriptorBase(ScDocShell* pDocSh) :
    aPropSet( lcl_GetDataPilotDescriptorBaseMap() ),
    pDocShell( pDocSh )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScDataPilotDescriptorBase::~ScDataPilotDescriptorBase()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

uno::Any SAL_CALL ScDataPilotDescriptorBase::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( sheet::XDataPilotDescriptor )
    SC_QUERYINTERFACE( beans::XPropertySet )
    SC_QUERYINTERFACE( container::XNamed )                  // base of XDataPilotDescriptor
    SC_QUERYINTERFACE( lang::XUnoTunnel )
    SC_QUERYINTERFACE( lang::XTypeProvider )
    SC_QUERYINTERFACE( lang::XServiceInfo )

    return OWeakObject::queryInterface( rType );
}

void SAL_CALL ScDataPilotDescriptorBase::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL ScDataPilotDescriptorBase::release() throw()
{
    OWeakObject::release();
}

uno::Sequence<uno::Type> SAL_CALL ScDataPilotDescriptorBase::getTypes()
                                                    throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        aTypes.realloc(5);
        uno::Type* pPtr = aTypes.getArray();
        pPtr[0] = getCppuType((const uno::Reference<sheet::XDataPilotDescriptor>*)0);
        pPtr[1] = getCppuType((const uno::Reference<beans::XPropertySet>*)0);
        pPtr[2] = getCppuType((const uno::Reference<lang::XUnoTunnel>*)0);
        pPtr[3] = getCppuType((const uno::Reference<lang::XTypeProvider>*)0);
        pPtr[4] = getCppuType((const uno::Reference<lang::XServiceInfo>*)0);
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScDataPilotDescriptorBase::getImplementationId()
                                                    throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

void ScDataPilotDescriptorBase::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //! Referenz-Update?

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

// XDataPilotDescriptor

table::CellRangeAddress SAL_CALL ScDataPilotDescriptorBase::getSourceRange()
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    table::CellRangeAddress aRet;

    ScDPObject* pDPObject(GetDPObject());
    if (pDPObject && pDPObject->IsSheetData())
    {
        const ScSheetSourceDesc* pSheetDesc = pDPObject->GetSheetDesc();
        aRet.StartColumn = pSheetDesc->aSourceRange.aStart.Col();
        aRet.EndColumn = pSheetDesc->aSourceRange.aEnd.Col();
        aRet.StartRow = pSheetDesc->aSourceRange.aStart.Row();
        aRet.EndRow = pSheetDesc->aSourceRange.aEnd.Row();
        aRet.Sheet = pSheetDesc->aSourceRange.aStart.Tab();
    }
    else
        throw uno::RuntimeException();

    return aRet;
}

void SAL_CALL ScDataPilotDescriptorBase::setSourceRange(
                                const table::CellRangeAddress& aSourceRange )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    ScDPObject* pDPObject = GetDPObject();
    if (pDPObject)
    {
        ScSheetSourceDesc aSheetDesc;
        if (pDPObject->IsSheetData())
            aSheetDesc = *(pDPObject->GetSheetDesc());
        aSheetDesc.aSourceRange.aStart.SetCol((SCCOL)aSourceRange.StartColumn);
        aSheetDesc.aSourceRange.aEnd.SetCol((SCCOL)aSourceRange.EndColumn);
        aSheetDesc.aSourceRange.aStart.SetRow((SCROW)aSourceRange.StartRow);
        aSheetDesc.aSourceRange.aEnd.SetRow((SCROW)aSourceRange.EndRow);
        aSheetDesc.aSourceRange.aStart.SetTab((SCTAB)aSourceRange.Sheet);
        pDPObject->SetSheetDesc(aSheetDesc);
        SetDPObject(pDPObject);
    }
    else
        throw uno::RuntimeException();
}

uno::Reference<sheet::XSheetFilterDescriptor> SAL_CALL ScDataPilotDescriptorBase::getFilterDescriptor()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScDataPilotFilterDescriptor( pDocShell, this );
}

uno::Reference<container::XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getDataPilotFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScDataPilotFieldsObj( this, SC_FIELDORIENT_ALL );
}

uno::Reference<container::XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getColumnFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScDataPilotFieldsObj( this, DATA_PILOT_COLUMN );
}

uno::Reference<container::XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getRowFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScDataPilotFieldsObj( this, DATA_PILOT_ROW );
}

uno::Reference<container::XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getPageFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScDataPilotFieldsObj( this, DATA_PILOT_PAGE );
}

uno::Reference<container::XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getDataFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScDataPilotFieldsObj( this, DATA_PILOT_DATA );
}

uno::Reference<container::XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getHiddenFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScDataPilotFieldsObj( this, DATA_PILOT_HIDDEN );
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL ScDataPilotDescriptorBase::getPropertySetInfo(  )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScDataPilotDescriptorBase::setPropertyValue( const ::rtl::OUString& aPropertyName,
                                const ::com::sun::star::uno::Any& aValue )
                                                throw(beans::UnknownPropertyException,
                                                    beans::PropertyVetoException,
                                                    lang::IllegalArgumentException,
                                                    lang::WrappedTargetException,
                                                    uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDPObject* pDPObject = GetDPObject();
    if (pDPObject)
    {
        ScDPSaveData* pOldData = pDPObject->GetSaveData();
        DBG_ASSERT(pOldData, "Here should be a SaveData");
        if ( pOldData )
        {
            ScDPSaveData aNewData( *pOldData );

            String aNameString = aPropertyName;
            if ( aNameString.EqualsAscii( SC_UNO_COLGRAND ) )
            {
                aNewData.SetColumnGrand(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.EqualsAscii( SC_UNO_IGNEMPROWS ) )
            {
                aNewData.SetIgnoreEmptyRows(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.EqualsAscii( SC_UNO_RPTEMPTY ) )
            {
                aNewData.SetRepeatIfEmpty(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.EqualsAscii( SC_UNO_ROWGRAND ) )
            {
                aNewData.SetRowGrand(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.EqualsAscii( SC_UNO_SHOWFILT ) )
            {
                aNewData.SetFilterButton(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.EqualsAscii( SC_UNO_DRILLDOWN ) )
            {
                aNewData.SetDrillDown(::cppu::any2bool( aValue ));
            }
            else
                throw beans::UnknownPropertyException();

            pDPObject->SetSaveData( aNewData );
        }

        SetDPObject(pDPObject);
    }
}

uno::Any SAL_CALL ScDataPilotDescriptorBase::getPropertyValue(const ::rtl::OUString& aPropertyName )
                                                throw(beans::UnknownPropertyException,
                                                    lang::WrappedTargetException,
                                                    uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Any aRet;

    ScDPObject* pDPObject(GetDPObject());
    if (pDPObject)
    {
        ScDPSaveData* pOldData = pDPObject->GetSaveData();
        DBG_ASSERT(pOldData, "Here should be a SaveData");
        if ( pOldData )
        {
            ScDPSaveData aNewData( *pOldData );

            String aNameString = aPropertyName;
            if ( aNameString.EqualsAscii( SC_UNO_COLGRAND ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetColumnGrand() );
            }
            else if ( aNameString.EqualsAscii( SC_UNO_IGNEMPROWS ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetIgnoreEmptyRows() );
            }
            else if ( aNameString.EqualsAscii( SC_UNO_RPTEMPTY ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetRepeatIfEmpty() );
            }
            else if ( aNameString.EqualsAscii( SC_UNO_ROWGRAND ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetRowGrand() );
            }
            else if ( aNameString.EqualsAscii( SC_UNO_SHOWFILT ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetFilterButton() );
            }
            else if ( aNameString.EqualsAscii( SC_UNO_DRILLDOWN ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetDrillDown() );
            }
            else
                throw beans::UnknownPropertyException();
        }
    }

    return aRet;
}

void SAL_CALL ScDataPilotDescriptorBase::addPropertyChangeListener( const ::rtl::OUString& /* aPropertyName */,
                                const uno::Reference<beans::XPropertyChangeListener >& /* xListener */ )
                                                throw(beans::UnknownPropertyException,
                                                    lang::WrappedTargetException,
                                                    uno::RuntimeException)
{
}

void SAL_CALL ScDataPilotDescriptorBase::removePropertyChangeListener( const ::rtl::OUString& /* aPropertyName */,
                                const uno::Reference<beans::XPropertyChangeListener >& /* aListener */ )
                                                throw(beans::UnknownPropertyException,
                                                    lang::WrappedTargetException,
                                                    uno::RuntimeException)
{
}

void SAL_CALL ScDataPilotDescriptorBase::addVetoableChangeListener( const ::rtl::OUString& /* PropertyName */,
                                const uno::Reference<beans::XVetoableChangeListener >& /* aListener */ )
                                                throw(beans::UnknownPropertyException,
                                                    lang::WrappedTargetException,
                                                    uno::RuntimeException)
{
}

void SAL_CALL ScDataPilotDescriptorBase::removeVetoableChangeListener( const ::rtl::OUString& /* PropertyName */,
                                const uno::Reference<beans::XVetoableChangeListener >& /* aListener */ )
                                                throw(beans::UnknownPropertyException,
                                                    lang::WrappedTargetException,
                                                    uno::RuntimeException)
{
}

// XUnoTunnel

sal_Int64 SAL_CALL ScDataPilotDescriptorBase::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return 0;
}

// static
const uno::Sequence<sal_Int8>& ScDataPilotDescriptorBase::getUnoTunnelId()
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
ScDataPilotDescriptorBase* ScDataPilotDescriptorBase::getImplementation(
                                const uno::Reference<sheet::XDataPilotDescriptor> xObj )
{
    ScDataPilotDescriptorBase* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = reinterpret_cast<ScDataPilotDescriptorBase*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

//------------------------------------------------------------------------

ScDataPilotTableObj::ScDataPilotTableObj(ScDocShell* pDocSh, SCTAB nT, const String& rN) :
    ScDataPilotDescriptorBase( pDocSh ),
    nTab( nT ),
    aName( rN )
{
}

ScDataPilotTableObj::~ScDataPilotTableObj()
{
}

uno::Any SAL_CALL ScDataPilotTableObj::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( sheet::XDataPilotTable )

    return ScDataPilotDescriptorBase::queryInterface( rType );
}

void SAL_CALL ScDataPilotTableObj::acquire() throw()
{
    ScDataPilotDescriptorBase::acquire();
}

void SAL_CALL ScDataPilotTableObj::release() throw()
{
    ScDataPilotDescriptorBase::release();
}

uno::Sequence<uno::Type> SAL_CALL ScDataPilotTableObj::getTypes()
                                                    throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes(ScDataPilotDescriptorBase::getTypes());
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        aTypes.realloc( nParentLen + 1 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = getCppuType((const uno::Reference<sheet::XDataPilotTable>*)0);

        for (long i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];                // parent types first
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScDataPilotTableObj::getImplementationId()
                                                    throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// ---
ScDPObject* ScDataPilotTableObj::GetDPObject() const
{
    return lcl_GetDPObject(GetDocShell(), nTab, aName);
}

void ScDataPilotTableObj::SetDPObject( ScDPObject* pDPObject )
{
    ScDocShell* pDocSh = GetDocShell();
    ScDPObject* pDPObj = lcl_GetDPObject(pDocSh, nTab, aName);
    if ( pDPObj && pDocSh )
    {
        ScDBDocFunc aFunc(*pDocSh);
        aFunc.DataPilotUpdate( pDPObj, pDPObject, TRUE, TRUE );
    }
}

// "rest of XDataPilotDescriptor"

rtl::OUString SAL_CALL ScDataPilotTableObj::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
        return pDPObj->GetName();
    return rtl::OUString();
}

void SAL_CALL ScDataPilotTableObj::setName( const rtl::OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
    {
        //! test for existing names !!!

        String aString(aNewName);
        pDPObj->SetName( aString );     //! Undo - DBDocFunc ???
        aName = aString;

        //  DataPilotUpdate would do too much (output table is not changed)
        GetDocShell()->SetDocumentModified();
    }
}

rtl::OUString SAL_CALL ScDataPilotTableObj::getTag() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
        return pDPObj->GetTag();
    return rtl::OUString();
}

void SAL_CALL ScDataPilotTableObj::setTag( const ::rtl::OUString& aNewTag )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
    {
        String aString(aNewTag);
        pDPObj->SetTag( aString );      //! Undo - DBDocFunc ???

        //  DataPilotUpdate would do too much (output table is not changed)
        GetDocShell()->SetDocumentModified();
    }
}

// XDataPilotTable

table::CellRangeAddress SAL_CALL ScDataPilotTableObj::getOutputRange()
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellRangeAddress aRet;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
    {
        ScRange aRange(pDPObj->GetOutRange());
        aRet.Sheet       = aRange.aStart.Tab();
        aRet.StartColumn = aRange.aStart.Col();
        aRet.StartRow    = aRange.aStart.Row();
        aRet.EndColumn   = aRange.aEnd.Col();
        aRet.EndRow      = aRange.aEnd.Row();
    }
    return aRet;
}

void SAL_CALL ScDataPilotTableObj::refresh() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
    {
        ScDPObject* pNew = new ScDPObject(*pDPObj);
        ScDBDocFunc aFunc(*GetDocShell());
        aFunc.DataPilotUpdate( pDPObj, pNew, TRUE, TRUE );
        delete pNew;        // DataPilotUpdate copies settings from "new" object
    }
}

//------------------------------------------------------------------------

ScDataPilotDescriptor::ScDataPilotDescriptor(ScDocShell* pDocSh) :
    ScDataPilotDescriptorBase( pDocSh ),
    mpDPObject(new ScDPObject(pDocSh ? pDocSh->GetDocument() : NULL) )
{
    mpDPObject->SetAlive(sal_True);
    ScDPSaveData aSaveData;
    // set defaults like in ScPivotParam constructor
     aSaveData.SetColumnGrand( sal_True );
    aSaveData.SetRowGrand( sal_True );
    aSaveData.SetIgnoreEmptyRows( sal_False );
    aSaveData.SetRepeatIfEmpty( sal_False );
    mpDPObject->SetSaveData(aSaveData);
    ScSheetSourceDesc aSheetDesc;
    mpDPObject->SetSheetDesc(aSheetDesc);
    mpDPObject->GetSource();
}

ScDataPilotDescriptor::~ScDataPilotDescriptor()
{
    delete mpDPObject;
}

ScDPObject* ScDataPilotDescriptor::GetDPObject() const
{
    return mpDPObject;
}

void ScDataPilotDescriptor::SetDPObject( ScDPObject* pDPObject )
{
    if (mpDPObject != pDPObject)
    {
        delete mpDPObject;
        mpDPObject = pDPObject;
        DBG_ERROR("replace DPObject should not happen");
    }
}

// "rest of XDataPilotDescriptor"

rtl::OUString SAL_CALL ScDataPilotDescriptor::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return mpDPObject->GetName();
}

void SAL_CALL ScDataPilotDescriptor::setName( const rtl::OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    mpDPObject->SetName( aNewName );
}

rtl::OUString SAL_CALL ScDataPilotDescriptor::getTag() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return mpDPObject->GetTag();
}

void SAL_CALL ScDataPilotDescriptor::setTag( const ::rtl::OUString& aNewTag )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    mpDPObject->SetTag( aNewTag );
}

//------------------------------------------------------------------------

ScDataPilotFieldsObj::ScDataPilotFieldsObj(ScDataPilotDescriptorBase* pPar, USHORT nTy) :
    pParent( pPar ),
    nType( nTy )
{
    pParent->acquire();
}

ScDataPilotFieldsObj::~ScDataPilotFieldsObj()
{
    pParent->release();
}

BOOL lcl_GetDim(ScDPObject* pDPObj, const ScFieldIdentifier& rIdent, ScDPSaveDimension*& rpDim)
{
    BOOL bRet = FALSE;
    ScDPSaveData* pSave = pDPObj->GetSaveData();
    if ( pSave )
    {
        if ( rIdent.bDataLayoutField )
        {
            rpDim = pSave->GetDataLayoutDimension();
            bRet = TRUE;
        }
        else if ( rIdent.nRepeat == 0 )
        {
            rpDim = pSave->GetDimensionByName( rIdent.sFieldName );
            bRet = TRUE;
        }
        else
        {
            // count all dimensions with the given name

            String aNameStr( rIdent.sFieldName );
            const List& rDimensions = pSave->GetDimensions();
            sal_Int32 nDimCount = rDimensions.Count();
            sal_Int32 nFound = 0;
            for ( sal_Int32 nDim = 0; nDim < nDimCount && !bRet; nDim++ )
            {
                ScDPSaveDimension* pOneDim = static_cast<ScDPSaveDimension*>(rDimensions.GetObject(nDim));
                if ( !pOneDim->IsDataLayout() && pOneDim->GetName() == aNameStr )
                {
                    if ( nFound == rIdent.nRepeat )
                    {
                        rpDim = pOneDim;
                        bRet = TRUE;
                    }
                    else
                        ++nFound;
                }
            }
        }
    }
    return bRet;
}

SCSIZE lcl_GetFieldCount( const com::sun::star::uno::Reference<com::sun::star::sheet::XDimensionsSupplier>& rSource, USHORT nType )
{
    SCSIZE nRet = 0;

    uno::Reference<container::XNameAccess> xDimsName(rSource->getDimensions());
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    sal_Int32 nIntCount = xIntDims->getCount();
    if (nType != SC_FIELDORIENT_ALL)
    {
        // all fields of the specified orientation, including duplicated

        uno::Reference<beans::XPropertySet> xDim;
        sheet::DataPilotFieldOrientation aOrient;
        for (sal_Int32 i = 0; i < nIntCount; ++i)
        {
            xDim.set(xIntDims->getByIndex(i), uno::UNO_QUERY);
            if (xDim.is())
            {
                xDim->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ORIENTAT))) >>= aOrient;
                if (aOrient == nType)
                    ++nRet;
            }
        }
    }
    else
    {
        // count all non-duplicated fields

        uno::Reference<beans::XPropertySet> xDim;
        for (sal_Int32 i = 0; i < nIntCount; ++i)
        {
            xDim.set(xIntDims->getByIndex(i), uno::UNO_QUERY);
            if ( xDim.is() && !lcl_IsDuplicated( xDim ) )
                ++nRet;
        }
    }

    return nRet;
}

BOOL lcl_GetFieldDataByIndex( const com::sun::star::uno::Reference<com::sun::star::sheet::XDimensionsSupplier>& rSource,
                                USHORT nType, SCSIZE nIndex, ScFieldIdentifier& rField )
{
    BOOL bOk = FALSE;
    SCSIZE nPos = 0;
    sal_Int32 nDimIndex = 0;

    uno::Reference<container::XNameAccess> xDimsName(rSource->getDimensions());
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    sal_Int32 nIntCount = xIntDims->getCount();
    uno::Reference<beans::XPropertySet> xDim;
    if (nType != SC_FIELDORIENT_ALL)
    {
        sheet::DataPilotFieldOrientation aOrient;
        sal_Int32 i = 0;
        while (i < nIntCount && !bOk)
        {
            xDim.set(xIntDims->getByIndex(i), uno::UNO_QUERY);
            if (xDim.is())
            {
                xDim->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ORIENTAT))) >>= aOrient;
                if (aOrient == nType)
                {
                    if (nPos == nIndex)
                    {
                        bOk = sal_True;
                        nDimIndex = i;
                    }
                    else
                        ++nPos;
                }
            }
            ++i;
        }
    }
    else
    {
        sal_Int32 i = 0;
        while (i < nIntCount && !bOk)
        {
            xDim.set(xIntDims->getByIndex(i), uno::UNO_QUERY);
            if ( xDim.is() && !lcl_IsDuplicated( xDim ) )
            {
                if (nPos == nIndex)
                {
                    bOk = sal_True;
                    nDimIndex = i;
                }
                else
                    ++nPos;
            }
            ++i;
        }
    }

    if ( bOk )
    {
        xDim.set( xIntDims->getByIndex(nDimIndex), uno::UNO_QUERY );
        uno::Reference<container::XNamed> xDimName( xDim, uno::UNO_QUERY );
        if ( xDimName.is() )
        {
            rtl::OUString sOriginalName( lcl_GetOriginalName( xDimName ) );
            rField.sFieldName = sOriginalName;
            rField.bDataLayoutField = ScUnoHelpFunctions::GetBoolProperty( xDim,
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ISDATALA)) );

            sal_Int32 nRepeat = 0;
            if ( nType != SC_FIELDORIENT_ALL && lcl_IsDuplicated( xDim ) )
            {
                // find the repeat count
                // (this relies on the original dimension always being before the duplicates)

                uno::Reference<container::XNamed> xPrevName;
                for (sal_Int32 i = 0; i < nDimIndex; ++i)
                {
                    xPrevName.set( xIntDims->getByIndex(i), uno::UNO_QUERY );
                    if ( xPrevName.is() && lcl_GetOriginalName( xPrevName ) == sOriginalName )
                        ++nRepeat;
                }
            }
            rField.nRepeat = nRepeat;
        }
        else
            bOk = sal_False;
    }

    return bOk;
}

BOOL lcl_GetFieldDataByName( ScDPObject* pDPObj, USHORT /* nType */, const rtl::OUString& sName, ScFieldIdentifier& rField )
{
    // "By name" is always the first match.
    // The name "Data" always refers to the data layout field.
    rField.sFieldName = sName;
    rField.bDataLayoutField = sName.equalsAscii( SC_DATALAYOUT_NAME );
    rField.nRepeat = 0;

    pDPObj->GetSource();    // IsDimNameInUse doesn't update source data

    // check if the named field exists (not for data layout)
    return ( rField.bDataLayoutField || pDPObj->IsDimNameInUse( sName ) );
}

// XDataPilotFields

ScDataPilotFieldObj* ScDataPilotFieldsObj::GetObjectByIndex_Impl(SCSIZE nIndex) const
{
    ScDPObject* pObj = pParent->GetDPObject();

// TODO
    if (pObj)
    {
        ScFieldIdentifier aSourceIdent;
        BOOL bOk = lcl_GetFieldDataByIndex( pObj->GetSource(), nType, nIndex, aSourceIdent );

        if (bOk)
            return new ScDataPilotFieldObj( pParent, nType, aSourceIdent );
    }

    return NULL;
}

ScDataPilotFieldObj* ScDataPilotFieldsObj::GetObjectByName_Impl(const rtl::OUString& aName) const
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScFieldIdentifier aSourceIdent;
        if (lcl_GetFieldDataByName( pDPObj, nType, aName, aSourceIdent ))
        {
            return new ScDataPilotFieldObj( pParent, nType, aSourceIdent );
        }
    }
    return NULL;
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScDataPilotFieldsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.DataPilotFieldsEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScDataPilotFieldsObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
// TODO
    ScDPObject* pDPObj(pParent->GetDPObject());

    return pDPObj ? static_cast<sal_Int32>(lcl_GetFieldCount( pDPObj->GetSource(), nType )) : 0;
}

uno::Any SAL_CALL ScDataPilotFieldsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<beans::XPropertySet> xField(GetObjectByIndex_Impl(static_cast<SCSIZE>(nIndex)));
    if (xField.is())
        return uno::makeAny(xField);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScDataPilotFieldsObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<beans::XPropertySet>*)0);
}

sal_Bool SAL_CALL ScDataPilotFieldsObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

uno::Any SAL_CALL ScDataPilotFieldsObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<beans::XPropertySet> xField(GetObjectByName_Impl(aName));
    if (xField.is())
        return uno::makeAny(xField);
    else
        throw container::NoSuchElementException();
}

uno::Sequence<rtl::OUString> SAL_CALL ScDataPilotFieldsObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
// TODO

    ScDPObject* pDPObj(pParent->GetDPObject());

    if (pDPObj)
    {
        uno::Sequence<rtl::OUString> aSeq(static_cast<sal_Int32>(lcl_GetFieldCount(pDPObj->GetSource(), nType)));
        rtl::OUString* pAry = aSeq.getArray();
        const List& rDimensions = pDPObj->GetSaveData()->GetDimensions();
        sal_Int32 nDimCount = rDimensions.Count();
        for (sal_Int32 nDim = 0; nDim < nDimCount; nDim++)
        {
            ScDPSaveDimension* pDim = (ScDPSaveDimension*)rDimensions.GetObject(nDim);
            if(pDim->GetOrientation() == nType)
            {
                *pAry = pDim->GetName();
                ++pAry;
            }
        }
        return aSeq;
    }
    return uno::Sequence<rtl::OUString>();
}

sal_Bool SAL_CALL ScDataPilotFieldsObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    return GetObjectByName_Impl(aName) != NULL;
}

//------------------------------------------------------------------------

ScDataPilotFieldObj::ScDataPilotFieldObj( ScDataPilotDescriptorBase* pPar,
                                            USHORT nST, const ScFieldIdentifier& rIdent ) :
    aPropSet( lcl_GetDataPilotFieldMap() ),
    pParent( pPar ),
    nSourceType( nST ),
    aSourceIdent( rIdent ),
    nLastFunc( sheet::GeneralFunction_NONE )
{
    pParent->acquire();
}

ScDataPilotFieldObj::~ScDataPilotFieldObj()
{
    pParent->release();
}

// XNamed

rtl::OUString SAL_CALL ScDataPilotFieldObj::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
// TODO
    rtl::OUString sRet;
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            if (pDim->IsDataLayout())
                return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(SC_DATALAYOUT_NAME));
            else
                sRet = pDim->GetLayoutName();
        }
    }
    return sRet;
}

void SAL_CALL ScDataPilotFieldObj::setName( const rtl::OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
// TODO
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            if (!pDim->IsDataLayout())
            {
                String aName(aNewName);
                pDim->SetLayoutName(&aName);
                pParent->SetDPObject(pDPObj);
            }
        }
    }
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDataPilotFieldObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScDataPilotFieldObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString(aPropertyName);
    if ( aNameString.EqualsAscii( SC_UNONAME_FUNCTION ) )
    {
        //! test for correct enum type?
        sheet::GeneralFunction eFunction = (sheet::GeneralFunction)
                            ScUnoHelpFunctions::GetEnumFromAny( aValue );
        setFunction( eFunction );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_ORIENT ) )
    {
        //! test for correct enum type?
        sheet::DataPilotFieldOrientation eOrient = (sheet::DataPilotFieldOrientation)
                            ScUnoHelpFunctions::GetEnumFromAny( aValue );
        setOrientation( eOrient );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_SELPAGE ) )
    {
        rtl::OUString sCurrentPage;
        if (aValue >>= sCurrentPage)
            setCurrentPage(sCurrentPage);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_USESELPAGE ) )
    {
        setUseCurrentPage(cppu::any2bool(aValue));
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_HASAUTOSHOW ) )
    {
        if (!cppu::any2bool(aValue))
            setAutoShowInfo(NULL);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_AUTOSHOW ) )
    {
        sheet::DataPilotFieldAutoShowInfo aInfo;
        if (aValue >>= aInfo)
            setAutoShowInfo(&aInfo);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_HASLAYOUTINFO ) )
    {
        if (!cppu::any2bool(aValue))
            setLayoutInfo(NULL);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_LAYOUTINFO ) )
    {
        sheet::DataPilotFieldLayoutInfo aInfo;
        if (aValue >>= aInfo)
            setLayoutInfo(&aInfo);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_HASREFERENCE ) )
    {
        if (!cppu::any2bool(aValue))
            setReference(NULL);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_REFERENCE ) )
    {
        sheet::DataPilotFieldReference aRef;
        if (aValue >>= aRef)
            setReference(&aRef);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_HASSORTINFO ) )
    {
        if (!cppu::any2bool(aValue))
            setSortInfo(NULL);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_SORTINFO ) )
    {
        sheet::DataPilotFieldSortInfo aInfo;
        if (aValue >>= aInfo)
            setSortInfo(&aInfo);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_ISGROUP ) )
    {
        if (!cppu::any2bool(aValue))
            setGroupInfo(NULL);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_GROUPINFO ) )
    {
        sheet::DataPilotFieldGroupInfo aInfo;
        if (aValue >>= aInfo)
            setGroupInfo(&aInfo);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_SHOWEMPTY ) )
    {
        setShowEmpty(cppu::any2bool(aValue));
    }
}

uno::Any SAL_CALL ScDataPilotFieldObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString(aPropertyName);
    uno::Any aRet;

    if ( aNameString.EqualsAscii( SC_UNONAME_FUNCTION ) )
        aRet <<= getFunction();
    else if ( aNameString.EqualsAscii( SC_UNONAME_ORIENT ) )
        aRet <<= getOrientation();
    else if ( aNameString.EqualsAscii( SC_UNONAME_SELPAGE ) )
        aRet <<= getCurrentPage();
    else if ( aNameString.EqualsAscii( SC_UNONAME_USESELPAGE ) )
        aRet <<= getUseCurrentPage();
    else if ( aNameString.EqualsAscii( SC_UNONAME_HASAUTOSHOW ) )
        aRet = ::cppu::bool2any(getAutoShowInfo() != NULL);
    else if ( aNameString.EqualsAscii( SC_UNONAME_AUTOSHOW ) )
    {
        const sheet::DataPilotFieldAutoShowInfo* pInfo = getAutoShowInfo();
        if (pInfo)
            aRet <<= sheet::DataPilotFieldAutoShowInfo(*pInfo);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_HASLAYOUTINFO ) )
        aRet = ::cppu::bool2any(getLayoutInfo() != NULL);
    else if ( aNameString.EqualsAscii( SC_UNONAME_LAYOUTINFO ) )
    {
        const sheet::DataPilotFieldLayoutInfo* pInfo = getLayoutInfo();
        if (pInfo)
            aRet <<= sheet::DataPilotFieldLayoutInfo(*pInfo);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_HASREFERENCE ) )
        aRet = ::cppu::bool2any(getReference() != NULL);
    else if ( aNameString.EqualsAscii( SC_UNONAME_REFERENCE ) )
    {
        const sheet::DataPilotFieldReference* pRef = getReference();
        if (pRef)
            aRet <<= sheet::DataPilotFieldReference(*pRef);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_HASSORTINFO ) )
        aRet = ::cppu::bool2any(getSortInfo() != NULL);
    else if ( aNameString.EqualsAscii( SC_UNONAME_SORTINFO ) )
    {
        const sheet::DataPilotFieldSortInfo* pInfo = getSortInfo();
        if (pInfo)
            aRet <<= sheet::DataPilotFieldSortInfo(*pInfo);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_ISGROUP ) )
        aRet = ::cppu::bool2any(hasGroupInfo());
    else if ( aNameString.EqualsAscii( SC_UNONAME_GROUPINFO ) )
    {
        aRet <<= getGroupInfo();
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_SHOWEMPTY ) )
        aRet <<= getShowEmpty();

    return aRet;
}

// XDatePilotField

uno::Reference<container::XIndexAccess> SAL_CALL ScDataPilotFieldObj::getItems()
                throw (uno::RuntimeException)
{
    if (!xItems.is())
        xItems.set(new ScDataPilotItemsObj(pParent, aSourceIdent));
    return xItems;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDataPilotFieldObj )

sheet::DataPilotFieldOrientation ScDataPilotFieldObj::getOrientation(void) const
{
    sheet::DataPilotFieldOrientation eOrient = DATA_PILOT_HIDDEN;
// TODO
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
            eOrient = (sheet::DataPilotFieldOrientation)pDim->GetOrientation();
    }

    return eOrient;
}

void ScDataPilotFieldObj::setOrientation(sheet::DataPilotFieldOrientation eNew)
{
    if ( eNew == nSourceType )
        return;                     // nix

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            if ( nSourceType == SC_FIELDORIENT_ALL && pDim->GetOrientation() != DATA_PILOT_HIDDEN &&
                 !aSourceIdent.bDataLayoutField && eNew == DATA_PILOT_DATA )
            {
                // If the field was taken from getDataPilotFields, don't reset the orientation
                // for an existing use, create a duplicated field instead (for "Data" orientation only)

                ScDPSaveDimension* pNewDim = NULL;
                ScDPSaveData* pSave = pDPObj->GetSaveData();

                // look for existing duplicate with orientation "hidden"

                String aNameStr( aSourceIdent.sFieldName );
                const List& rDimensions = pSave->GetDimensions();
                sal_Int32 nDimCount = rDimensions.Count();
                sal_Int32 nFound = 0;
                for ( sal_Int32 nDim = 0; nDim < nDimCount && !pNewDim; nDim++ )
                {
                    ScDPSaveDimension* pOneDim = static_cast<ScDPSaveDimension*>(rDimensions.GetObject(nDim));
                    if ( !pOneDim->IsDataLayout() && pOneDim->GetName() == aNameStr )
                    {
                        if ( pOneDim->GetOrientation() == DATA_PILOT_HIDDEN )
                            pNewDim = pOneDim;      // use this one
                        else
                            ++nFound;               // count existing non-hidden occurences
                    }
                }

                if ( !pNewDim )     // if none found, create a new duplicated dimension
                    pNewDim = &pSave->DuplicateDimension( *pDim );

                aSourceIdent.nRepeat = nFound;      // keep accessing the new one
                pDim = pNewDim;
            }

            pDim->SetOrientation(sal::static_int_cast<USHORT>(eNew));
            pParent->SetDPObject(pDPObj);

            nSourceType = sal::static_int_cast<USHORT>(eNew);   // modifying the same object's orientation again doesn't create another duplicate
        }
    }
}

sheet::GeneralFunction ScDataPilotFieldObj::getFunction(void) const
{
    sheet::GeneralFunction eRet = sheet::GeneralFunction_NONE;

// TODO

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            if ( pDim->GetOrientation() != DATA_PILOT_DATA )
            {
                // for non-data fields, property Function is the subtotals
                long nSubCount = pDim->GetSubTotalsCount();
                if ( nSubCount > 0 )
                    eRet = (sheet::GeneralFunction)pDim->GetSubTotalFunc(0);    // always use the first one
                // else keep NONE
            }
            else
                eRet = (sheet::GeneralFunction)pDim->GetFunction();
        }
    }

    return eRet;
}

void ScDataPilotFieldObj::setFunction(sheet::GeneralFunction eNewFunc)
{
// TODO
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            if ( pDim->GetOrientation() != DATA_PILOT_DATA )
            {
                // for non-data fields, property Function is the subtotals
                if ( eNewFunc == sheet::GeneralFunction_NONE )
                    pDim->SetSubTotals( 0, NULL );
                else
                {
                    USHORT nFunc = sal::static_int_cast<USHORT>( eNewFunc );
                    pDim->SetSubTotals( 1, &nFunc );
                }
            }
            else
                pDim->SetFunction( sal::static_int_cast<USHORT>( eNewFunc ) );
            pParent->SetDPObject(pDPObj);
        }
    }
}

rtl::OUString ScDataPilotFieldObj::getCurrentPage() const
{
    rtl::OUString sRet;

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
            if (pDim->HasCurrentPage())
                sRet = pDim->GetCurrentPage();
    }

    return sRet;
}

void ScDataPilotFieldObj::setCurrentPage(const rtl::OUString& sPage)
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            String sCur(sPage);
            pDim->SetCurrentPage(&sCur);
            pParent->SetDPObject(pDPObj);
        }
    }
}

sal_Bool ScDataPilotFieldObj::getUseCurrentPage() const
{
    sal_Bool bRet = sal_False;

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
            bRet = pDim->HasCurrentPage();
    }

    return bRet;
}

void ScDataPilotFieldObj::setUseCurrentPage(sal_Bool bUse)
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            if (bUse)
            {
                String sCur;
                pDim->SetCurrentPage(&sCur);
            }
            else
                pDim->SetCurrentPage(NULL);
            pParent->SetDPObject(pDPObj);
        }
    }
}

const sheet::DataPilotFieldAutoShowInfo* ScDataPilotFieldObj::getAutoShowInfo()
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
            return pDim->GetAutoShowInfo();
    }
    return NULL;
}

void ScDataPilotFieldObj::setAutoShowInfo(const sheet::DataPilotFieldAutoShowInfo* pInfo)
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            pDim->SetAutoShowInfo(pInfo);
            pParent->SetDPObject(pDPObj);
        }
    }
}

const sheet::DataPilotFieldLayoutInfo* ScDataPilotFieldObj::getLayoutInfo()
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
            return pDim->GetLayoutInfo();
    }

    return NULL;
}

void ScDataPilotFieldObj::setLayoutInfo(const sheet::DataPilotFieldLayoutInfo* pInfo)
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            pDim->SetLayoutInfo(pInfo);
            pParent->SetDPObject(pDPObj);
        }
    }
}

const sheet::DataPilotFieldReference* ScDataPilotFieldObj::getReference()
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
            return pDim->GetReferenceValue();
    }

    return NULL;
}

void ScDataPilotFieldObj::setReference(const sheet::DataPilotFieldReference* pInfo)
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            pDim->SetReferenceValue(pInfo);
            pParent->SetDPObject(pDPObj);
        }
    }
}

const sheet::DataPilotFieldSortInfo* ScDataPilotFieldObj::getSortInfo()
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
            return pDim->GetSortInfo();
    }

    return NULL;
}

void ScDataPilotFieldObj::setSortInfo(const sheet::DataPilotFieldSortInfo* pInfo)
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            pDim->SetSortInfo(pInfo);
            pParent->SetDPObject(pDPObj);
        }
    }
}

sal_Bool ScDataPilotFieldObj::getShowEmpty() const
{
    sal_Bool bRet = sal_False;

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
            bRet = pDim->GetShowEmpty();
    }

    return bRet;
}

void ScDataPilotFieldObj::setShowEmpty(sal_Bool bShow)
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            pDim->SetShowEmpty(bShow);
            pParent->SetDPObject(pDPObj);
        }
    }
}

void ScDataPilotFieldObj::SetGroupInfo(const ScDPNumGroupInfo& rGroupInfo,
                                       sheet::DataPilotFieldGroupInfo& rInfo)
{
    rInfo.HasDateValues = rGroupInfo.DateValues;
    rInfo.HasAutoStart = rGroupInfo.AutoStart;
    rInfo.Start = rGroupInfo.Start;
    rInfo.HasAutoEnd = rGroupInfo.AutoEnd;
    rInfo.End = rGroupInfo.End;
    rInfo.Step = rGroupInfo.Step;
}

void ScDataPilotFieldObj::FillGroupInfo(const ScDPSaveGroupDimension* pGroupDim,
    const ScDPSaveNumGroupDimension* pNumGroupDim, sheet::DataPilotFieldGroupInfo& rInfo)
{
    if (pGroupDim || pNumGroupDim)
    {
        if (pGroupDim)
        {
            rInfo.GroupBy = pGroupDim->GetDatePart();
            if (pParent)
            {
                uno::Reference<container::XNameAccess> xFields(pParent->getDataPilotFields(), uno::UNO_QUERY);
                if (xFields.is())
                {
                    rInfo.SourceField.set(xFields->getByName(pGroupDim->GetSourceDimName()), uno::UNO_QUERY);
                }
            }
            SetGroupInfo(pGroupDim->GetDateInfo(), rInfo);
            if (!pGroupDim->GetDatePart())
            {
                ScFieldGroups aGroups;
                sal_Int32 nCount = pGroupDim->GetGroupCount();
                for (sal_Int32 i = 0; i < nCount; ++i)
                {
                    const ScDPSaveGroupItem* pGroup = pGroupDim->GetGroupByIndex( i );
                    if (pGroup)
                    {
                        ScFieldGroup aGroup;
                        aGroup.sName = pGroup->GetGroupName();
                        sal_Int32 nElemCount = pGroup->GetElementCount();
                        for(sal_Int32 j = 0; j < nElemCount; ++j)
                        {
                            const String* pElem = pGroup->GetElementByIndex( j );
                            if (pElem)
                            {
                                aGroup.aMembers.push_back(*pElem);
                            }
                        }
                        aGroups.push_back(aGroup);
                    }
                }
                rInfo.Groups = new ScDataPilotFieldGroupsObj(aGroups);
            }
        }
        else
        {
            if (pNumGroupDim->GetDatePart())
            {
                rInfo.GroupBy = pNumGroupDim->GetDatePart();
                SetGroupInfo(pNumGroupDim->GetDateInfo(), rInfo);
            }
            else
            {
                SetGroupInfo(pNumGroupDim->GetInfo(), rInfo);
            }
        }
    }
}

sal_Bool ScDataPilotFieldObj::hasGroupInfo()
{
    sal_Bool bRet = sal_False;
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            const ScDPSaveData* pDPSave = pDPObj->GetSaveData();
            const ScDPDimensionSaveData* pDimData = pDPSave->GetExistingDimensionData();
            if (pDimData)
            {
                bRet = (pDimData->GetNamedGroupDim(pDim->GetName()) || pDimData->GetNumGroupDim(pDim->GetName()));
            }
        }
    }
    return bRet;
}

sheet::DataPilotFieldGroupInfo ScDataPilotFieldObj::getGroupInfo()
{
    sheet::DataPilotFieldGroupInfo aInfo;

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            const ScDPSaveData* pDPSave = pDPObj->GetSaveData();
            const ScDPDimensionSaveData* pDimData = pDPSave->GetExistingDimensionData();
            if (pDimData)
            {
                FillGroupInfo(pDimData->GetNamedGroupDim(pDim->GetName()),
                    pDimData->GetNumGroupDim(pDim->GetName()), aInfo);
            }
        }
    }

    return aInfo;
}

void ScDataPilotFieldObj::setGroupInfo(const sheet::DataPilotFieldGroupInfo* pInfo)
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            ScDPSaveData* pSaveData = pDPObj->GetSaveData();
            if (pInfo)
            {
                ScDPNumGroupInfo aInfo;
                aInfo.Enable = sal_True;
                aInfo.DateValues = pInfo->HasDateValues;
                aInfo.AutoStart = pInfo->HasAutoStart;
                aInfo.AutoEnd = pInfo->HasAutoEnd;
                aInfo.Start = pInfo->Start;
                aInfo.End = pInfo->End;
                aInfo.Step = pInfo->Step;
                uno::Reference<container::XNamed> xNamed(pInfo->SourceField, uno::UNO_QUERY);
                if (xNamed.is())
                {
                    ScDPSaveGroupDimension aGroupDim(xNamed->getName(), getName());
                    if (pInfo->GroupBy)
                        aGroupDim.SetDateInfo(aInfo, pInfo->GroupBy);
                    else
                    {
                        uno::Reference<container::XIndexAccess> xIndex(pInfo->Groups, uno::UNO_QUERY);
                        if (xIndex.is())
                        {
                            sal_Int32 nCount(xIndex->getCount());
                            for(sal_Int32 i = 0; i < nCount; i++)
                            {
                                uno::Reference<container::XNamed> xGroupNamed(xIndex->getByIndex(i), uno::UNO_QUERY);
                                if (xGroupNamed.is())
                                {
                                    ScDPSaveGroupItem aItem(xGroupNamed->getName());
                                    uno::Reference<container::XIndexAccess> xGroupIndex(xGroupNamed, uno::UNO_QUERY);
                                    if (xGroupIndex.is())
                                    {
                                        sal_Int32 nItemCount(xGroupIndex->getCount());
                                        for (sal_Int32 j = 0; j < nItemCount; ++j)
                                        {
                                            uno::Reference<container::XNamed> xItemNamed(xGroupIndex->getByIndex(j), uno::UNO_QUERY);
                                            if (xItemNamed.is())
                                                aItem.AddElement(xItemNamed->getName());
                                        }
                                    }
                                    aGroupDim.AddGroupItem(aItem);
                                }
                            }
                        }
                    }
                    ScDPDimensionSaveData aDimSaveData;

                    aDimSaveData.AddGroupDimension(aGroupDim);
                    pSaveData->SetDimensionData(&aDimSaveData);
                }
                else //NumGroup
                {
                    ScDPDimensionSaveData* pDimData = pSaveData->GetDimensionData();     // created if not there

                    ScDPSaveNumGroupDimension* pExisting = pDimData->GetNumGroupDimAcc( getName() );
                    if ( pExisting )
                    {
                        if (pInfo->GroupBy)
                            pExisting->SetDateInfo(aInfo, pInfo->GroupBy);
                        // modify existing group dimension
                        pExisting->SetGroupInfo( aInfo );
                    }
                    else
                    {
                        // create new group dimension
                        ScDPSaveNumGroupDimension aNumGroupDim( getName(), aInfo );
                        if (pInfo->GroupBy)
                            aNumGroupDim.SetDateInfo(aInfo, pInfo->GroupBy);
                        pDimData->AddNumGroupDimension( aNumGroupDim );
                    }

//                    pSaveData->SetDimensionData(pDimData); not neccessary
                }
            }
            else
            {
                pSaveData->SetDimensionData(NULL);
            }
            pDPObj->SetSaveData(*pSaveData);
            pParent->SetDPObject(pDPObj);
        }
    }
}

sal_Bool ScDataPilotFieldObj::HasString(const uno::Sequence< ::rtl::OUString >& rItems, const rtl::OUString& aString)
{
    sal_Bool bRet = sal_False;

    sal_Int32 nCount(rItems.getLength());
    sal_Int32 nItem(0);
    while (nItem < nCount && !bRet)
    {
        bRet = rItems[nItem] == aString;
        ++nItem;
    }

    return bRet;
}

// XDataPilotFieldGrouping
uno::Reference < sheet::XDataPilotField > SAL_CALL
        ScDataPilotFieldObj::createNameGroup(const uno::Sequence< rtl::OUString >& rItems)
             throw (::com::sun::star::uno::RuntimeException, lang::IllegalArgumentException)
{
    ScUnoGuard aGuard;

    uno::Reference < sheet::XDataPilotField > xRet;
    rtl::OUString sNewDim;

    if (!rItems.getLength())
        throw lang::IllegalArgumentException();

    ScDPObject* pDPObj(pParent->GetDPObject());
    if ( pDPObj )
    {
        ScDPSaveDimension* pDim = NULL;
        if ( rItems.getLength() > 0 && lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            String aDimName (pDim->GetName());

            ScDPSaveData aSaveData(*pDPObj->GetSaveData());
            ScDPDimensionSaveData* pDimData = aSaveData.GetDimensionData();     // created if not there

            // find original base
            String aBaseDimName( aDimName );
            const ScDPSaveGroupDimension* pBaseGroupDim = pDimData->GetNamedGroupDim( aDimName );
            if ( pBaseGroupDim )
            {
                // any entry's SourceDimName is the original base
                aBaseDimName = pBaseGroupDim->GetSourceDimName();
            }

            // find existing group dimension
            // (using the selected dim, can be intermediate group dim)
            ScDPSaveGroupDimension* pGroupDimension = pDimData->GetGroupDimAccForBase( aDimName );

            // remove the selected items from their groups
            // (empty groups are removed, too)
            sal_Int32 nEntryCount = rItems.getLength();
            sal_Int32 nEntry;
            if ( pGroupDimension )
            {
                for (nEntry=0; nEntry<nEntryCount; nEntry++)
                {
                    String aEntryName(rItems[nEntry]);
                    if ( pBaseGroupDim )
                    {
                        // for each selected (intermediate) group, remove all its items
                        // (same logic as for adding, below)
                        const ScDPSaveGroupItem* pBaseGroup = pBaseGroupDim->GetNamedGroup( aEntryName );
                        if ( pBaseGroup )
                            pBaseGroup->RemoveElementsFromGroups( *pGroupDimension );   // remove all elements
                        else
                            pGroupDimension->RemoveFromGroups( aEntryName );
                    }
                    else
                        pGroupDimension->RemoveFromGroups( aEntryName );
                }
            }

            ScDPSaveGroupDimension* pNewGroupDim = NULL;
            if ( !pGroupDimension )
            {
                // create a new group dimension
                String aGroupDimName = pDimData->CreateGroupDimName( aBaseDimName, *pDPObj, false, NULL );
                pNewGroupDim = new ScDPSaveGroupDimension( aBaseDimName, aGroupDimName );
                sNewDim = aGroupDimName;

                pGroupDimension = pNewGroupDim;     // make changes to the new dim if none existed

                if ( pBaseGroupDim )
                {
                    // If it's a higher-order group dimension, pre-allocate groups for all
                    // non-selected original groups, so the individual base members aren't
                    // used for automatic groups (this would make the original groups hard
                    // to find).
                    //! Also do this when removing groups?
                    //! Handle this case dynamically with automatic groups?

                    long nGroupCount = pBaseGroupDim->GetGroupCount();
                    for ( long nGroup = 0; nGroup < nGroupCount; nGroup++ )
                    {
                        const ScDPSaveGroupItem* pBaseGroup = pBaseGroupDim->GetGroupByIndex( nGroup );

                        StrData aStrData( pBaseGroup->GetGroupName() );
                        if ( !HasString(rItems, aStrData.GetString()) )    //! ignore case?
                        {
                            // add an additional group for each item that is not in the selection
                            ScDPSaveGroupItem aGroup( pBaseGroup->GetGroupName() );
                            aGroup.AddElementsFromGroup( *pBaseGroup );
                            pGroupDimension->AddGroupItem( aGroup );
                        }
                    }
                }
            }
            String aGroupDimName = pGroupDimension->GetGroupDimName();

            //! localized prefix string
            String aGroupName = pGroupDimension->CreateGroupName( String::CreateFromAscii("Group") );
            ScDPSaveGroupItem aGroup( aGroupName );
            uno::Reference<container::XNameAccess> xMembers;
            if (!lcl_GetMembers(pParent, aSourceIdent, xMembers))
            {
                delete pNewGroupDim;
                throw uno::RuntimeException();
            }

            for (nEntry=0; nEntry<nEntryCount; nEntry++)
            {
                String aEntryName(rItems[nEntry]);

                if (!xMembers->hasByName(aEntryName))
                {
                    delete pNewGroupDim;
                    throw lang::IllegalArgumentException();
                }

                if ( pBaseGroupDim )
                {
                    // for each selected (intermediate) group, add all its items
                    const ScDPSaveGroupItem* pBaseGroup = pBaseGroupDim->GetNamedGroup( aEntryName );
                    if ( pBaseGroup )
                        aGroup.AddElementsFromGroup( *pBaseGroup );
                    else
                        aGroup.AddElement( aEntryName );    // no group found -> automatic group, add the item itself
                }
                else
                    aGroup.AddElement( aEntryName );        // no group dimension, add all items directly
            }

            pGroupDimension->AddGroupItem( aGroup );

            if ( pNewGroupDim )
            {
                pDimData->AddGroupDimension( *pNewGroupDim );
                delete pNewGroupDim;        // AddGroupDimension copies the object
                // don't access pGroupDimension after here
            }
            pGroupDimension = pNewGroupDim = NULL;

            // set orientation
            ScDPSaveDimension* pSaveDimension = aSaveData.GetDimensionByName( aGroupDimName );
            if ( pSaveDimension->GetOrientation() == sheet::DataPilotFieldOrientation_HIDDEN )
            {
                ScDPSaveDimension* pOldDimension = aSaveData.GetDimensionByName( aDimName );
                pSaveDimension->SetOrientation( pOldDimension->GetOrientation() );
                long nPosition = 0;     //! before (immediate) base
                aSaveData.SetPosition( pSaveDimension, nPosition );
            }

            // apply changes
            pDPObj->SetSaveData( aSaveData );
            pParent->SetDPObject(pDPObj);
        }
    }

    if (sNewDim.getLength())
    {
        uno::Reference< container::XNameAccess > xFields(pParent->getDataPilotFields(), uno::UNO_QUERY);
        if (xFields.is())
        {
            xRet.set(xFields->getByName(sNewDim), uno::UNO_QUERY);
            DBG_ASSERT(xRet.is(), "there is a name, so there should be also a field");
        }
    }

    return xRet;
}

String lcl_GetDateByName( sal_Int32 nGroupBy )
{
    String aRet;        //! globstr-ID
    switch (nGroupBy)
    {
        //! use translated strings from globstr.src
        case com::sun::star::sheet::DataPilotFieldGroupBy::SECONDS:  aRet = String::CreateFromAscii("Seconds");  break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::MINUTES:  aRet = String::CreateFromAscii("Minutes");  break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::HOURS:    aRet = String::CreateFromAscii("Hours");    break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::DAYS:     aRet = String::CreateFromAscii("Days");     break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::MONTHS:   aRet = String::CreateFromAscii("Months");   break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::QUARTERS: aRet = String::CreateFromAscii("Quarters"); break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::YEARS:    aRet = String::CreateFromAscii("Years");    break;
        default:
            DBG_ERROR("invalid date part");
    }
    return aRet;
}

uno::Reference < sheet::XDataPilotField > SAL_CALL
        ScDataPilotFieldObj::createDateGroup(const sheet::DataPilotFieldGroupInfo& rInfo)
        throw (::com::sun::star::uno::RuntimeException, lang::IllegalArgumentException)
{
    ScUnoGuard aGuard;

    if (!rInfo.HasDateValues)
        throw lang::IllegalArgumentException();

    uno::Reference < sheet::XDataPilotField > xRet;

    ScDPObject* pDPObj(pParent->GetDPObject());
    if ( pDPObj )
    {
        ScDPSaveDimension* pDim = NULL;
        if ( lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            ScDPNumGroupInfo aInfo;
            aInfo.DateValues = rInfo.HasDateValues;
            aInfo.AutoStart = rInfo.HasAutoStart;
            aInfo.Start = rInfo.Start;
            aInfo.AutoEnd = rInfo.HasAutoEnd;
            aInfo.End = rInfo.End;
            aInfo.Step = rInfo.Step;

            String aDimName (pDim->GetName());

            ScDPSaveData aData( *pDPObj->GetSaveData() );
            ScDPDimensionSaveData* pDimData = aData.GetDimensionData();     // created if not there

            // find original base
            String aBaseDimName( aDimName );
            const ScDPSaveGroupDimension* pBaseGroupDim = pDimData->GetNamedGroupDim( aDimName );
            if ( pBaseGroupDim )
            {
                // any entry's SourceDimName is the original base
                aBaseDimName = pBaseGroupDim->GetSourceDimName();
            }

            if ( rInfo.GroupBy )
            {
                // create date group dimensions

                sal_Bool bFirst(sal_False);
                if (!pBaseGroupDim) // it's the base Dim
                {
                    // test whether there is already grouping
                    const ScDPSaveNumGroupDimension* pNumGroupDim = pDimData->GetNumGroupDim(pDim->GetName());
                    if (pNumGroupDim)
                    {
                        if (!pNumGroupDim->GetDateInfo().DateValues)
                            bFirst = sal_True;
                    }
                    else
                        bFirst = sal_True;
                }

                ScDPNumGroupInfo aEmpty;
                if ( bFirst )
                {
                    // innermost part: create NumGroupDimension (replacing original values)
                    // Dimension name is left unchanged

                    if ( rInfo.GroupBy == com::sun::star::sheet::DataPilotFieldGroupBy::DAYS && rInfo.Step != 0.0 )
                    {
                        // only days, and a step value specified: use numerical grouping
                        // with DateValues flag, not date grouping

                        ScDPNumGroupInfo aNumInfo( aInfo );
                        aNumInfo.DateValues = sal_True;

                        ScDPSaveNumGroupDimension aNumGroupDim( aBaseDimName, aNumInfo );
                        pDimData->AddNumGroupDimension( aNumGroupDim );
                    }
                    else
                    {
                        ScDPSaveNumGroupDimension aNumGroupDim( aBaseDimName, aEmpty );
                        aNumGroupDim.SetDateInfo( aInfo, rInfo.GroupBy );
                        pDimData->AddNumGroupDimension( aNumGroupDim );
                    }
                }
                else
                {
                    // additional parts: create GroupDimension (shown as additional dimensions)

                    String aPartName = lcl_GetDateByName( rInfo.GroupBy );

                    String aGroupDimName = pDimData->CreateGroupDimName(
                                        aPartName, *pDPObj, true, NULL );
                    ScDPSaveGroupDimension aGroupDim( aBaseDimName, aGroupDimName );
                    aGroupDim.SetDateInfo( aInfo, rInfo.GroupBy );
                    pDimData->AddGroupDimension( aGroupDim );

                    // set orientation
                    ScDPSaveDimension* pSaveDimension = aData.GetDimensionByName( aGroupDimName );
                    if ( pSaveDimension->GetOrientation() == sheet::DataPilotFieldOrientation_HIDDEN )
                    {
                        ScDPSaveDimension* pOldDimension = aData.GetDimensionByName( aBaseDimName );
                        pSaveDimension->SetOrientation( pOldDimension->GetOrientation() );
                        long nPosition = 0;     //! before (immediate) base
                        aData.SetPosition( pSaveDimension, nPosition );
                    }
                }
            }

            // apply changes
            pDPObj->SetSaveData( aData );
            pParent->SetDPObject(pDPObj);
        }
    }

    return xRet;
}
//------------------------------------------------------------------------

ScDataPilotFieldGroupsObj::ScDataPilotFieldGroupsObj(const ScFieldGroups& rGroups) :
    aGroups(rGroups)
{
}

ScDataPilotFieldGroupsObj::~ScDataPilotFieldGroupsObj()
{
}

// XNameAccess

uno::Any SAL_CALL ScDataPilotFieldGroupsObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    ScFieldGroups::const_iterator aItr(aGroups.begin());
    ScFieldGroups::const_iterator aEndItr(aGroups.end());
    sal_Bool bFound(sal_False);
    while (!bFound && aItr != aEndItr)
    {
        if (aItr->sName == aName)
            bFound = sal_True;
        else
            ++aItr;
    }
    if (bFound)
        return uno::makeAny(uno::Reference < container::XNameAccess > (new ScDataPilotFieldGroupObj(*aItr)));
    else
        throw container::NoSuchElementException();

//    return uno::Any();
}

uno::Sequence<rtl::OUString> SAL_CALL ScDataPilotFieldGroupsObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
// TODO

    uno::Sequence<rtl::OUString> aSeq(aGroups.size());
    ScFieldGroups::const_iterator aItr(aGroups.begin());
    ScFieldGroups::const_iterator aEndItr(aGroups.end());
    sal_Int32 i(0);
    while (aItr != aEndItr)
    {
        aSeq[i] = aItr->sName;
        ++aItr;
        ++i;
    }
    return aSeq;
}

sal_Bool SAL_CALL ScDataPilotFieldGroupsObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    ScFieldGroups::const_iterator aItr(aGroups.begin());
    ScFieldGroups::const_iterator aEndItr(aGroups.end());
    sal_Bool bFound(sal_False);
    while (!bFound && aItr != aEndItr)
    {
        if (aItr->sName == aName)
            bFound = sal_True;
        else
            ++aItr;
    }

    return bFound;
}

// XNameReplace
void SAL_CALL ScDataPilotFieldGroupsObj::replaceByName( const ::rtl::OUString& aName,
                                const uno::Any& aElement )
                                throw (lang::IllegalArgumentException,
                                    container::NoSuchElementException,
                                    lang::WrappedTargetException,
                                    uno::RuntimeException)
{
    ScUnoGuard aGuard;

    ScFieldGroups::iterator aItr(aGroups.begin());
    ScFieldGroups::iterator aEndItr(aGroups.end());
    sal_Bool bFound(sal_False);
    while (!bFound && aItr != aEndItr)
    {
        if (aItr->sName == aName)
            bFound = sal_True;
        else
            ++aItr;
    }
    if (bFound)
    {
        uno::Reference<container::XNamed> xNamed(aElement, uno::UNO_QUERY);
        if (xNamed.is())
        {
            ScFieldGroup aGroup;
            aGroup.sName = xNamed->getName();
            uno::Reference<container::XIndexAccess> xIndex(xNamed, uno::UNO_QUERY);
            if (xIndex.is())
            {
                sal_Int32 nCount(xIndex->getCount());
                for (sal_Int32 i = 0; i < nCount; ++i)
                {
                    uno::Reference<container::XNamed> xItem(xIndex->getByIndex(i), uno::UNO_QUERY);
                    if (xItem.is())
                        aGroup.aMembers.push_back(xNamed->getName());
                    else
                        throw lang::IllegalArgumentException();
                }
            }
            else
                throw lang::IllegalArgumentException();

            aGroups.erase(aItr);
            aGroups.push_back(aGroup);
        }
        else
            throw lang::IllegalArgumentException();
    }
    else
        throw container::NoSuchElementException();
}

// XNameContainer
void SAL_CALL ScDataPilotFieldGroupsObj::insertByName( const ::rtl::OUString& aName,
                                const uno::Any& aElement )
                                throw (lang::IllegalArgumentException,
                                    container::ElementExistException,
                                    lang::WrappedTargetException,
                                    uno::RuntimeException)
{
    ScUnoGuard aGuard;

    ScFieldGroups::const_iterator aItr(aGroups.begin());
    ScFieldGroups::const_iterator aEndItr(aGroups.end());
    sal_Bool bFound(sal_False);
    while (!bFound && aItr != aEndItr)
    {
        if (aItr->sName == aName)
            bFound = sal_True;
        else
            ++aItr;
    }
    if (!bFound)
    {
        uno::Reference<container::XNamed> xNamed(aElement, uno::UNO_QUERY);
        if (xNamed.is())
        {
            ScFieldGroup aGroup;
            aGroup.sName = xNamed->getName();
            uno::Reference<container::XIndexAccess> xIndex(xNamed, uno::UNO_QUERY);
            if (xIndex.is())
            {
                sal_Int32 nCount(xIndex->getCount());
                for (sal_Int32 i = 0; i < nCount; ++i)
                {
                    uno::Reference<container::XNamed> xItem(xIndex->getByIndex(i), uno::UNO_QUERY);
                    if (xItem.is())
                        aGroup.aMembers.push_back(xNamed->getName());
                    else
                        throw lang::IllegalArgumentException();
                }
            }
            else
                throw lang::IllegalArgumentException();

            aGroups.push_back(aGroup);
        }
        else
            throw lang::IllegalArgumentException();
    }
    else
        throw container::ElementExistException();
}

void SAL_CALL ScDataPilotFieldGroupsObj::removeByName( const ::rtl::OUString& aName )
                                throw (container::NoSuchElementException,
                                    lang::WrappedTargetException,
                                    uno::RuntimeException)
{
    ScUnoGuard aGuard;

    ScFieldGroups::iterator aItr(aGroups.begin());
    ScFieldGroups::iterator aEndItr(aGroups.end());
    sal_Bool bFound(sal_False);
    while (!bFound && aItr != aEndItr)
    {
        if (aItr->sName == aName)
            bFound = sal_True;
        else
            ++aItr;
    }
    if (bFound)
        aGroups.erase(aItr);
    else
        throw container::NoSuchElementException();
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScDataPilotFieldGroupsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.DataPilotFieldGroupsEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScDataPilotFieldGroupsObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return aGroups.size();
}

uno::Any SAL_CALL ScDataPilotFieldGroupsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (nIndex >= 0 && nIndex < sal::static_int_cast<sal_Int32>(aGroups.size()))
        return uno::makeAny(uno::Reference < container::XNameAccess > (new ScDataPilotFieldGroupObj(aGroups[nIndex])));
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScDataPilotFieldGroupsObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<container::XNameAccess>*)0);
}

sal_Bool SAL_CALL ScDataPilotFieldGroupsObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( !aGroups.empty() );
}

//------------------------------------------------------------------------

ScDataPilotFieldGroupObj::ScDataPilotFieldGroupObj(const ScFieldGroup& rGroup) :
    aGroup(rGroup)
{
}

ScDataPilotFieldGroupObj::~ScDataPilotFieldGroupObj()
{
}

// XNameAccess

uno::Any SAL_CALL ScDataPilotFieldGroupObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    std::vector< rtl::OUString >::const_iterator aItr(aGroup.aMembers.begin());
    std::vector< rtl::OUString >::const_iterator aEndItr(aGroup.aMembers.end());
    sal_Bool bFound(sal_False);
    while (!bFound && aItr != aEndItr)
    {
        if (*aItr == aName)
            bFound = sal_True;
        else
            ++aItr;
    }
    if (bFound)
        return uno::makeAny(uno::Reference < container::XNamed > (new ScDataPilotFieldGroupItemObj(*aItr)));

    return uno::Any();
}

uno::Sequence<rtl::OUString> SAL_CALL ScDataPilotFieldGroupObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
// TODO

    uno::Sequence<rtl::OUString> aSeq(aGroup.aMembers.size());
    std::vector< rtl::OUString >::const_iterator aItr(aGroup.aMembers.begin());
    std::vector< rtl::OUString >::const_iterator aEndItr(aGroup.aMembers.end());
    sal_Int32 i(0);
    while (aItr != aEndItr)
    {
        aSeq[i] = *aItr;
        ++aItr;
        ++i;
    }
    return aSeq;
}

sal_Bool SAL_CALL ScDataPilotFieldGroupObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    std::vector< rtl::OUString >::const_iterator aItr(aGroup.aMembers.begin());
    std::vector< rtl::OUString >::const_iterator aEndItr(aGroup.aMembers.end());
    sal_Bool bFound(sal_False);
    while (!bFound && aItr != aEndItr)
    {
        if (*aItr == aName)
            bFound = sal_True;
        else
            ++aItr;
    }

    return bFound;
}

// XNameReplace
void SAL_CALL ScDataPilotFieldGroupObj::replaceByName( const ::rtl::OUString& aName,
                                const uno::Any& aElement )
                                throw (lang::IllegalArgumentException,
                                    container::NoSuchElementException,
                                    lang::WrappedTargetException,
                                    uno::RuntimeException)
{
    ScUnoGuard aGuard;

    std::vector<rtl::OUString>::iterator aItr(aGroup.aMembers.begin());
    std::vector<rtl::OUString>::iterator aEndItr(aGroup.aMembers.end());
    sal_Bool bFound(sal_False);
    while (!bFound && aItr != aEndItr)
    {
        if (*aItr == aName)
            bFound = sal_True;
        else
            ++aItr;
    }
    if (bFound)
    {
        uno::Reference<container::XNamed> xNamed(aElement, uno::UNO_QUERY);
        if (xNamed.is())
        {
            aGroup.aMembers.erase(aItr);
            aGroup.aMembers.push_back(xNamed->getName());
        }
        else
            throw lang::IllegalArgumentException();
    }
    else
        throw container::NoSuchElementException();
}

// XNameContainer
void SAL_CALL ScDataPilotFieldGroupObj::insertByName( const ::rtl::OUString& aName,
                                const uno::Any& aElement )
                                throw (lang::IllegalArgumentException,
                                    container::ElementExistException,
                                    lang::WrappedTargetException,
                                    uno::RuntimeException)
{
    ScUnoGuard aGuard;

    std::vector<rtl::OUString>::iterator aItr(aGroup.aMembers.begin());
    std::vector<rtl::OUString>::iterator aEndItr(aGroup.aMembers.end());
    sal_Bool bFound(sal_False);
    while (!bFound && aItr != aEndItr)
    {
        if (*aItr == aName)
            bFound = sal_True;
        else
            ++aItr;
    }
    if (!bFound)
    {
        uno::Reference<container::XNamed> xNamed(aElement, uno::UNO_QUERY);
        if (xNamed.is())
        {
            if (aName == xNamed->getName())
                aGroup.aMembers.push_back(aName);
            else
                throw lang::IllegalArgumentException();
        }
        else
            throw lang::IllegalArgumentException();
    }
    else
        throw container::ElementExistException();
}

void SAL_CALL ScDataPilotFieldGroupObj::removeByName( const ::rtl::OUString& aName )
                                throw (container::NoSuchElementException,
                                    lang::WrappedTargetException,
                                    uno::RuntimeException)
{
    ScUnoGuard aGuard;

    std::vector<rtl::OUString>::iterator aItr(aGroup.aMembers.begin());
    std::vector<rtl::OUString>::iterator aEndItr(aGroup.aMembers.end());
    sal_Bool bFound(sal_False);
    while (!bFound && aItr != aEndItr)
    {
        if (*aItr == aName)
            bFound = sal_True;
        else
            ++aItr;
    }
    if (bFound)
        aGroup.aMembers.erase(aItr);
    else
        throw container::NoSuchElementException();
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScDataPilotFieldGroupObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.DataPilotFieldGroupEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScDataPilotFieldGroupObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return aGroup.aMembers.size();
}

uno::Any SAL_CALL ScDataPilotFieldGroupObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (nIndex >= 0 && nIndex < sal::static_int_cast<sal_Int32>(aGroup.aMembers.size()))
        return uno::makeAny(uno::Reference < container::XNamed > (new ScDataPilotFieldGroupItemObj(aGroup.aMembers[nIndex])));
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScDataPilotFieldGroupObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<container::XNamed>*)0);
}

sal_Bool SAL_CALL ScDataPilotFieldGroupObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( !aGroup.aMembers.empty() );
}

// XNamed
::rtl::OUString SAL_CALL ScDataPilotFieldGroupObj::getName() throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;

    return aGroup.sName;
}

void SAL_CALL ScDataPilotFieldGroupObj::setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;

    aGroup.sName = aName;
}

//------------------------------------------------------------------------

ScDataPilotFieldGroupItemObj::ScDataPilotFieldGroupItemObj(const rtl::OUString& rName)
    : sName(rName)
{
}

ScDataPilotFieldGroupItemObj::~ScDataPilotFieldGroupItemObj()
{
}

                            // XNamed
::rtl::OUString SAL_CALL ScDataPilotFieldGroupItemObj::getName() throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;

    return sName;
}

void SAL_CALL ScDataPilotFieldGroupItemObj::setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;

    sName = aName;
}

//------------------------------------------------------------------------

ScDataPilotItemsObj::ScDataPilotItemsObj(ScDataPilotDescriptorBase* pPar, const ScFieldIdentifier& rIdent) :
    pParent( pPar ),
    aSourceIdent( rIdent )
{
    pParent->acquire();
}

ScDataPilotItemsObj::~ScDataPilotItemsObj()
{
    pParent->release();
}

SCSIZE lcl_GetItemCount( ScDataPilotDescriptorBase* pParent, const ScFieldIdentifier& rIdent )
{
    SCSIZE nRet = 0;

    uno::Reference<container::XNameAccess> xMembers;
    if (lcl_GetMembers(pParent, rIdent, xMembers))
    {
        uno::Reference<container::XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
        nRet = static_cast<SCSIZE>(xMembersIndex->getCount());
    }

    return nRet;
}

// XDataPilotItemss

ScDataPilotItemObj* ScDataPilotItemsObj::GetObjectByIndex_Impl(SCSIZE nIndex) const
{
// TODO
    if (nIndex < lcl_GetItemCount(pParent, aSourceIdent))
        return new ScDataPilotItemObj( pParent, aSourceIdent, nIndex );

    return NULL;
}

// XNameAccess

uno::Any SAL_CALL ScDataPilotItemsObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    {
        uno::Reference<container::XNameAccess> xMembers;
        if (lcl_GetMembers(pParent, aSourceIdent, xMembers))
        {
            uno::Reference<container::XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
            sal_Int32 nCount = xMembersIndex->getCount();
            sal_Bool bFound(sal_False);
            sal_Int32 nItem = 0;
            while (nItem < nCount && !bFound )
            {
                uno::Reference<container::XNamed> xMember(xMembersIndex->getByIndex(nItem), uno::UNO_QUERY);
                if (xMember.is() && aName == xMember->getName())
                    return uno::makeAny(uno::Reference<beans::XPropertySet> (GetObjectByIndex_Impl(static_cast<SCSIZE>(nItem))));
                else
                    nItem++;
            }
            if (!bFound)
                throw container::NoSuchElementException();
        }
    }

    return uno::Any();
}

uno::Sequence<rtl::OUString> SAL_CALL ScDataPilotItemsObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
// TODO

    uno::Sequence<rtl::OUString> aSeq;
    if( ScDPObject* pDPObj = pParent->GetDPObject() )
    {
        sal_Int32 nObjIndex = lcl_GetObjectIndex( pDPObj, aSourceIdent );
        pDPObj->GetMembers( nObjIndex, aSeq );
    }
    return aSeq;
}

sal_Bool SAL_CALL ScDataPilotItemsObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    sal_Bool bFound(sal_False);
    {
        uno::Reference<container::XNameAccess> xMembers;
        if (lcl_GetMembers(pParent, aSourceIdent, xMembers))
        {
            uno::Reference<container::XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
            sal_Int32 nCount = xMembersIndex->getCount();
            sal_Int32 nItem = 0;
            while (nItem < nCount && !bFound )
            {
                uno::Reference<container::XNamed> xMember(xMembersIndex->getByIndex(nItem), uno::UNO_QUERY);
                if (xMember.is() && aName == xMember->getName())
                    bFound = sal_True;
                else
                    nItem++;
            }
        }
    }

    return bFound;
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScDataPilotItemsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.DataPilotItemsEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScDataPilotItemsObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
// TODO
    return static_cast<sal_Int32>(lcl_GetItemCount( pParent, aSourceIdent ));
}

uno::Any SAL_CALL ScDataPilotItemsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<beans::XPropertySet> xItem(GetObjectByIndex_Impl(static_cast<SCSIZE>(nIndex)));
    if (xItem.is())
        return uno::makeAny(xItem);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScDataPilotItemsObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<beans::XPropertySet>*)0);
}

sal_Bool SAL_CALL ScDataPilotItemsObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

//------------------------------------------------------------------------

ScDataPilotItemObj::ScDataPilotItemObj(ScDataPilotDescriptorBase* pPar, const ScFieldIdentifier& rIdent, SCSIZE nI)
    : aPropSet( lcl_GetDataPilotItemMap() ),
    pParent(pPar),
    aSourceIdent(rIdent),
    nIndex(nI)
{
    pParent->acquire();
}

ScDataPilotItemObj::~ScDataPilotItemObj()
{
    pParent->release();
}

                            // XNamed
::rtl::OUString SAL_CALL ScDataPilotItemObj::getName() throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    rtl::OUString sRet;
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        uno::Reference<container::XNameAccess> xMembers;
        if (lcl_GetMembers(pParent, aSourceIdent, xMembers))
        {
            uno::Reference<container::XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
            sal_Int32 nCount = xMembersIndex->getCount();
            if (nIndex < static_cast<SCSIZE>(nCount))
            {
                uno::Reference<container::XNamed> xMember(xMembersIndex->getByIndex(static_cast<sal_Int32>(nIndex)), uno::UNO_QUERY);
                sRet = xMember->getName();
            }
        }
    }
    return sRet;
}

void SAL_CALL ScDataPilotItemObj::setName( const ::rtl::OUString& /* aName */ )
                                throw(::com::sun::star::uno::RuntimeException)
{
}

                            // XPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL ScDataPilotItemObj::getPropertySetInfo(  )
                                throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScDataPilotItemObj::setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        uno::Reference<container::XNameAccess> xMembers;
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetMembers(pParent, aSourceIdent, xMembers) && lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            uno::Reference<container::XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
            sal_Int32 nCount = xMembersIndex->getCount();
            if (nIndex < static_cast<SCSIZE>(nCount) )
            {
                uno::Reference<container::XNamed> xMember(xMembersIndex->getByIndex(static_cast<sal_Int32>(nIndex)), uno::UNO_QUERY);
                String sName(xMember->getName());
                ScDPSaveMember* pMember = pDim->GetMemberByName(sName);
                if (pMember)
                {
                    bool bGetNewIndex = false;
                    if ( aNameString.EqualsAscii( SC_UNONAME_SHOWDETAIL ) )
                        pMember->SetShowDetails(cppu::any2bool(aValue));
                    else if ( aNameString.EqualsAscii( SC_UNONAME_ISHIDDEN ) )
                        pMember->SetIsVisible(!cppu::any2bool(aValue));
                    else if ( aNameString.EqualsAscii( SC_UNONAME_POS ) )
                    {
                        sal_Int32 nNewPos = 0;
                        if ( ( aValue >>= nNewPos ) && nNewPos >= 0 && nNewPos < nCount )
                        {
                            pDim->SetMemberPosition( sName, nNewPos );
                            // get new effective index (depends on sorting mode, which isn't modified)
                            bGetNewIndex = true;
                        }
                        else
                            throw lang::IllegalArgumentException();
                    }
                    pParent->SetDPObject(pDPObj);

                    if ( bGetNewIndex )     // after SetDPObject, get the new index
                    {
                        rtl::OUString aOUName( sName );
                        uno::Sequence<rtl::OUString> aItemNames = xMembers->getElementNames();
                        sal_Int32 nItemCount = aItemNames.getLength();
                        for (sal_Int32 nItem=0; nItem<nItemCount; ++nItem)
                            if (aItemNames[nItem] == aOUName)
                                nIndex = nItem;
                    }
                }
            }
        }
    }
}

::com::sun::star::uno::Any SAL_CALL ScDataPilotItemObj::getPropertyValue(
                                    const ::rtl::OUString& aPropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Any aRet;
    String aNameString = aPropertyName;

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        uno::Reference<container::XNameAccess> xMembers;
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetMembers(pParent, aSourceIdent, xMembers) && lcl_GetDim(pDPObj, aSourceIdent, pDim))
        {
            uno::Reference<container::XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
            sal_Int32 nCount = xMembersIndex->getCount();
            if (nIndex < static_cast<SCSIZE>(nCount) )
            {
                uno::Reference<container::XNamed> xMember(xMembersIndex->getByIndex(static_cast<sal_Int32>(nIndex)), uno::UNO_QUERY);
                String sName(xMember->getName());
                ScDPSaveMember* pMember = pDim->GetExistingMemberByName(sName);
                if ( aNameString.EqualsAscii( SC_UNONAME_SHOWDETAIL ) )
                {
                    if (pMember && pMember->HasShowDetails())
                    {
                        aRet = cppu::bool2any(pMember->GetShowDetails());
                    }
                    else
                    {
                        uno::Reference<beans::XPropertySet> xMemberProps(xMember, uno::UNO_QUERY);
                        if(xMemberProps.is())
                        {
                            aRet = xMemberProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_SHOWDETA)));
                        }
                        else
                            aRet = cppu::bool2any(sal_True);
                    }
                }
                else if ( aNameString.EqualsAscii( SC_UNONAME_ISHIDDEN ) )
                {
                    if (pMember && pMember->HasIsVisible())
                    {
                        aRet = cppu::bool2any(!pMember->GetIsVisible());
                    }
                    else
                    {
                        uno::Reference<beans::XPropertySet> xMemberProps(xMember, uno::UNO_QUERY);
                        if(xMemberProps.is())
                        {
                            aRet = cppu::bool2any(!cppu::any2bool(xMemberProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ISVISIBL)))));
                        }
                        else
                            aRet = cppu::bool2any(sal_False);
                    }
                }
                else if ( aNameString.EqualsAscii( SC_UNONAME_POS ) )
                {
                    aRet <<= static_cast<sal_Int32>( nIndex );
                }
            }
        }
    }
    return aRet;
}

void SAL_CALL ScDataPilotItemObj::addPropertyChangeListener( const ::rtl::OUString& /* aPropertyName */,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& /* xListener */ )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL ScDataPilotItemObj::removePropertyChangeListener( const ::rtl::OUString& /* aPropertyName */,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& /* aListener */ )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL ScDataPilotItemObj::addVetoableChangeListener( const ::rtl::OUString& /* PropertyName */,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& /* aListener */ )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL ScDataPilotItemObj::removeVetoableChangeListener( const ::rtl::OUString& /* PropertyName */,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& /* aListener */ )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException)
{
}

//------------------------------------------------------------------------




