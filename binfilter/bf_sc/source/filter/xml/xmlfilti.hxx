/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SC_XMLFILTI_HXX
#define SC_XMLFILTI_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <bf_xmloff/xmlictxt.hxx>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <bf_xmloff/xmlimp.hxx>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_FILTEROPERATOR_HPP_
#include <com/sun/star/sheet/FilterOperator.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_TABLEFILTERFIELD_HPP_
#include <com/sun/star/sheet/TableFilterField.hpp>
#endif
#ifndef _STACK_HXX
#include <tools/stack.hxx>
#endif

#include "xmldrani.hxx"
#include "xmldpimp.hxx"
namespace binfilter {

class ScXMLImport;

class ScXMLFilterContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

    ::com::sun::star::uno::Sequence < ::com::sun::star::sheet::TableFilterField> aFilterFields;
    ::com::sun::star::table::CellAddress aOutputPosition;
    ::com::sun::star::table::CellRangeAddress aConditionSourceRangeAddress;
    sal_Int16	nUserListIndex;
    sal_Bool	bSkipDuplicates : 1;
    sal_Bool	bCopyOutputData : 1;
    sal_Bool	bUseRegularExpressions : 1;
    sal_Bool	bIsCaseSensitive : 1;
    sal_Bool	bEnabledUserList : 1;
    sal_Bool	bConnectionOr : 1;
    sal_Bool	bNextConnectionOr : 1;
    sal_Bool	bConditionSourceRange : 1;
    Stack		aConnectionOrStack;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLFilterContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLFilterContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
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
    void AddFilterField (const ::com::sun::star::sheet::TableFilterField aFilterField) { aFilterFields.realloc(aFilterFields.getLength() + 1);
                                                                                aFilterFields[aFilterFields.getLength() - 1] = aFilterField; }
};

class ScXMLAndContext : public SvXMLImportContext
{
    ScXMLFilterContext* pFilterContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLAndContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLFilterContext* pTempFilterContext);

    virtual ~ScXMLAndContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
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

    ScXMLOrContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLFilterContext* pTempFilterContext);

    virtual ~ScXMLOrContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLConditionContext : public SvXMLImportContext
{
    ScXMLFilterContext* pFilterContext;

    ::rtl::OUString sDataType;
    ::rtl::OUString sConditionValue;
    ::rtl::OUString sOperator;
    sal_Int32	nField;
    sal_Bool	bIsCaseSensitive : 1;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLConditionContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLFilterContext* pTempFilterContext);

    virtual ~ScXMLConditionContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    void getOperatorXML(const ::rtl::OUString sTempOperator, ::com::sun::star::sheet::FilterOperator& aFilterOperator, sal_Bool& bUseRegularExpressions) const;
    virtual void EndElement();
};

// Datapilot (Core)

class ScXMLDPFilterContext : public SvXMLImportContext
{
    ScXMLDataPilotTableContext*	pDataPilotTable;

    ScQueryParam	aFilterFields;
    ScAddress		aOutputPosition;
    ScRange			aConditionSourceRangeAddress;
    sal_uInt8	nFilterFieldCount;
    sal_Int16	nUserListIndex;
    sal_Bool	bSkipDuplicates : 1;
    sal_Bool	bCopyOutputData : 1;
    sal_Bool	bUseRegularExpressions : 1;
    sal_Bool	bIsCaseSensitive : 1;
    sal_Bool	bEnabledUserList : 1;
    sal_Bool	bConnectionOr : 1;
    sal_Bool	bNextConnectionOr : 1;
    sal_Bool	bConditionSourceRange : 1;
    Stack		aConnectionOrStack;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLDPFilterContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotTableContext* pTempDataPilotTableContext);

    virtual ~ScXMLDPFilterContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
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

    ScXMLDPAndContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDPFilterContext* pTempFilterContext);

    virtual ~ScXMLDPAndContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
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

    ScXMLDPOrContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDPFilterContext* pTempFilterContext);

    virtual ~ScXMLDPOrContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDPConditionContext : public SvXMLImportContext
{
    ScXMLDPFilterContext* pFilterContext;

    ::rtl::OUString sDataType;
    ::rtl::OUString sConditionValue;
    ::rtl::OUString sOperator;
    sal_Int32	nField;
    sal_Bool	bIsCaseSensitive : 1;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLDPConditionContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDPFilterContext* pTempFilterContext);

    virtual ~ScXMLDPConditionContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    void getOperatorXML(const ::rtl::OUString sTempOperator, ScQueryOp& aFilterOperator, sal_Bool& bUseRegularExpressions,
                        double& dVal) const;
    virtual void EndElement();
};

} //namespace binfilter
#endif
