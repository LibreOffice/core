/*************************************************************************
 *
 *  $RCSfile: xmldrani.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-06 16:43:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef SC_XMLDRANI_HXX
#define SC_XMLDRANI_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAIMPORTMODE_HPP_
#include <com/sun/star/sheet/DataImportMode.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_SUBTOTALCOLUMN_HPP_
#include <com/sun/star/sheet/SubTotalColumn.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_TABLEFILTERFIELD_HPP_
#include <com/sun/star/sheet/TableFilterField.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif

class ScXMLImport;

class ScXMLDatabaseRangesContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLDatabaseRangesContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLDatabaseRangesContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDatabaseRangeContext : public SvXMLImportContext
{
    rtl::OUString   sDatabaseRangeName;
    rtl::OUString   sRangeAddress;
    rtl::OUString   sDatabaseName;
    rtl::OUString   sSourceObject;
    com::sun::star::uno::Sequence <com::sun::star::sheet::SubTotalColumn> aSubTotalColumns;
    com::sun::star::uno::Sequence <com::sun::star::beans::PropertyValue> aSortSequence;
    com::sun::star::uno::Sequence <com::sun::star::sheet::TableFilterField> aFilterFields;
    com::sun::star::table::CellAddress aFilterOutputPosition;
    com::sun::star::table::CellRangeAddress aFilterConditionSourceRangeAddress;
    com::sun::star::sheet::DataImportMode nSourceType;
    sal_Int16       nSubTotalsUserListIndex;
    sal_Int16       nSubTotalRuleGroupFieldNumber;
    sal_Bool        bNative : 1;
    sal_Bool        bIsSelection : 1;
    sal_Bool        bKeepFormats : 1;
    sal_Bool        bMoveCells : 1;
    sal_Bool        bStripData : 1;
    sal_Bool        bOrientation : 1;
    sal_Bool        bContainsHeader : 1;
    sal_Bool        bAutoFilter : 1;
    sal_Bool        bSubTotalsBindFormatsToContent : 1;
    sal_Bool        bSubTotalsIsCaseSensitive : 1;
    sal_Bool        bSubTotalsInsertPageBreaks : 1;
    sal_Bool        bSubTotalsSortGroups : 1;
    sal_Bool        bSubTotalsEnabledUserList : 1;
    sal_Bool        bSubTotalsAscending : 1;
    sal_Bool        bFilterCopyOutputData : 1;
    sal_Bool        bFilterIsCaseSensitive : 1;
    sal_Bool        bFilterSkipDuplicates : 1;
    sal_Bool        bFilterUseRegularExpressions : 1;
    sal_Bool        bFilterConditionSourceRange : 1;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLDatabaseRangeContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLDatabaseRangeContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

    void SetDatabaseName(const rtl::OUString sTempDatabaseName) { sDatabaseName = sTempDatabaseName; }
    void SetSourceObject(const rtl::OUString sTempSourceObject) { sSourceObject = sTempSourceObject; }
    void SetSourceType(const com::sun::star::sheet::DataImportMode nTempSourceType) { nSourceType = nTempSourceType; }
    void SetNative(const sal_Bool bTempNative) { bNative = bTempNative; }
    void SetSubTotalsBindFormatsToContent(const sal_Bool bTemp ) { bSubTotalsBindFormatsToContent = bTemp; }
    void SetSubTotalsIsCaseSensitive(const sal_Bool bTemp) { bSubTotalsIsCaseSensitive = bTemp; }
    void SetSubTotalsInsertPageBreaks(const sal_Bool bTemp) { bSubTotalsInsertPageBreaks = bTemp; }
    void SetSubTotalsEnabledUserList(const sal_Bool bTemp) { bSubTotalsEnabledUserList = bTemp; }
    void SetSubTotalsUserListIndex(const sal_Int16 nTemp) { nSubTotalsUserListIndex = nTemp; }
    void SetSubTotalsAscending(const sal_Bool bTemp) { bSubTotalsAscending = bTemp; }
    void SetSubTotalsSortGroups(const sal_Bool bTemp) { bSubTotalsSortGroups = bTemp; }
    void SetSubTotalRuleGroupFieldNumber(const sal_Int16 nTemp) { nSubTotalRuleGroupFieldNumber = nTemp; }
    void AddSubTotalColumn(const com::sun::star::sheet::SubTotalColumn aSubTotalColumn)
        { aSubTotalColumns.realloc(aSubTotalColumns.getLength() + 1); aSubTotalColumns[aSubTotalColumns.getLength() - 1] = aSubTotalColumn; }
    void SetSortSequence(const com::sun::star::uno::Sequence <com::sun::star::beans::PropertyValue>& aTempSortSequence) { aSortSequence = aTempSortSequence; }
    void SetFilterCopyOutputData(const sal_Bool bTemp) { bFilterCopyOutputData = bTemp; }
    void SetFilterIsCaseSensitive(const sal_Bool bTemp) { bFilterIsCaseSensitive = bTemp; }
    void SetFilterSkipDuplicates(const sal_Bool bTemp) { bFilterSkipDuplicates = bTemp; }
    void SetFilterUseRegularExpressions(const sal_Bool bTemp) { bFilterUseRegularExpressions = bTemp; }
    void SetFilterFields(const com::sun::star::uno::Sequence <com::sun::star::sheet::TableFilterField>& aTemp) { aFilterFields = aTemp; }
    void SetFilterOutputPosition(const com::sun::star::table::CellAddress& aTemp) { aFilterOutputPosition = aTemp; }
    void SetFilterConditionSourceRangeAddress(const com::sun::star::table::CellRangeAddress& aTemp) { aFilterConditionSourceRangeAddress = aTemp;
                                                                                                    bFilterConditionSourceRange = sal_True; }
};

class ScXMLSourceSQLContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSourceSQLContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSourceSQLContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLSourceTableContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSourceTableContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSourceTableContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLSourceQueryContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSourceQueryContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSourceQueryContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLSubTotalRulesContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSubTotalRulesContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSubTotalRulesContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLSortGroupsContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSortGroupsContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSortGroupsContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLSubTotalRuleContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSubTotalRuleContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSubTotalRuleContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLSubTotalFieldContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;
    rtl::OUString sFieldNumber;
    rtl::OUString sFunction;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSubTotalFieldContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSubTotalFieldContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

#endif
