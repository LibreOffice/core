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

#pragma once

#include <sal/config.h>

#include <optional>

#include <rtl/ustring.hxx>
#include <global.hxx>
#include <xmloff/xmltoken.hxx>
#include <unotools/textsearch.hxx>

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

    static OUString getDPOperatorXML(const ScQueryOp aFilterOperator, const utl::SearchParam::SearchType eSearchType);
    void WriteDPCondition(const ScQueryEntry& aQueryEntry, bool bIsCaseSensitive,
            utl::SearchParam::SearchType eSearchType);
    void WriteDPFilter(const ScDocument& rDoc, const ScQueryParam& aQueryParam);

    void WriteFieldReference(const ScDPSaveDimension* pDim);
    void WriteSortInfo(const ScDPSaveDimension* pDim);
    void WriteAutoShowInfo(const ScDPSaveDimension* pDim);
    void WriteLayoutInfo(const ScDPSaveDimension* pDim);
    void WriteSubTotals(const ScDPSaveDimension* pDim);
    void WriteMembers(const ScDPSaveDimension* pDim);
    void WriteLevels(const ScDPSaveDimension* pDim);
    void WriteDatePart(sal_Int32 nPart);
    void WriteNumGroupInfo(const ScDPNumGroupInfo& pGroupInfo);
    void WriteGroupDimAttributes(const ScDPSaveGroupDimension& rGroupDim);
    void WriteGroupDimElements(const ScDPSaveDimension* pDim, const ScDPDimensionSaveData* pDimData);
    void WriteNumGroupDim(const ScDPSaveNumGroupDimension& rNumGroupDim);
    void WriteDimension(const ScDPSaveDimension* pDim, const ScDPDimensionSaveData* pDimData);
    void WriteDimensions(const ScDPSaveData* pDPSave);

    void WriteGrandTotal(::xmloff::token::XMLTokenEnum eOrient, bool bVisible, const std::optional<OUString> & pGrandTotal);

public:
    explicit ScXMLExportDataPilot(ScXMLExport& rExport);
    void WriteDataPilots(ScDocument& rDoc);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
