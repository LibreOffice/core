/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "dpobject.hxx"
#include "dptabsrc.hxx"
#include "dpsave.hxx"
#include "dpdimsave.hxx"
#include "dpoutput.hxx"
#include "dpshttab.hxx"
#include "dpsdbtab.hxx"
#include "dpgroup.hxx"
#include "document.hxx"
#include "rechead.hxx"
#include "pivot.hxx"
#include "dapiuno.hxx"
#include "miscuno.hxx"
#include "scerrors.hxx"
#include "refupdat.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "attrib.hxx"
#include "scitems.hxx"
#include "unonames.hxx"
#include "dpglobal.hxx"
#include "globstr.hrc"
#include "queryentry.hxx"
#include "dputil.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotTableHeaderData.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionData.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionType.hpp>
#include <com/sun/star/sheet/DimensionFlags.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/sheet/XDrillDownDataSupplier.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <sal/macros.h>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <svl/zforlist.hxx>
#include <vcl/msgbox.hxx>

#include <vector>
#include <memory>

#include <boost/unordered_map.hpp>

using namespace com::sun::star;
using ::std::vector;
using ::std::unary_function;
using ::boost::shared_ptr;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::sheet::DataPilotTableHeaderData;
using ::com::sun::star::sheet::DataPilotTablePositionData;
using ::com::sun::star::sheet::XDimensionsSupplier;
using ::com::sun::star::beans::XPropertySet;

#define SC_SERVICE_ROWSET           "com.sun.star.sdb.RowSet"

#define SC_DBPROP_DATASOURCENAME    "DataSourceName"
#define SC_DBPROP_COMMAND           "Command"
#define SC_DBPROP_COMMANDTYPE       "CommandType"



#define SCDPSOURCE_SERVICE  "com.sun.star.sheet.DataPilotSource"

namespace {

/**
 * Database connection implementation for UNO database API.  Note that in
 * the UNO database API, column index is 1-based, whereas the interface
 * requires that column index be 0-based.
 */
class DBConnector : public ScDPCache::DBConnector
{
    ScDPCache& mrCache;

    uno::Reference<sdbc::XRowSet> mxRowSet;
    uno::Reference<sdbc::XRow> mxRow;
    uno::Reference<sdbc::XResultSetMetaData> mxMetaData;
    Date maNullDate;

public:
    DBConnector(ScDPCache& rCache, const uno::Reference<sdbc::XRowSet>& xRowSet, const Date& rNullDate);

    bool isValid() const;

    virtual void getValue(long nCol, ScDPItemData &rData, short& rNumType) const;
    virtual OUString getColumnLabel(long nCol) const;
    virtual long getColumnCount() const;
    virtual bool first();
    virtual bool next();
    virtual void finish();
};

DBConnector::DBConnector(ScDPCache& rCache, const uno::Reference<sdbc::XRowSet>& xRowSet, const Date& rNullDate) :
    mrCache(rCache), mxRowSet(xRowSet), maNullDate(rNullDate)
{
    Reference<sdbc::XResultSetMetaDataSupplier> xMetaSupp(mxRowSet, UNO_QUERY);
    if (xMetaSupp.is())
        mxMetaData = xMetaSupp->getMetaData();

    mxRow.set(mxRowSet, UNO_QUERY);
}

bool DBConnector::isValid() const
{
    return mxRowSet.is() && mxRow.is() && mxMetaData.is();
}

bool DBConnector::first()
{
    return mxRowSet->first();
}

bool DBConnector::next()
{
    return mxRowSet->next();
}

void DBConnector::finish()
{
    mxRowSet->beforeFirst();
}

long DBConnector::getColumnCount() const
{
    return mxMetaData->getColumnCount();
}

OUString DBConnector::getColumnLabel(long nCol) const
{
    return mxMetaData->getColumnLabel(nCol+1);
}

void DBConnector::getValue(long nCol, ScDPItemData &rData, short& rNumType) const
{
    rNumType = NUMBERFORMAT_NUMBER;
    sal_Int32 nType = mxMetaData->getColumnType(nCol+1);

    try
    {
        double fValue = 0.0;
        switch (nType)
        {
            case sdbc::DataType::BIT:
            case sdbc::DataType::BOOLEAN:
            {
                rNumType = NUMBERFORMAT_LOGICAL;
                fValue  = mxRow->getBoolean(nCol+1) ? 1 : 0;
                rData.SetValue(fValue);
                break;
            }
            case sdbc::DataType::TINYINT:
            case sdbc::DataType::SMALLINT:
            case sdbc::DataType::INTEGER:
            case sdbc::DataType::BIGINT:
            case sdbc::DataType::FLOAT:
            case sdbc::DataType::REAL:
            case sdbc::DataType::DOUBLE:
            case sdbc::DataType::NUMERIC:
            case sdbc::DataType::DECIMAL:
            {
                
                fValue = mxRow->getDouble(nCol+1);
                rData.SetValue(fValue);
                break;
            }
            case sdbc::DataType::DATE:
            {
                rNumType = NUMBERFORMAT_DATE;

                util::Date aDate = mxRow->getDate(nCol+1);
                fValue = Date(aDate.Day, aDate.Month, aDate.Year) - maNullDate;
                rData.SetValue(fValue);
                break;
            }
            case sdbc::DataType::TIME:
            {
                rNumType = NUMBERFORMAT_TIME;

                util::Time aTime = mxRow->getTime(nCol+1);
                fValue = aTime.Hours       / static_cast<double>(::Time::hourPerDay)   +
                         aTime.Minutes     / static_cast<double>(::Time::minutePerDay) +
                         aTime.Seconds     / static_cast<double>(::Time::secondPerDay) +
                         aTime.NanoSeconds / static_cast<double>(::Time::nanoSecPerDay);
                rData.SetValue(fValue);
                break;
            }
            case sdbc::DataType::TIMESTAMP:
            {
                rNumType = NUMBERFORMAT_DATETIME;

                util::DateTime aStamp = mxRow->getTimestamp(nCol+1);
                fValue = ( Date( aStamp.Day, aStamp.Month, aStamp.Year ) - maNullDate ) +
                         aStamp.Hours       / static_cast<double>(::Time::hourPerDay)   +
                         aStamp.Minutes     / static_cast<double>(::Time::minutePerDay) +
                         aStamp.Seconds     / static_cast<double>(::Time::secondPerDay) +
                         aStamp.NanoSeconds / static_cast<double>(::Time::nanoSecPerDay);
                rData.SetValue(fValue);
                break;
            }
            case sdbc::DataType::CHAR:
            case sdbc::DataType::VARCHAR:
            case sdbc::DataType::LONGVARCHAR:
            case sdbc::DataType::SQLNULL:
            case sdbc::DataType::BINARY:
            case sdbc::DataType::VARBINARY:
            case sdbc::DataType::LONGVARBINARY:
            default:
                rData.SetString(mrCache.InternString(mxRow->getString(nCol+1)));
        }
    }
    catch (uno::Exception&)
    {
        rData.SetEmpty();
    }
}

}

sal_uInt16 lcl_GetDataGetOrientation( const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
    long nRet = sheet::DataPilotFieldOrientation_HIDDEN;
    if ( xSource.is() )
    {
        uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
        uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
        long nIntCount = xIntDims->getCount();
        sal_Bool bFound = false;
        for (long nIntDim=0; nIntDim<nIntCount && !bFound; nIntDim++)
        {
            uno::Reference<uno::XInterface> xIntDim =
                ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nIntDim) );
            uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
            if ( xDimProp.is() )
            {
                bFound = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                    OUString(SC_UNO_DP_ISDATALAYOUT) );
                
                if (bFound)
                    nRet = ScUnoHelpFunctions::GetEnumProperty(
                            xDimProp, OUString(SC_UNO_DP_ORIENTATION),
                            sheet::DataPilotFieldOrientation_HIDDEN );
            }
        }
    }
    return static_cast< sal_uInt16 >( nRet );
}

ScDPServiceDesc::ScDPServiceDesc(
    const OUString& rServ, const OUString& rSrc, const OUString& rNam,
    const OUString& rUser, const OUString& rPass ) :
    aServiceName( rServ ),
    aParSource( rSrc ),
    aParName( rNam ),
    aParUser( rUser ),
    aParPass( rPass ) {}

bool ScDPServiceDesc::operator== ( const ScDPServiceDesc& rOther ) const
{
    return aServiceName == rOther.aServiceName &&
        aParSource == rOther.aParSource &&
        aParName == rOther.aParName &&
        aParUser == rOther.aParUser &&
        aParPass == rOther.aParPass;
}

ScDPObject::ScDPObject( ScDocument* pD ) :
    pDoc( pD ),
    pSaveData( NULL ),
    pSheetDesc( NULL ),
    pImpDesc( NULL ),
    pServDesc( NULL ),
    mpTableData(static_cast<ScDPTableData*>(NULL)),
    pOutput( NULL ),
    mnAutoFormatIndex( 65535 ),
    nHeaderRows( 0 ),
    mbHeaderLayout(false),
    bAllowMove(false),
    bSettingsChanged(false),
    mbEnableGetPivotData(true)
{
}

ScDPObject::ScDPObject(const ScDPObject& r) :
    pDoc( r.pDoc ),
    pSaveData( NULL ),
    aTableName( r.aTableName ),
    aTableTag( r.aTableTag ),
    aOutRange( r.aOutRange ),
    pSheetDesc( NULL ),
    pImpDesc( NULL ),
    pServDesc( NULL ),
    mpTableData(static_cast<ScDPTableData*>(NULL)),
    pOutput( NULL ),
    mnAutoFormatIndex( r.mnAutoFormatIndex ),
    nHeaderRows( r.nHeaderRows ),
    mbHeaderLayout( r.mbHeaderLayout ),
    bAllowMove(false),
    bSettingsChanged(false),
    mbEnableGetPivotData(r.mbEnableGetPivotData)
{
    if (r.pSaveData)
        pSaveData = new ScDPSaveData(*r.pSaveData);
    if (r.pSheetDesc)
        pSheetDesc = new ScSheetSourceDesc(*r.pSheetDesc);
    if (r.pImpDesc)
        pImpDesc = new ScImportSourceDesc(*r.pImpDesc);
    if (r.pServDesc)
        pServDesc = new ScDPServiceDesc(*r.pServDesc);
    
}

ScDPObject::~ScDPObject()
{
    Clear();
}

ScDPObject& ScDPObject::operator= (const ScDPObject& r)
{
    Clear();

    pDoc = r.pDoc;
    aTableName = r.aTableName;
    aTableTag = r.aTableTag;
    aOutRange = r.aOutRange;
    mnAutoFormatIndex = r.mnAutoFormatIndex;
    nHeaderRows = r.nHeaderRows;
    mbHeaderLayout = r.mbHeaderLayout;
    bAllowMove = false;
    bSettingsChanged = false;
    mbEnableGetPivotData = r.mbEnableGetPivotData;

    if (r.pSaveData)
        pSaveData = new ScDPSaveData(*r.pSaveData);
    if (r.pSheetDesc)
        pSheetDesc = new ScSheetSourceDesc(*r.pSheetDesc);
    if (r.pImpDesc)
        pImpDesc = new ScImportSourceDesc(*r.pImpDesc);
    if (r.pServDesc)
        pServDesc = new ScDPServiceDesc(*r.pServDesc);

    return *this;
}

void ScDPObject::EnableGetPivotData(bool b)
{
    mbEnableGetPivotData = b;
}

void ScDPObject::SetAllowMove(bool bSet)
{
    bAllowMove = bSet;
}

void ScDPObject::SetSaveData(const ScDPSaveData& rData)
{
    if ( pSaveData != &rData )      
    {
        delete pSaveData;
        pSaveData = new ScDPSaveData( rData );
    }

    InvalidateData();       
}

void ScDPObject::SetHeaderLayout (bool bUseGrid)
{
    mbHeaderLayout = bUseGrid;
}

bool ScDPObject::GetHeaderLayout() const
{
    return mbHeaderLayout;
}

void ScDPObject::SetOutRange(const ScRange& rRange)
{
    aOutRange = rRange;

    if ( pOutput )
        pOutput->SetPosition( rRange.aStart );
}

void ScDPObject::SetSheetDesc(const ScSheetSourceDesc& rDesc, bool /*bFromRefUpdate*/)
{
    if ( pSheetDesc && rDesc == *pSheetDesc )
        return;             

    DELETEZ( pImpDesc );
    DELETEZ( pServDesc );

    delete pSheetDesc;
    pSheetDesc = new ScSheetSourceDesc(rDesc);

    

    const ScRange& rSrcRange = pSheetDesc->GetSourceRange();
    ScQueryParam aParam = pSheetDesc->GetQueryParam();
    aParam.nCol1 = rSrcRange.aStart.Col();
    aParam.nRow1 = rSrcRange.aStart.Row();
    aParam.nCol2 = rSrcRange.aEnd.Col();
    aParam.nRow2 = rSrcRange.aEnd.Row();
    aParam.bHasHeader = true;
    pSheetDesc->SetQueryParam(aParam);

    ClearTableData();      
}

void ScDPObject::SetImportDesc(const ScImportSourceDesc& rDesc)
{
    if ( pImpDesc && rDesc == *pImpDesc )
        return;             

    DELETEZ( pSheetDesc );
    DELETEZ( pServDesc );

    delete pImpDesc;
    pImpDesc = new ScImportSourceDesc(rDesc);

    ClearTableData();      
}

void ScDPObject::SetServiceData(const ScDPServiceDesc& rDesc)
{
    if ( pServDesc && rDesc == *pServDesc )
        return;             

    DELETEZ( pSheetDesc );
    DELETEZ( pImpDesc );

    delete pServDesc;
    pServDesc = new ScDPServiceDesc(rDesc);

    ClearTableData();      
}

void ScDPObject::WriteSourceDataTo( ScDPObject& rDest ) const
{
    if ( pSheetDesc )
        rDest.SetSheetDesc( *pSheetDesc );
    else if ( pImpDesc )
        rDest.SetImportDesc( *pImpDesc );
    else if ( pServDesc )
        rDest.SetServiceData( *pServDesc );

    

    rDest.aTableName = aTableName;
    rDest.aTableTag  = aTableTag;
}

void ScDPObject::WriteTempDataTo( ScDPObject& rDest ) const
{
    rDest.nHeaderRows = nHeaderRows;
}

bool ScDPObject::IsSheetData() const
{
    return ( pSheetDesc != NULL );
}

void ScDPObject::SetName(const OUString& rNew)
{
    aTableName = rNew;
}

