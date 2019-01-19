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

#include "xmldpimp.hxx"
#include "xmlimprt.hxx"
#include "xmlfilti.hxx"
#include <document.hxx>
#include <dpshttab.hxx>
#include <dpsdbtab.hxx>
#include <attrib.hxx>
#include "XMLConverter.hxx"
#include <dpdimsave.hxx>
#include <rangeutl.hxx>
#include <dpoutputgeometry.hxx>
#include <generalfunction.hxx>

#include "pivotsource.hxx"

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>

#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceItemType.hpp>
#include <com/sun/star/sheet/DataPilotFieldShowItemsMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>

using namespace com::sun::star;
using namespace xmloff::token;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;

ScXMLDataPilotTablesContext::ScXMLDataPilotTablesContext( ScXMLImport& rImport ) :
    ScXMLImportContext( rImport )
{
    // has no Attributes
    rImport.LockSolarMutex();
}

ScXMLDataPilotTablesContext::~ScXMLDataPilotTablesContext()
{
    GetScImport().UnlockSolarMutex();
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDataPilotTablesContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_DATA_PILOT_TABLE ) :
        {
            pContext = new ScXMLDataPilotTableContext( GetScImport(), pAttribList );
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

ScXMLDataPilotTableContext::GrandTotalItem::GrandTotalItem() :
    mbVisible(true) {}

ScXMLDataPilotTableContext::ScXMLDataPilotTableContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList ) :
    ScXMLImportContext( rImport ),
    pDoc(GetScImport().GetDocument()),
    pDPSave(new ScDPSaveData()),
    sDataPilotTableName(),
    sApplicationData(),
    nSourceType(SQL),
    mnRowFieldCount(0),
    mnColFieldCount(0),
    mnPageFieldCount(0),
    mnDataFieldCount(0),
    mnDataLayoutType(sheet::DataPilotFieldOrientation_HIDDEN),
    bIsNative(true),
    bIgnoreEmptyRows(false),
    bIdentifyCategories(false),
    bTargetRangeAddress(false),
    bSourceCellRange(false),
    bShowFilter(true),
    bDrillDown(true),
    bHeaderGridLayout(false)
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_NAME ):
                {
                    sDataPilotTableName = aIter.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_APPLICATION_DATA ):
                {
                    sApplicationData = aIter.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_GRAND_TOTAL ):
                {
                    if (IsXMLToken(aIter, XML_BOTH))
                    {
                        maRowGrandTotal.mbVisible = true;
                        maColGrandTotal.mbVisible = true;
                    }
                    else if (IsXMLToken(aIter, XML_ROW))
                    {
                        maRowGrandTotal.mbVisible = true;
                        maColGrandTotal.mbVisible = false;
                    }
                    else if (IsXMLToken(aIter, XML_COLUMN))
                    {
                        maRowGrandTotal.mbVisible = false;
                        maColGrandTotal.mbVisible = true;
                    }
                    else
                    {
                        maRowGrandTotal.mbVisible = false;
                        maColGrandTotal.mbVisible = false;
                    }
                }
                break;
                case XML_ELEMENT( TABLE, XML_IGNORE_EMPTY_ROWS ):
                {
                    bIgnoreEmptyRows = IsXMLToken(aIter, XML_TRUE);
                }
                break;
                case XML_ELEMENT( TABLE, XML_IDENTIFY_CATEGORIES ):
                {
                    bIdentifyCategories = IsXMLToken(aIter, XML_TRUE);
                }
                break;
                case XML_ELEMENT( TABLE, XML_TARGET_RANGE_ADDRESS ):
                {
                    sal_Int32 nOffset(0);
                    bTargetRangeAddress = ScRangeStringConverter::GetRangeFromString( aTargetRangeAddress, aIter.toString(), pDoc, ::formula::FormulaGrammar::CONV_OOO, nOffset );
                }
                break;
                case XML_ELEMENT( TABLE, XML_BUTTONS ):
                {
                    sButtons = aIter.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_SHOW_FILTER_BUTTON ):
                {
                    bShowFilter = IsXMLToken(aIter, XML_TRUE);
                }
                break;
                case XML_ELEMENT( TABLE, XML_DRILL_DOWN_ON_DOUBLE_CLICK ):
                {
                    bDrillDown = IsXMLToken(aIter, XML_TRUE);
                }
                break;
                case XML_ELEMENT( TABLE, XML_HEADER_GRID_LAYOUT ):
                {
                    bHeaderGridLayout = IsXMLToken(aIter, XML_TRUE);
                }
                break;
            }
        }
    }
}

ScXMLDataPilotTableContext::~ScXMLDataPilotTableContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDataPilotTableContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_DATABASE_SOURCE_SQL ):
        {
            pContext = new ScXMLDPSourceSQLContext(GetScImport(), pAttribList, this);
            nSourceType = SQL;
        }
        break;
        case XML_ELEMENT( TABLE, XML_DATABASE_SOURCE_TABLE ):
        {
            pContext = new ScXMLDPSourceTableContext(GetScImport(), pAttribList, this);
            nSourceType = TABLE;
        }
        break;
        case XML_ELEMENT( TABLE, XML_DATABASE_SOURCE_QUERY ):
        {
            pContext = new ScXMLDPSourceQueryContext(GetScImport(), pAttribList, this);
            nSourceType = QUERY;
        }
        break;
        case XML_ELEMENT( TABLE, XML_SOURCE_SERVICE ):
        {
            pContext = new ScXMLSourceServiceContext(GetScImport(), pAttribList, this);
            nSourceType = SERVICE;
        }
        break;
        case XML_ELEMENT( TABLE, XML_DATA_PILOT_GRAND_TOTAL ):
        case XML_ELEMENT( TABLE_EXT, XML_DATA_PILOT_GRAND_TOTAL ):
        {
            pContext = new ScXMLDataPilotGrandTotalContext(GetScImport(), pAttribList, this);
        }
        break;
        case XML_ELEMENT( TABLE, XML_SOURCE_CELL_RANGE ):
        {
            pContext = new ScXMLSourceCellRangeContext(GetScImport(), pAttribList, this);
            nSourceType = CELLRANGE;
        }
        break;
        case XML_ELEMENT( TABLE, XML_DATA_PILOT_FIELD ):
            pContext = new ScXMLDataPilotFieldContext(GetScImport(), pAttribList, this);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

