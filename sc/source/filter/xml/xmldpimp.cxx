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
#include "xmlsorti.hxx"
#include "document.hxx"
#include "docuno.hxx"
#include "dpshttab.hxx"
#include "dpsdbtab.hxx"
#include "attrib.hxx"
#include "XMLConverter.hxx"
#include "dpgroup.hxx"
#include "dpdimsave.hxx"
#include "rangeutl.hxx"
#include "dpoutputgeometry.hxx"

#include "pivotsource.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include <sax/tools/converter.hxx>

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

ScXMLDataPilotTablesContext::ScXMLDataPilotTablesContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    // has no Attributes
    rImport.LockSolarMutex();
}

ScXMLDataPilotTablesContext::~ScXMLDataPilotTablesContext()
{
    GetScImport().UnlockSolarMutex();
}

SvXMLImportContext *ScXMLDataPilotTablesContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDataPilotTablesElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATA_PILOT_TABLE :
        {
            pContext = new ScXMLDataPilotTableContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotTablesContext::EndElement()
{
}

ScXMLDataPilotTableContext::GrandTotalItem::GrandTotalItem() :
    mbVisible(true) {}

ScXMLDataPilotTableContext::ScXMLDataPilotTableContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDoc(GetScImport().GetDocument()),
    pDPObject(nullptr),
    pDPDimSaveData(nullptr),
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
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataPilotTableAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DATA_PILOT_TABLE_ATTR_NAME :
            {
                sDataPilotTableName = sValue;
            }
            break;
            case XML_TOK_DATA_PILOT_TABLE_ATTR_APPLICATION_DATA :
            {
                sApplicationData = sValue;
            }
            break;
            case XML_TOK_DATA_PILOT_TABLE_ATTR_GRAND_TOTAL :
            {
                if (IsXMLToken(sValue, XML_BOTH))
                {
                    maRowGrandTotal.mbVisible = true;
                    maColGrandTotal.mbVisible = true;
                }
                else if (IsXMLToken(sValue, XML_ROW))
                {
                    maRowGrandTotal.mbVisible = true;
                    maColGrandTotal.mbVisible = false;
                }
                else if (IsXMLToken(sValue, XML_COLUMN))
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
            case XML_TOK_DATA_PILOT_TABLE_ATTR_IGNORE_EMPTY_ROWS :
            {
                bIgnoreEmptyRows = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_DATA_PILOT_TABLE_ATTR_IDENTIFY_CATEGORIES :
            {
                bIdentifyCategories = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_DATA_PILOT_TABLE_ATTR_TARGET_RANGE_ADDRESS :
            {
                sal_Int32 nOffset(0);
                bTargetRangeAddress = ScRangeStringConverter::GetRangeFromString( aTargetRangeAddress, sValue, pDoc, ::formula::FormulaGrammar::CONV_OOO, nOffset );
            }
            break;
            case XML_TOK_DATA_PILOT_TABLE_ATTR_BUTTONS :
            {
                sButtons = sValue;
            }
            break;
            case XML_TOK_DATA_PILOT_TABLE_ATTR_SHOW_FILTER_BUTTON :
            {
                bShowFilter = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_DATA_PILOT_TABLE_ATTR_DRILL_DOWN :
            {
                bDrillDown = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_DATA_PILOT_TABLE_ATTR_HEADER_GRID_LAYOUT :
            {
                bHeaderGridLayout = IsXMLToken(sValue, XML_TRUE);
            }
            break;
        }
    }

    pDPObject = new ScDPObject(pDoc);
    pDPSave.reset(new ScDPSaveData());
}

ScXMLDataPilotTableContext::~ScXMLDataPilotTableContext()
{
    delete pDPDimSaveData;
}

SvXMLImportContext *ScXMLDataPilotTableContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDataPilotTableElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_SQL :
        {
            pContext = new ScXMLDPSourceSQLContext(GetScImport(), nPrefix, rLName, xAttrList, this);
            nSourceType = SQL;
        }
        break;
        case XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_TABLE :
        {
            pContext = new ScXMLDPSourceTableContext(GetScImport(), nPrefix, rLName, xAttrList, this);
            nSourceType = TABLE;
        }
        break;
        case XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_QUERY :
        {
            pContext = new ScXMLDPSourceQueryContext(GetScImport(), nPrefix, rLName, xAttrList, this);
            nSourceType = QUERY;
        }
        break;
        case XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_SERVICE :
        {
            pContext = new ScXMLSourceServiceContext(GetScImport(), nPrefix, rLName, xAttrList, this);
            nSourceType = SERVICE;
        }
        break;
        case XML_TOK_DATA_PILOT_TABLE_ELEM_GRAND_TOTAL:
        case XML_TOK_DATA_PILOT_TABLE_ELEM_GRAND_TOTAL_EXT:
        {
            pContext = new ScXMLDataPilotGrandTotalContext(GetScImport(), nPrefix, rLName, xAttrList, this);
        }
        break;
        case XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_CELL_RANGE :
        {
            pContext = new ScXMLSourceCellRangeContext(GetScImport(), nPrefix, rLName, xAttrList, this);
            nSourceType = CELLRANGE;
        }
        break;
        case XML_TOK_DATA_PILOT_TABLE_ELEM_DATA_PILOT_FIELD :
            pContext = new ScXMLDataPilotFieldContext(GetScImport(), nPrefix, rLName, xAttrList, this);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

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

ScDPOutputGeometry::FieldType toFieldType(sal_uInt16 nOrient)
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

void ScXMLDataPilotTableContext::SetButtons()
{
    ScDPOutputGeometry aGeometry(aTargetRangeAddress, bShowFilter);
    aGeometry.setColumnFieldCount(mnColFieldCount);
    aGeometry.setRowFieldCount(mnRowFieldCount);
    aGeometry.setPageFieldCount(mnPageFieldCount);
    aGeometry.setDataFieldCount(mnDataFieldCount);
    aGeometry.setDataLayoutType(toFieldType(mnDataLayoutType));

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

    if ( pDPObject )
        pDPObject->RefreshAfterLoad();
}

void ScXMLDataPilotTableContext::SetSelectedPage( const OUString& rDimName, const OUString& rSelected )
{
    maSelectedPages.insert(SelectedPagesType::value_type(rDimName, rSelected));
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
        pDPDimSaveData = new ScDPDimensionSaveData();
    pDPDimSaveData->AddNumGroupDimension(aNumGroupDim);
}

void ScXMLDataPilotTableContext::AddGroupDim(const ScDPSaveGroupDimension& aGroupDim)
{
    if (!pDPDimSaveData)
        pDPDimSaveData = new ScDPDimensionSaveData();
    pDPDimSaveData->AddGroupDimension(aGroupDim);
}

void ScXMLDataPilotTableContext::EndElement()
{
    if (!bTargetRangeAddress)
        return;

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
            rPivotSources.appendDBSource(pDPObject, aImportDesc);
        }
        break;
        case TABLE :
        {
            ScImportSourceDesc aImportDesc(pDoc);
            aImportDesc.aDBName = sDatabaseName;
            aImportDesc.aObject = sSourceObject;
            aImportDesc.nType = sheet::DataImportMode_TABLE;
            rPivotSources.appendDBSource(pDPObject, aImportDesc);
        }
        break;
        case QUERY :
        {
            ScImportSourceDesc aImportDesc(pDoc);
            aImportDesc.aDBName = sDatabaseName;
            aImportDesc.aObject = sSourceObject;
            aImportDesc.nType = sheet::DataImportMode_QUERY;
            rPivotSources.appendDBSource(pDPObject, aImportDesc);
        }
        break;
        case SERVICE :
        {
            ScDPServiceDesc aServiceDesc(sServiceName, sServiceSourceName, sServiceSourceObject,
                                sServiceUsername, sServicePassword);
            rPivotSources.appendServiceSource(pDPObject, aServiceDesc);
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
                rPivotSources.appendSheetSource(pDPObject, aSheetDesc);
            }
        }
        break;
    }

    rPivotSources.appendSelectedPages(pDPObject, maSelectedPages);

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
        pDPSave->SetDimensionData(pDPDimSaveData);
    pDPObject->SetSaveData(*pDPSave);

    ScDPCollection* pDPCollection = pDoc->GetDPCollection();

    // #i94570# Names have to be unique, or the tables can't be accessed by API.
    if ( pDPCollection->GetByName(pDPObject->GetName()) )
        pDPObject->SetName( OUString() );     // ignore the invalid name, create a new name in AfterXMLLoading

    if (!pDPCollection->InsertNewTable(pDPObject))
    {
        OSL_FAIL("cannot insert DPObject");
        DELETEZ( pDPObject );
    }

    SetButtons();
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
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLDataPilotTableContext* pTempDataPilotTable) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDataPilotTable(pTempDataPilotTable)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeSourceSQLAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_SQL_ATTR_DATABASE_NAME :
            {
                pDataPilotTable->SetDatabaseName(sValue);
            }
            break;
            case XML_TOK_SOURCE_SQL_ATTR_SQL_STATEMENT :
            {
                pDataPilotTable->SetSourceObject(sValue);
            }
            break;
            case XML_TOK_SOURCE_SQL_ATTR_PARSE_SQL_STATEMENT :
            {
                pDataPilotTable->SetNative(!IsXMLToken(sValue, XML_TRUE));
            }
            break;
        }
    }
}

