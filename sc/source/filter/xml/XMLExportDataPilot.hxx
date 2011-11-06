/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_XMLEXPORTDATAPILOT_HXX
#define SC_XMLEXPORTDATAPILOT_HXX

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <rtl/ustring.hxx>
#include "global.hxx"
#include "xmloff/xmltoken.hxx"

class ScXMLExport;
class ScDocument;
class ScDPSaveDimension;
class ScDPSaveData;
class ScDPDimensionSaveData;
class ScDPSaveGroupDimension;
class ScDPSaveNumGroupDimension;
struct ScDPNumGroupInfo;
struct ScQueryParam;

class ScXMLExportDataPilot
{
    ScXMLExport&        rExport;
    ScDocument*         pDoc;

    rtl::OUString getDPOperatorXML(const ScQueryOp aFilterOperator, const sal_Bool bUseRegularExpressions,
                                    const sal_Bool bIsString, const double dVal, const String& sVal) const;
    void WriteDPCondition(const ScQueryEntry& aQueryEntry, sal_Bool bIsCaseSensitive, sal_Bool bUseRegularExpressions);
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

    void WriteGrandTotal(::xmloff::token::XMLTokenEnum eOrient, bool bVisible, const ::rtl::OUString* pGrandTotal);

public:
    ScXMLExportDataPilot(ScXMLExport& rExport);
    ~ScXMLExportDataPilot();
    void WriteDataPilots(const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheetDocument>& xSpreaDoc);
};

#endif