namespace {

const ScDPSaveDimension* getDimension(
    const std::vector<const ScDPSaveDimension*>& rRowDims,
    const std::vector<const ScDPSaveDimension*>& rColDims,
    const std::vector<const ScDPSaveDimension*>& rPageDims,
    ScDPOutputGeometry::FieldType eType, size_t nPos)
{
    switch (eType)
    {
        case ScDPOutputGeometry::Column:
        {
            if (rColDims.size() <= nPos)
                return nullptr;

            return rColDims[nPos];
        }
        case ScDPOutputGeometry::Row:
        {
            if (rRowDims.size() <= nPos)
                return nullptr;

            return rRowDims[nPos];
        }
        case ScDPOutputGeometry::Page:
        {
            if (rPageDims.size() <= nPos)
                return nullptr;

            return rPageDims[nPos];
        }
        case ScDPOutputGeometry::Data:
        break;
        case ScDPOutputGeometry::None:
        break;
        default:
            break;
    }
    return nullptr;
}

ScDPOutputGeometry::FieldType toFieldType(sheet::DataPilotFieldOrientation nOrient)
{
    switch (nOrient)
    {
        case sheet::DataPilotFieldOrientation_COLUMN:
            return ScDPOutputGeometry::Column;
        case sheet::DataPilotFieldOrientation_DATA:
            return ScDPOutputGeometry::Data;
        case sheet::DataPilotFieldOrientation_PAGE:
            return ScDPOutputGeometry::Page;
        case sheet::DataPilotFieldOrientation_ROW:
            return ScDPOutputGeometry::Row;
        case sheet::DataPilotFieldOrientation_HIDDEN:
            break;
        default:
            break;
    }
    return ScDPOutputGeometry::None;
}

}

void ScXMLDataPilotTableContext::SetButtons(ScDPObject* pDPObject)
{
    ScDPOutputGeometry aGeometry(aTargetRangeAddress, bShowFilter);
    aGeometry.setColumnFieldCount(mnColFieldCount);
    aGeometry.setRowFieldCount(mnRowFieldCount);
    aGeometry.setPageFieldCount(mnPageFieldCount);
    aGeometry.setDataFieldCount(mnDataFieldCount);
    aGeometry.setDataLayoutType(toFieldType(mnDataLayoutType));
    aGeometry.setHeaderLayout(bHeaderGridLayout);

    std::vector<const ScDPSaveDimension*> aRowDims, aColDims, aPageDims;
    pDPSave->GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_ROW, aRowDims);
    pDPSave->GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_COLUMN, aColDims);
    pDPSave->GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_PAGE, aPageDims);

    OUString sAddress;
    sal_Int32 nOffset = 0;
    while( nOffset >= 0 )
    {
        ScRangeStringConverter::GetTokenByOffset( sAddress, sButtons, nOffset );
        if( nOffset >= 0 )
        {
            ScAddress aScAddress;
            sal_Int32 nAddrOffset(0);
            if (pDoc && ScRangeStringConverter::GetAddressFromString( aScAddress, sAddress, pDoc, ::formula::FormulaGrammar::CONV_OOO, nAddrOffset ))
            {
                std::pair<ScDPOutputGeometry::FieldType, size_t> aBtnType = aGeometry.getFieldButtonType(aScAddress);
                const ScDPSaveDimension* pDim = getDimension(
                    aRowDims, aColDims, aPageDims, aBtnType.first, aBtnType.second);

                bool bDimension = pDim != nullptr;
                bool bDataLayout = pDim && pDim->IsDataLayout();
                bool bHasHidden = pDim && pDim->HasInvisibleMember();
                bool bPageDim = pDim && pDim->GetOrientation() == sheet::DataPilotFieldOrientation_PAGE;

                if (bPageDim)
                {
                    // Page dimension needs 2 buttons.

                    pDoc->ApplyFlagsTab(aScAddress.Col(), aScAddress.Row(), aScAddress.Col(), aScAddress.Row(), aScAddress.Tab(), ScMF::Button);

                    ScMF nMFlag = ScMF::ButtonPopup;
                    if (bHasHidden)
                        nMFlag |= ScMF::HiddenMember;
                    pDoc->ApplyFlagsTab(aScAddress.Col()+1, aScAddress.Row(), aScAddress.Col()+1, aScAddress.Row(), aScAddress.Tab(), nMFlag);
                }
                else
                {
                    ScMF nMFlag = ScMF::Button;
                    if (bDataLayout)
                    {
                        // Data layout dimension only has a plain button with no popup.
                    }
                    else if (bDimension)
                    {
                        // Normal dimension has a popup arrow button.
                        if (bHasHidden)
                            nMFlag |= ScMF::HiddenMember;

                        nMFlag |= ScMF::ButtonPopup;
                    }

                    pDoc->ApplyFlagsTab(aScAddress.Col(), aScAddress.Row(), aScAddress.Col(), aScAddress.Row(), aScAddress.Tab(), nMFlag);
                }
            }
        }
    }

    pDPObject->RefreshAfterLoad();
}

void ScXMLDataPilotTableContext::SetSelectedPage( const OUString& rDimName, const OUString& rSelected )
{
    maSelectedPages.emplace(rDimName, rSelected);
}

