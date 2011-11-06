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



#ifndef SC_XMLTEXTPCONTEXT_HXX
#define SC_XMLTEXTPCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <rtl/ustrbuf.hxx>

class ScXMLImport;
class ScXMLTableRowCellContext;

class ScXMLTextPContext : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList> xAttrList;
    SvXMLImportContext*         pTextPContext;
    ScXMLTableRowCellContext*   pCellContext;
    rtl::OUString               sLName;
    rtl::OUString               sSimpleContent;     // copy of the first Character call's argument
    rtl::OUStringBuffer*        pContentBuffer;     // used if there's more than one string
    sal_uInt16                      nPrefix;
    sal_Bool                    bIsOwn;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLTextPContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLTableRowCellContext* pCellContext);

    virtual ~ScXMLTextPContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void Characters( const ::rtl::OUString& rChars );

    virtual void EndElement();

    void AddSpaces(sal_Int32 nSpaceCount);
};

#endif
