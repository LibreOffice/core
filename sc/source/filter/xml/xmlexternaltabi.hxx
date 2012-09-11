/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_XMLEXTERNALTABI_HXX
#define SC_XMLEXTERNALTABI_HXX

#include <xmloff/xmlictxt.hxx>
#include "rtl/ustrbuf.hxx"

class ScXMLImport;
struct ScXMLExternalTabData;

class ScXMLExternalRefTabSourceContext : public SvXMLImportContext
{
public:
    ScXMLExternalRefTabSourceContext( ScXMLImport& rImport, sal_uInt16 nPrefix,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLExternalTabData& rRefInfo );

    virtual ~ScXMLExternalRefTabSourceContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
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
    ScXMLExternalRefRowsContext( ScXMLImport& rImport, sal_uInt16 nPrefix,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLExternalTabData& rRefInfo );

    virtual ~ScXMLExternalRefRowsContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
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
    ScXMLExternalRefRowContext( ScXMLImport& rImport, sal_uInt16 nPrefix,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLExternalTabData& rRefInfo );

    virtual ~ScXMLExternalRefRowContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
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
    ScXMLExternalRefCellContext( ScXMLImport& rImport, sal_uInt16 nPrefix,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLExternalTabData& rRefInfo );

    virtual ~ScXMLExternalRefCellContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

    void SetCellString(const ::rtl::OUString& rStr);

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
    ScXMLExternalRefCellTextContext( ScXMLImport& rImport, sal_uInt16 nPrefix,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLExternalRefCellContext& rParent );

    virtual ~ScXMLExternalRefCellTextContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void Characters(const ::rtl::OUString& rChar);

    virtual void EndElement();

private:
    ScXMLExternalRefCellContext& mrParent;

    ::rtl::OUStringBuffer   maCellStrBuf;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