void ScXMLDataPilotTableContext::AddDimension(ScDPSaveDimension* pDim)
{
    if (pDPSave)
    {
        if (pDim->IsDataLayout())
            mnDataLayoutType = pDim->GetOrientation();

        //  if a dimension with that name has already been inserted,
        //  mark the new one as duplicate
        if ( !pDim->IsDataLayout() &&
                pDPSave->GetExistingDimensionByName(pDim->GetName()) )
            pDim->SetDupFlag(true);

        switch (pDim->GetOrientation())
        {
            case sheet::DataPilotFieldOrientation_ROW:
                ++mnRowFieldCount;
            break;
            case sheet::DataPilotFieldOrientation_COLUMN:
                ++mnColFieldCount;
            break;
            case sheet::DataPilotFieldOrientation_PAGE:
                ++mnPageFieldCount;
            break;
            case sheet::DataPilotFieldOrientation_DATA:
                ++mnDataFieldCount;
            break;
            case sheet::DataPilotFieldOrientation_HIDDEN:
                break;
            default:
                break;
        }

        pDPSave->AddDimension(pDim);
    }
}

void ScXMLDataPilotTableContext::AddGroupDim(const ScDPSaveNumGroupDimension& aNumGroupDim)
{
    if (!pDPDimSaveData)
        pDPDimSaveData.reset( new ScDPDimensionSaveData );
    pDPDimSaveData->AddNumGroupDimension(aNumGroupDim);
}

void ScXMLDataPilotTableContext::AddGroupDim(const ScDPSaveGroupDimension& aGroupDim)
{
    if (!pDPDimSaveData)
        pDPDimSaveData.reset( new ScDPDimensionSaveData );
    pDPDimSaveData->AddGroupDimension(aGroupDim);
}

void SAL_CALL ScXMLDataPilotTableContext::endFastElement( sal_Int32 /*nElement*/ )
{
    if (!bTargetRangeAddress)
        return;

    std::unique_ptr<ScDPObject> pDPObject(new ScDPObject(pDoc));
    pDPObject->SetName(sDataPilotTableName);
    pDPObject->SetTag(sApplicationData);
    pDPObject->SetOutRange(aTargetRangeAddress);
    pDPObject->SetHeaderLayout(bHeaderGridLayout);

    sc::PivotTableSources& rPivotSources = GetScImport().GetPivotTableSources();

    switch (nSourceType)
    {
        case SQL :
        {
            ScImportSourceDesc aImportDesc(pDoc);
            aImportDesc.aDBName = sDatabaseName;
            aImportDesc.aObject = sSourceObject;
            aImportDesc.nType = sheet::DataImportMode_SQL;
            aImportDesc.bNative = bIsNative;
            rPivotSources.appendDBSource(pDPObject.get(), aImportDesc);
        }
        break;
        case TABLE :
        {
            ScImportSourceDesc aImportDesc(pDoc);
            aImportDesc.aDBName = sDatabaseName;
            aImportDesc.aObject = sSourceObject;
            aImportDesc.nType = sheet::DataImportMode_TABLE;
            rPivotSources.appendDBSource(pDPObject.get(), aImportDesc);
        }
        break;
        case QUERY :
        {
            ScImportSourceDesc aImportDesc(pDoc);
            aImportDesc.aDBName = sDatabaseName;
            aImportDesc.aObject = sSourceObject;
            aImportDesc.nType = sheet::DataImportMode_QUERY;
            rPivotSources.appendDBSource(pDPObject.get(), aImportDesc);
        }
        break;
        case SERVICE :
        {
            ScDPServiceDesc aServiceDesc(sServiceName, sServiceSourceName, sServiceSourceObject,
                                sServiceUsername, sServicePassword);
            rPivotSources.appendServiceSource(pDPObject.get(), aServiceDesc);
        }
        break;
        case CELLRANGE :
        {
            if (bSourceCellRange)
            {
                ScSheetSourceDesc aSheetDesc(pDoc);
                if (!sSourceRangeName.isEmpty())
                    // Range name takes precedence.
                    aSheetDesc.SetRangeName(sSourceRangeName);
                else
                    aSheetDesc.SetSourceRange(aSourceCellRangeAddress);
                aSheetDesc.SetQueryParam(aSourceQueryParam);
                rPivotSources.appendSheetSource(pDPObject.get(), aSheetDesc);
            }
        }
        break;
    }

    rPivotSources.appendSelectedPages(pDPObject.get(), maSelectedPages);

    pDPSave->SetRowGrand(maRowGrandTotal.mbVisible);
    pDPSave->SetColumnGrand(maColGrandTotal.mbVisible);
    if (!maRowGrandTotal.maDisplayName.isEmpty())
        // TODO: Right now, we only support one grand total name for both
        // column and row totals.  Take the value from the row total for
        // now.
        pDPSave->SetGrandTotalName(maRowGrandTotal.maDisplayName);

    pDPSave->SetIgnoreEmptyRows(bIgnoreEmptyRows);
    pDPSave->SetRepeatIfEmpty(bIdentifyCategories);
    pDPSave->SetFilterButton(bShowFilter);
    pDPSave->SetDrillDown(bDrillDown);
    if (pDPDimSaveData)
        pDPSave->SetDimensionData(pDPDimSaveData.get());
    pDPObject->SetSaveData(*pDPSave);

    ScDPCollection* pDPCollection = pDoc->GetDPCollection();

    // #i94570# Names have to be unique, or the tables can't be accessed by API.
    if ( pDPCollection->GetByName(pDPObject->GetName()) )
        pDPObject->SetName( OUString() );     // ignore the invalid name, create a new name in AfterXMLLoading

    SetButtons(pDPObject.get());

    pDPCollection->InsertNewTable(std::move(pDPObject));
}

void ScXMLDataPilotTableContext::SetGrandTotal(
    XMLTokenEnum eOrientation, bool bVisible, const OUString& rDisplayName)
{
    switch (eOrientation)
    {
        case XML_BOTH:
            maRowGrandTotal.mbVisible     = bVisible;
            maRowGrandTotal.maDisplayName = rDisplayName;
            maColGrandTotal.mbVisible     = bVisible;
            maColGrandTotal.maDisplayName = rDisplayName;
        break;
        case XML_ROW:
            maRowGrandTotal.mbVisible     = bVisible;
            maRowGrandTotal.maDisplayName = rDisplayName;
        break;
        case XML_COLUMN:
            maColGrandTotal.mbVisible     = bVisible;
            maColGrandTotal.maDisplayName = rDisplayName;
        break;
        default:
            break;
    }
}

