/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
        class XInputStream;
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
            ::com::sun::star::lang::XMultiServiceFactory > & rFactory,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const sal_Char* pComponentName );

    sal_Bool WriteThroughComponent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStor,
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
            xComponent,
        const sal_Char* pStreamName,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & rFactory,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const sal_Char* pComponentName,
        sal_Bool bCompress=sal_True );
};

////////////////////////////////////////////////////////////

class SmXMLExport : public SvXMLExport
{
    const SmNode *  pTree;
    String          aText;
    sal_Bool        bSuccess;

protected:
    void ExportNodes(const SmNode *pNode, int nLevel);
    void ExportTable(const SmNode *pNode, int nLevel);
    void ExportLine(const SmNode *pNode, int nLevel);
    void ExportExpression(const SmNode *pNode, int nLevel);
    void ExportText(const SmNode *pNode, int nLevel);
    void ExportMath(const SmNode *pNode, int nLevel);
    void ExportPolygon(const SmNode *pNode, int nLevel);
    void ExportBinaryHorizontal(const SmNode *pNode, int nLevel);
    void ExportUnaryHorizontal(const SmNode *pNode, int nLevel);
    void ExportBrace(const SmNode *pNode, int nLevel);
    void ExportBinaryVertical(const SmNode *pNode, int nLevel);
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
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        sal_uInt16 nExportFlags=EXPORT_ALL);
    virtual ~SmXMLExport() {};

    // XServiceInfo (override parent method)
    ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );

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
