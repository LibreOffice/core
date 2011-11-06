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


#ifndef _XMLOFF_SCH_XMLEXPORTHELPER_HXX_
#define _XMLOFF_SCH_XMLEXPORTHELPER_HXX_

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"
#include <xmloff/uniref.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/util/XStringMapping.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <xmloff/xmlprmap.hxx>

#include <queue>
#include <vector>

class SvXMLAutoStylePoolP;
class SvXMLExport;
class SchXMLExportHelper_Impl;

/** With this class you can export a <chart:chart> element containing
    its data as <table:table> element or without internal table. In
    the latter case you have to provide a table address mapper if the
    cell addressing set at the document is not in XML format.
 */
class XMLOFF_DLLPUBLIC SchXMLExportHelper : public UniRefBase
{
public:
    SchXMLExportHelper( SvXMLExport& rExport,
                        SvXMLAutoStylePoolP& rASPool );

    virtual ~SchXMLExportHelper();

    /// returns the string corresponding to the current FileFormat CLSID for Chart
    const rtl::OUString& getChartCLSID();

private:
    SchXMLExportHelper(); // not defined
    SchXMLExportHelper(SchXMLExportHelper &); // not defined
    void operator =(SchXMLExportHelper &); // not defined

private:
    SchXMLExportHelper_Impl* m_pImpl;
    friend class SchXMLExport;
};

#endif  // _XMLOFF_SCH_XMLEXPORTHELPER_HXX_