ScXMLDPSourceSQLContext::ScXMLDPSourceSQLContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDataPilotTableContext* pDataPilotTable) :
    ScXMLImportContext( rImport )
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_DATABASE_NAME ):
                    pDataPilotTable->SetDatabaseName(aIter.toString());
                break;
                case XML_ELEMENT( TABLE, XML_SQL_STATEMENT ):
                    pDataPilotTable->SetSourceObject(aIter.toString());
                break;
                case XML_ELEMENT( TABLE, XML_PARSE_SQL_STATEMENT ):
                    pDataPilotTable->SetNative(!IsXMLToken(aIter, XML_TRUE));
                break;
            }
        }
    }
}

ScXMLDPSourceSQLContext::~ScXMLDPSourceSQLContext()
{
}

ScXMLDPSourceTableContext::ScXMLDPSourceTableContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDataPilotTableContext* pDataPilotTable) :
    ScXMLImportContext( rImport )
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_DATABASE_NAME ):
                    pDataPilotTable->SetDatabaseName(aIter.toString());
                break;
                case XML_ELEMENT( TABLE, XML_TABLE_NAME ):
                case XML_ELEMENT( TABLE, XML_DATABASE_TABLE_NAME ):
                    pDataPilotTable->SetSourceObject(aIter.toString());
                break;
            }
        }
    }
}

ScXMLDPSourceTableContext::~ScXMLDPSourceTableContext()
{
}

ScXMLDPSourceQueryContext::ScXMLDPSourceQueryContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDataPilotTableContext* pDataPilotTable) :
    ScXMLImportContext( rImport )
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_DATABASE_NAME ):
                    pDataPilotTable->SetDatabaseName(aIter.toString());
                break;
                case XML_ELEMENT( TABLE, XML_QUERY_NAME ):
                    pDataPilotTable->SetSourceObject(aIter.toString());
                break;
            }
        }
    }
}

ScXMLDPSourceQueryContext::~ScXMLDPSourceQueryContext()
{
}

ScXMLSourceServiceContext::ScXMLSourceServiceContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDataPilotTableContext* pDataPilotTable) :
    ScXMLImportContext( rImport )
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_NAME ):
                    pDataPilotTable->SetServiceName(aIter.toString());
                break;
                case XML_ELEMENT( TABLE, XML_SOURCE_NAME ):
                    pDataPilotTable->SetServiceSourceName(aIter.toString());
                break;
                case XML_ELEMENT( TABLE, XML_OBJECT_NAME ):
                    pDataPilotTable->SetServiceSourceObject(aIter.toString());
                break;
                case XML_ELEMENT( TABLE, XML_USER_NAME ):
                    pDataPilotTable->SetServiceUsername(aIter.toString());
                break;
                case XML_ELEMENT( TABLE, XML_PASSWORD ):
                    pDataPilotTable->SetServicePassword(aIter.toString());
                break;
            }
        }
    }
}

ScXMLSourceServiceContext::~ScXMLSourceServiceContext()
{
}

ScXMLDataPilotGrandTotalContext::ScXMLDataPilotGrandTotalContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
    ScXMLDataPilotTableContext* pTableContext ) :
    ScXMLImportContext( rImport ),
    mpTableContext(pTableContext),
    meOrientation(NONE),
    mbVisible(false)
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_DISPLAY ):
                    mbVisible = IsXMLToken(aIter, XML_TRUE);
                break;
                case XML_ELEMENT( TABLE, XML_ORIENTATION ):
                    if (IsXMLToken(aIter, XML_BOTH))
                        meOrientation = BOTH;
                    else if (IsXMLToken(aIter, XML_ROW))
                        meOrientation = ROW;
                    else if (IsXMLToken(aIter, XML_COLUMN))
                        meOrientation = COLUMN;
                break;
                case XML_ELEMENT( TABLE, XML_DISPLAY_NAME ):
                case XML_ELEMENT( TABLE_EXT, XML_DISPLAY_NAME ):
                    maDisplayName = aIter.toString();
                break;
                default:
                    break;
            }
        }
    }
}

ScXMLDataPilotGrandTotalContext::~ScXMLDataPilotGrandTotalContext()
{
}

    void SAL_CALL ScXMLDataPilotGrandTotalContext::endFastElement( sal_Int32 /*nElement*/ )
{
    XMLTokenEnum eOrient = XML_NONE;
    switch (meOrientation)
    {
        case BOTH:
            eOrient = XML_BOTH;
        break;
        case ROW:
            eOrient = XML_ROW;
        break;
        case COLUMN:
            eOrient = XML_COLUMN;
        break;
        default:
            break;
    }
    mpTableContext->SetGrandTotal(eOrient, mbVisible, maDisplayName);
}

ScXMLSourceCellRangeContext::ScXMLSourceCellRangeContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDataPilotTableContext* pTempDataPilotTable) :
    ScXMLImportContext( rImport ),
    pDataPilotTable(pTempDataPilotTable)
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_CELL_RANGE_ADDRESS ):
                {
                    ScRange aSourceRangeAddress;
                    sal_Int32 nOffset(0);
                    if (ScRangeStringConverter::GetRangeFromString( aSourceRangeAddress, aIter.toString(), GetScImport().GetDocument(), ::formula::FormulaGrammar::CONV_OOO, nOffset ))
                        pDataPilotTable->SetSourceCellRangeAddress(aSourceRangeAddress);
                }
                break;
                case XML_ELEMENT( TABLE, XML_NAME ):
                    pDataPilotTable->SetSourceRangeName(aIter.toString());
                break;
            }
        }
    }
}

ScXMLSourceCellRangeContext::~ScXMLSourceCellRangeContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLSourceCellRangeContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_FILTER ):
            pContext = new ScXMLDPFilterContext(GetScImport(), pAttribList, pDataPilotTable);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

