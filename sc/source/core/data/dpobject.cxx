/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <docsh.hxx>
#include <dpcache.hxx>
#include <dpobject.hxx>
#include <dptabsrc.hxx>
#include <dpsave.hxx>
#include <dpdimsave.hxx>
#include <dpoutput.hxx>
#include <dpshttab.hxx>
#include <dpsdbtab.hxx>
#include <dpgroup.hxx>
#include <document.hxx>
#include <pivot.hxx>
#include <dapiuno.hxx>
#include <miscuno.hxx>
#include <refupdat.hxx>
#include <attrib.hxx>
#include <scitems.hxx>
#include <unonames.hxx>
#include <dpglobal.hxx>
#include <globstr.hrc>
#include <queryentry.hxx>
#include <dputil.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sheet/GeneralFunction2.hpp>
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

#include <unotools/charclass.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <o3tl/safeint.hxx>
#include <sal/macros.h>
#include <svl/numformat.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <svl/zforlist.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <vector>
#include <memory>
#include <algorithm>

using namespace com::sun::star;
using ::std::vector;
using ::std::shared_ptr;
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

constexpr OUStringLiteral SC_SERVICE_ROWSET = u"com.sun.star.sdb.RowSet";

constexpr OUStringLiteral SC_DBPROP_DATASOURCENAME = u"DataSourceName";
constexpr OUStringLiteral SC_DBPROP_COMMAND = u"Command";
constexpr OUStringLiteral SC_DBPROP_COMMANDTYPE = u"CommandType";

constexpr OUString SCDPSOURCE_SERVICE = u"com.sun.star.sheet.DataPilotSource"_ustr;

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
    DBConnector(ScDPCache& rCache, uno::Reference<sdbc::XRowSet> xRowSet, const Date& rNullDate);

    bool isValid() const;

    virtual void getValue(tools::Long nCol, ScDPItemData &rData, SvNumFormatType& rNumType) const override;
    virtual OUString getColumnLabel(tools::Long nCol) const override;
    virtual tools::Long getColumnCount() const override;
    virtual bool first() override;
    virtual bool next() override;
    virtual void finish() override;
};

DBConnector::DBConnector(ScDPCache& rCache, uno::Reference<sdbc::XRowSet> xRowSet, const Date& rNullDate) :
    mrCache(rCache), mxRowSet(std::move(xRowSet)), maNullDate(rNullDate)
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

tools::Long DBConnector::getColumnCount() const
{
    return mxMetaData->getColumnCount();
}

OUString DBConnector::getColumnLabel(tools::Long nCol) const
{
    return mxMetaData->getColumnLabel(nCol+1);
}

void DBConnector::getValue(tools::Long nCol, ScDPItemData &rData, SvNumFormatType& rNumType) const
{
    rNumType = SvNumFormatType::NUMBER;
    sal_Int32 nType = mxMetaData->getColumnType(nCol+1);

    try
    {
        double fValue = 0.0;
        switch (nType)
        {
            case sdbc::DataType::BIT:
            case sdbc::DataType::BOOLEAN:
            {
                rNumType = SvNumFormatType::LOGICAL;
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
                //TODO: do the conversion here?
                fValue = mxRow->getDouble(nCol+1);
                rData.SetValue(fValue);
                break;
            }
            case sdbc::DataType::DATE:
            {
                rNumType = SvNumFormatType::DATE;

                util::Date aDate = mxRow->getDate(nCol+1);
                fValue = Date(aDate.Day, aDate.Month, aDate.Year) - maNullDate;
                rData.SetValue(fValue);
                break;
            }
            case sdbc::DataType::TIME:
            {
                rNumType = SvNumFormatType::TIME;

                util::Time aTime = mxRow->getTime(nCol+1);
                fValue = aTime.Hours       / static_cast<double>(::tools::Time::hourPerDay)   +
                         aTime.Minutes     / static_cast<double>(::tools::Time::minutePerDay) +
                         aTime.Seconds     / static_cast<double>(::tools::Time::secondPerDay) +
                         aTime.NanoSeconds / static_cast<double>(::tools::Time::nanoSecPerDay);
                rData.SetValue(fValue);
                break;
            }
            case sdbc::DataType::TIMESTAMP:
            {
                rNumType = SvNumFormatType::DATETIME;

                util::DateTime aStamp = mxRow->getTimestamp(nCol+1);
                fValue = ( Date( aStamp.Day, aStamp.Month, aStamp.Year ) - maNullDate ) +
                         aStamp.Hours       / static_cast<double>(::tools::Time::hourPerDay)   +
                         aStamp.Minutes     / static_cast<double>(::tools::Time::minutePerDay) +
                         aStamp.Seconds     / static_cast<double>(::tools::Time::secondPerDay) +
                         aStamp.NanoSeconds / static_cast<double>(::tools::Time::nanoSecPerDay);
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
                // nCol is 0-based, and the left-most column always has nCol == 0.
                rData.SetStringInterned(
                    mrCache.InternString(nCol, mxRow->getString(nCol+1)));
        }
    }
    catch (uno::Exception&)
    {
        rData.SetEmpty();
    }
}

}

static sheet::DataPilotFieldOrientation lcl_GetDataGetOrientation( const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
    sheet::DataPilotFieldOrientation nRet = sheet::DataPilotFieldOrientation_HIDDEN;
    if ( xSource.is() )
    {
        uno::Reference<container::XNameAccess> xDimNameAccess = xSource->getDimensions();
        const uno::Sequence<OUString> aDimNames = xDimNameAccess->getElementNames();
        for (const OUString& rDimName : aDimNames)
        {
            uno::Reference<beans::XPropertySet> xDimProp(xDimNameAccess->getByName(rDimName),
                                                         uno::UNO_QUERY);
            if ( xDimProp.is() )
            {
                const bool bFound = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                    SC_UNO_DP_ISDATALAYOUT );
                //TODO: error checking -- is "IsDataLayoutDimension" property required??
                if (bFound)
                {
                    nRet = ScUnoHelpFunctions::GetEnumProperty(
                            xDimProp, SC_UNO_DP_ORIENTATION,
                            sheet::DataPilotFieldOrientation_HIDDEN );
                    break;
                }
            }
        }
    }
    return nRet;
}

ScDPServiceDesc::ScDPServiceDesc(
    OUString aServ, OUString aSrc, OUString aNam,
    OUString aUser, OUString aPass ) :
    aServiceName(std::move( aServ )),
    aParSource(std::move( aSrc )),
    aParName(std::move( aNam )),
    aParUser(std::move( aUser )),
    aParPass(std::move( aPass )) {}

bool ScDPServiceDesc::operator== ( const ScDPServiceDesc& rOther ) const
{
    return aServiceName == rOther.aServiceName &&
        aParSource == rOther.aParSource &&
        aParName == rOther.aParName &&
        aParUser == rOther.aParUser &&
        aParPass == rOther.aParPass;
}

ScDPObject::ScDPObject(ScDocument* pDocument)
    : mpDocument(pDocument)
    , mnHeaderRows(0)
    , mbHeaderLayout(false)
    , mbAllowMove(false)
    , mbSettingsChanged(false)
    , mbEnableGetPivotData(true)
{
}

ScDPObject::ScDPObject(const ScDPObject& rOther)
    : mpDocument(rOther.mpDocument)
    , maTableName(rOther.maTableName)
    , maTableTag(rOther.maTableTag)
    , maOutputRange(rOther.maOutputRange)
    , maInteropGrabBag(rOther.maInteropGrabBag)
    , mnHeaderRows(rOther.mnHeaderRows)
    , mbHeaderLayout(rOther.mbHeaderLayout)
    , mbAllowMove(false)
    , mbSettingsChanged(false)
    , mbEnableGetPivotData(rOther.mbEnableGetPivotData)
{
    if (rOther.mpSaveData)
        mpSaveData.reset(new ScDPSaveData(*rOther.mpSaveData));
    if (rOther.mpSheetDescription)
        mpSheetDescription.reset(new ScSheetSourceDesc(*rOther.mpSheetDescription));
    if (rOther.mpImportDescription)
        mpImportDescription.reset(new ScImportSourceDesc(*rOther.mpImportDescription));
    if (rOther.mpServiceDescription)
        mpServiceDescription.reset(new ScDPServiceDesc(*rOther.mpServiceDescription));
    // mxSource (and mpOutput) is not copied
}

ScDPObject::~ScDPObject()
{
    Clear();
}

ScDPObject& ScDPObject::operator= (const ScDPObject& rOther)
{
    if (this != &rOther)
    {
        Clear();

        mpDocument = rOther.mpDocument;
        maTableName = rOther.maTableName;
        maTableTag = rOther.maTableTag;
        maOutputRange = rOther.maOutputRange;
        maInteropGrabBag = rOther.maInteropGrabBag;
        mnHeaderRows = rOther.mnHeaderRows;
        mbHeaderLayout =rOther.mbHeaderLayout;
        mbAllowMove = false;
        mbSettingsChanged = false;
        mbEnableGetPivotData = rOther.mbEnableGetPivotData;

        if (rOther.mpSaveData)
            mpSaveData.reset(new ScDPSaveData(*rOther.mpSaveData));
        if (rOther.mpSheetDescription)
            mpSheetDescription.reset(new ScSheetSourceDesc(*rOther.mpSheetDescription));
        if (rOther.mpImportDescription)
            mpImportDescription.reset(new ScImportSourceDesc(*rOther.mpImportDescription));
        if (rOther.mpServiceDescription)
            mpServiceDescription.reset(new ScDPServiceDesc(*rOther.mpServiceDescription));
    }
    return *this;
}

void ScDPObject::EnableGetPivotData(bool b)
{
    mbEnableGetPivotData = b;
}

void ScDPObject::SetAllowMove(bool bSet)
{
    mbAllowMove = bSet;
}

void ScDPObject::SetSaveData(const ScDPSaveData& rData)
{
    if (mpSaveData.get() != &rData)      // API implementation modifies the original SaveData object
    {
        mpSaveData.reset(new ScDPSaveData(rData));
    }

    InvalidateData();       // re-init source from SaveData
}

void ScDPObject::SetHeaderLayout (bool bUseGrid)
{
    mbHeaderLayout = bUseGrid;
}

void ScDPObject::SetOutRange(const ScRange& rRange)
{
    maOutputRange = rRange;

    if (mpOutput)
        mpOutput->SetPosition( rRange.aStart );
}

const ScRange& ScDPObject::GetOutRange() const
{
    return maOutputRange;
}

void ScDPObject::SetSheetDesc(const ScSheetSourceDesc& rDesc)
{
    if (mpSheetDescription && rDesc == *mpSheetDescription)
        return; // nothing to do

    mpImportDescription.reset();
    mpServiceDescription.reset();

    mpSheetDescription.reset( new ScSheetSourceDesc(rDesc) );

    //  make valid QueryParam

    const ScRange& rSrcRange = mpSheetDescription->GetSourceRange();
    ScQueryParam aParam = mpSheetDescription->GetQueryParam();
    aParam.nCol1 = rSrcRange.aStart.Col();
    aParam.nRow1 = rSrcRange.aStart.Row();
    aParam.nCol2 = rSrcRange.aEnd.Col();
    aParam.nRow2 = rSrcRange.aEnd.Row();
    aParam.bHasHeader = true;
    mpSheetDescription->SetQueryParam(aParam);

    ClearTableData(); // new source must be created
}

void ScDPObject::SetImportDesc(const ScImportSourceDesc& rDesc)
{
    if (mpImportDescription && rDesc == *mpImportDescription)
        return; // nothing to do

    mpSheetDescription.reset();
    mpServiceDescription.reset();

    mpImportDescription.reset(new ScImportSourceDesc(rDesc));

    ClearTableData(); // new source must be created
}

void ScDPObject::SetServiceData(const ScDPServiceDesc& rDesc)
{
    if (mpServiceDescription && rDesc == *mpServiceDescription)
        return; // nothing to do

    mpSheetDescription.reset();
    mpImportDescription.reset();

    mpServiceDescription.reset(new ScDPServiceDesc(rDesc));

    ClearTableData();      // new source must be created
}

void ScDPObject::WriteSourceDataTo( ScDPObject& rDest ) const
{
    if (mpSheetDescription)
        rDest.SetSheetDesc(*mpSheetDescription);
    else if (mpImportDescription)
        rDest.SetImportDesc(*mpImportDescription);
    else if (mpServiceDescription)
        rDest.SetServiceData(*mpServiceDescription);

    //  name/tag are not source data, but needed along with source data

    rDest.maTableName = maTableName;
    rDest.maTableTag  = maTableTag;
}

void ScDPObject::WriteTempDataTo( ScDPObject& rDest ) const
{
    rDest.mnHeaderRows = mnHeaderRows;
}

bool ScDPObject::IsSheetData() const
{
    return mpSheetDescription != nullptr;
}

void ScDPObject::SetName(const OUString& rNew)
{
    maTableName = rNew;
}

void ScDPObject::SetTag(const OUString& rNew)
{
    maTableTag = rNew;
}

bool ScDPObject::IsDataDescriptionCell(const ScAddress& rPos)
{
    if (!mpSaveData)
        return false;

    tools::Long nDataDimCount = mpSaveData->GetDataDimensionCount();
    if (nDataDimCount != 1)
        // There has to be exactly one data dimension for the description to
        // appear at top-left corner.
        return false;

    CreateOutput();
    ScRange aTabRange = mpOutput->GetOutputRange(sheet::DataPilotOutputRangeType::TABLE);
    return (rPos == aTabRange.aStart);
}

uno::Reference<sheet::XDimensionsSupplier> const & ScDPObject::GetSource()
{
    CreateObjects();
    return mxSource;
}

