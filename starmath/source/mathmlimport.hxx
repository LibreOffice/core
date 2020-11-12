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

#ifndef INCLUDED_STARMATH_SOURCE_MATHMLIMPORT_HXX
#define INCLUDED_STARMATH_SOURCE_MATHMLIMPORT_HXX

#include <xmloff/xmlimp.hxx>
#include <vcl/errcode.hxx>

#include <deque>
#include <memory>

class SmNode;
class SfxMedium;
namespace com::sun::star {
    namespace beans {
        class XPropertySet; }
}


typedef std::deque<std::unique_ptr<SmNode>> SmNodeStack;

class SmXMLImportWrapper
{
    css::uno::Reference<css::frame::XModel> xModel;

public:
    explicit SmXMLImportWrapper(css::uno::Reference<css::frame::XModel> const &rRef)
        : xModel(rRef) {}

    ErrCode Import(SfxMedium &rMedium);

    static ErrCode ReadThroughComponent(
        const css::uno::Reference< css::io::XInputStream >& xInputStream,
        const css::uno::Reference< css::lang::XComponent >& xModelComponent,
        css::uno::Reference< css::uno::XComponentContext > const & rxContext,
        css::uno::Reference< css::beans::XPropertySet > const & rPropSet,
        const char* pFilterName,
        bool bEncrypted );

    static ErrCode ReadThroughComponent(
        const css::uno::Reference< css::embed::XStorage >& xStorage,
        const css::uno::Reference< css::lang::XComponent >& xModelComponent,
        const char* pStreamName,
        css::uno::Reference< css::uno::XComponentContext > const & rxContext,
        css::uno::Reference< css::beans::XPropertySet > const & rPropSet,
        const char* pFilterName );
};


class SmXMLImport : public SvXMLImport
{
        SmNodeStack aNodeStack;
        bool bSuccess;
        int nParseDepth;
        OUString aText;

public:
    SmXMLImport(
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        OUString const & implementationName, SvXMLImportFlags nImportFlags);
    virtual ~SmXMLImport() throw () override;

    // XUnoTunnel
    sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rId ) override;
    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();

    void SAL_CALL endDocument() override;

    SvXMLImportContext *CreateFastContext( sal_Int32 nElement,
        const css::uno::Reference<
        css::xml::sax::XFastAttributeList >& xAttrList ) override;

    SmNodeStack & GetNodeStack()    { return aNodeStack; }

    bool GetSuccess() const              { return bSuccess; }
    [[nodiscard]] const OUString& GetText() const { return aText; }
    void SetText(const OUString &rStr) { aText = rStr; }

    virtual void SetViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& aViewProps) override;
    virtual void SetConfigurationSettings(const css::uno::Sequence<css::beans::PropertyValue>& aViewProps) override;

    void IncParseDepth() { ++nParseDepth; }
    bool TooDeep() const { return nParseDepth >= 2048; }
    void DecParseDepth() { --nParseDepth; }
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