void ScDPObject::SetTag(const OUString& rNew)
{
    aTableTag = rNew;
}

bool ScDPObject::IsDataDescriptionCell(const ScAddress& rPos)
{
    if (!pSaveData)
        return false;

    long nDataDimCount = pSaveData->GetDataDimensionCount();
    if (nDataDimCount != 1)
        
        
        return false;

    CreateOutput();
    ScRange aTabRange = pOutput->GetOutputRange(sheet::DataPilotOutputRangeType::TABLE);
    return (rPos == aTabRange.aStart);
}

uno::Reference<sheet::XDimensionsSupplier> ScDPObject::GetSource()
{
    CreateObjects();
    return xSource;
}

void ScDPObject::CreateOutput()
{
    CreateObjects();
    if (!pOutput)
    {
        sal_Bool bFilterButton = IsSheetData() && pSaveData && pSaveData->GetFilterButton();
        pOutput = new ScDPOutput( pDoc, xSource, aOutRange.aStart, bFilterButton );
        pOutput->SetHeaderLayout ( mbHeaderLayout );

        long nOldRows = nHeaderRows;
        nHeaderRows = pOutput->GetHeaderRows();

        if ( bAllowMove && nHeaderRows != nOldRows )
        {
            long nDiff = nOldRows - nHeaderRows;
            if ( nOldRows == 0 )
                --nDiff;
            if ( nHeaderRows == 0 )
                ++nDiff;

            long nNewRow = aOutRange.aStart.Row() + nDiff;
            if ( nNewRow < 0 )
                nNewRow = 0;

            ScAddress aStart( aOutRange.aStart );
            aStart.SetRow(nNewRow);
            pOutput->SetPosition( aStart );

            

            bAllowMove = false;     
        }
    }
}

namespace {

class DisableGetPivotData
{
    ScDPObject& mrDPObj;
    bool mbOldState;
public:
    DisableGetPivotData(ScDPObject& rObj, bool bOld) : mrDPObj(rObj), mbOldState(bOld)
    {
        mrDPObj.EnableGetPivotData(false);
    }

    ~DisableGetPivotData()
    {
        mrDPObj.EnableGetPivotData(mbOldState);
    }
};

class FindIntersectingTable : std::unary_function<ScDPObject, bool>
{
    ScRange maRange;
public:
    FindIntersectingTable(const ScRange& rRange) : maRange(rRange) {}

    bool operator() (const ScDPObject& rObj) const
    {
        return maRange.Intersects(rObj.GetOutRange());
    }
};

class FindIntersetingTableByColumns : std::unary_function<ScDPObject, bool>
{
    SCCOL mnCol1;
    SCCOL mnCol2;
    SCROW mnRow;
    SCTAB mnTab;
public:
    FindIntersetingTableByColumns(SCCOL nCol1, SCCOL nCol2, SCROW nRow, SCTAB nTab) :
        mnCol1(nCol1), mnCol2(nCol2), mnRow(nRow), mnTab(nTab) {}

    bool operator() (const ScDPObject& rObj) const
    {
        const ScRange& rRange = rObj.GetOutRange();
        if (mnTab != rRange.aStart.Tab())
            
            return false;

        if (rRange.aEnd.Row() < mnRow)
            
            return false;

        if (mnCol1 <= rRange.aStart.Col() && rRange.aEnd.Col() <= mnCol2)
            
            return false;

        if (rRange.aEnd.Col() < mnCol1 || mnCol2 < rRange.aStart.Col())
            
            return false;

        
        return true;
    }
};

class FindIntersectingTableByRows : std::unary_function<ScDPObject, bool>
{
    SCCOL mnCol;
    SCROW mnRow1;
    SCROW mnRow2;
    SCTAB mnTab;
public:
    FindIntersectingTableByRows(SCCOL nCol, SCROW nRow1, SCROW nRow2, SCTAB nTab) :
        mnCol(nCol), mnRow1(nRow1), mnRow2(nRow2), mnTab(nTab) {}

    bool operator() (const ScDPObject& rObj) const
    {
        const ScRange& rRange = rObj.GetOutRange();
        if (mnTab != rRange.aStart.Tab())
            
            return false;

        if (rRange.aEnd.Col() < mnCol)
            
            return false;

        if (mnRow1 <= rRange.aStart.Row() && rRange.aEnd.Row() <= mnRow2)
            
            return false;

        if (rRange.aEnd.Row() < mnRow1 || mnRow2 < rRange.aStart.Row())
            
            return false;

        
        return true;
    }
};

class AccumulateOutputRanges : std::unary_function<ScDPObject, void>
{
    ScRangeList maRanges;
    SCTAB mnTab;
public:
    AccumulateOutputRanges(SCTAB nTab) : mnTab(nTab) {}
    AccumulateOutputRanges(const AccumulateOutputRanges& r) : maRanges(r.maRanges), mnTab(r.mnTab) {}

    void operator() (const ScDPObject& rObj)
    {
        const ScRange& rRange = rObj.GetOutRange();
        if (mnTab != rRange.aStart.Tab())
            
            return;

        maRanges.Join(rRange);
    }

    ScRangeList getRanges() const { return maRanges; }
};

}

ScDPTableData* ScDPObject::GetTableData()
{
    if (!mpTableData)
    {
        shared_ptr<ScDPTableData> pData;
        const ScDPDimensionSaveData* pDimData = pSaveData ? pSaveData->GetExistingDimensionData() : NULL;

        if ( pImpDesc )
        {
            
            const ScDPCache* pCache = pImpDesc->CreateCache(pDimData);
            if (pCache)
            {
                pCache->AddReference(this);
                pData.reset(new ScDatabaseDPData(pDoc, *pCache));
            }
        }
        else
        {
            
            if (!pSheetDesc)
            {
                OSL_FAIL("no source descriptor");
                pSheetDesc = new ScSheetSourceDesc(pDoc);     
            }

            {
                
                
                
                DisableGetPivotData aSwitch(*this, mbEnableGetPivotData);
                const ScDPCache* pCache = pSheetDesc->CreateCache(pDimData);
                if (pCache)
                {
                    pCache->AddReference(this);
                    pData.reset(new ScSheetDPData(pDoc, *pSheetDesc, *pCache));
                }
            }
        }

        
        if (pData && pDimData)
        {
            shared_ptr<ScDPGroupTableData> pGroupData(new ScDPGroupTableData(pData, pDoc));
            pDimData->WriteToData(*pGroupData);
            pData = pGroupData;
        }

        mpTableData = pData;                        
    }

    return mpTableData.get();
}

void ScDPObject::CreateObjects()
{
    if (!xSource.is())
    {
        DELETEZ( pOutput );     

        if ( pServDesc )
        {
            xSource = CreateSource( *pServDesc );
        }

        if ( !xSource.is() )    
        {
            OSL_ENSURE( !pServDesc, "DPSource could not be created" );
            ScDPTableData* pData = GetTableData();
            if (pData)
            {
                if (pSaveData)
                    
                    
                    pData->SetEmptyFlags(pSaveData->GetIgnoreEmptyRows(), pSaveData->GetRepeatIfEmpty());

                pData->ReloadCacheTable();
                ScDPSource* pSource = new ScDPSource( pData );
                xSource = pSource;
            }
        }

        if (pSaveData)
            pSaveData->WriteToSource( xSource );
    }
    else if (bSettingsChanged)
    {
        DELETEZ( pOutput );     

        uno::Reference<util::XRefreshable> xRef( xSource, uno::UNO_QUERY );
        if (xRef.is())
        {
            try
            {
                xRef->refresh();
            }
            catch(uno::Exception&)
            {
                OSL_FAIL("exception in refresh");
            }
        }

        if (pSaveData)
            pSaveData->WriteToSource( xSource );
    }
    bSettingsChanged = false;
}

void ScDPObject::InvalidateData()
{
    bSettingsChanged = true;
}

void ScDPObject::Clear()
{
    delete pOutput;
    delete pSaveData;
    delete pSheetDesc;
    delete pImpDesc;
    delete pServDesc;
    pOutput = NULL;
    pSaveData = NULL;
    pSheetDesc = NULL;
    pImpDesc = NULL;
    pServDesc = NULL;
    ClearTableData();
}

void ScDPObject::ClearTableData()
{
    ClearSource();

    if (mpTableData)
        mpTableData->GetCacheTable().getCache()->RemoveReference(this);
    mpTableData.reset();
}

void ScDPObject::ReloadGroupTableData()
{
    ClearSource();

    if (!mpTableData)
        
        return;

    if (!pSaveData)
        
        return;

    const ScDPDimensionSaveData* pDimData = pSaveData->GetExistingDimensionData();
    if (!pDimData || !pDimData->HasGroupDimensions())
    {
        
        
        ScDPGroupTableData* pData = dynamic_cast<ScDPGroupTableData*>(mpTableData.get());
        if (pData)
        {
            
            shared_ptr<ScDPTableData> pSource = pData->GetSourceTableData();
            mpTableData = pSource;
        }
        return;
    }

    ScDPGroupTableData* pData = dynamic_cast<ScDPGroupTableData*>(mpTableData.get());
    if (pData)
    {
        
        
        shared_ptr<ScDPTableData> pSource = pData->GetSourceTableData();
        shared_ptr<ScDPGroupTableData> pGroupData(new ScDPGroupTableData(pSource, pDoc));
        pDimData->WriteToData(*pGroupData);
        mpTableData = pGroupData;
    }
    else
    {
        
        shared_ptr<ScDPGroupTableData> pGroupData(new ScDPGroupTableData(mpTableData, pDoc));
        pDimData->WriteToData(*pGroupData);
        mpTableData = pGroupData;
    }

    bSettingsChanged = true;
}