ScXMLDPSourceSQLContext::~ScXMLDPSourceSQLContext()
{
}

SvXMLImportContext *ScXMLDPSourceSQLContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    SvXMLImportContext *pContext = nullptr;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDPSourceSQLContext::EndElement()
{
}

ScXMLDPSourceTableContext::ScXMLDPSourceTableContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLDataPilotTableContext* pTempDataPilotTable) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDataPilotTable(pTempDataPilotTable)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeSourceTableAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_TABLE_ATTR_DATABASE_NAME :
            {
                pDataPilotTable->SetDatabaseName(sValue);
            }
            break;
            case XML_TOK_SOURCE_TABLE_ATTR_TABLE_NAME :
            {
                pDataPilotTable->SetSourceObject(sValue);
            }
            break;
        }
    }
}

ScXMLDPSourceTableContext::~ScXMLDPSourceTableContext()
{
}

SvXMLImportContext *ScXMLDPSourceTableContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    SvXMLImportContext *pContext = nullptr;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDPSourceTableContext::EndElement()
{
}

ScXMLDPSourceQueryContext::ScXMLDPSourceQueryContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLDataPilotTableContext* pTempDataPilotTable) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDataPilotTable(pTempDataPilotTable)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeSourceQueryAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_QUERY_ATTR_DATABASE_NAME :
            {
                pDataPilotTable->SetDatabaseName(sValue);
            }
            break;
            case XML_TOK_SOURCE_QUERY_ATTR_QUERY_NAME :
            {
                pDataPilotTable->SetSourceObject(sValue);
            }
            break;
        }
    }
}

