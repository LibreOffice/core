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
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <osl/diagnose.h>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
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
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::report;
using namespace ::com::sun::star::xml::sax;


class ORptFilter : public SvXMLImport
{
public:
    typedef std::map< OUString, Sequence<PropertyValue> > TPropertyNameMap;
    typedef std::map< OUString, Reference<XFunction> > TGroupFunctionMap;
private:

    TGroupFunctionMap                               m_aFunctions;

    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pDocElemTokenMap;
    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pDocContentElemTokenMap;
    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pReportElemTokenMap;
    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pGroupElemTokenMap;
    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pSectionElemTokenMap;
    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pComponentElemTokenMap;
    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pElemTokenMap;
    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pControlElemTokenMap;
    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pFunctionElemTokenMap;
    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pSubDocumentElemTokenMap;
    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pFormatElemTokenMap;
    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pColumnTokenMap;
    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pCellElemTokenMap;

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
    SvXMLImportContext* CreateStylesContext(const OUString& rLocalName,
                                            const Reference< XAttributeList>& xAttrList, bool bIsAutoStyle );
    SvXMLImportContext* CreateMetaContext(const sal_Int32 nElement);
    SvXMLImportContext* CreateFontDeclsContext(const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList );
protected:
    // SvXMLImport
    virtual SvXMLImportContext *CreateDocumentContext( sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    virtual SvXMLImportContext *CreateFastContext( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual XMLShapeImportHelper* CreateShapeImport() override;

    virtual ~ORptFilter()  throw() override;
public:

    ORptFilter( const Reference< XComponentContext >& _rxContext, SvXMLImportFlags nImportFlags = SvXMLImportFlags::ALL );

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& rDescriptor ) override;

    /// @throws css::uno::RuntimeException
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
    /// @throws css::uno::RuntimeException
    static OUString getImplementationName_Static();
    static css::uno::Reference< css::uno::XInterface >
    create(css::uno::Reference< css::uno::XComponentContext > const & xContext);

    const Reference<XReportDefinition>& getReportDefinition() const {
        return m_xReportDefinition;
    }
    void FinishStyles();

    virtual void SAL_CALL startDocument() override;
    virtual void SAL_CALL endDocument() override;

    const SvXMLTokenMap& GetDocElemTokenMap() const;
    const SvXMLTokenMap& GetDocContentElemTokenMap() const;
    const SvXMLTokenMap& GetReportElemTokenMap() const;
    const SvXMLTokenMap& GetGroupElemTokenMap() const;
    const SvXMLTokenMap& GetSectionElemTokenMap() const;
    const SvXMLTokenMap& GetComponentElemTokenMap() const;
    const SvXMLTokenMap& GetReportElementElemTokenMap() const;
    const SvXMLTokenMap& GetControlElemTokenMap() const;
    const SvXMLTokenMap& GetControlPropertyElemTokenMap() const;
    const SvXMLTokenMap& GetFunctionElemTokenMap() const;
    const SvXMLTokenMap& GetFormatElemTokenMap() const;
    const SvXMLTokenMap& GetSubDocumentElemTokenMap() const;
    const SvXMLTokenMap& GetColumnTokenMap() const;
    const SvXMLTokenMap& GetCellElemTokenMap() const;

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

/** Imports only settings
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptImportHelper
{
public:
    /// @throws css::uno::RuntimeException
    static OUString getImplementationName_Static(  );
    /// @throws css::uno::RuntimeException
    static Sequence< OUString > getSupportedServiceNames_Static(  );
    static css::uno::Reference< css::uno::XInterface >
    create(css::uno::Reference< css::uno::XComponentContext > const & xContext);
};

/** Imports only content
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptContentImportHelper
{
public:
    /// @throws css::uno::RuntimeException
    static OUString getImplementationName_Static(  );
    /// @throws css::uno::RuntimeException
    static Sequence< OUString > getSupportedServiceNames_Static(  );
    static css::uno::Reference< css::uno::XInterface >
    create(css::uno::Reference< css::uno::XComponentContext > const & xContext);
};

/** Imports only styles
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptStylesImportHelper
{
public:
    /// @throws css::uno::RuntimeException
    static OUString getImplementationName_Static(  );
    /// @throws css::uno::RuntimeException
    static Sequence< OUString > getSupportedServiceNames_Static(  );
    static css::uno::Reference< css::uno::XInterface >
    create(css::uno::Reference< css::uno::XComponentContext > const & xContext);
};

/** Imports only meta data
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptMetaImportHelper
{
public:
    /// @throws css::uno::RuntimeException
    static OUString getImplementationName_Static(  );
    /// @throws css::uno::RuntimeException
    static Sequence< OUString > getSupportedServiceNames_Static(  );
    static css::uno::Reference< css::uno::XInterface >
    create(css::uno::Reference< css::uno::XComponentContext > const & xContext);
};


class RptXMLDocumentBodyContext : public SvXMLImportContext
{
public:
    RptXMLDocumentBodyContext(SvXMLImport & rImport,
           sal_uInt16 const nPrefix,
           const OUString& rLocalName)
        : SvXMLImportContext(rImport, nPrefix, rLocalName)
    {
    }

    virtual SvXMLImportContextRef CreateChildContext(sal_uInt16 const nPrefix,
           const OUString& rLocalName,
           const css::uno::Reference<css::xml::sax::XAttributeList> & xAttrList) override;
};

} // rptxml

#endif // INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
