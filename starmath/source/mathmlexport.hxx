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

#ifndef INCLUDED_STARMATH_SOURCE_MATHMLEXPORT_HXX
#define INCLUDED_STARMATH_SOURCE_MATHMLEXPORT_HXX

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/DocumentSettingsContext.hxx>
#include <xmloff/xmltoken.hxx>

#include <node.hxx>

class SfxMedium;
namespace com { namespace sun { namespace star {
    namespace io {
        class XOutputStream; }
    namespace beans {
        class XPropertySet; }
} } }


class SmXMLExportWrapper
{
    css::uno::Reference<css::frame::XModel> xModel;
    bool bFlat;     //set true for export to flat .mml, set false for
                        //export to a .sxm (or whatever) package
public:
    explicit SmXMLExportWrapper(css::uno::Reference<css::frame::XModel> &rRef)
        : xModel(rRef), bFlat(true) {}

    bool Export(SfxMedium &rMedium);
    void SetFlat(bool bIn) {bFlat = bIn;}

    static bool WriteThroughComponent(
        const css::uno::Reference< css::io::XOutputStream >&   xOutputStream,
        const css::uno::Reference< css::lang::XComponent >&    xComponent,
        css::uno::Reference< css::uno::XComponentContext > & rxContext,
        css::uno::Reference< css::beans::XPropertySet > & rPropSet,
        const sal_Char* pComponentName );

    static bool WriteThroughComponent(
        const css::uno::Reference< css::embed::XStorage >& xStor,
        const css::uno::Reference< css::lang::XComponent >& xComponent,
        const sal_Char* pStreamName,
        css::uno::Reference< css::uno::XComponentContext > & rxContext,
        css::uno::Reference< css::beans::XPropertySet > & rPropSet,
        const sal_Char* pComponentName );
};


class SmXMLExport : public SvXMLExport
{
    const SmNode *  pTree;
    OUString        aText;
    bool        bSuccess;

protected:
    void ExportNodes(const SmNode *pNode, int nLevel);
    void ExportTable(const SmNode *pNode, int nLevel);
    void ExportLine(const SmNode *pNode, int nLevel);
    void ExportExpression(const SmNode *pNode, int nLevel,
                          bool bNoMrowContainer = false);
    void ExportText(const SmNode *pNode, int nLevel);
    void ExportMath(const SmNode *pNode, int nLevel);
    void ExportBinaryHorizontal(const SmNode *pNode, int nLevel);
    void ExportUnaryHorizontal(const SmNode *pNode, int nLevel);
    void ExportBrace(const SmNode *pNode, int nLevel);
    void ExportBinaryVertical(const SmNode *pNode, int nLevel);
    void ExportBinaryDiagonal(const SmNode *pNode, int nLevel);
    void ExportSubSupScript(const SmNode *pNode, int nLevel);
    void ExportRoot(const SmNode *pNode, int nLevel);
    void ExportOperator(const SmNode *pNode, int nLevel);
    void ExportAttributes(const SmNode *pNode, int nLevel);
    void ExportFont(const SmNode *pNode, int nLevel);
    void ExportVerticalBrace(const SmNode *pNode, int nLevel);
    void ExportMatrix(const SmNode *pNode, int nLevel);
    void ExportBlank(const SmNode *pNode, int nLevel);

public:
    SmXMLExport(
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        OUString const & implementationName, SvXMLExportFlags nExportFlags);
    virtual ~SmXMLExport() {};

    // XUnoTunnel
    sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rId ) throw(css::uno::RuntimeException, std::exception) override;
    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();

    void ExportAutoStyles_() override {}
    void ExportMasterStyles_() override {}
    void ExportContent_() override;
    sal_uInt32 exportDoc(enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID) override;

    virtual void GetViewSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;
    virtual void GetConfigurationSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;

    bool GetSuccess() {return bSuccess;}
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
