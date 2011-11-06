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



#ifndef SC_XMLCONSOLIDATIONCONTEXT_HXX
#define SC_XMLCONSOLIDATIONCONTEXT_HXX

#include "global.hxx"
#include "address.hxx"
#include <xmloff/xmlimp.hxx>

class ScXMLImport;


//___________________________________________________________________

class ScXMLConsolidationContext : public SvXMLImportContext
{
private:
    ::rtl::OUString             sSourceList;
    ::rtl::OUString             sUseLabel;
    ScAddress                   aTargetAddr;
    ScSubTotalFunc              eFunction;
    sal_Bool                    bLinkToSource;
    sal_Bool                    bTargetAddr;

protected:
    const ScXMLImport&          GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport&                GetScImport()       { return (ScXMLImport&)GetImport(); }

public:
                                ScXMLConsolidationContext(
                                    ScXMLImport& rImport,
                                    sal_uInt16 nPrfx,
                                    const ::rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList
                                    );
    virtual                     ~ScXMLConsolidationContext();

    virtual SvXMLImportContext* CreateChildContext(
                                    sal_uInt16 nPrefix,
                                    const ::rtl::OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList
                                    );
    virtual void                EndElement();
};


#endif

