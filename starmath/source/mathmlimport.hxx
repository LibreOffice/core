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
#include <xmloff/xmlexp.hxx>
#include <xmloff/DocumentSettingsContext.hxx>
#include <xmloff/xmltoken.hxx>

#include <node.hxx>
#include <memory>

class SfxMedium;
namespace com { namespace sun { namespace star {
    namespace io {
        class XIutputStream; }
    namespace beans {
        class XPropertySet; }
} } }



class SmXMLImportWrapper
{
    css::uno::Reference<css::frame::XModel> xModel;

public:
    explicit SmXMLImportWrapper(css::uno::Reference<css::frame::XModel> &rRef)
        : xModel(rRef) {}

    sal_uLong Import(SfxMedium &rMedium);

    static sal_uLong ReadThroughComponent(
        css::uno::Reference< css::io::XInputStream > xInputStream,
        css::uno::Reference< css::lang::XComponent > xModelComponent,
        css::uno::Reference< css::uno::XComponentContext > & rxContext,
        css::uno::Reference< css::beans::XPropertySet > & rPropSet,
        const sal_Char* pFilterName,
        bool bEncrypted );

    static sal_uLong ReadThroughComponent(
        const css::uno::Reference< css::embed::XStorage >& xStorage,
        css::uno::Reference< css::lang::XComponent > xModelComponent,
        const sal_Char* pStreamName,
        const sal_Char* pCompatibilityStreamName,
        css::uno::Reference< css::uno::XComponentContext > & rxContext,
        css::uno::Reference< css::beans::XPropertySet > & rPropSet,
        const sal_Char* pFilterName );
};



class SmXMLImport : public SvXMLImport
{
    std::unique_ptr<SvXMLTokenMap> pPresLayoutElemTokenMap;
    std::unique_ptr<SvXMLTokenMap> pPresLayoutAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> pFencedAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> pOperatorAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> pAnnotationAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> pPresElemTokenMap;
    std::unique_ptr<SvXMLTokenMap> pPresScriptEmptyElemTokenMap;
    std::unique_ptr<SvXMLTokenMap> pPresTableElemTokenMap;
    std::unique_ptr<SvXMLTokenMap> pColorTokenMap;
    std::unique_ptr<SvXMLTokenMap> pActionAttrTokenMap;

        SmNodeStack aNodeStack;
        bool bSuccess;
        OUString aText;

public:
    SmXMLImport(
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        OUString const & implementationName, SvXMLImportFlags nImportFlags);
    virtual ~SmXMLImport() throw ();

    // XUnoTunnel
    sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rId ) throw(css::uno::RuntimeException, std::exception) override;
    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();

    void SAL_CALL endDocument()
        throw( css::xml::sax::SAXException,
        css::uno::RuntimeException, std::exception ) override;

    SvXMLImportContext *CreateContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList) override;
    SvXMLImportContext *CreateRowContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateEncloseContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateFracContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateNumberContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateTextContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateAnnotationContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateStringContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateIdentifierContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateOperatorContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateSpaceContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateSqrtContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateRootContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateStyleContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreatePaddedContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreatePhantomContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateFencedContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateErrorContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateSubContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateSupContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateSubSupContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateUnderContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateOverContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateUnderOverContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateMultiScriptsContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateNoneContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreatePrescriptsContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateTableContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateTableRowContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateTableCellContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateAlignGroupContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateActionContext(sal_uInt16 nPrefix,
        const OUString &rLocalName,
        const css::uno::Reference <
        css::xml::sax::XAttributeList> &xAttrList);

    const SvXMLTokenMap &GetPresLayoutElemTokenMap();
    const SvXMLTokenMap &GetPresLayoutAttrTokenMap();
    const SvXMLTokenMap &GetFencedAttrTokenMap();
    const SvXMLTokenMap &GetOperatorAttrTokenMap();
    const SvXMLTokenMap &GetAnnotationAttrTokenMap();
    const SvXMLTokenMap &GetPresElemTokenMap();
    const SvXMLTokenMap &GetPresScriptEmptyElemTokenMap();
    const SvXMLTokenMap &GetPresTableElemTokenMap();
    const SvXMLTokenMap &GetColorTokenMap();
    const SvXMLTokenMap &GetActionAttrTokenMap();

    SmNodeStack & GetNodeStack()    { return aNodeStack; }
    SmNode *GetTree()
    {
        return popOrZero(aNodeStack);
    }

    bool GetSuccess()              { return bSuccess; }
    SAL_WARN_UNUSED_RESULT OUString GetText() { return aText; }
    void SetText(const OUString &rStr) { aText = rStr; }

    virtual void SetViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& aViewProps) override;
    virtual void SetConfigurationSettings(const css::uno::Sequence<css::beans::PropertyValue>& aViewProps) override;
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
    XML_TOK_MATHCOLOR
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


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
