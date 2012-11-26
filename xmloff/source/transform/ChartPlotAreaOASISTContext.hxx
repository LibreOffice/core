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


#ifndef XMLOFF_XMLCHARTPLOTAREAOASISTCONTEXT_HXX
#define XMLOFF_XMLCHARTPLOTAREAOASISTCONTEXT_HXX

#include "ProcAttrTContext.hxx"
#include "PersAttrListTContext.hxx"
#include <rtl/ref.hxx>

class XMLChartPlotAreaOASISTContext  : public XMLProcAttrTransformerContext
{
public:
    explicit XMLChartPlotAreaOASISTContext(
        XMLTransformerBase & rTransformer, const ::rtl::OUString & rQName );
    virtual ~XMLChartPlotAreaOASISTContext();

    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::rtl::OUString& rQName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    void ExportCategories();

private:
    ::rtl::Reference< XMLPersAttrListTContext > m_rCategoriesContext;
};

// XMLOFF_XMLCHARTPLOTAREAOASISTCONTEXT_HXX
#endif
