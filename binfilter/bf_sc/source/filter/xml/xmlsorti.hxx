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
#ifndef SC_XMLSORTI_HXX
#define SC_XMLSORTI_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <bf_xmloff/xmlictxt.hxx>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <bf_xmloff/xmlimp.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_SORTFIELD_HPP_
#include <com/sun/star/util/SortField.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif

#include "xmldrani.hxx"
namespace binfilter {

class ScXMLImport;

class ScXMLSortContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

    ::com::sun::star::uno::Sequence < ::com::sun::star::util::SortField> aSortFields;
    ::com::sun::star::table::CellAddress aOutputPosition;
    ::rtl::OUString	sCountry;
    ::rtl::OUString	sLanguage;
    ::rtl::OUString	sAlgorithm;
    sal_Int16	nUserListIndex;
    sal_Bool	bCopyOutputData : 1;
    sal_Bool	bBindFormatsToContent : 1;
    sal_Bool	bIsCaseSensitive : 1;
    sal_Bool	bEnabledUserList : 1;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSortContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSortContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

    void AddSortField(const ::rtl::OUString& sFieldNumber, const ::rtl::OUString& sDataType, const ::rtl::OUString& sOrder);
};

class ScXMLSortByContext : public SvXMLImportContext
{
    ScXMLSortContext* pSortContext;

    ::rtl::OUString	sFieldNumber;
    ::rtl::OUString	sDataType;
    ::rtl::OUString	sOrder;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSortByContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLSortContext* pTempSortContext);

    virtual ~ScXMLSortByContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

} //namespace binfilter
#endif