ScXMLDataPilotFieldContext::ScXMLDataPilotFieldContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDataPilotTableContext* pTempDataPilotTable) :
    ScXMLImportContext( rImport ),
    pDataPilotTable(pTempDataPilotTable),
    xDim(),
    fStart(0.0),
    fEnd(0.0),
    fStep(0.0),
    nUsedHierarchy(1),
    nGroupPart(0),
    nFunction(ScGeneralFunction::NONE),
    nOrientation(sheet::DataPilotFieldOrientation_HIDDEN),
    bSelectedPage(false),
    bIsGroupField(false),
    bDateValue(false),
    bAutoStart(false),
    bAutoEnd(false),
    mbHasHiddenMember(false)
{
    bool bHasName = false;
    bool bDataLayout = false;
    bool bIgnoreSelectedPage = false;
    OUString aDisplayName;
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_SOURCE_FIELD_NAME ):
                    sName = aIter.toString();
                    bHasName = true;
                break;
                case XML_ELEMENT( TABLE, XML_DISPLAY_NAME ):
                case XML_ELEMENT( TABLE_EXT, XML_DISPLAY_NAME ):
                    aDisplayName = aIter.toString();
                break;
                case XML_ELEMENT( TABLE, XML_IS_DATA_LAYOUT_FIELD ):
                    bDataLayout = IsXMLToken(aIter, XML_TRUE);
                break;
                case XML_ELEMENT( TABLE, XML_FUNCTION ):
                    nFunction = ScXMLConverter::GetFunctionFromString2( aIter.toString() );
                break;
                case XML_ELEMENT( TABLE, XML_ORIENTATION ):
                    nOrientation = ScXMLConverter::GetOrientationFromString( aIter.toString() );
                break;
                case XML_ELEMENT( TABLE, XML_SELECTED_PAGE ):
                    sSelectedPage = aIter.toString();
                    bSelectedPage = true;
                break;
                case XML_ELEMENT( LO_EXT, XML_IGNORE_SELECTED_PAGE ):
                    bIgnoreSelectedPage = true;
                break;
                case XML_ELEMENT( TABLE, XML_USED_HIERARCHY ):
                    nUsedHierarchy = aIter.toInt32();
                break;
            }
        }
    }

    // use the new extension elements
    if (bIgnoreSelectedPage)
        bSelectedPage = false;

    if (bHasName)
    {
        xDim.reset(new ScDPSaveDimension(sName, bDataLayout));
        if (!aDisplayName.isEmpty())
            xDim->SetLayoutName(aDisplayName);
    }
}

ScXMLDataPilotFieldContext::~ScXMLDataPilotFieldContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDataPilotFieldContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_DATA_PILOT_LEVEL ):
            pContext = new ScXMLDataPilotLevelContext(GetScImport(), pAttribList, this);
        break;
        case XML_ELEMENT( TABLE, XML_DATA_PILOT_FIELD_REFERENCE ):
            pContext = new ScXMLDataPilotFieldReferenceContext(GetScImport(), pAttribList, this);
        break;
        case XML_ELEMENT( TABLE, XML_DATA_PILOT_GROUPS ):
            pContext = new ScXMLDataPilotGroupsContext(GetScImport(), pAttribList, this);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void ScXMLDataPilotFieldContext::AddMember(std::unique_ptr<ScDPSaveMember> pMember)
{
    if (xDim)
    {
        bool isVisible = pMember->GetIsVisible();
        xDim->AddMember(std::move(pMember));
        if (!isVisible)
            // This member is hidden.
            mbHasHiddenMember = true;
    }
}

void ScXMLDataPilotFieldContext::SetSubTotalName(const OUString& rName)
{
    if (xDim)
        xDim->SetSubtotalName(rName);
}

void ScXMLDataPilotFieldContext::AddGroup(const ::std::vector<OUString>& rMembers, const OUString& rName)
{
    ScXMLDataPilotGroup aGroup;
    aGroup.aMembers = rMembers;
    aGroup.aName = rName;
    aGroups.push_back(aGroup);
}

void SAL_CALL ScXMLDataPilotFieldContext::endFastElement( sal_Int32 /*nElement*/ )
{
    if (xDim)
    {
        xDim->SetUsedHierarchy(nUsedHierarchy);
        xDim->SetFunction(nFunction);
        xDim->SetOrientation(nOrientation);
        if (bSelectedPage)
        {
            pDataPilotTable->SetSelectedPage(xDim->GetName(), sSelectedPage);
        }
        pDataPilotTable->AddDimension(xDim.release());
        if (bIsGroupField)
        {
            ScDPNumGroupInfo aInfo;
            aInfo.mbEnable = true;
            aInfo.mbDateValues = bDateValue;
            aInfo.mbAutoStart = bAutoStart;
            aInfo.mbAutoEnd = bAutoEnd;
            aInfo.mfStart = fStart;
            aInfo.mfEnd = fEnd;
            aInfo.mfStep = fStep;
            if (!sGroupSource.isEmpty())
            {
                ScDPSaveGroupDimension aGroupDim(sGroupSource, sName);
                if (nGroupPart)
                    aGroupDim.SetDateInfo(aInfo, nGroupPart);
                else
                {
                    for (const auto& rGroup : aGroups)
                    {
                        ScDPSaveGroupItem aItem(rGroup.aName);
                        for (const auto& rMember : rGroup.aMembers)
                        {
                            aItem.AddElement(rMember);
                        }
                        aGroupDim.AddGroupItem(aItem);
                    }
                }
                pDataPilotTable->AddGroupDim(aGroupDim);
            }
            else //NumGroup
            {
                ScDPSaveNumGroupDimension aNumGroupDim(sName, aInfo);
                if (nGroupPart)
                    aNumGroupDim.SetDateInfo(aInfo, nGroupPart);
                pDataPilotTable->AddGroupDim(aNumGroupDim);
            }
        }
    }
}

