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

#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltoken.hxx>

class SfxMedium;
class SmNode;
class SmVerticalBraceNode;
namespace com::sun::star
{
namespace io
{
class XOutputStream;
}
namespace beans
{
class XPropertySet;
}
}

class SmXMLExportWrapper
{
    css::uno::Reference<css::frame::XModel> xModel;
    bool bFlat; //set true for export to flat .mml, set false for
        //export to a .sxm (or whatever) package

private:
    // Use customized entities
    bool m_bUseHTMLMLEntities;

public:
    explicit SmXMLExportWrapper(css::uno::Reference<css::frame::XModel> const& rRef)
        : xModel(rRef)
        , bFlat(true)
        , m_bUseHTMLMLEntities(false)
    {
    }

    bool Export(SfxMedium& rMedium);
    void SetFlat(bool bIn) { bFlat = bIn; }
    void useHTMLMLEntities(bool bUseHTMLMLEntities) { m_bUseHTMLMLEntities = bUseHTMLMLEntities; }

    static bool
    WriteThroughComponent(const css::uno::Reference<css::io::XOutputStream>& xOutputStream,
                          const css::uno::Reference<css::lang::XComponent>& xComponent,
                          css::uno::Reference<css::uno::XComponentContext> const& rxContext,
                          css::uno::Reference<css::beans::XPropertySet> const& rPropSet,
                          const char* pComponentName, bool bUseHTMLMLEntities);

    static bool
    WriteThroughComponent(const css::uno::Reference<css::embed::XStorage>& xStor,
                          const css::uno::Reference<css::lang::XComponent>& xComponent,
                          const char* pStreamName,
                          css::uno::Reference<css::uno::XComponentContext> const& rxContext,
                          css::uno::Reference<css::beans::XPropertySet> const& rPropSet,
                          const char* pComponentName, bool bUseHTMLMLEntities);
};

class SmXMLExport final : public SvXMLExport
{
    const SmNode* pTree;
    OUString aText;
    bool bSuccess;

public:
    SmXMLExport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
                OUString const& implementationName, SvXMLExportFlags nExportFlags);

    // XUnoTunnel
    sal_Int64 SAL_CALL getSomething(const css::uno::Sequence<sal_Int8>& rId) override;
    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId() throw();

    void ExportAutoStyles_() override {}
    void ExportMasterStyles_() override {}
    void ExportContent_() override;
    ErrCode exportDoc(enum ::xmloff::token::XMLTokenEnum eClass
                      = ::xmloff::token::XML_TOKEN_INVALID) override;

    virtual void GetViewSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;
    virtual void
    GetConfigurationSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;

    bool GetSuccess() const { return bSuccess; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