void ScDPObject::CreateOutput()
{
    CreateObjects();
    if (mpOutput)
        return;

    bool bFilterButton = IsSheetData() && mpSaveData && mpSaveData->GetFilterButton();
    bool bExpandCollapse = mpSaveData ? mpSaveData->GetExpandCollapse() : false;

    mpOutput.reset(new ScDPOutput(mpDocument, mxSource, maOutputRange.aStart, bFilterButton, bExpandCollapse, *this));
    mpOutput->SetHeaderLayout(mbHeaderLayout);
    if (mpSaveData->hasFormats())
        mpOutput->setFormats(mpSaveData->getFormats());

    sal_Int32 nOldRows = mnHeaderRows;
    mnHeaderRows = mpOutput->GetHeaderRows();

    if (!(mbAllowMove && mnHeaderRows != nOldRows))
        return;

    sal_Int32 nDiff = nOldRows - mnHeaderRows;
    if ( nOldRows == 0 )
        --nDiff;
    if (mnHeaderRows == 0)
        ++nDiff;

    sal_Int32 nNewRow = maOutputRange.aStart.Row() + nDiff;
    if ( nNewRow < 0 )
        nNewRow = 0;

    ScAddress aStart(maOutputRange.aStart);
    aStart.SetRow(nNewRow);
    mpOutput->SetPosition( aStart );

    //TODO: modify maOutputRange?

    mbAllowMove = false; // use only once
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

class FindIntersectingTable
{
    ScRange maRange;
public:
    explicit FindIntersectingTable(const ScRange& rRange) : maRange(rRange) {}

    bool operator() (const std::unique_ptr<ScDPObject>& rObj) const
    {
        return maRange.Intersects(rObj->GetOutRange());
    }
};

class FindIntersectingTableByColumns
{
    SCCOL mnCol1;
    SCCOL mnCol2;
    SCROW mnRow;
    SCTAB mnTab;
public:
    FindIntersectingTableByColumns(SCCOL nCol1, SCCOL nCol2, SCROW nRow, SCTAB nTab) :
        mnCol1(nCol1), mnCol2(nCol2), mnRow(nRow), mnTab(nTab) {}

    bool operator() (const std::unique_ptr<ScDPObject>& rObj) const
    {
        const ScRange& rRange = rObj->GetOutRange();
        if (mnTab != rRange.aStart.Tab())
            // Not on this sheet.
            return false;

        if (rRange.aEnd.Row() < mnRow)
            // This table is above the row.  It's safe.
            return false;

        if (mnCol1 <= rRange.aStart.Col() && rRange.aEnd.Col() <= mnCol2)
            // This table is fully enclosed in this column range.
            return false;

        if (rRange.aEnd.Col() < mnCol1 || mnCol2 < rRange.aStart.Col())
            // This table is entirely outside this column range.
            return false;

        // This table must be intersected by this column range.
        return true;
    }
};

class FindIntersectingTableByRows
{
    SCCOL mnCol;
    SCROW mnRow1;
    SCROW mnRow2;
    SCTAB mnTab;
public:
    FindIntersectingTableByRows(SCCOL nCol, SCROW nRow1, SCROW nRow2, SCTAB nTab) :
        mnCol(nCol), mnRow1(nRow1), mnRow2(nRow2), mnTab(nTab) {}

    bool operator() (const std::unique_ptr<ScDPObject>& rObj) const
    {
        const ScRange& rRange = rObj->GetOutRange();
        if (mnTab != rRange.aStart.Tab())
            // Not on this sheet.
            return false;

        if (rRange.aEnd.Col() < mnCol)
            // This table is to the left of the column.  It's safe.
            return false;

        if (mnRow1 <= rRange.aStart.Row() && rRange.aEnd.Row() <= mnRow2)
            // This table is fully enclosed in this row range.
            return false;

        if (rRange.aEnd.Row() < mnRow1 || mnRow2 < rRange.aStart.Row())
            // This table is entirely outside this row range.
            return false;

        // This table must be intersected by this row range.
        return true;
    }
};

class AccumulateOutputRanges
{
    ScRangeList maRanges;
    SCTAB mnTab;
public:
    explicit AccumulateOutputRanges(SCTAB nTab) : mnTab(nTab) {}
    AccumulateOutputRanges(const AccumulateOutputRanges& r) : maRanges(r.maRanges), mnTab(r.mnTab) {}

    void operator() (const std::unique_ptr<ScDPObject>& rObj)
    {
        const ScRange& rRange = rObj->GetOutRange();
        if (mnTab != rRange.aStart.Tab())
            // Not on this sheet.
            return;

        maRanges.Join(rRange);
    }

    const ScRangeList& getRanges() const { return maRanges; }
};

}

ScDPTableData* ScDPObject::GetTableData()
{
    if (!mpTableData)
    {
        shared_ptr<ScDPTableData> pData;
        const ScDPDimensionSaveData* pDimData = mpSaveData ? mpSaveData->GetExistingDimensionData() : nullptr;

        if (mpImportDescription)
        {
            // database data
            const ScDPCache* pCache = mpImportDescription->CreateCache(pDimData);
            if (pCache)
            {
                pCache->AddReference(this);
                pData = std::make_shared<ScDatabaseDPData>(mpDocument, *pCache);
            }
        }
        else
        {
            // cell data
            if (!mpSheetDescription)
            {
                OSL_FAIL("no source descriptor");
                mpSheetDescription.reset(new ScSheetSourceDesc(mpDocument)); // dummy defaults
            }

            {
                // Temporarily disable GETPIVOTDATA to avoid having
                // GETPIVOTDATA called onto itself from within the source
                // range.
                DisableGetPivotData aSwitch(*this, mbEnableGetPivotData);
                const ScDPCache* pCache = mpSheetDescription->CreateCache(pDimData);
                if (pCache)
                {
                    pCache->AddReference(this);
                    pData = std::make_shared<ScSheetDPData>(mpDocument, *mpSheetDescription, *pCache);
                }
            }
        }

        // grouping (for cell or database data)
        if (pData && pDimData)
        {
            auto pGroupData = std::make_shared<ScDPGroupTableData>(pData, mpDocument);
            pDimData->WriteToData(*pGroupData);
            pData = pGroupData;
        }

        mpTableData = pData;                        // after SetCacheId
    }

    return mpTableData.get();
}

void ScDPObject::CreateObjects()
{
    if (!mxSource.is())
    {
        mpOutput.reset(); // not valid when mxSource is changed

        if (mpServiceDescription)
        {
            mxSource = CreateSource(*mpServiceDescription);
        }

        if (!mxSource.is())    // database or sheet data, or error in CreateSource
        {
            OSL_ENSURE(!mpServiceDescription, "DPSource could not be created");
            ScDPTableData* pData = GetTableData();
            if (pData)
            {
                if (mpSaveData)
                    // Make sure to transfer these flags to the table data
                    // since they may have changed.
                    pData->SetEmptyFlags(mpSaveData->GetIgnoreEmptyRows(), mpSaveData->GetRepeatIfEmpty());

                pData->ReloadCacheTable();
                mxSource = new ScDPSource( pData );
            }
        }

        if (mpSaveData)
            mpSaveData->WriteToSource(mxSource);
    }
    else if (mbSettingsChanged)
    {
        mpOutput.reset(); // not valid when mxSource is changed

        uno::Reference<util::XRefreshable> xRef(mxSource, uno::UNO_QUERY);
        if (xRef.is())
        {
            try
            {
                xRef->refresh();
            }
            catch(uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION( "sc", "exception in refresh");
            }
        }

        if (mpSaveData)
            mpSaveData->WriteToSource(mxSource);
    }
    mbSettingsChanged = false;
}

void ScDPObject::InvalidateData()
{
    mbSettingsChanged = true;
}

void ScDPObject::Clear()
{
    mpOutput.reset();
    mpSaveData.reset();
    mpSheetDescription.reset();
    mpImportDescription.reset();
    mpServiceDescription.reset();
    ClearTableData();
    maInteropGrabBag.clear();
}

void ScDPObject::ClearTableData()
{
    ClearSource();

    if (mpTableData)
        mpTableData->GetCacheTable().getCache().RemoveReference(this);
    mpTableData.reset();
}

void ScDPObject::ReloadGroupTableData()
{
    ClearSource();

    if (!mpTableData)
        // Table data not built yet.  No need to reload the group data.
        return;

    if (!mpSaveData)
        // How could it not have the save data... but whatever.
        return;

    const ScDPDimensionSaveData* pDimData = mpSaveData->GetExistingDimensionData();
    if (!pDimData || !pDimData->HasGroupDimensions())
    {
        // No group dimensions exist.  Check if it currently has group
        // dimensions, and if so, remove all of them.
        ScDPGroupTableData* pData = dynamic_cast<ScDPGroupTableData*>(mpTableData.get());
        if (pData)
        {
            // Replace the existing group table data with the source data.
            mpTableData = pData->GetSourceTableData();
        }
        return;
    }

    ScDPGroupTableData* pData = dynamic_cast<ScDPGroupTableData*>(mpTableData.get());
    if (pData)
    {
        // This is already a group table data. Salvage the source data and
        // re-create a new group data.
        const shared_ptr<ScDPTableData>& pSource = pData->GetSourceTableData();
        auto pGroupData = std::make_shared<ScDPGroupTableData>(pSource, mpDocument);
        pDimData->WriteToData(*pGroupData);
        mpTableData = pGroupData;
    }
    else
    {
        // This is a source data.  Create a group data based on it.
        auto pGroupData = std::make_shared<ScDPGroupTableData>(mpTableData, mpDocument);
        pDimData->WriteToData(*pGroupData);
        mpTableData = pGroupData;
    }

    mbSettingsChanged = true;
}

void ScDPObject::ClearSource()
{
    uno::Reference<XComponent> xObjectComp(mxSource, UNO_QUERY);
    if (xObjectComp.is())
    {
        try
        {
            xObjectComp->dispose();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("sc.core");
        }
    }
    mxSource = nullptr;
}

ScRange ScDPObject::GetNewOutputRange( bool& rOverflow )
{
    CreateOutput(); // create mxSource and mpOutput if not already done

    rOverflow = mpOutput->HasError();        // range overflow or exception from source
    if ( rOverflow )
        return ScRange(maOutputRange.aStart);
    else
    {
        //  don't store the result in maOutputRange, because nothing has been output yet
        return mpOutput->GetOutputRange();
    }
}

void ScDPObject::Output( const ScAddress& rPos )
{
    //  clear old output area
    mpDocument->DeleteAreaTab(maOutputRange.aStart.Col(), maOutputRange.aStart.Row(),
                         maOutputRange.aEnd.Col(), maOutputRange.aEnd.Row(),
                         maOutputRange.aStart.Tab(), InsertDeleteFlags::ALL );
    mpDocument->RemoveFlagsTab( maOutputRange.aStart.Col(), maOutputRange.aStart.Row(),
                          maOutputRange.aEnd.Col(), maOutputRange.aEnd.Row(),
                          maOutputRange.aStart.Tab(), ScMF::Auto );

    CreateOutput(); // create mxSource and mpOutput if not already done

    mpOutput->SetPosition( rPos );

    mpOutput->Output();

    // maOutputRange is always the range that was last output to the document
    maOutputRange = mpOutput->GetOutputRange();
    const ScAddress& s = maOutputRange.aStart;
    const ScAddress& e = maOutputRange.aEnd;
    mpDocument->ApplyFlagsTab(s.Col(), s.Row(), e.Col(), e.Row(), s.Tab(), ScMF::DpTable);
}

ScRange ScDPObject::GetOutputRangeByType( sal_Int32 nType )
{
    CreateOutput();

    if (mpOutput->HasError())
        return ScRange(maOutputRange.aStart);

    return mpOutput->GetOutputRange(nType);
}

ScRange ScDPObject::GetOutputRangeByType( sal_Int32 nType ) const
{
    if (!mpOutput || mpOutput->HasError())
        return ScRange(ScAddress::INITIALIZE_INVALID);

    return mpOutput->GetOutputRange(nType);
}

static bool lcl_HasButton( const ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    return pDoc->GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG )->HasPivotButton();
}

void ScDPObject::RefreshAfterLoad()
{
    // apply drop-down attribute, initialize mnHeaderRows, without accessing the source
    // (button attribute must be present)

    // simple test: block of button cells at the top, followed by an empty cell

    SCCOL nFirstCol = maOutputRange.aStart.Col();
    SCROW nFirstRow = maOutputRange.aStart.Row();
    SCTAB nTab = maOutputRange.aStart.Tab();

    SCROW nInitial = 0;
    SCROW nOutRows = maOutputRange.aEnd.Row() + 1 - maOutputRange.aStart.Row();
    while (nInitial + 1 < nOutRows && lcl_HasButton(mpDocument, nFirstCol, nFirstRow + nInitial, nTab))
        ++nInitial;

    if ( nInitial + 1 < nOutRows &&
        mpDocument->IsBlockEmpty( nFirstCol, nFirstRow + nInitial, nFirstCol, nFirstRow + nInitial, nTab ) &&
        maOutputRange.aEnd.Col() > nFirstCol )
    {
        mnHeaderRows = nInitial;
    }
    else
        mnHeaderRows = 0;        // nothing found, no drop-down lists
}

void ScDPObject::BuildAllDimensionMembers()
{
    if (!mpSaveData)
        return;

    // #i111857# don't always create empty mpTableData for external service.
    if (mpServiceDescription)
        return;

    ScDPTableData* pTableData = GetTableData();
    if(pTableData)
        mpSaveData->BuildAllDimensionMembers(pTableData);
}

bool ScDPObject::SyncAllDimensionMembers()
{
    if (!mpSaveData)
        return false;

    // #i111857# don't always create empty mpTableData for external service.
    // Ideally, mxSource should be used instead of mpTableData.
    if (mpServiceDescription)
        return false;

    ScDPTableData* pData = GetTableData();
    if (!pData)
        // No table data exists.  This can happen when refreshing from an
        // external source which doesn't exist.
        return false;

    // Refresh the cache wrapper since the cache may have changed.
    pData->SetEmptyFlags(mpSaveData->GetIgnoreEmptyRows(), mpSaveData->GetRepeatIfEmpty());
    pData->ReloadCacheTable();
    mpSaveData->SyncAllDimensionMembers(pData);
    return true;
}

bool ScDPObject::GetMemberNames( sal_Int32 nDim, Sequence<OUString>& rNames )
{
    vector<ScDPLabelData::Member> aMembers;
    if (!GetMembers(nDim, GetUsedHierarchy(nDim), aMembers))
        return false;

    size_t n = aMembers.size();
    rNames.realloc(n);
    auto pNames = rNames.getArray();
    for (size_t i = 0; i < n; ++i)
        pNames[i] = aMembers[i].maName;

    return true;
}

bool ScDPObject::GetMembers( sal_Int32 nDim, sal_Int32 nHier, vector<ScDPLabelData::Member>& rMembers )
{
    Reference< sheet::XMembersAccess > xMembersNA;
    if (!GetMembersNA( nDim, nHier, xMembersNA ))
        return false;

    Reference<container::XIndexAccess> xMembersIA( new ScNameToIndexAccess(xMembersNA) );
    sal_Int32 nCount = xMembersIA->getCount();
    vector<ScDPLabelData::Member> aMembers;
    aMembers.reserve(nCount);

    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        Reference<container::XNamed> xMember;
        try
        {
            xMember = Reference<container::XNamed>(xMembersIA->getByIndex(i), UNO_QUERY);
        }
        catch (const container::NoSuchElementException&)
        {
            TOOLS_WARN_EXCEPTION("sc", "ScNameToIndexAccess getByIndex failed");
        }

        ScDPLabelData::Member aMem;

        if (xMember.is())
            aMem.maName = xMember->getName();

        Reference<beans::XPropertySet> xMemProp(xMember, UNO_QUERY);
        if (xMemProp.is())
        {
            aMem.mbVisible     = ScUnoHelpFunctions::GetBoolProperty(xMemProp, SC_UNO_DP_ISVISIBLE);
            aMem.mbShowDetails = ScUnoHelpFunctions::GetBoolProperty(xMemProp, SC_UNO_DP_SHOWDETAILS);

            aMem.maLayoutName = ScUnoHelpFunctions::GetStringProperty(
                xMemProp, SC_UNO_DP_LAYOUTNAME, OUString());
        }

        aMembers.push_back(aMem);
    }
    rMembers.swap(aMembers);
    return true;
}