ScXMLDPSourceQueryContext::~ScXMLDPSourceQueryContext()
{
}

SvXMLImportContext *ScXMLDPSourceQueryContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    SvXMLImportContext *pContext = nullptr;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDPSourceQueryContext::EndElement()
{
}

ScXMLSourceServiceContext::ScXMLSourceServiceContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLDataPilotTableContext* pTempDataPilotTable) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDataPilotTable(pTempDataPilotTable)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataPilotTableSourceServiceAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_SERVICE_ATTR_NAME :
            {
                pDataPilotTable->SetServiceName(sValue);
            }
            break;
            case XML_TOK_SOURCE_SERVICE_ATTR_SOURCE_NAME :
            {
                pDataPilotTable->SetServiceSourceName(sValue);
            }
            break;
            case XML_TOK_SOURCE_SERVICE_ATTR_OBJECT_NAME :
            {
                pDataPilotTable->SetServiceSourceObject(sValue);
            }
            break;
            case XML_TOK_SOURCE_SERVICE_ATTR_USER_NAME :
            {
                pDataPilotTable->SetServiceUsername(sValue);
            }
            break;
            case XML_TOK_SOURCE_SERVICE_ATTR_PASSWORD :
            {
                pDataPilotTable->SetServicePassword(sValue);
            }
            break;
        }
    }
}

ScXMLSourceServiceContext::~ScXMLSourceServiceContext()
{
}

SvXMLImportContext *ScXMLSourceServiceContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    SvXMLImportContext *pContext = nullptr;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSourceServiceContext::EndElement()
{
}

ScXMLImport& ScXMLDataPilotGrandTotalContext::GetScImport()
{
    return static_cast<ScXMLImport&>(GetImport());
}

ScXMLDataPilotGrandTotalContext::ScXMLDataPilotGrandTotalContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, const Reference<XAttributeList>& xAttrList,
    ScXMLDataPilotTableContext* pTableContext ) :
    SvXMLImportContext( rImport, nPrefix, rLName ),
    mpTableContext(pTableContext),
    meOrientation(NONE),
    mbVisible(false)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataPilotGrandTotalAttrTokenMap();
    for (sal_Int16 i = 0; i < nAttrCount; ++i)
    {
        const OUString& rAttrName  = xAttrList->getNameByIndex(i);
        const OUString& rAttrValue = xAttrList->getValueByIndex(i);

        OUString aLocalName;
        sal_uInt16 nLocalPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(rAttrName, &aLocalName);
        switch (rAttrTokenMap.Get(nLocalPrefix, aLocalName))
        {
            case XML_TOK_DATA_PILOT_GRAND_TOTAL_ATTR_DISPLAY:
                mbVisible = IsXMLToken(rAttrValue, XML_TRUE);
            break;
            case XML_TOK_DATA_PILOT_GRAND_TOTAL_ATTR_ORIENTATION:
                if (IsXMLToken(rAttrValue, XML_BOTH))
                    meOrientation = BOTH;
                else if (IsXMLToken(rAttrValue, XML_ROW))
                    meOrientation = ROW;
                else if (IsXMLToken(rAttrValue, XML_COLUMN))
                    meOrientation = COLUMN;
            break;
            case XML_TOK_DATA_PILOT_GRAND_TOTAL_ATTR_DISPLAY_NAME:
            case XML_TOK_DATA_PILOT_GRAND_TOTAL_ATTR_DISPLAY_NAME_EXT:
                maDisplayName = rAttrValue;
            break;
            default:
                break;
        }
    }
}

ScXMLDataPilotGrandTotalContext::~ScXMLDataPilotGrandTotalContext()
{
}

SvXMLImportContext* ScXMLDataPilotGrandTotalContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const Reference<XAttributeList>& /*xAttrList*/ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