ScXMLDataPilotFieldReferenceContext::ScXMLDataPilotFieldReferenceContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField) :
    ScXMLImportContext( rImport )
{
    sheet::DataPilotFieldReference aReference;

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_TYPE ):
                {
                    if (IsXMLToken(aIter, XML_NONE))
                        aReference.ReferenceType = sheet::DataPilotFieldReferenceType::NONE;
                    else if (IsXMLToken(aIter, XML_MEMBER_DIFFERENCE))
                        aReference.ReferenceType = sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE;
                    else if (IsXMLToken(aIter, XML_MEMBER_PERCENTAGE))
                        aReference.ReferenceType = sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE;
                    else if (IsXMLToken(aIter, XML_MEMBER_PERCENTAGE_DIFFERENCE))
                        aReference.ReferenceType = sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE;
                    else if (IsXMLToken(aIter, XML_RUNNING_TOTAL))
                        aReference.ReferenceType = sheet::DataPilotFieldReferenceType::RUNNING_TOTAL;
                    else if (IsXMLToken(aIter, XML_ROW_PERCENTAGE))
                        aReference.ReferenceType = sheet::DataPilotFieldReferenceType::ROW_PERCENTAGE;
                    else if (IsXMLToken(aIter, XML_COLUMN_PERCENTAGE))
                        aReference.ReferenceType = sheet::DataPilotFieldReferenceType::COLUMN_PERCENTAGE;
                    else if (IsXMLToken(aIter, XML_TOTAL_PERCENTAGE))
                        aReference.ReferenceType = sheet::DataPilotFieldReferenceType::TOTAL_PERCENTAGE;
                    else if (IsXMLToken(aIter, XML_INDEX))
                        aReference.ReferenceType = sheet::DataPilotFieldReferenceType::INDEX;
                }
                break;
                case XML_ELEMENT( TABLE, XML_FIELD_NAME ):
                {
                    aReference.ReferenceField = aIter.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_MEMBER_TYPE ):
                {
                    if (IsXMLToken(aIter, XML_NAMED))
                        aReference.ReferenceItemType = sheet::DataPilotFieldReferenceItemType::NAMED;
                    else if (IsXMLToken(aIter, XML_PREVIOUS))
                        aReference.ReferenceItemType = sheet::DataPilotFieldReferenceItemType::PREVIOUS;
                    else if (IsXMLToken(aIter, XML_NEXT))
                        aReference.ReferenceItemType = sheet::DataPilotFieldReferenceItemType::NEXT;
                }
                break;
                case XML_ELEMENT( TABLE, XML_MEMBER_NAME ):
                {
                    aReference.ReferenceItemName = aIter.toString();
                }
                break;
            }
        }
    }
    pDataPilotField->SetFieldReference(aReference);
}

ScXMLDataPilotFieldReferenceContext::~ScXMLDataPilotFieldReferenceContext()
{
}

ScXMLDataPilotLevelContext::ScXMLDataPilotLevelContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDataPilotFieldContext* pTempDataPilotField) :
    ScXMLImportContext( rImport ),
    pDataPilotField(pTempDataPilotField)
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_SHOW_EMPTY ):
                    pDataPilotField->SetShowEmpty(IsXMLToken(aIter, XML_TRUE));
                break;
                case XML_ELEMENT( CALC_EXT, XML_REPEAT_ITEM_LABELS ):
                    pDataPilotField->SetRepeatItemLabels(IsXMLToken(aIter, XML_TRUE));
                break;
            }
        }
    }
}

ScXMLDataPilotLevelContext::~ScXMLDataPilotLevelContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDataPilotLevelContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_DATA_PILOT_SUBTOTALS ):
            pContext = new ScXMLDataPilotSubTotalsContext(GetScImport(), pDataPilotField);
        break;
        case XML_ELEMENT( TABLE, XML_DATA_PILOT_MEMBERS ):
            pContext = new ScXMLDataPilotMembersContext(GetScImport(), pDataPilotField);
        break;
        case XML_ELEMENT( TABLE, XML_DATA_PILOT_DISPLAY_INFO ):
            pContext = new ScXMLDataPilotDisplayInfoContext(GetScImport(), pAttribList, pDataPilotField);
        break;
        case XML_ELEMENT( TABLE, XML_DATA_PILOT_SORT_INFO ):
            pContext = new ScXMLDataPilotSortInfoContext(GetScImport(), pAttribList, pDataPilotField);
        break;
        case XML_ELEMENT( TABLE, XML_DATA_PILOT_LAYOUT_INFO ):
            pContext = new ScXMLDataPilotLayoutInfoContext(GetScImport(), pAttribList, pDataPilotField);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

ScXMLDataPilotDisplayInfoContext::ScXMLDataPilotDisplayInfoContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField) :
    ScXMLImportContext( rImport )
{
    sheet::DataPilotFieldAutoShowInfo aInfo;

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_ENABLED ):
                    if (IsXMLToken(aIter, XML_TRUE))
                        aInfo.IsEnabled = true;
                    else
                        aInfo.IsEnabled = false;
                break;
                case XML_ELEMENT( TABLE, XML_DISPLAY_MEMBER_MODE ):
                    if (IsXMLToken(aIter, XML_FROM_TOP))
                        aInfo.ShowItemsMode = sheet::DataPilotFieldShowItemsMode::FROM_TOP;
                    else if (IsXMLToken(aIter, XML_FROM_BOTTOM))
                        aInfo.ShowItemsMode = sheet::DataPilotFieldShowItemsMode::FROM_BOTTOM;
                break;
                case XML_ELEMENT( TABLE, XML_MEMBER_COUNT ):
                    aInfo.ItemCount = aIter.toInt32();
                break;
                case XML_ELEMENT( TABLE, XML_DATA_FIELD ):
                    aInfo.DataField = aIter.toString();
                break;
            }
        }
    }
    pDataPilotField->SetAutoShowInfo(aInfo);
}

ScXMLDataPilotDisplayInfoContext::~ScXMLDataPilotDisplayInfoContext()
{
}