void ScDPObject::ClearSource()
{
    Reference< XComponent > xObjectComp( xSource, UNO_QUERY );
    if (xObjectComp.is())
    {
        try
        {
            xObjectComp->dispose();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    xSource = NULL;
}

ScRange ScDPObject::GetNewOutputRange( bool& rOverflow )
{
    CreateOutput();             

    rOverflow = pOutput->HasError();        
    if ( rOverflow )
        return ScRange( aOutRange.aStart );
    else
    {
        
        return pOutput->GetOutputRange();
    }
}

void ScDPObject::Output( const ScAddress& rPos )
{
    
    pDoc->DeleteAreaTab( aOutRange.aStart.Col(), aOutRange.aStart.Row(),
                         aOutRange.aEnd.Col(),   aOutRange.aEnd.Row(),
                         aOutRange.aStart.Tab(), IDF_ALL );
    pDoc->RemoveFlagsTab( aOutRange.aStart.Col(), aOutRange.aStart.Row(),
                          aOutRange.aEnd.Col(),   aOutRange.aEnd.Row(),
                          aOutRange.aStart.Tab(), SC_MF_AUTO );

    CreateOutput();             

    pOutput->SetPosition( rPos );

    pOutput->Output();

    
    aOutRange = pOutput->GetOutputRange();
    const ScAddress& s = aOutRange.aStart;
    const ScAddress& e = aOutRange.aEnd;
    pDoc->ApplyFlagsTab(s.Col(), s.Row(), e.Col(), e.Row(), s.Tab(), SC_MF_DP_TABLE);
}

const ScRange ScDPObject::GetOutputRangeByType( sal_Int32 nType )
{
    CreateOutput();

    if (pOutput->HasError())
        return ScRange(aOutRange.aStart);

    return pOutput->GetOutputRange(nType);
}

static sal_Bool lcl_HasButton( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    return ((const ScMergeFlagAttr*)pDoc->GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->HasPivotButton();
}

void ScDPObject::RefreshAfterLoad()
{
    
    

    

    SCCOL nFirstCol = aOutRange.aStart.Col();
    SCROW nFirstRow = aOutRange.aStart.Row();
    SCTAB nTab = aOutRange.aStart.Tab();

    SCROW nInitial = 0;
    SCROW nOutRows = aOutRange.aEnd.Row() + 1 - aOutRange.aStart.Row();
    while ( nInitial + 1 < nOutRows && lcl_HasButton( pDoc, nFirstCol, nFirstRow + nInitial, nTab ) )
        ++nInitial;

    if ( nInitial + 1 < nOutRows &&
        pDoc->IsBlockEmpty( nTab, nFirstCol, nFirstRow + nInitial, nFirstCol, nFirstRow + nInitial ) &&
        aOutRange.aEnd.Col() > nFirstCol )
    {
        nHeaderRows = nInitial;
    }
    else
        nHeaderRows = 0;        
}

void ScDPObject::BuildAllDimensionMembers()
{
    if (!pSaveData)
        return;

    
    if (pServDesc)
        return;

    ScDPTableData* pTableData = GetTableData();
    if(pTableData)
        pSaveData->BuildAllDimensionMembers(pTableData);
}

bool ScDPObject::SyncAllDimensionMembers()
{
    if (!pSaveData)
        return false;

    
    
    if (pServDesc)
        return false;

    ScDPTableData* pData = GetTableData();
    if (!pData)
        
        
        return false;

    
    pData->SetEmptyFlags(pSaveData->GetIgnoreEmptyRows(), pSaveData->GetRepeatIfEmpty());
    pData->ReloadCacheTable();
    pSaveData->SyncAllDimensionMembers(pData);
    return true;
}

bool ScDPObject::GetMemberNames( sal_Int32 nDim, Sequence<OUString>& rNames )
{
    vector<ScDPLabelData::Member> aMembers;
    if (!GetMembers(nDim, GetUsedHierarchy(nDim), aMembers))
        return false;

    size_t n = aMembers.size();
    rNames.realloc(n);
    for (size_t i = 0; i < n; ++i)
        rNames[i] = aMembers[i].maName;

    return true;
}

bool ScDPObject::GetMembers( sal_Int32 nDim, sal_Int32 nHier, vector<ScDPLabelData::Member>& rMembers )
{
    Reference< container::XNameAccess > xMembersNA;
    if (!GetMembersNA( nDim, nHier, xMembersNA ))
        return false;

    Reference<container::XIndexAccess> xMembersIA( new ScNameToIndexAccess(xMembersNA) );
    sal_Int32 nCount = xMembersIA->getCount();
    vector<ScDPLabelData::Member> aMembers;
    aMembers.reserve(nCount);

    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        Reference<container::XNamed> xMember(xMembersIA->getByIndex(i), UNO_QUERY);
        ScDPLabelData::Member aMem;

        if (xMember.is())
            aMem.maName = xMember->getName();

        Reference<beans::XPropertySet> xMemProp(xMember, UNO_QUERY);
        if (xMemProp.is())
        {
            aMem.mbVisible     = ScUnoHelpFunctions::GetBoolProperty(xMemProp, OUString(SC_UNO_DP_ISVISIBLE));
            aMem.mbShowDetails = ScUnoHelpFunctions::GetBoolProperty(xMemProp, OUString(SC_UNO_DP_SHOWDETAILS));

            aMem.maLayoutName = ScUnoHelpFunctions::GetStringProperty(
                xMemProp, OUString(SC_UNO_DP_LAYOUTNAME), OUString());
        }

        aMembers.push_back(aMem);
    }
    rMembers.swap(aMembers);
    return true;
}

void ScDPObject::UpdateReference( UpdateRefMode eUpdateRefMode,
                                     const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    

    SCCOL nCol1 = aOutRange.aStart.Col();
    SCROW nRow1 = aOutRange.aStart.Row();
    SCTAB nTab1 = aOutRange.aStart.Tab();
    SCCOL nCol2 = aOutRange.aEnd.Col();
    SCROW nRow2 = aOutRange.aEnd.Row();
    SCTAB nTab2 = aOutRange.aEnd.Tab();

    ScRefUpdateRes eRes =
        ScRefUpdate::Update( pDoc, eUpdateRefMode,
            rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
            rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aEnd.Tab(), nDx, nDy, nDz,
            nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    if ( eRes != UR_NOTHING )
        SetOutRange( ScRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 ) );

    

    if ( pSheetDesc )
    {
        const OUString& rRangeName = pSheetDesc->GetRangeName();
        if (!rRangeName.isEmpty())
            
            return;

        const ScRange& rSrcRange = pSheetDesc->GetSourceRange();
        nCol1 = rSrcRange.aStart.Col();
        nRow1 = rSrcRange.aStart.Row();
        nTab1 = rSrcRange.aStart.Tab();
        nCol2 = rSrcRange.aEnd.Col();
        nRow2 = rSrcRange.aEnd.Row();
        nTab2 = rSrcRange.aEnd.Tab();

        eRes = ScRefUpdate::Update( pDoc, eUpdateRefMode,
                rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
                rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aEnd.Tab(), nDx, nDy, nDz,
                nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        if ( eRes != UR_NOTHING )
        {
            SCsCOL nDiffX = nCol1 - pSheetDesc->GetSourceRange().aStart.Col();
            SCsROW nDiffY = nRow1 - pSheetDesc->GetSourceRange().aStart.Row();

            ScQueryParam aParam = pSheetDesc->GetQueryParam();
            aParam.nCol1 = sal::static_int_cast<SCCOL>( aParam.nCol1 + nDiffX );
            aParam.nCol2 = sal::static_int_cast<SCCOL>( aParam.nCol2 + nDiffX );
            aParam.nRow1 += nDiffY; 
            aParam.nRow2 += nDiffY; 
            SCSIZE nEC = aParam.GetEntryCount();
            for (SCSIZE i=0; i<nEC; i++)
                if (aParam.GetEntry(i).bDoQuery)
                    aParam.GetEntry(i).nField += nDiffX;

            pSheetDesc->SetQueryParam(aParam);
            pSheetDesc->SetSourceRange(ScRange(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2));
        }
    }
}

bool ScDPObject::RefsEqual( const ScDPObject& r ) const
{
    if ( aOutRange != r.aOutRange )
        return false;

    if ( pSheetDesc && r.pSheetDesc )
    {
        if ( pSheetDesc->GetSourceRange() != r.pSheetDesc->GetSourceRange() )
            return false;
    }
    else if ( pSheetDesc || r.pSheetDesc )
    {
        OSL_FAIL("RefsEqual: SheetDesc set at only one object");
        return false;
    }

    return true;
}

void ScDPObject::WriteRefsTo( ScDPObject& r ) const
{
    r.SetOutRange( aOutRange );
    if ( pSheetDesc )
        r.SetSheetDesc( *pSheetDesc, true );
}

void ScDPObject::GetPositionData(const ScAddress& rPos, DataPilotTablePositionData& rPosData)
{
    CreateOutput();
    pOutput->GetPositionData(rPos, rPosData);
}

bool ScDPObject::GetDataFieldPositionData(
    const ScAddress& rPos, Sequence<sheet::DataPilotFieldFilter>& rFilters)
{
    CreateOutput();

    vector<sheet::DataPilotFieldFilter> aFilters;
    if (!pOutput->GetDataResultPositionData(aFilters, rPos))
        return false;

    sal_Int32 n = static_cast<sal_Int32>(aFilters.size());
    rFilters.realloc(n);
    for (sal_Int32 i = 0; i < n; ++i)
        rFilters[i] = aFilters[i];

    return true;
}

void ScDPObject::GetDrillDownData(const ScAddress& rPos, Sequence< Sequence<Any> >& rTableData)
{
    CreateOutput();

    Reference<sheet::XDrillDownDataSupplier> xDrillDownData(xSource, UNO_QUERY);
    if (!xDrillDownData.is())
        return;

    Sequence<sheet::DataPilotFieldFilter> filters;
    if (!GetDataFieldPositionData(rPos, filters))
        return;

    rTableData = xDrillDownData->getDrillDownData(filters);
}

bool ScDPObject::IsDimNameInUse(const OUString& rName) const
{
    if (!xSource.is())
        return false;

    Reference<container::XNameAccess> xDims = xSource->getDimensions();
    Sequence<OUString> aDimNames = xDims->getElementNames();
    sal_Int32 n = aDimNames.getLength();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        const OUString& rDimName = aDimNames[i];
        if (rDimName.equalsIgnoreAsciiCase(rName))
            return true;

        Reference<beans::XPropertySet> xPropSet(xDims->getByName(rDimName), UNO_QUERY);
        if (!xPropSet.is())
            continue;

        OUString aLayoutName = ScUnoHelpFunctions::GetStringProperty(
            xPropSet, OUString(SC_UNO_DP_LAYOUTNAME), OUString());
        if (aLayoutName.equalsIgnoreAsciiCase(rName))
            return true;
    }
    return false;
}

OUString ScDPObject::GetDimName( long nDim, bool& rIsDataLayout, sal_Int32* pFlags )
{
    rIsDataLayout = false;
    OUString aRet;

    if ( xSource.is() )
    {
        uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
        uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
        long nDimCount = xDims->getCount();
        if ( nDim < nDimCount )
        {
            uno::Reference<uno::XInterface> xIntDim =
                ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
            uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
            uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
            if ( xDimName.is() && xDimProp.is() )
            {
                sal_Bool bData = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                                OUString(SC_UNO_DP_ISDATALAYOUT) );
                

                OUString aName;
                try
                {
                    aName = xDimName->getName();
                }
                catch(uno::Exception&)
                {
                }
                if ( bData )
                    rIsDataLayout = true;
                else
                    aRet = aName;

                if (pFlags)
                    *pFlags = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                OUString(SC_UNO_DP_FLAGS), 0 );
            }
        }
    }

    return aRet;
}

bool ScDPObject::IsDuplicated( long nDim )
{
    bool bDuplicated = false;
    if ( xSource.is() )
    {
        uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
        uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
        long nDimCount = xDims->getCount();
        if ( nDim < nDimCount )
        {
            uno::Reference<uno::XInterface> xIntDim =
                ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
            uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
            if ( xDimProp.is() )
            {
                try
                {
                    uno::Any aOrigAny = xDimProp->getPropertyValue(
                                OUString(SC_UNO_DP_ORIGINAL) );
                    uno::Reference<uno::XInterface> xIntOrig;
                    if ( (aOrigAny >>= xIntOrig) && xIntOrig.is() )
                        bDuplicated = true;
                }
                catch(uno::Exception&)
                {
                }
            }
        }
    }
    return bDuplicated;
}

long ScDPObject::GetDimCount()
{
    long nRet = 0;
    if ( xSource.is() )
    {
        try
        {
            uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
            if ( xDimsName.is() )
                nRet = xDimsName->getElementNames().getLength();
        }
        catch(uno::Exception&)
        {
        }
    }
    return nRet;
}

void ScDPObject::GetHeaderPositionData(const ScAddress& rPos, DataPilotTableHeaderData& rData)
{
    using namespace ::com::sun::star::sheet::DataPilotTablePositionType;

    CreateOutput();             

    
    rData.Dimension = rData.Hierarchy = rData.Level = -1;
    rData.Flags = 0;

    DataPilotTablePositionData aPosData;
    pOutput->GetPositionData(rPos, aPosData);
    const sal_Int32 nPosType = aPosData.PositionType;
    if (nPosType == COLUMN_HEADER || nPosType == ROW_HEADER)
        aPosData.PositionData >>= rData;
}

namespace {

class FindByName : std::unary_function<const ScDPSaveDimension*, bool>
{
    OUString maName; 
public:
    FindByName(const OUString& rName) : maName(rName) {}
    bool operator() (const ScDPSaveDimension* pDim) const
    {
        
        const OUString* pLayoutName = pDim->GetLayoutName();
        if (pLayoutName && ScGlobal::pCharClass->uppercase(*pLayoutName) == maName)
            return true;

        sheet::GeneralFunction eGenFunc = static_cast<sheet::GeneralFunction>(pDim->GetFunction());
        ScSubTotalFunc eFunc = ScDPUtil::toSubTotalFunc(eGenFunc);
        OUString aSrcName = ScDPUtil::getSourceDimensionName(pDim->GetName());
        OUString aFuncName = ScDPUtil::getDisplayedMeasureName(aSrcName, eFunc);
        if (maName == ScGlobal::pCharClass->uppercase(aFuncName))
            return true;

        return maName == ScGlobal::pCharClass->uppercase(aSrcName);
    }
};

class LessByDimOrder : std::binary_function<sheet::DataPilotFieldFilter, sheet::DataPilotFieldFilter, bool>
{
    const ScDPSaveData::DimOrderType& mrDimOrder;

public:
    LessByDimOrder(const ScDPSaveData::DimOrderType& rDimOrder) : mrDimOrder(rDimOrder) {}

    bool operator() (const sheet::DataPilotFieldFilter& r1, const sheet::DataPilotFieldFilter& r2) const
    {
        size_t nRank1 = mrDimOrder.size();
        size_t nRank2 = mrDimOrder.size();
        ScDPSaveData::DimOrderType::const_iterator it1 = mrDimOrder.find(r1.FieldName);
        if (it1 != mrDimOrder.end())
            nRank1 = it1->second;

        ScDPSaveData::DimOrderType::const_iterator it2 = mrDimOrder.find(r2.FieldName);
        if (it2 != mrDimOrder.end())
            nRank2 = it2->second;

        return nRank1 < nRank2;
    }
};

}

double ScDPObject::GetPivotData(const OUString& rDataFieldName, std::vector<sheet::DataPilotFieldFilter>& rFilters)
{
    double fRet;
    rtl::math::setNan(&fRet);
    if (!mbEnableGetPivotData)
        return fRet;

    CreateObjects();

    std::vector<const ScDPSaveDimension*> aDataDims;
    pSaveData->GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_DATA, aDataDims);
    if (aDataDims.empty())
        return fRet;

    std::vector<const ScDPSaveDimension*>::iterator it = std::find_if(
        aDataDims.begin(), aDataDims.end(),
        FindByName(ScGlobal::pCharClass->uppercase(rDataFieldName)));

    if (it == aDataDims.end())
        return fRet;

    size_t nDataIndex = std::distance(aDataDims.begin(), it);

    uno::Reference<sheet::XDataPilotResults> xDPResults(xSource, uno::UNO_QUERY);
    if (!xDPResults.is())
        return fRet;

    
    
    std::sort(rFilters.begin(), rFilters.end(), LessByDimOrder(pSaveData->GetDimensionSortOrder()));

    size_t n = rFilters.size();
    uno::Sequence<sheet::DataPilotFieldFilter> aFilters(n);
    for (size_t i = 0; i < n; ++i)
        aFilters[i] = rFilters[i];

    uno::Sequence<double> aRes = xDPResults->getFilteredResults(aFilters);
    if (static_cast<sal_Int32>(nDataIndex) >= aRes.getLength())
        return fRet;

    return aRes[nDataIndex];
}

bool ScDPObject::IsFilterButton( const ScAddress& rPos )
{
    CreateOutput();             

    return pOutput->IsFilterButton( rPos );
}

long ScDPObject::GetHeaderDim( const ScAddress& rPos, sal_uInt16& rOrient )
{
    CreateOutput();             

    return pOutput->GetHeaderDim( rPos, rOrient );
}

bool ScDPObject::GetHeaderDrag( const ScAddress& rPos, bool bMouseLeft, bool bMouseTop, long nDragDim,
                                Rectangle& rPosRect, sal_uInt16& rOrient, long& rDimPos )
{
    CreateOutput();             

    return pOutput->GetHeaderDrag( rPos, bMouseLeft, bMouseTop, nDragDim, rPosRect, rOrient, rDimPos );
}

void ScDPObject::GetMemberResultNames(ScDPUniqueStringSet& rNames, long nDimension)
{
    CreateOutput();             

    pOutput->GetMemberResultNames(rNames, nDimension);    
}