void ScXMLDataPilotGrandTotalContext::EndElement()
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
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLDataPilotTableContext* pTempDataPilotTable) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDataPilotTable(pTempDataPilotTable)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataPilotTableSourceCellRangeAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_CELL_RANGE_ATTR_CELL_RANGE_ADDRESS :
            {
                ScRange aSourceRangeAddress;
                sal_Int32 nOffset(0);
                if (ScRangeStringConverter::GetRangeFromString( aSourceRangeAddress, sValue, GetScImport().GetDocument(), ::formula::FormulaGrammar::CONV_OOO, nOffset ))
                    pDataPilotTable->SetSourceCellRangeAddress(aSourceRangeAddress);
            }
            break;
            case XML_TOK_SOURCE_CELL_RANGE_ATTR_NAME:
                pDataPilotTable->SetSourceRangeName(sValue);
            break;
        }
    }
}

ScXMLSourceCellRangeContext::~ScXMLSourceCellRangeContext()
{
}

SvXMLImportContext *ScXMLSourceCellRangeContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<
                                          css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDataPilotTableSourceCellRangeElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_SOURCE_CELL_RANGE_ELEM_FILTER :
            pContext = new ScXMLDPFilterContext(GetScImport(), nPrefix, rLName, xAttrList, pDataPilotTable);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSourceCellRangeContext::EndElement()
{
}

ScXMLDataPilotFieldContext::ScXMLDataPilotFieldContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLDataPilotTableContext* pTempDataPilotTable) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDataPilotTable(pTempDataPilotTable),
    pDim(nullptr),
    fStart(0.0),
    fEnd(0.0),
    fStep(0.0),
    nUsedHierarchy(1),
    nGroupPart(0),
    nFunction(0),
    nOrientation(0),
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
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataPilotFieldAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DATA_PILOT_FIELD_ATTR_SOURCE_FIELD_NAME :
            {
                sName = sValue;
                bHasName = true;
            }
            break;
            case XML_TOK_DATA_PILOT_FIELD_ATTR_DISPLAY_NAME:
            case XML_TOK_DATA_PILOT_FIELD_ATTR_DISPLAY_NAME_EXT:
            {
                aDisplayName = sValue;
            }
            break;
            case XML_TOK_DATA_PILOT_FIELD_ATTR_IS_DATA_LAYOUT_FIELD :
            {
                bDataLayout = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_DATA_PILOT_FIELD_ATTR_FUNCTION :
            {
                nFunction = (sal_Int16) ScXMLConverter::GetFunctionFromString( sValue );
            }
            break;
            case XML_TOK_DATA_PILOT_FIELD_ATTR_ORIENTATION :
            {
                nOrientation = (sal_Int16) ScXMLConverter::GetOrientationFromString( sValue );
            }
            break;
            case XML_TOK_DATA_PILOT_FIELD_ATTR_SELECTED_PAGE :
            {
                sSelectedPage = sValue;
                bSelectedPage = true;
            }
            break;
            case XML_TOK_DATA_PILOT_FIELD_ATTR_IGNORE_SELECTED_PAGE:
            {
                bIgnoreSelectedPage = true;
            }
            break;
            case XML_TOK_DATA_PILOT_FIELD_ATTR_USED_HIERARCHY :
            {
                nUsedHierarchy = sValue.toInt32();
            }
            break;
        }
    }

    // use the new extension elements
    if (bIgnoreSelectedPage)
        bSelectedPage = false;

    if (bHasName)
    {
        pDim = new ScDPSaveDimension(sName, bDataLayout);
        if (!aDisplayName.isEmpty())
            pDim->SetLayoutName(aDisplayName);
    }
}

ScXMLDataPilotFieldContext::~ScXMLDataPilotFieldContext()
{
}

SvXMLImportContext *ScXMLDataPilotFieldContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDataPilotFieldElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_LEVEL :
            pContext = new ScXMLDataPilotLevelContext(GetScImport(), nPrefix, rLName, xAttrList, this);
        break;
        case XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_REFERENCE :
            pContext = new ScXMLDataPilotFieldReferenceContext(GetScImport(), nPrefix, rLName, xAttrList, this);
        break;
        case XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_GROUPS :
            pContext = new ScXMLDataPilotGroupsContext(GetScImport(), nPrefix, rLName, xAttrList, this);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotFieldContext::AddMember(ScDPSaveMember* pMember)
{
    if (pDim)
    {
        pDim->AddMember(pMember);
        if (!pMember->GetIsVisible())
            // This member is hidden.
            mbHasHiddenMember = true;
    }
    else
        delete pMember;
}

void ScXMLDataPilotFieldContext::SetSubTotalName(const OUString& rName)
{
    if (pDim)
        pDim->SetSubtotalName(rName);
}

void ScXMLDataPilotFieldContext::AddGroup(const ::std::vector<OUString>& rMembers, const OUString& rName)
{
    ScXMLDataPilotGroup aGroup;
    aGroup.aMembers = rMembers;
    aGroup.aName = rName;
    aGroups.push_back(aGroup);
}