ScXMLDataPilotSortInfoContext::ScXMLDataPilotSortInfoContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField) :
    ScXMLImportContext( rImport )
{
    sheet::DataPilotFieldSortInfo aInfo;

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_ORDER ):
                    if (IsXMLToken(aIter, XML_ASCENDING))
                        aInfo.IsAscending = true;
                    else if (IsXMLToken(aIter, XML_DESCENDING))
                        aInfo.IsAscending = false;
                break;
                case XML_ELEMENT( TABLE, XML_SORT_MODE ):
                    if (IsXMLToken(aIter, XML_NONE))
                        aInfo.Mode = sheet::DataPilotFieldSortMode::NONE;
                    else if (IsXMLToken(aIter, XML_MANUAL))
                        aInfo.Mode = sheet::DataPilotFieldSortMode::MANUAL;
                    else if (IsXMLToken(aIter, XML_NAME))
                        aInfo.Mode = sheet::DataPilotFieldSortMode::NAME;
                    else if (IsXMLToken(aIter, XML_DATA))
                        aInfo.Mode = sheet::DataPilotFieldSortMode::DATA;
                break;
                case XML_ELEMENT( TABLE, XML_DATA_FIELD ):
                    aInfo.Field = aIter.toString();
                break;
            }
        }
    }
    pDataPilotField->SetSortInfo(aInfo);
}

ScXMLDataPilotSortInfoContext::~ScXMLDataPilotSortInfoContext()
{
}

ScXMLDataPilotLayoutInfoContext::ScXMLDataPilotLayoutInfoContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField) :
    ScXMLImportContext( rImport )
{
    sheet::DataPilotFieldLayoutInfo aInfo;

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_ADD_EMPTY_LINES ):
                    if (IsXMLToken(aIter, XML_TRUE))
                        aInfo.AddEmptyLines = true;
                    else
                        aInfo.AddEmptyLines = false;
                break;
                case XML_ELEMENT( TABLE, XML_LAYOUT_MODE ):
                    if (IsXMLToken(aIter, XML_TABULAR_LAYOUT))
                        aInfo.LayoutMode = sheet::DataPilotFieldLayoutMode::TABULAR_LAYOUT;
                    else if (IsXMLToken(aIter, XML_OUTLINE_SUBTOTALS_TOP))
                        aInfo.LayoutMode = sheet::DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_TOP;
                    else if (IsXMLToken(aIter, XML_OUTLINE_SUBTOTALS_BOTTOM))
                        aInfo.LayoutMode = sheet::DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_BOTTOM;
                break;
            }
        }
    }
    pDataPilotField->SetLayoutInfo(aInfo);}

ScXMLDataPilotLayoutInfoContext::~ScXMLDataPilotLayoutInfoContext()
{
}

ScXMLDataPilotSubTotalsContext::ScXMLDataPilotSubTotalsContext( ScXMLImport& rImport,
                                      ScXMLDataPilotFieldContext* pTempDataPilotField) :
    ScXMLImportContext( rImport ),
    pDataPilotField(pTempDataPilotField)
{

    // has no attributes
}

ScXMLDataPilotSubTotalsContext::~ScXMLDataPilotSubTotalsContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDataPilotSubTotalsContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_DATA_PILOT_SUBTOTAL ):
            pContext = new ScXMLDataPilotSubTotalContext(GetScImport(), pAttribList, this);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLDataPilotSubTotalsContext::endFastElement( sal_Int32 /*nElement*/ )
{
    pDataPilotField->SetSubTotals(maFunctions);
    if (!maDisplayName.isEmpty())
        pDataPilotField->SetSubTotalName(maDisplayName);
}

void ScXMLDataPilotSubTotalsContext::AddFunction(ScGeneralFunction nFunction)
{
    maFunctions.push_back(nFunction);
}

void ScXMLDataPilotSubTotalsContext::SetDisplayName(const OUString& rName)
{
    maDisplayName = rName;
}

ScXMLDataPilotSubTotalContext::ScXMLDataPilotSubTotalContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDataPilotSubTotalsContext* pDataPilotSubTotals) :
    ScXMLImportContext( rImport )
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_FUNCTION ):
                    pDataPilotSubTotals->AddFunction( ScXMLConverter::GetFunctionFromString2( aIter.toString() ) );
                break;
                case XML_ELEMENT( TABLE, XML_DISPLAY_NAME ):
                case XML_ELEMENT( TABLE_EXT, XML_DISPLAY_NAME ):
                    pDataPilotSubTotals->SetDisplayName(aIter.toString());
                break;
            }
        }
    }
}

ScXMLDataPilotSubTotalContext::~ScXMLDataPilotSubTotalContext()
{
}

ScXMLDataPilotMembersContext::ScXMLDataPilotMembersContext( ScXMLImport& rImport,
                                      ScXMLDataPilotFieldContext* pTempDataPilotField) :
    ScXMLImportContext( rImport ),
    pDataPilotField(pTempDataPilotField)
{
    // has no attributes
}

ScXMLDataPilotMembersContext::~ScXMLDataPilotMembersContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDataPilotMembersContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_DATA_PILOT_MEMBER ):
            pContext = new ScXMLDataPilotMemberContext(GetScImport(), pAttribList, pDataPilotField);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

ScXMLDataPilotMemberContext::ScXMLDataPilotMemberContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDataPilotFieldContext* pTempDataPilotField) :
    ScXMLImportContext( rImport ),
    pDataPilotField(pTempDataPilotField),
    bDisplay( true ),
    bDisplayDetails( true ),
    bHasName( false )
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_NAME ):
                    sName = aIter.toString();
                    bHasName = true;
                break;
                case XML_ELEMENT( TABLE, XML_DISPLAY_NAME ):
                case XML_ELEMENT( TABLE_EXT, XML_DISPLAY_NAME ):
                    maDisplayName = aIter.toString();
                break;
                case XML_ELEMENT( TABLE, XML_DISPLAY ):
                    bDisplay = IsXMLToken(aIter, XML_TRUE);
                break;
                case XML_ELEMENT( TABLE, XML_SHOW_DETAILS ):
                    bDisplayDetails = IsXMLToken(aIter, XML_TRUE);
                break;
            }
        }
    }
}

ScXMLDataPilotMemberContext::~ScXMLDataPilotMemberContext()
{
}