namespace {

bool dequote( const OUString& rSource, sal_Int32 nStartPos, sal_Int32& rEndPos, OUString& rResult )
{
    

    bool bRet = false;
    const sal_Unicode cQuote = '\'';

    if (rSource[nStartPos] == cQuote)
    {
        OUStringBuffer aBuffer;
        sal_Int32 nPos = nStartPos + 1;
        const sal_Int32 nLen = rSource.getLength();

        while ( nPos < nLen )
        {
            const sal_Unicode cNext = rSource[nPos];
            if ( cNext == cQuote )
            {
                if (nPos+1 < nLen && rSource[nPos+1] == cQuote)
                {
                    
                    aBuffer.append( cNext );    
                    ++nPos;                     
                }
                else
                {
                    
                    rResult = aBuffer.makeStringAndClear();
                    rEndPos = nPos + 1;         
                    return true;
                }
            }
            else
                aBuffer.append( cNext );

            ++nPos;
        }
        
    }

    return bRet;
}

struct ScGetPivotDataFunctionEntry
{
    const sal_Char*         pName;
    sheet::GeneralFunction  eFunc;
};

bool parseFunction( const OUString& rList, sal_Int32 nStartPos, sal_Int32& rEndPos, sheet::GeneralFunction& rFunc )
{
    static const ScGetPivotDataFunctionEntry aFunctions[] =
    {
        
        { "Sum",        sheet::GeneralFunction_SUM       },
        { "Count",      sheet::GeneralFunction_COUNT     },
        { "Average",    sheet::GeneralFunction_AVERAGE   },
        { "Max",        sheet::GeneralFunction_MAX       },
        { "Min",        sheet::GeneralFunction_MIN       },
        { "Product",    sheet::GeneralFunction_PRODUCT   },
        { "CountNums",  sheet::GeneralFunction_COUNTNUMS },
        { "StDev",      sheet::GeneralFunction_STDEV     },
        { "StDevp",     sheet::GeneralFunction_STDEVP    },
        { "Var",        sheet::GeneralFunction_VAR       },
        { "VarP",       sheet::GeneralFunction_VARP      },
        
        { "Count Nums", sheet::GeneralFunction_COUNTNUMS },
        { "StdDev",     sheet::GeneralFunction_STDEV     },
        { "StdDevp",    sheet::GeneralFunction_STDEVP    }
    };

    const sal_Int32 nListLen = rList.getLength();
    while (nStartPos < nListLen && rList[nStartPos] == ' ')
        ++nStartPos;

    bool bParsed = false;
    bool bFound = false;
    OUString aFuncStr;
    sal_Int32 nFuncEnd = 0;
    if (nStartPos < nListLen && rList[nStartPos] == '\'')
        bParsed = dequote( rList, nStartPos, nFuncEnd, aFuncStr );
    else
    {
        nFuncEnd = rList.indexOf(']', nStartPos);
        if (nFuncEnd >= 0)
        {
            aFuncStr = rList.copy(nStartPos, nFuncEnd - nStartPos);
            bParsed = true;
        }
    }

    if ( bParsed )
    {
        aFuncStr = comphelper::string::strip(aFuncStr, ' ');

        const sal_Int32 nFuncCount = sizeof(aFunctions) / sizeof(aFunctions[0]);
        for ( sal_Int32 nFunc=0; nFunc<nFuncCount && !bFound; nFunc++ )
        {
            if (aFuncStr.equalsIgnoreAsciiCaseAscii(aFunctions[nFunc].pName))
            {
                rFunc = aFunctions[nFunc].eFunc;
                bFound = true;

                while (nFuncEnd < nListLen && rList[nFuncEnd] == ' ')
                    ++nFuncEnd;
                rEndPos = nFuncEnd;
            }
        }
    }

    return bFound;
}

bool isAtStart(
    const OUString& rList, const OUString& rSearch, sal_Int32& rMatched,
    bool bAllowBracket, sheet::GeneralFunction* pFunc )
{
    sal_Int32 nMatchList = 0;
    sal_Int32 nMatchSearch = 0;
    sal_Unicode cFirst = rList[0];
    if ( cFirst == '\'' || cFirst == '[' )
    {
        

        OUString aDequoted;
        sal_Int32 nQuoteEnd = 0;
        bool bParsed = false;

        if ( cFirst == '\'' )
            bParsed = dequote( rList, 0, nQuoteEnd, aDequoted );
        else if ( cFirst == '[' )
        {
            

            sal_Int32 nStartPos = 1;
            const sal_Int32 nListLen = rList.getLength();
            while (nStartPos < nListLen && rList[nStartPos] == ' ')
                ++nStartPos;

            if (nStartPos < nListLen && rList[nStartPos] == '\'')         
            {
                if ( dequote( rList, nStartPos, nQuoteEnd, aDequoted ) )
                {
                    
                    
                    while (nQuoteEnd < nListLen && rList[nQuoteEnd] == ' ')
                        ++nQuoteEnd;

                    
                    if (nQuoteEnd < nListLen && rList[nQuoteEnd] == ';' && pFunc)
                    {
                        sal_Int32 nFuncEnd = 0;
                        if ( parseFunction( rList, nQuoteEnd + 1, nFuncEnd, *pFunc ) )
                            nQuoteEnd = nFuncEnd;
                    }
                    if (nQuoteEnd < nListLen && rList[nQuoteEnd] == ']')
                    {
                        ++nQuoteEnd;        
                        bParsed = true;
                    }
                }
            }
            else
            {
                

                sal_Int32 nClosePos = rList.indexOf(']', nStartPos);
                if (nClosePos >= 0)
                {
                    sal_Int32 nNameEnd = nClosePos;
                    sal_Int32 nSemiPos = rList.indexOf(';', nStartPos);
                    if (nSemiPos >= 0 && nSemiPos < nClosePos && pFunc)
                    {
                        sal_Int32 nFuncEnd = 0;
                        if (parseFunction(rList, nSemiPos+1, nFuncEnd, *pFunc))
                            nNameEnd = nSemiPos;
                    }

                    aDequoted = rList.copy(nStartPos, nNameEnd - nStartPos);
                    
                    aDequoted = comphelper::string::stripEnd(aDequoted, ' ');
                    nQuoteEnd = nClosePos + 1;
                    bParsed = true;
                }
            }
        }

        if ( bParsed && ScGlobal::GetpTransliteration()->isEqual( aDequoted, rSearch ) )
        {
            nMatchList = nQuoteEnd;             
            nMatchSearch = rSearch.getLength();
        }
    }
    else
    {
        
        ScGlobal::GetpTransliteration()->equals(
            rList, 0, rList.getLength(), nMatchList, rSearch, 0, rSearch.getLength(), nMatchSearch);
    }

    if (nMatchSearch == rSearch.getLength())
    {
        

        bool bValid = false;
        if ( sal::static_int_cast<sal_Int32>(nMatchList) >= rList.getLength() )
            bValid = true;
        else
        {
            sal_Unicode cNext = rList[nMatchList];
            if ( cNext == ' ' || ( bAllowBracket && cNext == '[' ) )
                bValid = true;
        }

        if ( bValid )
        {
            rMatched = nMatchList;
            return true;
        }
    }

    return false;
}

} 

bool ScDPObject::ParseFilters(
    OUString& rDataFieldName,
    std::vector<sheet::DataPilotFieldFilter>& rFilters,
    std::vector<sheet::GeneralFunction>& rFilterFuncs, const OUString& rFilterList )
{
    

    CreateObjects();            

    std::vector<OUString> aDataNames;     
    std::vector<OUString> aGivenNames;    
    std::vector<OUString> aFieldNames;    
    std::vector< uno::Sequence<OUString> > aFieldValues;

    //
    
    //

    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
    sal_Int32 nDimCount = xIntDims->getCount();
    for ( sal_Int32 nDim = 0; nDim<nDimCount; nDim++ )
    {
        uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nDim) );
        uno::Reference<container::XNamed> xDim( xIntDim, uno::UNO_QUERY );
        uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
        uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDim, uno::UNO_QUERY );
        sal_Bool bDataLayout = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                            OUString(SC_UNO_DP_ISDATALAYOUT) );
        sal_Int32 nOrient = ScUnoHelpFunctions::GetEnumProperty(
                            xDimProp, OUString(SC_UNO_DP_ORIENTATION),
                            sheet::DataPilotFieldOrientation_HIDDEN );
        if ( !bDataLayout )
        {
            if ( nOrient == sheet::DataPilotFieldOrientation_DATA )
            {
                OUString aSourceName;
                OUString aGivenName;
                ScDPOutput::GetDataDimensionNames( aSourceName, aGivenName, xIntDim );
                aDataNames.push_back( aSourceName );
                aGivenNames.push_back( aGivenName );
            }
            else if ( nOrient != sheet::DataPilotFieldOrientation_HIDDEN )
            {
                

                uno::Reference<container::XIndexAccess> xHiers = new ScNameToIndexAccess( xDimSupp->getHierarchies() );
                sal_Int32 nHierarchy = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                                    OUString(SC_UNO_DP_USEDHIERARCHY) );
                if ( nHierarchy >= xHiers->getCount() )
                    nHierarchy = 0;

                uno::Reference<uno::XInterface> xHier = ScUnoHelpFunctions::AnyToInterface(
                                                    xHiers->getByIndex(nHierarchy) );
                uno::Reference<sheet::XLevelsSupplier> xHierSupp( xHier, uno::UNO_QUERY );
                if ( xHierSupp.is() )
                {
                    uno::Reference<container::XIndexAccess> xLevels = new ScNameToIndexAccess( xHierSupp->getLevels() );
                    sal_Int32 nLevCount = xLevels->getCount();
                    for (sal_Int32 nLev=0; nLev<nLevCount; nLev++)
                    {
                        uno::Reference<uno::XInterface> xLevel = ScUnoHelpFunctions::AnyToInterface(
                                                            xLevels->getByIndex(nLev) );
                        uno::Reference<container::XNamed> xLevNam( xLevel, uno::UNO_QUERY );
                        uno::Reference<sheet::XMembersSupplier> xLevSupp( xLevel, uno::UNO_QUERY );
                        if ( xLevNam.is() && xLevSupp.is() )
                        {
                            uno::Reference<container::XNameAccess> xMembers = xLevSupp->getMembers();

                            OUString aFieldName( xLevNam->getName() );
                            uno::Sequence<OUString> aMemberNames( xMembers->getElementNames() );

                            aFieldNames.push_back( aFieldName );
                            aFieldValues.push_back( aMemberNames );
                        }
                    }
                }
            }
        }
    }

    //
    
    //

    SCSIZE nDataFields = aDataNames.size();
    SCSIZE nFieldCount = aFieldNames.size();
    OSL_ENSURE( aGivenNames.size() == nDataFields && aFieldValues.size() == nFieldCount, "wrong count" );

    bool bError = false;
    bool bHasData = false;
    OUString aRemaining(comphelper::string::strip(rFilterList, ' '));
    while (!aRemaining.isEmpty() && !bError)
    {
        bool bUsed = false;

        

        for ( SCSIZE nDataPos=0; nDataPos<nDataFields && !bUsed; nDataPos++ )
        {
            OUString aFound;
            sal_Int32 nMatched = 0;
            if (isAtStart(aRemaining, aDataNames[nDataPos], nMatched, false, NULL))
                aFound = aDataNames[nDataPos];
            else if (isAtStart(aRemaining, aGivenNames[nDataPos], nMatched, false, NULL))
                aFound = aGivenNames[nDataPos];

            if (!aFound.isEmpty())
            {
                rDataFieldName = aFound;
                aRemaining = aRemaining.copy(nMatched);
                bHasData = true;
                bUsed = true;
            }
        }

        

        OUString aSpecField;
        bool bHasFieldName = false;
        if ( !bUsed )
        {
            sal_Int32 nMatched = 0;
            for ( SCSIZE nField=0; nField<nFieldCount && !bHasFieldName; nField++ )
            {
                if (isAtStart(aRemaining, aFieldNames[nField], nMatched, true, NULL))
                {
                    aSpecField = aFieldNames[nField];
                    aRemaining = aRemaining.copy(nMatched);
                    aRemaining = comphelper::string::stripStart(aRemaining, ' ');

                    
                    if (aRemaining.startsWith("["))
                    {
                        bHasFieldName = true;
                        
                    }
                    else
                    {
                        bUsed = true;
                        bError = true;
                    }
                }
            }
        }

        

        if ( !bUsed )
        {
            bool bItemFound = false;
            sal_Int32 nMatched = 0;
            OUString aFoundName;
            OUString aFoundValue;
            sheet::GeneralFunction eFunc = sheet::GeneralFunction_NONE;
            sheet::GeneralFunction eFoundFunc = sheet::GeneralFunction_NONE;

            for ( SCSIZE nField=0; nField<nFieldCount; nField++ )
            {
                
                
                if ( !bHasFieldName || aFieldNames[nField] == aSpecField )
                {
                    const uno::Sequence<OUString>& rItems = aFieldValues[nField];
                    sal_Int32 nItemCount = rItems.getLength();
                    const OUString* pItemArr = rItems.getConstArray();
                    for ( sal_Int32 nItem=0; nItem<nItemCount; nItem++ )
                    {
                        if ( isAtStart( aRemaining, pItemArr[nItem], nMatched, false, &eFunc ) )
                        {
                            if ( bItemFound )
                                bError = true;      
                            else
                            {
                                aFoundName = aFieldNames[nField];
                                aFoundValue = pItemArr[nItem];
                                eFoundFunc = eFunc;
                                bItemFound = true;
                                bUsed = true;
                            }
                        }
                    }
                }
            }

            if ( bItemFound && !bError )
            {
                sheet::DataPilotFieldFilter aField;
                aField.FieldName = aFoundName;
                aField.MatchValue = aFoundValue;
                rFilters.push_back(aField);
                rFilterFuncs.push_back(eFoundFunc);
                aRemaining = aRemaining.copy(nMatched);
            }
        }

        if ( !bUsed )
            bError = true;

        
        aRemaining = comphelper::string::stripStart(aRemaining, ' ');
    }

    if ( !bError && !bHasData && aDataNames.size() == 1 )
    {
        
        rDataFieldName = aDataNames[0];
        bHasData = true;
    }

    return bHasData && !bError;
}