void ScXMLDataPilotFieldContext::EndElement()
{
    if (pDim)
    {
        pDim->SetUsedHierarchy(nUsedHierarchy);
        pDim->SetFunction(nFunction);
        pDim->SetOrientation(nOrientation);
        if (bSelectedPage)
        {
            pDataPilotTable->SetSelectedPage(pDim->GetName(), sSelectedPage);
        }
        pDataPilotTable->AddDimension(pDim);
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
                    ::std::vector<ScXMLDataPilotGroup>::const_iterator aItr(aGroups.begin());
                    ::std::vector<ScXMLDataPilotGroup>::const_iterator aEndItr(aGroups.end());
                    while (aItr != aEndItr)
                    {
                        ScDPSaveGroupItem aItem(aItr->aName);
                        ::std::vector<OUString>::const_iterator aMembersItr(aItr->aMembers.begin());
                        ::std::vector<OUString>::const_iterator aMembersEndItr(aItr->aMembers.end());
                        while (aMembersItr != aMembersEndItr)
                        {
                            aItem.AddElement(*aMembersItr);
                            ++aMembersItr;
                        }
                        ++aItr;
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

ScXMLDataPilotFieldReferenceContext::ScXMLDataPilotFieldReferenceContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sheet::DataPilotFieldReference aReference;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        OUString sValue(xAttrList->getValueByIndex( i ));

        if ( nPrefix == XML_NAMESPACE_TABLE )
        {
            if (IsXMLToken(aLocalName, XML_TYPE))
            {
                if (IsXMLToken(sValue, XML_NONE))
                    aReference.ReferenceType = sheet::DataPilotFieldReferenceType::NONE;
                else if (IsXMLToken(sValue, XML_MEMBER_DIFFERENCE))
                    aReference.ReferenceType = sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE;
                else if (IsXMLToken(sValue, XML_MEMBER_PERCENTAGE))
                    aReference.ReferenceType = sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE;
                else if (IsXMLToken(sValue, XML_MEMBER_PERCENTAGE_DIFFERENCE))
                    aReference.ReferenceType = sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE;
                else if (IsXMLToken(sValue, XML_RUNNING_TOTAL))
                    aReference.ReferenceType = sheet::DataPilotFieldReferenceType::RUNNING_TOTAL;
                else if (IsXMLToken(sValue, XML_ROW_PERCENTAGE))
                    aReference.ReferenceType = sheet::DataPilotFieldReferenceType::ROW_PERCENTAGE;
                else if (IsXMLToken(sValue, XML_COLUMN_PERCENTAGE))
                    aReference.ReferenceType = sheet::DataPilotFieldReferenceType::COLUMN_PERCENTAGE;
                else if (IsXMLToken(sValue, XML_TOTAL_PERCENTAGE))
                    aReference.ReferenceType = sheet::DataPilotFieldReferenceType::TOTAL_PERCENTAGE;
                else if (IsXMLToken(sValue, XML_INDEX))
                    aReference.ReferenceType = sheet::DataPilotFieldReferenceType::INDEX;
            }
            else if (IsXMLToken(aLocalName, XML_FIELD_NAME))
            {
                aReference.ReferenceField = sValue;
            }
            else if (IsXMLToken(aLocalName, XML_MEMBER_TYPE))
            {
                if (IsXMLToken(sValue, XML_NAMED))
                    aReference.ReferenceItemType = sheet::DataPilotFieldReferenceItemType::NAMED;
                else if (IsXMLToken(sValue, XML_PREVIOUS))
                    aReference.ReferenceItemType = sheet::DataPilotFieldReferenceItemType::PREVIOUS;
                else if (IsXMLToken(sValue, XML_NEXT))
                    aReference.ReferenceItemType = sheet::DataPilotFieldReferenceItemType::NEXT;
            }
            else if (IsXMLToken(aLocalName, XML_MEMBER_NAME))
            {
                aReference.ReferenceItemName = sValue;
            }
        }
    }
    pDataPilotField->SetFieldReference(aReference);
}

ScXMLDataPilotFieldReferenceContext::~ScXMLDataPilotFieldReferenceContext()
{
}

ScXMLDataPilotLevelContext::ScXMLDataPilotLevelContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLDataPilotFieldContext* pTempDataPilotField) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDataPilotField(pTempDataPilotField)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataPilotLevelAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DATA_PILOT_LEVEL_ATTR_SHOW_EMPTY :
            {
                pDataPilotField->SetShowEmpty(IsXMLToken(sValue, XML_TRUE));
            }
            break;
            case XML_TOK_DATA_PILOT_LEVEL_ATTR_REPEAT_ITEM_LABELS :
            {
                pDataPilotField->SetRepeatItemLabels(IsXMLToken(sValue, XML_TRUE));
            }
            break;
        }
    }
}

ScXMLDataPilotLevelContext::~ScXMLDataPilotLevelContext()
{
}

