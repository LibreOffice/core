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

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>

#include "global.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "queryparam.hxx"
#include "xmlimprt.hxx"

#include <unordered_map>

class ScDPSaveNumGroupDimension;
class ScDPSaveGroupDimension;

enum ScMySourceType
{
    SQL,
    TABLE,
    QUERY,
    SERVICE,
    CELLRANGE
};

class ScXMLDataPilotTablesContext : public SvXMLImportContext
{

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDataPilotTablesContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLDataPilotTablesContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
};

class ScXMLDataPilotTableContext : public SvXMLImportContext
{
    typedef std::unordered_map<OUString, OUString, OUStringHash> SelectedPagesType;

    struct GrandTotalItem
    {
        OUString maDisplayName;
        bool            mbVisible;
        GrandTotalItem();
    };
    ScDocument*     pDoc;
    ScDPObject*     pDPObject;
    boost::scoped_ptr<ScDPSaveData> pDPSave;
    ScDPDimensionSaveData* pDPDimSaveData;
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
    ScRange         aFilterSourceRange;
    ScAddress       aFilterOutputPosition;
    ScQueryParam    aSourceQueryParam;
    ScMySourceType  nSourceType;
    sal_uInt32      mnRowFieldCount;
    sal_uInt32      mnColFieldCount;
    sal_uInt32      mnPageFieldCount;
    sal_uInt32      mnDataFieldCount;
    sal_uInt16 mnDataLayoutType;
    bool            bIsNative:1;
    bool            bIgnoreEmptyRows:1;
    bool            bIdentifyCategories:1;
    bool            bTargetRangeAddress:1;
    bool            bSourceCellRange:1;
    bool            bShowFilter:1;
    bool            bDrillDown:1;
    bool            bHeaderGridLayout:1;

    SelectedPagesType maSelectedPages;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDataPilotTableContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLDataPilotTableContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;

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
    void SetFilterOutputPosition(const ScAddress& aValue) { aFilterOutputPosition = aValue; }
    void SetFilterSourceRange(const ScRange& aValue) { aFilterSourceRange = aValue; }
    void AddDimension(ScDPSaveDimension* pDim);
    void AddGroupDim(const ScDPSaveNumGroupDimension& aNumGroupDim);
    void AddGroupDim(const ScDPSaveGroupDimension& aGroupDim);
    void SetButtons();
    void SetSelectedPage( const OUString& rDimName, const OUString& rSelected );
};

class ScXMLDPSourceSQLContext : public SvXMLImportContext
{
    ScXMLDataPilotTableContext* pDataPilotTable;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDPSourceSQLContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotTableContext* pDataPilotTable);

    virtual ~ScXMLDPSourceSQLContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
};

class ScXMLDPSourceTableContext : public SvXMLImportContext
{
    ScXMLDataPilotTableContext* pDataPilotTable;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDPSourceTableContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotTableContext* pDataPilotTable);

    virtual ~ScXMLDPSourceTableContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
};

class ScXMLDPSourceQueryContext : public SvXMLImportContext
{
    ScXMLDataPilotTableContext* pDataPilotTable;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDPSourceQueryContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotTableContext* pDataPilotTable);

    virtual ~ScXMLDPSourceQueryContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
};

class ScXMLSourceServiceContext : public SvXMLImportContext
{
    ScXMLDataPilotTableContext* pDataPilotTable;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLSourceServiceContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotTableContext* pDataPilotTable);

    virtual ~ScXMLSourceServiceContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
};

class ScXMLDataPilotGrandTotalContext : public SvXMLImportContext
{
    enum Orientation { COLUMN, ROW, BOTH, NONE };

    ScXMLImport& GetScImport();

    ScXMLDataPilotTableContext* mpTableContext;
    OUString             maDisplayName;
    Orientation                 meOrientation;
    bool                        mbVisible;

public:
    ScXMLDataPilotGrandTotalContext(
        ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        ScXMLDataPilotTableContext* pTableContext );

    virtual ~ScXMLDataPilotGrandTotalContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
};

class ScXMLSourceCellRangeContext : public SvXMLImportContext
{
    ScXMLDataPilotTableContext* pDataPilotTable;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLSourceCellRangeContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotTableContext* pDataPilotTable);

    virtual ~ScXMLSourceCellRangeContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
};

struct ScXMLDataPilotGroup
{
    ::std::vector<OUString> aMembers;
    OUString aName;
};

class ScXMLDataPilotFieldContext : public SvXMLImportContext
{
    ScXMLDataPilotTableContext* pDataPilotTable;
    ScDPSaveDimension*          pDim;

