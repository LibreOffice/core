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

#ifndef _SC_XMLDDELINKSCONTEXT_HXX
#define _SC_XMLDDELINKSCONTEXT_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <bf_xmloff/xmlictxt.hxx>
#endif

#ifndef __SGI_STL_LIST
#include <list>
#endif
namespace binfilter {

class ScXMLImport;

class ScXMLDDELinksContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLDDELinksContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLDDELinksContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

struct ScDDELinkCell
{
    ::rtl::OUString sValue;
    double fValue;
    sal_Bool bString : 1;
    sal_Bool bEmpty : 1;
};

typedef std::list<ScDDELinkCell> ScDDELinkCells;

class ScXMLDDELinkContext : public SvXMLImportContext
{
    ScDDELinkCells	aDDELinkTable;
    ScDDELinkCells	aDDELinkRow;
    ::rtl::OUString	sApplication;
    ::rtl::OUString	sTopic;
    ::rtl::OUString	sItem;
    sal_Int32		nPosition;
    sal_Int32		nColumns;
    sal_Int32		nRows;
    sal_uInt8		nMode;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLDDELinkContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLDDELinkContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    void SetApplication(const ::rtl::OUString& sValue) { sApplication = sValue; }
    void SetTopic(const ::rtl::OUString& sValue) { sTopic = sValue; }
    void SetItem(const ::rtl::OUString& sValue) { sItem = sValue; }
    void SetMode(const sal_uInt8 nValue) { nMode = nValue; }
    void CreateDDELink();
    void AddColumns(const sal_Int32 nValue) { nColumns += nValue; }
    void AddRows(const sal_Int32 nValue) { nRows += nValue; }
    void AddCellToRow(const ScDDELinkCell& aCell);
    void AddRowsToTable(const sal_Int32 nRows);

    virtual void EndElement();
};

class ScXMLDDESourceContext : public SvXMLImportContext
{
    ScXMLDDELinkContext* pDDELink;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLDDESourceContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDESourceContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDDETableContext : public SvXMLImportContext
{
    ScXMLDDELinkContext* pDDELink;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLDDETableContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDETableContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDDEColumnContext : public SvXMLImportContext
{
    ScXMLDDELinkContext* pDDELink;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLDDEColumnContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDEColumnContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDDERowContext : public SvXMLImportContext
{
    ScXMLDDELinkContext*	pDDELink;
    sal_Int32 				nRows;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLDDERowContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDERowContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDDECellContext : public SvXMLImportContext
{
    ::rtl::OUString	sValue;
    double			fValue;
    sal_Int32		nCells;
    sal_Bool		bString : 1;
    sal_Bool		bString2 : 1;
    sal_Bool		bEmpty : 1;

    ScXMLDDELinkContext* pDDELink;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLDDECellContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDECellContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

} //namespace binfilter
#endif