SvXMLImportContext *ScXMLDataPilotLevelContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDataPilotLevelElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATA_PILOT_LEVEL_ELEM_DATA_PILOT_SUBTOTALS :
            pContext = new ScXMLDataPilotSubTotalsContext(GetScImport(), nPrefix, rLName, xAttrList, pDataPilotField);
        break;
        case XML_TOK_DATA_PILOT_LEVEL_ELEM_DATA_PILOT_MEMBERS :
            pContext = new ScXMLDataPilotMembersContext(GetScImport(), nPrefix, rLName, xAttrList, pDataPilotField);
        break;
        case XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_DISPLAY_INFO :
            pContext = new ScXMLDataPilotDisplayInfoContext(GetScImport(), nPrefix, rLName, xAttrList, pDataPilotField);
        break;
        case XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_SORT_INFO :
            pContext = new ScXMLDataPilotSortInfoContext(GetScImport(), nPrefix, rLName, xAttrList, pDataPilotField);
        break;
        case XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_LAYOUT_INFO :
            pContext = new ScXMLDataPilotLayoutInfoContext(GetScImport(), nPrefix, rLName, xAttrList, pDataPilotField);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotLevelContext::EndElement()
{
}

ScXMLDataPilotDisplayInfoContext::ScXMLDataPilotDisplayInfoContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sheet::DataPilotFieldAutoShowInfo aInfo;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        OUString sValue(xAttrList->getValueByIndex( i ));

        if ( nPrefix == XML_NAMESPACE_TABLE )
        {
            if (IsXMLToken(aLocalName, XML_ENABLED))
            {
                if (IsXMLToken(sValue, XML_TRUE))
                    aInfo.IsEnabled = true;
                else
                    aInfo.IsEnabled = false;
            }
            else if (IsXMLToken(aLocalName, XML_DISPLAY_MEMBER_MODE))
            {
                if (IsXMLToken(sValue, XML_FROM_TOP))
                    aInfo.ShowItemsMode = sheet::DataPilotFieldShowItemsMode::FROM_TOP;
                else if (IsXMLToken(sValue, XML_FROM_BOTTOM))
                    aInfo.ShowItemsMode = sheet::DataPilotFieldShowItemsMode::FROM_BOTTOM;
            }
            else if (IsXMLToken(aLocalName, XML_MEMBER_COUNT))
            {
                aInfo.ItemCount = sValue.toInt32();
            }
            else if (IsXMLToken(aLocalName, XML_DATA_FIELD))
            {
                aInfo.DataField = sValue;
            }
        }
    }
    pDataPilotField->SetAutoShowInfo(aInfo);
}

ScXMLDataPilotDisplayInfoContext::~ScXMLDataPilotDisplayInfoContext()
{
}

ScXMLDataPilotSortInfoContext::ScXMLDataPilotSortInfoContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sheet::DataPilotFieldSortInfo aInfo;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        OUString sValue(xAttrList->getValueByIndex( i ));

        if ( nPrefix == XML_NAMESPACE_TABLE )
        {
            if (IsXMLToken(aLocalName, XML_ORDER))
            {
                if (IsXMLToken(sValue, XML_ASCENDING))
                    aInfo.IsAscending = true;
                else if (IsXMLToken(sValue, XML_DESCENDING))
                    aInfo.IsAscending = false;
            }
            else if (IsXMLToken(aLocalName, XML_SORT_MODE))
            {
                if (IsXMLToken(sValue, XML_NONE))
                    aInfo.Mode = sheet::DataPilotFieldSortMode::NONE;
                else if (IsXMLToken(sValue, XML_MANUAL))
                    aInfo.Mode = sheet::DataPilotFieldSortMode::MANUAL;
                else if (IsXMLToken(sValue, XML_NAME))
                    aInfo.Mode = sheet::DataPilotFieldSortMode::NAME;
                else if (IsXMLToken(sValue, XML_DATA))
                    aInfo.Mode = sheet::DataPilotFieldSortMode::DATA;
            }
            else if (IsXMLToken(aLocalName, XML_DATA_FIELD))
                aInfo.Field = sValue;
        }
    }
    pDataPilotField->SetSortInfo(aInfo);
}

ScXMLDataPilotSortInfoContext::~ScXMLDataPilotSortInfoContext()
{
}

ScXMLDataPilotLayoutInfoContext::ScXMLDataPilotLayoutInfoContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sheet::DataPilotFieldLayoutInfo aInfo;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        OUString sValue(xAttrList->getValueByIndex( i ));

        if ( nPrefix == XML_NAMESPACE_TABLE )
        {
            if (IsXMLToken(aLocalName, XML_ADD_EMPTY_LINES))
            {
                if (IsXMLToken(sValue, XML_TRUE))
                    aInfo.AddEmptyLines = true;
                else
                    aInfo.AddEmptyLines = false;
            }
            else if (IsXMLToken(aLocalName, XML_LAYOUT_MODE))
            {
                if (IsXMLToken(sValue, XML_TABULAR_LAYOUT))
                    aInfo.LayoutMode = sheet::DataPilotFieldLayoutMode::TABULAR_LAYOUT;
                else if (IsXMLToken(sValue, XML_OUTLINE_SUBTOTALS_TOP))
                    aInfo.LayoutMode = sheet::DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_TOP;
                else if (IsXMLToken(sValue, XML_OUTLINE_SUBTOTALS_BOTTOM))
                    aInfo.LayoutMode = sheet::DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_BOTTOM;
            }
        }
    }
    pDataPilotField->SetLayoutInfo(aInfo);}

