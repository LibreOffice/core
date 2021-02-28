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

#include <xmloff/xmlimp.hxx>
#include <vcl/errcode.hxx>

#include <deque>

class SmNode;
class SfxMedium;
namespace com::sun::star
{
namespace beans
{
class XPropertySet;
}
}

typedef std::deque<std::unique_ptr<SmNode>> SmNodeStack;

class SmXMLImportWrapper
{
    css::uno::Reference<css::frame::XModel> xModel;

private:
    // Use customized entities
    bool m_bUseHTMLMLEntities;

public:
    explicit SmXMLImportWrapper(css::uno::Reference<css::frame::XModel> const& rRef)
        : xModel(rRef)
        , m_bUseHTMLMLEntities(false)
    {
    }

    ErrCode Import(SfxMedium& rMedium);
    void useHTMLMLEntities(bool bUseHTMLMLEntities) { m_bUseHTMLMLEntities = bUseHTMLMLEntities; }

    static ErrCode
    ReadThroughComponent(const css::uno::Reference<css::io::XInputStream>& xInputStream,
                         const css::uno::Reference<css::lang::XComponent>& xModelComponent,
                         css::uno::Reference<css::uno::XComponentContext> const& rxContext,
                         css::uno::Reference<css::beans::XPropertySet> const& rPropSet,
                         const char* pFilterName, bool bEncrypted, bool bUseHTMLMLEntities);

    static ErrCode
    ReadThroughComponent(const css::uno::Reference<css::embed::XStorage>& xStorage,
                         const css::uno::Reference<css::lang::XComponent>& xModelComponent,
                         const char* pStreamName,
                         css::uno::Reference<css::uno::XComponentContext> const& rxContext,
                         css::uno::Reference<css::beans::XPropertySet> const& rPropSet,
                         const char* pFilterName, bool bUseHTMLMLEntities);
};

class SmXMLImport : public SvXMLImport
{
    SmNodeStack aNodeStack;
    bool bSuccess;
    int nParseDepth;
    OUString aText;
    sal_uInt16 mnSmSyntaxVersion;

public:
    SmXMLImport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
                OUString const& implementationName, SvXMLImportFlags nImportFlags);
    virtual ~SmXMLImport() throw() override;

    // XUnoTunnel
    sal_Int64 SAL_CALL getSomething(const css::uno::Sequence<sal_Int8>& rId) override;
    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId() throw();

    void SAL_CALL endDocument() override;

    SvXMLImportContext* CreateFastContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;

    SmNodeStack& GetNodeStack() { return aNodeStack; }

    bool GetSuccess() const { return bSuccess; }
    [[nodiscard]] const OUString& GetText() const { return aText; }
    void SetText(const OUString& rStr) { aText = rStr; }

    virtual void
    SetViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& aViewProps) override;
    virtual void SetConfigurationSettings(
        const css::uno::Sequence<css::beans::PropertyValue>& aViewProps) override;

    void IncParseDepth() { ++nParseDepth; }
    bool TooDeep() const { return nParseDepth >= 2048; }
    void DecParseDepth() { --nParseDepth; }
    void SetSmSyntaxVersion(sal_uInt16 nSmSyntaxVersion) { mnSmSyntaxVersion = nSmSyntaxVersion; }
    sal_uInt16 GetSmSyntaxVersion() const { return mnSmSyntaxVersion; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
