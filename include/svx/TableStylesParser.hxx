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
#pragma once

#include <svx/svxdllapi.h>
#include <bitset>
#include <editeng/borderline.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/table/XMLTableImport.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>

class SvxAutoFormat;
class SvxAutoFormatData;
class SvxAutoFormatDataField;

struct TableStyle
{
    rtl::OUString sParentName;
    std::unique_ptr<SvxAutoFormatDataField> pDataField;
};

class SvxTableStylesImport : public SvXMLImport
{
private:
    SvxAutoFormat& mpAutoFormat;
    std::map<OUString, TableStyle> maCellStyles;
    XMLTableTemplateMap maTableTemplates;
    std::map<OUString, std::bitset<6>> maTableStyles;

public:
    SvxTableStylesImport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
                         SvxAutoFormat& rAutoFormat);
    virtual ~SvxTableStylesImport() override;

    // Override from SvXMLImport
    virtual SvXMLImportContext* CreateFastContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;

    void addCellStyle(const OUString& rName, const OUString& rParentName,
                      std::unique_ptr<SvxAutoFormatDataField> pField);
    const TableStyle* getCellStyle(const OUString& rName);
    void addTableTemplate(const OUString& rsStyleName, const XMLTableTemplate& xTableTemplate,
                          std::bitset<6>& rUseSet);
    SvxAutoFormat& GetFormat() { return mpAutoFormat; }
    void finishStyles();
};

class SvxTableStylesContext : public SvXMLStylesContext
{
private:
    mutable std::unique_ptr<SvXMLImportPropertyMapper> mxTableCellPropMapper;
    SvxTableStylesImport* mpImport;

public:
    SvxTableStylesContext(SvxTableStylesImport& rImport);

    virtual SvXMLStyleContext* CreateStyleChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;

    virtual SvXMLStyleContext* CreateStyleStyleChildContext(
        XmlStyleFamily nFamily, sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;

    virtual SvXMLImportPropertyMapper*
    GetImportPropertyMapper(XmlStyleFamily nFamily) const override;

    virtual void SAL_CALL endFastElement(sal_Int32 Element) override;
};

class SvxTableTemplateContext : public SvXMLStyleContext
{
private:
    XMLTableTemplate maTableTemplate;
    OUString msTemplateName;
    SvxTableStylesImport* mpImport;
    std::bitset<6> maUseSet;

protected:
    virtual void SetAttribute(sal_Int32 nElement, const OUString& rValue) override;

public:
    SvxTableTemplateContext(SvXMLImport& rImport, SvxTableStylesImport& rSvxImport);

    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& AttrList) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

class SvxCellStyleContext : public XMLPropStyleContext
{
private:
    std::unique_ptr<SvxAutoFormatDataField> mpField;
    SvxTableStylesImport* mpImport;

public:
    explicit SvxCellStyleContext(SvXMLImport& rImport, SvxTableStylesContext& rStyles,
                                 SvxTableStylesImport& rSvxImport);

    virtual void Finish(bool bOverwrite) override;

    void setPropertyValue(OUString& rPropName, const css::uno::Any& aValue);
};

class SVX_DLLPUBLIC SvxTableStylesExport : public SvXMLExport
{
private:
    SvxAutoFormat& mpAutoFormat;
    std::map<OUString, std::unique_ptr<SvxAutoFormatDataField>> maCellStyles;

    void exportTableTemplate(const SvxAutoFormatData& rData);
    void exportCellStyle(const SvxAutoFormatDataField& rField, OUString& rStyleName,
                         OUString& rParentName);
    void exportCellProperties(const SvxAutoFormatDataField& rField,
                              const SvxAutoFormatDataField& rParent);
    void exportParaProperties(const SvxAutoFormatDataField& rField,
                              const SvxAutoFormatDataField& rParent);
    void exportTextProperties(const SvxAutoFormatDataField& rField,
                              const SvxAutoFormatDataField& rParent);

public:
    SvxTableStylesExport(
        const css::uno::Reference<css::uno::XComponentContext>& rContext, OUString const& rFileName,
        const css::uno::Reference<com::sun::star::xml::sax::XDocumentHandler>& xHandler,
        SvxAutoFormat& rAutoFormat);

    virtual void ExportAutoStyles_() override {}
    virtual void ExportMasterStyles_() override {}
    virtual void ExportContent_() override {}

    void ExportStyles();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
