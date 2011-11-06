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



#ifndef SC_DPOUTPUTGEOMETRY_HXX
#define SC_DPOUTPUTGEOMETRY_HXX

#include "address.hxx"
#include <vector>

class ScAddress;

class SC_DLLPUBLIC ScDPOutputGeometry
{
public:
    enum FieldType { Column, Row, Page, Data, None };
    enum ImportType { ODF, XLS };

    ScDPOutputGeometry(const ScRange& rOutRange, bool bShowFilter, ImportType eImportType);
    ~ScDPOutputGeometry();

    /**
     * @param nCount number of row fields, <b>excluding the data layout
     *               field if exists</b>.
     */
    void setRowFieldCount(sal_uInt32 nCount);
    void setColumnFieldCount(sal_uInt32 nCount);
    void setPageFieldCount(sal_uInt32 nCount);
    void setDataFieldCount(sal_uInt32 nCount);

    void getColumnFieldPositions(::std::vector<ScAddress>& rAddrs) const;
    void getRowFieldPositions(::std::vector<ScAddress>& rAddrs) const;
    void getPageFieldPositions(::std::vector<ScAddress>& rAddrs) const;

    SCROW getRowFieldHeaderRow() const;

    FieldType getFieldButtonType(const ScAddress& rPos) const;

private:
    ScDPOutputGeometry(); // disabled

private:
    ScRange     maOutRange;
    sal_uInt32  mnRowFields;    /// number of row fields (data layout field NOT included!)
    sal_uInt32  mnColumnFields;
    sal_uInt32  mnPageFields;
    sal_uInt32  mnDataFields;

    ImportType  meImportType;

    bool        mbShowFilter;
};

#endif
