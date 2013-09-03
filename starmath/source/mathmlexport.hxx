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

#ifndef _MATHMLEXPORT_HXX_
#define _MATHMLEXPORT_HXX_

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


////////////////////////////////////////////////////////////

class SmXMLExportWrapper
{
    com::sun::star::uno::Reference<com::sun::star::frame::XModel> xModel;
    sal_Bool bFlat;     //set true for export to flat .mml, set false for
                        //export to a .sxm (or whatever) package
public:
    SmXMLExportWrapper(com::sun::star::uno::Reference<com::sun::star::frame::XModel> &rRef)
        : xModel(rRef), bFlat(sal_True) {}

    sal_Bool Export(SfxMedium &rMedium);
    void SetFlat(sal_Bool bIn) {bFlat = bIn;}

    sal_Bool WriteThroughComponent(
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
            xOutputStream,
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
            xComponent,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & rxContext,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const sal_Char* pComponentName );

    sal_Bool WriteThroughComponent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStor,
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComponent,
        const sal_Char* pStreamName,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & rxContext,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const sal_Char* pComponentName );
};

////////////////////////////////////////////////////////////

class SmXMLExport : public SvXMLExport
{
    const SmNode *  pTree;
    OUString        aText;
    sal_Bool        bSuccess;

protected:
    void ExportNodes(const SmNode *pNode, int nLevel);
    void ExportTable(const SmNode *pNode, int nLevel);
    void ExportLine(const SmNode *pNode, int nLevel);
    void ExportExpression(const SmNode *pNode, int nLevel,
                          bool bNoMrowContainer = false);
    void ExportText(const SmNode *pNode, int nLevel);
    void ExportMath(const SmNode *pNode, int nLevel);
    void ExportPolygon(const SmNode *pNode, int nLevel);
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
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
        sal_uInt16 nExportFlags=EXPORT_ALL);
    virtual ~SmXMLExport() {};

    // XServiceInfo (override parent method)
    OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );

    // XUnoTunnel
    sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw(::com::sun::star::uno::RuntimeException);
    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();

    void _ExportAutoStyles() {}
    void _ExportMasterStyles() {}
    void _ExportContent();
    sal_uInt32 exportDoc(enum ::xmloff::token::XMLTokenEnum eClass);

    virtual void GetViewSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps);
    virtual void GetConfigurationSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps);

    sal_Bool GetSuccess() {return bSuccess;}
};

////////////////////////////////////////////////////////////

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