void ScDPObject::UpdateReference( UpdateRefMode eUpdateRefMode,
                                     const ScRange& rRange, SCCOL nDx, SCROW nDy, SCTAB nDz )
{
    // Output area

    SCCOL nCol1 = maOutputRange.aStart.Col();
    SCROW nRow1 = maOutputRange.aStart.Row();
    SCTAB nTab1 = maOutputRange.aStart.Tab();
    SCCOL nCol2 = maOutputRange.aEnd.Col();
    SCROW nRow2 = maOutputRange.aEnd.Row();
    SCTAB nTab2 = maOutputRange.aEnd.Tab();

    ScRefUpdateRes eRes =
        ScRefUpdate::Update(mpDocument, eUpdateRefMode,
            rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
            rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aEnd.Tab(), nDx, nDy, nDz,
            nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    if ( eRes != UR_NOTHING )
        SetOutRange( ScRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 ) );

    // sheet source data

    if (!mpSheetDescription)
        return;

    const OUString& rRangeName = mpSheetDescription->GetRangeName();
    if (!rRangeName.isEmpty())
        // Source range is a named range.  No need to update.
        return;

    const ScRange& rSrcRange = mpSheetDescription->GetSourceRange();
    nCol1 = rSrcRange.aStart.Col();
    nRow1 = rSrcRange.aStart.Row();
    nTab1 = rSrcRange.aStart.Tab();
    nCol2 = rSrcRange.aEnd.Col();
    nRow2 = rSrcRange.aEnd.Row();
    nTab2 = rSrcRange.aEnd.Tab();

    eRes = ScRefUpdate::Update(mpDocument, eUpdateRefMode,
            rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
            rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aEnd.Tab(), nDx, nDy, nDz,
            nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    if ( eRes == UR_NOTHING )
        return;

    SCCOL nDiffX = nCol1 - mpSheetDescription->GetSourceRange().aStart.Col();
    SCROW nDiffY = nRow1 - mpSheetDescription->GetSourceRange().aStart.Row();

    ScQueryParam aParam = mpSheetDescription->GetQueryParam();
    aParam.nCol1 = sal::static_int_cast<SCCOL>( aParam.nCol1 + nDiffX );
    aParam.nCol2 = sal::static_int_cast<SCCOL>( aParam.nCol2 + nDiffX );
    aParam.nRow1 += nDiffY; //TODO: used?
    aParam.nRow2 += nDiffY; //TODO: used?
    SCSIZE nEC = aParam.GetEntryCount();
    for (SCSIZE i=0; i<nEC; i++)
        if (aParam.GetEntry(i).bDoQuery)
            aParam.GetEntry(i).nField += nDiffX;

    mpSheetDescription->SetQueryParam(aParam);
    mpSheetDescription->SetSourceRange(ScRange(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2));
}

bool ScDPObject::RefsEqual( const ScDPObject& rOther) const
{
    if (maOutputRange != rOther.maOutputRange)
        return false;

    if (mpSheetDescription && rOther.mpSheetDescription)
    {
        if (mpSheetDescription->GetSourceRange() != rOther.mpSheetDescription->GetSourceRange())
            return false;
    }
    else if (mpSheetDescription || rOther.mpSheetDescription)
    {
        OSL_FAIL("RefsEqual: SheetDesc set at only one object");
        return false;
    }

    return true;
}

void ScDPObject::WriteRefsTo(ScDPObject& rObject) const
{
    rObject.SetOutRange(maOutputRange);
    if (mpSheetDescription)
        rObject.SetSheetDesc(*mpSheetDescription);
}

void ScDPObject::GetPositionData(const ScAddress& rPos, DataPilotTablePositionData& rPosData)
{
    CreateOutput();
    mpOutput->GetPositionData(rPos, rPosData);
}

bool ScDPObject::GetDataFieldPositionData(
    const ScAddress& rPos, Sequence<sheet::DataPilotFieldFilter>& rFilters)
{
    CreateOutput();

    vector<sheet::DataPilotFieldFilter> aFilters;
    if (!mpOutput->GetDataResultPositionData(aFilters, rPos))
        return false;

    sal_Int32 n = static_cast<sal_Int32>(aFilters.size());
    rFilters.realloc(n);
    auto pFilters = rFilters.getArray();
    for (sal_Int32 i = 0; i < n; ++i)
        pFilters[i] = aFilters[i];

    return true;
}

void ScDPObject::GetDrillDownData(const ScAddress& rPos, Sequence< Sequence<Any> >& rTableData)
{
    CreateOutput();

    uno::Reference<sheet::XDrillDownDataSupplier> xDrillDownData(mxSource, UNO_QUERY);
    if (!xDrillDownData.is())
        return;

    Sequence<sheet::DataPilotFieldFilter> filters;
    if (!GetDataFieldPositionData(rPos, filters))
        return;

    rTableData = xDrillDownData->getDrillDownData(filters);
}

bool ScDPObject::IsDimNameInUse(std::u16string_view rName) const
{
    if (!mxSource.is())
        return false;

    Reference<container::XNameAccess> xDims = mxSource->getDimensions();
    const Sequence<OUString> aDimNames = xDims->getElementNames();
    for (const OUString& rDimName : aDimNames)
    {
        if (rDimName.equalsIgnoreAsciiCase(rName))
            return true;

        Reference<beans::XPropertySet> xPropSet(xDims->getByName(rDimName), UNO_QUERY);
        if (!xPropSet.is())
            continue;

        OUString aLayoutName = ScUnoHelpFunctions::GetStringProperty(
            xPropSet, SC_UNO_DP_LAYOUTNAME, OUString());
        if (aLayoutName.equalsIgnoreAsciiCase(rName))
            return true;
    }
    return false;
}

OUString ScDPObject::GetDimName( tools::Long nDim, bool& rIsDataLayout, sal_Int32* pFlags )
{
    rIsDataLayout = false;
    OUString aRet;

    if (mxSource.is())
    {
        uno::Reference<container::XNameAccess> xDimsName = mxSource->getDimensions();
        uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
        tools::Long nDimCount = xDims->getCount();
        if ( nDim < nDimCount )
        {
            uno::Reference<uno::XInterface> xIntDim(xDims->getByIndex(nDim), uno::UNO_QUERY);
            uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
            uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
            if ( xDimName.is() && xDimProp.is() )
            {
                bool bData = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                                SC_UNO_DP_ISDATALAYOUT );
                //TODO: error checking -- is "IsDataLayoutDimension" property required??

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
                                SC_UNO_DP_FLAGS );
            }
        }
    }
    else if (ScDPTableData* pData = GetTableData())
    {
        aRet = pData->getDimensionName(nDim);
        rIsDataLayout = pData->getIsDataLayoutDimension(nDim);
    }

    return aRet;
}

