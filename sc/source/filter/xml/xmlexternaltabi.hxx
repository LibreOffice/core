/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xmlexternaltabi.hxx,v $
 * $Revision: 1.1.2.2 $
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

#ifndef SC_XMLEXTERNALTABI_HXX
#define SC_XMLEXTERNALTABI_HXX

#include <xmloff/xmlictxt.hxx>

class ScXMLImport;
struct ScXMLExternalTabData;

class ScXMLExternalRefTabSourceContext : public SvXMLImportContext
{
public:
    ScXMLExternalRefTabSourceContext( ScXMLImport& rImport, USHORT nPrefix,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLExternalTabData& rRefInfo );

    virtual ~ScXMLExternalRefTabSourceContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
private:
    ScXMLImport&            mrScImport;
    ScXMLExternalTabData&   mrExternalRefInfo;

    ::rtl::OUString         maRelativeUrl;
    ::rtl::OUString         maTableName;
    ::rtl::OUString         maFilterName;
    ::rtl::OUString         maFilterOptions;
};

// ============================================================================

class ScXMLExternalRefRowsContext : public SvXMLImportContext
{
public:
    ScXMLExternalRefRowsContext( ScXMLImport& rImport, USHORT nPrefix,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLExternalTabData& rRefInfo );

    virtual ~ScXMLExternalRefRowsContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
private:
    ScXMLImport&            mrScImport;
    ScXMLExternalTabData&   mrExternalRefInfo;
};

// ============================================================================

class ScXMLExternalRefRowContext : public SvXMLImportContext
{
public:
    ScXMLExternalRefRowContext( ScXMLImport& rImport, USHORT nPrefix,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLExternalTabData& rRefInfo );

    virtual ~ScXMLExternalRefRowContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
private:
    ScXMLImport&            mrScImport;
    ScXMLExternalTabData&   mrExternalRefInfo;
    sal_Int32               mnRepeatRowCount;
};

// ============================================================================

class ScXMLExternalRefCellContext : public SvXMLImportContext
{
public:
    ScXMLExternalRefCellContext( ScXMLImport& rImport, USHORT nPrefix,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLExternalTabData& rRefInfo );

    virtual ~ScXMLExternalRefCellContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

private:
    ScXMLImport&            mrScImport;
    ScXMLExternalTabData&   mrExternalRefInfo;
    ::rtl::OUString         maCellString;
    double                  mfCellValue;
    sal_Int32               mnRepeatCount;
    sal_Int32               mnNumberFormat;
    sal_Int16               mnCellType;
    bool                    mbIsNumeric;
    bool                    mbIsEmpty;
};

// ============================================================================

class ScXMLExternalRefCellTextContext : public SvXMLImportContext
{
public:
    ScXMLExternalRefCellTextContext( ScXMLImport& rImport, USHORT nPrefix,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ::rtl::OUString& rCellString );

    virtual ~ScXMLExternalRefCellTextContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void Characters(const ::rtl::OUString& rChar);

    virtual void EndElement();

private:
    ScXMLImport&            mrScImport;
    ::rtl::OUString&        mrCellString;
};

#endif
