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


#ifndef SC_XMLTABI_HXX
#define SC_XMLTABI_HXX

#include "externalrefmgr.hxx"

#include <xmloff/xmlictxt.hxx>
#include <memory>

class ScXMLImport;

struct ScXMLExternalTabData
{
    String maFileUrl;
    ScExternalRefCache::TableTypeRef mpCacheTable;
    sal_Int32 mnRow;
    sal_Int32 mnCol;
    sal_uInt16 mnFileId;

    ScXMLExternalTabData();
};

class ScXMLTableContext : public SvXMLImportContext
{
    rtl::OUString   sPrintRanges;
    ::std::auto_ptr<ScXMLExternalTabData> pExternalRefInfo;
    sal_Int32       nStartOffset;
    sal_Bool        bStartFormPage;
    sal_Bool        bPrintEntireSheet;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLTableContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        const sal_Bool bTempIsSubTable = sal_False,
                        const sal_Int32 nSpannedCols = 0);

    virtual ~ScXMLTableContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

#endif