void SAL_CALL ScXMLDataPilotMemberContext::endFastElement( sal_Int32 /*nElement*/ )
{
    if (bHasName)   // #i53407# don't check sName, empty name is allowed
    {
        std::unique_ptr<ScDPSaveMember> pMember(new ScDPSaveMember(sName));
        if (!maDisplayName.isEmpty())
            pMember->SetLayoutName(maDisplayName);
        pMember->SetIsVisible(bDisplay);
        pMember->SetShowDetails(bDisplayDetails);
        pDataPilotField->AddMember(std::move(pMember));
    }
}

ScXMLDataPilotGroupsContext::ScXMLDataPilotGroupsContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDataPilotFieldContext* pTempDataPilotField) :
    ScXMLImportContext( rImport ),
    pDataPilotField(pTempDataPilotField)
{
    OUString                sGroupSource;
    double                  fStart(0.0);
    double                  fEnd(0.0);
    double                  fStep(0.0);
    sal_Int32               nGroupPart(0);
    bool                    bDateValue(false);
    bool                    bAutoStart(true);
    bool                    bAutoEnd(true);

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken() & TOKEN_MASK)
            {
                case XML_SOURCE_FIELD_NAME :
                {
                    sGroupSource = aIter.toString();
                }
                break;
                case XML_DATE_START :
                {
                    bDateValue = true;
                    if (IsXMLToken(aIter, XML_AUTO))
                        bAutoStart = true;
                    else
                    {
                        GetScImport().GetMM100UnitConverter().convertDateTime(fStart, aIter.toString());
                        bAutoStart = false;
                    }
                }
                break;
                case XML_DATE_END :
                {
                    bDateValue = true;
                    if (IsXMLToken(aIter, XML_AUTO))
                        bAutoEnd = true;
                    else
                    {
                        GetScImport().GetMM100UnitConverter().convertDateTime(fEnd, aIter.toString());
                        bAutoEnd = false;
                    }
                }
                break;
                case XML_START :
                {
                    if (IsXMLToken(aIter, XML_AUTO))
                        bAutoStart = true;
                    else
                    {
                        fStart = aIter.toDouble();
                        bAutoStart = false;
                    }
                }
                break;
                case XML_END :
                {
                    if (IsXMLToken(aIter, XML_AUTO))
                        bAutoEnd = true;
                    else
                    {
                        fEnd = aIter.toDouble();
                        bAutoEnd = false;
                    }
                }
                break;
                case XML_STEP :
                {
                    fStep = aIter.toDouble();
                }
                break;
                case XML_GROUPED_BY :
                {
                    if (IsXMLToken(aIter, XML_SECONDS))
                        nGroupPart = css::sheet::DataPilotFieldGroupBy::SECONDS;
                    else if (IsXMLToken(aIter, XML_MINUTES))
                        nGroupPart = css::sheet::DataPilotFieldGroupBy::MINUTES;
                    else if (IsXMLToken(aIter, XML_HOURS))
                        nGroupPart = css::sheet::DataPilotFieldGroupBy::HOURS;
                    else if (IsXMLToken(aIter, XML_DAYS))
                        nGroupPart = css::sheet::DataPilotFieldGroupBy::DAYS;
                    else if (IsXMLToken(aIter, XML_MONTHS))
                        nGroupPart = css::sheet::DataPilotFieldGroupBy::MONTHS;
                    else if (IsXMLToken(aIter, XML_QUARTERS))
                        nGroupPart = css::sheet::DataPilotFieldGroupBy::QUARTERS;
                    else if (IsXMLToken(aIter, XML_YEARS))
                        nGroupPart = css::sheet::DataPilotFieldGroupBy::YEARS;
                }
                break;
            }
        }
    }
    pDataPilotField->SetGrouping(sGroupSource, fStart, fEnd, fStep, nGroupPart, bDateValue, bAutoStart, bAutoEnd);
}

ScXMLDataPilotGroupsContext::~ScXMLDataPilotGroupsContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDataPilotGroupsContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    if (nElement == XML_ELEMENT( TABLE, XML_DATA_PILOT_GROUP ))
    {
        pContext = new ScXMLDataPilotGroupContext(GetScImport(), pAttribList, pDataPilotField);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

ScXMLDataPilotGroupContext::ScXMLDataPilotGroupContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDataPilotFieldContext* pTempDataPilotField) :
    ScXMLImportContext( rImport ),
    pDataPilotField(pTempDataPilotField)
{
    if ( rAttrList.is() )
    {
        auto &aIter( rAttrList->find( XML_ELEMENT( TABLE, XML_NAME ) ) );
        if (aIter != rAttrList->end())
            sName = aIter.toString();
    }
}

ScXMLDataPilotGroupContext::~ScXMLDataPilotGroupContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDataPilotGroupContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    if (nElement == XML_ELEMENT( TABLE, XML_DATA_PILOT_MEMBER ) ||
        nElement == XML_ELEMENT( TABLE, XML_DATA_PILOT_GROUP_MEMBER ))
    {
        pContext = new ScXMLDataPilotGroupMemberContext(GetScImport(), pAttribList, this);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLDataPilotGroupContext::endFastElement( sal_Int32 /*nElement*/ )
{
    pDataPilotField->AddGroup(aMembers, sName);
}

ScXMLDataPilotGroupMemberContext::ScXMLDataPilotGroupMemberContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDataPilotGroupContext* pTempDataPilotGroup) :
    ScXMLImportContext( rImport ),
    pDataPilotGroup(pTempDataPilotGroup)
{
    if ( rAttrList.is() )
    {
        auto &aIter( rAttrList->find( XML_ELEMENT( TABLE, XML_NAME ) ) );
        if (aIter != rAttrList->end())
            sName = aIter.toString();
    }
}

ScXMLDataPilotGroupMemberContext::~ScXMLDataPilotGroupMemberContext()
{
}

void SAL_CALL ScXMLDataPilotGroupMemberContext::endFastElement( sal_Int32 /*nElement*/ )
{
    if (!sName.isEmpty())
        pDataPilotGroup->AddMember(sName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