void ScDPObject::ToggleDetails(const DataPilotTableHeaderData& rElemDesc, ScDPObject* pDestObj)
{
    CreateObjects();            

    

    uno::Reference<container::XNamed> xDim;
    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
    long nIntCount = xIntDims->getCount();
    if ( rElemDesc.Dimension < nIntCount )
    {
        uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface(
                                    xIntDims->getByIndex(rElemDesc.Dimension) );
        xDim = uno::Reference<container::XNamed>( xIntDim, uno::UNO_QUERY );
    }
    OSL_ENSURE( xDim.is(), "dimension not found" );
    if ( !xDim.is() ) return;
    OUString aDimName = xDim->getName();

    uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
    sal_Bool bDataLayout = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                        OUString(SC_UNO_DP_ISDATALAYOUT) );
    if (bDataLayout)
    {
        
        
        return;
    }

    

    long nHierCount = 0;
    uno::Reference<container::XIndexAccess> xHiers;
    uno::Reference<sheet::XHierarchiesSupplier> xHierSupp( xDim, uno::UNO_QUERY );
    if ( xHierSupp.is() )
    {
        uno::Reference<container::XNameAccess> xHiersName = xHierSupp->getHierarchies();
        xHiers = new ScNameToIndexAccess( xHiersName );
        nHierCount = xHiers->getCount();
    }
    uno::Reference<uno::XInterface> xHier;
    if ( rElemDesc.Hierarchy < nHierCount )
        xHier = ScUnoHelpFunctions::AnyToInterface( xHiers->getByIndex(rElemDesc.Hierarchy) );
    OSL_ENSURE( xHier.is(), "hierarchy not found" );
    if ( !xHier.is() ) return;

    long nLevCount = 0;
    uno::Reference<container::XIndexAccess> xLevels;
    uno::Reference<sheet::XLevelsSupplier> xLevSupp( xHier, uno::UNO_QUERY );
    if ( xLevSupp.is() )
    {
        uno::Reference<container::XNameAccess> xLevsName = xLevSupp->getLevels();
        xLevels = new ScNameToIndexAccess( xLevsName );
        nLevCount = xLevels->getCount();
    }
    uno::Reference<uno::XInterface> xLevel;
    if ( rElemDesc.Level < nLevCount )
        xLevel = ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex(rElemDesc.Level) );
    OSL_ENSURE( xLevel.is(), "level not found" );
    if ( !xLevel.is() ) return;

    uno::Reference<container::XNameAccess> xMembers;
    uno::Reference<sheet::XMembersSupplier> xMbrSupp( xLevel, uno::UNO_QUERY );
    if ( xMbrSupp.is() )
        xMembers = xMbrSupp->getMembers();

    sal_Bool bFound = false;
    sal_Bool bShowDetails = sal_True;

    if ( xMembers.is() )
    {
        if ( xMembers->hasByName(rElemDesc.MemberName) )
        {
            uno::Reference<uno::XInterface> xMemberInt = ScUnoHelpFunctions::AnyToInterface(
                                            xMembers->getByName(rElemDesc.MemberName) );
            uno::Reference<beans::XPropertySet> xMbrProp( xMemberInt, uno::UNO_QUERY );
            if ( xMbrProp.is() )
            {
                bShowDetails = ScUnoHelpFunctions::GetBoolProperty( xMbrProp,
                                    OUString(SC_UNO_DP_SHOWDETAILS) );
                
                bFound = sal_True;
            }
        }
    }

    OSL_ENSURE( bFound, "member not found" );
    (void)bFound;

    

    
    ScDPSaveData* pModifyData = pDestObj ? ( pDestObj->pSaveData ) : pSaveData;
    OSL_ENSURE( pModifyData, "no data?" );
    if ( pModifyData )
    {
        const OUString aName = rElemDesc.MemberName;
        pModifyData->GetDimensionByName(aDimName)->
            GetMemberByName(aName)->SetShowDetails( !bShowDetails );    

        if ( pDestObj )
            pDestObj->InvalidateData();     
        else
            InvalidateData();               
    }
}

static sal_uInt16 lcl_FirstSubTotal( const uno::Reference<beans::XPropertySet>& xDimProp )     
{
    uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDimProp, uno::UNO_QUERY );
    if ( xDimProp.is() && xDimSupp.is() )
    {
        uno::Reference<container::XIndexAccess> xHiers = new ScNameToIndexAccess( xDimSupp->getHierarchies() );
        long nHierarchy = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                OUString(SC_UNO_DP_USEDHIERARCHY) );
        if ( nHierarchy >= xHiers->getCount() )
            nHierarchy = 0;

        uno::Reference<uno::XInterface> xHier = ScUnoHelpFunctions::AnyToInterface(
                                    xHiers->getByIndex(nHierarchy) );
        uno::Reference<sheet::XLevelsSupplier> xHierSupp( xHier, uno::UNO_QUERY );
        if ( xHierSupp.is() )
        {
            uno::Reference<container::XIndexAccess> xLevels = new ScNameToIndexAccess( xHierSupp->getLevels() );
            uno::Reference<uno::XInterface> xLevel =
                ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex( 0 ) );
            uno::Reference<beans::XPropertySet> xLevProp( xLevel, uno::UNO_QUERY );
            if ( xLevProp.is() )
            {
                uno::Any aSubAny;
                try
                {
                    aSubAny = xLevProp->getPropertyValue(
                            OUString(SC_UNO_DP_SUBTOTAL) );
                }
                catch(uno::Exception&)
                {
                }
                uno::Sequence<sheet::GeneralFunction> aSeq;
                if ( aSubAny >>= aSeq )
                {
                    sal_uInt16 nMask = 0;
                    const sheet::GeneralFunction* pArray = aSeq.getConstArray();
                    long nCount = aSeq.getLength();
                    for (long i=0; i<nCount; i++)
                        nMask |= ScDataPilotConversion::FunctionBit(pArray[i]);
                    return nMask;
                }
            }
        }
    }

    OSL_FAIL("FirstSubTotal: NULL");
    return 0;
}

namespace {

class FindByColumn : public std::unary_function<ScPivotField, bool>
{
    SCsCOL mnCol;
    sal_uInt16 mnMask;
public:
    FindByColumn(SCsCOL nCol, sal_uInt16 nMask) : mnCol(nCol), mnMask(nMask) {}
    bool operator() (const ScPivotField& r) const
    {
        return r.nCol == mnCol && r.nFuncMask == mnMask;
    }
};

}

void lcl_FillOldFields( ScPivotFieldVector& rFields,
    const uno::Reference<sheet::XDimensionsSupplier>& xSource,
    sal_uInt16 nOrient, bool bAddData )
{
    ScPivotFieldVector aFields;

    bool bDataFound = false;

    
    

    vector<long> aPos;

    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
    long nDimCount = xDims->getCount();
    for (long nDim = 0; nDim < nDimCount; ++nDim)
    {
        
        uno::Reference<uno::XInterface> xIntDim =
            ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );

        
        uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );

        
        long nDimOrient = ScUnoHelpFunctions::GetEnumProperty(
                            xDimProp, OUString(SC_UNO_DP_ORIENTATION),
                            sheet::DataPilotFieldOrientation_HIDDEN );

        if ( xDimProp.is() && nDimOrient == nOrient )
        {
            

            
            sal_uInt16 nMask = 0;
            if ( nOrient == sheet::DataPilotFieldOrientation_DATA )
            {
                sheet::GeneralFunction eFunc = (sheet::GeneralFunction)ScUnoHelpFunctions::GetEnumProperty(
                                            xDimProp, OUString(SC_UNO_DP_FUNCTION),
                                            sheet::GeneralFunction_NONE );
                if ( eFunc == sheet::GeneralFunction_AUTO )
                {
                    
                    eFunc = sheet::GeneralFunction_SUM;
                }
                nMask = ScDataPilotConversion::FunctionBit(eFunc);
            }
            else
                nMask = lcl_FirstSubTotal( xDimProp );      

            
            bool bDataLayout = ScUnoHelpFunctions::GetBoolProperty(
                xDimProp, OUString(SC_UNO_DP_ISDATALAYOUT));

            
            long nDupSource = -1;
            try
            {
                uno::Any aOrigAny = xDimProp->getPropertyValue(
                    OUString(SC_UNO_DP_ORIGINAL_POS));
                sal_Int32 nTmp = 0;
                if (aOrigAny >>= nTmp)
                    nDupSource = static_cast<sal_Int32>(nTmp);
            }
            catch(uno::Exception&)
            {
            }

            sal_uInt8 nDupCount = 0;
            if (nDupSource >= 0)
            {
                

                SCsCOL nCompCol; 
                if ( bDataLayout )
                    nCompCol = PIVOT_DATA_FIELD;
                else
                    nCompCol = static_cast<SCsCOL>(nDupSource);     

                ScPivotFieldVector::iterator it = std::find_if(aFields.begin(), aFields.end(), FindByColumn(nCompCol, nMask));
                if (it != aFields.end())
                    nDupCount = it->mnDupCount + 1;
            }

            aFields.push_back(ScPivotField());
            ScPivotField& rField = aFields.back();
            if (bDataLayout)
            {
                rField.nCol = PIVOT_DATA_FIELD;
                bDataFound = true;
            }
            else
            {
                rField.mnOriginalDim = nDupSource;
                rField.nCol = static_cast<SCCOL>(nDim);    
            }

            rField.nFuncMask = nMask;
            rField.mnDupCount = nDupCount;
            long nPos = ScUnoHelpFunctions::GetLongProperty(
                xDimProp, OUString(SC_UNO_DP_POSITION));
            aPos.push_back(nPos);

            try
            {
                if (nOrient == sheet::DataPilotFieldOrientation_DATA)
                    xDimProp->getPropertyValue(OUString(SC_UNO_DP_REFVALUE))
                        >>= rField.maFieldRef;
            }
            catch (uno::Exception&)
            {
            }
        }
    }

    

    size_t nOutCount = aFields.size();
    if (nOutCount >= 1)
    {
        for (size_t i = 0; i < nOutCount - 1; ++i)
        {
            for (size_t j = 0; j + i < nOutCount - 1; ++j)
            {
                if ( aPos[j+1] < aPos[j] )
                {
                    std::swap( aPos[j], aPos[j+1] );
                    std::swap( aFields[j], aFields[j+1] );
                }
            }
        }
    }

    if (bAddData && !bDataFound)
        aFields.push_back(ScPivotField(PIVOT_DATA_FIELD, 0));

    rFields.swap(aFields);
}

bool ScDPObject::FillOldParam(ScPivotParam& rParam) const
{
    ((ScDPObject*)this)->CreateObjects();       

    if (!xSource.is())
        return false;

    rParam.nCol = aOutRange.aStart.Col();
    rParam.nRow = aOutRange.aStart.Row();
    rParam.nTab = aOutRange.aStart.Tab();
    

    bool bAddData = ( lcl_GetDataGetOrientation( xSource ) == sheet::DataPilotFieldOrientation_HIDDEN );
    lcl_FillOldFields(
        rParam.maPageFields, xSource, sheet::DataPilotFieldOrientation_PAGE, false);
    lcl_FillOldFields(
        rParam.maColFields, xSource, sheet::DataPilotFieldOrientation_COLUMN, bAddData);
    lcl_FillOldFields(
        rParam.maRowFields, xSource, sheet::DataPilotFieldOrientation_ROW, false);
    lcl_FillOldFields(
        rParam.maDataFields, xSource, sheet::DataPilotFieldOrientation_DATA, false);

    uno::Reference<beans::XPropertySet> xProp( xSource, uno::UNO_QUERY );
    if (xProp.is())
    {
        try
        {
            rParam.bMakeTotalCol = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        OUString(SC_UNO_DP_COLGRAND), true );
            rParam.bMakeTotalRow = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        OUString(SC_UNO_DP_ROWGRAND), true );

            
            rParam.bIgnoreEmptyRows = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        OUString(SC_UNO_DP_IGNOREEMPTY) );
            rParam.bDetectCategories = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        OUString(SC_UNO_DP_REPEATEMPTY) );
        }
        catch(uno::Exception&)
        {
            
        }
    }
    return true;
}

static void lcl_FillLabelData( ScDPLabelData& rData, const uno::Reference< beans::XPropertySet >& xDimProp )
{
    uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDimProp, uno::UNO_QUERY );
    if (!xDimProp.is() || !xDimSupp.is())
        return;

    uno::Reference<container::XIndexAccess> xHiers = new ScNameToIndexAccess( xDimSupp->getHierarchies() );
    long nHierarchy = ScUnoHelpFunctions::GetLongProperty(
        xDimProp, OUString(SC_UNO_DP_USEDHIERARCHY));
    if ( nHierarchy >= xHiers->getCount() )
        nHierarchy = 0;
    rData.mnUsedHier = nHierarchy;

    uno::Reference<uno::XInterface> xHier =
        ScUnoHelpFunctions::AnyToInterface(xHiers->getByIndex(nHierarchy));

    uno::Reference<sheet::XLevelsSupplier> xHierSupp( xHier, uno::UNO_QUERY );
    if (!xHierSupp.is())
        return;

    uno::Reference<container::XIndexAccess> xLevels =
        new ScNameToIndexAccess( xHierSupp->getLevels() );

    uno::Reference<uno::XInterface> xLevel =
        ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex(0) );
    uno::Reference<beans::XPropertySet> xLevProp( xLevel, uno::UNO_QUERY );
    if (!xLevProp.is())
        return;

    rData.mbShowAll = ScUnoHelpFunctions::GetBoolProperty(
        xLevProp, OUString(SC_UNO_DP_SHOWEMPTY));

    try
    {
        xLevProp->getPropertyValue( OUString( SC_UNO_DP_SORTING ) )
            >>= rData.maSortInfo;
        xLevProp->getPropertyValue( OUString( SC_UNO_DP_LAYOUT ) )
            >>= rData.maLayoutInfo;
        xLevProp->getPropertyValue( OUString( SC_UNO_DP_AUTOSHOW ) )
            >>= rData.maShowInfo;
    }
    catch(uno::Exception&)
    {
    }
}

bool ScDPObject::FillLabelDataForDimension(
    const uno::Reference<container::XIndexAccess>& xDims, sal_Int32 nDim, ScDPLabelData& rLabelData)
{
    uno::Reference<uno::XInterface> xIntDim =
        ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
    uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
    uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );

    if (!xDimName.is() || !xDimProp.is())
        return false;

    bool bData = ScUnoHelpFunctions::GetBoolProperty(
        xDimProp, OUString(SC_UNO_DP_ISDATALAYOUT));
    

    sal_Int32 nOrigPos = -1;
    OUString aFieldName;
    try
    {
        aFieldName = xDimName->getName();
        uno::Any aOrigAny = xDimProp->getPropertyValue(
                    OUString(SC_UNO_DP_ORIGINAL_POS));
        aOrigAny >>= nOrigPos;
    }
    catch(uno::Exception&)
    {
    }

    OUString aLayoutName = ScUnoHelpFunctions::GetStringProperty(
        xDimProp, OUString(SC_UNO_DP_LAYOUTNAME), OUString());

    OUString aSubtotalName = ScUnoHelpFunctions::GetStringProperty(
        xDimProp, OUString(SC_UNO_DP_FIELD_SUBTOTALNAME), OUString());

    bool bIsValue = true;                               

    
    

    sal_uInt8 nDupCount = ScDPUtil::getDuplicateIndex(aFieldName);
    aFieldName = ScDPUtil::getSourceDimensionName(aFieldName);

    rLabelData.maName = aFieldName;
    rLabelData.mnCol = static_cast<SCCOL>(nDim);
    rLabelData.mnDupCount = nDupCount;
    rLabelData.mbDataLayout = bData;
    rLabelData.mbIsValue = bIsValue;

    if (!bData)
    {
        rLabelData.mnOriginalDim = static_cast<long>(nOrigPos);
        rLabelData.maLayoutName = aLayoutName;
        rLabelData.maSubtotalName = aSubtotalName;
        if (nOrigPos >= 0)
            
            nDim = nOrigPos;
        GetHierarchies(nDim, rLabelData.maHiers);
        GetMembers(nDim, GetUsedHierarchy(nDim), rLabelData.maMembers);
        lcl_FillLabelData(rLabelData, xDimProp);
        rLabelData.mnFlags = ScUnoHelpFunctions::GetLongProperty(
            xDimProp, OUString(SC_UNO_DP_FLAGS), 0);
    }
    return true;
}

