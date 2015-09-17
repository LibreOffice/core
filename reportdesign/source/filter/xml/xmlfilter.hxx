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
#include <unotools/ucbstreamhelper.hxx>
#include <xmloff/xmlimp.hxx>
#include <comphelper/sequence.hxx>
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


// - ORptFilter -

class ORptFilter : public SvXMLImport
{
public:
    typedef std::map< OUString, Sequence<PropertyValue> > TPropertyNameMap;
    typedef std::map< OUString, Reference<XFunction> > TGroupFunctionMap;
private:

    TGroupFunctionMap                               m_aFunctions;
    Reference< XComponent >                         m_xSrcDoc;

    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pDocElemTokenMap;
    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pReportElemTokenMap;
    mutable ::std::unique_ptr<SvXMLTokenMap>        m_pGroupsElemTokenMap;
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
    rtl::Reference < XMLPropertySetMapper >           m_xTableStylesPropertySetMapper;

    Reference<XReportDefinition>                    m_xReportDefinition;
    std::shared_ptr<rptui::OReportModel>        m_pReportModel;

    bool                            implImport( const Sequence< PropertyValue >& rDescriptor ) throw (RuntimeException, std::exception);

    SvXMLImportContext* CreateStylesContext(const OUString& rLocalName,
                                     const Reference< XAttributeList>& xAttrList, bool bIsAutoStyle );
    SvXMLImportContext* CreateMetaContext(const OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    SvXMLImportContext* CreateFontDeclsContext(const OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
protected:
    // SvXMLImport
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                                      const OUString& rLocalName,
                                      const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;

    virtual XMLShapeImportHelper* CreateShapeImport() SAL_OVERRIDE;

    virtual ~ORptFilter()  throw();
public:

    ORptFilter( const Reference< XComponentContext >& _rxContext, SvXMLImportFlags nImportFlags = SvXMLImportFlags::ALL );

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& rDescriptor ) throw(RuntimeException, std::exception) SAL_OVERRIDE;

    static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( ::com::sun::star::uno::RuntimeException );
    static OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);

    inline Reference<XReportDefinition> getReportDefinition() const { return m_xReportDefinition; }
    void FinishStyles();

    virtual void SAL_CALL startDocument()
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL endDocument()
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    const SvXMLTokenMap& GetDocElemTokenMap() const;
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

    inline rtl::Reference < XMLPropertySetMapper > GetCellStylesPropertySetMapper()      const   { return m_xCellStylesPropertySetMapper;    }
    inline rtl::Reference < XMLPropertySetMapper > GetColumnStylesPropertySetMapper()    const   { return m_xColumnStylesPropertySetMapper;  }
    inline rtl::Reference < XMLPropertySetMapper > GetRowStylesPropertySetMapper()       const   { return m_xRowStylesPropertySetMapper;     }
    static OUString convertFormula(const OUString& _sFormula);
    /** inserts a new function
    *
    * \param _xFunction
    */
    void insertFunction(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunction > & _xFunction);
    void removeFunction(const OUString& _sFunctionName);
    inline const TGroupFunctionMap& getFunctions() const { return m_aFunctions; }

    bool                        isOldFormat() const;
};

/** Imports only settings
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptImportHelper
{
public:
    static OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};

/** Imports only content
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptContentImportHelper
{
public:
    static OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};

/** Imports only styles
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptStylesImportHelper
{
public:
    static OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};

/** Imports only meta data
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptMetaImportHelper
{
public:
    static OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};


} // rptxml

#endif // INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
