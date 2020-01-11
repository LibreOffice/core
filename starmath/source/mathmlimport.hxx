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
namespace com { namespace sun { namespace star {
    namespace beans {
        class XPropertySet; }
} } }


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
        const char* pCompatibilityStreamName,
        css::uno::Reference< css::uno::XComponentContext > const & rxContext,
        css::uno::Reference< css::beans::XPropertySet > const & rPropSet,
        const char* pFilterName );
};


class SmXMLImport : public SvXMLImport
{
    std::unique_ptr<SvXMLTokenMap> pColorTokenMap;

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

    const SvXMLTokenMap &GetColorTokenMap();

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


enum SmXMLMathElemTokenMap
{
    XML_TOK_MATH
};

enum SmXMLPresLayoutElemTokenMap
{
    XML_TOK_SEMANTICS,
    XML_TOK_MSTYLE,
    XML_TOK_MERROR,
    XML_TOK_MPHANTOM,
    XML_TOK_MROW,
    XML_TOK_MENCLOSE,
    XML_TOK_MFRAC,
    XML_TOK_MSQRT,
    XML_TOK_MROOT,
    XML_TOK_MSUB,
    XML_TOK_MSUP,
    XML_TOK_MSUBSUP,
    XML_TOK_MMULTISCRIPTS,
    XML_TOK_MUNDER,
    XML_TOK_MOVER,
    XML_TOK_MUNDEROVER,
    XML_TOK_MTABLE,
    XML_TOK_MACTION,
    XML_TOK_MFENCED,
    XML_TOK_MPADDED
};

enum SmXMLPresLayoutAttrTokenMap
{
    XML_TOK_FONTWEIGHT,
    XML_TOK_FONTSTYLE,
    XML_TOK_FONTSIZE,
    XML_TOK_FONTFAMILY,
    XML_TOK_COLOR,
    XML_TOK_MATHCOLOR,
    XML_TOK_MATHVARIANT
};


enum SmXMLFencedAttrTokenMap
{
    XML_TOK_OPEN,
    XML_TOK_CLOSE
};


enum SmXMLPresTableElemTokenMap
{
    XML_TOK_MTR,
    XML_TOK_MTD
};

enum SmXMLPresElemTokenMap
{
    XML_TOK_ANNOTATION,
    XML_TOK_MI,
    XML_TOK_MN,
    XML_TOK_MO,
    XML_TOK_MTEXT,
    XML_TOK_MSPACE,
    XML_TOK_MS,
    XML_TOK_MALIGNGROUP
};

enum SmXMLPresScriptEmptyElemTokenMap
{
    XML_TOK_MPRESCRIPTS,
    XML_TOK_NONE
};

enum SmXMLOperatorAttrTokenMap
{
    XML_TOK_STRETCHY
};

enum SmXMLAnnotationAttrTokenMap
{
    XML_TOK_ENCODING
};

enum SmXMLActionAttrTokenMap
{
    XML_TOK_SELECTION
};

enum SmXMLMspaceAttrTokenMap
{
    XML_TOK_WIDTH
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