bool ScDPObject::FillLabelData(sal_Int32 nDim, ScDPLabelData& rLabels)
{
    CreateObjects();
    if (!xSource.is())
        return false;

    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
    sal_Int32 nDimCount = xDims->getCount();
    if (nDimCount <= 0 || nDim >= nDimCount)
        return false;

    return FillLabelDataForDimension(xDims, nDim, rLabels);
}

bool ScDPObject::FillLabelData(ScPivotParam& rParam)
{
    rParam.maLabelArray.clear();

    CreateObjects();
    if (!xSource.is())
        return false;

    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
    sal_Int32 nDimCount = xDims->getCount();
    if (nDimCount <= 0)
        return false;

    for (sal_Int32 nDim = 0; nDim < nDimCount; ++nDim)
    {
        std::auto_ptr<ScDPLabelData> pNewLabel(new ScDPLabelData);
        FillLabelDataForDimension(xDims, nDim, *pNewLabel);
        rParam.maLabelArray.push_back(pNewLabel);
    }

    return true;
}

bool ScDPObject::GetHierarchiesNA( sal_Int32 nDim, uno::Reference< container::XNameAccess >& xHiers )
{
    bool bRet = false;
    uno::Reference<container::XNameAccess> xDimsName( GetSource()->getDimensions() );
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    if( xIntDims.is() )
    {
        uno::Reference<sheet::XHierarchiesSupplier> xHierSup(xIntDims->getByIndex( nDim ), uno::UNO_QUERY);
        if (xHierSup.is())
        {
            xHiers.set( xHierSup->getHierarchies() );
            bRet = xHiers.is();
        }
    }
    return bRet;
}

bool ScDPObject::GetHierarchies( sal_Int32 nDim, uno::Sequence< OUString >& rHiers )
{
    bool bRet = false;
    uno::Reference< container::XNameAccess > xHiersNA;
    if( GetHierarchiesNA( nDim, xHiersNA ) )
    {
        rHiers = xHiersNA->getElementNames();
        bRet = true;
    }
    return bRet;
}

sal_Int32 ScDPObject::GetUsedHierarchy( sal_Int32 nDim )
{
    sal_Int32 nHier = 0;
    uno::Reference<container::XNameAccess> xDimsName( GetSource()->getDimensions() );
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    uno::Reference<beans::XPropertySet> xDim(xIntDims->getByIndex( nDim ), uno::UNO_QUERY);
    if (xDim.is())
        nHier = ScUnoHelpFunctions::GetLongProperty( xDim, OUString( SC_UNO_DP_USEDHIERARCHY ) );
    return nHier;
}

bool ScDPObject::GetMembersNA( sal_Int32 nDim, uno::Reference< container::XNameAccess >& xMembers )
{
    return GetMembersNA( nDim, GetUsedHierarchy( nDim ), xMembers );
}

bool ScDPObject::GetMembersNA( sal_Int32 nDim, sal_Int32 nHier, uno::Reference< container::XNameAccess >& xMembers )
{
    bool bRet = false;
    uno::Reference<container::XNameAccess> xDimsName( GetSource()->getDimensions() );
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    uno::Reference<beans::XPropertySet> xDim(xIntDims->getByIndex( nDim ), uno::UNO_QUERY);
    if (xDim.is())
    {
        uno::Reference<sheet::XHierarchiesSupplier> xHierSup(xDim, uno::UNO_QUERY);
        if (xHierSup.is())
        {
            uno::Reference<container::XIndexAccess> xHiers(new ScNameToIndexAccess(xHierSup->getHierarchies()));
            uno::Reference<sheet::XLevelsSupplier> xLevSupp( xHiers->getByIndex(nHier), uno::UNO_QUERY );
            if ( xLevSupp.is() )
            {
                uno::Reference<container::XIndexAccess> xLevels(new ScNameToIndexAccess( xLevSupp->getLevels()));
                if (xLevels.is())
                {
                    sal_Int32 nLevCount = xLevels->getCount();
                    if (nLevCount > 0)
                    {
                        uno::Reference<sheet::XMembersSupplier> xMembSupp( xLevels->getByIndex(0), uno::UNO_QUERY );
                        if ( xMembSupp.is() )
                        {
                            xMembers.set(xMembSupp->getMembers());
                            bRet = true;
                        }
                    }
                }
            }
        }
    }
    return bRet;
}




namespace {

OUString lcl_GetDimName( const uno::Reference<sheet::XDimensionsSupplier>& xSource, long nDim )
{
    OUString aName;
    if ( xSource.is() )
    {
        uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
        uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
        long nDimCount = xDims->getCount();
        if ( nDim < nDimCount )
        {
            uno::Reference<uno::XInterface> xIntDim =
                ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
            uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
            if (xDimName.is())
            {
                try
                {
                    aName = xDimName->getName();
                }
                catch(uno::Exception&)
                {
                }
            }
        }
    }
    return aName;
}

bool hasFieldColumn(const vector<ScPivotField>* pRefFields, SCCOL nCol)
{
    if (!pRefFields)
        return false;

    vector<ScPivotField>::const_iterator itr = pRefFields->begin(), itrEnd = pRefFields->end();
    for (; itr != itrEnd; ++itr)
    {
        if (itr->nCol == nCol)
            
            return true;
    }
    return false;
}

class FindByOriginalDim : public std::unary_function<ScPivotField, bool>
{
    long mnDim;
public:
    FindByOriginalDim(long nDim) : mnDim(nDim) {}
    bool operator() (const ScPivotField& r) const
    {
        return mnDim == r.getOriginalDim();
    }
};

}

void ScDPObject::ConvertOrientation(
    ScDPSaveData& rSaveData, const ScPivotFieldVector& rFields, sal_uInt16 nOrient,
    const Reference<XDimensionsSupplier>& xSource,
    const ScDPLabelDataVector& rLabels,
    const ScPivotFieldVector* pRefColFields,
    const ScPivotFieldVector* pRefRowFields,
    const ScPivotFieldVector* pRefPageFields )
{
    ScPivotFieldVector::const_iterator itr, itrBeg = rFields.begin(), itrEnd = rFields.end();
    for (itr = itrBeg; itr != itrEnd; ++itr)
    {
        const ScPivotField& rField = *itr;

        long nCol = rField.getOriginalDim();
        sal_uInt16 nFuncs = rField.nFuncMask;
        const sheet::DataPilotFieldReference& rFieldRef = rField.maFieldRef;

        ScDPSaveDimension* pDim = NULL;
        if ( nCol == PIVOT_DATA_FIELD )
            pDim = rSaveData.GetDataLayoutDimension();
        else
        {
            OUString aDocStr = lcl_GetDimName( xSource, nCol );   
            if (!aDocStr.isEmpty())
                pDim = rSaveData.GetDimensionByName(aDocStr);
            else
                pDim = NULL;
        }

        if (!pDim)
            continue;

        if ( nOrient == sheet::DataPilotFieldOrientation_DATA )     
        {
            
            bool bFirst = true;

            
            
            if (hasFieldColumn(pRefColFields, nCol))
                bFirst = false;

            if (bFirst && hasFieldColumn(pRefRowFields, nCol))
                bFirst = false;

            if (bFirst && hasFieldColumn(pRefPageFields, nCol))
                bFirst = false;

            if (bFirst)
            {
                
                
                bFirst = std::find_if(itrBeg, itr, FindByOriginalDim(nCol)) == itr;
            }

            sheet::GeneralFunction eFunc = ScDataPilotConversion::FirstFunc(rField.nFuncMask);
            if (!bFirst)
                pDim = rSaveData.DuplicateDimension(pDim->GetName());
            pDim->SetOrientation(nOrient);
            pDim->SetFunction(sal::static_int_cast<sal_uInt16>(eFunc));

            if( rFieldRef.ReferenceType == sheet::DataPilotFieldReferenceType::NONE )
                pDim->SetReferenceValue(0);
            else
                pDim->SetReferenceValue(&rFieldRef);
        }
        else                                            
        {
            pDim->SetOrientation( nOrient );

            sal_uInt16 nFuncArray[16];
            sal_uInt16 nFuncCount = 0;
            sal_uInt16 nMask = 1;
            for (sal_uInt16 nBit=0; nBit<16; nBit++)
            {
                if ( nFuncs & nMask )
                    nFuncArray[nFuncCount++] = sal::static_int_cast<sal_uInt16>(ScDataPilotConversion::FirstFunc( nMask ));
                nMask *= 2;
            }
            pDim->SetSubTotals( nFuncCount, nFuncArray );

            
            
            if ( nCol == PIVOT_DATA_FIELD )
                pDim->SetShowEmpty( true );
        }

        size_t nDimIndex = rField.nCol;
        pDim->RemoveLayoutName();
        pDim->RemoveSubtotalName();
        if (nDimIndex < rLabels.size())
        {
            const ScDPLabelData& rLabel = rLabels[nDimIndex];
            if (!rLabel.maLayoutName.isEmpty())
                pDim->SetLayoutName(rLabel.maLayoutName);
            if (!rLabel.maSubtotalName.isEmpty())
                pDim->SetSubtotalName(rLabel.maSubtotalName);
        }
    }
}

bool ScDPObject::IsOrientationAllowed( sal_uInt16 nOrient, sal_Int32 nDimFlags )
{
    bool bAllowed = true;
    switch (nOrient)
    {
        case sheet::DataPilotFieldOrientation_PAGE:
            bAllowed = ( nDimFlags & sheet::DimensionFlags::NO_PAGE_ORIENTATION ) == 0;
            break;
        case sheet::DataPilotFieldOrientation_COLUMN:
            bAllowed = ( nDimFlags & sheet::DimensionFlags::NO_COLUMN_ORIENTATION ) == 0;
            break;
        case sheet::DataPilotFieldOrientation_ROW:
            bAllowed = ( nDimFlags & sheet::DimensionFlags::NO_ROW_ORIENTATION ) == 0;
            break;
        case sheet::DataPilotFieldOrientation_DATA:
            bAllowed = ( nDimFlags & sheet::DimensionFlags::NO_DATA_ORIENTATION ) == 0;
            break;
        default:
            {
                
            }
    }
    return bAllowed;
}



bool ScDPObject::HasRegisteredSources()
{
    bool bFound = false;

    uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
    uno::Reference<container::XContentEnumerationAccess> xEnAc( xManager, uno::UNO_QUERY );
    if ( xEnAc.is() )
    {
        uno::Reference<container::XEnumeration> xEnum = xEnAc->createContentEnumeration(
                                        OUString( SCDPSOURCE_SERVICE ) );
        if ( xEnum.is() && xEnum->hasMoreElements() )
            bFound = true;
    }

    return bFound;
}

uno::Sequence<OUString> ScDPObject::GetRegisteredSources()
{
    uno::Sequence<OUString> aSeq(0);

    

    uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
    uno::Reference<container::XContentEnumerationAccess> xEnAc( xManager, uno::UNO_QUERY );
    if ( xEnAc.is() )
    {
        uno::Reference<container::XEnumeration> xEnum = xEnAc->createContentEnumeration(
                                        OUString( SCDPSOURCE_SERVICE ) );
        if ( xEnum.is() )
        {
            long nCount = 0;
            while ( xEnum->hasMoreElements() )
            {
                uno::Any aAddInAny = xEnum->nextElement();

                {
                    uno::Reference<uno::XInterface> xIntFac;
                    aAddInAny >>= xIntFac;
                    if ( xIntFac.is() )
                    {
                        uno::Reference<lang::XServiceInfo> xInfo( xIntFac, uno::UNO_QUERY );
                        if ( xInfo.is() )
                        {
                            OUString sName = xInfo->getImplementationName();

                            aSeq.realloc( nCount+1 );
                            aSeq.getArray()[nCount] = sName;
                            ++nCount;
                        }
                    }
                }
            }
        }
    }

    return aSeq;
}

uno::Reference<sheet::XDimensionsSupplier> ScDPObject::CreateSource( const ScDPServiceDesc& rDesc )
{
    OUString aImplName = rDesc.aServiceName;
    uno::Reference<sheet::XDimensionsSupplier> xRet = NULL;

    uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
    uno::Reference<container::XContentEnumerationAccess> xEnAc(xManager, uno::UNO_QUERY);
    if (!xEnAc.is())
        return xRet;

    uno::Reference<container::XEnumeration> xEnum =
        xEnAc->createContentEnumeration(OUString(SCDPSOURCE_SERVICE));
    if (!xEnum.is())
        return xRet;

    while (xEnum->hasMoreElements() && !xRet.is())
    {
        uno::Any aAddInAny = xEnum->nextElement();
        uno::Reference<uno::XInterface> xIntFac;
        aAddInAny >>= xIntFac;
        if (!xIntFac.is())
            continue;

        uno::Reference<lang::XServiceInfo> xInfo(xIntFac, uno::UNO_QUERY);
        if (!xInfo.is() || xInfo->getImplementationName() != aImplName)
            continue;

        try
        {
            
            

            uno::Reference<uno::XInterface> xInterface;
            uno::Reference<uno::XComponentContext> xCtx(
                comphelper::getComponentContext(xManager));
            uno::Reference<lang::XSingleComponentFactory> xCFac( xIntFac, uno::UNO_QUERY );
            if (xCFac.is())
                xInterface = xCFac->createInstanceWithContext(xCtx);

            if (!xInterface.is())
            {
                uno::Reference<lang::XSingleServiceFactory> xFac( xIntFac, uno::UNO_QUERY );
                if ( xFac.is() )
                    xInterface = xFac->createInstance();
            }

            uno::Reference<lang::XInitialization> xInit( xInterface, uno::UNO_QUERY );
            if (xInit.is())
            {
                
                uno::Sequence<uno::Any> aSeq(4);
                uno::Any* pArray = aSeq.getArray();
                pArray[0] <<= OUString( rDesc.aParSource );
                pArray[1] <<= OUString( rDesc.aParName );
                pArray[2] <<= OUString( rDesc.aParUser );
                pArray[3] <<= OUString( rDesc.aParPass );
                xInit->initialize( aSeq );
            }
            xRet = uno::Reference<sheet::XDimensionsSupplier>( xInterface, uno::UNO_QUERY );
        }
        catch(uno::Exception&)
        {
        }
    }

    return xRet;
}

#if DEBUG_PIVOT_TABLE
void ScDPObject::DumpCache() const
{
    if (!mpTableData)
        return;

    const ScDPCache* pCache = mpTableData->GetCacheTable().getCache();
    if (!pCache)
        return;

    pCache->Dump();
}
#endif

