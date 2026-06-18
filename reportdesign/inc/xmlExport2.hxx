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

#include "xmlExport.hxx"
#include <com/sun/star/sheet/XFormulaParser.hpp>
#include "reportformula.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>

namespace rptxml
{
using namespace com::sun::star::sdbc;

class ORptExecuteExport : public ORptExport
{
public:
    ORptExecuteExport(const Reference<XComponentContext>& _rxContext,
                      OUString const& implementationName, SvXMLExportFlags nExportFlag);

protected:
    Reference<css::sheet::XFormulaParser> m_xFormulaParser;
    Reference<XRow> m_xRow;
    Reference<XResultSet> m_xResultSet;

    void exportGroupsExpressionAsFunction(const Reference<XGroups>& _xGroups);
    static OUString getColumnNameString(const Reference<XReportDefinition>& _xReportDefinition);
    static OUString getStringFromAny(Any& aAnswer);
    static Any callFunction(const Sequence<Any>& aArgs, const OUString& sFuncName);
    static sal_Int32 getColumnNum(std::u16string_view sColumnName, Reference<XRow>& xRow);
    bool exportFormula(const OUString& _sFormula, rptui::ReportFormula& aFormula);
    virtual void exportReport(const Reference<XReportDefinition>& _xReportDefinition) override;
    virtual void exportStyleName(XPropertySet* _xProp, comphelper::AttributeList& _rAtt,
                                 const OUString& _sName) override;
    virtual void
    exportReportElement(const Reference<XReportControlModel>& _xReportElement) override;
    virtual void exportGroup(const Reference<XReportDefinition>& _xReportDefinition,
                             sal_Int32 _nPos, bool _bExportAutoStyle = false) override;
    virtual void exportSection(const Reference<XSection>& _xProp, bool bHeader = false) override;
    virtual void exportComponent(const Reference<XReportComponent>& _xReportComponent) override;
    virtual void handleTextElement(const Reference<XServiceInfo>& xElement, bool bShapeHandled,
                                   const Reference<XSection>& _xSection) override;
    virtual void ExportContent_() override;

public:
    const rtl::Reference<XMLPropertySetMapper>& GetCellStylePropertyMapper() const
    {
        return m_xCellStylesPropertySetMapper;
    }

    // Helper methods to create exporters
    static rtl::Reference<ORptExecuteExport>
    createSettingsExporter(const Reference<XComponentContext>& rxContext);
    static rtl::Reference<ORptExecuteExport>
    createStylesExporter(const Reference<XComponentContext>& rxContext);
    static rtl::Reference<ORptExecuteExport>
    createMetaExporter(const Reference<XComponentContext>& rxContext);
    static rtl::Reference<ORptExecuteExport>
    createExportFilter(const Reference<XComponentContext>& rxContext);
};

} // rptxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
