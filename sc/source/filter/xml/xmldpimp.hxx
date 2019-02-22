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
#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLDPIMP_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLDPIMP_HXX

#include <memory>
#include <xmloff/xmltoken.hxx>

#include <dpsave.hxx>
#include <queryparam.hxx>
#include "importcontext.hxx"

#include <unordered_map>

namespace com { namespace sun { namespace star { namespace sheet { struct DataPilotFieldAutoShowInfo; } } } }
namespace com { namespace sun { namespace star { namespace sheet { struct DataPilotFieldLayoutInfo; } } } }
namespace com { namespace sun { namespace star { namespace sheet { struct DataPilotFieldReference; } } } }
namespace com { namespace sun { namespace star { namespace sheet { struct DataPilotFieldSortInfo; } } } }
namespace sax_fastparser { class FastAttributeList; }

class ScDPSaveNumGroupDimension;
class ScDPSaveGroupDimension;
class ScDPObject;

enum ScMySourceType
{
    SQL,
    TABLE,
    QUERY,
    SERVICE,
    CELLRANGE
};

class ScXMLDataPilotTablesContext : public ScXMLImportContext
{
public:

    ScXMLDataPilotTablesContext( ScXMLImport& rImport);

    virtual ~ScXMLDataPilotTablesContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

class ScXMLDataPilotTableContext : public ScXMLImportContext
{
    typedef std::unordered_map<OUString, OUString> SelectedPagesType;

    struct GrandTotalItem
    {
        OUString maDisplayName;
        bool            mbVisible;
        GrandTotalItem();
    };
    ScDocument*     pDoc;
    std::unique_ptr<ScDPSaveData> pDPSave;
    std::unique_ptr<ScDPDimensionSaveData> pDPDimSaveData;
    GrandTotalItem  maRowGrandTotal;
    GrandTotalItem  maColGrandTotal;
    OUString   sDataPilotTableName;
    OUString   sApplicationData;
    OUString   sDatabaseName;
    OUString   sSourceObject;
    OUString   sServiceName;
    OUString   sServiceSourceName;
    OUString   sServiceSourceObject;
    OUString   sServiceUsername;
    OUString   sServicePassword;
    OUString   sButtons;
    OUString   sSourceRangeName;
    ScRange         aSourceCellRangeAddress;
    ScRange         aTargetRangeAddress;
    ScQueryParam    aSourceQueryParam;
    ScMySourceType  nSourceType;
    sal_uInt32      mnRowFieldCount;
    sal_uInt32      mnColFieldCount;
    sal_uInt32      mnPageFieldCount;
    sal_uInt32      mnDataFieldCount;
    css::sheet::DataPilotFieldOrientation
                    mnDataLayoutType;
    bool            bIsNative:1;
    bool            bIgnoreEmptyRows:1;
    bool            bIdentifyCategories:1;
    bool            bTargetRangeAddress:1;
    bool            bSourceCellRange:1;
    bool            bShowFilter:1;
    bool            bDrillDown:1;
    bool            bHeaderGridLayout:1;

    SelectedPagesType maSelectedPages;

public:

    ScXMLDataPilotTableContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList );

    virtual ~ScXMLDataPilotTableContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    void SetGrandTotal(::xmloff::token::XMLTokenEnum eOrientation, bool bVisible, const OUString& rDisplayName);
    void SetDatabaseName(const OUString& sValue) { sDatabaseName = sValue; }
    void SetSourceObject(const OUString& sValue) { sSourceObject = sValue; }
    void SetNative(bool bValue) { bIsNative = bValue; }
    void SetServiceName(const OUString& sValue) { sServiceName = sValue; }
    void SetServiceSourceName(const OUString& sValue) { sServiceSourceName = sValue; }
    void SetServiceSourceObject(const OUString& sValue) { sServiceSourceObject = sValue; }
    void SetServiceUsername(const OUString& sValue) { sServiceUsername = sValue; }
    void SetServicePassword(const OUString& sValue) { sServicePassword = sValue; }
    void SetSourceRangeName(const OUString& sValue) { sSourceRangeName = sValue; bSourceCellRange = true; }
    void SetSourceCellRangeAddress(const ScRange& aValue) { aSourceCellRangeAddress = aValue; bSourceCellRange = true; }
    void SetSourceQueryParam(const ScQueryParam& aValue) { aSourceQueryParam = aValue; }
    void AddDimension(ScDPSaveDimension* pDim);
    void AddGroupDim(const ScDPSaveNumGroupDimension& aNumGroupDim);
    void AddGroupDim(const ScDPSaveGroupDimension& aGroupDim);
    void SetButtons(ScDPObject* pDPObject);
    void SetSelectedPage( const OUString& rDimName, const OUString& rSelected );
};