ScDPCollection::SheetCaches::SheetCaches(ScDocument* pDoc) : mpDoc(pDoc) {}

namespace {

struct FindInvalidRange : public std::unary_function<ScRange, bool>
{
    bool operator() (const ScRange& r) const
    {
        return !r.IsValid();
    }
};

void setGroupItemsToCache( ScDPCache& rCache, const std::set<ScDPObject*>& rRefs )
{
    
    std::set<ScDPObject*>::const_iterator itRef = rRefs.begin(), itRefEnd = rRefs.end();
    for (; itRef != itRefEnd; ++itRef)
    {
        const ScDPObject* pObj = *itRef;
        const ScDPSaveData* pSave = pObj->GetSaveData();
        if (!pSave)
            continue;

        const ScDPDimensionSaveData* pGroupDims = pSave->GetExistingDimensionData();
        if (!pGroupDims)
            continue;

        pGroupDims->WriteToCache(rCache);
    }
}

}

bool ScDPCollection::SheetCaches::hasCache(const ScRange& rRange) const
{
    RangeIndexType::const_iterator it = std::find(maRanges.begin(), maRanges.end(), rRange);
    if (it == maRanges.end())
        return false;

    
    size_t nIndex = std::distance(maRanges.begin(), it);
    CachesType::const_iterator itCache = maCaches.find(nIndex);
    return itCache != maCaches.end();
}

const ScDPCache* ScDPCollection::SheetCaches::getCache(const ScRange& rRange, const ScDPDimensionSaveData* pDimData)
{
    RangeIndexType::iterator it = std::find(maRanges.begin(), maRanges.end(), rRange);
    if (it != maRanges.end())
    {
        
        size_t nIndex = std::distance(maRanges.begin(), it);
        CachesType::iterator itCache = maCaches.find(nIndex);
        if (itCache == maCaches.end())
        {
            OSL_FAIL("Cache pool and index pool out-of-sync !!!");
            return NULL;
        }

        if (pDimData)
            pDimData->WriteToCache(*itCache->second);

        return itCache->second;
    }

    
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScDPCache> pCache(new ScDPCache(mpDoc));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    pCache->InitFromDoc(mpDoc, rRange);
    if (pDimData)
        pDimData->WriteToCache(*pCache);

    
    it = std::find_if(maRanges.begin(), maRanges.end(), FindInvalidRange());

    size_t nIndex = maRanges.size();
    if (it == maRanges.end())
    {
        
        maRanges.push_back(rRange);
    }
    else
    {
        
        *it = rRange;
        nIndex = std::distance(maRanges.begin(), it);
    }

    const ScDPCache* p = pCache.get();
    maCaches.insert(nIndex, pCache);
    return p;
}

ScDPCache* ScDPCollection::SheetCaches::getExistingCache(const ScRange& rRange)
{
    RangeIndexType::iterator it = std::find(maRanges.begin(), maRanges.end(), rRange);
    if (it == maRanges.end())
        
        return NULL;

    
    size_t nIndex = std::distance(maRanges.begin(), it);
    CachesType::iterator itCache = maCaches.find(nIndex);
    if (itCache == maCaches.end())
    {
        OSL_FAIL("Cache pool and index pool out-of-sync !!!");
        return NULL;
    }

    return itCache->second;
}

size_t ScDPCollection::SheetCaches::size() const
{
    return maCaches.size();
}

void ScDPCollection::SheetCaches::updateReference(
    UpdateRefMode eMode, const ScRange& r, SCsCOL nDx, SCsROW nDy, SCsTAB nDz)
{
    if (maRanges.empty())
        
        return;

    RangeIndexType::iterator it = maRanges.begin(), itEnd = maRanges.end();
    for (; it != itEnd; ++it)
    {
        const ScRange& rKeyRange = *it;
        SCCOL nCol1 = rKeyRange.aStart.Col();
        SCROW nRow1 = rKeyRange.aStart.Row();
        SCTAB nTab1 = rKeyRange.aStart.Tab();
        SCCOL nCol2 = rKeyRange.aEnd.Col();
        SCROW nRow2 = rKeyRange.aEnd.Row();
        SCTAB nTab2 = rKeyRange.aEnd.Tab();

        ScRefUpdateRes eRes = ScRefUpdate::Update(
            mpDoc, eMode,
            r.aStart.Col(), r.aStart.Row(), r.aStart.Tab(),
            r.aEnd.Col(), r.aEnd.Row(), r.aEnd.Tab(), nDx, nDy, nDz,
            nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);

        if (eRes != UR_NOTHING)
        {
            
            ScRange aNew(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            *it = aNew;
        }
    }
}

void ScDPCollection::SheetCaches::updateCache(const ScRange& rRange, std::set<ScDPObject*>& rRefs)
{
    RangeIndexType::iterator it = std::find(maRanges.begin(), maRanges.end(), rRange);
    if (it == maRanges.end())
    {
        
        rRefs.clear();
        return;
    }

    size_t nIndex = std::distance(maRanges.begin(), it);
    CachesType::iterator itCache = maCaches.find(nIndex);
    if (itCache == maCaches.end())
    {
        OSL_FAIL("Cache pool and index pool out-of-sync !!!");
        rRefs.clear();
        return;
    }

    ScDPCache& rCache = *itCache->second;

    
    rCache.InitFromDoc(mpDoc, rRange);

    std::set<ScDPObject*> aRefs(rCache.GetAllReferences());
    rRefs.swap(aRefs);

    
    setGroupItemsToCache(rCache, rRefs);
}

bool ScDPCollection::SheetCaches::remove(const ScDPCache* p)
{
    CachesType::iterator it = maCaches.begin(), itEnd = maCaches.end();
    for (; it != itEnd; ++it)
    {
        if (it->second == p)
        {
            size_t idx = it->first;
            maCaches.erase(it);
            maRanges[idx].SetInvalid();
            return true;
        }
    }
    return false;
}

ScDPCollection::NameCaches::NameCaches(ScDocument* pDoc) : mpDoc(pDoc) {}

bool ScDPCollection::NameCaches::hasCache(const OUString& rName) const
{
    return maCaches.count(rName) != 0;
}

const ScDPCache* ScDPCollection::NameCaches::getCache(
    const OUString& rName, const ScRange& rRange, const ScDPDimensionSaveData* pDimData)
{
    CachesType::const_iterator itr = maCaches.find(rName);
    if (itr != maCaches.end())
        
        return itr->second;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScDPCache> pCache(new ScDPCache(mpDoc));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    pCache->InitFromDoc(mpDoc, rRange);
    if (pDimData)
        pDimData->WriteToCache(*pCache);

    const ScDPCache* p = pCache.get();
    maCaches.insert(rName, pCache);
    return p;
}

ScDPCache* ScDPCollection::NameCaches::getExistingCache(const OUString& rName)
{
    CachesType::iterator itr = maCaches.find(rName);
    return itr != maCaches.end() ? itr->second : NULL;
}

size_t ScDPCollection::NameCaches::size() const
{
    return maCaches.size();
}

void ScDPCollection::NameCaches::updateCache(
    const OUString& rName, const ScRange& rRange, std::set<ScDPObject*>& rRefs)
{
    CachesType::iterator itr = maCaches.find(rName);
    if (itr == maCaches.end())
    {
        rRefs.clear();
        return;
    }

    ScDPCache& rCache = *itr->second;
    
    rCache.InitFromDoc(mpDoc, rRange);

    std::set<ScDPObject*> aRefs(rCache.GetAllReferences());
    rRefs.swap(aRefs);

    
    setGroupItemsToCache(rCache, rRefs);
}

bool ScDPCollection::NameCaches::remove(const ScDPCache* p)
{
    CachesType::iterator it = maCaches.begin(), itEnd = maCaches.end();
    for (; it != itEnd; ++it)
    {
        if (it->second == p)
        {
            maCaches.erase(it);
            return true;
        }
    }
    return false;
}

ScDPCollection::DBType::DBType(sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand) :
    mnSdbType(nSdbType), maDBName(rDBName), maCommand(rCommand) {}

bool ScDPCollection::DBType::less::operator() (const DBType& left, const DBType& right) const
{
    return left < right;
}

ScDPCollection::DBCaches::DBCaches(ScDocument* pDoc) : mpDoc(pDoc) {}

bool ScDPCollection::DBCaches::hasCache(sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand) const
{
    DBType aType(nSdbType, rDBName, rCommand);
    CachesType::const_iterator itr = maCaches.find(aType);
    return itr != maCaches.end();
}

const ScDPCache* ScDPCollection::DBCaches::getCache(
    sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand,
    const ScDPDimensionSaveData* pDimData)
{
    DBType aType(nSdbType, rDBName, rCommand);
    CachesType::const_iterator itr = maCaches.find(aType);
    if (itr != maCaches.end())
        
        return itr->second;

    uno::Reference<sdbc::XRowSet> xRowSet = createRowSet(nSdbType, rDBName, rCommand);
    if (!xRowSet.is())
        return NULL;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScDPCache> pCache(new ScDPCache(mpDoc));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    SvNumberFormatter aFormat( comphelper::getProcessComponentContext(), ScGlobal::eLnge);
    DBConnector aDB(*pCache, xRowSet, *aFormat.GetNullDate());
    if (!aDB.isValid())
        return NULL;

    if (!pCache->InitFromDataBase(aDB))
    {
        
        comphelper::disposeComponent(xRowSet);
        return NULL;
    }

    if (pDimData)
        pDimData->WriteToCache(*pCache);

    ::comphelper::disposeComponent(xRowSet);
    const ScDPCache* p = pCache.get();
    maCaches.insert(aType, pCache);
    return p;
}

ScDPCache* ScDPCollection::DBCaches::getExistingCache(
    sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand)
{
    DBType aType(nSdbType, rDBName, rCommand);
    CachesType::iterator itr = maCaches.find(aType);
    return itr != maCaches.end() ? itr->second : NULL;
}

uno::Reference<sdbc::XRowSet> ScDPCollection::DBCaches::createRowSet(
    sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand)
{
    uno::Reference<sdbc::XRowSet> xRowSet;
    try
    {
        xRowSet = uno::Reference<sdbc::XRowSet>(
            comphelper::getProcessServiceFactory()->createInstance(
                OUString(SC_SERVICE_ROWSET)),
            UNO_QUERY);

        uno::Reference<beans::XPropertySet> xRowProp(xRowSet, UNO_QUERY);
        OSL_ENSURE( xRowProp.is(), "can't get RowSet" );
        if (!xRowProp.is())
        {
            xRowSet.set(NULL);
            return xRowSet;
        }

        //
        
        //
        uno::Any aAny;
        aAny <<= rDBName;
        xRowProp->setPropertyValue(
            OUString(SC_DBPROP_DATASOURCENAME), aAny );

        aAny <<= rCommand;
        xRowProp->setPropertyValue(
            OUString(SC_DBPROP_COMMAND), aAny );

        aAny <<= nSdbType;
        xRowProp->setPropertyValue(
            OUString(SC_DBPROP_COMMANDTYPE), aAny );

        uno::Reference<sdb::XCompletedExecution> xExecute( xRowSet, uno::UNO_QUERY );
        if ( xExecute.is() )
        {
            uno::Reference<task::XInteractionHandler> xHandler(
                task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(), 0),
                uno::UNO_QUERY_THROW);
            xExecute->executeWithCompletion( xHandler );
        }
        else
            xRowSet->execute();

        return xRowSet;
    }
    catch ( const sdbc::SQLException& rError )
    {
        
        InfoBox aInfoBox( 0, OUString(rError.Message) );
        aInfoBox.Execute();
    }
    catch ( uno::Exception& )
    {
        OSL_FAIL("Unexpected exception in database");
    }

    xRowSet.set(NULL);
    return xRowSet;
}

void ScDPCollection::DBCaches::updateCache(
    sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand,
    std::set<ScDPObject*>& rRefs)
{
    DBType aType(nSdbType, rDBName, rCommand);
    CachesType::iterator it = maCaches.find(aType);
    if (it == maCaches.end())
    {
        
        rRefs.clear();
        return;
    }

    ScDPCache& rCache = *it->second;

    uno::Reference<sdbc::XRowSet> xRowSet = createRowSet(nSdbType, rDBName, rCommand);
    if (!xRowSet.is())
    {
        rRefs.clear();
        return;
    }

    SvNumberFormatter aFormat( comphelper::getProcessComponentContext(), ScGlobal::eLnge);
    DBConnector aDB(rCache, xRowSet, *aFormat.GetNullDate());
    if (!aDB.isValid())
        return;

    if (!rCache.InitFromDataBase(aDB))
    {
        
        rRefs.clear();
        comphelper::disposeComponent(xRowSet);
        return;
    }

    comphelper::disposeComponent(xRowSet);
    std::set<ScDPObject*> aRefs(rCache.GetAllReferences());
    aRefs.swap(rRefs);

    
    setGroupItemsToCache(rCache, rRefs);
}

bool ScDPCollection::DBCaches::remove(const ScDPCache* p)
{
    CachesType::iterator it = maCaches.begin(), itEnd = maCaches.end();
    for (; it != itEnd; ++it)
    {
        if (it->second == p)
        {
            maCaches.erase(it);
            return true;
        }
    }
    return false;
}

ScDPCollection::ScDPCollection(ScDocument* pDocument) :
    mpDoc( pDocument ),
    maSheetCaches(pDocument),
    maNameCaches(pDocument),
    maDBCaches(pDocument)
{
}

ScDPCollection::ScDPCollection(const ScDPCollection& r) :
    mpDoc(r.mpDoc),
    maSheetCaches(r.mpDoc),
    maNameCaches(r.mpDoc),
    maDBCaches(r.mpDoc)
{
}

ScDPCollection::~ScDPCollection()
{
    maTables.clear();
}

namespace {

/**
 * Unary predicate to match DP objects by the table ID.
 */
class MatchByTable : public unary_function<ScDPObject, bool>
{
    SCTAB mnTab;
public:
    MatchByTable(SCTAB nTab) : mnTab(nTab) {}

    bool operator() (const ScDPObject& rObj) const
    {
        return rObj.GetOutRange().aStart.Tab() == mnTab;
    }
};

}

