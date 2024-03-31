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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLFILTER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLFILTER_HXX

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <map>
#include <memory>
#include <xmloff/prhdlfac.hxx>
#include <xmloff/xmlprmap.hxx>

namespace rptui
{
class OReportModel;
}
namespace rptxml
{
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::report;
using namespace ::com::sun::star::xml::sax;


class ORptFilter : public SvXMLImport
{
public:
    typedef std::map< OUString, Sequence<PropertyValue> > TPropertyNameMap;
    typedef std::map< OUString, Reference<XFunction> > TGroupFunctionMap;
private:

    TGroupFunctionMap                               m_aFunctions;

    rtl::Reference < XMLPropertyHandlerFactory >      m_xPropHdlFactory;
    rtl::Reference < XMLPropertySetMapper >           m_xCellStylesPropertySetMapper;
    rtl::Reference < XMLPropertySetMapper >           m_xColumnStylesPropertySetMapper;
    rtl::Reference < XMLPropertySetMapper >           m_xRowStylesPropertySetMapper;

    Reference<XReportDefinition>                    m_xReportDefinition;
    std::shared_ptr<rptui::OReportModel>        m_pReportModel;

    /// @throws RuntimeException
    bool                            implImport( const Sequence< PropertyValue >& rDescriptor );

public:
    using SvXMLImport::SetMasterStyles;
    SvXMLImportContext* CreateStylesContext( bool bIsAutoStyle );
    SvXMLImportContext* CreateMetaContext(const sal_Int32 nElement);
    SvXMLImportContext* CreateFontDeclsContext();
protected:
    // SvXMLImport
    virtual SvXMLImportContext *CreateFastContext( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual XMLShapeImportHelper* CreateShapeImport() override;

    virtual ~ORptFilter()  noexcept override;
public:

    ORptFilter( const Reference< XComponentContext >& _rxContext, OUString const & rImplementationName, SvXMLImportFlags nImportFlags );

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& rDescriptor ) override;

    const Reference<XReportDefinition>& getReportDefinition() const {
        return m_xReportDefinition;
    }
    void FinishStyles();

    virtual void SAL_CALL startDocument() override;
    virtual void SAL_CALL endDocument() override;

    const rtl::Reference< XMLPropertySetMapper >& GetCellStylesPropertySetMapper()      const   {
        return m_xCellStylesPropertySetMapper;
    }
    const rtl::Reference< XMLPropertySetMapper >& GetColumnStylesPropertySetMapper()    const   {
        return m_xColumnStylesPropertySetMapper;
    }
    const rtl::Reference< XMLPropertySetMapper >& GetRowStylesPropertySetMapper()       const   {
        return m_xRowStylesPropertySetMapper;
    }
    static const OUString& convertFormula(const OUString& _sFormula);
    /** inserts a new function
    *
    * \param _xFunction
    */
    void insertFunction(const css::uno::Reference< css::report::XFunction > & _xFunction);
    void removeFunction(const OUString& _sFunctionName);
    const TGroupFunctionMap& getFunctions() const {
        return m_aFunctions;
    }

    bool                        isOldFormat() const;
};


class RptXMLDocumentBodyContext : public SvXMLImportContext
{
public:
    RptXMLDocumentBodyContext(SvXMLImport & rImport)
        : SvXMLImportContext(rImport)
    {
    }

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
            sal_Int32 /*nElement*/, const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ ) override;
};

} // rptxml

#endif // INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