class ScXMLDPSourceSQLContext : public ScXMLImportContext
{
public:

    ScXMLDPSourceSQLContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotTableContext* pDataPilotTable);

    virtual ~ScXMLDPSourceSQLContext() override;
};

class ScXMLDPSourceTableContext : public ScXMLImportContext
{
public:

    ScXMLDPSourceTableContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotTableContext* pDataPilotTable);

    virtual ~ScXMLDPSourceTableContext() override;
};

class ScXMLDPSourceQueryContext : public ScXMLImportContext
{
public:

    ScXMLDPSourceQueryContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotTableContext* pDataPilotTable);

    virtual ~ScXMLDPSourceQueryContext() override;
};

class ScXMLSourceServiceContext : public ScXMLImportContext
{
public:

    ScXMLSourceServiceContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotTableContext* pDataPilotTable);

    virtual ~ScXMLSourceServiceContext() override;
};

class ScXMLDataPilotGrandTotalContext : public ScXMLImportContext
{
    enum Orientation { COLUMN, ROW, BOTH, NONE };

    ScXMLDataPilotTableContext* mpTableContext;
    OUString                    maDisplayName;
    Orientation                 meOrientation;
    bool                        mbVisible;

public:
    ScXMLDataPilotGrandTotalContext(
        ScXMLImport& rImport,
        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
        ScXMLDataPilotTableContext* pTableContext );

    virtual ~ScXMLDataPilotGrandTotalContext() override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLSourceCellRangeContext : public ScXMLImportContext
{
    ScXMLDataPilotTableContext* pDataPilotTable;

public:

    ScXMLSourceCellRangeContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotTableContext* pDataPilotTable);

    virtual ~ScXMLSourceCellRangeContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

struct ScXMLDataPilotGroup
{
    ::std::vector<OUString> aMembers;
    OUString aName;
};

class ScXMLDataPilotFieldContext : public ScXMLImportContext
{
    ScXMLDataPilotTableContext* pDataPilotTable;
    std::unique_ptr<ScDPSaveDimension> xDim;

    ::std::vector<ScXMLDataPilotGroup> aGroups;
    OUString                    sGroupSource;
    OUString                    sSelectedPage;
    OUString                    sName;
    double                      fStart;
    double                      fEnd;
    double                      fStep;
    sal_Int32                   nUsedHierarchy;
    sal_Int32                   nGroupPart;
    ScGeneralFunction           nFunction;
    css::sheet::DataPilotFieldOrientation
                                nOrientation;
    bool                        bSelectedPage:1;
    bool                        bIsGroupField:1;
    bool                        bDateValue:1;
    bool                        bAutoStart:1;
    bool                        bAutoEnd:1;
    bool                        mbHasHiddenMember:1; // TODO: import to document core

public:

    ScXMLDataPilotFieldContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotTableContext* pDataPilotTable);

    virtual ~ScXMLDataPilotFieldContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    void SetShowEmpty(const bool bValue) { if (xDim) xDim->SetShowEmpty(bValue); }
    void SetRepeatItemLabels(const bool bSet) { if (xDim) xDim->SetRepeatItemLabels(bSet); }
    void SetSubTotals(std::vector<ScGeneralFunction> const & rFunctions) { if (xDim) xDim->SetSubTotals(rFunctions); }
    void AddMember(std::unique_ptr<ScDPSaveMember> pMember);
    void SetSubTotalName(const OUString& rName);
    void SetFieldReference(const css::sheet::DataPilotFieldReference& aRef) { if (xDim) xDim->SetReferenceValue(&aRef); }
    void SetAutoShowInfo(const css::sheet::DataPilotFieldAutoShowInfo& aInfo) { if (xDim) xDim->SetAutoShowInfo(&aInfo); }
    void SetSortInfo(const css::sheet::DataPilotFieldSortInfo& aInfo) { if (xDim) xDim->SetSortInfo(&aInfo); }
    void SetLayoutInfo(const css::sheet::DataPilotFieldLayoutInfo& aInfo) { if (xDim) xDim->SetLayoutInfo(&aInfo); }
    void SetGrouping(const OUString& rGroupSource, const double& rStart, const double& rEnd, const double& rStep,
        sal_Int32 nPart, bool bDate, bool bAutoSt, bool bAutoE)
    {
        bIsGroupField = true;
        sGroupSource = rGroupSource;
        fStart = rStart;
        fEnd = rEnd;
        fStep = rStep;
        nGroupPart = nPart;
        bDateValue = bDate;
        bAutoStart = bAutoSt;
        bAutoEnd = bAutoE;
    }
    void AddGroup(const ::std::vector<OUString>& rMembers, const OUString& rName);
};

class ScXMLDataPilotFieldReferenceContext : public ScXMLImportContext
{
public:

    ScXMLDataPilotFieldReferenceContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotFieldReferenceContext() override;
};

class ScXMLDataPilotLevelContext : public ScXMLImportContext
{
    ScXMLDataPilotFieldContext* pDataPilotField;

public:

    ScXMLDataPilotLevelContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotLevelContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

class ScXMLDataPilotDisplayInfoContext : public ScXMLImportContext
{
public:

    ScXMLDataPilotDisplayInfoContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotDisplayInfoContext() override;
};

class ScXMLDataPilotSortInfoContext : public ScXMLImportContext
{
public:

    ScXMLDataPilotSortInfoContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotSortInfoContext() override;
};

class ScXMLDataPilotLayoutInfoContext : public ScXMLImportContext
{
public:

    ScXMLDataPilotLayoutInfoContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotLayoutInfoContext() override;
};

class ScXMLDataPilotSubTotalsContext : public ScXMLImportContext
{
    ScXMLDataPilotFieldContext*    pDataPilotField;

    std::vector<ScGeneralFunction> maFunctions;
    OUString                       maDisplayName;

public:
    ScXMLDataPilotSubTotalsContext( ScXMLImport& rImport,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotSubTotalsContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
    void AddFunction(ScGeneralFunction nFunction);
    void SetDisplayName(const OUString& rName);
};

class ScXMLDataPilotSubTotalContext : public ScXMLImportContext
{
public:

    ScXMLDataPilotSubTotalContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotSubTotalsContext* pDataPilotSubTotals);

    virtual ~ScXMLDataPilotSubTotalContext() override;
};

class ScXMLDataPilotMembersContext : public ScXMLImportContext
{
    ScXMLDataPilotFieldContext* const pDataPilotField;

public:

    ScXMLDataPilotMembersContext( ScXMLImport& rImport,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotMembersContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

class ScXMLDataPilotMemberContext : public ScXMLImportContext
{
    ScXMLDataPilotFieldContext* pDataPilotField;

    OUString sName;
    OUString maDisplayName;
    bool     bDisplay;
    bool     bDisplayDetails;
    bool     bHasName;

public:

    ScXMLDataPilotMemberContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotMemberContext() override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLDataPilotGroupsContext : public ScXMLImportContext
{
    ScXMLDataPilotFieldContext* pDataPilotField;

public:

    ScXMLDataPilotGroupsContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotGroupsContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

class ScXMLDataPilotGroupContext : public ScXMLImportContext
{
    ScXMLDataPilotFieldContext* pDataPilotField;

    OUString sName;
    ::std::vector<OUString> aMembers;

public:

    ScXMLDataPilotGroupContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotGroupContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    void AddMember(const OUString& sMember) { aMembers.push_back(sMember); }
};

class ScXMLDataPilotGroupMemberContext : public ScXMLImportContext
{
    ScXMLDataPilotGroupContext* pDataPilotGroup;
    OUString sName;

public:

    ScXMLDataPilotGroupMemberContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotGroupContext* pDataPilotGroup);

    virtual ~ScXMLDataPilotGroupMemberContext() override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
