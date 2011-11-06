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



#ifndef SC_XMLEXPORTDDELINKS_HXX
#define SC_XMLEXPORTDDELINKS_HXX

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>

class String;
class ScXMLExport;

class ScXMLExportDDELinks
{
    ScXMLExport&        rExport;

    sal_Bool            CellsEqual(const sal_Bool bPrevEmpty, const sal_Bool bPrevString, const String& sPrevValue, const double& fPrevValue,
                                    const sal_Bool bEmpty, const sal_Bool bString, const String& sValue, const double& fValue);
    void                WriteCell(const sal_Bool bEmpty, const sal_Bool bString, const String& sValue, const double& fValue, const sal_Int32 nRepeat);
    void                WriteTable(const sal_Int32 nPos);
public:
    ScXMLExportDDELinks(ScXMLExport& rExport);
    ~ScXMLExportDDELinks();
    void WriteDDELinks(::com::sun::star::uno::Reference < ::com::sun::star::sheet::XSpreadsheetDocument >& xSpreadDoc);
};

#endif


