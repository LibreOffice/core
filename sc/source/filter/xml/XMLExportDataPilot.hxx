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

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLEXPORTDATAPILOT_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLEXPORTDATAPILOT_HXX

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <rtl/ustring.hxx>
#include "global.hxx"
#include <xmloff/xmltoken.hxx>

class ScXMLExport;
class ScDocument;
class ScDPSaveDimension;
class ScDPSaveData;
class ScDPDimensionSaveData;
class ScDPSaveGroupDimension;
class ScDPSaveNumGroupDimension;
struct ScDPNumGroupInfo;
struct ScQueryParam;
struct ScQueryEntry;

class ScXMLExportDataPilot
{
    ScXMLExport&        rExport;
    ScDocument*         pDoc;

    static OUString getDPOperatorXML(const ScQueryOp aFilterOperator, const bool bUseRegularExpressions);
    void WriteDPCondition(const ScQueryEntry& aQueryEntry, bool bIsCaseSensitive, bool bUseRegularExpressions);
    void WriteDPFilter(const ScQueryParam& aQueryParam);

    void WriteFieldReference(ScDPSaveDimension* pDim);
    void WriteSortInfo(ScDPSaveDimension* pDim);
    void WriteAutoShowInfo(ScDPSaveDimension* pDim);
    void WriteLayoutInfo(ScDPSaveDimension* pDim);
    void WriteSubTotals(ScDPSaveDimension* pDim);
    void WriteMembers(ScDPSaveDimension* pDim);
    void WriteLevels(ScDPSaveDimension* pDim);
    void WriteDatePart(sal_Int32 nPart);
    void WriteNumGroupInfo(const ScDPNumGroupInfo& pGroupInfo);
    void WriteGroupDimAttributes(const ScDPSaveGroupDimension* pGroupDim);
    void WriteGroupDimElements(ScDPSaveDimension* pDim, const ScDPDimensionSaveData* pDimData);
    void WriteNumGroupDim(const ScDPSaveNumGroupDimension* pNumGroupDim);
    void WriteDimension(ScDPSaveDimension* pDim, const ScDPDimensionSaveData* pDimData);
    void WriteDimensions(ScDPSaveData* pDPSave);

    void WriteGrandTotal(::xmloff::token::XMLTokenEnum eOrient, bool bVisible, const OUString* pGrandTotal);

public:
    ScXMLExportDataPilot(ScXMLExport& rExport);
    ~ScXMLExportDataPilot();
    void WriteDataPilots(const css::uno::Reference <css::sheet::XSpreadsheetDocument>& xSpreaDoc);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
