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



#ifndef _XMLOFF_PAGEMASTEREXPORTPROPMAPPER_HXX
#define _XMLOFF_PAGEMASTEREXPORTPROPMAPPER_HXX

#include <xmloff/xmlexppr.hxx>
#include "XMLBackgroundImageExport.hxx"
#include "XMLTextColumnsExport.hxx"
#include "XMLFootnoteSeparatorExport.hxx"

//______________________________________________________________________________

class XMLPageMasterExportPropMapper : public SvXMLExportPropertyMapper
{
protected:
    XMLBackgroundImageExport aBackgroundImageExport;
    XMLTextColumnsExport aTextColumnsExport;
    XMLFootnoteSeparatorExport aFootnoteSeparatorExport;

    virtual void        ContextFilter(
                            ::std::vector< XMLPropertyState >& rProperties,
                            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rPropSet
                            ) const;

public:
                        XMLPageMasterExportPropMapper(
                             const UniReference< XMLPropertySetMapper >& rMapper,
                            SvXMLExport& rExport
                            );
    virtual             ~XMLPageMasterExportPropMapper();

    virtual void        handleElementItem(
                            SvXMLExport& rExport,
                            const XMLPropertyState& rProperty,
                            sal_uInt16 nFlags,
                            const ::std::vector< XMLPropertyState >* pProperties = 0,
                            sal_uInt32 nIdx = 0
                            ) const;
    virtual void        handleSpecialItem(
                            SvXMLAttributeList& rAttrList,
                            const XMLPropertyState& rProperty,
                            const SvXMLUnitConverter& rUnitConverter,
                            const SvXMLNamespaceMap& rNamespaceMap,
                            const ::std::vector< XMLPropertyState >* pProperties = 0,
                            sal_uInt32 nIdx = 0
                            ) const;

    inline XMLBackgroundImageExport& GetBackgroundImageExport()
                                { return aBackgroundImageExport; }

};

#endif