bool ScDPObject::IsDuplicated( tools::Long nDim )
{
    bool bDuplicated = false;
    if (mxSource.is())
    {
        uno::Reference<container::XNameAccess> xDimsName = mxSource->getDimensions();
        uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
        tools::Long nDimCount = xDims->getCount();
        if ( nDim < nDimCount )
        {
            uno::Reference<beans::XPropertySet> xDimProp(xDims->getByIndex(nDim), uno::UNO_QUERY);
            if ( xDimProp.is() )
            {
                try
                {
                    uno::Any aOrigAny = xDimProp->getPropertyValue( SC_UNO_DP_ORIGINAL );
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

tools::Long ScDPObject::GetDimCount()
{
    tools::Long nRet = 0;
    if (mxSource.is())
    {
        try
        {
            uno::Reference<container::XNameAccess> xDimsName = mxSource->getDimensions();
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
    CreateOutput(); // create mxSource and mpOutput if not already done

    // Reset member values to invalid state.
    rData.Dimension = rData.Hierarchy = rData.Level = -1;
    rData.Flags = 0;

    DataPilotTablePositionData aPosData;
    mpOutput->GetPositionData(rPos, aPosData);
    const sal_Int32 nPosType = aPosData.PositionType;
    if (nPosType == css::sheet::DataPilotTablePositionType::COLUMN_HEADER || nPosType == css::sheet::DataPilotTablePositionType::ROW_HEADER)
        aPosData.PositionData >>= rData;
}

namespace {

class FindByName
{
    OUString maName; // must be all uppercase.
public:
    explicit FindByName(OUString aName) : maName(std::move(aName)) {}
    bool operator() (const ScDPSaveDimension* pDim) const
    {
        // Layout name takes precedence.
        const std::optional<OUString> & pLayoutName = pDim->GetLayoutName();
        if (pLayoutName && ScGlobal::getCharClass().uppercase(*pLayoutName) == maName)
            return true;

        ScGeneralFunction eGenFunc = pDim->GetFunction();
        ScSubTotalFunc eFunc = ScDPUtil::toSubTotalFunc(eGenFunc);
        OUString aSrcName = ScDPUtil::getSourceDimensionName(pDim->GetName());
        OUString aFuncName = ScDPUtil::getDisplayedMeasureName(aSrcName, eFunc);
        if (maName == ScGlobal::getCharClass().uppercase(aFuncName))
            return true;

        return maName == ScGlobal::getCharClass().uppercase(aSrcName);
    }
};

class LessByDimOrder
{
    const ScDPSaveData::DimOrderType& mrDimOrder;

public:
    explicit LessByDimOrder(const ScDPSaveData::DimOrderType& rDimOrder) : mrDimOrder(rDimOrder) {}

    bool operator() (const sheet::DataPilotFieldFilter& r1, const sheet::DataPilotFieldFilter& r2) const
    {
        size_t nRank1 = mrDimOrder.size();
        size_t nRank2 = mrDimOrder.size();
        ScDPSaveData::DimOrderType::const_iterator it1 = mrDimOrder.find(
            ScGlobal::getCharClass().uppercase(r1.FieldName));
        if (it1 != mrDimOrder.end())
            nRank1 = it1->second;

        ScDPSaveData::DimOrderType::const_iterator it2 = mrDimOrder.find(
            ScGlobal::getCharClass().uppercase(r2.FieldName));
        if (it2 != mrDimOrder.end())
            nRank2 = it2->second;

        return nRank1 < nRank2;
    }
};

}

double ScDPObject::GetPivotData(const OUString& rDataFieldName, std::vector<sheet::DataPilotFieldFilter>& rFilters)
{
    if (!mbEnableGetPivotData)
        return std::numeric_limits<double>::quiet_NaN();

    CreateObjects();

    std::vector<const ScDPSaveDimension*> aDataDims;
    mpSaveData->GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_DATA, aDataDims);
    if (aDataDims.empty())
        return std::numeric_limits<double>::quiet_NaN();

    std::vector<const ScDPSaveDimension*>::iterator it = std::find_if(
        aDataDims.begin(), aDataDims.end(),
        FindByName(ScGlobal::getCharClass().uppercase(rDataFieldName)));

    if (it == aDataDims.end())
        return std::numeric_limits<double>::quiet_NaN();

    size_t nDataIndex = std::distance(aDataDims.begin(), it);

    uno::Reference<sheet::XDataPilotResults> xDPResults(mxSource, uno::UNO_QUERY);
    if (!xDPResults.is())
        return std::numeric_limits<double>::quiet_NaN();

    // Dimensions must be sorted in order of appearance, and row dimensions
    // must come before column dimensions.
    std::sort(rFilters.begin(), rFilters.end(), LessByDimOrder(mpSaveData->GetDimensionSortOrder()));

    size_t n = rFilters.size();
    uno::Sequence<sheet::DataPilotFieldFilter> aFilters(n);
    auto aFiltersRange = asNonConstRange(aFilters);
    for (size_t i = 0; i < n; ++i)
        aFiltersRange[i] = rFilters[i];

    uno::Sequence<double> aRes = xDPResults->getFilteredResults(aFilters);
    if (nDataIndex >= o3tl::make_unsigned(aRes.getLength()))
        return std::numeric_limits<double>::quiet_NaN();

    return aRes[nDataIndex];
}

bool ScDPObject::IsFilterButton( const ScAddress& rPos )
{
    CreateOutput(); // create mxSource and mpOutput if not already done

    return mpOutput->IsFilterButton( rPos );
}

tools::Long ScDPObject::GetHeaderDim( const ScAddress& rPos, sheet::DataPilotFieldOrientation& rOrient )
{
    CreateOutput(); // create mxSource and mpOutput if not already done

    return mpOutput->GetHeaderDim( rPos, rOrient );
}

bool ScDPObject::GetHeaderDrag( const ScAddress& rPos, bool bMouseLeft, bool bMouseTop, tools::Long nDragDim,
                                tools::Rectangle& rPosRect, sheet::DataPilotFieldOrientation& rOrient, tools::Long& rDimPos )
{
    CreateOutput();// create mxSource and mpOutput if not already done

    return mpOutput->GetHeaderDrag( rPos, bMouseLeft, bMouseTop, nDragDim, rPosRect, rOrient, rDimPos );
}

void ScDPObject::GetMemberResultNames(ScDPUniqueStringSet& rNames, tools::Long nDimension)
{
    CreateOutput();// create mxSource and mpOutput if not already done

    mpOutput->GetMemberResultNames(rNames, nDimension);    // used only with table data -> level not needed
}

OUString ScDPObject::GetFormattedString(ScDPTableData* pTableData, tools::Long nDimension, const double fValue)
{
    ScDPItemData aItemData;
    aItemData.SetValue(fValue);
    return pTableData->GetFormattedString(nDimension, aItemData, false);
}

OUString ScDPObject::GetFormattedString(std::u16string_view rDimName, const double fValue)
{
    ScDPTableData* pTableData = GetTableData();
    if(!pTableData)
        return OUString();

    tools::Long nDim;
    for (nDim = 0; nDim < pTableData->GetColumnCount(); ++nDim)
    {
        if(rDimName == pTableData->getDimensionName(nDim))
            break;
    }

    return GetFormattedString(pTableData, nDim, fValue);
}


namespace {

bool dequote( std::u16string_view rSource, sal_Int32 nStartPos, sal_Int32& rEndPos, OUString& rResult )
{
    // nStartPos has to point to opening quote

    const sal_Unicode cQuote = '\'';

    if (rSource[nStartPos] == cQuote)
    {
        OUStringBuffer aBuffer;
        sal_Int32 nPos = nStartPos + 1;
        const sal_Int32 nLen = rSource.size();

        while ( nPos < nLen )
        {
            const sal_Unicode cNext = rSource[nPos];
            if ( cNext == cQuote )
            {
                if (nPos+1 < nLen && rSource[nPos+1] == cQuote)
                {
                    // double quote is used for an embedded quote
                    aBuffer.append( cNext );    // append one quote
                    ++nPos;                     // skip the next one
                }
                else
                {
                    // end of quoted string
                    rResult = aBuffer.makeStringAndClear();
                    rEndPos = nPos + 1;         // behind closing quote
                    return true;
                }
            }
            else
                aBuffer.append( cNext );

            ++nPos;
        }
        // no closing quote before the end of the string -> error (bRet still false)
    }

    return false;
}

struct ScGetPivotDataFunctionEntry
{
    const char*       pName;
    sal_Int16         eFunc;
};

bool parseFunction( std::u16string_view rList, sal_Int32 nStartPos, sal_Int32& rEndPos, sal_Int16& rFunc )
{
    static const ScGetPivotDataFunctionEntry aFunctions[] =
    {
        // our names
        { "Sum",        sheet::GeneralFunction2::SUM       },
        { "Count",      sheet::GeneralFunction2::COUNT     },
        { "Average",    sheet::GeneralFunction2::AVERAGE   },
        { "Max",        sheet::GeneralFunction2::MAX       },
        { "Min",        sheet::GeneralFunction2::MIN       },
        { "Product",    sheet::GeneralFunction2::PRODUCT   },
        { "CountNums",  sheet::GeneralFunction2::COUNTNUMS },
        { "StDev",      sheet::GeneralFunction2::STDEV     },
        { "StDevp",     sheet::GeneralFunction2::STDEVP    },
        { "Var",        sheet::GeneralFunction2::VAR       },
        { "VarP",       sheet::GeneralFunction2::VARP      },
        // compatibility names
        { "Count Nums", sheet::GeneralFunction2::COUNTNUMS },
        { "StdDev",     sheet::GeneralFunction2::STDEV     },
        { "StdDevp",    sheet::GeneralFunction2::STDEVP    }
    };

    const sal_Int32 nListLen = rList.size();
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
        nFuncEnd = rList.find(']', nStartPos);
        if (nFuncEnd >= 0)
        {
            aFuncStr = rList.substr(nStartPos, nFuncEnd - nStartPos);
            bParsed = true;
        }
    }

    if ( bParsed )
    {
        aFuncStr = comphelper::string::strip(aFuncStr, ' ');

        const sal_Int32 nFuncCount = SAL_N_ELEMENTS(aFunctions);
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

bool extractAtStart( std::u16string_view rList, sal_Int32& rMatched, bool bAllowBracket, sal_Int16* pFunc,
        OUString& rDequoted )
{
    size_t nMatchList = 0;
    sal_Unicode cFirst = rList[0];
    bool bParsed = false;
    if ( cFirst == '\'' || cFirst == '[' )
    {
        // quoted string or string in brackets must match completely

        OUString aDequoted;
        sal_Int32 nQuoteEnd = 0;

        if ( cFirst == '\'' )
            bParsed = dequote( rList, 0, nQuoteEnd, aDequoted );
        else if ( cFirst == '[' )
        {
            // skip spaces after the opening bracket

            sal_Int32 nStartPos = 1;
            const sal_Int32 nListLen = rList.size();
            while (nStartPos < nListLen && rList[nStartPos] == ' ')
                ++nStartPos;

            if (nStartPos < nListLen && rList[nStartPos] == '\'')         // quoted within the brackets?
            {
                if ( dequote( rList, nStartPos, nQuoteEnd, aDequoted ) )
                {
                    // after the quoted string, there must be the closing bracket, optionally preceded by spaces,
                    // and/or a function name
                    while (nQuoteEnd < nListLen && rList[nQuoteEnd] == ' ')
                        ++nQuoteEnd;

                    // semicolon separates function name
                    if (nQuoteEnd < nListLen && rList[nQuoteEnd] == ';' && pFunc)
                    {
                        sal_Int32 nFuncEnd = 0;
                        if ( parseFunction( rList, nQuoteEnd + 1, nFuncEnd, *pFunc ) )
                            nQuoteEnd = nFuncEnd;
                    }
                    if (nQuoteEnd < nListLen && rList[nQuoteEnd] == ']')
                    {
                        ++nQuoteEnd;        // include the closing bracket for the matched length
                        bParsed = true;
                    }
                }
            }
            else
            {
                // implicit quoting to the closing bracket

                sal_Int32 nClosePos = rList.find(']', nStartPos);
                if (nClosePos >= 0)
                {
                    sal_Int32 nNameEnd = nClosePos;
                    sal_Int32 nSemiPos = rList.find(';', nStartPos);
                    if (nSemiPos >= 0 && nSemiPos < nClosePos && pFunc)
                    {
                        sal_Int32 nFuncEnd = 0;
                        if (parseFunction(rList, nSemiPos+1, nFuncEnd, *pFunc))
                            nNameEnd = nSemiPos;
                    }

                    aDequoted = rList.substr(nStartPos, nNameEnd - nStartPos);
                    // spaces before the closing bracket or semicolon
                    aDequoted = comphelper::string::stripEnd(aDequoted, ' ');
                    nQuoteEnd = nClosePos + 1;
                    bParsed = true;
                }
            }
        }

        if ( bParsed )
        {
            nMatchList = nQuoteEnd;             // match count in the list string, including quotes
            rDequoted = aDequoted;
        }
    }

    if (bParsed)
    {
        // look for following space or end of string

        bool bValid = false;
        if ( nMatchList >= rList.size() )
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

bool isAtStart(
    const OUString& rList, const OUString& rSearch, sal_Int32& rMatched,
    bool bAllowBracket, sal_Int16* pFunc )
{
    sal_Int32 nMatchList = 0;
    sal_Int32 nMatchSearch = 0;
    sal_Unicode cFirst = rList[0];
    if ( cFirst == '\'' || cFirst == '[' )
    {
        OUString aDequoted;
        bool bParsed = extractAtStart( rList, rMatched, bAllowBracket, pFunc, aDequoted);
        if ( bParsed && ScGlobal::GetTransliteration().isEqual( aDequoted, rSearch ) )
        {
            nMatchList = rMatched;             // match count in the list string, including quotes
            nMatchSearch = rSearch.getLength();
        }
    }
    else
    {
        // otherwise look for search string at the start of rList
        ScGlobal::GetTransliteration().equals(
            rList, 0, rList.getLength(), nMatchList, rSearch, 0, rSearch.getLength(), nMatchSearch);
    }

    if (nMatchSearch == rSearch.getLength())
    {
        // search string is at start of rList - look for following space or end of string

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

} // anonymous namespace

bool ScDPObject::ParseFilters(
    OUString& rDataFieldName,
    std::vector<sheet::DataPilotFieldFilter>& rFilters,
    std::vector<sal_Int16>& rFilterFuncs, std::u16string_view rFilterList )
{
    // parse the string rFilterList into parameters for GetPivotData

    CreateObjects(); // create mxSource if not already done

    std::vector<OUString> aDataNames;     // data fields (source name)
    std::vector<OUString> aGivenNames;    // data fields (compound name)
    std::vector<OUString> aFieldNames;    // column/row/data fields
    std::vector< uno::Sequence<OUString> > aFieldValueNames;
    std::vector< uno::Sequence<OUString> > aFieldValues;

    // get all the field and item names

    uno::Reference<container::XNameAccess> xDimsName = mxSource->getDimensions();
    uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
    sal_Int32 nDimCount = xIntDims->getCount();
    for ( sal_Int32 nDim = 0; nDim<nDimCount; nDim++ )
    {
        uno::Reference<uno::XInterface> xIntDim(xIntDims->getByIndex(nDim), uno::UNO_QUERY);
        uno::Reference<container::XNamed> xDim( xIntDim, uno::UNO_QUERY );
        uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
        uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDim, uno::UNO_QUERY );
        bool bDataLayout = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                            SC_UNO_DP_ISDATALAYOUT );
        sheet::DataPilotFieldOrientation nOrient = ScUnoHelpFunctions::GetEnumProperty(
                            xDimProp, SC_UNO_DP_ORIENTATION,
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
                // get level names, as in ScDPOutput

                uno::Reference<container::XIndexAccess> xHiers = new ScNameToIndexAccess( xDimSupp->getHierarchies() );
                sal_Int32 nHierarchy = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                                    SC_UNO_DP_USEDHIERARCHY );
                if ( nHierarchy >= xHiers->getCount() )
                    nHierarchy = 0;

                uno::Reference<sheet::XLevelsSupplier> xHierSupp(xHiers->getByIndex(nHierarchy),
                                                                 uno::UNO_QUERY);
                if ( xHierSupp.is() )
                {
                    uno::Reference<container::XIndexAccess> xLevels = new ScNameToIndexAccess( xHierSupp->getLevels() );
                    sal_Int32 nLevCount = xLevels->getCount();
                    for (sal_Int32 nLev=0; nLev<nLevCount; nLev++)
                    {
                        uno::Reference<uno::XInterface> xLevel(xLevels->getByIndex(nLev),
                                                               uno::UNO_QUERY);
                        uno::Reference<container::XNamed> xLevNam( xLevel, uno::UNO_QUERY );
                        uno::Reference<sheet::XMembersSupplier> xLevSupp( xLevel, uno::UNO_QUERY );
                        if ( xLevNam.is() && xLevSupp.is() )
                        {
                            uno::Reference<sheet::XMembersAccess> xMembers = xLevSupp->getMembers();

                            OUString aFieldName( xLevNam->getName() );
                            // getElementNames() and getLocaleIndependentElementNames()
                            // must be consecutive calls to obtain strings in matching order.
                            uno::Sequence<OUString> aMemberValueNames( xMembers->getElementNames() );
                            uno::Sequence<OUString> aMemberValues( xMembers->getLocaleIndependentElementNames() );

                            aFieldNames.push_back( aFieldName );
                            aFieldValueNames.push_back( aMemberValueNames );
                            aFieldValues.push_back( aMemberValues );
                        }
                    }
                }
            }
        }
    }

    // compare and build filters

    SCSIZE nDataFields = aDataNames.size();
    SCSIZE nFieldCount = aFieldNames.size();
    OSL_ENSURE( aGivenNames.size() == nDataFields && aFieldValueNames.size() == nFieldCount &&
            aFieldValues.size() == nFieldCount, "wrong count" );

    bool bError = false;
    bool bHasData = false;
    OUString aRemaining(comphelper::string::strip(rFilterList, ' '));
    while (!aRemaining.isEmpty() && !bError)
    {
        bool bUsed = false;

        // look for data field name

        for ( SCSIZE nDataPos=0; nDataPos<nDataFields && !bUsed; nDataPos++ )
        {
            OUString aFound;
            sal_Int32 nMatched = 0;
            if (isAtStart(aRemaining, aDataNames[nDataPos], nMatched, false, nullptr))
                aFound = aDataNames[nDataPos];
            else if (isAtStart(aRemaining, aGivenNames[nDataPos], nMatched, false, nullptr))
                aFound = aGivenNames[nDataPos];

            if (!aFound.isEmpty())
            {
                rDataFieldName = aFound;
                aRemaining = aRemaining.copy(nMatched);
                bHasData = true;
                bUsed = true;
            }
        }

        // look for field name

        OUString aSpecField;
        bool bHasFieldName = false;
        if ( !bUsed )
        {
            sal_Int32 nMatched = 0;
            for ( SCSIZE nField=0; nField<nFieldCount && !bHasFieldName; nField++ )
            {
                if (isAtStart(aRemaining, aFieldNames[nField], nMatched, true, nullptr))
                {
                    aSpecField = aFieldNames[nField];
                    aRemaining = aRemaining.copy(nMatched);
                    aRemaining = comphelper::string::stripStart(aRemaining, ' ');

                    // field name has to be followed by item name in brackets
                    if (aRemaining.startsWith("["))
                    {
                        bHasFieldName = true;
                        // bUsed remains false - still need the item
                    }
                    else
                    {
                        bUsed = true;
                        bError = true;
                    }
                }
            }
        }

        // look for field item

        if ( !bUsed )
        {
            bool bItemFound = false;
            sal_Int32 nMatched = 0;
            OUString aFoundName;
            OUString aFoundValueName;
            OUString aFoundValue;
            sal_Int16 eFunc = sheet::GeneralFunction2::NONE;
            sal_Int16 eFoundFunc = sheet::GeneralFunction2::NONE;

            OUString aQueryValueName;
            const bool bHasQuery = extractAtStart( aRemaining, nMatched, false, &eFunc, aQueryValueName);

            OUString aQueryValue = aQueryValueName;
            if (mpTableData)
            {
                ScInterpreterContext& rContext = mpTableData->GetCacheTable().getCache().GetInterpreterContext();
                // Parse possible number from aQueryValueName and format
                // locale independent as aQueryValue.
                sal_uInt32 nNumFormat = 0;
                double fValue;
                if (rContext.NFIsNumberFormat(aQueryValueName, nNumFormat, fValue))
                    aQueryValue = ScDPCache::GetLocaleIndependentFormattedString(fValue, rContext, nNumFormat);
            }

            for ( SCSIZE nField=0; nField<nFieldCount; nField++ )
            {
                // If a field name is given, look in that field only, otherwise in all fields.
                // aSpecField is initialized from aFieldNames array, so exact comparison can be used.
                if ( !bHasFieldName || aFieldNames[nField] == aSpecField )
                {
                    const uno::Sequence<OUString>& rItemNames = aFieldValueNames[nField];
                    const uno::Sequence<OUString>& rItemValues = aFieldValues[nField];
                    sal_Int32 nItemCount = rItemNames.getLength();
                    assert(nItemCount == rItemValues.getLength());
                    const OUString* pItemNamesArr = rItemNames.getConstArray();
                    const OUString* pItemValuesArr = rItemValues.getConstArray();
                    for ( sal_Int32 nItem=0; nItem<nItemCount; nItem++ )
                    {
                        bool bThisItemFound;
                        if (bHasQuery)
                        {
                            // First check given value name against both.
                            bThisItemFound = ScGlobal::GetTransliteration().isEqual(
                                    aQueryValueName, pItemNamesArr[nItem]);
                            if (!bThisItemFound && pItemValuesArr[nItem] != pItemNamesArr[nItem])
                                bThisItemFound = ScGlobal::GetTransliteration().isEqual(
                                        aQueryValueName, pItemValuesArr[nItem]);
                            if (!bThisItemFound && aQueryValueName != aQueryValue)
                            {
                                // Second check locale independent value
                                // against both.
                                /* TODO: or check only value string against
                                 * value string, not against the value name? */
                                bThisItemFound = ScGlobal::GetTransliteration().isEqual(
                                        aQueryValue, pItemNamesArr[nItem]);
                                if (!bThisItemFound && pItemValuesArr[nItem] != pItemNamesArr[nItem])
                                    bThisItemFound = ScGlobal::GetTransliteration().isEqual(
                                            aQueryValue, pItemValuesArr[nItem]);
                            }
                        }
                        else
                        {
                            bThisItemFound = isAtStart( aRemaining, pItemNamesArr[nItem], nMatched, false, &eFunc );
                            if (!bThisItemFound && pItemValuesArr[nItem] != pItemNamesArr[nItem])
                                bThisItemFound = isAtStart( aRemaining, pItemValuesArr[nItem], nMatched, false, &eFunc );
                            /* TODO: this checks only the given value name,
                             * check also locale independent value. But we'd
                             * have to do that in each iteration of the loop
                             * inside isAtStart() since a query could not be
                             * extracted and a match could be on the passed
                             * item value name string or item value string
                             * starting at aRemaining. */
                        }
                        if (bThisItemFound)
                        {
                            if ( bItemFound )
                                bError = true;      // duplicate (also across fields)
                            else
                            {
                                aFoundName = aFieldNames[nField];
                                aFoundValueName = pItemNamesArr[nItem];
                                aFoundValue = pItemValuesArr[nItem];
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
                aField.MatchValueName = aFoundValueName;
                aField.MatchValue = aFoundValue;
                rFilters.push_back(aField);
                rFilterFuncs.push_back(eFoundFunc);
                aRemaining = aRemaining.copy(nMatched);
            }
        }

        if ( !bUsed )
            bError = true;

        // remove any number of spaces between entries
        aRemaining = comphelper::string::stripStart(aRemaining, ' ');
    }

    if ( !bError && !bHasData && aDataNames.size() == 1 )
    {
        // if there's only one data field, its name need not be specified
        rDataFieldName = aDataNames[0];
        bHasData = true;
    }

    return bHasData && !bError;
}

void ScDPObject::ToggleDetails(const DataPilotTableHeaderData& rElemDesc, ScDPObject* pDestObj)
{
    CreateObjects(); // create mxSource if not already done

    //  find dimension name

    uno::Reference<container::XNamed> xDim;
    uno::Reference<container::XNameAccess> xDimsName = mxSource->getDimensions();
    uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
    tools::Long nIntCount = xIntDims->getCount();
    if ( rElemDesc.Dimension < nIntCount )
    {
        xDim.set(xIntDims->getByIndex(rElemDesc.Dimension), uno::UNO_QUERY);
    }
    OSL_ENSURE( xDim.is(), "dimension not found" );
    if ( !xDim.is() ) return;
    OUString aDimName = xDim->getName();

    uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
    bool bDataLayout = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                        SC_UNO_DP_ISDATALAYOUT );
    if (bDataLayout)
    {
        //  the elements of the data layout dimension can't be found by their names
        //  -> don't change anything
        return;
    }

    //  query old state

    tools::Long nHierCount = 0;
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
        xHier.set(xHiers->getByIndex(rElemDesc.Hierarchy), uno::UNO_QUERY);
    OSL_ENSURE( xHier.is(), "hierarchy not found" );
    if ( !xHier.is() ) return;

    tools::Long nLevCount = 0;
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
        xLevel.set(xLevels->getByIndex(rElemDesc.Level), uno::UNO_QUERY);
    OSL_ENSURE( xLevel.is(), "level not found" );
    if ( !xLevel.is() ) return;

    uno::Reference<sheet::XMembersAccess> xMembers;
    uno::Reference<sheet::XMembersSupplier> xMbrSupp( xLevel, uno::UNO_QUERY );
    if ( xMbrSupp.is() )
        xMembers = xMbrSupp->getMembers();

    bool bFound = false;
    bool bShowDetails = true;

    if ( xMembers.is() )
    {
        if ( xMembers->hasByName(rElemDesc.MemberName) )
        {
            uno::Reference<beans::XPropertySet> xMbrProp(xMembers->getByName(rElemDesc.MemberName),
                                                         uno::UNO_QUERY);
            if ( xMbrProp.is() )
            {
                bShowDetails = ScUnoHelpFunctions::GetBoolProperty( xMbrProp,
                                    SC_UNO_DP_SHOWDETAILS );
                //TODO: don't set bFound if property is unknown?
                bFound = true;
            }
        }
    }

    OSL_ENSURE( bFound, "member not found" );

    //TODO: use Hierarchy and Level in SaveData !!!!

    //  modify pDestObj if set, this object otherwise
    ScDPSaveData* pModifyData = pDestObj ? ( pDestObj->mpSaveData.get() ) : mpSaveData.get();
    OSL_ENSURE( pModifyData, "no data?" );
    if ( pModifyData )
    {
        const OUString aName = rElemDesc.MemberName;
        pModifyData->GetDimensionByName(aDimName)->
            GetMemberByName(aName)->SetShowDetails( !bShowDetails );    // toggle

        if ( pDestObj )
            pDestObj->InvalidateData();     // re-init source from SaveData
        else
            InvalidateData();               // re-init source from SaveData
    }
}

static PivotFunc lcl_FirstSubTotal( const uno::Reference<beans::XPropertySet>& xDimProp )     // PIVOT_FUNC mask
{
    uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDimProp, uno::UNO_QUERY );
    if ( xDimProp.is() && xDimSupp.is() )
    {
        uno::Reference<container::XIndexAccess> xHiers = new ScNameToIndexAccess( xDimSupp->getHierarchies() );
        tools::Long nHierarchy = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                SC_UNO_DP_USEDHIERARCHY );
        if ( nHierarchy >= xHiers->getCount() )
            nHierarchy = 0;

        uno::Reference<sheet::XLevelsSupplier> xHierSupp(xHiers->getByIndex(nHierarchy),
                                                         uno::UNO_QUERY);
        if ( xHierSupp.is() )
        {
            uno::Reference<container::XIndexAccess> xLevels = new ScNameToIndexAccess( xHierSupp->getLevels() );
            uno::Reference<uno::XInterface> xLevel(xLevels->getByIndex(0), uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xLevProp( xLevel, uno::UNO_QUERY );
            if ( xLevProp.is() )
            {
                uno::Any aSubAny;
                try
                {
                    aSubAny = xLevProp->getPropertyValue( SC_UNO_DP_SUBTOTAL2 );
                }
                catch(uno::Exception&)
                {
                }
                uno::Sequence<sal_Int16> aSeq;
                if ( aSubAny >>= aSeq )
                {
                    PivotFunc nMask = PivotFunc::NONE;
                    for (const sal_Int16 nElem : aSeq)
                        nMask |= ScDataPilotConversion::FunctionBit(nElem);
                    return nMask;
                }
            }
        }
    }

    OSL_FAIL("FirstSubTotal: NULL");
    return PivotFunc::NONE;
}

namespace {

class FindByColumn
{
    SCCOL mnCol;
    PivotFunc mnMask;
public:
    FindByColumn(SCCOL nCol, PivotFunc nMask) : mnCol(nCol), mnMask(nMask) {}
    bool operator() (const ScPivotField& r) const
    {
        return r.nCol == mnCol && r.nFuncMask == mnMask;
    }
};

}

static void lcl_FillOldFields( ScPivotFieldVector& rFields,
    const uno::Reference<sheet::XDimensionsSupplier>& xSource,
    sheet::DataPilotFieldOrientation nOrient, bool bAddData )
{
    ScPivotFieldVector aFields;

    bool bDataFound = false;

    //TODO: merge multiple occurrences (data field with different functions)
    //TODO: force data field in one dimension

    vector<tools::Long> aPos;

    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
    tools::Long nDimCount = xDims->getCount();
    for (tools::Long nDim = 0; nDim < nDimCount; ++nDim)
    {
        // dimension properties
        uno::Reference<beans::XPropertySet> xDimProp(xDims->getByIndex(nDim), uno::UNO_QUERY);

        // dimension orientation, hidden by default.
        sheet::DataPilotFieldOrientation nDimOrient = ScUnoHelpFunctions::GetEnumProperty(
                            xDimProp, SC_UNO_DP_ORIENTATION,
                            sheet::DataPilotFieldOrientation_HIDDEN );

        if ( xDimProp.is() && nDimOrient == nOrient )
        {
            // Let's take this dimension.

            // function mask.
            PivotFunc nMask = PivotFunc::NONE;
            if ( nOrient == sheet::DataPilotFieldOrientation_DATA )
            {
                sal_Int16 eFunc = ScUnoHelpFunctions::GetShortProperty(
                                  xDimProp, SC_UNO_DP_FUNCTION2,
                                  sheet::GeneralFunction2::NONE );
                if ( eFunc == sheet::GeneralFunction2::AUTO )
                {
                    //TODO: test for numeric data
                    eFunc = sheet::GeneralFunction2::SUM;
                }
                nMask = ScDataPilotConversion::FunctionBit(eFunc);
            }
            else
                nMask = lcl_FirstSubTotal( xDimProp );      // from first hierarchy

            // is this data layout dimension?
            bool bDataLayout = ScUnoHelpFunctions::GetBoolProperty(
                xDimProp, SC_UNO_DP_ISDATALAYOUT);

            // is this dimension cloned?
            tools::Long nDupSource = -1;
            try
            {
                uno::Any aOrigAny = xDimProp->getPropertyValue(SC_UNO_DP_ORIGINAL_POS);
                sal_Int32 nTmp = 0;
                if (aOrigAny >>= nTmp)
                    nDupSource = nTmp;
            }
            catch(uno::Exception&)
            {
            }

            sal_uInt8 nDupCount = 0;
            if (nDupSource >= 0)
            {
                // this dimension is cloned.

                SCCOL nCompCol; // ID of the original dimension.
                if ( bDataLayout )
                    nCompCol = PIVOT_DATA_FIELD;
                else
                    nCompCol = static_cast<SCCOL>(nDupSource);     //TODO: seek source column from name

                ScPivotFieldVector::iterator it = std::find_if(aFields.begin(), aFields.end(), FindByColumn(nCompCol, nMask));
                if (it != aFields.end())
                    nDupCount = it->mnDupCount + 1;
            }

            aFields.emplace_back();
            ScPivotField& rField = aFields.back();
            if (bDataLayout)
            {
                rField.nCol = PIVOT_DATA_FIELD;
                bDataFound = true;
            }
            else
            {
                rField.mnOriginalDim = nDupSource;
                rField.nCol = static_cast<SCCOL>(nDim);    //TODO: seek source column from name
            }

            rField.nFuncMask = nMask;
            rField.mnDupCount = nDupCount;
            tools::Long nPos = ScUnoHelpFunctions::GetLongProperty(
                xDimProp, SC_UNO_DP_POSITION);
            aPos.push_back(nPos);

            try
            {
                if (nOrient == sheet::DataPilotFieldOrientation_DATA)
                    xDimProp->getPropertyValue(SC_UNO_DP_REFVALUE)
                        >>= rField.maFieldRef;
            }
            catch (uno::Exception&)
            {
            }
        }
    }

    //  sort by getPosition() value

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
        aFields.emplace_back(PIVOT_DATA_FIELD);

    rFields.swap(aFields);
}

void ScDPObject::FillOldParam(ScPivotParam& rParam) const
{
    const_cast<ScDPObject*>(this)->CreateObjects(); // mxSource is needed for field numbers

    if (!mxSource.is())
        return;

    rParam.nCol = maOutputRange.aStart.Col();
    rParam.nRow = maOutputRange.aStart.Row();
    rParam.nTab = maOutputRange.aStart.Tab();
    // ppLabelArr / nLabels is not changed

    bool bAddData = lcl_GetDataGetOrientation(mxSource) == sheet::DataPilotFieldOrientation_HIDDEN;
    lcl_FillOldFields(
        rParam.maPageFields, mxSource, sheet::DataPilotFieldOrientation_PAGE, false);
    lcl_FillOldFields(
        rParam.maColFields, mxSource, sheet::DataPilotFieldOrientation_COLUMN, bAddData);
    lcl_FillOldFields(
        rParam.maRowFields, mxSource, sheet::DataPilotFieldOrientation_ROW, false);
    lcl_FillOldFields(
        rParam.maDataFields, mxSource, sheet::DataPilotFieldOrientation_DATA, false);

    uno::Reference<beans::XPropertySet> xProp(mxSource, uno::UNO_QUERY);
    if (!xProp.is())
        return;

    try
    {
        rParam.bMakeTotalCol = ScUnoHelpFunctions::GetBoolProperty( xProp,
                    SC_UNO_DP_COLGRAND, true );
        rParam.bMakeTotalRow = ScUnoHelpFunctions::GetBoolProperty( xProp,
                    SC_UNO_DP_ROWGRAND, true );

        // following properties may be missing for external sources
        rParam.bIgnoreEmptyRows = ScUnoHelpFunctions::GetBoolProperty( xProp,
                    SC_UNO_DP_IGNOREEMPTY );
        rParam.bDetectCategories = ScUnoHelpFunctions::GetBoolProperty( xProp,
                    SC_UNO_DP_REPEATEMPTY );
    }
    catch(uno::Exception&)
    {
        // no error
    }
}

static void lcl_FillLabelData( ScDPLabelData& rData, const uno::Reference< beans::XPropertySet >& xDimProp )
{
    uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDimProp, uno::UNO_QUERY );
    if (!xDimProp.is() || !xDimSupp.is())
        return;

    uno::Reference<container::XIndexAccess> xHiers = new ScNameToIndexAccess( xDimSupp->getHierarchies() );
    tools::Long nHierarchy = ScUnoHelpFunctions::GetLongProperty(
        xDimProp, SC_UNO_DP_USEDHIERARCHY);
    if ( nHierarchy >= xHiers->getCount() )
        nHierarchy = 0;
    rData.mnUsedHier = nHierarchy;

    uno::Reference<sheet::XLevelsSupplier> xHierSupp(xHiers->getByIndex(nHierarchy),
                                                     uno::UNO_QUERY);
    if (!xHierSupp.is())
        return;

    uno::Reference<container::XIndexAccess> xLevels =
        new ScNameToIndexAccess( xHierSupp->getLevels() );

    uno::Reference<beans::XPropertySet> xLevProp(xLevels->getByIndex(0), uno::UNO_QUERY);
    if (!xLevProp.is())
        return;

    rData.mbShowAll = ScUnoHelpFunctions::GetBoolProperty(
        xLevProp, SC_UNO_DP_SHOWEMPTY);

    rData.mbRepeatItemLabels = ScUnoHelpFunctions::GetBoolProperty(
        xLevProp, SC_UNO_DP_REPEATITEMLABELS);

    try
    {
        xLevProp->getPropertyValue( SC_UNO_DP_SORTING )
            >>= rData.maSortInfo;
        xLevProp->getPropertyValue( SC_UNO_DP_LAYOUT )
            >>= rData.maLayoutInfo;
        xLevProp->getPropertyValue( SC_UNO_DP_AUTOSHOW )
            >>= rData.maShowInfo;
    }
    catch(uno::Exception&)
    {
    }
}

void ScDPObject::FillLabelDataForDimension(
    const uno::Reference<container::XIndexAccess>& xDims, sal_Int32 nDim, ScDPLabelData& rLabelData)
{
    uno::Reference<uno::XInterface> xIntDim(xDims->getByIndex(nDim), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
    uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );

    if (!xDimName.is() || !xDimProp.is())
        return;

    bool bData = ScUnoHelpFunctions::GetBoolProperty(
        xDimProp, SC_UNO_DP_ISDATALAYOUT);
    //TODO: error checking -- is "IsDataLayoutDimension" property required??

    sal_Int32 nOrigPos = -1;
    OUString aFieldName;
    try
    {
        aFieldName = xDimName->getName();
        uno::Any aOrigAny = xDimProp->getPropertyValue(SC_UNO_DP_ORIGINAL_POS);
        aOrigAny >>= nOrigPos;
    }
    catch(uno::Exception&)
    {
    }

    OUString aLayoutName = ScUnoHelpFunctions::GetStringProperty(
        xDimProp, SC_UNO_DP_LAYOUTNAME, OUString());

    OUString aSubtotalName = ScUnoHelpFunctions::GetStringProperty(
        xDimProp, SC_UNO_DP_FIELD_SUBTOTALNAME, OUString());

    // Name from the UNO dimension object may have trailing '*'s in which
    // case it's a duplicate dimension. Convert that to a duplicate index.

    sal_uInt8 nDupCount = ScDPUtil::getDuplicateIndex(aFieldName);
    aFieldName = ScDPUtil::getSourceDimensionName(aFieldName);

    rLabelData.maName = aFieldName;
    rLabelData.mnCol = static_cast<SCCOL>(nDim);
    rLabelData.mnDupCount = nDupCount;
    rLabelData.mbDataLayout = bData;
    rLabelData.mbIsValue = true; //TODO: check

    if (bData)
        return;

    rLabelData.mnOriginalDim = static_cast<tools::Long>(nOrigPos);
    rLabelData.maLayoutName = aLayoutName;
    rLabelData.maSubtotalName = aSubtotalName;
    if (nOrigPos >= 0)
        // This is a duplicated dimension. Use the original dimension index.
        nDim = nOrigPos;
    GetHierarchies(nDim, rLabelData.maHiers);
    GetMembers(nDim, GetUsedHierarchy(nDim), rLabelData.maMembers);
    lcl_FillLabelData(rLabelData, xDimProp);
    rLabelData.mnFlags = ScUnoHelpFunctions::GetLongProperty(
        xDimProp, SC_UNO_DP_FLAGS );
}

void ScDPObject::FillLabelData(sal_Int32 nDim, ScDPLabelData& rLabels)
{
    CreateObjects();
    if (!mxSource.is())
        return;

    uno::Reference<container::XNameAccess> xDimsName = mxSource->getDimensions();
    uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
    sal_Int32 nDimCount = xDims->getCount();
    if (nDimCount <= 0 || nDim >= nDimCount)
        return;

    FillLabelDataForDimension(xDims, nDim, rLabels);
}

void ScDPObject::FillLabelData(ScPivotParam& rParam)
{
    rParam.maLabelArray.clear();

    CreateObjects();
    if (!mxSource.is())
        return;

    uno::Reference<container::XNameAccess> xDimsName = mxSource->getDimensions();
    uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
    sal_Int32 nDimCount = xDims->getCount();
    if (nDimCount <= 0)
        return;

    for (sal_Int32 nDim = 0; nDim < nDimCount; ++nDim)
    {
        ScDPLabelData* pNewLabel = new ScDPLabelData;
        FillLabelDataForDimension(xDims, nDim, *pNewLabel);
        rParam.maLabelArray.push_back(std::unique_ptr<ScDPLabelData>(pNewLabel));
    }
}

void ScDPObject::GetFieldIdsNames(sheet::DataPilotFieldOrientation nOrient, std::vector<tools::Long>& rIndices,
                                     std::vector<OUString>& rNames)
{
    CreateObjects();
    if (!mxSource.is())
        return;

    uno::Reference<container::XNameAccess> xDimsName = mxSource->getDimensions();
    uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
    tools::Long nDimCount = xDims->getCount();
    for (tools::Long nDim = 0; nDim < nDimCount; ++nDim)
    {
        uno::Reference<uno::XInterface> xIntDim(xDims->getByIndex(nDim), uno::UNO_QUERY);
        uno::Reference<container::XNamed> xDimName(xIntDim, uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xDimProp(xIntDim, uno::UNO_QUERY);

        sheet::DataPilotFieldOrientation nDimOrient = ScUnoHelpFunctions::GetEnumProperty(
                            xDimProp, SC_UNO_DP_ORIENTATION,
                            sheet::DataPilotFieldOrientation_HIDDEN );

        if ( xDimProp.is() && nDimOrient == nOrient)
        {
            rIndices.push_back(nDim);
            rNames.push_back(xDimName->getName());
        }
    }
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

void ScDPObject::GetHierarchies( sal_Int32 nDim, uno::Sequence< OUString >& rHiers )
{
    uno::Reference< container::XNameAccess > xHiersNA;
    if( GetHierarchiesNA( nDim, xHiersNA ) )
    {
        rHiers = xHiersNA->getElementNames();
    }
}

sal_Int32 ScDPObject::GetUsedHierarchy( sal_Int32 nDim )
{
    sal_Int32 nHier = 0;
    uno::Reference<container::XNameAccess> xDimsName( GetSource()->getDimensions() );
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    uno::Reference<beans::XPropertySet> xDim(xIntDims->getByIndex( nDim ), uno::UNO_QUERY);
    if (xDim.is())
        nHier = ScUnoHelpFunctions::GetLongProperty( xDim, SC_UNO_DP_USEDHIERARCHY );
    return nHier;
}

bool ScDPObject::GetMembersNA( sal_Int32 nDim, uno::Reference< sheet::XMembersAccess >& xMembers )
{
    return GetMembersNA( nDim, GetUsedHierarchy( nDim ), xMembers );
}

bool ScDPObject::GetMembersNA( sal_Int32 nDim, sal_Int32 nHier, uno::Reference< sheet::XMembersAccess >& xMembers )
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

//  convert old pivot tables into new datapilot tables

namespace {

OUString lcl_GetDimName( const uno::Reference<sheet::XDimensionsSupplier>& xSource, tools::Long nDim )
{
    OUString aName;
    if ( xSource.is() )
    {
        uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
        uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
        tools::Long nDimCount = xDims->getCount();
        if ( nDim < nDimCount )
        {
            uno::Reference<container::XNamed> xDimName(xDims->getByIndex(nDim), uno::UNO_QUERY);
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

    return std::any_of(pRefFields->begin(), pRefFields->end(),
        [&nCol](const ScPivotField& rField) {
            // This array of fields contains the specified column.
            return rField.nCol == nCol; });
}

class FindByOriginalDim
{
    tools::Long mnDim;
public:
    explicit FindByOriginalDim(tools::Long nDim) : mnDim(nDim) {}
    bool operator() (const ScPivotField& r) const
    {
        return mnDim == r.getOriginalDim();
    }
};

}

void ScDPObject::ConvertOrientation(
    ScDPSaveData& rSaveData, const ScPivotFieldVector& rFields, sheet::DataPilotFieldOrientation nOrient,
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

        tools::Long nCol = rField.getOriginalDim();
        PivotFunc nFuncs = rField.nFuncMask;
        const sheet::DataPilotFieldReference& rFieldRef = rField.maFieldRef;

        ScDPSaveDimension* pDim = nullptr;
        if ( nCol == PIVOT_DATA_FIELD )
            pDim = rSaveData.GetDataLayoutDimension();
        else
        {
            OUString aDocStr = lcl_GetDimName( xSource, nCol );   // cols must start at 0
            if (!aDocStr.isEmpty())
                pDim = rSaveData.GetDimensionByName(aDocStr);
            else
                pDim = nullptr;
        }

        if (!pDim)
            continue;

        if ( nOrient == sheet::DataPilotFieldOrientation_DATA )     // set summary function
        {
            //  generate an individual entry for each function
            bool bFirst = true;

            //  if a dimension is used for column/row/page and data,
            //  use duplicated dimensions for all data occurrences
            if (hasFieldColumn(pRefColFields, nCol))
                bFirst = false;

            if (bFirst && hasFieldColumn(pRefRowFields, nCol))
                bFirst = false;

            if (bFirst && hasFieldColumn(pRefPageFields, nCol))
                bFirst = false;

            if (bFirst)
            {
                //  if set via api, a data column may occur several times
                //  (if the function hasn't been changed yet) -> also look for duplicate data column
                bFirst = std::none_of(itrBeg, itr, FindByOriginalDim(nCol));
            }

            ScGeneralFunction eFunc = ScDataPilotConversion::FirstFunc(rField.nFuncMask);
            if (!bFirst)
                pDim = rSaveData.DuplicateDimension(pDim->GetName());
            pDim->SetOrientation(nOrient);
            pDim->SetFunction(eFunc);

            if( rFieldRef.ReferenceType == sheet::DataPilotFieldReferenceType::NONE )
                pDim->SetReferenceValue(nullptr);
            else
                pDim->SetReferenceValue(&rFieldRef);
        }
        else                                            // set SubTotals
        {
            pDim->SetOrientation( nOrient );

            std::vector<ScGeneralFunction> nSubTotalFuncs;
            nSubTotalFuncs.reserve(16);
            sal_uInt16 nMask = 1;
            for (sal_uInt16 nBit=0; nBit<16; nBit++)
            {
                if ( nFuncs & static_cast<PivotFunc>(nMask) )
                    nSubTotalFuncs.push_back( ScDataPilotConversion::FirstFunc( static_cast<PivotFunc>(nMask) ) );
                nMask *= 2;
            }
            pDim->SetSubTotals( std::move(nSubTotalFuncs) );

            //  ShowEmpty was implicit in old tables,
            //  must be set for data layout dimension (not accessible in dialog)
            if ( nCol == PIVOT_DATA_FIELD )
                pDim->SetShowEmpty( true );
        }

        size_t nDimIndex = rField.nCol;
        pDim->RemoveLayoutName();
        pDim->RemoveSubtotalName();
        if (nDimIndex < rLabels.size())
        {
            const ScDPLabelData& rLabel = *rLabels[nDimIndex];
            if (!rLabel.maLayoutName.isEmpty())
                pDim->SetLayoutName(rLabel.maLayoutName);
            if (!rLabel.maSubtotalName.isEmpty())
                pDim->SetSubtotalName(rLabel.maSubtotalName);
        }
    }
}

bool ScDPObject::IsOrientationAllowed( sheet::DataPilotFieldOrientation nOrient, sal_Int32 nDimFlags )
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
                // allowed to remove from previous orientation
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
                                        SCDPSOURCE_SERVICE );
        if ( xEnum.is() && xEnum->hasMoreElements() )
            bFound = true;
    }

    return bFound;
}

std::vector<OUString> ScDPObject::GetRegisteredSources()
{
    std::vector<OUString> aVec;

    //  use implementation names...

    uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
    uno::Reference<container::XContentEnumerationAccess> xEnAc( xManager, uno::UNO_QUERY );
    if ( xEnAc.is() )
    {
        uno::Reference<container::XEnumeration> xEnum = xEnAc->createContentEnumeration(
                                        SCDPSOURCE_SERVICE );
        if ( xEnum.is() )
        {
            while ( xEnum->hasMoreElements() )
            {
                uno::Any aAddInAny = xEnum->nextElement();
//              if ( aAddInAny.getReflection()->getTypeClass() == TypeClass_INTERFACE )
                {
                    uno::Reference<uno::XInterface> xIntFac;
                    aAddInAny >>= xIntFac;
                    if ( xIntFac.is() )
                    {
                        uno::Reference<lang::XServiceInfo> xInfo( xIntFac, uno::UNO_QUERY );
                        if ( xInfo.is() )
                        {
                            OUString sName = xInfo->getImplementationName();
                            aVec.push_back( sName );
                        }
                    }
                }
            }
        }
    }

    return aVec;
}

uno::Reference<sheet::XDimensionsSupplier> ScDPObject::CreateSource( const ScDPServiceDesc& rDesc )
{
    OUString aImplName = rDesc.aServiceName;
    uno::Reference<sheet::XDimensionsSupplier> xRet;

    uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
    uno::Reference<container::XContentEnumerationAccess> xEnAc(xManager, uno::UNO_QUERY);
    if (!xEnAc.is())
        return xRet;

    uno::Reference<container::XEnumeration> xEnum =
        xEnAc->createContentEnumeration(SCDPSOURCE_SERVICE);
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
            // #i113160# try XSingleComponentFactory in addition to (old) XSingleServiceFactory,
            // passing the context to the component (see ScUnoAddInCollection::Initialize)

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
                //  initialize
                uno::Sequence<uno::Any> aSeq(4);
                uno::Any* pArray = aSeq.getArray();
                pArray[0] <<= rDesc.aParSource;
                pArray[1] <<= rDesc.aParName;
                pArray[2] <<= rDesc.aParUser;
                pArray[3] <<= rDesc.aParPass;
                xInit->initialize( aSeq );
            }
            xRet.set( xInterface, uno::UNO_QUERY );
        }
        catch(uno::Exception&)
        {
        }
    }

    return xRet;
}

#if DUMP_PIVOT_TABLE

void ScDPObject::Dump() const
{
    if (mpSaveData)
        mpSaveData->Dump();

    if (mpTableData)
        mpTableData->Dump();
}

void ScDPObject::DumpCache() const
{
    if (!mpTableData)
        return;

    const ScDPCache &rCache = mpTableData->GetCacheTable().getCache();

    rCache.Dump();
}
#endif

ScDPCollection::SheetCaches::SheetCaches(ScDocument& rDoc) : mrDoc(rDoc) {}

namespace {

struct FindInvalidRange
{
    bool operator() (const ScRange& r) const
    {
        return !r.IsValid();
    }
};

void setGroupItemsToCache( ScDPCache& rCache, const o3tl::sorted_vector<ScDPObject*>& rRefs )
{
    // Go through all referencing pivot tables, and re-fill the group dimension info.
    for (const ScDPObject* pObj : rRefs)
    {
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

    // Already cached.
    size_t nIndex = std::distance(maRanges.begin(), it);
    CachesType::const_iterator const itCache = m_Caches.find(nIndex);
    return itCache != m_Caches.end();
}

const ScDPCache* ScDPCollection::SheetCaches::getCache(const ScRange& rRange, const ScDPDimensionSaveData* pDimData)
{
    RangeIndexType::iterator it = std::find(maRanges.begin(), maRanges.end(), rRange);
    if (it != maRanges.end())
    {
        // Already cached.
        size_t nIndex = std::distance(maRanges.begin(), it);
        CachesType::iterator const itCache = m_Caches.find(nIndex);
        if (itCache == m_Caches.end())
        {
            OSL_FAIL("Cache pool and index pool out-of-sync !!!");
            return nullptr;
        }

        if (pDimData)
        {
            (itCache->second)->ClearGroupFields();
            pDimData->WriteToCache(*itCache->second);
        }

        return itCache->second.get();
    }

    // Not cached.  Create a new cache.
    ::std::unique_ptr<ScDPCache> pCache(new ScDPCache(mrDoc));
    pCache->InitFromDoc(mrDoc, rRange);
    if (pDimData)
        pDimData->WriteToCache(*pCache);

    // Get the smallest available range index.
    it = std::find_if(maRanges.begin(), maRanges.end(), FindInvalidRange());

    size_t nIndex = maRanges.size();
    if (it == maRanges.end())
    {
        // All range indices are valid.  Append a new index.
        maRanges.push_back(rRange);
    }
    else
    {
        // Slot with invalid range.  Re-use this slot.
        *it = rRange;
        nIndex = std::distance(maRanges.begin(), it);
    }

    const ScDPCache* p = pCache.get();
    m_Caches.insert(std::make_pair(nIndex, std::move(pCache)));
    return p;
}

ScDPCache* ScDPCollection::SheetCaches::getExistingCache(const ScRange& rRange)
{
    RangeIndexType::iterator it = std::find(maRanges.begin(), maRanges.end(), rRange);
    if (it == maRanges.end())
        // Not cached.
        return nullptr;

    // Already cached.
    size_t nIndex = std::distance(maRanges.begin(), it);
    CachesType::iterator const itCache = m_Caches.find(nIndex);
    if (itCache == m_Caches.end())
    {
        OSL_FAIL("Cache pool and index pool out-of-sync !!!");
        return nullptr;
    }

    return itCache->second.get();
}

const ScDPCache* ScDPCollection::SheetCaches::getExistingCache(const ScRange& rRange) const
{
    RangeIndexType::const_iterator it = std::find(maRanges.begin(), maRanges.end(), rRange);
    if (it == maRanges.end())
        // Not cached.
        return nullptr;

    // Already cached.
    size_t nIndex = std::distance(maRanges.begin(), it);
    CachesType::const_iterator const itCache = m_Caches.find(nIndex);
    if (itCache == m_Caches.end())
    {
        OSL_FAIL("Cache pool and index pool out-of-sync !!!");
        return nullptr;
    }

    return itCache->second.get();
}

size_t ScDPCollection::SheetCaches::size() const
{
    return m_Caches.size();
}

void ScDPCollection::SheetCaches::updateReference(
    UpdateRefMode eMode, const ScRange& r, SCCOL nDx, SCROW nDy, SCTAB nDz)
{
    if (maRanges.empty())
        // No caches.
        return;

    for (ScRange& rKeyRange : maRanges)
    {
        SCCOL nCol1 = rKeyRange.aStart.Col();
        SCROW nRow1 = rKeyRange.aStart.Row();
        SCTAB nTab1 = rKeyRange.aStart.Tab();
        SCCOL nCol2 = rKeyRange.aEnd.Col();
        SCROW nRow2 = rKeyRange.aEnd.Row();
        SCTAB nTab2 = rKeyRange.aEnd.Tab();

        ScRefUpdateRes eRes = ScRefUpdate::Update(
            &mrDoc, eMode,
            r.aStart.Col(), r.aStart.Row(), r.aStart.Tab(),
            r.aEnd.Col(), r.aEnd.Row(), r.aEnd.Tab(), nDx, nDy, nDz,
            nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);

        if (eRes != UR_NOTHING)
        {
            // range updated.
            ScRange aNew(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            rKeyRange = aNew;
        }
    }
}

void ScDPCollection::SheetCaches::updateCache(const ScRange& rRange, o3tl::sorted_vector<ScDPObject*>& rRefs)
{
    RangeIndexType::iterator it = std::find(maRanges.begin(), maRanges.end(), rRange);
    if (it == maRanges.end())
    {
        // Not cached.  Nothing to do.
        rRefs.clear();
        return;
    }

    size_t nIndex = std::distance(maRanges.begin(), it);
    CachesType::iterator const itCache = m_Caches.find(nIndex);
    if (itCache == m_Caches.end())
    {
        OSL_FAIL("Cache pool and index pool out-of-sync !!!");
        rRefs.clear();
        return;
    }

    ScDPCache& rCache = *itCache->second;

    // Update the cache with new cell values. This will clear all group dimension info.
    rCache.InitFromDoc(mrDoc, rRange);

    o3tl::sorted_vector<ScDPObject*> aRefs(rCache.GetAllReferences());
    rRefs.swap(aRefs);

    // Make sure to re-populate the group dimension info.
    setGroupItemsToCache(rCache, rRefs);
}

bool ScDPCollection::SheetCaches::remove(const ScDPCache* p)
{
    CachesType::iterator it = std::find_if(m_Caches.begin(), m_Caches.end(),
        [&p](const CachesType::value_type& rEntry) { return rEntry.second.get() == p; });
    if (it != m_Caches.end())
    {
        size_t idx = it->first;
        m_Caches.erase(it);
        maRanges[idx].SetInvalid();
        return true;
    }
    return false;
}

const std::vector<ScRange>& ScDPCollection::SheetCaches::getAllRanges() const
{
    return maRanges;
}

ScDPCollection::NameCaches::NameCaches(ScDocument& rDoc) : mrDoc(rDoc) {}

bool ScDPCollection::NameCaches::hasCache(const OUString& rName) const
{
    return m_Caches.count(rName) != 0;
}

const ScDPCache* ScDPCollection::NameCaches::getCache(
    const OUString& rName, const ScRange& rRange, const ScDPDimensionSaveData* pDimData)
{
    CachesType::const_iterator const itr = m_Caches.find(rName);
    if (itr != m_Caches.end())
        // already cached.
        return itr->second.get();

    ::std::unique_ptr<ScDPCache> pCache(new ScDPCache(mrDoc));
    pCache->InitFromDoc(mrDoc, rRange);
    if (pDimData)
        pDimData->WriteToCache(*pCache);

    const ScDPCache *const p = pCache.get();
    m_Caches.insert(std::make_pair(rName, std::move(pCache)));
    return p;
}

ScDPCache* ScDPCollection::NameCaches::getExistingCache(const OUString& rName)
{
    CachesType::iterator const itr = m_Caches.find(rName);
    return itr != m_Caches.end() ? itr->second.get() : nullptr;
}

size_t ScDPCollection::NameCaches::size() const
{
    return m_Caches.size();
}

void ScDPCollection::NameCaches::updateCache(
    const OUString& rName, const ScRange& rRange, o3tl::sorted_vector<ScDPObject*>& rRefs)
{
    CachesType::iterator const itr = m_Caches.find(rName);
    if (itr == m_Caches.end())
    {
        rRefs.clear();
        return;
    }

    ScDPCache& rCache = *itr->second;
    // Update the cache with new cell values. This will clear all group dimension info.
    rCache.InitFromDoc(mrDoc, rRange);

    o3tl::sorted_vector<ScDPObject*> aRefs(rCache.GetAllReferences());
    rRefs.swap(aRefs);

    // Make sure to re-populate the group dimension info.
    setGroupItemsToCache(rCache, rRefs);
}

bool ScDPCollection::NameCaches::remove(const ScDPCache* p)
{
    CachesType::iterator it = std::find_if(m_Caches.begin(), m_Caches.end(),
        [&p](const CachesType::value_type& rEntry) { return rEntry.second.get() == p; });
    if (it != m_Caches.end())
    {
        m_Caches.erase(it);
        return true;
    }
    return false;
}

ScDPCollection::DBType::DBType(sal_Int32 nSdbType, OUString aDBName, OUString aCommand) :
    mnSdbType(nSdbType), maDBName(std::move(aDBName)), maCommand(std::move(aCommand)) {}

bool ScDPCollection::DBType::less::operator() (const DBType& left, const DBType& right) const
{
    return left < right;
}

ScDPCollection::DBCaches::DBCaches(ScDocument& rDoc) : mrDoc(rDoc) {}

bool ScDPCollection::DBCaches::hasCache(sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand) const
{
    DBType aType(nSdbType, rDBName, rCommand);
    CachesType::const_iterator const itr = m_Caches.find(aType);
    return itr != m_Caches.end();
}

const ScDPCache* ScDPCollection::DBCaches::getCache(
    sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand,
    const ScDPDimensionSaveData* pDimData)
{
    DBType aType(nSdbType, rDBName, rCommand);
    CachesType::const_iterator const itr = m_Caches.find(aType);
    if (itr != m_Caches.end())
        // already cached.
        return itr->second.get();

    uno::Reference<sdbc::XRowSet> xRowSet = createRowSet(nSdbType, rDBName, rCommand);
    if (!xRowSet.is())
        return nullptr;

    ::std::unique_ptr<ScDPCache> pCache(new ScDPCache(mrDoc));
    SvNumberFormatter aFormat( comphelper::getProcessComponentContext(), ScGlobal::eLnge);
    DBConnector aDB(*pCache, xRowSet, aFormat.GetNullDate());
    if (!aDB.isValid())
        return nullptr;

    if (!pCache->InitFromDataBase(aDB))
    {
        // initialization failed.
        comphelper::disposeComponent(xRowSet);
        return nullptr;
    }

    if (pDimData)
        pDimData->WriteToCache(*pCache);

    ::comphelper::disposeComponent(xRowSet);
    const ScDPCache* p = pCache.get();
    m_Caches.insert(std::make_pair(aType, std::move(pCache)));
    return p;
}

ScDPCache* ScDPCollection::DBCaches::getExistingCache(
    sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand)
{
    DBType aType(nSdbType, rDBName, rCommand);
    CachesType::iterator const itr = m_Caches.find(aType);
    return itr != m_Caches.end() ? itr->second.get() : nullptr;
}

uno::Reference<sdbc::XRowSet> ScDPCollection::DBCaches::createRowSet(
    sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand)
{
    uno::Reference<sdbc::XRowSet> xRowSet;
    try
    {
        xRowSet.set(comphelper::getProcessServiceFactory()->createInstance(
                       SC_SERVICE_ROWSET),
                    UNO_QUERY);

        uno::Reference<beans::XPropertySet> xRowProp(xRowSet, UNO_QUERY);
        OSL_ENSURE( xRowProp.is(), "can't get RowSet" );
        if (!xRowProp.is())
        {
            xRowSet.clear();
            return xRowSet;
        }

        //  set source parameters

        xRowProp->setPropertyValue( SC_DBPROP_DATASOURCENAME, Any(rDBName) );
        xRowProp->setPropertyValue( SC_DBPROP_COMMAND, Any(rCommand) );
        xRowProp->setPropertyValue( SC_DBPROP_COMMANDTYPE, Any(nSdbType) );

        uno::Reference<sdb::XCompletedExecution> xExecute( xRowSet, uno::UNO_QUERY );
        if ( xExecute.is() )
        {
            uno::Reference<task::XInteractionHandler> xHandler(
                task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(), nullptr),
                uno::UNO_QUERY_THROW);
            xExecute->executeWithCompletion( xHandler );
        }
        else
            xRowSet->execute();

        return xRowSet;
    }
    catch ( const sdbc::SQLException& rError )
    {
        //! store error message
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(ScDocShell::GetActiveDialogParent(),
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      rError.Message));
        xInfoBox->run();
    }
    catch ( uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sc", "Unexpected exception in database");
    }

    xRowSet.clear();
    return xRowSet;
}

void ScDPCollection::DBCaches::updateCache(
    sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand,
    o3tl::sorted_vector<ScDPObject*>& rRefs)
{
    DBType aType(nSdbType, rDBName, rCommand);
    CachesType::iterator const it = m_Caches.find(aType);
    if (it == m_Caches.end())
    {
        // not cached.
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
    DBConnector aDB(rCache, xRowSet, aFormat.GetNullDate());
    if (!aDB.isValid())
        return;

    if (!rCache.InitFromDataBase(aDB))
    {
        // initialization failed.
        rRefs.clear();
        comphelper::disposeComponent(xRowSet);
        return;
    }

    comphelper::disposeComponent(xRowSet);
    o3tl::sorted_vector<ScDPObject*> aRefs(rCache.GetAllReferences());
    aRefs.swap(rRefs);

    // Make sure to re-populate the group dimension info.
    setGroupItemsToCache(rCache, rRefs);
}

bool ScDPCollection::DBCaches::remove(const ScDPCache* p)
{
    CachesType::iterator it = std::find_if(m_Caches.begin(), m_Caches.end(),
        [&p](const CachesType::value_type& rEntry) { return rEntry.second.get() == p; });
    if (it != m_Caches.end())
    {
        m_Caches.erase(it);
        return true;
    }
    return false;
}

ScDPCollection::ScDPCollection(ScDocument& rDocument) :
    mrDoc(rDocument),
    maSheetCaches(rDocument),
    maNameCaches(rDocument),
    maDBCaches(rDocument)
{
}

ScDPCollection::ScDPCollection(const ScDPCollection& r) :
    mrDoc(r.mrDoc),
    maSheetCaches(r.mrDoc),
    maNameCaches(r.mrDoc),
    maDBCaches(r.mrDoc)
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
class MatchByTable
{
    SCTAB mnTab;
public:
    explicit MatchByTable(SCTAB nTab) : mnTab(nTab) {}

    bool operator() (const std::unique_ptr<ScDPObject>& rObj) const
    {
        return rObj->GetOutRange().aStart.Tab() == mnTab;
    }
};

}

TranslateId ScDPCollection::ReloadCache(const ScDPObject* pDPObj, o3tl::sorted_vector<ScDPObject*>& rRefs)
{
    if (!pDPObj)
        return STR_ERR_DATAPILOTSOURCE;

    if (pDPObj->IsSheetData())
    {
        // data source is internal sheet.
        const ScSheetSourceDesc* pDesc = pDPObj->GetSheetDesc();
        if (!pDesc)
            return STR_ERR_DATAPILOTSOURCE;

        TranslateId pErrId = pDesc->CheckSourceRange();
        if (pErrId)
            return pErrId;

        if (pDesc->HasRangeName())
        {
            // cache by named range
            ScDPCollection::NameCaches& rCaches = GetNameCaches();
            if (rCaches.hasCache(pDesc->GetRangeName()))
                rCaches.updateCache(pDesc->GetRangeName(), pDesc->GetSourceRange(), rRefs);
            else
            {
                // Not cached yet.  Collect all tables that use this named
                // range as data source.
                GetAllTables(pDesc->GetRangeName(), rRefs);
            }
        }
        else
        {
            // cache by cell range
            ScDPCollection::SheetCaches& rCaches = GetSheetCaches();
            if (rCaches.hasCache(pDesc->GetSourceRange()))
                rCaches.updateCache(pDesc->GetSourceRange(), rRefs);
            else
            {
                // Not cached yet.  Collect all tables that use this range as
                // data source.
                GetAllTables(pDesc->GetSourceRange(), rRefs);
            }
        }
    }
    else if (pDPObj->IsImportData())
    {
        // data source is external database.
        const ScImportSourceDesc* pDesc = pDPObj->GetImportSourceDesc();
        if (!pDesc)
            return STR_ERR_DATAPILOTSOURCE;

        ScDPCollection::DBCaches& rCaches = GetDBCaches();
        if (rCaches.hasCache(pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject))
            rCaches.updateCache(
                pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject, rRefs);
        else
        {
            // Not cached yet.  Collect all tables that use this range as
            // data source.
            GetAllTables(pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject, rRefs);
        }
    }
    return {};
}

bool ScDPCollection::ReloadGroupsInCache(const ScDPObject* pDPObj, o3tl::sorted_vector<ScDPObject*>& rRefs)
{
    if (!pDPObj)
        return false;

    const ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    if (!pSaveData)
        return false;

    // Note: Unlike reloading cache, when modifying the group dimensions the
    // cache may not have all its references when this method is called.
    // Therefore, we need to always call GetAllTables to get its correct
    // references even when the cache exists.  This may become a non-issue
    // if/when we implement loading and saving of pivot caches.

    ScDPCache* pCache = nullptr;

    if (pDPObj->IsSheetData())
    {
        // data source is internal sheet.
        const ScSheetSourceDesc* pDesc = pDPObj->GetSheetDesc();
        if (!pDesc)
            return false;

        if (pDesc->HasRangeName())
        {
            // cache by named range
            ScDPCollection::NameCaches& rCaches = GetNameCaches();
            if (rCaches.hasCache(pDesc->GetRangeName()))
                pCache = rCaches.getExistingCache(pDesc->GetRangeName());
            else
            {
                // Not cached yet.  Cache the source dimensions.  Groups will
                // be added below.
                pCache = const_cast<ScDPCache*>(
                    rCaches.getCache(pDesc->GetRangeName(), pDesc->GetSourceRange(), nullptr));
            }
            GetAllTables(pDesc->GetRangeName(), rRefs);
        }
        else
        {
            // cache by cell range
            ScDPCollection::SheetCaches& rCaches = GetSheetCaches();
            if (rCaches.hasCache(pDesc->GetSourceRange()))
                pCache = rCaches.getExistingCache(pDesc->GetSourceRange());
            else
            {
                // Not cached yet.  Cache the source dimensions.  Groups will
                // be added below.
                pCache = const_cast<ScDPCache*>(
                    rCaches.getCache(pDesc->GetSourceRange(), nullptr));
            }
            GetAllTables(pDesc->GetSourceRange(), rRefs);
        }
    }
    else if (pDPObj->IsImportData())
    {
        // data source is external database.
        const ScImportSourceDesc* pDesc = pDPObj->GetImportSourceDesc();
        if (!pDesc)
            return false;

        ScDPCollection::DBCaches& rCaches = GetDBCaches();
        if (rCaches.hasCache(pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject))
            pCache = rCaches.getExistingCache(
                pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject);
        else
        {
            // Not cached yet.  Cache the source dimensions.  Groups will
            // be added below.
            pCache = const_cast<ScDPCache*>(
                rCaches.getCache(pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject, nullptr));
        }
        GetAllTables(pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject, rRefs);
    }

    if (!pCache)
        return false;

    // Clear the existing group/field data from the cache, and rebuild it from the
    // dimension data.
    pCache->ClearAllFields();
    const ScDPDimensionSaveData* pDimData = pSaveData->GetExistingDimensionData();
    if (pDimData)
        pDimData->WriteToCache(*pCache);
    return true;
}

bool ScDPCollection::GetReferenceGroups(const ScDPObject& rDPObj, const ScDPDimensionSaveData** pGroups) const
{
    for (const std::unique_ptr<ScDPObject>& aTable : maTables)
    {
        const ScDPObject& rRefObj = *aTable;

        if (&rRefObj == &rDPObj)
            continue;

        if (rDPObj.IsSheetData()){
            if(!rRefObj.IsSheetData())
                continue;

            const ScSheetSourceDesc* pDesc = rDPObj.GetSheetDesc();
            const ScSheetSourceDesc* pRefDesc = rRefObj.GetSheetDesc();
            if (pDesc == nullptr || pRefDesc == nullptr)
                continue;

            if (pDesc->HasRangeName())
            {
                if (!pRefDesc->HasRangeName())
                    continue;

                if (pDesc->GetRangeName() == pRefDesc->GetRangeName())
                {
                    *pGroups = rRefObj.GetSaveData()->GetExistingDimensionData();
                    return true;
                }
            }
            else
            {
                if (pRefDesc->HasRangeName())
                    continue;

                if (pDesc->GetSourceRange() == pRefDesc->GetSourceRange())
                {
                    *pGroups = rRefObj.GetSaveData()->GetExistingDimensionData();
                    return true;
                }
            }
        }
        else if (rDPObj.IsImportData())
        {
            if (!rRefObj.IsImportData ())
                continue;

            const ScImportSourceDesc* pDesc = rDPObj.GetImportSourceDesc();
            const ScImportSourceDesc* pRefDesc = rRefObj.GetImportSourceDesc();
            if (pDesc == nullptr || pRefDesc == nullptr)
                continue;

            if (pDesc->aDBName == pRefDesc->aDBName &&
                pDesc->aObject == pRefDesc->aObject &&
                pDesc->GetCommandType() == pRefDesc->GetCommandType())
            {
                *pGroups = rRefObj.GetSaveData()->GetExistingDimensionData();
                return true;
            }

        }
    }
    return false;
}


void ScDPCollection::DeleteOnTab( SCTAB nTab )
{
    std::erase_if(maTables, MatchByTable(nTab));
}

void ScDPCollection::UpdateReference( UpdateRefMode eUpdateRefMode,
                                         const ScRange& r, SCCOL nDx, SCROW nDy, SCTAB nDz )
{
    for (auto& rxTable : maTables)
        rxTable->UpdateReference(eUpdateRefMode, r, nDx, nDy, nDz);

    // Update the source ranges of the caches.
    maSheetCaches.updateReference(eUpdateRefMode, r, nDx, nDy, nDz);
}

void ScDPCollection::CopyToTab( SCTAB nOld, SCTAB nNew )
{
    TablesType aAdded;
    for (const auto& rxTable : maTables)
    {
        const ScDPObject& rObj = *rxTable;
        ScRange maOutputRange = rObj.GetOutRange();
        if (maOutputRange.aStart.Tab() != nOld)
            continue;

        ScAddress& start = maOutputRange.aStart;
        ScAddress& end = maOutputRange.aEnd;
        start.SetTab(nNew);
        end.SetTab(nNew);
        ScDPObject* pNew = new ScDPObject(rObj);
        pNew->SetOutRange(maOutputRange);
        mrDoc.ApplyFlagsTab(start.Col(), start.Row(), end.Col(), end.Row(), start.Tab(), ScMF::DpTable);
        aAdded.push_back(std::unique_ptr<ScDPObject>(pNew));
    }

    std::move(aAdded.begin(), aAdded.end(), std::back_inserter(maTables));
}

bool ScDPCollection::RefsEqual( const ScDPCollection& r ) const
{
    return std::equal(maTables.begin(), maTables.end(), r.maTables.begin(), r.maTables.end(),
        [](const TablesType::value_type& a, const TablesType::value_type& b) { return a->RefsEqual(*b); });
}

void ScDPCollection::WriteRefsTo( ScDPCollection& r ) const
{
    if ( maTables.size() == r.maTables.size() )
    {
        //TODO: assert equal names?
        TablesType::iterator itr2 = r.maTables.begin();
        for (const auto& rxTable : maTables)
        {
            rxTable->WriteRefsTo(**itr2);
            ++itr2;
        }
    }
    else
    {
        // #i8180# If data pilot tables were deleted with their sheet,
        // this collection contains extra entries that must be restored.
        // Matching objects are found by their names.
        size_t nSrcSize = maTables.size();
        size_t nDestSize = r.maTables.size();
        OSL_ENSURE( nSrcSize >= nDestSize, "WriteRefsTo: missing entries in document" );
        for (size_t nSrcPos = 0; nSrcPos < nSrcSize; ++nSrcPos)
        {
            const ScDPObject& rSrcObj = *maTables[nSrcPos];
            const OUString& aName = rSrcObj.GetName();
            bool bFound = false;
            for (size_t nDestPos = 0; nDestPos < nDestSize && !bFound; ++nDestPos)
            {
                ScDPObject& rDestObj = *r.maTables[nDestPos];
                if (rDestObj.GetName() == aName)
                {
                    rSrcObj.WriteRefsTo(rDestObj);     // found object, copy refs
                    bFound = true;
                }
            }

            if (!bFound)
            {
                // none found, re-insert deleted object (see ScUndoDataPilot::Undo)
                r.InsertNewTable(std::make_unique<ScDPObject>(rSrcObj));
            }
        }
        OSL_ENSURE( maTables.size() == r.maTables.size(), "WriteRefsTo: couldn't restore all entries" );
    }
}

size_t ScDPCollection::GetCount() const
{
    return maTables.size();
}

ScDPObject& ScDPCollection::operator [](size_t nIndex)
{
    return *maTables[nIndex];
}

const ScDPObject& ScDPCollection::operator [](size_t nIndex) const
{
    return *maTables[nIndex];
}

ScDPObject* ScDPCollection::GetByName(std::u16string_view rName) const
{
    for (std::unique_ptr<ScDPObject> const & pObject : maTables)
    {
        if (pObject->GetName() == rName)
            return pObject.get();
    }

    return nullptr;
}

OUString ScDPCollection::CreateNewName() const
{
    size_t n = maTables.size();
    for (size_t nAdd = 0; nAdd <= n; ++nAdd)   //  nCount+1 tries
    {
        OUString aNewName = "DataPilot" + OUString::number(1 + nAdd);
        if (std::none_of(maTables.begin(), maTables.end(),
                         [&aNewName](const TablesType::value_type& rxObj) { return rxObj->GetName() == aNewName; }))
            return aNewName;            // found unused Name
    }
    return OUString();                    // should not happen
}

void ScDPCollection::FreeTable(const ScDPObject* pDPObject)
{
    const ScRange& rOutRange = pDPObject->GetOutRange();
    const ScAddress& s = rOutRange.aStart;
    const ScAddress& e = rOutRange.aEnd;
    mrDoc.RemoveFlagsTab(s.Col(), s.Row(), e.Col(), e.Row(), s.Tab(), ScMF::DpTable);

    auto funcRemoveCondition = [pDPObject] (std::unique_ptr<ScDPObject> const & pCurrent)
    {
        return pCurrent.get() == pDPObject;
    };

    std::erase_if(maTables, funcRemoveCondition);
}

ScDPObject* ScDPCollection::InsertNewTable(std::unique_ptr<ScDPObject> pDPObj)
{
    const ScRange& rOutRange = pDPObj->GetOutRange();
    const ScAddress& s = rOutRange.aStart;
    const ScAddress& e = rOutRange.aEnd;
    mrDoc.ApplyFlagsTab(s.Col(), s.Row(), e.Col(), e.Row(), s.Tab(), ScMF::DpTable);

    maTables.push_back(std::move(pDPObj));
    return maTables.back().get();
}

bool ScDPCollection::HasTable(const ScDPObject* pDPObj) const
{
    for (const std::unique_ptr<ScDPObject>& aTable : maTables)
    {
        if (aTable.get() == pDPObj)
        {
            return true;
        }
    }
    return false;
}

ScDPCollection::SheetCaches& ScDPCollection::GetSheetCaches()
{
    return maSheetCaches;
}

const ScDPCollection::SheetCaches& ScDPCollection::GetSheetCaches() const
{
    return maSheetCaches;
}

ScDPCollection::NameCaches& ScDPCollection::GetNameCaches()
{
    return maNameCaches;
}

const ScDPCollection::NameCaches& ScDPCollection::GetNameCaches() const
{
    return maNameCaches;
}

ScDPCollection::DBCaches& ScDPCollection::GetDBCaches()
{
    return maDBCaches;
}

const ScDPCollection::DBCaches& ScDPCollection::GetDBCaches() const
{
    return maDBCaches;
}

ScRangeList ScDPCollection::GetAllTableRanges( SCTAB nTab ) const
{
    return std::for_each(maTables.begin(), maTables.end(), AccumulateOutputRanges(nTab)).getRanges();
}

bool ScDPCollection::IntersectsTableByColumns( SCCOL nCol1, SCCOL nCol2, SCROW nRow, SCTAB nTab ) const
{
    return std::any_of(maTables.begin(), maTables.end(), FindIntersectingTableByColumns(nCol1, nCol2, nRow, nTab));
}

bool ScDPCollection::IntersectsTableByRows( SCCOL nCol, SCROW nRow1, SCROW nRow2, SCTAB nTab ) const
{
    return std::any_of(maTables.begin(), maTables.end(), FindIntersectingTableByRows(nCol, nRow1, nRow2, nTab));
}

bool ScDPCollection::HasTable( const ScRange& rRange ) const
{
    return std::any_of(maTables.begin(), maTables.end(), FindIntersectingTable(rRange));
}

#if DEBUG_PIVOT_TABLE

namespace {

struct DumpTable
{
    void operator() (const std::unique_ptr<ScDPObject>& rObj) const
    {
        cout << "-- '" << rObj->GetName() << "'" << endl;
        ScDPSaveData* pSaveData = rObj->GetSaveData();
        if (!pSaveData)
            return;

        pSaveData->Dump();

        cout << endl; // blank line
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
        // sheet cache removed.
        return;

    if (maNameCaches.remove(pCache))
        // named range cache removed.
        return;

    if (maDBCaches.remove(pCache))
        // database cache removed.
        return;
}

void ScDPCollection::GetAllTables(const ScRange& rSrcRange, o3tl::sorted_vector<ScDPObject*>& rRefs) const
{
    o3tl::sorted_vector<ScDPObject*> aRefs;
    for (const auto& rxTable : maTables)
    {
        const ScDPObject& rObj = *rxTable;
        if (!rObj.IsSheetData())
            // Source is not a sheet range.
            continue;

        const ScSheetSourceDesc* pDesc = rObj.GetSheetDesc();
        if (!pDesc)
            continue;

        if (pDesc->HasRangeName())
            // This table has a range name as its source.
            continue;

        if (pDesc->GetSourceRange() != rSrcRange)
            // Different source range.
            continue;

        aRefs.insert(const_cast<ScDPObject*>(&rObj));
    }

    rRefs.swap(aRefs);
}

void ScDPCollection::GetAllTables(std::u16string_view rSrcName, o3tl::sorted_vector<ScDPObject*>& rRefs) const
{
    o3tl::sorted_vector<ScDPObject*> aRefs;
    for (const auto& rxTable : maTables)
    {
        const ScDPObject& rObj = *rxTable;
        if (!rObj.IsSheetData())
            // Source is not a sheet range.
            continue;

        const ScSheetSourceDesc* pDesc = rObj.GetSheetDesc();
        if (!pDesc)
            continue;

        if (!pDesc->HasRangeName())
            // This table probably has a sheet range as its source.
            continue;

        if (pDesc->GetRangeName() != rSrcName)
            // Different source name.
            continue;

        aRefs.insert(const_cast<ScDPObject*>(&rObj));
    }

    rRefs.swap(aRefs);
}

void ScDPCollection::GetAllTables(
    sal_Int32 nSdbType, std::u16string_view rDBName, std::u16string_view rCommand,
    o3tl::sorted_vector<ScDPObject*>& rRefs) const
{
    o3tl::sorted_vector<ScDPObject*> aRefs;
    for (const auto& rxTable : maTables)
    {
        const ScDPObject& rObj = *rxTable;
        if (!rObj.IsImportData())
            // Source data is not a database.
            continue;

        const ScImportSourceDesc* pDesc = rObj.GetImportSourceDesc();
        if (!pDesc)
            continue;

        if (pDesc->aDBName != rDBName || pDesc->aObject != rCommand || pDesc->GetCommandType() != nSdbType)
            // Different database source.
            continue;

        aRefs.insert(const_cast<ScDPObject*>(&rObj));
    }

    rRefs.swap(aRefs);
}

bool operator<(const ScDPCollection::DBType& left, const ScDPCollection::DBType& right)
{
    if (left.mnSdbType != right.mnSdbType)
        return left.mnSdbType < right.mnSdbType;

    if (left.maDBName != right.maDBName)
        return left.maDBName < right.maDBName;

    return left.maCommand < right.maCommand;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