sal_uLong ScDPCollection::ReloadCache(ScDPObject* pDPObj, std::set<ScDPObject*>& rRefs)
{
    if (!pDPObj)
        return STR_ERR_DATAPILOTSOURCE;

    if (pDPObj->IsSheetData())
    {
        
        const ScSheetSourceDesc* pDesc = pDPObj->GetSheetDesc();
        if (!pDesc)
            return STR_ERR_DATAPILOTSOURCE;

        sal_uLong nErrId = pDesc->CheckSourceRange();
        if (nErrId)
            return nErrId;

        if (pDesc->HasRangeName())
        {
            
            ScDPCollection::NameCaches& rCaches = GetNameCaches();
            if (rCaches.hasCache(pDesc->GetRangeName()))
                rCaches.updateCache(pDesc->GetRangeName(), pDesc->GetSourceRange(), rRefs);
            else
            {
                
                
                GetAllTables(pDesc->GetRangeName(), rRefs);
            }
        }
        else
        {
            
            ScDPCollection::SheetCaches& rCaches = GetSheetCaches();
            if (rCaches.hasCache(pDesc->GetSourceRange()))
                rCaches.updateCache(pDesc->GetSourceRange(), rRefs);
            else
            {
                
                
                GetAllTables(pDesc->GetSourceRange(), rRefs);
            }
        }
    }
    else if (pDPObj->IsImportData())
    {
        
        const ScImportSourceDesc* pDesc = pDPObj->GetImportSourceDesc();
        if (!pDesc)
            return STR_ERR_DATAPILOTSOURCE;

        ScDPCollection::DBCaches& rCaches = GetDBCaches();
        if (rCaches.hasCache(pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject))
            rCaches.updateCache(
                pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject, rRefs);
        else
        {
            
            
            GetAllTables(pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject, rRefs);
        }
    }
    return 0;
}

bool ScDPCollection::ReloadGroupsInCache(ScDPObject* pDPObj, std::set<ScDPObject*>& rRefs)
{
    if (!pDPObj)
        return false;

    const ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    if (!pSaveData)
        return false;

    
    
    
    
    

    ScDPCache* pCache = NULL;

    if (pDPObj->IsSheetData())
    {
        
        const ScSheetSourceDesc* pDesc = pDPObj->GetSheetDesc();
        if (!pDesc)
            return false;

        if (pDesc->HasRangeName())
        {
            
            ScDPCollection::NameCaches& rCaches = GetNameCaches();
            if (rCaches.hasCache(pDesc->GetRangeName()))
                pCache = rCaches.getExistingCache(pDesc->GetRangeName());
            else
            {
                
                
                pCache = const_cast<ScDPCache*>(
                    rCaches.getCache(pDesc->GetRangeName(), pDesc->GetSourceRange(), NULL));
            }
            GetAllTables(pDesc->GetRangeName(), rRefs);
        }
        else
        {
            
            ScDPCollection::SheetCaches& rCaches = GetSheetCaches();
            if (rCaches.hasCache(pDesc->GetSourceRange()))
                pCache = rCaches.getExistingCache(pDesc->GetSourceRange());
            else
            {
                
                
                pCache = const_cast<ScDPCache*>(
                    rCaches.getCache(pDesc->GetSourceRange(), NULL));
            }
            GetAllTables(pDesc->GetSourceRange(), rRefs);
        }
    }
    else if (pDPObj->IsImportData())
    {
        
        const ScImportSourceDesc* pDesc = pDPObj->GetImportSourceDesc();
        if (!pDesc)
            return false;

        ScDPCollection::DBCaches& rCaches = GetDBCaches();
        if (rCaches.hasCache(pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject))
            pCache = rCaches.getExistingCache(
                pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject);
        else
        {
            
            
            pCache = const_cast<ScDPCache*>(
                rCaches.getCache(pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject, NULL));
        }
        GetAllTables(pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject, rRefs);
    }

    if (!pCache)
        return false;

    
    
    pCache->ClearGroupFields();
    const ScDPDimensionSaveData* pDimData = pSaveData->GetExistingDimensionData();
    if (pDimData)
        pDimData->WriteToCache(*pCache);
    return true;
}

void ScDPCollection::DeleteOnTab( SCTAB nTab )
{
    maTables.erase_if(MatchByTable(nTab));
}

void ScDPCollection::UpdateReference( UpdateRefMode eUpdateRefMode,
                                         const ScRange& r, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    TablesType::iterator itr = maTables.begin(), itrEnd = maTables.end();
    for (; itr != itrEnd; ++itr)
        itr->UpdateReference(eUpdateRefMode, r, nDx, nDy, nDz);

    
    maSheetCaches.updateReference(eUpdateRefMode, r, nDx, nDy, nDz);
}

void ScDPCollection::CopyToTab( SCTAB nOld, SCTAB nNew )
{
    TablesType aAdded;
    TablesType::const_iterator it = maTables.begin(), itEnd = maTables.end();
    for (; it != itEnd; ++it)
    {
        const ScDPObject& rObj = *it;
        ScRange aOutRange = rObj.GetOutRange();
        if (aOutRange.aStart.Tab() != nOld)
            continue;

        ScAddress& s = aOutRange.aStart;
        ScAddress& e = aOutRange.aEnd;
        s.SetTab(nNew);
        e.SetTab(nNew);
        std::auto_ptr<ScDPObject> pNew(new ScDPObject(rObj));
        pNew->SetOutRange(aOutRange);
        mpDoc->ApplyFlagsTab(s.Col(), s.Row(), e.Col(), e.Row(), s.Tab(), SC_MF_DP_TABLE);
        aAdded.push_back(pNew);
    }

    maTables.transfer(maTables.end(), aAdded.begin(), aAdded.end(), aAdded);
}

bool ScDPCollection::RefsEqual( const ScDPCollection& r ) const
{
    if (maTables.size() != r.maTables.size())
        return false;

    TablesType::const_iterator itr = maTables.begin(), itr2 = r.maTables.begin(), itrEnd = maTables.end();
    for (; itr != itrEnd; ++itr, ++itr2)
        if (!itr->RefsEqual(*itr2))
            return false;

    return true;
}

void ScDPCollection::WriteRefsTo( ScDPCollection& r ) const
{
    if ( maTables.size() == r.maTables.size() )
    {
        
        TablesType::const_iterator itr = maTables.begin(), itrEnd = maTables.end();
        TablesType::iterator itr2 = r.maTables.begin();
        for (; itr != itrEnd; ++itr, ++itr2)
            itr->WriteRefsTo(*itr2);
    }
    else
    {
        
        
        
        size_t nSrcSize = maTables.size();
        size_t nDestSize = r.maTables.size();
        OSL_ENSURE( nSrcSize >= nDestSize, "WriteRefsTo: missing entries in document" );
        for (size_t nSrcPos = 0; nSrcPos < nSrcSize; ++nSrcPos)
        {
            const ScDPObject& rSrcObj = maTables[nSrcPos];
            const OUString& aName = rSrcObj.GetName();
            bool bFound = false;
            for (size_t nDestPos = 0; nDestPos < nDestSize && !bFound; ++nDestPos)
            {
                ScDPObject& rDestObj = r.maTables[nDestPos];
                if (rDestObj.GetName() == aName)
                {
                    rSrcObj.WriteRefsTo(rDestObj);     
                    bFound = true;
                }
            }

            if (!bFound)
            {
                

                ScDPObject* pDestObj = new ScDPObject(rSrcObj);
                r.InsertNewTable(pDestObj);
            }
        }
        OSL_ENSURE( maTables.size() == r.maTables.size(), "WriteRefsTo: couldn't restore all entries" );
    }
}

size_t ScDPCollection::GetCount() const
{
    return maTables.size();
}

ScDPObject* ScDPCollection::operator [](size_t nIndex)
{
    return &maTables[nIndex];
}

const ScDPObject* ScDPCollection::operator [](size_t nIndex) const
{
    return &maTables[nIndex];
}

const ScDPObject* ScDPCollection::GetByName(const OUString& rName) const
{
    TablesType::const_iterator itr = maTables.begin(), itrEnd = maTables.end();
    for (; itr != itrEnd; ++itr)
        if (itr->GetName() == rName)
            return &(*itr);

    return NULL;
}

OUString ScDPCollection::CreateNewName( sal_uInt16 nMin ) const
{
    OUString aBase("DataPilot");

    size_t n = maTables.size();
    for (size_t nAdd = 0; nAdd <= n; ++nAdd)   
    {
        OUStringBuffer aBuf;
        aBuf.append(aBase);
        aBuf.append(static_cast<sal_Int32>(nMin + nAdd));
        OUString aNewName = aBuf.makeStringAndClear();
        bool bFound = false;
        TablesType::const_iterator itr = maTables.begin(), itrEnd = maTables.end();
        for (; itr != itrEnd; ++itr)
        {
            if (itr->GetName() == aNewName)
            {
                bFound = true;
                break;
            }
        }
        if (!bFound)
            return aNewName;            
    }
    return OUString();                    
}

void ScDPCollection::FreeTable(ScDPObject* pDPObj)
{
    const ScRange& rOutRange = pDPObj->GetOutRange();
    const ScAddress& s = rOutRange.aStart;
    const ScAddress& e = rOutRange.aEnd;
    mpDoc->RemoveFlagsTab(s.Col(), s.Row(), e.Col(), e.Row(), s.Tab(), SC_MF_DP_TABLE);
    TablesType::iterator itr = maTables.begin(), itrEnd = maTables.end();
    for (; itr != itrEnd; ++itr)
    {
        ScDPObject* p = &(*itr);
        if (p == pDPObj)
        {
            maTables.erase(itr);
            break;
        }
    }
}

bool ScDPCollection::InsertNewTable(ScDPObject* pDPObj)
{
    const ScRange& rOutRange = pDPObj->GetOutRange();
    const ScAddress& s = rOutRange.aStart;
    const ScAddress& e = rOutRange.aEnd;
    mpDoc->ApplyFlagsTab(s.Col(), s.Row(), e.Col(), e.Row(), s.Tab(), SC_MF_DP_TABLE);

    maTables.push_back(pDPObj);
    return true;
}

ScDPCollection::SheetCaches& ScDPCollection::GetSheetCaches()
{
    return maSheetCaches;
}

ScDPCollection::NameCaches& ScDPCollection::GetNameCaches()
{
    return maNameCaches;
}

ScDPCollection::DBCaches& ScDPCollection::GetDBCaches()
{
    return maDBCaches;
}

ScRangeList ScDPCollection::GetAllTableRanges( SCTAB nTab ) const
{
    return std::for_each(maTables.begin(), maTables.end(), AccumulateOutputRanges(nTab)).getRanges();
}

bool ScDPCollection::IntersectsTableByColumns( SCCOL nCol1, SCCOL nCol2, SCROW nRow, SCTAB nTab ) const
{
    return std::find_if(
        maTables.begin(), maTables.end(), FindIntersetingTableByColumns(nCol1, nCol2, nRow, nTab)) != maTables.end();
}

bool ScDPCollection::IntersectsTableByRows( SCCOL nCol, SCROW nRow1, SCROW nRow2, SCTAB nTab ) const
{
    return std::find_if(
        maTables.begin(), maTables.end(), FindIntersectingTableByRows(nCol, nRow1, nRow2, nTab)) != maTables.end();
}

bool ScDPCollection::HasTable( const ScRange& rRange ) const
{
    return std::find_if(
        maTables.begin(), maTables.end(), FindIntersectingTable(rRange)) != maTables.end();
}

#if DEBUG_PIVOT_TABLE

namespace {

struct DumpTable : std::unary_function<ScDPObject, void>
{
    void operator() (const ScDPObject& rObj) const
    {
        cout << "-- '" << rObj.GetName() << "'" << endl;
        ScDPSaveData* pSaveData = rObj.GetSaveData();
        if (!pSaveData)
            return;

        pSaveData->Dump();

        cout << endl; 
    }
};

}

void ScDPCollection::DumpTables() const
{
    std::for_each(maTables.begin(), maTables.end(), DumpTable());
}

#endif

void ScDPCollection::RemoveCache(const ScDPCache* pCache)
{
    if (maSheetCaches.remove(pCache))
        
        return;

    if (maNameCaches.remove(pCache))
        
        return;

    if (maDBCaches.remove(pCache))
        
        return;
}

void ScDPCollection::GetAllTables(const ScRange& rSrcRange, std::set<ScDPObject*>& rRefs) const
{
    std::set<ScDPObject*> aRefs;
    TablesType::const_iterator it = maTables.begin(), itEnd = maTables.end();
    for (; it != itEnd; ++it)
    {
        const ScDPObject& rObj = *it;
        if (!rObj.IsSheetData())
            
            continue;

        const ScSheetSourceDesc* pDesc = rObj.GetSheetDesc();
        if (!pDesc)
            continue;

        if (pDesc->HasRangeName())
            
            continue;

        if (pDesc->GetSourceRange() != rSrcRange)
            
            continue;

        aRefs.insert(const_cast<ScDPObject*>(&rObj));
    }

    rRefs.swap(aRefs);
}

void ScDPCollection::GetAllTables(const OUString& rSrcName, std::set<ScDPObject*>& rRefs) const
{
    std::set<ScDPObject*> aRefs;
    TablesType::const_iterator it = maTables.begin(), itEnd = maTables.end();
    for (; it != itEnd; ++it)
    {
        const ScDPObject& rObj = *it;
        if (!rObj.IsSheetData())
            
            continue;

        const ScSheetSourceDesc* pDesc = rObj.GetSheetDesc();
        if (!pDesc)
            continue;

        if (!pDesc->HasRangeName())
            
            continue;

        if (pDesc->GetRangeName() != rSrcName)
            
            continue;

        aRefs.insert(const_cast<ScDPObject*>(&rObj));
    }

    rRefs.swap(aRefs);
}

void ScDPCollection::GetAllTables(
    sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand,
    std::set<ScDPObject*>& rRefs) const
{
    std::set<ScDPObject*> aRefs;
    TablesType::const_iterator it = maTables.begin(), itEnd = maTables.end();
    for (; it != itEnd; ++it)
    {
        const ScDPObject& rObj = *it;
        if (!rObj.IsImportData())
            
            continue;

        const ScImportSourceDesc* pDesc = rObj.GetImportSourceDesc();
        if (!pDesc)
            continue;

        if (!pDesc->aDBName.equals(rDBName) || !pDesc->aObject.equals(rCommand) || pDesc->GetCommandType() != nSdbType)
            
            continue;

        aRefs.insert(const_cast<ScDPObject*>(&rObj));
    }

    rRefs.swap(aRefs);
}

bool operator<(const ScDPCollection::DBType& left, const ScDPCollection::DBType& right)
{
    if (left.mnSdbType != right.mnSdbType)
        return left.mnSdbType < right.mnSdbType;

    if (!left.maDBName.equals(right.maDBName))
        return left.maDBName < right.maDBName;

    return left.maCommand < right.maCommand;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