ScXMLDataPilotLayoutInfoContext::~ScXMLDataPilotLayoutInfoContext()
{
}

ScXMLDataPilotSubTotalsContext::ScXMLDataPilotSubTotalsContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */,
                                      ScXMLDataPilotFieldContext* pTempDataPilotField) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDataPilotField(pTempDataPilotField),
    nFunctionCount(0),
    pFunctions(nullptr)
{

    // has no attributes
}

ScXMLDataPilotSubTotalsContext::~ScXMLDataPilotSubTotalsContext()
{
    delete[] pFunctions;
}

SvXMLImportContext *ScXMLDataPilotSubTotalsContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDataPilotSubTotalsElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATA_PILOT_SUBTOTALS_ELEM_DATA_PILOT_SUBTOTAL :
            pContext = new ScXMLDataPilotSubTotalContext(GetScImport(), nPrefix, rLName, xAttrList, this);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotSubTotalsContext::EndElement()
{
    pDataPilotField->SetSubTotals(pFunctions, nFunctionCount);
    if (!maDisplayName.isEmpty())
        pDataPilotField->SetSubTotalName(maDisplayName);
}

void ScXMLDataPilotSubTotalsContext::AddFunction(sal_Int16 nFunction)
{
    if (nFunctionCount)
    {
        ++nFunctionCount;
        sal_uInt16* pTemp = new sal_uInt16[nFunctionCount];
        for (sal_Int16 i = 0; i < nFunctionCount - 1; ++i)
            pTemp[i] = pFunctions[i];
        pTemp[nFunctionCount - 1] = nFunction;
        delete[] pFunctions;
        pFunctions = pTemp;
    }
    else
    {
        nFunctionCount = 1;
        pFunctions = new sal_uInt16[nFunctionCount];
        pFunctions[0] = nFunction;
    }
}

void ScXMLDataPilotSubTotalsContext::SetDisplayName(const OUString& rName)
{
    maDisplayName = rName;
}

ScXMLDataPilotSubTotalContext::ScXMLDataPilotSubTotalContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLDataPilotSubTotalsContext* pTempDataPilotSubTotals) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDataPilotSubTotals(pTempDataPilotSubTotals)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataPilotSubTotalAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DATA_PILOT_SUBTOTAL_ATTR_FUNCTION :
            {
                pDataPilotSubTotals->AddFunction( sal::static_int_cast<sal_Int16>(
                                ScXMLConverter::GetFunctionFromString( sValue ) ) );
            }
            break;
            case XML_TOK_DATA_PILOT_SUBTOTAL_ATTR_DISPLAY_NAME:
            case XML_TOK_DATA_PILOT_SUBTOTAL_ATTR_DISPLAY_NAME_EXT:
                pDataPilotSubTotals->SetDisplayName(sValue);
            break;
        }
    }
}

ScXMLDataPilotSubTotalContext::~ScXMLDataPilotSubTotalContext()
{
}

SvXMLImportContext *ScXMLDataPilotSubTotalContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    SvXMLImportContext *pContext = nullptr;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotSubTotalContext::EndElement()
{
}

ScXMLDataPilotMembersContext::ScXMLDataPilotMembersContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */,
                                      ScXMLDataPilotFieldContext* pTempDataPilotField) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDataPilotField(pTempDataPilotField)
{
    // has no attributes
}

ScXMLDataPilotMembersContext::~ScXMLDataPilotMembersContext()
{
}

SvXMLImportContext *ScXMLDataPilotMembersContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<
                                          css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDataPilotMembersElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATA_PILOT_MEMBERS_ELEM_DATA_PILOT_MEMBER :
            pContext = new ScXMLDataPilotMemberContext(GetScImport(), nPrefix, rLName, xAttrList, pDataPilotField);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotMembersContext::EndElement()
{
}

ScXMLDataPilotMemberContext::ScXMLDataPilotMemberContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLDataPilotFieldContext* pTempDataPilotField) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDataPilotField(pTempDataPilotField),
    bDisplay( true ),
    bDisplayDetails( true ),
    bHasName( false )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataPilotMemberAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DATA_PILOT_MEMBER_ATTR_NAME :
            {
                sName = sValue;
                bHasName = true;
            }
            break;
            case XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY_NAME:
            case XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY_NAME_EXT:
            {
                maDisplayName = sValue;
            }
            break;
            case XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY :
            {
                bDisplay = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_DATA_PILOT_MEMBER_ATTR_SHOW_DETAILS :
            {
                bDisplayDetails = IsXMLToken(sValue, XML_TRUE);
            }
            break;
        }
    }
}

ScXMLDataPilotMemberContext::~ScXMLDataPilotMemberContext()
{
}

SvXMLImportContext *ScXMLDataPilotMemberContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    SvXMLImportContext *pContext = nullptr;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotMemberContext::EndElement()
{
    if (bHasName)   // #i53407# don't check sName, empty name is allowed
    {
        ScDPSaveMember* pMember = new ScDPSaveMember(sName);
        if (!maDisplayName.isEmpty())
            pMember->SetLayoutName(maDisplayName);
        pMember->SetIsVisible(bDisplay);
        pMember->SetShowDetails(bDisplayDetails);
        pDataPilotField->AddMember(pMember);
    }
}

