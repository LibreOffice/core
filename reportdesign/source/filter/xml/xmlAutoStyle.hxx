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


#ifndef RPT_XMLAUTOSTYLE_HXX
#define RPT_XMLAUTOSTYLE_HXX

#include <xmloff/maptype.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmlprmap.hxx>

namespace rptxml
{
    class ORptExport;
    class OXMLAutoStylePoolP : public SvXMLAutoStylePoolP
    {
        ORptExport& rORptExport;

        virtual void exportStyleAttributes(
                SvXMLAttributeList& rAttrList,
                sal_Int32 nFamily,
                const ::std::vector< XMLPropertyState >& rProperties,
                const SvXMLExportPropertyMapper& rPropExp,
                const SvXMLUnitConverter& rUnitConverter,
                const SvXMLNamespaceMap& rNamespaceMap
                ) const;

        OXMLAutoStylePoolP(const OXMLAutoStylePoolP&);
        void operator =(const OXMLAutoStylePoolP&);
    public:
        OXMLAutoStylePoolP(ORptExport& rXMLExport);
        virtual ~OXMLAutoStylePoolP();
    };
// -----------------------------------------------------------------------------
} // rptxml
// -----------------------------------------------------------------------------
#endif // RPT_XMLAUTOSTYLE_HXX