    ::std::vector<ScXMLDataPilotGroup> aGroups;
    OUString                    sGroupSource;
    OUString                    sSelectedPage;
    OUString                    sName;
    double                      fStart;
    double                      fEnd;
    double                      fStep;
    sal_Int32                   nUsedHierarchy;
    sal_Int32                   nGroupPart;
    sal_Int16                   nFunction;
    sal_Int16                   nOrientation;
    bool                        bSelectedPage:1;
    bool                        bIsGroupField:1;
    bool                        bDateValue:1;
    bool                        bAutoStart:1;
    bool                        bAutoEnd:1;
    bool                        mbHasHiddenMember:1;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDataPilotFieldContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotTableContext* pDataPilotTable);

    virtual ~ScXMLDataPilotFieldContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;

    void SetShowEmpty(const bool bValue) { if (pDim) pDim->SetShowEmpty(bValue); }
    void SetRepeatItemLabels(const bool bSet) { if (pDim) pDim->SetRepeatItemLabels(bSet); }
    void SetSubTotals(const sal_uInt16* pFunctions, const sal_Int16 nCount) { if(pDim) pDim->SetSubTotals(nCount, pFunctions); }
    void AddMember(ScDPSaveMember* pMember);
    void SetSubTotalName(const OUString& rName);
    void SetFieldReference(const com::sun::star::sheet::DataPilotFieldReference& aRef) { if (pDim) pDim->SetReferenceValue(&aRef); }
    void SetAutoShowInfo(const com::sun::star::sheet::DataPilotFieldAutoShowInfo& aInfo) { if (pDim) pDim->SetAutoShowInfo(&aInfo); }
    void SetSortInfo(const com::sun::star::sheet::DataPilotFieldSortInfo& aInfo) { if (pDim) pDim->SetSortInfo(&aInfo); }
    void SetLayoutInfo(const com::sun::star::sheet::DataPilotFieldLayoutInfo& aInfo) { if (pDim) pDim->SetLayoutInfo(&aInfo); }
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

class ScXMLDataPilotFieldReferenceContext : public SvXMLImportContext
{
//    com::sun::star::sheet::DataPilotFieldReference aReference;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDataPilotFieldReferenceContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotFieldReferenceContext();
};

class ScXMLDataPilotLevelContext : public SvXMLImportContext
{
    ScXMLDataPilotFieldContext* pDataPilotField;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDataPilotLevelContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotLevelContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
};

class ScXMLDataPilotDisplayInfoContext : public SvXMLImportContext
{
//    com::sun::star::sheet::DataPilotFieldAutoShowInfo aInfo;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDataPilotDisplayInfoContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotDisplayInfoContext();
};

class ScXMLDataPilotSortInfoContext : public SvXMLImportContext
{
//    com::sun::star::sheet::DataPilotFieldSortInfo aInfo;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDataPilotSortInfoContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotSortInfoContext();
};

class ScXMLDataPilotLayoutInfoContext : public SvXMLImportContext
{
//    com::sun::star::sheet::DataPilotFieldLayoutInfo aInfo;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDataPilotLayoutInfoContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotLayoutInfoContext();
};

class ScXMLDataPilotSubTotalsContext : public SvXMLImportContext
{
    ScXMLDataPilotFieldContext* pDataPilotField;

    sal_Int16   nFunctionCount;
    sal_uInt16* pFunctions;
    OUString maDisplayName;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLDataPilotSubTotalsContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotSubTotalsContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
    void AddFunction(sal_Int16 nFunction);
    void SetDisplayName(const OUString& rName);
};

class ScXMLDataPilotSubTotalContext : public SvXMLImportContext
{
    ScXMLDataPilotSubTotalsContext* pDataPilotSubTotals;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDataPilotSubTotalContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotSubTotalsContext* pDataPilotSubTotals);

    virtual ~ScXMLDataPilotSubTotalContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
};

class ScXMLDataPilotMembersContext : public SvXMLImportContext
{
    ScXMLDataPilotFieldContext* pDataPilotField;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDataPilotMembersContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotMembersContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
};

class ScXMLDataPilotMemberContext : public SvXMLImportContext
{
    ScXMLDataPilotFieldContext* pDataPilotField;

    OUString sName;
    OUString maDisplayName;
    bool     bDisplay;
    bool     bDisplayDetails;
    bool     bHasName;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDataPilotMemberContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotMemberContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
};

class ScXMLDataPilotGroupsContext : public SvXMLImportContext
{
    ScXMLDataPilotFieldContext* pDataPilotField;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDataPilotGroupsContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotGroupsContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
};

class ScXMLDataPilotGroupContext : public SvXMLImportContext
{
    ScXMLDataPilotFieldContext* pDataPilotField;

    OUString sName;
    ::std::vector<OUString> aMembers;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDataPilotGroupContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotFieldContext* pDataPilotField);

    virtual ~ScXMLDataPilotGroupContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;

    void AddMember(const OUString& sMember) { aMembers.push_back(sMember); }
};

class ScXMLDataPilotGroupMemberContext : public SvXMLImportContext
{
    ScXMLDataPilotGroupContext* pDataPilotGroup;

    OUString sName;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDataPilotGroupMemberContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotGroupContext* pDataPilotGroup);

    virtual ~ScXMLDataPilotGroupMemberContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
