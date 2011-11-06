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



#ifndef SC_XMLCOLUMNROWGROUPEXPORT_HXX
#define SC_XMLCOLUMNROWGROUPEXPORT_HXX

#include <list>
#include <rtl/ustring.hxx>
#include <sal/types.h>

struct ScMyColumnRowGroup
{
    sal_Int32   nField;
    sal_Int16   nLevel;
    sal_Bool    bDisplay;

    ScMyColumnRowGroup();
    sal_Bool operator< (const ScMyColumnRowGroup& rGroup) const;
};

typedef std::list <ScMyColumnRowGroup> ScMyColumnRowGroupVec;
typedef std::list <sal_Int32> ScMyFieldGroupVec;

class ScXMLExport;
class ScMyOpenCloseColumnRowGroup
{
    ScXMLExport&                rExport;
    const rtl::OUString         rName;
    ScMyColumnRowGroupVec       aTableStart;
    ScMyFieldGroupVec           aTableEnd;

    void OpenGroup(const ScMyColumnRowGroup& rGroup);
    void CloseGroup();
public:
    ScMyOpenCloseColumnRowGroup(ScXMLExport& rExport, sal_uInt32 nToken);
    ~ScMyOpenCloseColumnRowGroup();

    void NewTable();
    void AddGroup(const ScMyColumnRowGroup& aGroup, const sal_Int32 nEndField);
    sal_Bool IsGroupStart(const sal_Int32 nField);
    void OpenGroups(const sal_Int32 nField);
    sal_Bool IsGroupEnd(const sal_Int32 nField);
    void CloseGroups(const sal_Int32 nField);
    sal_Int32 GetLast();
    void Sort();
};

#endif

