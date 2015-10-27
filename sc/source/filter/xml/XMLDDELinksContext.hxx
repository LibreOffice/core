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

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLDDELINKSCONTEXT_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLDDELINKSCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include "xmlimprt.hxx"
#include <list>

class ScXMLDDELinksContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLDDELinksContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLDDELinksContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

struct ScDDELinkCell
{
    OUString sValue;
    double fValue;
    bool bString;
    bool bEmpty;
};

typedef std::list<ScDDELinkCell> ScDDELinkCells;

class ScXMLDDELinkContext : public SvXMLImportContext
{
    ScDDELinkCells  aDDELinkTable;
    ScDDELinkCells  aDDELinkRow;
    OUString   sApplication;
    OUString   sTopic;
    OUString   sItem;
    sal_Int32       nPosition;
    sal_Int32       nColumns;
    sal_Int32       nRows;
    sal_uInt8       nMode;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLDDELinkContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLDDELinkContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    void SetApplication(const OUString& sValue) { sApplication = sValue; }
    void SetTopic(const OUString& sValue) { sTopic = sValue; }
    void SetItem(const OUString& sValue) { sItem = sValue; }
    void SetMode(const sal_uInt8 nValue) { nMode = nValue; }
    void CreateDDELink();
    void AddColumns(const sal_Int32 nValue) { nColumns += nValue; }
    void AddRows(const sal_Int32 nValue) { nRows += nValue; }
    void AddCellToRow(const ScDDELinkCell& aCell);
    void AddRowsToTable(const sal_Int32 nRows);

    virtual void EndElement() override;
};

class ScXMLDDESourceContext : public SvXMLImportContext
{
    ScXMLDDELinkContext* pDDELink;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLDDESourceContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDESourceContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLDDETableContext : public SvXMLImportContext
{
    ScXMLDDELinkContext* pDDELink;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLDDETableContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDETableContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLDDEColumnContext : public SvXMLImportContext
{
    ScXMLDDELinkContext* pDDELink;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLDDEColumnContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDEColumnContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLDDERowContext : public SvXMLImportContext
{
    ScXMLDDELinkContext*    pDDELink;
    sal_Int32               nRows;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLDDERowContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDERowContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLDDECellContext : public SvXMLImportContext
{
    OUString        sValue;
    double          fValue;
    sal_Int32       nCells;
    bool            bString;
    bool            bString2;
    bool            bEmpty;

    ScXMLDDELinkContext* pDDELink;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLDDECellContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDECellContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
