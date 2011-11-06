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



#ifndef SC_XMLFILTI_HXX
#define SC_XMLFILTI_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/FilterOperator.hpp>
#include <com/sun/star/sheet/FilterOperator2.hpp>
#include <com/sun/star/sheet/TableFilterField2.hpp>
#include <tools/stack.hxx>

#include "xmldrani.hxx"
#include "xmldpimp.hxx"

class ScXMLImport;

class ScXMLFilterContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

    com::sun::star::uno::Sequence <com::sun::star::sheet::TableFilterField2> aFilterFields;
    com::sun::star::table::CellAddress aOutputPosition;
    com::sun::star::table::CellRangeAddress aConditionSourceRangeAddress;
    sal_Int16   nUserListIndex;
    sal_Bool    bSkipDuplicates;
    sal_Bool    bCopyOutputData;
    sal_Bool    bUseRegularExpressions;
    sal_Bool    bIsCaseSensitive;
    sal_Bool    bEnabledUserList;
    sal_Bool    bConnectionOr;
    sal_Bool    bNextConnectionOr;
    sal_Bool    bConditionSourceRange;
    Stack       aConnectionOrStack;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLFilterContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLFilterContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

    void SetIsCaseSensitive(const sal_Bool bTemp) { bIsCaseSensitive = bTemp; }
    void SetUseRegularExpressions(const sal_Bool bTemp) { if (!bUseRegularExpressions) bUseRegularExpressions = bTemp;}
    void OpenConnection(const sal_Bool bTemp) { sal_Bool* pTemp = new sal_Bool; *pTemp = bConnectionOr;
                            bConnectionOr = bNextConnectionOr; bNextConnectionOr = bTemp;
                            aConnectionOrStack.Push(pTemp);}
    void CloseConnection() { sal_Bool* pTemp = static_cast <sal_Bool*> (aConnectionOrStack.Pop()); bConnectionOr = *pTemp; bNextConnectionOr = *pTemp; delete pTemp;}
    sal_Bool GetConnection() { sal_Bool bTemp = bConnectionOr; bConnectionOr = bNextConnectionOr; return bTemp; }
    void AddFilterField(const com::sun::star::sheet::TableFilterField2 aFilterField) { aFilterFields.realloc(aFilterFields.getLength() + 1);
                                                                                aFilterFields[aFilterFields.getLength() - 1] = aFilterField; }
};

class ScXMLAndContext : public SvXMLImportContext
{
    ScXMLFilterContext* pFilterContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLAndContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLFilterContext* pTempFilterContext);

    virtual ~ScXMLAndContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLOrContext : public SvXMLImportContext
{
    ScXMLFilterContext* pFilterContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLOrContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLFilterContext* pTempFilterContext);

    virtual ~ScXMLOrContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLConditionContext : public SvXMLImportContext
{
    ScXMLFilterContext* pFilterContext;

    rtl::OUString sDataType;
    rtl::OUString sConditionValue;
    rtl::OUString sOperator;
    sal_Int32   nField;
    sal_Bool    bIsCaseSensitive;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLConditionContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLFilterContext* pTempFilterContext);

    virtual ~ScXMLConditionContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    void getOperatorXML(const rtl::OUString sTempOperator, sal_Int32& aFilterOperator, sal_Bool& bUseRegularExpressions) const;
    virtual void EndElement();
};

// Datapilot (Core)

class ScXMLDPFilterContext : public SvXMLImportContext
{
    ScXMLDataPilotTableContext* pDataPilotTable;

    ScQueryParam    aFilterFields;
    ScAddress       aOutputPosition;
    ScRange         aConditionSourceRangeAddress;
    sal_uInt8   nFilterFieldCount;
    sal_Int16   nUserListIndex;
    sal_Bool    bSkipDuplicates;
    sal_Bool    bCopyOutputData;
    sal_Bool    bUseRegularExpressions;
    sal_Bool    bIsCaseSensitive;
    sal_Bool    bEnabledUserList;
    sal_Bool    bConnectionOr;
    sal_Bool    bNextConnectionOr;
    sal_Bool    bConditionSourceRange;
    Stack       aConnectionOrStack;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLDPFilterContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotTableContext* pTempDataPilotTableContext);

    virtual ~ScXMLDPFilterContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

    void SetIsCaseSensitive(const sal_Bool bTemp) { bIsCaseSensitive = bTemp; }
    void SetUseRegularExpressions(const sal_Bool bTemp) { if (!bUseRegularExpressions) bUseRegularExpressions = bTemp;}
    void OpenConnection(const sal_Bool bTemp) { sal_Bool* pTemp = new sal_Bool; *pTemp = bConnectionOr;
                            bConnectionOr = bNextConnectionOr; bNextConnectionOr = bTemp;
                            aConnectionOrStack.Push(pTemp);}
    void CloseConnection() { sal_Bool* pTemp = static_cast <sal_Bool*> (aConnectionOrStack.Pop()); bConnectionOr = *pTemp; bNextConnectionOr = *pTemp; delete pTemp;}
    sal_Bool GetConnection() { sal_Bool bTemp = bConnectionOr; bConnectionOr = bNextConnectionOr; return bTemp; }
    void AddFilterField (const ScQueryEntry& aFilterField);
};

class ScXMLDPAndContext : public SvXMLImportContext
{
    ScXMLDPFilterContext* pFilterContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLDPAndContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDPFilterContext* pTempFilterContext);

    virtual ~ScXMLDPAndContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDPOrContext : public SvXMLImportContext
{
    ScXMLDPFilterContext* pFilterContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLDPOrContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDPFilterContext* pTempFilterContext);

    virtual ~ScXMLDPOrContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDPConditionContext : public SvXMLImportContext
{
    ScXMLDPFilterContext* pFilterContext;

    rtl::OUString sDataType;
    rtl::OUString sConditionValue;
    rtl::OUString sOperator;
    sal_Int32   nField;
    sal_Bool    bIsCaseSensitive;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLDPConditionContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDPFilterContext* pTempFilterContext);

    virtual ~ScXMLDPConditionContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    void getOperatorXML(const rtl::OUString sTempOperator, ScQueryOp& aFilterOperator, sal_Bool& bUseRegularExpressions,
                        double& dVal) const;
    virtual void EndElement();
};

#endif