ScXMLDataPilotGroupsContext::ScXMLDataPilotGroupsContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLDataPilotFieldContext* pTempDataPilotField) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
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

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );

        (void)nPrefix;  //! compare below!

        if (IsXMLToken(aLocalName, XML_SOURCE_FIELD_NAME))
                sGroupSource = sValue;
        else if (IsXMLToken(aLocalName, XML_DATE_START))
        {
            bDateValue = true;
            if (IsXMLToken(sValue, XML_AUTO))
                bAutoStart = true;
            else
            {
                GetScImport().GetMM100UnitConverter().convertDateTime(fStart, sValue);
                bAutoStart = false;
            }
        }
        else if (IsXMLToken(aLocalName, XML_DATE_END))
        {
            bDateValue = true;
            if (IsXMLToken(sValue, XML_AUTO))
                bAutoEnd = true;
            else
            {
                GetScImport().GetMM100UnitConverter().convertDateTime(fEnd, sValue);
                bAutoEnd = false;
            }
        }
        else if (IsXMLToken(aLocalName, XML_START))
        {
            if (IsXMLToken(sValue, XML_AUTO))
                bAutoStart = true;
            else
            {
                ::sax::Converter::convertDouble(fStart, sValue);
                bAutoStart = false;
            }
        }
        else if (IsXMLToken(aLocalName, XML_END))
        {
            if (IsXMLToken(sValue, XML_AUTO))
                bAutoEnd = true;
            else
            {
                ::sax::Converter::convertDouble(fEnd, sValue);
                bAutoEnd = false;
            }
        }
        else if (IsXMLToken(aLocalName, XML_STEP))
        {
                ::sax::Converter::convertDouble(fStep, sValue);
        }
        else if (IsXMLToken(aLocalName, XML_GROUPED_BY))
        {
            if (IsXMLToken(sValue, XML_SECONDS))
                nGroupPart = css::sheet::DataPilotFieldGroupBy::SECONDS;
            else if (IsXMLToken(sValue, XML_MINUTES))
                nGroupPart = css::sheet::DataPilotFieldGroupBy::MINUTES;
            else if (IsXMLToken(sValue, XML_HOURS))
                nGroupPart = css::sheet::DataPilotFieldGroupBy::HOURS;
            else if (IsXMLToken(sValue, XML_DAYS))
                nGroupPart = css::sheet::DataPilotFieldGroupBy::DAYS;
            else if (IsXMLToken(sValue, XML_MONTHS))
                nGroupPart = css::sheet::DataPilotFieldGroupBy::MONTHS;
            else if (IsXMLToken(sValue, XML_QUARTERS))
                nGroupPart = css::sheet::DataPilotFieldGroupBy::QUARTERS;
            else if (IsXMLToken(sValue, XML_YEARS))
                nGroupPart = css::sheet::DataPilotFieldGroupBy::YEARS;
        }
    }
    pDataPilotField->SetGrouping(sGroupSource, fStart, fEnd, fStep, nGroupPart, bDateValue, bAutoStart, bAutoEnd);
}

ScXMLDataPilotGroupsContext::~ScXMLDataPilotGroupsContext()
{
}

SvXMLImportContext *ScXMLDataPilotGroupsContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLName, XML_DATA_PILOT_GROUP))
            pContext = new ScXMLDataPilotGroupContext(GetScImport(), nPrefix, rLName,  xAttrList, pDataPilotField);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotGroupsContext::EndElement()
{
}

ScXMLDataPilotGroupContext::ScXMLDataPilotGroupContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLDataPilotFieldContext* pTempDataPilotField) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDataPilotField(pTempDataPilotField)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_NAME))
                sName = sValue;
        }
    }
}

ScXMLDataPilotGroupContext::~ScXMLDataPilotGroupContext()
{
}

SvXMLImportContext *ScXMLDataPilotGroupContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLName, XML_DATA_PILOT_MEMBER) || IsXMLToken(rLName, XML_DATA_PILOT_GROUP_MEMBER))
            pContext = new ScXMLDataPilotGroupMemberContext(GetScImport(), nPrefix, rLName, xAttrList, this);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotGroupContext::EndElement()
{
    pDataPilotField->AddGroup(aMembers, sName);
}

ScXMLDataPilotGroupMemberContext::ScXMLDataPilotGroupMemberContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLDataPilotGroupContext* pTempDataPilotGroup) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDataPilotGroup(pTempDataPilotGroup)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_NAME))
                sName = sValue;
        }
    }
}

ScXMLDataPilotGroupMemberContext::~ScXMLDataPilotGroupMemberContext()
{
}

SvXMLImportContext *ScXMLDataPilotGroupMemberContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    SvXMLImportContext *pContext = nullptr;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotGroupMemberContext::EndElement()
{
    if (!sName.isEmpty())
        pDataPilotGroup->AddMember(sName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
